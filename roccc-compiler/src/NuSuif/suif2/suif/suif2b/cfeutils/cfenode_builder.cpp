#include "cfenode_builder.h"
#include "suifkernel/suifkernel_messages.h"
#include "suifkernel/suif_exception.h"
//#include "basicnodes/basic_factory.h"
//#include "suifnodes/suif_factory.h"
#include "cfenodes/cfe_factory.h"
//#include "typebuilder/type_builder.h"
//#include "utils/symbol_utils.h"
//#include "utils/type_utils.h"
//#include "utils/expression_utils.h"
#include "suifnodes/suif.h"
#include "utils/print_utils.h"
//#include "utils/trash_utils.h"


CfeNodeBuilder::CfeNodeBuilder(SymbolTable*symtab) :
  NodeBuilder(symtab)
{}
CfeNodeBuilder::CfeNodeBuilder(ScopedObject* obj) :
  NodeBuilder(obj)
{}


CallExpression* CfeNodeBuilder::call(Expression* callee_addr,
				  suif_vector<Expression*>* args)
{
  CProcedureType* ctype =
    to_ref_type<CProcedureType>(this, callee_addr->get_result_type());
  if (ctype == 0) {
    trash(callee_addr);
    for (unsigned i = 0; i<args->length(); i++) {
      trash(args->at(i));
    }
    SUIF_THROW(SuifException(String("Type error in CallExpression ") + 
			     to_id_string(callee_addr)));
  }
  DataType* rtype = to<CProcedureType>(ctype)->get_result_type();
  CallExpression* exp = create_call_expression(get_suif_env(), rtype, callee_addr);
  for (unsigned i = 0; i<args->length(); i++) {
    exp->append_argument(args->at(i));
  }
  return exp;
}


CallExpression* CfeNodeBuilder::call0(ProcedureSymbol* psym)
{
  suif_vector<Expression*> args;
  return call(sym_addr(psym), &args);
}

CallExpression* CfeNodeBuilder::call1(ProcedureSymbol* psym,
				   Expression* arg1)
{
  suif_vector<Expression*> args;
  args.push_back(arg1);
  return call(sym_addr(psym), &args);
}


CallExpression* CfeNodeBuilder::call2(ProcedureSymbol* psym,
			       Expression* arg1,
			       Expression* arg2)
{
  suif_vector<Expression*> args;
  args.push_back(arg1);
  args.push_back(arg2);
  return call(sym_addr(psym), &args);
}


CallExpression* CfeNodeBuilder::call3(ProcedureSymbol* psym,
				   Expression* arg1,
				   Expression* arg2,
				   Expression* arg3)
{
  suif_vector<Expression*> args;
  args.push_back(arg1);
  args.push_back(arg2);
  args.push_back(arg3);
  return call(sym_addr(psym), &args);
}


CallExpression* CfeNodeBuilder::call4(ProcedureSymbol* psym,
				   Expression* arg1,
				   Expression* arg2,
				   Expression* arg3,
				   Expression* arg4)
{
  suif_vector<Expression*> args;
  args.push_back(arg1);
  args.push_back(arg2);
  args.push_back(arg3);
  args.push_back(arg4);
  return call(sym_addr(psym), &args);
}

CallExpression* CfeNodeBuilder::call5(ProcedureSymbol* psym,
				   Expression* arg1,
				   Expression* arg2,
				   Expression* arg3,
				   Expression* arg4,
				   Expression* arg5)
{
  suif_vector<Expression*> args;
  args.push_back(arg1);
  args.push_back(arg2);
  args.push_back(arg3);
  args.push_back(arg4);
  args.push_back(arg5);
  return call(sym_addr(psym), &args);
}

