/* file "s2m/s2m.cpp" */

/*
   Copyright (c) 2000 The President and Fellows of Harvard College

   All rights reserved.

   This software is provided under the terms described in
   the "machine/copyright.h" include file. 
   */

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "s2m/s2m.h"
#endif

#include <cfenodes/cfe.h>	// needed only in this lowering pass

#include <machine/pass.h>
#include <machine/machine.h>
#include <suifvm/suifvm.h>

#include <s2m/s2m.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace suifvm;		// all opcodes are SUIFvm


  S2mSuifPass::S2mSuifPass(SuifEnv *suif_env, const IdString& name)
: PipelinablePass(suif_env, name)
{
  the_suif_env = suif_env;
  cur_line_annote = NULL;
}

  void
S2mSuifPass::initialize()
{
  PipelinablePass::initialize();

  // Create parse tree for parsing the command line.  This must occur
  // after the parent class's initialize routine has been executed.
  _command_line->set_description("lower from SUIF to Machine SUIF");

  // Collect optional flags.
  // [Currently, this pass has only one optional flag, but we set things
  //  up for the general case.]
  OptionSelection *flags = new OptionSelection(false);

  OptionList *l;

  // -debug level
  l = new OptionList;
  l->add(new OptionLiteral("-debug"));
  l->add(new OptionInt("level", &debuglvl));
  l->set_description("set verbosity level for debugging messages");
  flags->add(l);

  // Accept tagged options in any order.
  _command_line->add(new OptionLoop(flags));

  // Set flag defaults.
  // [Nothing needed for this pass, yet.]

  // zero or more file names
  file_names = new OptionString("file name");
  OptionLoop *files =
    new OptionLoop(file_names,
        "names of optional input and/or output files");
  _command_line->add(files);
  o_fname = empty_id_string;

  // Initialize rest of object state.
  mil = NULL;
}


  bool
S2mSuifPass::parse_command_line(TokenStream *command_line_stream)
{
  if (!PipelinablePass::parse_command_line(command_line_stream))
    return false;

  debug(1, "Debug level is %d", debuglvl);

  o_fname = process_file_names(file_names);

  return true;
}

  void
S2mSuifPass::execute()
{
  PipelinablePass::execute();

  // Process the output file name, if any.
  if (!o_fname.is_empty())
    the_suif_env->write(o_fname.chars());
}

  void
S2mSuifPass::do_file_set_block(FileSetBlock *fsb)
{
  // Search the external symbol table for machine-independent types.
  // (This needs only needs to be done here, not in any other
  // Machine-SUIF passes.)
  attach_opi_predefined_types(fsb);

  claim(o_fname.is_empty() || fsb->get_file_block_count() == 1,
      "Command-line output file => file set must be a singleton");
}


  void
S2mSuifPass::do_file_block(FileBlock *fb)
{
  source_file = fb->get_source_file_name();
  debug(2, "Processing source file %s", source_file.chars());

  // sanity check
  claim(!has_note(fb, k_target_lib));

  // Define libsuifvm as the target architecture library.

  OneNote<IdString> note(k_suifvm);
  set_note(fb, k_target_lib, note);

  // Focus the OPI.  (Should be done in all Machine-SUIF passes.)
  the_context = target_context(fb);
}


  void
S2mSuifPass::do_procedure_definition(ProcDef *pd)
{
  cur_unit = pd;
  IdString cur_pname = get_name(cur_unit);
  debug(3, "Processing procedure %s", cur_pname.chars());

  // Focus the OPI for the current optimization unit.  (Should be
  // done in all Machine-SUIF passes.)
  focus(cur_unit);

  // Set and clear any per-procedure variables used in conversion here.
  is_varargs = false;		// till we see va_start(...)
  is_leaf = true;		// till we see a call

  // Build a machine-level instance of the procedure's body.  We put the
  // converted instructions in the data member mil, which is initialized
  // by convert_eo and filled by convert_expression.  Once we have built
  // a new InstrList, we steal the old body's annotations, delete the
  // original body, and update the ProcedureDefinition's body pointer.

  ExecutionObject *suif2_il = cur_unit->get_body();
  convert_eo(suif2_il);
  copy_notes(suif2_il, mil);

  set_body(cur_unit, mil);
  delete suif2_il;

  // mark procedure entry point -- at top by default
  Instr *mi = new_instr_dot(opcode_null);

  set_note(mi, k_proc_entry, note_flag());
  prepend(mil, mi);

  // If the front end didn't attach a line note to the procedure
  // definition, then do it here.  It only needs to carry the file name.

  if (!has_note(cur_unit, k_line)) {
    LineNote note;
    note.set_file(source_file);
    note.set_line(0);
    set_note(cur_unit, k_line, note);
  }

  // Create or update properties needed by code_gen/code_fin
  StackFrameInfoNote sfi_note = take_note(cur_unit, k_stack_frame_info);
  if (is_null(sfi_note))
    sfi_note = StackFrameInfoNote();
  sfi_note.set_is_varargs(is_varargs);
  sfi_note.set_is_leaf(is_leaf);
  sfi_note.set_max_arg_area(0);	// place holder

  set_note(cur_unit, k_stack_frame_info, sfi_note);

  // Drop the focus on the current optimization unit.  (Should be
  // done in all Machine-SUIF passes.)
  defocus(cur_unit);
  cur_unit = NULL;
}


  void
S2mSuifPass::convert_eo(ExecutionObject *the_eo)
{
  // sanity checks
  if (the_eo == NULL)
    return;

  // construct a new local instruction list
  mil = new_instr_list();

  claim(is_kind_of<StatementList>(the_eo),
      "Found non-StatementList ExecutionObject");

  StatementList *stmts = static_cast<StatementList*>(the_eo);
  Statement     *stmt;
  for (Iter<Statement *> stmts_iter = stmts->get_statement_iterator();
      stmts_iter.is_valid();
      stmts_iter.next())
    convert_statement(stmt = stmts_iter.current());

  // The front end may allow the end of a procedure body to be reached
  // without a return instruction.  If the last statement of the body is
  // a label, emit a value-less return instruction so that control
  // doesn't wander off aimlessly.

  if (is_kind_of<LabelLocationStatement>(stmt))
    emit(new_instr_cti(RET, NULL));
}


  Opnd
S2mSuifPass::convert_address_op(Expression *e)
{
  // A variable operand V means that variable V holds the address, not
  // that V is the addressed location.
  // An Expression operand I yields the effective address.
  // Generate 0(base), where base is either variable V or the result of I.
  TypeId e_type = e->get_result_type();
  TypeId deref_type = is_pointer(e_type) ? get_referent_type(e_type) : NULL;
  if (deref_type == NULL)
    warn("convert_address_op: referent type is NULL");
  Opnd base = convert_expression(e);

  /*
  // PRINTME JOHN
  fprintf(stderr, "s2m::convert_address_op(line:%d): ", __LINE__);
  fprint(stderr, e_type);
  fprintf(stderr, ":");
  fprint(stderr, deref_type);
  fprintf(stderr, "\n");
  */

  return BaseDispOpnd(base, opnd_immed(0, type_u8), deref_type);
}


/***** Expression translators *****/

  Opnd
S2mSuifPass::convert_expression(Expression *e)
{
  // handle constants
  if (is_kind_of<Constant>(e))
    return convert_constant_expression((Constant *)e);

  // handle operations
  else if (is_kind_of<LoadVariableExpression>(e))
    return convert_load_variable_expression((LoadVariableExpression *)e);

  else if (is_kind_of<SymbolAddressExpression>(e))
    return convert_symbol_address_expression((SymbolAddressExpression *)e);

  else if (is_kind_of<BinaryExpression>(e)) 
    return convert_binary_expression((BinaryExpression *)e);

  else if (is_kind_of<UnaryExpression>(e)) 
    return convert_unary_expression((UnaryExpression *)e);

  else if (is_kind_of<SelectExpression>(e)) 
    return convert_select_expression((SelectExpression *)e);

  else if (is_kind_of<LoadExpression>(e)) 
    return convert_load_expression((LoadExpression *)e);

  else if (is_kind_of<CallExpression>(e)) 
    return convert_call_expression((CallExpression *)e);

  else if (is_kind_of<ArrayReferenceExpression>(e)) 
    return
      convert_array_reference_expression((ArrayReferenceExpression *)e);

  else if (is_kind_of<VaArgExpression>(e)) 
    return convert_va_arg_expression((VaArgExpression *)e);

  else if (e) {
    warn("Unrecognized expression of class `%s' found.",
        get_class_name(e));
    return convert_generic_expression(e);
  }

  return opnd_null();
}

  Opnd
S2mSuifPass::convert_constant_expression(Constant *e)
{
  Type *t = e->get_result_type();
  Opnd c;

  if (is_kind_of<IntConstant>(e)) {
    c = opnd_immed(((IntConstant *)e)->get_value(), t);
  } else {
    claim(is_kind_of<FloatConstant>(e));
    c = opnd_immed(((FloatConstant *)e)->get_value(), t);
  }
  Opnd d = opnd_reg(t);
  Instr *mi = new_instr_alm(d, LDC, c);
  emit(mi);
  return d;
}

  Opnd
S2mSuifPass::convert_load_variable_expression(LoadVariableExpression *in)
{
  return opnd_var(in->get_source());
}


  Opnd
S2mSuifPass::convert_binary_expression(BinaryExpression *in)
{
  int suifvm_opcode;
  Expression *s1 = in->get_source1();
  bool switch_operands = false;

  IdString opc = in->get_opcode();
  if (opc == k_add)
    suifvm_opcode = ADD;
  else if (opc == k_subtract)
    suifvm_opcode = SUB;
  else if (opc == k_multiply)
    suifvm_opcode = MUL;
  else if (opc == k_divide)
    suifvm_opcode = DIV;
  else if (opc == k_remainder)
    suifvm_opcode = REM;
  else if (opc == k_bitwise_and)
    //suifvm_opcode = AND;
    suifvm_opcode = BAND;
  else if (opc == k_bitwise_or)
    //suifvm_opcode = IOR;
    suifvm_opcode = BIOR;
  else if (opc == k_bitwise_xor)
    //suifvm_opcode = XOR;
    suifvm_opcode = BXOR;
  else if (opc == k_left_shift)
    suifvm_opcode = LSL;
  else if (opc == k_right_shift) {
    Type *s1t = s1->get_result_type();
    if (is_kind_of<IntegerType>(s1t))
      suifvm_opcode =
        ((IntegerType*)s1t)->get_is_signed() ? ASR : LSR;
    else
      suifvm_opcode = LSR;
  }
  else if (opc == k_rotate)
    suifvm_opcode = ROT;
  else if (opc == k_is_equal_to)
    suifvm_opcode = SEQ;
  else if (opc == k_is_not_equal_to)
    suifvm_opcode = SNE;
  else if (opc == k_is_less_than)
    suifvm_opcode = SL;
  else if (opc == k_is_less_than_or_equal_to)
    suifvm_opcode = SLE;
  else if (opc == k_is_greater_than) {
    switch_operands = true;
    suifvm_opcode = SL;
  }
  else if (opc == k_is_greater_than_or_equal_to) {
    switch_operands = true;
    suifvm_opcode = SLE;
  }
  else if (opc == k_logical_and)
    //suifvm_opcode = AND;
    suifvm_opcode = LAND;
  else if (opc == k_logical_or)
    //suifvm_opcode = IOR;
    suifvm_opcode = LOR;
  else if (opc == k_maximum)
    //suifvm_opcode = MAX;
    suifvm_opcode = MAX2;
  else if (opc == k_minimum)
    //suifvm_opcode = MIN;
    suifvm_opcode = MIN2;
  else
    claim(false, "BinaryExpression with opcode `%s' found",
        IdString(in->get_opcode()).chars());

  Expression *s2 = in->get_source2();
  if (switch_operands) {
    Expression *t = s1;
    s1 = s2;
    s2 = t;
  }

  Opnd d = opnd_reg(in->get_result_type());
  Instr *mi = new_instr_alm(d, suifvm_opcode,
      convert_expression(s1),
      convert_expression(s2));
  copy_notes(in, mi);
  emit(mi);

  return d;
}


  Opnd
S2mSuifPass::convert_unary_expression(UnaryExpression *in)
{
  int suifvm_opcode;
  Opnd s2 = opnd_null();

  IdString opc = in->get_opcode();
  if (opc == k_negate)
    suifvm_opcode = NEG;
  else if (opc == k_absolute_value)
    suifvm_opcode = ABS;
  else if (opc == k_bitwise_not)
    suifvm_opcode = NOT;
  else if (opc == k_logical_not) {
    suifvm_opcode = SEQ;
    s2 = opnd_immed(0, in->get_source()->get_result_type());
  }
  else if (opc == k_convert)
    suifvm_opcode = CVT;
  else if (opc == k_treat_as) {
    Type *source_type = in->get_source()->get_result_type();
    Type *result_type = in->get_result_type();
    claim((source_type == NULL) || (result_type == NULL) ||
        (is_kind_of<IntegerType>(source_type) &&
         is_kind_of<IntegerType>(result_type)
         && ((to<IntegerType>(source_type))->get_bit_size() ==
           (to<IntegerType>(result_type))->get_bit_size())) ||
        (is_kind_of<PointerType>(source_type) &&
         is_kind_of<PointerType>(result_type)),
        "``Treat As'' Expression found with bad types");
    suifvm_opcode = CVT;
  }
  else
    claim(false, "UnaryExpression with opcode `%s' found",
        IdString(in->get_opcode()).chars());

  Opnd d = opnd_reg(in->get_result_type());
  Instr *mi = new_instr_alm(d, suifvm_opcode,
      convert_expression(in->get_source()));
  if (!is_null(s2))
    set_src(mi, 1, s2);
  copy_notes(in, mi);
  emit(mi);

  return d;
}


  Opnd
S2mSuifPass::convert_select_expression(SelectExpression *in)
{
  Integer selection1_value;
  Expression *selection1_op = in->get_selection1();
  if (is_kind_of<IntConstant>(selection1_op)) {
    IntConstant *the_constant = (IntConstant*)selection1_op;
    selection1_value = the_constant->get_value();
  }
  Integer selection2_value;
  Expression *selection2_op = in->get_selection2();
  if (is_kind_of<IntConstant>(selection2_op)) {
    IntConstant *the_constant = (IntConstant*)selection2_op;
    selection2_value = the_constant->get_value();
  }
  claim((((selection1_value == 0) && (selection2_value == 1)) ||
        ((selection1_value == 1) && (selection2_value == 0))),
      "Non-zero/one SelectExpression found.");

  Instr *mi;
  Opnd cmp_result = convert_expression(in->get_selector());

  // was the polarity correct?
  if (selection1_value == 0) {
    mi = new_instr_alm(SEQ, cmp_result,
        opnd_immed(0, get_type(cmp_result)));
    cmp_result = opnd_reg(get_type(cmp_result));
    set_dst(mi, 0, cmp_result);
    emit(mi);
  }

  // generate the final convert
  Opnd d = opnd_reg(in->get_result_type());
  mi = new_instr_alm(d, CVT, cmp_result);
  copy_notes(in, mi);
  emit(mi);

  return d;
}


  Opnd
S2mSuifPass::convert_load_expression(LoadExpression *in)
{
  TypeId d_type = in->get_result_type();
  Opnd d = opnd_reg(d_type);
  Opnd a = convert_address_op(in->get_source_address());
  TypeId s_type = get_deref_type(a);
  Instr *load;
#ifndef CFE_LOAD_EXP_FIXED
  // FIXME: The front end should not fold an implicit type conversion
  // into a LoadExpression.  If it has done so, make the CVT explicit.
  if (!TypeHelper::is_isomorphic_type(d_type, s_type)
      && is_scalar(d_type) && is_scalar(s_type)) {
    Opnd t = opnd_reg(s_type);
    load = new_instr_alm(t, LOD, a);
    emit(load);
    emit(new_instr_alm(d, CVT, t));
  } else {
    load = new_instr_alm(d, LOD, a);
    emit(load);
  }
#else
  load = new_instr_alm(d, LOD, a);
  emit(load);
#endif
  copy_notes(in, load);
  return d;
}

  Opnd
S2mSuifPass::convert_symbol_address_expression(SymbolAddressExpression *in)
{
  // FIXME: front end ought to do this, NOT!  Have to fix for multidimensional arrays
  // Update 2007Feb24, fixed, should work now for 1d and 2d arrays
 
  // PRINTME JOHN
  //fprintf(stderr, "s2m::convert_symbol_address_exp(line:%d): ", __LINE__);

  Sym *sym = in->get_addressed_symbol();
  Type *ut = unqualify_type(sym->get_type());

  // if is array type, keep traversing tree to see its base data type
  while( is_kind_of<ArrayType>(ut) ) {
      Type *temp_q_element_type = ((ArrayType*)ut)->get_element_type();
      Type *temp_uq_element_type = unqualify_type(temp_q_element_type);
      ut = temp_uq_element_type;
  }
  
  const int ref_type_bitsize = get_bit_size(ut);
  //fprintf(stderr, "Array: p%d\n", ref_type_bitsize);
  TypeId ptr_type = build_typeid_type(TYPEID_PTR, ref_type_bitsize);
  Opnd d2 = opnd_reg(ptr_type);


  sym->set_is_address_taken(true);	
  Instr *mi = new_instr_alm(d2, LDA, opnd_addr_sym(sym, ptr_type));
  copy_notes(in, mi);
  emit(mi);

  return d2;
}

/*
 * Yes, this is a peephole optimization (Arghhh!).  If the target of the
 * call is expressed as a simple symbol, return that symbol in `callee' so
 * that we can construct a direct call, and in that case, return a null
 * operand from this mehtod.  Otherwise, set `callee' to NULL and return a
 * non-null operand to be used as the callee register in an indirect call.
 */
  Opnd
S2mSuifPass::convert_callee_address_expression(Expression *e, Sym **callee)
{
  *callee = NULL;

  if (!is_kind_of<SymbolAddressExpression>(e) || e->get_annote_count())
    return convert_expression(e);

  SymbolAddressExpression *in = (SymbolAddressExpression *)e;
  *callee = in->get_addressed_symbol();
  return opnd_null();
}

/* Format of resulting call Expression:
 *  label = NULL,	// always call through register
 *  dst[0] = result operand (if exists),
 *  src[0] = callee_address operand,
 *  src[1] = 1st actual parameter,
 *   ...
 *  src[N] = Nth actual parameter. */
  Opnd
S2mSuifPass::convert_call_expression(CallExpression *in)
{
  // deal with conversion of callee target -- only one of callee and
  // callee_sym will be non-null
  Sym *callee_sym;
  Opnd callee = convert_callee_address_expression(in->get_callee_address(),
      &callee_sym);

  // build the suifvm call instruction
  TypeId result_type = in->get_result_type();
  Instr *mi;
  Opnd result;
  if (result_type == NULL		// C front end bug
      || is_kind_of<VoidType>(result_type)) {
    result = opnd_null();
    mi = new_instr_cti(CAL, callee_sym, callee);
  } else {
    result = opnd_reg(result_type);
    mi = new_instr_cti(result, CAL, callee_sym, callee);
  }

  // convert the arguments
  for (unsigned n = 1; n <= in->get_argument_count(); ++n)
    set_src(mi, n, convert_expression(in->get_argument(n - 1)));

  copy_notes(in, mi);
  emit(mi);

  is_leaf = false;
  return result;
}

/*
 * From the array-reference, obtain the base and index expressions.
 *
 * The base must be a pointer (not reference [1]) type whose referent type
 * may be an array type or not.  If it's an array type, check that its
 * lower bound expression is the constant zero.  Obtain the bit-size of its
 * element type and assume that this has been padded so that consecutive
 * elements are properly aligned.  Also, convert the array base to a base
 * pointer having the type of the overall array reference.
 *
 * Otherwise, the base is not an array, but a base pointer whose referent
 * type is the type of the element being selected.
 *
 * Compute scale factor as element-bit-size divided by bits_per_byte
 * (obtainable from the target-info-block).
 *
 * Construct a multiply of that constant times the index expression, using
 * the type of the latter as the type of the scaling constant.  Then put
 * that in an add instruction whose other operand is the base pointer and
 * whose result type is the same as that of the base pointer.  Return the
 * destination operand of that add instruction.
 *
 * -------------
 * [1] This pass should never be given a ReferenceType.
 */
  Opnd
S2mSuifPass::convert_array_reference_expression(ArrayReferenceExpression *are)
{

  // PRINTME JOHN
  //fprintf(stderr, "s2m::convert_array_ref_exp(line:%d): ", __LINE__);


  Expression *base_exp  = are->get_base_array_address();
  Expression *index_exp = are->get_index();

  DataType *base_type = base_exp->get_result_type();
  TypeId referent_type =
    unqualify_type(to<PointerType>(base_type)->get_reference_type());
  TypeId element_type;

  bool is_array_referent = is_kind_of<ArrayType>(referent_type);

  if (is_array_referent) {
    ArrayType *array_type = to<ArrayType>(referent_type);

    Expression *lower = array_type->get_lower_bound();
    claim(is_kind_of<IntConstant>(lower) &&
        ((IntConstant*)lower)->get_value() == 0,
        "Non-zero lower bound for array reference");
    element_type = array_type->get_element_type();
  } else {
    element_type = referent_type;
  }
  int element_bit_size = get_bit_size(element_type);
  TargetInformationBlock *target_info = 
    find_target_information_block(the_suif_env);
  int byte_size = target_info->get_byte_size().c_int();
  claim(element_bit_size % byte_size == 0);
  int element_byte_size = element_bit_size / byte_size;
  TypeId word_type = target_info->get_word_type();

  Opnd index = convert_expression(index_exp);
  if (element_byte_size > 1) {
    Opnd scale = opnd_immed(element_byte_size, word_type);
    Opnd scaled = opnd_reg(word_type);
    emit(new_instr_alm(scaled, MUL, index, scale));
    index = scaled;
  }
  Opnd base = convert_expression(base_exp);
  Opnd base_ptr;
  if (!is_array_referent)
    base_ptr = base;
  else {
    base_ptr = opnd_reg(are->get_result_type());
    emit(new_instr_alm(base_ptr, CVT, base));
  }
  Opnd element_ptr = opnd_reg(get_type(base_ptr));
  Instr *mi = new_instr_alm(element_ptr, ADD, base_ptr, index);
  copy_notes(are, mi);
  emit(mi);
  return element_ptr;
}

/*
 * Handle va_arg(ap, value_type)
 *
 * Emit an ANY whose name is __builtin_va_arg, whose src(0) is the ap
 * variable, and whose destination is a new VR.  Let the new VR have type
 * pointer-to-value_type, with suitable bit size for the current target.
 * (Attach a k_builtin_args note to the emitted ANY instruction, but only
 * to mark it as a call on a builtin; it carries no other info.)
 *
 * Then emit a load whose effective address indirects through the new VR
 * just defined.  Let its result type be value_type.
 */
  Opnd
S2mSuifPass::convert_va_arg_expression(VaArgExpression *vax)
{
  Expression *ap_exp = vax->get_ap_address();
  TypeId value_type = vax->get_result_type();
  VarSym *ap_var;

  if (is_kind_of<LoadVariableExpression>(ap_exp))
    ap_var = ((LoadVariableExpression*)ap_exp)->get_source();
  else {
    Sym *s = to<SymbolAddressExpression>(ap_exp)->get_addressed_symbol();
    ap_var = to<VarSym>(s);
  }
  Opnd ap = opnd_var(ap_var);
  Opnd addr = opnd_reg(pointer_type(value_type));

  Instr *mi = new_instr_alm(addr, ANY, ap);
  set_note(mi, k_instr_opcode, OneNote<IdString>("__builtin_va_arg"));
  set_note(mi, k_builtin_args, ListNote<IrObject*>());

  emit(mi);

  Opnd value = opnd_reg(value_type);
  Opnd ea = BaseDispOpnd(addr, opnd_immed(0, type_s32), value_type);
  mi = new_instr_alm(value, LOD, ea);
  copy_notes(vax, mi);
  emit(mi);

  return value;
}

  Opnd
S2mSuifPass::convert_generic_expression(Expression *in)
{
  Instr *mi = new_instr_alm(ANY);

  // remember original opcode
  OneNote<IdString> note(in->get_class_name());
  set_note(mi, k_instr_opcode, note);

  copy_notes(in, mi);
  emit(mi);

  return opnd_null();
}


/***** statement translators *****/

  void
S2mSuifPass::convert_statement(Statement *s)
{
  Annote *old_line_annote = cur_line_annote;
  Annote *new_line_annote = s->peek_annote(k_line);
  if (new_line_annote)
    cur_line_annote = new_line_annote;

  if (is_kind_of<EvalStatement>(s)) 
    convert_eval_statement((EvalStatement *)s);

  else if (is_kind_of<StoreStatement>(s)) 
    convert_store_statement((StoreStatement *)s);

  else if (is_kind_of<StoreVariableStatement>(s)) 
    convert_store_variable_statement((StoreVariableStatement *)s);

  else if (is_kind_of<CallStatement>(s)) 
    convert_call_statement((CallStatement *)s);

  else if (is_kind_of<ReturnStatement>(s)) 
    convert_return_statement((ReturnStatement *)s);

  else if (is_kind_of<JumpStatement>(s)) 
    convert_jump_statement((JumpStatement *)s);

  else if (is_kind_of<JumpIndirectStatement>(s)) 
    convert_jump_indirect_statement((JumpIndirectStatement *)s);

  else if (is_kind_of<BranchStatement>(s)) 
    convert_branch_statement((BranchStatement *)s);

  else if (is_kind_of<MultiWayBranchStatement>(s)) 
    convert_multiway_branch_statement((MultiWayBranchStatement *)s);

  else if (is_kind_of<LabelLocationStatement>(s)) 
    convert_label_location_statement((LabelLocationStatement *)s);

  else if (is_kind_of<MarkStatement>(s)) 
    convert_mark_statement((MarkStatement *)s);

  else if (is_kind_of<VaStartStatement>(s)) 
    convert_va_start_statement((VaStartStatement *)s);

  else if (is_kind_of<VaEndStatement>(s)) 
    convert_va_end_statement((VaEndStatement *)s);

  else if (s) {
    warn("Unrecognized statement of class `%s' found.", get_class_name(s));
    convert_generic_statement(s);
  }
  cur_line_annote = old_line_annote;
}


  void
S2mSuifPass::convert_eval_statement(EvalStatement *stmt)
{
  for (int i = 0; i < stmt->get_expression_count(); i++)
    convert_expression(stmt->get_expression(i));
}

  void
S2mSuifPass::convert_store_statement(StoreStatement *stmt)
{
  Opnd src = convert_expression(stmt->get_value());
  TypeId src_type = unqualify_type(get_type(src));
  Opnd dst_addr = convert_address_op(stmt->get_destination_address());

  if (is_var(src) && is_record(src_type)) {
    Opnd src_vr = opnd_reg(src_type);
    emit(new_instr_alm(src_vr, LOD, opnd_addr_sym(get_var(src))));
    src = src_vr;
  }
#ifndef CFE_STORE_STMT_FIXED
  // FIXED: The front end should not fold an implicit type conversion
  // into a StoreStatement.  If it has done so, make the CVT explicit.
  if (!is_record(src_type)) {
    TypeId dst_type = unqualify_type(get_deref_type(dst_addr));
    if (!TypeHelper::is_isomorphic_type(dst_type, src_type)) {
      Opnd converted = opnd_reg(dst_type);
      emit(new_instr_alm(converted, CVT, src));
      src = converted;
    }
  }
#endif
  Instr *mi = new_instr_alm(dst_addr, STR, src);
  copy_notes(stmt, mi);
  emit(mi);
}

  void
S2mSuifPass::convert_store_variable_statement(StoreVariableStatement *stmt)
{
  VarSym *dst_var = stmt->get_destination();

  Opnd src = convert_expression(stmt->get_value());
  TypeId src_type = unqualify_type(get_type(src));
  TypeId dst_type = unqualify_type(get_type(dst_var));

#ifndef CFE_STORE_VAR_FIXED
  // FIXME: The front end should not fold an implicit type conversion
  // into a StoreStatement.  If it has done so, make the CVT explicit.
  if (!TypeHelper::is_isomorphic_type(dst_type, src_type)
      && is_integral(src_type) && is_integral(dst_type)) {
    if_debug(1) {
      fprintf(stderr, "Adding conversion from ");
      fprint (stderr, src_type);
      fprintf(stderr, " to ");
      fprint (stderr, dst_type);
      fprintf(stderr, "\n");
    }
    Opnd new_src = opnd_reg(dst_type);
    emit(new_instr_alm(new_src, CVT, src));
    src = new_src;
  }
#endif

  Instr *mi;

  if (is_record(src_type) || is_array(src_type)) {
    if (is_var(src)) {
      Opnd src_vr = opnd_reg(src_type);
      mi = new_instr_alm(src_vr, LOD, opnd_addr_sym(get_var(src)));
      emit(mi);
      src = src_vr;
    }
    mi = new_instr_alm(opnd_addr_sym(dst_var), STR, src);
  } else {
    mi = new_instr_alm(opnd_var(dst_var), MOV, src);
  }
  copy_notes(stmt, mi);
  emit(mi);
}

/* Format of resulting call instruction:
 *  target = callee symbol, if known (in which case src[0] is null), else NULL
 *  dst[0] = result operand if callee returns a value, else null operand
 *  src[0] = callee-address operand, null exactly when target is non-NULL
 *  src[1] = 1st actual parameter,
 *   ...
 *  src[N] = Nth actual parameter. */
  void
S2mSuifPass::convert_call_statement(CallStatement *stmt)
{
  // deal with conversion of callee target -- only one of callee and
  // callee_sym will be non-null
  Sym *callee_sym;
  Opnd callee = convert_callee_address_expression(stmt->get_callee_address(),
      &callee_sym);
  Instr *mi = new_instr_cti(CAL, callee_sym, callee);

  // insert dst opnd, if callee returns a value
  VarSym *dv = stmt->get_destination();
  if (dv != NULL)
    set_dst(mi, opnd_var(dv));

  // convert the arguments
  for (unsigned n = 1; n <= stmt->get_argument_count(); ++n)
    set_src(mi, n, convert_expression(stmt->get_argument(n - 1)));

  copy_notes(stmt, mi);
  is_leaf = false;
  emit(mi);
}


  void
S2mSuifPass::convert_return_statement(ReturnStatement *stmt)
{
  Opnd value = convert_expression(stmt->get_return_value());

#ifndef CFE_RET_CVT_FIXED
  // FIXME: I added this patch because I thought that the C front end
  // should have widened a sub-word integral result to type int.  I
  // insert a CVT if it hasn't done so.  However, I can't find this in
  // the C standard, so it may need to be handled in individual code
  // generators instead.

  TypeId type = get_type(value);
  int size = get_bit_size(type);
  if (size > 0) {
    TargetInformationBlock *target_info = 
      find_target_information_block(the_suif_env);
    TypeId word_type = target_info->get_word_type();
    if (get_bit_size(word_type) > size) {
      Opnd word = opnd_reg(word_type);
      emit(new_instr_alm(word, CVT, value));
      value = word;
    }
  }
#endif

  Instr *mi = new_instr_cti(RET, NULL, value);
  copy_notes(stmt, mi);
  emit(mi);
}

  void
S2mSuifPass::convert_jump_statement(JumpStatement *stmt)
{
  Instr *mi = new_instr_cti(JMP, stmt->get_target());
  copy_notes(stmt, mi);
  emit(mi);
}

  void
S2mSuifPass::convert_jump_indirect_statement(JumpIndirectStatement *stmt)
{
  Instr *mi = new_instr_cti(JMPI, NULL, 
      convert_expression(stmt->get_target()));
  copy_notes(stmt, mi);
  emit(mi);
}

/* This routine is more complicated than the other convert routines
 * because it attempts to produce more complex instructions when possible.
 * We will combine a conditional compare operation with its dependent
 * branch instruction, if possible, so that we will not "lose"
 * information--i.e., lose the obvious fact that the result of a compare
 * operation is used only by a conditional branch instruction.  This
 * action makes it easier to code effective gen passes. */
  void
S2mSuifPass::convert_branch_statement(BranchStatement *stmt)
{
  Instr *mi = NULL;

  // check the target code_label_symbol
  CodeLabelSymbol *suifvm_target = stmt->get_target();
  claim(suifvm_target);

  // SUIF has only a single conditional branch opcode -- branch on true.
  // We start with this as the default suifvm opcode.  If the decision
  // operand is something simple that we can short-circuit, we fold the
  // unary logical-not, if it exists, into the decision operand.  If the
  // decision operand cannot be folded into the conditional branch
  // operation, we fold the unary not, if it exists, into the branch
  // opcode to produce a suifvm BFALSE operation.
  bool sc_failed = false;
  int suifvm_opcode = BTRUE;
  Expression *cond_in = stmt->get_decision_operand();

  // deal with a possible logical_not
  UnaryExpression *logical_not_in = NULL;
  if (is_kind_of<UnaryExpression>(cond_in)) {
    logical_not_in = (UnaryExpression *)cond_in;
    if (logical_not_in->get_opcode() == k_logical_not) {
      // found logical_not expression
      cond_in = logical_not_in->get_source();
    } else {		// wrong kind of unary instruction
      sc_failed = true;
      logical_not_in = NULL;
    }
  }

  // if ok so far, check for binary comparison to short circuit
  if (is_kind_of<BinaryExpression>(cond_in)) {
    // found collapsible compare and branch
    BinaryExpression *cmp_in = (BinaryExpression *)cond_in;
    IdString opc = cmp_in->get_opcode();
    if (opc == k_is_equal_to)
      suifvm_opcode = (logical_not_in) ? BNE : BEQ;
    else if (opc == k_is_not_equal_to)
      suifvm_opcode = (logical_not_in) ? BEQ : BNE;
    else if (opc == k_is_less_than)
      suifvm_opcode = (logical_not_in) ? BGE : BLT;
    else if (opc == k_is_less_than_or_equal_to)
      suifvm_opcode = (logical_not_in) ? BGT : BLE;
    else if (opc == k_is_greater_than)
      suifvm_opcode = (logical_not_in) ? BLE : BGT;
    else if (opc == k_is_greater_than_or_equal_to)
      suifvm_opcode = (logical_not_in) ? BLT : BGE;
    else
      claim(false, "Unexpected decision opcode `%s' found",
          IdString(cmp_in->get_opcode()).chars());

    mi = new_instr_cti(suifvm_opcode, suifvm_target,
        convert_expression(cmp_in->get_source1()),
        convert_expression(cmp_in->get_source2()));
    if (logical_not_in)
      copy_notes(logical_not_in, mi);
    copy_notes(cmp_in, mi);

  } else
    sc_failed = true;

  if (sc_failed) {	// perform convert without full short-circuit
    if (logical_not_in)
      suifvm_opcode = BFALSE;

    mi = new_instr_cti(suifvm_opcode, suifvm_target,
        convert_expression(cond_in));

    copy_notes(stmt, mi);
    if (logical_not_in)
      copy_notes(logical_not_in, mi);
  }

  claim(mi);		// sanity check of convoluted logic
  emit(mi);
}

/*
 * Format of resulting multi-way branch instruction:
 *  target = default target,
 *  dst[0] = <none>,
 *  src[0] = decision operand.
 *
 * The case constants and target labels are recorded in a k_instr_mbr_tgts
 * annotation. * We assume that the case constants occur in increasing
 * order, but may have gaps.  The C front ends and lowering passes
 * establish this condition and they ensure reasonable compactness.
 *
 * FIXME: In most cases they also do a range check on the decision operand
 * prior to the multiway branch.  It would be easy to do that in all cases,
 * so that the extra range check could be dropped here.
 *
 * FIXME: convertsuif1to2b creates gaps by skipping default labels that the
 * SUIF1 front end has put in.  It would be nice if this were eliminated.
 */
  void
S2mSuifPass::convert_multiway_branch_statement(MultiWayBranchStatement *stmt)
{
  LabelSym *default_target = stmt->get_default_target();
  Opnd decision = convert_expression(stmt->get_decision_operand());

  // create annotation with targets, leaving room for dispatch-table symbol
  MbrNote note;
  note.set_table_sym(NULL);
  note.set_default_label(default_target);

  // Copy (constant, target-label) pairs, filling in any gaps.

  long first, last;			// first, last case constants
  Iter<MultiWayBranchStatement::case_pair> iter =
    stmt->get_case_iterator();
  for (long i = 0,  next;
      iter.is_valid();
      iter.next(),  ++i,  last = next) {
    next = iter.current().first.c_int();
    if (i == 0)
      first = next;
    else
      for (long j = last + 1 ; j < next; ++j, ++i) {
        note.set_case_constant(j, i);
        note.set_case_label(default_target, i);
      }
    note.set_case_constant(next, i);
    note.set_case_label(iter.current().second, i);
  }
  // Check for decision opnd outside [first,last].
  // FIXME: in nearly all cases, the front end has added such a check
  // already.
  Opnd lower = opnd_reg(type_s32);
  Opnd upper = opnd_reg(type_s32);

  emit(new_instr_alm(lower, LDC, opnd_immed(first, type_s32)));
  emit(new_instr_cti(BLT, default_target, decision, lower));

  emit(new_instr_alm(upper, LDC, opnd_immed(last,  type_s32)));
  emit(new_instr_cti(BGT, default_target, decision, upper));

  // Emit the MBR instruction.
  Instr *mi = new_instr_cti(MBR, default_target, decision);
  set_note(mi, k_instr_mbr_tgts, note);
  copy_notes(stmt, mi);
  emit(mi);
}

  void
S2mSuifPass::convert_label_location_statement(LabelLocationStatement *stmt)
{
  Instr *mi = new_instr_label(stmt->get_defined_label());
  copy_notes(stmt, mi);
  emit(mi);
}

  void
S2mSuifPass::convert_mark_statement(MarkStatement *stmt)
{
  Instr *mi = new_instr_dot(MRK);
  copy_notes(stmt, mi);
  emit(mi);
}

/*
 * Handle va_start(ap, param_n), i.e., the stdarg variant of a va_start
 * call.
 *
 * Emit a generic (ANY) instruction with opcode_name "__builtin_va_start" and
 * three operands, the two variables corresponding to ap and param_n, and
 * an integer immediate equal to one (to indicate stdarg instead of
 * varargs, whose indicator is zero).
 *
 * Attach the k_builtin_args note to the emitted instruction, but only
 * to mark it as a call on a builtin function; it carries no other info.
 */
  void
S2mSuifPass::convert_va_start_statement(VaStartStatement *stmt)
{
  Expression *ap_exp = stmt->get_ap_address();
  VarSym *ap_var;

  if (is_kind_of<LoadVariableExpression>(ap_exp))
    ap_var = ((LoadVariableExpression*)ap_exp)->get_source();
  else {
    Sym *s = to<SymbolAddressExpression>(ap_exp)->get_addressed_symbol();
    ap_var = to<VarSym>(s);
  }
  Opnd ap = opnd_var(ap_var);
  Opnd param_n = opnd_var(stmt->get_parmn());

  Instr *mi = new_instr_alm(ANY, ap, param_n);
  set_src(mi, 2, opnd_immed(1, type_s32));	// 1 => stdarg, 0 => varargs

  copy_notes(stmt, mi);

  set_note(mi, k_instr_opcode, OneNote<IdString>("__builtin_va_start"));
  set_note(mi, k_builtin_args, ListNote<IrObject*>());

  emit(mi);
  is_varargs = true;
}



/*
 * Handle va_end(ap).
 *
 * Emit a generic (ANY) instruction with opcode_name "__builtin_va_end" and
 * one operand, the variable corresponding to ap.
 *
 * Attach the k_builtin_args note to the emitted instruction, but only
 * to mark it as a call on a builtin function; it carries no other info.
 */
  void
S2mSuifPass::convert_va_end_statement(VaEndStatement *stmt)
{
  Expression *ap_exp = stmt->get_ap_address();
  VarSym *ap_var;

  if (is_kind_of<LoadVariableExpression>(ap_exp))
    ap_var = ((LoadVariableExpression*)ap_exp)->get_source();
  else {
    Sym *s = to<SymbolAddressExpression>(ap_exp)->get_addressed_symbol();
    ap_var = to<VarSym>(s);
  }
  Opnd ap = opnd_var(ap_var);

  Instr *mi = new_instr_alm(ANY, ap);

  copy_notes(stmt, mi);

  set_note(mi, k_instr_opcode, OneNote<IdString>("__builtin_va_end"));
  set_note(mi, k_builtin_args, ListNote<IrObject*>());

  emit(mi);
}

  void
S2mSuifPass::convert_generic_statement(Statement *stmt)
{
  Instr *mi = new_instr_alm(ANY);

  // remember original opcode
  OneNote<IdString> note(stmt->get_class_name());
  set_note(mi, k_instr_opcode, note);

  copy_notes(stmt, mi);
  emit(mi);
}

/*
 * Append instruction `mi' to the output list `mil'.  If there is a
 * non-null annote cached in cur_line_annote, and if `mi' doesn't already
 * have a line annote, then clone cur_line_annote and attach it to `mi'.
 */
  void
S2mSuifPass::emit(Instr *mi)
{
  if (cur_line_annote && !mi->peek_annote(k_line))
    mi->append_annote((Annote*)cur_line_annote->deep_clone(the_suif_env));
  if_debug(6) {
    fprintf(stderr, "[%lx]: ", (unsigned long)mi);
    fprint (stderr, mi);
  }
  append(mil, mi);
}

  extern "C" void
init_s2m(SuifEnv *suif_env)
{
  static bool init_done = false;

  if (init_done)
    return;
  init_done = true;

  ModuleSubSystem *mSubSystem = suif_env->get_module_subsystem();
  mSubSystem->register_module(new S2mSuifPass(suif_env));

  init_suifpasses(suif_env);
  init_cfenodes(suif_env);
  init_machine(suif_env);
  init_suifvm(suif_env);
}
