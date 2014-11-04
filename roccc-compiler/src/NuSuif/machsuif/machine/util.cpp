/* file "machine/util.cpp" */

/*
   Copyright (c) 2000 The President and Fellows of Harvard College

   All rights reserved.

   This software is provided under the terms described in
   the "machine/copyright.h" include file.
   */

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "machine/util.h"
#endif

#include <machine/substrate.h>
#include <machine/machine_ir.h>
#include <machine/problems.h>
#include <machine/init.h>
#include <machine/opnd.h>
#include <machine/instr.h>
#include <machine/util.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif


IdString make_unique_sym_name(SymTable*, IdString prefix);


/* -------------------------  instruction editing  ------------------------- */

static void
map_opnd_vector(OpndHandle, OpndHandle, OpndFilter&, OpndFilter::InOrOut);

  void
map_opnds(Instr *instr, OpndFilter &filter)
{
  map_src_opnds(instr, filter);
  map_dst_opnds(instr, filter);
}

  void
map_src_opnds(Instr *instr, OpndFilter &filter)
{
  if (srcs_size(instr) > 0)
    map_opnd_vector(srcs_start(instr), srcs_end(instr), filter,
        OpndFilter::IN);
}

  void
map_dst_opnds(Instr *instr, OpndFilter &filter)
{
  if (dsts_size(instr) >0)
    map_opnd_vector(dsts_start(instr), dsts_end(instr), filter,
        OpndFilter::OUT);
}

  void
map_opnds(Opnd addr_exp, OpndFilter &filter)
{
  AddrExpOpnd aeo(addr_exp);
  claim(!is_null(aeo), "map_opnds applied to non-address-expression");

  map_opnd_vector(aeo.srcs_start(), aeo.srcs_end(), filter, OpndFilter::IN);
}

  void
map_opnd_vector(OpndHandle start, OpndHandle end, OpndFilter &filter,
    OpndFilter::InOrOut dir)
{
  for (OpndHandle handle = start; handle != end; handle++) {

    if (!is_addr_exp(*handle) || !filter.thru_addr_exps())
      *handle = filter(*handle, dir);
    else if (AddrExpOpnd aeo = (Opnd)*handle)
      map_opnd_vector(aeo.srcs_start(), aeo.srcs_end(), filter,
          OpndFilter::IN);
  }
}


/* ---------------------------  symbol utilities  --------------------------- */

  bool
is_global(Sym *sym)
{
  return is_global(sym->get_symbol_table());
}

  bool
is_external(Sym *sym)
{
  return is_external(sym->get_symbol_table());
}

  bool
is_defined(Sym *sym)
{
  if (!is_global(sym))
    return false;
  else if (is_kind_of<VarSym>(sym))
    return ((VarSym*)sym)->get_definition() != NULL;
  else if (is_kind_of<ProcSym>(sym))
    return ((ProcSym*)sym)->get_definition() != NULL;
  return false;
}

bool is_private(Sym *sym)
{
  return is_private(sym->get_symbol_table());
}

bool is_auto(VarSym *var)
{
  return !is_global(var->get_symbol_table())
    && var->get_definition() == NULL;
}

  VarSym*
lookup_local_var(IdString name)
{
  claim(the_local_unit, "Forget to focus on opt unit?");
  return lookup_var_locally(find_scope(the_local_unit), name);
}

  VarSym*
lookup_external_var(IdString name)
{
  return lookup_var_locally(external_sym_table(), name);
}

  ProcSym*
lookup_external_proc(IdString name)
{
  return lookup_proc_locally(external_sym_table(), name);
}


/* ---------------------------  symbol creators  --------------------------- */

  static QualifiedType*
make_var_type(TypeId type)
{
  if (is_kind_of<QualifiedType>(type))
    return (QualifiedType*)type;
  return retrieve_qualified_type(to<DataType>(type));
}

// local helper
  VarSym*
new_named_var(TypeId t, IdString name, SymTable *scope)
{
  VarSym *var = create_variable_symbol(the_suif_env, make_var_type(t), name);
  scope->add_symbol(var);
  return var;
}

  VarSym*
new_named_var(TypeId t, IdString name)
{
  claim(the_local_unit, "Forget to focus on opt unit?");
  ScopeTable *scope = find_scope(the_local_unit);

  claim(scope->num_lookup_table_with_key(name) == 0,
      "Cannot add local variable with name `%s'", name.chars());
  return new_named_var(t, name, scope);
}

  VarSym*
new_unique_var(TypeId t, const char *prefix)
{
  claim(the_local_unit, "Forget to focus on opt unit?");
  SymTable *scope = find_scope(the_local_unit);

  IdString name = make_unique_sym_name(scope, prefix);
  return new_named_var(t, name, scope);
}

/*
 * Create a new local var with a unique name and an initial value given by
 * `init', which must either be a numerically-typed immediate operand
 * or an untyped immediate string.
 */
  VarSym*
new_unique_var(Opnd init, const char *prefix)
{
  claim(is_immed(init));
  SuifEnv* const env = the_suif_env;
  TypeId t = get_type(init);
  VarSym *var;
  VarDef *vd;

  if (t == NULL) {
    const char *chars = get_immed_string(init).chars();
    int length = strlen(chars);
    DataType *var_type = to<DataType>(array_type(type_s8, 0, length));
    var = new_unique_var(var_type, prefix);

    MultiValueBlock *mvb = create_multi_value_block(env, var_type);
    for (int i = 0; i <= length; ++i) {	// include null terminator
      DataType *dt = to<DataType>(type_s8);
      Constant *c = create_int_constant(env, dt, Integer(chars[i]));
      ValueBlock *char_value = create_expression_value_block(env, c);
      mvb->add_sub_block(i * 8, char_value);
    }
    vd = create_variable_definition(env, var, 8, mvb);

  } else {
    var = new_unique_var(t, prefix);
    DataType *dt = unqualify_data_type(t);
    Constant *c =
      (is_kind_of<FloatingPointType>(dt))
      ? static_cast<Constant*>
      (create_float_constant(env, dt, get_immed_string(init)))
      : static_cast<Constant*>
      (create_int_constant  (env, dt, get_immed_integer(init)));
    ExpressionValueBlock *xvb = create_expression_value_block(env, c);
    vd = create_variable_definition(env, var, dt->get_bit_alignment(), xvb);
  }
  var->set_definition(vd);
  DefinitionBlock *db = the_local_unit->get_definition_block();
  db->append_variable_definition(vd);
  return var;
}

  LabelSym*
new_unique_label(const char *prefix)
{
  return new_unique_label(the_suif_env, the_local_scope, prefix);
}

/*
 * Create and initialize an mbr dispatch table from values in the argument
 * MbrNote.
 *
 * o  Create the array type dttype and a new local VarSym dtsym.
 *
 * o  Give dtsym a definition that initializes the table with the mbr
 *    target labels.  For each explicit case constant, use the corres-
 *    ponding explicit label.  For those in between, use the default label
 *    (the target field of the MBR instruction).
 *
 *    (The dispatch table is initialized as though it were a local static
 *    array in the source procedure; no executable code is generated for
 *    that.)
 *
 * o  Store dtsym in the MbrNote.
 *
 * o  Return dtsym.
 */
  VarSym*
new_dispatch_table_var(MbrNote &note)
{
  int num_cases = note.get_case_count();
  claim(num_cases > 0);

  int lower = note.get_case_constant(0);
  int upper = note.get_case_constant(num_cases - 1);
  claim(lower <= upper);

  DataType *dttype = to<DataType>(array_type(type_ptr, lower, upper));
  VarSym *dtsym = new_unique_var(dttype);
  MultiValueBlock *dtvalue = create_multi_value_block(the_suif_env, dttype);

  int elem_bit_size = get_bit_size(type_ptr),
      elem_bit_alignment = get_bit_alignment(type_ptr);

  int k = 0;					// explicit case number
  Sym *label;					// current-case label

  for (int i = 0; i <= (upper - lower); i++) {
    int value = note.get_case_constant(k);
    if (value == (lower + i))
      label = note.get_case_label(k++);
    else
      label = note.get_default_label();

    Expression *label_expr =
      create_symbol_address_expression(the_suif_env,
          to<DataType>(type_ptr), label);
    ValueBlock *label_value =
      create_expression_value_block(the_suif_env, label_expr);

    dtvalue->add_sub_block(elem_bit_size * i, label_value);
  }

  claim(k == num_cases);

  VariableDefinition *dtvd =
    create_variable_definition(the_suif_env, dtsym,
        elem_bit_alignment, dtvalue);
  the_local_unit->get_definition_block()->append_variable_definition(dtvd);

  dtsym->set_definition(dtvd);
  note.set_table_sym(dtsym);			// complete k_mbr_instr_tgts

  return dtsym;
}

/*
 * Create and return a new array-valued (but uninitialized) local variable
 * with element type `elem_type' and `length' entries.
 */
  VarSym*
new_empty_table_var(TypeId elem_type, int length)
{
  DataType *at = to<DataType>(array_type(elem_type, 0, length - 1));
  VarSym *var = new_unique_var(at);

  int elem_bit_alignment = get_bit_alignment(elem_type);
  UndefinedValueBlock *uvb = create_undefined_value_block(the_suif_env, at);
  VariableDefinition *vd =
    create_variable_definition(the_suif_env, var, elem_bit_alignment, uvb);
  the_local_unit->get_definition_block()->append_variable_definition(vd);

  var->set_definition(vd);
  return var;
}

/*
 * Update one target label in the dispatch table for a multiway branch.
 * Take the dispatch table's variable symbol and the new target label from
 * the annotation `note'.  Position `index' is the zero-based index of the
 * case label.
 */
  void
update_dispatch_table_var(MbrNote &note, int index)
{
  claim(index < note.get_case_count());

  VarSym *dtsym = note.get_table_sym();
  VariableDefinition *dtvd = dtsym->get_definition();
  claim(dtvd != NULL);

  MultiValueBlock *dtvalue = to<MultiValueBlock>(dtvd->get_initialization());
  claim(index < dtvalue->get_sub_block_count());

  Sym *label = note.get_case_label(index);
  Expression *label_expr =
    create_symbol_address_expression(the_suif_env,
        to<DataType>(type_addr()), label);
  ValueBlock *label_value =
    create_expression_value_block(the_suif_env, label_expr);
  int elem_bit_offset = index * get_bit_size(type_ptr);
  delete dtvalue->remove_sub_block(elem_bit_offset);
  dtvalue->insert_sub_block(elem_bit_offset, label_value);
}

/*
 * Find and strip down the dispatch-table definition (if any) associated
 * with a MbrNote, leaving only one entry as a place holder.  Use a zero
 * value in lieu of the label in that one remaining entry.
 *
 * An MbrNote has a table_sym field that is either NULL (if there's no jump
 * table) or the symbol for a compiler-created array variable that holds
 * the table.  The symbol's definition is a multi-value block with at least
 * one subblock.  Holding on to the type of its 1st subblock, delete the
 * existing subblocks and substitute a single, constant-value subblock
 * whose value is zero and whose type is the original subblock type (a
 * pointer type).
 */
  void
strip_dispatch_table_var(MbrNote &note)
{
  claim(!is_null(note));

  VarSym *dtsym = note.get_table_sym();
  if (dtsym == NULL)				// no table in this dialect
    return;

  VarDef *dtvd = dtsym->get_definition();
  MultiValueBlock *dtvalue = to<MultiValueBlock>(dtvd->get_initialization());

  claim(dtvalue->get_sub_block_count() > 0);
  DataType *elem_type = dtvalue->get_sub_block(0).second->get_type();
  int elem_bit_size = get_bit_size(type_ptr);

  for (int i = dtvalue->get_sub_block_count() - 1; i >= 0; --i)
    delete dtvalue->remove_sub_block(i * elem_bit_size);

  Expression *a0 = create_int_constant(the_suif_env, elem_type, 0);
  dtvalue->add_sub_block(0, create_expression_value_block(the_suif_env, a0));
}

/*
 * make_unique_sym_name
 * Choose a fresh name when renaming symbols.
 *
 * Args are the symbol table in which you need a new unique symbol name and
 * a prefix to use in generating the unique name.  The result is a string
 * derived from the prefix by adding a numeric suffix that distinguishes
 * the new name from those used in the table.  Actually, since all symbol
 * names are hashed into a universal "lexicon", the SUIF way is simply to
 * produce a name that is unique in the whole lexicon, and therefore cannot
 * collide in the particular symbol table.
 */
  IdString
make_unique_sym_name(SymTable *st, IdString prefix)
{
  static unsigned char seed;			// to mix up the suffixes a bit

  for (Integer suffix = seed++; ; ++suffix) {
    String next = prefix + "_" + suffix.to_String();
    if (!IdString::exists(next))
      return next;
  }
  return empty_id_string;			// never reached
}

  ProcSym*
find_proc_sym(TypeId type, IdString name)
{
  ProcedureType *pt = to<ProcedureType>(type);
  SymTable *xst = external_sym_table();

  ProcSym *ps = NULL;
  int matches = xst->num_lookup_table_with_key(name);

  for (int i = 0; i < matches; ++i) {
    SymbolTableObject *sto = xst->lookup_lookup_table(name, i);
    if (is_kind_of<ProcSym>(sto)) {
      ProcSym *si = static_cast<ProcSym*>(sto);
      if (TypeHelper::is_isomorphic_type(pt, get_type(si))) {
        ps = si;
        break;
      }
    }
  }
  if (ps == NULL) {
    ps = create_procedure_symbol(the_suif_env, pt, name, true);
    xst->add_symbol(ps);
  }
  return ps;
}


/* -----------------------------  type helpers  ----------------------------- */

// type predicates

  bool
is_void(TypeId t)
{
  return is_kind_of<VoidType>(unqualify_type(t));
}

  bool
is_scalar(TypeId t)
{
  TypeId ut = unqualify_type(t);

  return is_kind_of<IntegerType>(ut)	|| is_kind_of<PointerType>(ut)
    || is_kind_of<BooleanType>(ut)	|| is_kind_of<FloatingPointType>(ut)
    || is_kind_of<EnumeratedType>(ut);
}

  bool
is_integral(TypeId t)
{
  return is_kind_of<IntegerType>(unqualify_type(t));
}

  bool
is_boolean(TypeId t)
{
  return is_kind_of<BooleanType>(unqualify_type(t));
}

  bool
is_signed(TypeId t)
{
  TypeId ut = unqualify_type(t);
  claim(is_kind_of<IntegerType>(ut));

  return ((IntegerType*)ut)->get_is_signed();
}

  bool
is_floating_point(TypeId t)
{
  return is_kind_of<FloatingPointType>(unqualify_type(t));
}

  bool
is_pointer(TypeId type)
{
  return is_kind_of<PointerType>(unqualify_type(type));
}

  bool
is_enum(TypeId t)
{
  return is_kind_of<EnumeratedType>(unqualify_type(t));
}

  bool
is_record(TypeId t)
{
  return is_kind_of<GroupType>(unqualify_type(t));
}

  bool
is_struct(TypeId t)
{
  return is_kind_of<StructType>(unqualify_type(t));
}

  bool
is_union(TypeId t)
{
  return is_kind_of<UnionType>(unqualify_type(t));
}

  bool
is_array(TypeId t)
{
  return is_kind_of<ArrayType>(unqualify_type(t));
}

// type constructors

  TypeId
pointer_type(TypeId referent)
{
  TypeBuilder *tb =
    (TypeBuilder *)the_suif_env->
    get_object_factory(TypeBuilder::get_class_name());
  claim(tb != NULL);

  TargetInformationBlock *tinfo = find_target_information_block(the_suif_env);
  claim(tinfo != NULL);

  Integer size = tinfo->get_pointer_size();
  int alignment = tinfo->get_pointer_alignment();

  return tb->get_pointer_type(size, alignment, referent);
}

  TypeId
array_type(TypeId element_type, int lower_bound, int upper_bound)
{
  TypeBuilder *tb =
    (TypeBuilder *)the_suif_env->
    get_object_factory(TypeBuilder::get_class_name());
  claim(tb != NULL);

  QualifiedType *qet =
    is_kind_of<QualifiedType>(element_type)
    ? (QualifiedType*)element_type
    : retrieve_qualified_type(to<DataType>(element_type));

  return tb->get_array_type(qet, lower_bound, upper_bound);
}

// type field accessors

  TypeId
get_element_type(TypeId type)
{
  return to<ArrayType>(unqualify_type(type))->get_element_type();
}

  TypeId
get_referent_type(TypeId type)
{
  return to<PointerType>(unqualify_type(type))->get_reference_type();
}

  TypeId
get_result_type(TypeId type)
{
  return to<CProcedureType>(unqualify_type(type))->get_result_type();
}

  TypeId
find_proc_type(TypeId result, TypeId arg0, TypeId arg1)
{
  DataType *result_dt = unqualify_data_type(result);
  TypeBuilder *tb = get_type_builder(the_suif_env);
  list<QualifiedType*> args;

  args.push_back(tb->get_qualified_type(arg0));
  args.push_back(tb->get_qualified_type(arg1));
  return tb->get_c_procedure_type(result_dt, args);
}


/* -------------------------  symbol table helpers  ------------------------- */

bool is_global(SymbolTable *st)
{
  if (st == NULL)
    return false;

  IrObject *st_parent = (IrObject*)st->get_parent();

  return is_kind_of<FileBlock>(st_parent)
    || is_kind_of<FileSetBlock>(st_parent);
}

bool is_external(SymbolTable *st)
{
  if (st == NULL)
    return false;

  IrObject *st_parent = (IrObject*)st->get_parent();

  return is_kind_of<FileSetBlock>(st_parent)
    && st == ((FileSetBlock *)st_parent)->get_external_symbol_table();
}

bool is_private(SymbolTable *st)
{
  if (st == NULL)
    return false;

  return is_kind_of<FileBlock>(st->get_parent());
}

  SymTable*
external_sym_table()
{
  FileSetBlock* fsb =
    to<FileSetBlock>(the_file_block->get_parent());

  return fsb->get_external_symbol_table();
}

  SymTable*
file_set_sym_table()
{
  FileSetBlock* fsb =
    to<FileSetBlock>(the_file_block->get_parent());

  return fsb->get_file_set_symbol_table();
}

  SymTable*
get_sym_table(FileBlock *fb)
{
  return fb->get_symbol_table();
}

  SymTable*
get_sym_table(ProcDef *pd)
{
  return pd->get_symbol_table();
}


/* --------------------------  formal parameters  -------------------------- */

  int
get_formal_param_count(OptUnit *unit)
{
  return unit->get_formal_parameter_count();
}

  VarSym*
get_formal_param(OptUnit *unit, int pos)
{
  return unit->get_formal_parameter(pos);
}


/* ---------------------------  annotation help  --------------------------- */

  bool
is_reg_param(VarSym *p)
{
  if (is_null(get_note(p, k_param_reg)))
    return false;
  return true;
}

  int
get_param_reg(VarSym *p)
{
  if (OneNote<long> note = get_note(p, k_param_reg))
    return note.get_value();
  return -1;
}

  void
set_param_reg(VarSym *p, int reg)
{
  set_note(p, k_param_reg, OneNote<long>(reg));
}

  void
copy_notes(IrObject *from, IrObject *to)
{
  Iter<Annote*> anit = from->get_annote_iterator();
  for ( ; anit.is_valid(); anit.next())
    to->append_annote((Annote*)anit.current()->deep_clone(the_suif_env));
}

  void
move_notes(IrObject *from, IrObject *to)
{
  while(from->get_annote_count() > 0)
    to->append_annote(from->remove_annote(0));
}

Set<IdString> nonprinting_notes;



/* -------------------------------  cloning  ------------------------------- */

/*
 * We don't want global symbol-table objects to be cloned during cloning of
 * a local IR object.  So we derive from DispatchCloner, with a CloneAction
 * for SymbolTableObject that distinguishes local from global.  The action
 * for the former is to clone; that for the latter is to retain the
 * reference without cloning the referent.
 *
 * The CloneAction that we define enbeds a reference to the cloner so that it
 * can call the methods that determine whether to clone a subobject or not.
 *
 * The cloner that we define embeds the action that customizes behavior for
 * SymbolTableObjects.  This cannot be a transient value.  It must live as
 * long as the cloner itself.
 */

class RenamingCloner;

class RenamingCloneAction : public CloneAction {
  public:
    RenamingCloneAction(RenamingCloner &cloner)
      : CloneAction(SymbolTableObject::get_class_name()), cloner(cloner) { }

    void object_enquiry(Object*, CloneStreamObjectInstance*, PTR_TYPE);
  protected:
    RenamingCloner &cloner;
};

class RenamingCloner : public DispatchCloner {
  public:
    RenamingCloner(SuifEnv*, SymTable *receiver = NULL);
    void finish_orphan_object_cloning(Object *orig, Object *orphan);
    void set_receiver(SymTable* receiver) { this->receiver = receiver; }

  protected:
    SymTable *receiver;
    RenamingCloneAction action;
};

/*
 * The cloner sees three kinds of subobjects:
 *
 * owned:       having the containing object as parent.
 * defined:     not owned, but always needing to be cloned for some other
 *              reason that depends on location within the IR.  (E.g. the
 *              definition point of a label.) 
 * referenced:  not needing to be cloned except for reasons such as scope.
 *
 * For present purposes, the first two kinds are always cloned.  But
 * references must be handled case by case.
 */
  RenamingCloner::RenamingCloner(SuifEnv *env, SymTable *receiver)
: DispatchCloner(env), receiver(receiver), action(*this)
{
  set_owned_object_handling(CLONE_PTR);
  set_defined_object_handling(CLONE_PTR);
  set_refed_object_handling(ENQUIRE_PTR);
  append_selector(action);
}

/*
 * The finish_orphan_object_cloning method expects to see only STO's that
 * have been cloned and need a new home.
 */
  void
RenamingCloner::finish_orphan_object_cloning(Object*, Object *orphan)
{
  SymbolTableObject *sto = to<SymbolTableObject>(orphan);
  sto->set_parent(NULL);

  IdString name = sto->get_name();
  if (name.is_empty())
    receiver->append_symbol_table_object(sto);
  else {
    sto->set_name(empty_id_string);
    receiver->add_symbol(make_unique_sym_name(receiver, name), sto);
  }
}

/*
 * Given a symbol-table object that is referenced, but not owned, by the
 * object being cloned, decide whether to clone it (because it is not
 * global) or continue to reference it (the global case).
 *
 * We only call this method for referenced (REF_PTR) objects, not for
 * owned ones, because RenamingCloner sets things up that way.
 */
  void
RenamingCloneAction::object_enquiry(Object *sto,
    CloneStreamObjectInstance *handle,
    PTR_TYPE ptr_type)
{
  claim (ptr_type == REF_PTR);
  SymTable *st = to<SymbolTableObject>(sto)->get_symbol_table();
  if (is_global(st))
    cloner.set_reference(handle);
  else
    cloner.set_clone_object(handle);
}

  IrObject*
renaming_clone(IrObject *clonee, SymTable *receiving_scope)
{
  RenamingCloner *cloner = new RenamingCloner(the_suif_env, receiving_scope);
  IrObject *clone = (IrObject*)cloner->clone(clonee);
  delete cloner;
  return clone;
}

/* ---------------------------------------------------------------------- */

  size_t
hash(const unsigned long key)
{
  return (key >> 2) + (key >> 10);
}

  void
fprint(FILE *fp, IrObject *obj)
{
  FormattedText text;
  obj->print(text);
  fprintf(fp, "%s\n", (const char *)text.get_value());
}

  void
fprint(FILE *fp, Integer value)
{
  static char buffer[1024];
  claim(value.written_length() < 1024); 

  value.write(buffer);
  fputs(buffer, fp);
}

/* ---------------------------------------------------------------------- */

  AnyBody*
get_body(OptUnit *unit)
{
  return to<AnyBody>(unit->get_body());
}

  void
set_body(OptUnit *unit, AnyBody *body)
{
  unit->set_body(body);
}

  TypeId
get_type(VarSym *s)
{
  return s->get_type();
}

  TypeId
get_type(ProcSym *s)
{
  return s->get_type();
}

  void
set_type(VarSym *s, TypeId type)
{
  s->set_type(make_var_type(type));
}

  void
set_type(ProcSym *s, TypeId type)
{
  s->set_type(to<ProcedureType>(type));
}

  bool
is_addr_taken(VarSym *s)
{
  return s->get_is_address_taken();
}

  void
set_addr_taken(VarSym *s, bool addr_taken)
{
  s->set_is_address_taken(addr_taken);
}
