#include "rocccLibrary/DefinitionInst.h"

#include "llvm/Constants.h"
#include "llvm/Instructions.h"
#include "llvm/Support/CFG.h"

#include <map>

#include "rocccLibrary/ROCCCNames.h"
#include "rocccLibrary/DatabaseInterface.h"


using namespace llvm;
using namespace Database;

// If the predecessor instruction (passed in as i) defines the value
//  (passed in as v), then return true.  Otherwise return false.
bool isDefinition(Instruction* i, Value* v)
{
  assert(i);
  assert(v);
  //nothing is the definition of a constant, so return false
  if (dynamic_cast<Constant*>(v) != NULL)
  {
    return false ;
  }
  // If the instruction and value are the same, then by definition this
  //  is a destination.
  if (i == v)
  {
    return true ;
  }
  // We have to worry about two special cases, when we define input scalars
  //  and define values coming out of hardware blocks.
  CallInst* callCast = dynamic_cast<CallInst*>(i) ;
  if (callCast == NULL)
  {
    return false ;
  }
  if ( isROCCCFunctionCall(callCast, ROCCCNames::InputScalar) )
  {
    for(unsigned int i = 0; i < callCast->getNumOperands(); ++i)
    {
      if (v == callCast->getOperand(i))
	      return true ;
    }
  }
  else if ( isROCCCInputStream(callCast) )
  {
    for(unsigned int i = 0; i < callCast->getNumOperands(); ++i)
    {
      if (v == callCast->getOperand(i) and i % (getROCCCStreamDimension(callCast)+1) == 1)
	      return true ;
    }
  }
  else if ( isROCCCFunctionCall(callCast, ROCCCNames::InvokeHardware) )
  {
    DatabaseInterface* tmpInterface = DatabaseInterface::getInstance() ;
    LibraryEntry lookup = tmpInterface->LookupEntry(getComponentNameFromCallInst(callCast)) ;
    // Skip the name of the function and the type of the function
    for(unsigned int i = 2; i < callCast->getNumOperands(); ++i)
    {
      if (v == callCast->getOperand(i) && lookup.IsOutputPort(i-2))
      {
	      return true ;
      }
    }
  }
  else if ( isROCCCFunctionCall(callCast, ROCCCNames::LoadPrevious) )
  {
    return /*false;//*/(v == callCast->getOperand(1));
  }
  else if ( isROCCCFunctionCall(callCast, ROCCCNames::SystolicPrevious) )
  {
    return (v == callCast->getOperand(1));
  }
  else if ( isROCCCFunctionCall(callCast, ROCCCNames::StoreNext) )
  {
    return (v == callCast->getOperand(1));
  }
  else if ( isROCCCFunctionCall(callCast, ROCCCNames::SummationFeedback) )
  {
    return (v == callCast->getOperand(1));
  }
  return false ;
}
std::map<Instruction*,Instruction*> defMap;
Instruction* getDefinitionInstruction(Instruction* i, BasicBlock* BB)
{
  assert(i);
  //if( defMap.find(i) != defMap.end() )
  //  return defMap[i];
  assert(BB);
  for(pred_iterator pred = pred_begin(BB); pred != pred_end(BB); ++pred)
  {
    for(BasicBlock::iterator II = (*pred)->begin(); II != (*pred)->end(); ++II)
    {
      if( isDefinition(&*II, i) )
        return (defMap[i] = &*II);
    }
  }
  for(succ_iterator succ = succ_begin(BB); succ != succ_end(BB); ++succ)
  {
    for(BasicBlock::iterator II = (*succ)->begin(); II != (*succ)->end(); ++II)
    {
      if( isDefinition(&*II, i) )
        return (defMap[i] = &*II);
    }
  }
  return i;
}
