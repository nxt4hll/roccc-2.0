// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

Detects misusage of functions.
1) All function calls must be calls to ROCCC functions
2) All calls to InvokeHardware should be invoking an ipcore that exists in the database
3) All calls to InvokeHardware should have the same number of arguments as the ipcore
      has ports.

*/

#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"

#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/DatabaseInterface.h"
#include "rocccLibrary/ROCCCNames.h"

namespace llvm
{
  class FunctionNameVerifyPass : public FunctionPass
  {
  private:
  public:
    static char ID ;
    FunctionNameVerifyPass() ;
    ~FunctionNameVerifyPass() ;
    virtual bool runOnFunction(Function& b) ;
  } ;
}

using namespace llvm ;
using namespace Database;

char FunctionNameVerifyPass::ID = 0 ;

static RegisterPass<FunctionNameVerifyPass> X ("functionVerify", 
					"Detect uses of functions that do not conform to ROCCC specification.");

FunctionNameVerifyPass::FunctionNameVerifyPass() : FunctionPass((intptr_t)&ID) 
{
  ; // Nothing in here
}

FunctionNameVerifyPass::~FunctionNameVerifyPass()
{
  ; // Nothing to delete either
}

/*
Detects misusage of functions.
1) All function calls must be calls to ROCCC functions
2) All calls to InvokeHardware should be invoking an ipcore that exists in the database
3) All calls to InvokeHardware should have the same number of arguments as the ipcore
      has ports.
*/
bool FunctionNameVerifyPass::runOnFunction(Function& f)
{
  CurrentFile::set(__FILE__);
  bool changed = false ;
  
  for(Function::iterator BB = f.begin(); BB != f.end(); ++BB)
  {
    for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
    {
      if( CallInst* CI = dynamic_cast<CallInst*>(&*II) )
      {
        //if a call instruction isnt a ROCCC function, error out
        if( !ROCCCNames::isROCCCFunction(CI) )
        {
          INTERNAL_ERROR("Detected non-roccc function!" << *II << "\n");
          assert( 0 and "Bad function found!" );
        }
        //if its an InvokeHardware, make sure it follows the rules
        if( isROCCCFunctionCall(CI, ROCCCNames::InvokeHardware) )
        {
          DatabaseInterface* dbInterface = DatabaseInterface::getInstance();
          //if the ipcore doesnt exist in the database, error out
          if( !dbInterface->EntryExists(getComponentNameFromCallInst(CI)) )
          {
            INTERNAL_ERROR("Component " << getComponentNameFromCallInst(CI) << " does not exist in database!\n");
            assert(0 and "Bad call to invoke hardware!");
          }
          //if the number of ports doesnt match the number of operands, error out
          LibraryEntry currentEntry = dbInterface->LookupEntry(getComponentNameFromCallInst(CI)) ;
          //rewritten to handle streams
          unsigned count = 0;
          const std::list<Port*> ports = currentEntry.getNonDebugScalarPorts();
          for(std::list<Port*>::const_iterator PI = ports.begin(); PI != ports.end(); ++PI)
          {
            ++count;
          }
          const std::list<Stream> streams = currentEntry.getStreams();
          for(std::list<Stream>::const_iterator SI = streams.begin(); SI != streams.end(); ++SI)
          {
            ++count;
          }
          //subtract 2 because one operand is the name of the function to call, and one is the ipcore to invoke
          if( count != CI->getNumOperands() - 2 )
          {
            INTERNAL_ERROR("Bad call to invoke hardware " << getComponentNameFromCallInst(CI) << ": mismatched number of arguments!\n");
            INTERNAL_MESSAGE("NumPorts = " << ports.size() << "\n");
            INTERNAL_MESSAGE(*CI);
            assert(0 and "Bad call to invoke hardware!");
          }
        }
      }
    }
  }

  return changed ;
}

