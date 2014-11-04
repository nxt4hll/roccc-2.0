#include "rocccLibrary/SizeInBits.h"

#include "llvm/Constants.h"
#include "llvm/Function.h"
#include "llvm/Module.h"

#include <map>

#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/ROCCCNames.h"
#include "rocccLibrary/CopyValue.h"
#include "rocccLibrary/DFBasicBlock.h"

using namespace llvm;

int getSizeInBits(const Type* T)
{
  if ( T->getTypeID() == Type::PointerTyID or T->getTypeID() == Type::ArrayTyID )
  {
    const SequentialType* ST = dynamic_cast<const SequentialType*>( T );
    assert( ST and "Problem trying to figure out a type of a value." ); 
    return getSizeInBits( ST->getElementType() );
  }
  if( !T->isInteger() and !T->isFloatingPoint() )
  {
    INTERNAL_WARNING("Type #" << T->getTypeID() << "(" << T->getDescription() << ") is not integer or floating point, and has size of " << T->getPrimitiveSizeInBits() << "!\n");
  }
  return T->getPrimitiveSizeInBits();
}

bool shouldMaximizePrecision(Function* f)
{
  for(Function::iterator BB = f->begin(); BB != f->end(); ++BB)
  {
    for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
    {
      if(CallInst* CI = dynamic_cast<CallInst*>(&*II))
      {
        if( isROCCCFunctionCall(CI, ROCCCNames::MaximizePrecision) )
        {
          assert( CI->getNumOperands() == 2 );
          ConstantInt* constInt = dynamic_cast<ConstantInt*>( CI->getOperand(1));
          assert( constInt and "Preference Maximize Precision must be a constant int!" );
          return (constInt->getValue().getSExtValue() == 1 or constInt == ConstantInt::getTrue());
        }
      }
    }
  }
  //default to . . . false!
  return false;
  //assert(0 and "Could not find preference for Maximize Precision!");
}

int getSizeOfBinaryOperation(BinaryOperator* i)
{
  int lhs_size = 0;
  int rhs_size = 0;
  if( !dynamic_cast<Constant*>(i->getOperand(0)) )
    lhs_size = getSizeInBits(i->getOperand(0));
  if( !dynamic_cast<Constant*>(i->getOperand(1)) )
    rhs_size = getSizeInBits(i->getOperand(1));
  int max_size = (lhs_size > rhs_size)?lhs_size:rhs_size;
  bool shouldMaximize = shouldMaximizePrecision(i->getParent()->getParent()) and !i->getType()->isFloatingPoint();
  if( isCopyValue(i) )
    return max_size;
  switch( i->getOpcode() )
  {
    case BinaryOperator::Add:
    {
      if( shouldMaximize )
        return max_size + 1;
      else
        return max_size;
    }
    break ;
    case BinaryOperator::Sub:
    {
      return max_size;
    }
    break ;
    case BinaryOperator::Mul:
    {
  	    if( shouldMaximize )
          return lhs_size + rhs_size;
        else
          return max_size;
    }
    break;
    case BinaryOperator::SRem:
    case BinaryOperator::UDiv:
    case BinaryOperator::SDiv:
    case BinaryOperator::FDiv:
    {
      return max_size;
    }
    break;
    case BinaryOperator::Shl: //shift left
    {
      return lhs_size;
    }
    break ;
    case BinaryOperator::LShr: // Shift right logical
    {
      return lhs_size;
    }
    break;
    case BinaryOperator::AShr: // Shift right arithmetic
    {
      return lhs_size;
    }
    break ;
    case BinaryOperator::And:
    {
      return max_size;
    }
    break ;
    case BinaryOperator::Or:
    {
      return max_size;
    }
    break ;
    case BinaryOperator::Xor:
    {
      return max_size;
    }
    break ;
    default:
    {
      INTERNAL_ERROR("Unknown binary operation " << *i);
      assert( 0 and "Unknown binary operation!" );
    }
  }
}

std::map<Value*, int> sizeMap;

void resetSizeMap()
{
  sizeMap.clear();
}

int getSizeInBits(Value* V)
{
  assert( V );
  //if this is the first time through the map, fill the map with the values we
  //  find from the ROCCCSize calls
  if( sizeMap.empty() )
  {
    if(Instruction* II = dynamic_cast<Instruction*>(V))
    {
      Function* f = II->getParent()->getParent();
      for(Function::iterator BB = f->begin(); BB != f->end(); ++BB)
      {
        for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
        {
          if(CallInst* CI = dynamic_cast<CallInst*>(&*II))
          {
            if( isROCCCFunctionCall(CI, ROCCCNames::VariableSize) )
            {
              assert( CI->getNumOperands() == 3 );
              ConstantInt* constInt = dynamic_cast<ConstantInt*>( CI->getOperand(1));
              assert( constInt and "ROCCC variable size must be a constant int!" );
              sizeMap[CI->getOperand(2)] = constInt->getValue().getSExtValue();
            }
            //now that we are doing conversions, also fill the map with the 
            //  result of the convert
            if( isROCCCFunctionCall(CI, ROCCCNames::ConvertFloatToFloat) or 
                isROCCCFunctionCall(CI, ROCCCNames::ConvertFloatToInt) or
                isROCCCFunctionCall(CI, ROCCCNames::ConvertIntToFloat) or
                isROCCCFunctionCall(CI, ROCCCNames::ConvertIntToInt) )
            {
              assert( CI->getNumOperands() == 3 );
              ConstantInt* constInt = dynamic_cast<ConstantInt*>( CI->getOperand(2));
              assert( constInt and "Conversion variable size must be a constant int!" );
              sizeMap[CI] = constInt->getValue().getSExtValue();
            }
          }
        }
      }
    }
  }
  if( sizeMap.find(V) != sizeMap.end() )
  {
    return sizeMap.find(V)->second;
  }
  if(UnaryInstruction* UI = dynamic_cast<UnaryInstruction*>(V))
  {
    if( !dynamic_cast<Constant*>(UI->getOperand(0)) )
      return ( sizeMap[V] = getSizeInBits(UI->getOperand(0)) );
  }
  if(BinaryOperator* BI = dynamic_cast<BinaryOperator*>(V))
  {
    return ( sizeMap[V] = getSizeOfBinaryOperation(BI) );
  }
  INTERNAL_WARNING("Forced to use size of type " << *V->getType() << " to find size of value " << *V << "!\n");
  return ( sizeMap[V] = getSizeInBits(V->getType()) );
}

void setSizeInBits(Value* val, int size)
{
  Instruction* II = dynamic_cast<Instruction*>(val);
  assert(II and "Cannot set size in bits on a non-instruction!");
  assert(II->getParent());
  assert(II->getParent()->getParent());
  assert(II->getParent()->getParent()->getParent());
  Function* sizeFunc = II->getParent()->getParent()->getParent()->getFunction(ROCCCNames::VariableSize);
  assert(sizeFunc);
  //create the insertion point
  DFBasicBlock* dfbb = new DFBasicBlock("", II->getParent()->getParent());
	//now create the actual call instruction
  std::vector<Value*> valArgs;
  valArgs.push_back(llvm::ConstantInt::get(llvm::IntegerType::get(32), size));
  valArgs.push_back(val);
	CallInst::Create( sizeFunc,
          			    valArgs.begin(),
          			    valArgs.end(),
          			    "" ,
          			    dfbb->getTerminator());
  resetSizeMap();
}
