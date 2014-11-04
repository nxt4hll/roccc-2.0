#include "llvm/Constants.h"
#include "llvm/Instructions.h"
#include "llvm/InstrTypes.h"

#include "llvm/Support/CFG.h"
#include "llvm/Constants.h"

#include "rocccLibrary/CopyValue.h"

llvm::Value* getCopyOfValue(llvm::Value* copyVal)
{
  llvm::Value* ret = NULL;
  if (copyVal->getType()->isFloatingPoint())
  {
    ret = llvm::BinaryOperator::create(llvm::BinaryOperator::Add,
				copyVal,
				llvm::ConstantFP::getNegativeZero(copyVal->getType()),
				copyVal->getName()) ;
  }
  else if(copyVal->getType()->getTypeID() == llvm::Type::IntegerTyID)
  {
    llvm::ConstantInt* allOnes ;
    allOnes = llvm::ConstantInt::getAllOnesValue(copyVal->getType()) ;
    assert(allOnes != NULL) ;
    ret = llvm::BinaryOperator::create(llvm::BinaryOperator::And,
				copyVal,
				allOnes,
				copyVal->getName()) ;
  }
  return ret;
}

bool isCopyValue(llvm::Value* inst)
{
  llvm::Instruction* i = dynamic_cast<llvm::Instruction*>(inst);
  if( !i )
    return false;
  if( llvm::BinaryOperator* BO = dynamic_cast<llvm::BinaryOperator*>(i) )
  {
    if ( BO->getOpcode() == llvm::BinaryOperator::Add )
    {
      // Special case of copying floats
  	  if (dynamic_cast<llvm::ConstantFP*>(i->getOperand(1)) != NULL &&
  		  dynamic_cast<llvm::ConstantFP*>(i->getOperand(1)) == llvm::ConstantFP::getNegativeZero(i->getOperand(1)->getType()))
  	  {
  	    return true;
  	  }
    }
    if( BO->getOpcode() == llvm::BinaryOperator::And )
    {
      // Special case of copies.  They are created as ands with all ones values.
      if (dynamic_cast<llvm::ConstantInt*>(i->getOperand(1)) != NULL &&
          dynamic_cast<llvm::ConstantInt*>(i->getOperand(1))->isAllOnesValue())
      {
    	return true;
      }
    }
  }
  return false;
}
