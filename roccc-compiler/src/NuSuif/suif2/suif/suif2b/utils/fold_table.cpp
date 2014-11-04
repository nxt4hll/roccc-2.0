#include "fold_table.h"
#include "suifkernel/suifkernel_messages.h"
#include "iokernel/cast.h"
#include "basicnodes/basic_constants.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif_factory.h"
#include "suifnodes/suif.h"
#include "trash_utils.h"
#include "expression_utils.h"
#include "statement_utils.h"
#include "cloning_utils.h"
#include "common/suif_hash_map.h"
#include "suifkernel/cascading_map.h"
#include "suifkernel/module_subsystem.h"

static IInteger binary_default_fn(const IInteger &val1, const IInteger &val2,
				  const IInteger &bit_size, bool is_signed) {
  IInteger val;
  return(val);
}

static IInteger binary_add_fn(const IInteger &val1, const IInteger &val2,
			      const IInteger &bit_size, bool is_signed) {
  IInteger val = val1 + val2;
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}

static IInteger binary_subtract_fn(const IInteger &val1, const IInteger &val2,
			      const IInteger &bit_size, bool is_signed) {
  IInteger val = val1 - val2;
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}

static IInteger binary_multiply_fn(const IInteger &val1, const IInteger &val2,
				   const IInteger &bit_size, bool is_signed) {
  IInteger val = val1 * val2;
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
static IInteger binary_divide_fn(const IInteger &val1, const IInteger &val2,
				 const IInteger &bit_size, bool is_signed) {
  IInteger val = val1 / val2;
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
static IInteger binary_divfloor_fn(const IInteger &val1, const IInteger &val2,
				   const IInteger &bit_size, bool is_signed) {
  IInteger val = (val1 - val1 % val2) / val2;
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
static IInteger binary_divceil_fn(const IInteger &val1, const IInteger &val2,
				  const IInteger &bit_size, bool is_signed) {
  IInteger val;
  IInteger r = val1 % val2;
  if (r == 0) {
    val = val1 / val2;
  } else {
    val = (val1 - r) / val2 + 1;
  }
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
static IInteger binary_remainder_fn(const IInteger &val1, const IInteger &val2,
				    const IInteger &bit_size, bool is_signed) {
  IInteger val = val1 % val2;
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
static IInteger binary_bitwise_and_fn(const IInteger &val1, const IInteger &val2,
				      const IInteger &bit_size, bool is_signed) {
  IInteger val = val1 & val2;
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
static IInteger binary_bitwise_or_fn(const IInteger &val1, const IInteger &val2,
				     const IInteger &bit_size, bool is_signed) {
  IInteger val = val1 | val2;
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
static IInteger binary_bitwise_nand_fn(const IInteger &val1, const IInteger &val2,
				       const IInteger &bit_size, bool is_signed) {
  IInteger val = ~(val1 & val2);
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
static IInteger binary_bitwise_nor_fn(const IInteger &val1, const IInteger &val2,
				      const IInteger &bit_size, bool is_signed) {
  IInteger val = ~(val1 | val2);
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
static IInteger binary_bitwise_xor_fn(const IInteger &val1, const IInteger &val2,
				      const IInteger &bit_size, bool is_signed) {
  IInteger val = val1 ^ val2;
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
static IInteger binary_left_shift_fn(const IInteger &val1,
				     const IInteger &val2,
				     const IInteger &bit_size, 
				     bool is_signed) {
  IInteger val = val1 << val2;
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
static IInteger binary_right_shift_fn(const IInteger &val1,
					      const IInteger &val2,
					      const IInteger &bit_size, 
					      bool is_signed) {
  IInteger val = val1 >> val2;
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
static IInteger binary_rotate_fn(const IInteger &val1,
				 const IInteger &val2,
				 const IInteger &bit_size, bool is_signed) {
  // Code cribbed from the suif1 constant folding.
  if (bit_size == 0)
    return(val1);
  
  IInteger remainder = val2 % bit_size;
  if (remainder < 0)
    remainder += bit_size;
  suif_assert((remainder >= 0) && (remainder < bit_size));
  
  IInteger top_mask =
    (~(IInteger(-1) << remainder)) << (bit_size - remainder);
  IInteger bottom_mask = ~(IInteger(-1) << (bit_size - remainder));
  
  IInteger base = val1;
  IInteger top_part = (top_mask & base) >> (bit_size - remainder);
  IInteger bottom_part = (bottom_mask & base) << remainder;
  
  IInteger val = top_part | bottom_part;
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
static IInteger binary_is_equal_to_fn(const IInteger &val1,
				      const IInteger &val2,
				      const IInteger &bit_size, 
				      bool is_signed) {
  IInteger val;
  if (val1.is_undetermined() || val2.is_undetermined()) return(val);
  bool result = (val1 == val2);
  val = (int)result;
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
static IInteger binary_is_not_equal_to_fn(const IInteger &val1,
				      const IInteger &val2,
				      const IInteger &bit_size, 
				      bool is_signed) {
  IInteger val;
  if (val1.is_undetermined() || val2.is_undetermined()) return(val);
  val = (int)(val1 != val2);
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
static IInteger binary_is_less_than_fn(const IInteger &val1,
				      const IInteger &val2,
				      const IInteger &bit_size, 
				      bool is_signed) {
  IInteger val;
  if (val1.is_undetermined() || val2.is_undetermined()) return(val);
  val = (int)(val1 < val2);
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
static IInteger binary_is_less_than_or_equal_to_fn(const IInteger &val1,
						   const IInteger &val2,
						   const IInteger &bit_size, 
						   bool is_signed) {
  IInteger val;
  if (val1.is_undetermined() || val2.is_undetermined()) return(val);
  val = (int)(val1 <= val2);
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
static IInteger binary_is_greater_than_fn(const IInteger &val1,
				      const IInteger &val2,
				      const IInteger &bit_size, 
				      bool is_signed) {
  IInteger val;
  if (val1.is_undetermined() || val2.is_undetermined()) return(val);
  val = (int)(val1 > val2);
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
static IInteger binary_is_greater_than_or_equal_to_fn(const IInteger &val1,
						   const IInteger &val2,
						   const IInteger &bit_size, 
						   bool is_signed) {
  IInteger val;
  if (val1.is_undetermined() || val2.is_undetermined()) return(val);
  val = (int)(val1 >= val2);
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
static IInteger binary_logical_and_fn(const IInteger &val1, const IInteger &val2,
				      const IInteger &bit_size, bool is_signed) {
  IInteger val;
  if (val1.is_undetermined() || val2.is_undetermined()) return(val);
  val = (int)(val1 && val2);
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
static IInteger binary_logical_or_fn(const IInteger &val1, const IInteger &val2,
				     const IInteger &bit_size, bool is_signed) {
  IInteger val;
  if (val1.is_undetermined() || val2.is_undetermined()) return(val);
  val = (int)(val1 || val2);
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
static IInteger binary_minimum_fn(const IInteger &val1, const IInteger &val2,
				  const IInteger &bit_size, bool is_signed) {
  IInteger val;
  if (val1.is_undetermined() || val2.is_undetermined()) return(val);
  if (val1 <= val2) val = val1;
  if (val1 <= val2) val = val2;
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}

static IInteger binary_maximum_fn(const IInteger &val1, const IInteger &val2,
				  const IInteger &bit_size, bool is_signed) {
  IInteger val;
  if (val1.is_undetermined() || val2.is_undetermined()) return(val);
  if (val1 >= val2) val = val1;
  if (val1 <= val2) val = val2;
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}

static IInteger unary_default_fn(const IInteger &srcval, 
				  const IInteger &bit_size, bool is_signed) {
  IInteger val;
  return(val);
}

static IInteger unary_negate_fn(const IInteger &srcval,
				const IInteger &bit_size, bool is_signed) {
  if (srcval.is_undetermined()) return(srcval);
  IInteger val = (-srcval);
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
/* This is one of the silliest functions if the result is an integer */
static IInteger unary_invert_fn(const IInteger &srcval,
				const IInteger &bit_size, bool is_signed) {
  IInteger val;
  if (srcval == 0)
    return(val);
  if (srcval.is_undetermined())
    return(srcval);
  return(0);
}
static IInteger unary_absolute_value_fn(const IInteger &srcval,
					const IInteger &bit_size, bool is_signed) {
  IInteger val = srcval;
  if (srcval.is_undetermined()) return(srcval);
  if (srcval < 0)
    val = (-srcval);
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
static IInteger unary_bitwise_not_fn(const IInteger &srcval,
				     const IInteger &bit_size, bool is_signed) {
  if (srcval.is_undetermined()) return(srcval);
  IInteger val = ~srcval;
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
static IInteger unary_logical_not_fn(const IInteger &srcval,
				     const IInteger &bit_size, bool is_signed) {
  if (srcval.is_undetermined()) return(srcval);
  IInteger val = (int)(!srcval);
  return(IInteger::ii_finite_size(val, bit_size, is_signed));
}
static IInteger unary_copy_fn(const IInteger &srcval,
			      const IInteger &bit_size, bool is_signed) {
  if (srcval.is_undetermined()) return(srcval);
  return(IInteger::ii_finite_size(srcval, bit_size, is_signed));
}


/*
 * Folding and replacing expressions
 */
static Expression *fold_and_replace_default_expression(Expression *expr) {
  return(expr);
}

static Expression *fold_and_replace_int_constant(Expression *expr) {
  return(expr);
}
static Expression *fold_and_replace_float_constant(Expression *expr) {
  return(expr);
}

static Expression *fold_and_replace_binary_expression(Expression *expr) {
  SuifEnv *s = expr->get_suif_env();
  FoldTable *fold_table = FoldTable::get_fold_table(s);
  BinaryExpression *bin_expr = to<BinaryExpression>(expr);
  DataType *result_type = bin_expr->get_result_type();
  
  IInteger bit_size = result_type->get_bit_size();
  bool is_signed = false;
  if (is_kind_of<IntegerType>(result_type)) 
    is_signed = to<IntegerType>(result_type)->get_is_signed();
  else if (is_kind_of<BooleanType>(result_type))
    is_signed = true;
  else if (is_kind_of<PointerType>(result_type))
    is_signed = true;
  else 
    return(expr);
    
  Expression *source1 = 
    fold_table->fold_and_replace_expression(bin_expr->get_source1());
  Expression *source2 = 
    fold_table->fold_and_replace_expression(bin_expr->get_source2());
  IInteger src1 = get_expression_constant(source1);
  IInteger src2 = get_expression_constant(source2);
  LString opcode = bin_expr->get_opcode();
    
  IInteger result = fold_table->binary_fold(opcode, src1, src2, 
					    bit_size, is_signed);
  if (result.is_undetermined()) {
    return(expr);
  }
  // otherwise, create a new constant.
  Expression *new_expr = create_int_constant(s, result_type, result);
  trash_it(s, replace_expression(expr, new_expr));
  return(new_expr);
}

static Expression *fold_and_replace_unary_expression(Expression *expr) {
  SuifEnv *s = expr->get_suif_env();
  FoldTable *fold_table = FoldTable::get_fold_table(s);
  UnaryExpression *unary_expr = to<UnaryExpression>(expr);
  DataType *result_type = unary_expr->get_result_type();
    
  IInteger bit_size = result_type->get_bit_size();
  bool is_signed = false;
  if (is_kind_of<IntegerType>(result_type)) 
    is_signed = to<IntegerType>(result_type)->get_is_signed();
  else if (is_kind_of<BooleanType>(result_type))
    is_signed = true;
  else if (is_kind_of<PointerType>(result_type))
    is_signed = true;
  else 
    return(expr);
    
  Expression *source = 
    fold_table->fold_and_replace_expression(unary_expr->get_source());
  IInteger src = get_expression_constant(source);
  LString opcode = unary_expr->get_opcode();
    
  IInteger result = fold_table->unary_fold(opcode, src,
					   bit_size, is_signed);
  if (result.is_undetermined()) {
    return(expr);
  }
  // otherwise, create a new constant.
  Expression *new_expr = create_int_constant(s, result_type, result);
  trash_it(s, replace_expression(expr, new_expr));
  return(new_expr);
}

static Statement *fold_and_replace_default_statement(Statement *stmt) {
  // replace the sub statements and the expressions
  SuifEnv *s = stmt->get_suif_env();
  FoldTable *fold_table = FoldTable::get_fold_table(s);
  list<Statement*> st_list;
  list<Expression*> exp_list;
  for (Iter<Statement*> iter = stmt->get_child_statement_iterator();
       iter.is_valid(); iter.next()) 
    st_list.push_back(iter.current());

  {for (Iter<Expression*> iter = stmt->get_source_op_iterator();
       iter.is_valid(); iter.next()) 
    exp_list.push_back(iter.current());
  }

  {for (list<Statement*>::iterator iter = st_list.begin();
       iter != st_list.end(); iter++) {
    fold_table->fold_and_replace_statement(*iter);
  }}

  {for (list<Expression*>::iterator iter = exp_list.begin();
       iter != exp_list.end(); iter++) {
    fold_table->fold_and_replace_expression(*iter);
  }}
  
  return(stmt);
}

static Statement *fold_and_replace_eval_statement(Statement *stmt) {
  SuifEnv *s = stmt->get_suif_env();
  FoldTable *fold_table = FoldTable::get_fold_table(s);
  EvalStatement *eval = to<EvalStatement>(stmt);
  size_t num_exprs = eval->get_expression_count();
  for (size_t num = num_exprs; num != 0; num--) {
    Expression *expr = eval->get_expression(num-1);
    expr = fold_table->fold_and_replace_expression(expr);
    IInteger src = get_expression_constant(expr);
    // If it is a constant, it does not need to be evaluated.
    if (!src.is_undetermined()) {
      eval->remove_expression(num-1);
      trash_it(s, expr);
    }
  }
  if (eval->get_expression_count() == 0) {
    trash_it(s, remove_statement(stmt));
    return(NULL);
  }
  return(eval);
}

static Statement *fold_and_replace_if_statement(Statement *stmt) {
  SuifEnv *s = stmt->get_suif_env();
  FoldTable *fold_table = FoldTable::get_fold_table(s);
  IfStatement *the_if = to<IfStatement>(stmt);
  Expression *cond = 
    fold_table->fold_and_replace_expression(the_if->get_condition());
  IInteger src = get_expression_constant(cond);
  // If it is a constant, it does not need to be evaluated.

  Statement *new_statement = NULL;

  if (!src.is_undetermined()) {
    if (src == 0) {
      new_statement = remove_statement(the_if->get_else_part());
    } else {
      new_statement = remove_statement(the_if->get_then_part());
    }
    trash_it(s, replace_statement(the_if, new_statement));
    return(fold_table->fold_and_replace_statement(new_statement));
  }
  
  // Otherwise, fold them both
  fold_table->fold_and_replace_statement(the_if->get_then_part());
  fold_table->fold_and_replace_statement(the_if->get_then_part());
  return(the_if);
}

static Statement *fold_and_replace_for_statement(Statement *stmt) {
  SuifEnv *s = stmt->get_suif_env();
  FoldTable *fold_table = FoldTable::get_fold_table(s);
  ForStatement *the_for = to<ForStatement>(stmt);

  // First fold the lower bounds and upper bounds expressions.
  fold_table->fold_and_replace_expression(the_for->get_lower_bound());
  fold_table->fold_and_replace_expression(the_for->get_upper_bound());

  IInteger ii = evaluate_for_statement_entry_test(the_for);
  if (ii.is_undetermined() || (ii != 0)) {
    // fold the prepad
    fold_table->fold_and_replace_statement(the_for->get_pre_pad());
    // fold the body
    fold_table->fold_and_replace_statement(the_for->get_body());

    // Now check.  If the body and prepad are null then we can
    // replace the for with an assignment of the
    // COUNT expression to Index.
    return(stmt);
  }
  // The for will never be executed.  Replace it with 
  // an assignment to the index variable.
  Statement *assign =
    build_assign(the_for->get_index(),
		 deep_suif_clone<Expression>(the_for->get_lower_bound()));
  
  trash_it(replace_statement(the_for, assign));
  return(assign);
}

static Statement *fold_and_replace_branch_statement(Statement *stmt) {
  SuifEnv *s = stmt->get_suif_env();
  FoldTable *fold_table = FoldTable::get_fold_table(s);
  BranchStatement *the_branch = to<BranchStatement>(stmt);
  Expression *cond = 
    fold_table->fold_and_replace_expression(the_branch->get_decision_operand());
  IInteger src = get_expression_constant(cond);
  // If it is a constant, it does not need to be evaluated.

  if (!src.is_undetermined()) {
    Statement *new_statement = NULL;
    if (src == 0) {
      //trash_it(s, remove_statement(stmt));
      //return(NULL);
    } else {
      // replace with a jump statement
      new_statement = create_jump_statement(s, the_branch->get_target());
    }
    trash_it(s, replace_statement(the_branch, new_statement));
    return(new_statement);
  }
  
  // Otherwise, done
  return(the_branch);
}

static Statement *fold_and_replace_while_statement(Statement *stmt) {
  SuifEnv *s = stmt->get_suif_env();
  FoldTable *fold_table = FoldTable::get_fold_table(s);
  WhileStatement *the_while = to<WhileStatement>(stmt);
  Expression *cond = 
    fold_table->fold_and_replace_expression(the_while->get_condition());
  IInteger src = get_expression_constant(cond);
  // If it is a constant, it does not need to be evaluated.

  if (!src.is_undetermined()) {
    if (src == 0) {
      trash_it(s, remove_statement(stmt));
      return(NULL);
    }
  }
  
  // Otherwise, fold the body
  fold_table->fold_and_replace_statement(the_while->get_body());
  return(the_while);
}

static Statement *fold_and_replace_scope_statement(Statement *stmt) {
  SuifEnv *s = stmt->get_suif_env();
  FoldTable *fold_table = FoldTable::get_fold_table(s);
  ScopeStatement *the_scope = to<ScopeStatement>(stmt);
  Statement *body = fold_table->fold_and_replace_statement(the_scope->get_body());
  if (the_scope->get_symbol_table()->get_symbol_table_object_count() != 0)
    return(stmt);
  DefinitionBlock *def = the_scope->get_definition_block();
  if (def->get_variable_definition_count() != 0)
    return(stmt);

  if (def->get_procedure_definition_count() != 0)
    return(stmt);

  // replace
  remove_statement(body);
  trash_it(s, replace_statement(the_scope, body));
  return(body);
}

static Statement *fold_and_replace_statement_list(Statement *stmt) {
  SuifEnv *s = stmt->get_suif_env();
  FoldTable *fold_table = FoldTable::get_fold_table(s);
  StatementList *the_list = to<StatementList>(stmt);
  size_t count = the_list->get_statement_count();
  for (size_t num = count ; num != 0; num--) {
    Statement *child = the_list->get_statement(num-1);
    fold_table->fold_and_replace_statement(child);
  }
  if (the_list->get_statement_count() == 0) {
    trash_it(s, remove_statement(the_list));
    return(NULL);
  }
  if (the_list->get_statement_count() == 1) {
    Statement *new_st = the_list->remove_statement(0);
    trash_it(s, replace_statement(the_list, new_st));
    return(new_st);
  }
  return(the_list);
}

FoldTable *FoldTable::get_fold_table(SuifEnv *s) {
  static LString k_constant_folding_table = "constant_folding_table";
  ModuleSubSystem *ms = s->get_module_subsystem();
  Module *m = ms->retrieve_module(k_constant_folding_table);
  suif_assert_message(m != NULL,
		      ("No constant folding table registered"));
  return((FoldTable*)m);
}
  
FoldTable::FoldTable(SuifEnv *s, const LString &name) :
  Module(s, name),
  _binary_folds(new suif_hash_map<LString, binary_fold_fn>),
  _unary_folds(new suif_hash_map<LString, unary_fold_fn>),
  _expression_folds(new CascadingMap<expression_fold_fn>(s,
		    fold_and_replace_default_expression)),
  _statement_folds(new CascadingMap<statement_fold_fn>(s, 
		   fold_and_replace_default_statement))
{
  
}

FoldTable::~FoldTable() {
  delete _binary_folds;
  delete _unary_folds;
  delete _expression_folds;
  delete _statement_folds;
}

void FoldTable::initialize() {
  Module::initialize();
  initialize_suifnodes_folds();
}
void FoldTable::initialize_suifnodes_folds() {
  add_binary_fold_fn(k_add, binary_add_fn);
  add_binary_fold_fn(k_subtract, binary_subtract_fn);
  add_binary_fold_fn(k_multiply, binary_multiply_fn);
  add_binary_fold_fn(k_divide, binary_divide_fn);
  add_binary_fold_fn("divfloor", binary_divfloor_fn);
  add_binary_fold_fn("divceil", binary_divceil_fn);
  add_binary_fold_fn(k_remainder, binary_remainder_fn);
  add_binary_fold_fn(k_bitwise_and, binary_bitwise_and_fn);
  add_binary_fold_fn(k_bitwise_or, binary_bitwise_or_fn);
  add_binary_fold_fn(k_bitwise_nand, binary_bitwise_nand_fn);
  add_binary_fold_fn(k_bitwise_nor, binary_bitwise_nor_fn);
  add_binary_fold_fn(k_bitwise_xor, binary_bitwise_xor_fn);
    add_binary_fold_fn(k_left_shift, binary_left_shift_fn);
    add_binary_fold_fn(k_right_shift, binary_right_shift_fn);
    add_binary_fold_fn(k_rotate, binary_rotate_fn);
    add_binary_fold_fn(k_is_equal_to, binary_is_equal_to_fn);
    add_binary_fold_fn(k_is_not_equal_to, binary_is_not_equal_to_fn);
    add_binary_fold_fn(k_is_less_than, binary_is_less_than_fn);
    add_binary_fold_fn(k_is_less_than_or_equal_to, binary_is_less_than_or_equal_to_fn);
    add_binary_fold_fn(k_is_greater_than, binary_is_greater_than_fn);
    add_binary_fold_fn(k_is_greater_than_or_equal_to, binary_is_greater_than_or_equal_to_fn);
    add_binary_fold_fn(k_logical_and, binary_logical_and_fn);
    add_binary_fold_fn(k_logical_or, binary_logical_or_fn);
    add_binary_fold_fn(k_minimum, binary_minimum_fn);
    add_binary_fold_fn(k_maximum, binary_maximum_fn);
    
    add_unary_fold_fn(k_negate, unary_negate_fn);
    add_unary_fold_fn(k_invert, unary_invert_fn);
    add_unary_fold_fn(k_absolute_value, unary_absolute_value_fn);
    add_unary_fold_fn(k_bitwise_not, unary_bitwise_not_fn);
    add_unary_fold_fn(k_logical_not, unary_logical_not_fn);
    add_unary_fold_fn(k_convert, unary_copy_fn);
    add_unary_fold_fn(k_treat_as, unary_copy_fn);
    add_unary_fold_fn(k_copy, unary_copy_fn);

    // Now the expression folds
    add_expression_fold_fn(IntConstant::get_class_name(),
				       fold_and_replace_int_constant);
    add_expression_fold_fn(FloatConstant::get_class_name(),
				       fold_and_replace_float_constant);
    add_expression_fold_fn(UnaryExpression::get_class_name(),
				       fold_and_replace_unary_expression);
    add_expression_fold_fn(BinaryExpression::get_class_name(),
				       fold_and_replace_binary_expression);

    // Now the statement folds
    add_statement_fold_fn(EvalStatement::get_class_name(),
				      fold_and_replace_eval_statement);
    add_statement_fold_fn(StatementList::get_class_name(),
				      fold_and_replace_statement_list);
    add_statement_fold_fn(IfStatement::get_class_name(),
				      fold_and_replace_if_statement);
    add_statement_fold_fn(BranchStatement::get_class_name(),
				      fold_and_replace_branch_statement);
    add_statement_fold_fn(WhileStatement::get_class_name(),
				      fold_and_replace_while_statement);
    add_statement_fold_fn(ScopeStatement::get_class_name(),
				      fold_and_replace_scope_statement);
    add_statement_fold_fn(ForStatement::get_class_name(),
				      fold_and_replace_for_statement);
}

Module *FoldTable::clone() const {
  return((Module*)this);
}

void FoldTable::add_binary_fold_fn(const LString &opcode, binary_fold_fn fn)
{
  _binary_folds->enter_value(opcode, fn);
}
void FoldTable::add_unary_fold_fn(const LString &opcode, unary_fold_fn fn) {
  _unary_folds->enter_value(opcode, fn);
}

void FoldTable::add_expression_fold_fn(const LString &meta_class, 
				       expression_fold_fn fn) {
  _expression_folds->assign(meta_class, fn);
}
void FoldTable::add_statement_fold_fn(const LString &meta_class, 
				      statement_fold_fn fn) {
  _statement_folds->assign(meta_class, fn);
}

IInteger FoldTable::binary_fold(const LString &opcode,
				const IInteger &val1, const IInteger &val2,
				const IInteger &bit_size, 
				bool is_signed) const {
  binary_fold_fn fn = binary_default_fn;
  suif_hash_map<LString, binary_fold_fn>::iterator iter =
    _binary_folds->find(opcode);
  if (iter != _binary_folds->end()) {
    fn = (*iter).second;
  }
  return((*fn)(val1, val2, bit_size, is_signed));
}

IInteger FoldTable::unary_fold(const LString &opcode,
			       const IInteger &srcval,
			       const IInteger &bit_size,
			       bool is_signed) const {
  unary_fold_fn fn = unary_default_fn;
  suif_hash_map<LString, unary_fold_fn>::iterator iter =
    _unary_folds->find(opcode);
  if (iter != _unary_folds->end()) {
    fn = (*iter).second;
  }
  return((*fn)(srcval, bit_size, is_signed));
}

Expression *FoldTable::fold_and_replace_expression(Expression *expr) {
  if (expr == NULL) return(NULL);
  expression_fold_fn fn = _expression_folds->lookup(expr);
  return((*fn)(expr));
}
Statement *FoldTable::fold_and_replace_statement(Statement *stmt) {
  if (stmt == NULL) return(NULL);
  statement_fold_fn fn = _statement_folds->lookup(stmt);
  return((*fn)(stmt));
}

