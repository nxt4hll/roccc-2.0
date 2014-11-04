#include "rocccLibrary/DFFunction.h"
#include "rocccLibrary/InternalWarning.h"

#include <sstream>

using namespace llvm;

// ROCCC Code - Loop Information function definitions, intentionally
//                 defined here.
ROCCCLoopInformation::ROCCCLoopInformation()
{
  // Nothing yet
}

ROCCCLoopInformation::~ROCCCLoopInformation()
{
  // Nothing yet
}

DFFunction::DFFunction(const FunctionType* Ty, 
		       Function::LinkageTypes Linkage,
		       const std::string& N, Module* M) :
  Function(Ty, Linkage, N, M)
{
  source = NULL ;
  sink = NULL ;
  delay = -1 ;
  functionType = -1 ;
}
// No destructor so the only destructor that gets called is the Function
//  destructor.  This will probably change in the future
DFFunction::~DFFunction()
{
} 
bool DFFunction::isDFFunction()
{
  return true ;
}
DFFunction* DFFunction::getDFFunction()
{
  return this ;
}
DFFunction* DFFunction::Create(const FunctionType* Ty,
		    Function::LinkageTypes Linkage,
			  const std::string& N,
			  Module* M)
{
  return new(0) DFFunction(Ty, Linkage, N, M) ;
}
DFBasicBlock* DFFunction::getSink()
{
  return sink ;
}
DFBasicBlock* DFFunction::getSource()
{
  return source ;
}
void DFFunction::setSource(DFBasicBlock* s)
{
  source = s ; 
}
void DFFunction::setSink(DFBasicBlock* s)
{
  sink = s ;
}
int DFFunction::getDelay()
{
  return delay ;
}
void DFFunction::setDelay(int d)
{
  delay = d ;
}
int DFFunction::getFunctionType()
{
  return functionType;
} 
void DFFunction::setFunctionType(int t)
{
  functionType = t;
}
const ROCCCLoopInformation DFFunction::getROCCCLoopInfo() 
{
  return rocccLoopInfo ;
}
void DFFunction::setROCCCLoopInfo(ROCCCLoopInformation _r) 
{
  rocccLoopInfo = _r ;
}

