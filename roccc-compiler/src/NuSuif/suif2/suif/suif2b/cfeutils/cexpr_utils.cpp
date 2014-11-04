#include "cexpr_utils.h"
#include "cfe_dismantle.h"
#include "cfenodes/cfe.h"
#include "utils/expression_utils.h"

ProcedureSymbol *get_procedure_target_from_call_expression(CallExpression *st) {
  Expression *procedure_address = st->get_callee_address();
  return(get_procedure_from_address_op(procedure_address));
}


void inline_call_expression(CallExpression *the_call,
			    ProcedureSymbol *target_proc) {
  // first dismantle it
  list<CallExpression *> call_list;
  force_call_dest_not_expr(the_call, &call_list);
  for (list<CallExpression*>::iterator siter =
	 call_list.begin(); siter != call_list.end(); siter++) {
    CallExpression *cal_expr = *siter;
    CallStatement *st = dismantle_a_call_expression(cal_expr);
    inline_call(st, target_proc);
  }
}

void force_call_dest_not_expr(CallExpression *the_call,
			      list<CallExpression*> *call_list)
{
  call_list->clear_list();
  list<StoreVariableStatement *> *store_list = force_dest_not_expr(the_call);
  if (!store_list) {
    call_list->push_back(the_call);
  } else {
    for (list<StoreVariableStatement*>::iterator siter =
	   store_list->begin(); siter != store_list->end(); siter++) {
      StoreVariableStatement *store = *siter;
      CallExpression *cal_expr = to<CallExpression>(store->get_value());
      call_list->push_back(cal_expr);
    }
  }
  delete store_list;
}

