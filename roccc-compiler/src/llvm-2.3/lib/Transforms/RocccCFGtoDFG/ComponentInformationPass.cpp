// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

This pass writes component specific information to the database.
Specifically, for modules it writes:
1) the delay of the component
2) the original struct name used
3) the port ordering necessary to call the module in c

There is currently no system specific information written to the database
in this pass.

Eventually, area and frequency estimations will be written to the database here.

*/

#include "llvm/Pass.h"
#include "llvm/Constants.h"

#include <sstream>
#include <algorithm>

#include "rocccLibrary/sqlite3.h"
#include "rocccLibrary/DFFunction.h"
#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/ROCCCNames.h"
#include "rocccLibrary/DatabaseInterface.h"
#include "rocccLibrary/FunctionType.h"
#include "rocccLibrary/PipelineBlocks.h"
#include "rocccLibrary/SizeInBits.h"
#include "rocccLibrary/MessageLogger.h"
#include "rocccLibrary/CopyValue.h"
#include "rocccLibrary/DefinitionInst.h"
#include "rocccLibrary/GetValueName.h"

namespace llvm
{
  class ComponentInformationPass : public FunctionPass
  {
  private:
  public:
    static char ID ;
    ComponentInformationPass() ;
    ~ComponentInformationPass() ;
    virtual bool runOnFunction(Function& b) ;
    void getAnalysisUsage(AnalysisUsage &AU) const;
  } ;
}

using namespace llvm ;
using namespace Database;

char ComponentInformationPass::ID = 0 ;

static RegisterPass<ComponentInformationPass> X ("componentInfo", 
					"Write component info to the database.");

ComponentInformationPass::ComponentInformationPass() : FunctionPass((intptr_t)&ID) 
{
  ; // Nothing in here
}

ComponentInformationPass::~ComponentInformationPass()
{
  ; // Nothing to delete either
}

void ComponentInformationPass::getAnalysisUsage(AnalysisUsage &AU) const
{
  AU.setPreservesAll();
}

/*
Given a component name and a map of resources used by that component,
update (by deleting, and then inserting) into the ResourcesUsed table the value
(id, resource-id, resource-type, number-of-uses). This data is used by the GUI to discern
dependencies between components and do estimations.
*/
void insertModulesUsedTable(sqlite3* handle, std::string componentName, std::map<std::string,std::map<int,int> > resources)
{
  //handle is for the sqlite3 database
  assert(handle != NULL) ;
  sqlite3_stmt* returnHandle ;
  const char* remainder ;
  int currentValue ;
  
  //get the id of the current component
  std::string idQuery = "SELECT id FROM ComponentInfo WHERE componentName = \'" + componentName + "\'";
  sqlite3_prepare(handle,
                  idQuery.c_str(),
                  idQuery.size(),
                  &returnHandle,
                  &remainder) ;
  currentValue = sqlite3_step(returnHandle) ;
  if (currentValue == SQLITE_DONE)
  {
    INTERNAL_SERIOUS_WARNING("Could not find component when trying to update resources used in database!\n");
    return;
  }
  if (currentValue == SQLITE_MISUSE || currentValue == SQLITE_ERROR)
  {
    INTERNAL_WARNING("Problem finding component " << componentName << " in database; " << sqlite3_errmsg(handle) << "\n");
  }
  int currentID = sqlite3_column_int(returnHandle, 0);
  sqlite3_finalize(returnHandle) ;
  //remove all entries in ModulesUsed that are of component name ComponentName
  std::stringstream checkQuery;
  checkQuery << "DELETE FROM ResourcesUsed WHERE id = \'" << currentID << "\'" ;
  sqlite3_prepare(handle,
                  checkQuery.str().c_str(),
                  checkQuery.str().size(),
                  &returnHandle,
                  &remainder) ;
  currentValue = sqlite3_step(returnHandle) ;
  if (currentValue != SQLITE_DONE)
  {
    INTERNAL_WARNING("Problem deleting component " << componentName << " from database; " << sqlite3_errmsg(handle) << "\n");
  }
  sqlite3_finalize(returnHandle) ;
  //insert the resources used into the resources used table
  for(std::map<std::string,std::map<int,int> >::const_iterator resourceIter = resources.begin() ; resourceIter != resources.end(); ++resourceIter)
  {
    std::string resourceName = resourceIter->first;
    for(std::map<int,int>::const_iterator countIter = resourceIter->second.begin(); countIter != resourceIter->second.end(); ++countIter)
    {
      int resourceSize = countIter->first;
      int resourceCount = countIter->second;
      std::stringstream insertQuery ;
      insertQuery << "INSERT INTO ResourcesUsed (id, resourceID, resourceType, numUsed) VALUES (\'" << currentID << "\', ";
      if( resourceSize == 0 ) //it is a module call
      {
        insertQuery << DatabaseInterface::getInstance()->LookupEntry(resourceName).getID() << ", \'MODULE\', " << resourceCount << ")";
      }
      else //it is a built-in resource
      {
        insertQuery << "NULL, \'" << resourceName << " " << resourceSize << "\', " << resourceCount << ")";
      }
      //execute the query
      sqlite3_prepare(handle,
                      insertQuery.str().c_str(),
                      insertQuery.str().size(),
                      &returnHandle,
                      &remainder) ;
      currentValue = sqlite3_step(returnHandle) ;
      if (currentValue == SQLITE_MISUSE || currentValue == SQLITE_ERROR)
      {
        INTERNAL_WARNING("Cannot insert resource " << resourceName << " into the resource used table; " << sqlite3_errmsg(handle) << "\n");
      }
      sqlite3_finalize(returnHandle) ;
      
      insertQuery.clear() ;
    }
  }
}

/*
Open the sqlite3 database, and return a handle to it.
*/
sqlite3* openDatabaseHandle()
{
  return DatabaseInterface::getInstance()->getHandle();
}

/*
The port order is ordering used to call the component from the c level. The GUI
uses this value to correctly plug a default string into the call instruction.
This command requires that the ComponentInfo table exists, and updates the
port order column of the component named componentName.
*/
void updatePortOrder(sqlite3* handle, std::string componentName, std::string port_order)
{
  assert(handle != NULL and "Must open database before creating table!") ;
  
  std::string query = "UPDATE ComponentInfo SET portOrder = \'" + port_order + "\' WHERE componentName = \'" + componentName + "\' and active=1";
  sqlite3_stmt* returnHandle ;
  const char* remainder ;
  int currentValue ;
  
  sqlite3_prepare(handle,
                  query.c_str(),
                  query.size(),
                  &returnHandle,
                  &remainder) ;
  currentValue = sqlite3_step(returnHandle) ;
  if (currentValue != SQLITE_DONE)
  {
    //failing to update the database . . . is a serious warning
    INTERNAL_SERIOUS_WARNING("Problem updating port order for component " << componentName << "; " << sqlite3_errmsg(handle) << "\n");
  }
  sqlite3_finalize(returnHandle) ;
}

/*
The struct name is used by the GUI to generate top level c code. This value should
be the original name of the module's struct, defined at the top level c code.
This command requires that the ComponentInfo table exists, and updates the struct
name column.
*/
void updateStructName(sqlite3* handle, std::string componentName, std::string struct_name)
{
  assert(handle != NULL and "Must open database before creating table!") ;
  
  std::string query = "UPDATE ComponentInfo SET structName = \'" + struct_name + "\' WHERE componentName = \'" + componentName + "\' and active=1";
  sqlite3_stmt* returnHandle ;
  const char* remainder ;
  int currentValue ;
  
  sqlite3_prepare(handle,
                  query.c_str(),
                  query.size(),
                  &returnHandle,
                  &remainder) ;
  currentValue = sqlite3_step(returnHandle) ;
  if (currentValue != SQLITE_DONE)
  {
    //failing to update the database . . . is a serious warning
    INTERNAL_SERIOUS_WARNING("Problem updating struct name for component " << componentName << "; " << sqlite3_errmsg(handle) << "\n");
  }
  sqlite3_finalize(returnHandle) ;
}

void setCompiled(sqlite3* handle, std::string componentName)
{
  assert(handle != NULL and "Must open database before creating table!") ;
  
  std::stringstream query;
  query << "UPDATE CompileInfo SET isCompiled = \'true\' WHERE id = " << Database::DatabaseInterface::getInstance()->LookupEntry(componentName).getID();
  sqlite3_stmt* returnHandle ;
  const char* remainder ;
  int currentValue ;
  
  sqlite3_prepare(handle,
                  query.str().c_str(),
                  query.str().size(),
                  &returnHandle,
                  &remainder) ;
  currentValue = sqlite3_step(returnHandle) ;
  if (currentValue != SQLITE_DONE)
  {
    //failing to update the database . . . is a serious warning
    INTERNAL_SERIOUS_WARNING("Problem updating compiled flag for component " << componentName << "; " << sqlite3_errmsg(handle) << "\n");
  }
  sqlite3_finalize(returnHandle) ;
}

llvm::Value* getCopiedValue(llvm::Value* v)
{
  if( isCopyValue(v) )
    return getCopiedValue(dynamic_cast<Instruction*>(v)->getOperand(0));
  return v;
}

std::string getLLVMPrintableValueName(llvm::Value* v)
{
  ConstantInt* ci = dynamic_cast<ConstantInt*>(v) ;
  ConstantFP* cf = dynamic_cast<ConstantFP*>(v) ;
  if (ci != NULL)
  {
    std::stringstream ss;
    ss << static_cast<int>(ci->getValue().getSExtValue());
    return ss.str();
  }
  else if (cf != NULL)
  {
    std::stringstream ss;
    if( APFloat::semanticsPrecision(cf->getValueAPF().getSemantics()) < 32 )
    {
      ss << cf->getValueAPF().convertToFloat();
    }
    else
    {
      ss << cf->getValueAPF().convertToDouble();
    }
    return ss.str();
  }
  return getValueName(v);
}

void updateStreamInfo(sqlite3* handle, std::string componentName, ROCCCLoopInformation loopInfo)
{
  assert(handle != NULL and "Must open database before updating stream info!") ;
  std::map<std::string,std::string> streamToFunction;
  std::map<std::string,int> streamToNumElementsAccessed;
  std::map<std::string,int> streamToNumWindowElements;
  for(std::vector<Value*>::iterator IBI = loopInfo.inputBuffers.begin(); IBI != loopInfo.inputBuffers.end(); ++IBI)
  {
    std::stringstream formula;
    formula << "1";
    int livIndex = 0;
    bool isInfinite = false;
    streamToNumWindowElements[getValueName(*IBI)] = loopInfo.inputBufferNumInvalidated[*IBI];
    for(std::vector<Value*>::iterator LIV = loopInfo.inputBufferLoopIndexes[*IBI].begin(); LIV != loopInfo.inputBufferLoopIndexes[*IBI].end(); ++LIV)
    {
      if( isROCCCFunctionCall(dynamic_cast<llvm::CallInst*>(getCopiedValue(loopInfo.endValues[*LIV])), ROCCCNames::InfiniteLoopCondition) )
        isInfinite = true;
      else
      {
        formula << " * (" << getLLVMPrintableValueName(loopInfo.endValues[*LIV]) << " + ";
        int max = -1;
        streamToNumElementsAccessed[getValueName(*IBI)] = 0;
        for(std::vector<std::pair<Value*,std::vector<int> > >::iterator BII = loopInfo.inputBufferIndexes[*IBI].begin(); BII != loopInfo.inputBufferIndexes[*IBI].end(); ++BII)
        {
          if( max < BII->second.at(livIndex) )
            max = BII->second.at(livIndex);
          ++streamToNumElementsAccessed[getValueName(*IBI)];
        }
        assert( max >= 0 );
        formula << max << ")";
      }
      ++livIndex;
    }
    if( isInfinite )
      streamToFunction[getValueName(*IBI)] = "INFINITE";
    else
      streamToFunction[getValueName(*IBI)] = formula.str();
  }
  for(std::vector<Value*>::iterator OBI = loopInfo.outputBuffers.begin(); OBI != loopInfo.outputBuffers.end(); ++OBI)
  {
    std::stringstream formula;
    formula << "1";
    int livIndex = 0;
    bool isInfinite = false;
    streamToNumWindowElements[getValueName(*OBI)] = loopInfo.outputBufferNumInvalidated[*OBI];
    for(std::vector<Value*>::iterator LIV = loopInfo.outputBufferLoopIndexes[*OBI].begin(); LIV != loopInfo.outputBufferLoopIndexes[*OBI].end(); ++LIV)
    {
      if( isROCCCFunctionCall(dynamic_cast<llvm::CallInst*>(getCopiedValue(loopInfo.endValues[*LIV])), ROCCCNames::InfiniteLoopCondition) )
        isInfinite = true;
      else
      {
        formula << " * (" << getLLVMPrintableValueName(loopInfo.endValues[*LIV]) << " + ";
        int max = -1;
        streamToNumElementsAccessed[getValueName(*OBI)] = 0;
        for(std::vector<std::pair<Value*,std::vector<int> > >::iterator BII = loopInfo.outputBufferIndexes[*OBI].begin(); BII != loopInfo.outputBufferIndexes[*OBI].end(); ++BII)
        {
          if( max < BII->second.at(livIndex) )
            max = BII->second.at(livIndex);
          ++streamToNumElementsAccessed[getValueName(*OBI)];
        }
        assert( max >= 0 );
        formula << max << ")";
        ++livIndex;
      }
    }
    if( isInfinite )
      streamToFunction[getValueName(*OBI)] = "INFINITE";
    else
      streamToFunction[getValueName(*OBI)] = formula.str();
  }
  for(std::map<std::string,std::string>::iterator SFI = streamToFunction.begin(); SFI != streamToFunction.end(); ++SFI)
  {
    std::stringstream query;
    query << "INSERT INTO StreamInfo (id, readableName, NumElementsCalculationFormula, NumAccessedWindowElements, NumTotalWindowElements) VALUES (" << Database::DatabaseInterface::getInstance()->LookupEntry(componentName).getID() << ", '" << SFI->first << "', '" << SFI->second << "', " << streamToNumElementsAccessed[SFI->first] << ", " << streamToNumWindowElements[SFI->first] << ")";
    sqlite3_stmt* returnHandle ;
    const char* remainder ;
    int currentValue ;
    
    sqlite3_prepare(handle,
                    query.str().c_str(),
                    query.str().size(),
                    &returnHandle,
                    &remainder) ;
    currentValue = sqlite3_step(returnHandle) ;
    if (currentValue != SQLITE_DONE)
    {
      INTERNAL_WARNING("Problem updating stream info for component " << componentName << ", stream " << SFI->first << "; " << sqlite3_errmsg(handle) << "\n");
    }
    sqlite3_finalize(returnHandle) ;
  }
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

std::vector<llvm::Value*> getUsedVars(Instruction* i)
{
  assert(i);
  std::map<llvm::Value*, bool> allVarsUsedMap;
  for(User::op_iterator OP = i->op_begin(); OP != i->op_end(); ++OP)
  {
    allVarsUsedMap[OP->get()] = true;
  }
  std::vector<llvm::Value*> allVarsUsed;
  for(std::map<llvm::Value*, bool>::iterator MI = allVarsUsedMap.begin(); MI != allVarsUsedMap.end(); ++MI)
  {
    Instruction* inst = dynamic_cast<Instruction*>(MI->first);
    if( inst )
    {
      if( MI->second == true and !isDefinition(i, inst) )
        allVarsUsed.push_back( MI->first );
    }
  }
  return allVarsUsed;
}
std::vector<llvm::Value*> getLiveInVars(Instruction* i)
{
  assert(i);
  int curLevel = i->getParent()->getDFBasicBlock()->getPipelineLevel();
  std::vector<llvm::Value*> allVarsUsed = getUsedVars(i);
  std::vector<llvm::Value*> liveInVars;
  for(std::vector<llvm::Value*>::iterator MI = allVarsUsed.begin(); MI != allVarsUsed.end(); ++MI)
  {
    Instruction* inst = dynamic_cast<Instruction*>(*MI);
    if( inst )
    {
      Instruction* definition = getDefinitionInstruction(inst, i->getParent());
      int definitionLevel = definition->getParent()->getDFBasicBlock()->getPipelineLevel();
      if( curLevel < definitionLevel and !isDefinition(i, inst) )
        liveInVars.push_back( *MI );
    }
  }
  return liveInVars;
}

bool ComponentInformationPass::runOnFunction(Function& f)
{
  CurrentFile::set(__FILE__);
  bool changed = false ;
  // Make sure this is a function that we can use
  if (f.isDeclaration() || (!f.isDFFunction() and ROCCC::ROCCCFunctionType(&f) != ROCCC::SYSTEM) )
  {
    return changed ;
  }
  
  //open the database, then create the ComponentInfo table - if the table already
  //exists, this does nothing
  sqlite3* handle = openDatabaseHandle();
    
  //we assume that a component has already been created with PortNamePrinterPass,
  //   so we arent going to worry about creating it here
  
  //First, go through all of the instructions, find the global information, and save it.
  std::string port_order;
  std::string struct_name;
  for(Function::iterator BB = f.begin(); BB != f.end(); ++BB)
  {
    for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
    {
      CallInst* CI = dynamic_cast<CallInst*>(&*II);
      //a portOrder call means we need to pull out the relevant data and save it
      if( isROCCCFunctionCall(CI, ROCCCNames::PortOrder) )
      {
        //portOrder calls are of the form ROCCCInvokeHardware("PortOrder", <port_order constant character string>)
        assert( CI->getNumOperands() == 2 and "Mismatched number of arguments to PortOrder!" );
        Constant* tmpCast = dynamic_cast<Constant*>(CI->getOperand(1));
        assert( tmpCast and "Port order must be a constant!" );
        assert( tmpCast->getStringValue() != "" and "Port order must be an inline constant, and not a global!" );
        port_order = tmpCast->getStringValue();
      }
      //a moduleStructName call means we need to pull out the relevant data and save it
      if( isROCCCFunctionCall(CI, ROCCCNames::ModuleStructName) )
      {
        //moduleStructName calls are of the form ROCCCInvokeHardware("ModuleStructName", <struct name constant character string>)
        assert( CI->getNumOperands() == 2 and "Mismatched number of arguments to ModuleStructName!" );
        Constant* tmpCast = dynamic_cast<Constant*>(CI->getOperand(1));
        assert( tmpCast and "Struct name must be a constant!");
        assert( tmpCast->getStringValue() != "" and "Struct name must be an inline constant, and not a global!" );
        struct_name = tmpCast->getStringValue();
      }
    }
  }
  //find all of the resources used
  std::map<std::string,std::map<int,int> > resources;
  std::map<DFBasicBlock*, bool> allBlocks = getPipelineBlocks(f);
  if( allBlocks.empty() ) //its a system, so go through manually?
  {
    INTERNAL_WARNING("Manual resource checking not yet implemented for SystemToSystem!\n");
  }
  for(std::map<DFBasicBlock*,bool>::iterator BMI = allBlocks.begin(); BMI != allBlocks.end(); ++BMI)
  {
    Instruction* II = &*BMI->first->begin();
    CallInst* CI = dynamic_cast<CallInst*>(&*II);
    //check for incoming pipeline boundary registers
    if( isROCCCFunctionCall(CI, ROCCCNames::InvokeHardware) )
    {
      //because InvokeHardwares are mapped directly to the component instantiations, they have no boundary registers (other than whats in the actual component)
    }
    else //check to see if there are any pipeline boundary registers
    {
      //check how many registers are created at pipeline boundaries
      std::vector<llvm::Value*> liveInVars = getLiveInVars(II);
      std::stringstream ss;
      for(std::vector<llvm::Value*>::iterator LVI = liveInVars.begin(); LVI != liveInVars.end(); ++LVI)
      {
        if(getSizeInBits(*LVI) > 0)
        {
          ++resources["REGISTER"][getSizeInBits(*LVI)];
          ss << (*LVI)->getName() << " - " << getSizeInBits(*LVI) << "\n";
        }
      }
      std::stringstream ss2;
      ss2 << *II << " has the following boundary registers:";
      if( !liveInVars.empty() )
      {
        LOG_MESSAGE2("Resource Utilization", ss2.str(), ss.str() );
      }
    }
    //now check each resource type
    if( isROCCCFunctionCall(CI, ROCCCNames::BoolSelect) )
    {
      ++resources["MUX"][getSizeInBits(II)];
    }
    if( isROCCCFunctionCall(CI, ROCCCNames::InvokeHardware) )
    {
      //an InvokeHardware call means we are using a module,
      //so add it to the list of modules_used
      std::string componentName = getComponentNameFromCallInst( CI );
      ++resources[componentName][0];
    }
    else if( isCopyValue(II) or dynamic_cast<BitCastInst*>(II) )
    {
      //copies do not add to area, but are used in frequency calculating
      ++resources["COPY"][getSizeInBits(II)];
    }
    else if( dynamic_cast<BinaryOperator*>(II) )
    {
      switch(dynamic_cast<BinaryOperator*>(II)->getOpcode())
      {
        case BinaryOperator::Add:
        {
     	    ++resources["ADD"][getSizeInBits(II)];
     	    break;
        }
        case BinaryOperator::Sub:
        {
      	  ++resources["SUB"][getSizeInBits(II)];
     	    break;
        }
        case BinaryOperator::Mul:
        {
          ++resources["MUL"][getSizeInBits(II)];
     	    break;
        }
        case BinaryOperator::Shl:  // Shift left
        {
          ++resources["SHL"][getSizeInBits(II)];
     	    break;
        }
        case BinaryOperator::LShr: // Shift right logical
        case BinaryOperator::AShr: // Shift right arithmetic
        {
          ++resources["SHR"][getSizeInBits(II)];
     	    break;
        }
        case BinaryOperator::And:
        {
          ++resources["AND"][getSizeInBits(II)];
     	    break;
        }
        case BinaryOperator::Or:
        {
          ++resources["OR"][getSizeInBits(II)];
     	    break;
        }
        case BinaryOperator::Xor:
        {
          ++resources["XOR"][getSizeInBits(II)];
     	    break;
        }
        default:
        {
          INTERNAL_ERROR("Unknown binary operation " << *II);
          assert( 0 and "Unknown binary operation!" );
        }
      }
    }
    else if( dynamic_cast<CmpInst*>(II) )
    {
      switch (dynamic_cast<CmpInst*>(II)->getPredicate())
      {
        case ICmpInst::ICMP_EQ: // equal
        {
          ++resources["EQ"][getSizeInBits(II)];
          break;
        }
        case ICmpInst::ICMP_NE: // Not equal
        {
          ++resources["NE"][getSizeInBits(II)];
          break;
        }
        case ICmpInst::ICMP_SGT: // Greater than (signed)
        case ICmpInst::ICMP_UGT: // Greater than (unsigned)
        {
          ++resources["GT"][getSizeInBits(II)];
          break;
        }
        case ICmpInst::ICMP_SGE: // Greater than or equal (signed)
        case ICmpInst::ICMP_UGE: // Greater than or equal (unsigned)
        {
          ++resources["GTE"][getSizeInBits(II)];
          break;
        }
        case ICmpInst::ICMP_SLT: // Less than (signed)
        case ICmpInst::ICMP_ULT: // Less than (unsigned)
        {
          ++resources["LT"][getSizeInBits(II)];
          break;
        }    
        case ICmpInst::ICMP_SLE: // Less than or equal (signed)
        case ICmpInst::ICMP_ULE: // Less than or equal (unsigned)
        {
          ++resources["LTE"][getSizeInBits(II)];
          break;
        }
        default:
        {
          INTERNAL_ERROR("Unknown comparison " << *II);
          assert( 0 and "Unknown operator!" );
        }
      }
    }
    else
    {
      //INTERNAL_WARNING("Unknown instruction " << *II);
    }
  }
  //output the resources used to the log file
  for(std::map<std::string,std::map<int,int> >::iterator RMI = resources.begin(); RMI != resources.end(); ++RMI)
  {
    std::stringstream ss;
    ss << "Resource " << RMI->first << ": \n";
    for(std::map<int,int>::iterator SMI = RMI->second.begin(); SMI != RMI->second.end(); ++SMI)
    {
      if( SMI->first == 0 )
        ss << "Module instantiated " << SMI->second << " times.\n";
      else
        ss << SMI->first << "-bit instantiated " << SMI->second << " times.\n";
    }
    LOG_MESSAGE1("Resource Utilization", ss.str());
  }
  //update the relevant information
  insertModulesUsedTable(handle, f.getName(), resources);
  if( port_order != "" )
  {
    updatePortOrder(handle, f.getName(), port_order);
  }
  else
  {
    INTERNAL_WARNING("No port order found!\n");
  }
  if( struct_name != "" )
  {
    updateStructName(handle, f.getName(), struct_name);
  }
  else
  {
    INTERNAL_WARNING("No struct name found!\n");
  }
  if( f.getDFFunction() )
    updateStreamInfo(handle, f.getName(), f.getDFFunction()->getROCCCLoopInfo());
  
  setCompiled(handle, f.getName());

  return changed ;
}

