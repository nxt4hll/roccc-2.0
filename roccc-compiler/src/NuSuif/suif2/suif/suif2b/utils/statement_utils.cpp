#include "statement_utils.h"
#include <common/lstring.h>
#include <basicnodes/basic.h>
#include <suifnodes/suif.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif_factory.h>
#include <typebuilder/type_builder.h>
#include "type_utils.h"
#include "cloning_utils.h"
#include "trash_utils.h"
#include "expression_utils.h"

static LString k_guarded = "guarded";

bool is_for_statement_guarded(ForStatement *the_for) {
  return(the_for->peek_annote(k_guarded) != NULL);
}

void set_for_statement_guarded(ForStatement *the_for) {
  if (is_for_statement_guarded(the_for)) return;
  the_for->append_annote(create_general_annote(the_for->get_suif_env(),
					       k_guarded));
}

void set_for_statement_unguarded(ForStatement *the_for) {
  while (is_for_statement_guarded(the_for)) {
    trash_it(the_for->take_annote(k_guarded));
  }
}

Statement *guard_for_statement(ForStatement *the_for) {
  if (is_for_statement_guarded(the_for)) return(the_for);
  SuifEnv *s = the_for->get_suif_env();

  IInteger ii = evaluate_for_statement_entry_test(the_for);
  if (ii == 1) {
    set_for_statement_guarded(the_for);
    return(the_for);
  }
  Statement *lb_assign =
    build_assign(the_for->get_index(),
		 deep_suif_clone<Expression>(the_for->get_lower_bound()));
  if (ii == 0) { 
    // will never be executed, replace with an assignment of the 
    // lower bounds
    trash_it(replace_statement(the_for, lb_assign));
    return(lb_assign);
  } 
  // undetermined create an if.
  Expression *x = build_for_statement_entry_test(the_for);
  IfStatement *the_if = create_if_statement(s, x, NULL, lb_assign);
  the_if->set_then_part(replace_statement(the_for, the_if));
  // We are now free to move the prepad
  Statement *prepad = the_for->get_pre_pad();
  if (prepad) {
    // remove it from the prepad
    the_for->set_pre_pad(NULL);
    // place it before the for statement
    insert_statement_before(the_for, prepad);
  }
  return(the_if);
}




Expression *build_for_statement_entry_test(ForStatement *the_for) {
  suif_assert(the_for != NULL);
  SuifEnv *s = the_for->get_suif_env();
  Expression *x = create_binary_expression(s,
	    get_type_builder(s)->get_boolean_type(),
	    the_for->get_comparison_opcode(),
	    deep_suif_clone<Expression>(the_for->get_lower_bound()),
	    deep_suif_clone<Expression>(the_for->get_upper_bound()));
  return(x);
}

IInteger evaluate_for_statement_entry_test(ForStatement *the_for) {
  Expression *x = build_for_statement_entry_test(the_for);
  x = fold_and_replace_constants(x);
  IInteger ii = get_expression_constant(x);
  delete x;
  return(ii);
}

