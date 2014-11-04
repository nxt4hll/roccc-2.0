#ifndef _CFE_UTILS__CEXPR_UTILS_H
#define _CFE_UTILS__CEXPR_UTILS_H

#include "cfenodes/cfe_forwarders.h"
#include "basicnodes/basic_forwarders.h"
#include "common/common_forwarders.h"

extern ProcedureSymbol *get_procedure_target_from_call_expression(CallExpression *call);

/**
 * Inline a call expression
 */

void inline_call_expression(CallExpression *the_call,
			    ProcedureSymbol *target_proc);

/**
 * force the call expression destination to be a
 * StoreVariableStatement or EvalStatement
 * and return the list of CallExpressions to process.
 */
void force_call_dest_not_expr(CallExpression *the_call,
			      list<CallExpression*> *call_list);

#endif
