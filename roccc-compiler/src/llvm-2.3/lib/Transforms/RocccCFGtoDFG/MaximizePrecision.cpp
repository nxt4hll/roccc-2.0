// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

  DESC

 */

#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Constants.h"

#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/ROCCCNames.h"

namespace llvm
{
  class MaximizePrecisionPass : public FunctionPass
  {
  private:
  public:
    static char ID ;
    MaximizePrecisionPass() ;
    ~MaximizePrecisionPass() ;
    virtual bool runOnFunction(Function& b) ;
  } ;
}

using namespace llvm ;

char MaximizePrecisionPass::ID = 0 ;

static RegisterPass<MaximizePrecisionPass> X ("maximizePrecision", 
					"Sets the precision of arithmetic operations to maximum (default, maintain precision).");

MaximizePrecisionPass::MaximizePrecisionPass() : FunctionPass((intptr_t)&ID) 
{
  ; // Nothing in here
}

MaximizePrecisionPass::~MaximizePrecisionPass()
{
  ; // Nothing to delete either
}


bool MaximizePrecisionPass::runOnFunction(Function& f)
{
  CurrentFile::set(__FILE__);
  bool changed = false ;
  
  bool alreadyExists = false;
  for(Function::iterator BB = f.begin(); BB != f.end(); ++BB)
  {
    for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
    {
      CallInst* CI = dynamic_cast<CallInst*>(&*II);
      if( isROCCCFunctionCall(CI, ROCCCNames::MaximizePrecision) )
      {
        alreadyExists = true;
        break;
      }
    }
  }
  if( !alreadyExists )
  {
    //it doesnt already exist, so set it!
    //create the operand
    Value* val = ConstantInt::getTrue();
    //create a function that takes the arguments
    std::vector<const Type*> paramTypes;
    paramTypes.push_back( val->getType() );
    FunctionType* ft = FunctionType::get(Type::VoidTy, paramTypes, false);
    Function* rocccInvokeHw = NULL;
    rocccInvokeHw = Function::Create(ft,
		  		   (GlobalValue::LinkageTypes)0,
			  	   ROCCCNames::MaximizePrecision, 
				     f.getParent() );
		//now create the actual call instruction
    assert(f.begin() != f.end());
    BasicBlock* BB = &*f.begin();
    assert(BB);
    assert(BB->begin());
    std::vector<Value*> valArgs;
    valArgs.push_back(val);
		CallInst::Create( rocccInvokeHw,
				    valArgs.begin(),
				    valArgs.end(),
				    "" ,
				    BB->begin());
  }
  return changed ;
}

