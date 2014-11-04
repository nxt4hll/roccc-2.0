// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
/* FILE "dismantle_if.cc" */


/*
       Copyright (c) 1998 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include "common/suif_copyright.h"


#include "iokernel/cast.h"
#include "suifkernel/utilities.h"
#include "common/i_integer.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "cfenodes/cfe.h"
#include "basicnodes/basic_constants.h"
#include "suifkernel/suifkernel_messages.h"
#include "procedure_walker_utilities.h"
#include "utils/expression_utils.h"
#include "utils/type_utils.h"
#include "cfeutils/cfe_dismantle.h"
#include "cfeutils/cexpr_utils.h"
#include "typebuilder/type_builder.h"

#include "expression_dismantlers.h"
#include "utils/trash_utils.h"
#include "common/suif_hash_map.h"
#include "utils/type_utils.h"
#include "common/suif_list.h"
#include "suifkernel/command_line_parsing.h"

field_access_expression_walker::field_access_expression_walker(SuifEnv *the_env,ProcedureDefinition *def)
                : ProcedureWalker(the_env,def,FieldAccessExpression::get_class_name()) {
}

Walker::ApplyStatus field_access_expression_walker::operator () (SuifObject *x)
    {
      static LString k_fields = "fields";
    FieldAccessExpression *the_field_access = to<FieldAccessExpression>(x);
    //Expression *result = NULL;

    Expression *base_group_address = 
      the_field_access->get_base_group_address();
    remove_suif_object(base_group_address);
    //    the_field_access->set_base_group_address(NULL);

    FieldSymbol *the_field = the_field_access->get_field();
    IInteger bit_offset = get_expression_constant(the_field->get_bit_offset());

    TargetInformationBlock *tinfo = 
      find_target_information_block(get_env());
    if (tinfo == NULL) {
      return(Walker::Continue);
    }
    IInteger bits_per_byte = tinfo->get_byte_size();

    IInteger byte_offset = bit_offset / bits_per_byte;
    suif_assert_message(byte_offset * bits_per_byte == bit_offset,
			("Bit offset is not a multiple of the byte size"));

    PointerType *access_result_type = 
      retrieve_pointer_type(get_data_type(the_field));
    

    // This is a bit_offset
    BinaryExpression *new_expr =
      create_binary_expression(get_env(),
			       access_result_type,
		       //       the_field_access->get_result_type(),
			       k_add, base_group_address,
			       create_int_constant(get_env(), byte_offset));
    // Toss on the annotation that tells use the field name
    if (the_field->get_name() != emptyLString) {
      BrickAnnote *an = create_brick_annote(get_env(),
					    k_fields);
      an->append_brick(create_suif_object_brick(get_env(), 
						the_field));
      //an->append_brick(create_string_brick(get_env(), the_field->get_name()));
      new_expr->append_annote(an);
    }

    replace_expression(the_field_access,new_expr);
    trash_it(get_env(), the_field_access);
    //    the_field_access->get_parent()->replace(
    set_address(new_expr);
    return Walker::Replaced;
    }


call_expression_walker::call_expression_walker(SuifEnv *the_env,
					       ProcedureDefinition *def)
  : ProcedureWalker(the_env,def,CallExpression::get_class_name()) {
  set_post_order();
}

Walker::ApplyStatus call_expression_walker::operator () (SuifObject *x)
    {
      CallExpression *exp = to<CallExpression>(x);
      force_dest_not_expr(exp);
      return Walker::Replaced;
    }

CallExpressionDismantlerPass::
CallExpressionDismantlerPass(SuifEnv *the_env, 
			     const LString &name) :
  PipelinablePass(the_env, name) {}
Module *CallExpressionDismantlerPass::clone() const {
  return((Module*)this);
}

void CallExpressionDismantlerPass::initialize() {
  PipelinablePass::initialize();
  _command_line->set_description("Dismantle all CallExpressions into CallStatements");
}

void CallExpressionDismantlerPass::
do_procedure_definition(ProcedureDefinition *pd) {
  list<CallExpression *> *l = collect_objects<CallExpression>(pd);
  for (list<CallExpression*>::iterator iter = l->begin();
       iter != l->end(); iter++) {
    CallExpression *cal = *iter;
    //suif_information(cal, 1, "Preprocessing Call");
    //list<StoreVariableStatement *> *store_list = force_dest_not_expr(cal);
    list<CallExpression *> call_list;
    force_call_dest_not_expr(cal, &call_list);
    for (list<CallExpression*>::iterator siter =
	   call_list.begin(); siter != call_list.end(); siter++) {
      //suif_information(cal, 1, "Dismantling Call");
      CallExpression *cal_expr = *siter;
      dismantle_a_call_expression(cal_expr);
    }
  }
  delete l;
}

LoadExpressionDismantlerPass::
LoadExpressionDismantlerPass(SuifEnv *the_env, 
			     const LString &name) :
  PipelinablePass(the_env, name) {}
Module *LoadExpressionDismantlerPass::clone() const {
  return((Module*)this);
}

void LoadExpressionDismantlerPass::initialize() {
  PipelinablePass::initialize();
  _command_line->set_description("Force all load results into a temporary scalar variable");
}

void LoadExpressionDismantlerPass::
do_procedure_definition(ProcedureDefinition *pd) {
  list<LoadExpression *> *l = collect_objects<LoadExpression>(pd);
  for (list<LoadExpression*>::iterator iter = l->begin();
       iter != l->end(); iter++) {
    LoadExpression *load = *iter;
    force_dest_not_expr(load);
  }
  delete l;
}



FieldBuilderPass::
FieldBuilderPass(SuifEnv *the_env, 
		 const LString &name) :
  PipelinablePass(the_env, name) {}
Module *FieldBuilderPass::clone() const {
  return((Module*)this);
}
void FieldBuilderPass::initialize() {
  PipelinablePass::initialize();
  _command_line->set_description("Build FieldExpressions from arithmetic on pointers to group types.");
}

static FieldSymbol *get_gst_field(GroupSymbolTable *gst, 
				  const String &s) {
  LString ls = s;
  if (!gst->has_lookup_table_member(ls)) return(NULL);
  int num = gst->num_lookup_table_with_key(ls);
  for (int i = 0; i < num; i++) {
    SymbolTableObject* sto = gst->lookup_lookup_table(ls, i);
    if (is_kind_of<FieldSymbol>(sto)) {
      return(to<FieldSymbol>(sto));
    }
  }
  return(NULL);
}

static GroupType *get_pointed_to_group_type(DataType *source_type) {
  if (!is_kind_of<PointerType>(source_type)) return(NULL);
  PointerType *pt = to<PointerType>(source_type);
  Type *ref_t = pt->get_reference_type();
  if (!is_kind_of<QualifiedType>(ref_t)) return(NULL);
  DataType *ref_dt = to<QualifiedType>(ref_t)->get_base_type();
  if (!is_kind_of<GroupType>(ref_dt)) return(NULL);
  return(to<GroupType>(ref_dt));
}

void FieldBuilderPass::
do_procedure_definition(ProcedureDefinition *pd) {
  //SuifEnv *s = pd->get_suif_env();
  static LString k_fields = "fields";
  // This is a suif1 convention

  list<Expression *> l;
  // collect all expression with a "fields" annotation
  for (Iter<Expression> iter = object_iterator<Expression>(pd);
       iter.is_valid(); iter.next()) {
    Expression *expr = &iter.current();
    BrickAnnote *an = to<BrickAnnote>(expr->peek_annote(k_fields));
    if (an != NULL) {
      l.push_back(expr);
    }
  }
  
  for (list<Expression *>::iterator eiter = l.begin();
       eiter != l.end(); eiter++) {
    Expression *expr = *eiter;
    if (!process_a_field_expression(expr)) {
      suif_warning(expr, ("Could not convert field expression"));
      continue;
    }
    //    trash_it(expr);
  }
}
/*
 * this function belongs in the utils library
 * and we should call it there.
 */
bool FieldBuilderPass::process_a_field_expression(Expression *expr) {
  SuifEnv *s = expr->get_suif_env();
  static LString k_fields = "fields";
  BrickAnnote *an = to<BrickAnnote>(expr->peek_annote(k_fields));
  if (an == NULL) return(false);

  suif_assert(an != NULL);
  // If it's a convert, then the offset must be 0
  // Otherwise, if it's an add, then get the offset.
  Expression *base_expr = NULL;
  IInteger offset = 0;
  GroupType *base_gt = NULL;
  if (is_kind_of<UnaryExpression>(expr)) {
    UnaryExpression *uexpr = to<UnaryExpression>(expr);
    if (uexpr->get_opcode() != k_convert) {
      suif_warning(uexpr, "non-convert with fields %s", 

			   uexpr->get_opcode().c_str());
      return(false);
    }
    base_expr = uexpr->get_source();
    base_gt = 
      get_pointed_to_group_type(base_expr->get_result_type());
    if (base_gt == NULL) {
      suif_warning(uexpr, "non-group pointer type for source");
      return(false);
    }
  } else if (is_kind_of<BinaryExpression>(expr)) {
    BinaryExpression *bexpr = to<BinaryExpression>(expr);
    if (bexpr->get_opcode() != k_add) {
      suif_warning(bexpr, "non-add with fields %s", 
			   bexpr->get_opcode().c_str());
      return(false);
    }
    base_expr = bexpr->get_source1();
    base_gt = 
      get_pointed_to_group_type(base_expr->get_result_type());
    if (base_gt == NULL) {
      suif_warning(bexpr, "non-group pointer type for source");
      return(false);
    }
    // get the constant.
    Expression *source2 = bexpr->get_source2();
    if (!is_kind_of<IntConstant>(source2)) {
      suif_warning(bexpr, "non-const add with fields"); 
      return(false);
    }
    offset = to<IntConstant>(source2)->get_value();
  } else {
    suif_warning(expr, "unexpected '%s' with fields", 
		 expr->getClassName().c_str());
      return(false);
  }
    // Now we have 
    //  Expression  *expr - to be replaced.
    //  Expression *base_expr
    //  GroupType *base_gt;
    //  IInteger offset
    // try to do the transform.

  list<FieldAccessExpression *> access_list;
  bool OK = false;

  IInteger actual_offset = 0;
  //  GroupType *gt = base_gt;
  DataType *result_type = base_gt;
  for (int i = 0; i < an->get_brick_count(); i++) {
    SuifBrick *br = an->get_brick(i);
    FieldSymbol *fs = NULL;

    if (is_kind_of<SuifObjectBrick>(br)) {
      SuifObject *so = to<SuifObjectBrick>(br)->get_object();
      if (!is_kind_of<FieldSymbol>(so)) {
	suif_warning("non-field symbol SuifObject in fields");
	break;
      }
      fs = to<FieldSymbol>(so);
    } else {
      if (!is_kind_of<StringBrick>(br)) {
	suif_warning("non-string in fields");
	break;
      }
      String field_name = to<StringBrick>(br)->get_value();
      if (!is_kind_of<GroupType>(result_type)) {
	suif_warning("Too many fields, too few group types");
	break;
      }
      GroupType *gt = to<GroupType>(result_type);
      GroupSymbolTable *gst = gt->get_group_symbol_table();
      fs = get_gst_field(gst, field_name);
      if (fs == NULL) {
	suif_warning("could not find field '%s'", field_name.c_str());
	break;
      }
    }

    Expression *exp = fs->get_bit_offset();
    if (!is_kind_of<IntConstant>(exp)) {
      suif_warning("non-constant bit offset for field");
      break;
    }
    IntConstant *c = to<IntConstant>(exp);
    actual_offset += c->get_value();

    // create it but don't populate it.
    PointerType *pt = retrieve_pointer_type(result_type);

    FieldAccessExpression *fe = 
      create_field_access_expression(s, pt, NULL, fs);
    access_list.push_front(fe);
    result_type = to<DataType>(get_data_type(fs));
    OK = true;
  }
  if (!OK) return(false);

  TargetInformationBlock *tinfo = 
    find_target_information_block(get_suif_env());
  if (tinfo == NULL) {
    return(Walker::Continue);
  }
  IInteger bits_per_byte = tinfo->get_byte_size();

  IInteger bit_offset = offset * bits_per_byte; // where do I read this bits per byte from

  if (actual_offset != bit_offset) {
    suif_warning("Offset %s bytes * %s != computed offset %s bits",
		 offset.to_String().c_str(),
		 bits_per_byte.to_String().c_str(),
		 actual_offset.to_String().c_str());
    return(false);
  }
    
  
  //  SymbolAddressExpression *lse = 
  //    create_symbol_address_expression(s, result_type,
  //				     var);
  remove_suif_object(base_expr);

  FieldAccessExpression *last_access = access_list.back();
  access_list.pop_back();
  
  last_access->set_base_group_address(base_expr);
  while (!access_list.empty()) {
    FieldAccessExpression *access = access_list.back();
    access_list.pop_back();
    access->set_base_group_address(last_access);
    last_access = access;
  }
  replace_expression(expr, last_access);
  trash_it(s, expr);
  return(true);
}


class repeat_builder {
private:
  SuifEnv *_s;
  ExpressionValueBlock *_evb;
  IInteger _evb_val;
  IInteger _base_offset;
  int _count;
  list<MultiValueBlock::sub_block_pair> *_new_list;
  TypeBuilder *_tb;
public:
  repeat_builder(SuifEnv *s) :
    _s(s), _evb(NULL), _evb_val(0), _base_offset(0), _count(0),
    _new_list(new list<MultiValueBlock::sub_block_pair>),
    _tb(NULL)
  {
    _tb = (TypeBuilder *)s->get_object_factory(TypeBuilder::get_class_name());
  }
  ~repeat_builder() {
    delete _new_list;
  }

  void append_value_block(const IInteger &ii, ValueBlock *vb) {
    _new_list->push_back(MultiValueBlock::sub_block_pair(ii, vb));
  }
    
  void clear_pending() {
    if (_count == 0) return;
    if (_count == 1) {
      append_value_block(_base_offset, _evb);
    } else {
      // need to build a RepeatValueBlock
      DataType *dt = _tb->get_array_type(_tb->get_qualified_type(_evb->get_type()),
					 0, _count-1);
      RepeatValueBlock *rvb = create_repeat_value_block(_s, _count, _evb, dt);
      append_value_block(_base_offset, rvb);
    }
    _count = 0;
    _evb = NULL;
    _base_offset = 0;
  }

  void add_non_matching_expression_value_block(const IInteger &ii, 
					       ExpressionValueBlock *evb) {
    clear_pending();
    Expression *exp = evb->get_expression();
    IInteger exp_val = get_expression_constant(exp);
    _evb = evb;
    _evb_val = exp_val;
    _base_offset = ii;
    _count = 1;
  }

  void add_expression_value_block(const IInteger &ii, 
				  ExpressionValueBlock *evb) {
    Expression *exp = evb->get_expression();
    IInteger exp_val = get_expression_constant(exp);
    if (exp_val.is_undetermined()) {
      clear_pending();
      append_value_block(ii, evb);
      return;
    }
    if (_count == 0) {
      add_non_matching_expression_value_block(ii, evb);
      return;
    }
    // check to see if it's the same type and value
    if ((_evb->get_type() != evb->get_type())
	|| (exp_val != _evb_val)
	|| (ii != (_base_offset + _evb->get_type()->get_bit_size() * _count)))
      {
	add_non_matching_expression_value_block(ii, evb);
	return;
      }
    _count++;
    // trash it
    trash_it(_s, evb);
  }

  void add_value_block(const IInteger &ii, ValueBlock *vb) {
    if (!is_kind_of<ExpressionValueBlock>(vb)) {
      clear_pending();
      append_value_block(ii, vb);
      return;
    }
    ExpressionValueBlock *evb = to<ExpressionValueBlock>(vb);
    add_expression_value_block(ii, evb);
  }

  void handle_multi_value_block(MultiValueBlock *mvb) {
    // Use the iterator.
    // remove the first element, 
    //
    while (!_new_list->empty()) _new_list->pop_back();
    _count = 0;

    {for (Iter<MultiValueBlock::sub_block_pair> iter =
	   mvb->get_sub_block_iterator();
	 iter.is_valid(); iter = mvb->get_sub_block_iterator()) {
      const MultiValueBlock::sub_block_pair &pair = iter.current();
      IInteger offset = pair.first;
      ValueBlock *vb = mvb->remove_sub_block(offset, 0);
      vb->set_parent(0);

      suif_assert_message(vb->get_parent() == NULL, 
			  ("removed value block did NOT get parent set to NULL"));
      
      add_value_block(offset, vb);
    }}
    clear_pending();
    
    // Now walk over the "new_list" and add them back
    {for (list<MultiValueBlock::sub_block_pair>::iterator iter =
	   _new_list->begin(); iter != _new_list->end(); iter++) {
      // use add because it's faster.
      IInteger ii = (*iter).first;
      ValueBlock *vb = (*iter).second;
      //      fprintf(stderr, "adding a block\n");
      mvb->add_sub_block(ii, vb);
    }}
  }
};

void RepeatValueBlockBuilderPass::initialize() {
  Pass::initialize();
  _command_line->set_description("build RepeatValueBlocks from multiple identical ExpressionValueBlocks");
}
      
Module *RepeatValueBlockBuilderPass::clone() const {
  return((Module*)this);
}
RepeatValueBlockBuilderPass::
RepeatValueBlockBuilderPass(SuifEnv *s,
			    const LString &name) :
  Pass(s, name)
{
}

void RepeatValueBlockBuilderPass::do_file_set_block(FileSetBlock *fsb) {
  for (Iter<MultiValueBlock> iter = 
	 object_iterator<MultiValueBlock>(fsb);
       iter.is_valid(); iter.next()) {
    MultiValueBlock *mvb = &iter.current();
    repeat_builder bld(fsb->get_suif_env());
    bld.handle_multi_value_block(mvb);
  }
}
      
FoldStatementsPass::FoldStatementsPass(SuifEnv *the_env, 
				     const LString &name) :
  PipelinablePass(the_env, name)
{}
void FoldStatementsPass::initialize() {
  PipelinablePass::initialize();
  _command_line->set_description("Fold constant expressions and simplify empty Statements and Statements with constant expressions");

}
Module *FoldStatementsPass::clone() const { return(Module*)this; }

void FoldStatementsPass::
do_procedure_definition(ProcedureDefinition *proc_def) {
  if (is_kind_of<Statement>(proc_def->get_body())) {
    //Statement *st = 
    fold_and_replace_constants(to<Statement>(proc_def->get_body()));
  }
}
