
#include <cassert>
#include <string>
#include "functionUtilities.h"

bool RedundantVoter(CallStatement* c)
{
  if (c == NULL)
  {
    return false ;
  }
  SymbolAddressExpression* procSymExpr = 
    dynamic_cast<SymbolAddressExpression*>(c->get_callee_address()) ;
  assert(procSymExpr != NULL) ;
  ProcedureSymbol* procSym = 
    dynamic_cast<ProcedureSymbol*>(procSymExpr->get_addressed_symbol()) ;
  assert(procSym != NULL) ;
  std::string tmpName = procSym->get_name().c_str() ;
  if (tmpName.find("ROCCCTripleVote") != std::string::npos ||
      tmpName.find("ROCCCDoubleVote") != std::string::npos)
  {
    return true ;
  }
  return false ;
}

bool IsBoolSel(CallStatement* c)
{
  if (c == NULL)
  {
    return false ;
  }
  SymbolAddressExpression* procSymExpr = 
    dynamic_cast<SymbolAddressExpression*>(c->get_callee_address()) ;
  assert(procSymExpr != NULL) ;
  ProcedureSymbol* procSym = 
    dynamic_cast<ProcedureSymbol*>(procSymExpr->get_addressed_symbol()) ;
  assert(procSym != NULL) ;
  std::string tmpName = procSym->get_name().c_str() ;
  if (tmpName.find("ROCCC_boolsel") != std::string::npos)
  {
    return true ;
  }
  return false ;
}

bool IsBuiltIn(CallStatement* c)
{
  if (c == NULL)
  {
    return false ;
  }

  SymbolAddressExpression* procSymExpr = 
    dynamic_cast<SymbolAddressExpression*>(c->get_callee_address()) ;
  assert(procSymExpr != NULL) ;
  ProcedureSymbol* procSym = 
    dynamic_cast<ProcedureSymbol*>(procSymExpr->get_addressed_symbol()) ;
  assert(procSym != NULL) ;
  std::string tmpName = procSym->get_name().c_str() ;
  if (tmpName.find("ROCCC_boolsel")       != std::string::npos ||
      tmpName.find("ROCCCSummation")      != std::string::npos ||
      tmpName.find("ROCCCIntToInt")       != std::string::npos ||
      tmpName.find("ROCCCFloatToInt")     != std::string::npos ||
      tmpName.find("ROCCCIntToFloat")     != std::string::npos ||
      tmpName.find("ROCCCFloatToFloat")   != std::string::npos ||
      tmpName.find("ROCCCLUTLookup")      != std::string::npos ||
      tmpName.find("ROCCCLUTStore")       != std::string::npos ||
      tmpName.find("ROCCCLUTDeclaration") != std::string::npos) 
  {
    return true ;
  }
  return false ;
}
