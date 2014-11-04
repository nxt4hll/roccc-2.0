#include "common/system_specific.h"
#include "cloning_utils.h"
#include "suifcloning/cloner.h"
/*
SourceOp clone_source_op(const SourceOp &op, SuifEnv *s) {
  if (op.is_expression()) {
    Expression *e = 
      deep_suif_clone<Expression>(op.get_expression());
    e->set_parent(NULL);
    return(SourceOp(e));
  }
  return(op);
}
*/

Statement *clone_statement(SuifEnv *env,SymbolTable *table,const Statement *stat) {
     CloneRemote clone(env,table);
    clone.set_deep_clone();
    return  to<Statement>(clone.clone(stat));
    }
