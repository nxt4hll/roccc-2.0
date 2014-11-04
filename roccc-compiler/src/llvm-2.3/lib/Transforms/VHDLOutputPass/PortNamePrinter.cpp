#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "rocccLibrary/passes/VHDLOutputPass.h"
#include "rocccLibrary/passes/SystemToSystemPass.h"
#include "rocccLibrary/VHDLInterface.h"
#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/GetValueName.h"
#include "rocccLibrary/ROCCCNames.h"
#include "rocccLibrary/FunctionType.h"
#include "rocccLibrary/DatabaseInterface.h"
#include "rocccLibrary/DatabaseHelpers.h"

using namespace llvm ;
using namespace Database;

class PortNamePrinterPass : public FunctionPass {
public:
  static char ID;
  PortNamePrinterPass();
  ~PortNamePrinterPass();
  virtual bool runOnFunction(Function&);
  void getAnalysisUsage(AnalysisUsage &AU) const;
};

char PortNamePrinterPass::ID = 0 ;

static RegisterPass<PortNamePrinterPass> X("printPortNames",
					   "Generate a file listing the port names") ;

PortNamePrinterPass::PortNamePrinterPass() : FunctionPass((intptr_t)&ID)
{
}

PortNamePrinterPass::~PortNamePrinterPass()
{
}

std::string getTypeAsString(const llvm::Type* T)
{
  if( !T )
    return "NULL";
  if ( T->getTypeID() == Type::PointerTyID or T->getTypeID() == Type::ArrayTyID )
  {
    const SequentialType* ST = dynamic_cast<const SequentialType*>( T );
    assert( ST and "Problem trying to figure out a type of a value." ); 
    return getTypeAsString( ST->getElementType() ) + "*";
  }
  if( T->isInteger() )
  {
    return "int";
  }
  else if( T->isFloatingPoint() )
  {
    return "float";
  }
  else
  {
    INTERNAL_WARNING("Type #" << T->getTypeID() << "(" << T->getDescription() << ") is not integer or floating point, and has type of " << T->getDescription() << "!\n");
  }
  return T->getDescription();
}

std::string getTypeOfValueAsString(const llvm::Value* v)
{
  if( !v )
    return "NULL";
  return getTypeAsString(v->getType());
}

std::string getFunctionTypeAsString(Function* f)
{
  for(Function::iterator BB = f->begin(); BB != f->end(); ++BB)
  {
    for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
    {
      if(CallInst* CI = dynamic_cast<CallInst*>(&*II))
      {
        if( isROCCCFunctionCall(CI, ROCCCNames::IntrinsicType) )
        {
          assert( CI->getNumOperands() == 2 and "Mismatched number of arguments to IntrinsicType!" );
          Constant* tmpCast = dynamic_cast<Constant*>(CI->getOperand(1));
          assert( tmpCast and "Intrinsic type must be a constant!" );
          assert( tmpCast->getStringValue() != "" and "Intrinsic type must be an inline constant, and not a global!" );
          return tmpCast->getStringValue();
        }
      }
    }
  }
  if( f->isDFFunction() )
  {
    if( f->getDFFunction()->getFunctionType() == ROCCC::BLOCK )
      return LibraryEntry::convTypeToString(LibraryEntry::MODULE);
    else if( f->getDFFunction()->getFunctionType() == ROCCC::MODULE )
      return LibraryEntry::convTypeToString(LibraryEntry::SYSTEM);
    else
      return LibraryEntry::convTypeToString(LibraryEntry::SYSTEM);
  }
  assert(0 and "Unknown function type! Could this not be a dffunction?");
}

bool PortNamePrinterPass::runOnFunction(Function& f)
{
  CurrentFile::set(__FILE__);
  bool changed = false;
  // Make sure this is a function that we can use
  if (f.isDeclaration() || (!f.isDFFunction() and ROCCC::ROCCCFunctionType(&f) != ROCCC::SYSTEM) )
  {
    return changed ;
  }
  VHDLInterface::Entity* UUT = NULL;
  std::vector<llvm::StreamVariable>::iterator streamsBegin, streamsEnd = streamsBegin;
  int delay = 1;
  std::string componentType;
  if( !f.getDFFunction() )
  {
    UUT = getAnalysis<ROCCCSystemToSystemPass>().entity;
    streamsBegin = getAnalysis<ROCCCSystemToSystemPass>().streamBegin();
    streamsEnd = getAnalysis<ROCCCSystemToSystemPass>().streamEnd();
    componentType = LibraryEntry::convTypeToString(LibraryEntry::SYSTEM);
  }
  else
  {
    DFFunction* df = f.getDFFunction() ;
    UUT = getAnalysis<VHDLOutputPass>().entity;
    streamsBegin = getAnalysis<VHDLOutputPass>().streamBegin();
    streamsEnd = getAnalysis<VHDLOutputPass>().streamEnd();
    delay = df->getDelay() + 1;
    componentType = getFunctionTypeAsString(df);
  }
  std::map< VHDLInterface::Port*,std::pair<std::string, std::string> > portMap;
  std::map< VHDLInterface::Port*,llvm::Value* > streamValuesMap;
  for(std::vector<llvm::StreamVariable>::iterator SVI = streamsBegin; SVI != streamsEnd; ++SVI)
  {
    std::string readableName = SVI->readableName;
    for(std::vector<VHDLInterface::Variable*>::iterator DPI = SVI->data_channels.begin(); DPI != SVI->data_channels.end(); ++DPI)
    {
      streamValuesMap[dynamic_cast<VHDLInterface::Port*>(*DPI)] = SVI->value;
      portMap[dynamic_cast<VHDLInterface::Port*>(*DPI)] = std::pair<std::string,std::string>(readableName, "STREAM_CHANNEL");
    }
    for(std::vector<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> >::iterator API = SVI->address_channels.begin(); API != SVI->address_channels.end(); ++API)
    {
      portMap[dynamic_cast<VHDLInterface::Port*>(API->first)] = std::pair<std::string,std::string>(readableName, "STREAM_ADDRESS_BASE");
      portMap[dynamic_cast<VHDLInterface::Port*>(API->second)] = std::pair<std::string,std::string>(readableName, "STREAM_ADDRESS_COUNT");
    }
    portMap[dynamic_cast<VHDLInterface::Port*>(SVI->cross_clk)] = std::pair<std::string,std::string>(readableName, "STREAM_CROSS_CLK");
    portMap[dynamic_cast<VHDLInterface::Port*>(SVI->stop_access)] = std::pair<std::string,std::string>(readableName, "STREAM_STOP_ACCESS");
    portMap[dynamic_cast<VHDLInterface::Port*>(SVI->enable_access)] = std::pair<std::string,std::string>(readableName, "STREAM_ENABLE_ACCESS");
    portMap[dynamic_cast<VHDLInterface::Port*>(SVI->address_rdy)] = std::pair<std::string,std::string>(readableName, "STREAM_ADDRESS_RDY");
    portMap[dynamic_cast<VHDLInterface::Port*>(SVI->address_stall)] = std::pair<std::string,std::string>(readableName, "STREAM_ADDRESS_STALL");
    portMap[dynamic_cast<VHDLInterface::Port*>(SVI->address_clk)] = std::pair<std::string,std::string>(readableName, "STREAM_ADDRESS_CLK");
  }
  
  std::list<Port*> ports;
  int portCount = 1;
  for( std::vector<VHDLInterface::Port*>::iterator PI = UUT->getDeclaration()->firstNonStandardPort(); PI != UUT->getDeclaration()->getPorts().end(); ++PI )
  {
    llvm::Value* llvmVal = (*PI)->getLLVMValue();
    if( streamValuesMap.find(*PI) != streamValuesMap.end() )
    {
      llvmVal = streamValuesMap[*PI];
    }
    if( portMap.find(*PI) == portMap.end() )
    {
      std::string readableName = (*PI)->getInternalName();
      std::string type = "REGISTER";
      if( (*PI)->getLLVMValue() )
        readableName = getValueName((*PI)->getLLVMValue());
      if( (*PI)->getAttributeValue(UUT->getAttribute("readable_name")) != "" )
        readableName = (*PI)->getAttributeValue(UUT->getAttribute("readable_name"));
      if( (*PI)->getAttributeValue(UUT->getAttribute("port_type")) != "" )
      {
        type = (*PI)->getAttributeValue(UUT->getAttribute("port_type"));
      }
      ports.push_back(new Database::Port((*PI)->getName(), (*PI)->getType() == VHDLInterface::Port::INPUT, (*PI)->getSize(), type, readableName, getTypeOfValueAsString(llvmVal)));
    }
    else
    {
      ports.push_back(new Database::Port((*PI)->getName(), (*PI)->getType() == VHDLInterface::Port::INPUT, (*PI)->getSize(), portMap[*PI].second, portMap[*PI].first, getTypeOfValueAsString(llvmVal)));
    }
    ++portCount;
  }

  LibraryEntry toAdd(UUT->getDeclaration()->getName(), delay, ports, componentType, Database::getCurrentID()) ;
  DatabaseInterface::getInstance()->CreateEntry(&toAdd) ;
 
  return changed;
}

void PortNamePrinterPass::getAnalysisUsage(AnalysisUsage &AU) const
{
  AU.addRequired<VHDLOutputPass>();
  AU.addRequired<ROCCCSystemToSystemPass>();
  AU.setPreservesAll();
}
