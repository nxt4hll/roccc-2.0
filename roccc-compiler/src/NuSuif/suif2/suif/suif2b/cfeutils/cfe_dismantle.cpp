#include "common/system_specific.h"
#include "cfe_dismantle.h"
#include "utils/expression_utils.h"
#include "cfenodes/cfe.h"
#include "suifnodes/suif_factory.h"
#include "utils/trash_utils.h"

CallStatement *dismantle_a_call_expression(CallExpression *cal) {

  ExecutionObject *cal_dest = cal->get_destination();
  // Either a Store Variable or an Eval
  if (!is_kind_of<EvalStatement>(cal_dest)
      && !is_kind_of<StoreVariableStatement>(cal_dest))
    {
      suif_information(cal_dest, 1, "dismantle_a_call_expression: expected an Eval or StoreVariable destination. Found '%s'\n",
			cal_dest->getClassName().c_str());
      return(NULL);
    }
  
  
  //force_dest_not_expr(cal);

  CallStatement *call_st = create_call_statement(cal->get_suif_env(),
						 NULL, NULL);

  // Now move its elements.
  // Move the call address
  Expression *addr = cal->get_callee_address();
  remove_suif_object(addr);
  call_st->set_callee_address(addr);
  
  // Move the arguments
  while (cal->get_argument_count() != 0) {
    Expression *arg = cal->get_argument(0);
    cal->remove_argument(0);
    call_st->append_argument(arg);
  }
  // Move the annotes
  list<Annote*> an_list;

  for (Iter<Annote*> aiter = cal->get_annote_iterator();
       aiter.is_valid(); aiter.next()) {
    Annote *an = aiter.current();
    an_list.push_back(an);
	   }
  for (list<Annote*>::iterator iter = an_list.begin();
       iter != an_list.end(); iter++) {
    Annote *an = *iter;
    cal->remove_annote(an);
    call_st->append_annote(an);
  }
  
  VariableSymbol *dst = NULL;
  
  ExecutionObject *eo = cal->get_destination();
  // Either a Store Variable or an Eval
  if (is_kind_of<EvalStatement>(eo)) {
    //    EvalStatement *eval = to<EvalStatement>(eo);
    dst = NULL;
  } else {
    suif_assert_message(is_kind_of<StoreVariableStatement>(eo),
			("expected an Eval or StoreVariable. Found '%s'",
			 eo->getClassName().c_str()));
    dst = to<StoreVariableStatement>(eo)->get_destination();
  }
  Statement *old_st = to<Statement>(eo);  
  //assign the destination
  call_st->set_destination(dst);
  replace_statement(old_st, call_st);
  trash_it(old_st->get_suif_env(), old_st);
  
  // Done!!
  return(call_st);
}

class SuifEnv;

extern "C" void init_cfeutils(SuifEnv *s) {
#if 0
  // not implemented in v2.1
  s->require_module("utils");
#endif
}

