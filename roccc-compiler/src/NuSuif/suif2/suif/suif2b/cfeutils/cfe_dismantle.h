#ifndef CFE_DISMANTLE
#define CFE_DISMANTLE

#include "cfenodes/cfe_forwarders.h"
#include "suifnodes/suif_forwarders.h"
/**
 * convert a call expression
 *
 * force_dest_not_expr should be called on the CallExpression
 * before calling this.
 */
CallStatement *dismantle_a_call_expression(CallExpression *cal);

#endif /* CFE_DISMANTLE */
