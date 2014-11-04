//this pass will add in the necessary dependencies between LUT calls
//  in order to schedule them in the datapath correctly.

#include "llvm/Instructions.h"
#include "rocccLibrary/ROCCCNames.h"
#include <cassert>
#include "llvm/Pass.h"
#include "llvm/Function.h"
#include <map>

namespace llvm
{

class LUTDependencyPass : public FunctionPass {
private:
public:
  static char ID ;
  LUTDependencyPass() ;
  ~LUTDependencyPass() ;
  virtual bool runOnFunction(Function& b) ;
  virtual void getAnalysisUsage(AnalysisUsage &AU) const;
} ;

}

using namespace llvm ;

char LUTDependencyPass::ID = 0 ;

static RegisterPass<LUTDependencyPass> X ("lutDependency", 
					"Explicitly create implied dependencies between LUT operations.");

LUTDependencyPass::LUTDependencyPass() : FunctionPass((intptr_t)&ID) 
{
  ; // Nothing in here
}

LUTDependencyPass::~LUTDependencyPass()
{
  ; // Nothing to delete either
}

void LUTDependencyPass::getAnalysisUsage(AnalysisUsage &AU) const
{
  AU.setPreservesAll();
}

llvm::CallInst* getLUTDeclarationCall(llvm::Value* array)
{
  for(llvm::Value::use_iterator UI = array->use_begin(); UI != array->use_end(); ++UI)
  {
    llvm::CallInst* CI = dynamic_cast<llvm::CallInst*>(*UI);
    if( isROCCCFunctionCall(CI, ROCCCNames::InternalLUTDeclaration) )
      return CI;
  }
  assert(0 and "Could not find declaration call!");
}

llvm::CallInst* getLUTOrderCall(llvm::Value* array)
{
  for(llvm::Value::use_iterator UI = array->use_begin(); UI != array->use_end(); ++UI)
  {
    llvm::CallInst* CI = dynamic_cast<llvm::CallInst*>(*UI);
    if( isROCCCFunctionCall(CI, ROCCCNames::LUTOrder) )
      return CI;
  }
  assert(0 and "Could not find order call!");
}

int getOperandPosition(llvm::User* u, llvm::Value* val)
{
  for(unsigned n = 0; n < u->getNumOperands(); ++n)
  {
    if( u->getOperand(n) == val )
      return n;
  }
  assert(0 and "Could not find operand!");
}

//TODO: allow a certain number of simultaneous requests per clock cycle
bool LUTDependencyPass::runOnFunction(Function& f)
{
  //maps LUT arrays to the last used operation on that array.
  std::map<Value*,Value*> lastLUTAccess;
  //maps LUT arrays to the first used operation on that array.
  std::map<Value*,Value*> firstLUTAccess;
  for(Function::iterator BB = f.begin(); BB != f.end(); ++BB)
  {
    for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
    {
      CallInst* CI = dynamic_cast<CallInst*>(&*II);
      if( isROCCCFunctionCall(CI, ROCCCNames::LUTRead) or
          isROCCCFunctionCall(CI, ROCCCNames::LUTWrite) )
      {
        assert(CI->getNumOperands() >= 2);
        Value* array = CI->getOperand(1);
        assert(array);
        if( firstLUTAccess.find(array) == firstLUTAccess.find(array) )
        {
          firstLUTAccess[array] = CI;
          CI->setOperand(CI->getNumOperands()-1, getLUTDeclarationCall(array));
        }
        if( lastLUTAccess.find(array) != lastLUTAccess.end() )
        {
          CI->setOperand(CI->getNumOperands()-1, lastLUTAccess[array]);
        }
        lastLUTAccess[array] = CI;
      }
    }
  }
  for(std::map<Value*,Value*>::iterator LLI = lastLUTAccess.begin(); LLI != lastLUTAccess.end(); ++LLI)
  {
    llvm::CallInst* CI = getLUTOrderCall(LLI->first);
    int orderPosition = getOperandPosition(CI, LLI->first);
    CI->setOperand(orderPosition+1, LLI->second);
  }
  return false;
}

