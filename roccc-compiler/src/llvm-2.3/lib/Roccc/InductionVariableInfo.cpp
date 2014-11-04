#include "rocccLibrary/InductionVariableInfo.h"

#include "llvm/Constants.h"
#include "llvm/Function.h"

#include <map>

#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/ROCCCNames.h"

using namespace llvm;

int getIVStepSize(Value* v)
{
  assert( v );
  static std::map<Value*, int> sizeMap;
  //if this is the first time through the map, fill the map with the values we
  //  find from the ROCCCSize calls
  if( sizeMap.empty() )
  {
    if(Instruction* II = dynamic_cast<Instruction*>(v))
    {
      Function* f = II->getParent()->getParent();
      for(Function::iterator BB = f->begin(); BB != f->end(); ++BB)
      {
        for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
        {
          if(CallInst* CI = dynamic_cast<CallInst*>(&*II))
          {
            if( isROCCCFunctionCall(CI, ROCCCNames::InductionVariableStepSize) )
            {
              assert( CI->getNumOperands() == 3 );
              ConstantInt* constInt = dynamic_cast<ConstantInt*>( CI->getOperand(2));
              assert( constInt and "Induction variable step size must be a constant int!" );
              sizeMap[CI->getOperand(1)] = constInt->getValue().getSExtValue();
            }
          }
        }
      }
    }
  }
  if( sizeMap.find(v) == sizeMap.end() )
  {
    INTERNAL_WARNING("No " << ROCCCNames::InductionVariableStepSize << " found for induction variable " << v->getName() << "! Using default (1) step size!\n");
    sizeMap[v] = 1;
  }
  return sizeMap.find(v)->second;
}

int getIVStartValue(llvm::Value* v)
{
  assert( v );
  static std::map<llvm::Value*, int> startMap;
  //if this is the first time through the map, fill the map with the values we
  //  find from the ROCCCSize calls
  if( startMap.empty() )
  {
    if(Instruction* II = dynamic_cast<Instruction*>(v))
    {
      Function* f = II->getParent()->getParent();
      for(Function::iterator BB = f->begin(); BB != f->end(); ++BB)
      {
        for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
        {
          if(CallInst* CI = dynamic_cast<CallInst*>(&*II))
          {
            if( isROCCCFunctionCall(CI, ROCCCNames::InductionVariableStartValue) )
            {
              assert( CI->getNumOperands() == 3 );
              llvm::ConstantInt* constInt = dynamic_cast<llvm::ConstantInt*>( CI->getOperand(2));
              assert( constInt and "Induction variable start value must be a constant int!" );
              startMap[CI->getOperand(1)] = constInt->getValue().getSExtValue();
            }
          }
        }
      }
    }
  }
  if( startMap.find(v) == startMap.end() )
  {
    INTERNAL_WARNING("No " << ROCCCNames::InductionVariableStartValue << " found for induction variable " << v->getName() << "! Using default (0) start value!\n");
    startMap[v] = 0;
  }
  return startMap.find(v)->second;
}

int getIVEndValue(llvm::Value* v)
{
  assert(0 and "getEndValue() not implemented!");
  return 0;
}
