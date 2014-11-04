#include "rocccLibrary/IsValueSigned.h"

#include "llvm/Constants.h"
#include "llvm/Function.h"
#include "llvm/Module.h"

#include <map>

#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/ROCCCNames.h"
#include "rocccLibrary/CopyValue.h"
#include "rocccLibrary/DFBasicBlock.h"

using namespace llvm;


std::map<Value*, bool> signMap;

ConstantInt* getSignedConstantFlag(Value* v)
{
  for(Value::use_iterator UI = v->use_begin(); UI != v->use_end(); ++UI)
  {
    if(CallInst* CI = dynamic_cast<CallInst*>(*UI))
    {
      if( isROCCCFunctionCall(CI, ROCCCNames::VariableSigned) )
      {
        assert( CI->getNumOperands() == 3 );
        ConstantInt* tmpCast = dynamic_cast<ConstantInt*>(CI->getOperand(1));
        assert( tmpCast and "ROCCC variable signed must be a constant int!");
        return tmpCast;
      }
    }
  }
  return NULL;
}

bool isValueSigned(Value* v)
{
  if( !v )
    return false;
  //if this is the first time through the map, fill the map with the values we
  //  find from the ROCCCSigned calls
  if( signMap.empty() )
  {
    if(Instruction* II = dynamic_cast<Instruction*>(v))
    {
      if( !II->getParent() )
      {
        INTERNAL_ERROR(*II << " has no parent!\n");
        assert(II->getParent());
      }
      Function* f = II->getParent()->getParent();
      assert(f);
      for(Function::iterator BB = f->begin(); BB != f->end(); ++BB)
      {
        for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
        {
          if(CallInst* CI = dynamic_cast<CallInst*>(&*II))
          {
            if( isROCCCFunctionCall(CI, ROCCCNames::VariableSigned) )
            {
              assert( CI->getNumOperands() == 3 );
              ConstantInt* tmpCast = dynamic_cast<ConstantInt*>(CI->getOperand(1));
              assert( tmpCast and "ROCCC variable signed must be a constant int!");
              signMap[CI->getOperand(2)] = (tmpCast->getSExtValue() == 1);
            }
          }
        }
      }
    }
  }
  if( signMap.find(v) == signMap.end() )
  {
    //maybe we are a copy . . . check!
    if( isCopyValue(v) )
    {
      signMap[v] = isValueSigned(dynamic_cast<Instruction*>(v)->getOperand(0));
    }
    else
    {
      // This is probably a temporary constructed in the middle of any
      //  large expression
      llvm::UnaryInstruction* ui = dynamic_cast<llvm::UnaryInstruction*>(v) ;
      llvm::BinaryOperator*   bi = dynamic_cast<llvm::BinaryOperator*  >(v) ;
      if (ui != NULL)
      {
	Value* v1 = ui->getOperand(0) ;
	return isValueSigned(v1) ;
      }
      else if (bi != NULL)
      {
	Value* v1 = bi->getOperand(0) ;
	Value* v2 = bi->getOperand(1) ;
	return isValueSigned(v1) || isValueSigned(v2) ;
      }
      else
      {
	INTERNAL_WARNING("No ROCCCSigned found for variable " << *v << "! Using default (unsigned)!\n");
	signMap[v] = false;
      }
    }
  }
  return signMap.find(v)->second;
}

bool isValueUnsigned(Value* v)
{
  return !isValueSigned(v);
}

void setValueSigned(Value* val, bool sign)
{
  Instruction* II = dynamic_cast<Instruction*>(val);
  assert(II and "Cannot set signed on a non-instruction!");
  assert(II->getParent());
  assert(II->getParent()->getParent());
  assert(II->getParent()->getParent()->getParent());
  Function* signFunc = II->getParent()->getParent()->getParent()->getFunction(ROCCCNames::VariableSigned);
  assert(signFunc);
  //create the insertion point
  DFBasicBlock* dfbb = new DFBasicBlock("", II->getParent()->getParent());
	//now create the actual call instruction
  std::vector<Value*> valArgs;
  if( sign )
    valArgs.push_back(llvm::ConstantInt::get(llvm::IntegerType::get(32), 1));
  else
    valArgs.push_back(llvm::ConstantInt::get(llvm::IntegerType::get(32), 0));
  valArgs.push_back(val);
	CallInst::Create( signFunc,
          			    valArgs.begin(),
          			    valArgs.end(),
          			    "" ,
          			    dfbb->getTerminator());
  signMap.clear();
}
