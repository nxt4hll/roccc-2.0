#include "rocccLibrary/FunctionType.h"
#include "llvm/Instructions.h"
#include "rocccLibrary/ROCCCNames.h"
#include "llvm/Constants.h"

using namespace llvm;

namespace ROCCC {

/*
Get the function type of the given function
*/
ROCCC::FunctionType ROCCCFunctionType( Function* F )
{
  assert( F && "ROCCCFunctionType() passed a NULL function pointer!" );
  for ( Function::iterator BB = F->begin(); BB != F->end(); ++BB)
  {
    for (BasicBlock::iterator II = BB->begin(); II != BB->end();++II)
    {
      if ( CallInst* CI = dynamic_cast<CallInst*>(&*II) )
      {
        if ( isROCCCFunctionCall(CI, ROCCCNames::FunctionType) )
        {
          if( ConstantInt* conInt = dynamic_cast<ConstantInt*>(&*(CI->getOperand(1))) )
          {
            if ( conInt->getValue() == APInt(32, BLOCK) )
              return BLOCK;
            else if ( conInt->getValue() == APInt(32, MODULE) )
              return MODULE;
            else if ( conInt->getValue() == APInt(32, SYSTEM) )
              return SYSTEM;
            else
              assert(0 and "Unknown function type!");
	        }
        }
      }
    }
  }
  return ERROR;
}

}
