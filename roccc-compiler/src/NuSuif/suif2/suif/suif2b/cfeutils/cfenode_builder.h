#ifndef _CFE_NODE_BUILDER_H_
#define _CFE_NODE_BUILDER_H_
/**
  * The NodeBuilder class is a utility class for building IR nodes.
  *
  * The constructor takes in a SymbolTable.  This symbol table represents the
  * environment of the new IR nodes.
  * New SymbolTableObjects will be automatically owned by this symbol table.
  * 
  * All lookup_ method will return a null if no matching object is found,
  * unless otherwise stated.
  */

#include "common/lstring.h"
#include "basicnodes/basic.h"
#include "suifnodes/suif.h"
#include "utils/node_builder.h"
#include "cfenodes/cfe.h"


class CfeNodeBuilder : public NodeBuilder {
 public:
  CfeNodeBuilder(SymbolTable*);
  CfeNodeBuilder(ScopedObject*);

  CallExpression* call(Expression* callee_addr,
		       suif_vector<Expression*>* args);
  CallExpression* call0(ProcedureSymbol*);
  CallExpression* call1(ProcedureSymbol*,
			Expression* arg1);
  CallExpression* call2(ProcedureSymbol*,
			Expression* arg1,
			Expression* arg2);
  CallExpression* call3(ProcedureSymbol*,
			Expression* arg1,
			Expression* arg2,
			Expression* arg3);
  CallExpression* call4(ProcedureSymbol*,
			Expression* arg1,
			Expression* arg2,
			Expression* arg3,
			Expression* arg4);
  CallExpression* call5(ProcedureSymbol*,
			Expression*, Expression*, Expression*, Expression*,
			Expression*);
}; // _CFE_NODE_BUILDER_H_

#endif // _CFE_NODE_BUILDER_H_
