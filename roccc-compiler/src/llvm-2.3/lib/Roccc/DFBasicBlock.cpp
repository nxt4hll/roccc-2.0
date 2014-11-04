#include "rocccLibrary/DFBasicBlock.h"

#include "llvm/Constants.h"
#include "llvm/Support/CFG.h"

#include <sstream>

#include "rocccLibrary/DatabaseInterface.h"
#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/ROCCCNames.h"

using namespace llvm;
using namespace Database;

// In the constructor, we add a terminator instruction (unreachable).  This
//  could be replaced with a switch later on, but is there to make sure 
//  we have a well formed block.
DFBasicBlock::DFBasicBlock(const std::string& Name,
                           Function* Parent,
                           BasicBlock* InsertBefore) : 
BasicBlock(Name, Parent, InsertBefore)
{
  dataflowLevel = -1 ;
  pipelineLevel = -1 ;
  marked = false ;
  isSynch = false ;
  new UnreachableInst( this );
}

DFBasicBlock::~DFBasicBlock()
{
  ;
}

//  This function treats "this" as a definition and connects to a 
//   use that is passed in.
void DFBasicBlock::AddUse(DFBasicBlock* u)
{
  if ( u == this ) //dont connect a basicblock to itself
    return;
  assert(u != NULL and "AddUse() passed null argument!") ;
  Instruction* term = getTerminator();
  if (term == NULL)
  {
    // We don't have a terminator, so create a new Switch statement
    //  and append it to my list of instructions
    SwitchInst::Create(ConstantInt::getFalse(), u, 0, this) ;
    return ;
  }
  if (dynamic_cast<SwitchInst*>(term) != NULL)
  {
    bool already_added = false;
    for ( unsigned int suc = 0; suc < dynamic_cast<SwitchInst*>(term)->getNumSuccessors(); ++suc ){
      if ( dynamic_cast<SwitchInst*>(term)->getSuccessor(suc) == u ){
        already_added = true;
      }
    }
    if ( !already_added ){
      dynamic_cast<SwitchInst*>(term)->addCase(ConstantInt::getFalse(),
                                               u) ;
    }
  }
  else
  {
    term->eraseFromParent() ;
    SwitchInst::Create(ConstantInt::getFalse(), u, 0, this) ;
  }
}

// This function treats "this" as the old definition and removes the
//  case to the old basic block.
//  When this is called (the InsertCopy pass) we can assume the graph has
//  been set up correctly.
void DFBasicBlock::RemoveUse(DFBasicBlock* u)
{
  assert(u != NULL and "RemoveUse() passed null argument!") ;
  if (u == this)
  {
    return ;
  }
  TerminatorInst* term = getTerminator() ;
  assert(term != NULL and "BasicBlock does not have terminator!") ;
  SwitchInst* endSwitch = dynamic_cast<SwitchInst*>(term) ;
  assert(endSwitch != NULL and "BasicBlock's terminator is not a switch instruction!") ;
  SwitchInst* new_switch = NULL;
  for (unsigned int successorNumber = 0 ; successorNumber < endSwitch->getNumSuccessors() ; ++successorNumber)
  {
    if( endSwitch->getSuccessor(successorNumber) != u )
    {
      if( new_switch == NULL )
        new_switch = SwitchInst::Create(ConstantInt::getFalse(), endSwitch->getSuccessor(successorNumber), 0, this) ;
      else
        new_switch->addCase(ConstantInt::getFalse(), endSwitch->getSuccessor(successorNumber));
    }
  }
  endSwitch->eraseFromParent();
  if( !new_switch )
    new UnreachableInst( this );
  // One link will have to remain...
  // One link to draw them in,
  // One link to find them.
  // One link to rule them all,
  // and in the darkness bind them.
}      

Instruction* DFBasicBlock::addInstruction(Instruction* II)
{
  if( II->getParent() == this )
    return II;
  assert( !II->getParent() and "Cannot add an instruction that already has a parent to a different basicblock!" );
  this->getInstList().push_front(II) ;
  for(Instruction::op_iterator OP = II->op_begin(); OP != II->op_end(); ++OP)
  {
    Instruction* opi = dynamic_cast<Instruction*>(&**OP);
    if( !opi )
      continue;
    if( opi->getParent() and opi->getParent()->getDFBasicBlock() )
      opi->getParent()->getDFBasicBlock()->AddUse(this);
  }
  return II;
}

int DFBasicBlock::getDelay()
{
  Instruction* singleInstr = getFirstNonPHI() ;
  CallInst* callCast = dynamic_cast<CallInst*>(singleInstr) ;
  // Check to make sure the substring ROCCCInvokeHardware is in the beginning
  //  of the string
  if( isROCCCFunctionCall(callCast, ROCCCNames::InvokeHardware) )
  {
    // Find the corresponding delay.  This involves opening up the library
    //  and searching for it.
    DatabaseInterface* theDB = DatabaseInterface::getInstance() ;
    std::string componentName = getComponentNameFromCallInst(callCast);
    LibraryEntry currentEntry = theDB->LookupEntry(componentName) ;
    return currentEntry.getDelay() ;
  }
  // Not a component, so just a delay of 1
  return 1 ;
}

