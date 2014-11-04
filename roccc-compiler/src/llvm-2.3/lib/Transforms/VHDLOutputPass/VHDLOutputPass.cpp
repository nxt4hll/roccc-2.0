// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*
  File: VHDLOutputPass.cpp
  
  Purpose: The declaration of the pass responsible for taking the 
           data flow graph and outputting synthesizable VHDL is located
	   here.  This pass is also responsible for importing and exporting
	   cores into the VHDL library that we are maintaining.
*/
#include "rocccLibrary/passes/VHDLOutputPass.h"

#include "llvm/ValueSymbolTable.h"
#include "llvm/Support/CFG.h"
#include "llvm/Constants.h"
#include "llvm/Module.h"

#include <sstream>
#include <fstream>
#include <iomanip>

#include "rocccLibrary/passes/InputController.h"
#include "rocccLibrary/passes/OutputController.h"
#include "rocccLibrary/DatabaseInterface.h"
#include "rocccLibrary/SizeInBits.h"
#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/MessageLogger.h"
#include "rocccLibrary/ROCCCNames.h"
#include "rocccLibrary/CopyValue.h"
#include "rocccLibrary/DefinitionInst.h"
#include "rocccLibrary/GetValueName.h"
#include "rocccLibrary/FunctionType.h"
#include "rocccLibrary/FileInfo.h"
#include "rocccLibrary/DatabaseHelpers.h"

using namespace llvm ;
using namespace Database;

// The one ID used in all VHDL Output Pass instances
char VHDLOutputPass::ID = 0 ;

// The call to the constructor that registers this pass
static RegisterPass<VHDLOutputPass> X("vhdl", "VHDL Output Pass") ;

void VHDLOutputPass::getAnalysisUsage(AnalysisUsage &AU) const
{
  AU.addRequired<InputControllerPass>();
  AU.addRequired<OutputControllerPass>();
  AU.addPreserved<OutputControllerPass>();
  AU.addPreserved<InputControllerPass>();
}

VHDLOutputPass::VHDLOutputPass() : FunctionPass((intptr_t)&ID), 
				   inputComponent(NULL), 
				   outputComponent(NULL)
{
}

VHDLOutputPass::~VHDLOutputPass() 
{
}

std::vector<StreamVariable>::iterator VHDLOutputPass::streamBegin()
{
  return streams.begin();
}
std::vector<StreamVariable>::iterator VHDLOutputPass::streamEnd()
{
  return streams.end();
}

std::string getConstValuePrintableName(llvm::Value* V)
{
  static int num = 0;
  std::stringstream name;
  name << "CONST_VALUE" << num++;
  llvm::ConstantInt* ci = dynamic_cast<llvm::ConstantInt*>(V) ;
  ConstantFP* cf = dynamic_cast<ConstantFP*>(V) ;
  if( ci )
  {
    name << "_" << std::hex 
	 << ci->getValue().getSExtValue() << std::dec << "_I" ;
  }
  if( cf )
  {
    if( APFloat::semanticsPrecision(cf->getValueAPF().getSemantics()) < 32 )
    {
      float val = cf->getValueAPF().convertToFloat();
      name << "_" << std::hex << *reinterpret_cast<int*>(&val) 
	   << std::dec << "_F" ;
    }
    else
    {
      double val = cf->getValueAPF().convertToDouble();
      name << "_" << std::hex << *reinterpret_cast<int*>(&val) 
	   << std::dec << "_F" ;
    }
  }
  return name.str();
}

VHDLInterface::VWrap getLLVMValueToVHDLValue(llvm::Value* v, 
					     VHDLInterface::Entity* e)
{
  if( e->findSignal(v).size() > 0 )
  {
    return VHDLInterface::VWrap(e->findSignal(v).at(0));
  }
  else if (dynamic_cast<Constant*>(v) != NULL)
  {
    ConstantInt* ci = dynamic_cast<ConstantInt*>(v) ;
    ConstantFP* cf = dynamic_cast<ConstantFP*>(v) ;
    std::string name = getConstValuePrintableName(v);
    if (ci != NULL)
    {
      VHDLInterface::Signal* val = new VHDLInterface::NamedConstant(name, static_cast<int>(ci->getValue().getSExtValue()), ci);
      e->addSignal(val);
      return VHDLInterface::VWrap(val);
    }
    else if (cf != NULL)
    {
      if( APFloat::semanticsPrecision(cf->getValueAPF().getSemantics()) < 32 )
      {
        VHDLInterface::Signal* val = new VHDLInterface::NamedConstant(name,cf->getValueAPF().convertToFloat(), cf);
        e->addSignal(val);
        return VHDLInterface::VWrap(val);
      }
      else
      {
        VHDLInterface::Signal* val = new VHDLInterface::NamedConstant(name,(float)cf->getValueAPF().convertToDouble(), cf);
        e->addSignal(val);
        return VHDLInterface::VWrap(val);
      }
    }
    else
    {
      assert(0) ;
    }
  }
  return VHDLInterface::VWrap(NULL);
}

bool isLUTArray(llvm::Value* v)
{
  assert(v);
  for(Value::use_iterator UI = v->use_begin(); UI != v->use_end(); ++UI)
  {
    if( isROCCCFunctionCall(dynamic_cast<CallInst*>(*UI), ROCCCNames::InternalLUTDeclaration) )
    {
      return true;
    }
  }
  return false;
}

// I don't like this being global, but I'll find out where to put this
//  a little later.
static int numWatchpoints = 0 ;
std::list<VHDLInterface::Signal*> allWatchpoints ;
VHDLInterface::Signal* watchpointMerged = NULL ;
VHDLInterface::AssignmentStatement* watchedStatement = NULL ;

void VHDLOutputPass::setupVHDLInterface(DFFunction* f)
{
  entity = new VHDLInterface::Entity( f->getName() );
  inputComponent = NULL;
  outputComponent = NULL;
  //add all the signals from the valuetable
  ValueSymbolTable& allValues = f->getValueSymbolTable() ;
  llvm::ValueSymbolTable::iterator valIter = allValues.begin() ;
  while (valIter != allValues.end())
  {
    // Only output if it isnt a port, or a basic block, or a function
    if( dynamic_cast<BasicBlock*>((*valIter).getValue()) == NULL and
        dynamic_cast<Function*>((*valIter).getValue()) == NULL)
    {
      Value* v = (*valIter).getValue();
      //only add it if it isnt already a signal, and is being used, and isnt a LUT array
      if( entity->findSignal(v).size() == 0 and !v->use_empty() and !isLUTArray(v) )
      {
          VHDLInterface::Signal* s = entity->createSignal<VHDLInterface::Signal>( v->getName(), getSizeInBits(v), v );
          if( s->getSize() == 1 and !s->isUnsigned() )
          {
            LOG_MESSAGE2("VHDL Generation", "Datapath Generation", "Warning: signal " << s->getName() << " is generated as a 1-bit signed! This may lead to results different than the software!\n");
          }
      }
    }
    ++valIter ;
  }
  if( f->getFunctionType() == ROCCC::MODULE )
  { 
    //create the outputController
    outputComponent = entity->createComponent("outputController0", getAnalysis<OutputControllerPass>().outputEntity->getDeclaration());
    entity->mapPortToSubComponentPort(entity->getStandardPorts().clk, outputComponent, outputComponent->getDeclaration()->getStandardPorts().clk);
    entity->mapPortToSubComponentPort(entity->getStandardPorts().rst, outputComponent, outputComponent->getDeclaration()->getStandardPorts().rst);
    entity->mapPortToSubComponentPort(entity->getStandardPorts().stall, outputComponent, outputComponent->getDeclaration()->getStandardPorts().stall);
    VHDLInterface::Signal* has_had_input = entity->createSignal<VHDLInterface::Signal>("has_had_input", 1);
    VHDLInterface::AssignmentStatement* has_input_assign = entity->createSynchronousStatement(has_had_input);
    has_input_assign->addCase(VHDLInterface::ConstantInt::get(0), VHDLInterface::Wrap(entity->getStandardPorts().rst) == VHDLInterface::ConstantInt::get(1));
    has_input_assign->addCase(VHDLInterface::ConstantInt::get(1), VHDLInterface::EventCondition::rising_edge(entity->getStandardPorts().clk) and VHDLInterface::Wrap(entity->getStandardPorts().inputReady) == VHDLInterface::ConstantInt::get(1));
    has_input_assign->addCase(has_had_input);
    VHDLInterface::Variable* outputController_done = entity->getVariableMappedTo(outputComponent, outputComponent->getDeclaration()->getStandardPorts().done);
    // Modified on January 9, 2013 in order to handle watch points

    // New code    
    watchpointMerged = entity->createSignal<VHDLInterface::Signal>("watchpointMerged", 1);    
    watchedStatement = entity->createSynchronousStatement(watchpointMerged);
    watchedStatement->addCase(VHDLInterface::ConstantInt::get(0)) ;
    
    /*
    std::list<VHDLInterface::Signal*>::iterator watchpointIter ;
    watchpointIter = allWatchpoints.begin() ;
    while(watchpointIter != allWatchpoints.end())
    {
      // Or together all of the watchpoints 


      ++watchpointIter ;
    }
    */
    
    //    if (allWatchpoints.size() == 0)
    //   {
    //  entity->createSynchronousStatement(entity->getStandardPorts().done, VHDLInterface::Wrap(outputController_done) & VHDLInterface::Wrap(has_had_input));
    //    }
    // else
    //    {
      entity->createSynchronousStatement(entity->getStandardPorts().done, 
					 (VHDLInterface::Wrap(outputController_done) & VHDLInterface::Wrap(has_had_input)) | VHDLInterface::Wrap(watchpointMerged) );
      //    }

    // Original code
    //    entity->createSynchronousStatement(entity->getStandardPorts().done, VHDLInterface::Wrap(outputController_done) & VHDLInterface::Wrap(has_had_input));
    
    //create the inputController
    inputComponent = entity->createComponent("inputController0", getAnalysis<InputControllerPass>().inputEntity->getDeclaration());
    entity->mapPortToSubComponentPort(entity->getStandardPorts().clk, inputComponent, inputComponent->getDeclaration()->getStandardPorts().clk);
    entity->mapPortToSubComponentPort(entity->getStandardPorts().rst, inputComponent, inputComponent->getDeclaration()->getStandardPorts().rst);
    entity->createSynchronousStatement(entity->getVariableMappedTo(inputComponent, inputComponent->getDeclaration()->getStandardPorts().stall), VHDLInterface::Wrap(entity->getVariableMappedTo(outputComponent, getAnalysis<OutputControllerPass>().stall_internal)) | VHDLInterface::Wrap(entity->getVariableMappedTo(inputComponent, inputComponent->getDeclaration()->getStandardPorts().done)));
    entity->mapPortToSubComponentPort(entity->getStandardPorts().inputReady, inputComponent, inputComponent->getDeclaration()->getStandardPorts().inputReady);
    //connect the inputController ports
    CallInst* inputStreamOrderCall = NULL;
    CallInst* outputStreamOrderCall = NULL;
    for(Function::iterator BB = f->begin(); BB != f->end(); ++BB)
    {
      for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
      {
        if( isROCCCFunctionCall(dynamic_cast<CallInst*>(&*II), ROCCCNames::InputStream) )
        {
          assert(!inputStreamOrderCall and "More than one InputStreamOrder call detected!");
          inputStreamOrderCall = dynamic_cast<CallInst*>(&*II);
        }
        else if( isROCCCFunctionCall(dynamic_cast<CallInst*>(&*II), ROCCCNames::OutputStream) )
        {
          assert(!outputStreamOrderCall and "More than one OutputStreamOrder call detected!");
          outputStreamOrderCall = dynamic_cast<CallInst*>(&*II);
        }
      }
    }
    assert(inputStreamOrderCall and "Could not find InputStream ordering!");
    assert(outputStreamOrderCall and "Could not find OutputStream ordering!");
    for(unsigned OP = 1; OP < inputStreamOrderCall->getNumOperands(); ++OP)
    {
      Value* cur_input_stream = inputStreamOrderCall->getOperand(OP);
      bool found = false;
      for(std::vector<InputControllerPass::InputStream>::iterator ISI = getAnalysis<InputControllerPass>().input_streams.begin(); ISI != getAnalysis<InputControllerPass>().input_streams.end(); ++ISI)
      {
        if( ISI->stream->value == cur_input_stream )
        {
          StreamVariable sv(ISI->stream->readableName, ISI->stream->value);
          VHDLInterface::Port* cross_clk = dynamic_cast<VHDLInterface::Port*>(ISI->stream->cross_clk);
          assert(cross_clk and "StreamVariable elements must be ports!");
          sv.cross_clk = entity->addPortCopy(cross_clk, cross_clk->getType());
          entity->mapPortToSubComponentPort(dynamic_cast<VHDLInterface::Port*>(sv.cross_clk), inputComponent, cross_clk);
          VHDLInterface::Port* stop_access = dynamic_cast<VHDLInterface::Port*>(ISI->stream->stop_access);
          assert(stop_access and "StreamVariable elements must be ports!");
          sv.stop_access = entity->addPortCopy(ISI->stream->stop_access, stop_access->getType());
          entity->mapPortToSubComponentPort(dynamic_cast<VHDLInterface::Port*>(sv.stop_access), inputComponent, stop_access);
          VHDLInterface::Port* enable_access = dynamic_cast<VHDLInterface::Port*>(ISI->stream->enable_access);
          assert(enable_access and "StreamVariable elements must be ports!");
          sv.enable_access = entity->addPortCopy(ISI->stream->enable_access, enable_access->getType());
          entity->mapPortToSubComponentPort(dynamic_cast<VHDLInterface::Port*>(sv.enable_access), inputComponent, enable_access);
          for(std::vector<VHDLInterface::Variable*>::iterator DCI = ISI->stream->data_channels.begin(); DCI != ISI->stream->data_channels.end(); ++DCI)
          {
            VHDLInterface::Port* dci = dynamic_cast<VHDLInterface::Port*>(*DCI);
            assert(dci and "StreamVariable elements must be ports!");
            VHDLInterface::Port* in = entity->addPortCopy(dci, dci->getType());
            entity->mapPortToSubComponentPort(in, inputComponent, dci);
            sv.data_channels.push_back(in);
          }
          for(std::vector<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> >::iterator ACI = ISI->stream->address_channels.begin(); ACI != ISI->stream->address_channels.end(); ++ACI)
          {
            VHDLInterface::Port* aci_b = dynamic_cast<VHDLInterface::Port*>(ACI->first);
            assert(aci_b and "StreamVariable elements must be ports!");
            VHDLInterface::Port* aci_c = dynamic_cast<VHDLInterface::Port*>(ACI->second);
            assert(aci_c and "StreamVariable elements must be ports!");
            VHDLInterface::Port* ia_b = entity->addPortCopy(aci_b, aci_b->getType());
            entity->mapPortToSubComponentPort(ia_b, inputComponent, aci_b);
            VHDLInterface::Port* ia_c = entity->addPortCopy(aci_c, aci_c->getType());
            entity->mapPortToSubComponentPort(ia_c, inputComponent, aci_c);
            sv.address_channels.push_back(std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*>(ia_b, ia_c));
          }
          VHDLInterface::Port* address_clk = dynamic_cast<VHDLInterface::Port*>(ISI->stream->address_clk);
          assert(address_clk and "StreamVariable elements must be ports!");
          sv.address_clk = entity->addPortCopy(address_clk, address_clk->getType());
          entity->mapPortToSubComponentPort(dynamic_cast<VHDLInterface::Port*>(sv.address_clk), inputComponent, address_clk);
          VHDLInterface::Port* address_rdy = dynamic_cast<VHDLInterface::Port*>(ISI->stream->address_rdy);
          assert(address_rdy and "StreamVariable elements must be ports!");
          sv.address_rdy = entity->addPortCopy(address_rdy, address_rdy->getType());
          entity->mapPortToSubComponentPort(dynamic_cast<VHDLInterface::Port*>(sv.address_rdy), inputComponent, address_rdy);
          if( ISI->stream->address_stall )
          {
            VHDLInterface::Port* address_stall = dynamic_cast<VHDLInterface::Port*>(ISI->stream->address_stall);
            assert(address_stall and "StreamVariable elements must be ports!");
            sv.address_stall = entity->addPortCopy(address_stall, address_stall->getType());
            entity->mapPortToSubComponentPort(dynamic_cast<VHDLInterface::Port*>(sv.address_stall), inputComponent, address_stall);
          }
          streams.push_back(sv);
          found = true;
        }
      }
      if( !found )
      {
        INTERNAL_ERROR("Stream " << cur_input_stream->getName() << " is listed in stream order call, but input streams are as follows:\n");
        for(std::vector<InputControllerPass::InputStream>::iterator ISI = getAnalysis<InputControllerPass>().input_streams.begin(); ISI != getAnalysis<InputControllerPass>().input_streams.end(); ++ISI)
        {
          if( ISI->stream->value )
          {
            INTERNAL_ERROR("     " << ISI->stream->value->getName() << "\n");
          }
          else
            INTERNAL_ERROR("     NULL\n");
        }
      }
      assert( found and "Could not find stream listed in stream order call!" );
    }
    //connect the outputController ports
    for(unsigned OP = 1; OP < outputStreamOrderCall->getNumOperands(); ++OP)
    {
      Value* cur_output_stream = outputStreamOrderCall->getOperand(OP);
      bool found = false;
      for(std::vector<OutputControllerPass::OutputStream>::iterator OSI = getAnalysis<OutputControllerPass>().output_streams.begin(); OSI != getAnalysis<OutputControllerPass>().output_streams.end(); ++OSI)
      {
        if( OSI->stream->value == cur_output_stream )
        {
          StreamVariable sv(OSI->stream->readableName, OSI->stream->value);
          VHDLInterface::Port* cross_clk = dynamic_cast<VHDLInterface::Port*>(OSI->stream->cross_clk);
          assert(cross_clk and "StreamVariable elements must be ports!");
          sv.cross_clk = entity->addPortCopy(cross_clk, cross_clk->getType());
          entity->mapPortToSubComponentPort(dynamic_cast<VHDLInterface::Port*>(sv.cross_clk), outputComponent, cross_clk);
          VHDLInterface::Port* stop_access = dynamic_cast<VHDLInterface::Port*>(OSI->stream->stop_access);
          assert(stop_access and "StreamVariable elements must be ports!");
          sv.stop_access = entity->addPortCopy(OSI->stream->stop_access, stop_access->getType());
          entity->mapPortToSubComponentPort(dynamic_cast<VHDLInterface::Port*>(sv.stop_access), outputComponent, stop_access);
          VHDLInterface::Port* enable_access = dynamic_cast<VHDLInterface::Port*>(OSI->stream->enable_access);
          assert(enable_access and "StreamVariable elements must be ports!");
          sv.enable_access = entity->addPortCopy(OSI->stream->enable_access, enable_access->getType());
          entity->mapPortToSubComponentPort(dynamic_cast<VHDLInterface::Port*>(sv.enable_access), outputComponent, enable_access);
          for(std::vector<VHDLInterface::Variable*>::iterator DCI = OSI->stream->data_channels.begin(); DCI != OSI->stream->data_channels.end(); ++DCI)
          {
            VHDLInterface::Port* dci = dynamic_cast<VHDLInterface::Port*>(*DCI);
            assert(dci and "StreamVariable elements must be ports!");
            VHDLInterface::Port* in = entity->addPortCopy(dci, dci->getType());
            entity->mapPortToSubComponentPort(in, outputComponent, dci);
            sv.data_channels.push_back(in);
          }
          for(std::vector<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> >::iterator ACI = OSI->stream->address_channels.begin(); ACI != OSI->stream->address_channels.end(); ++ACI)
          {
            VHDLInterface::Port* aci_b = dynamic_cast<VHDLInterface::Port*>(ACI->first);
            assert(aci_b and "StreamVariable elements must be ports!");
            VHDLInterface::Port* aci_c = dynamic_cast<VHDLInterface::Port*>(ACI->second);
            assert(aci_c and "StreamVariable elements must be ports!");
            VHDLInterface::Port* ia_b = entity->addPortCopy(aci_b, aci_b->getType());
            entity->mapPortToSubComponentPort(ia_b, outputComponent, aci_b);
            VHDLInterface::Port* ia_c = entity->addPortCopy(aci_c, aci_c->getType());
            entity->mapPortToSubComponentPort(ia_c, outputComponent, aci_c);
            sv.address_channels.push_back(std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*>(ia_b, ia_c));
          }
          VHDLInterface::Port* address_clk = dynamic_cast<VHDLInterface::Port*>(OSI->stream->address_clk);
          assert(address_clk and "StreamVariable elements must be ports!");
          sv.address_clk = entity->addPortCopy(address_clk, address_clk->getType());
          entity->mapPortToSubComponentPort(dynamic_cast<VHDLInterface::Port*>(sv.address_clk), outputComponent, address_clk);
          VHDLInterface::Port* address_rdy = dynamic_cast<VHDLInterface::Port*>(OSI->stream->address_rdy);
          assert(address_rdy and "StreamVariable elements must be ports!");
          sv.address_rdy = entity->addPortCopy(address_rdy, address_rdy->getType());
          entity->mapPortToSubComponentPort(dynamic_cast<VHDLInterface::Port*>(sv.address_rdy), outputComponent, address_rdy);
          if( OSI->stream->address_stall )
          {
            VHDLInterface::Port* address_stall = dynamic_cast<VHDLInterface::Port*>(OSI->stream->address_stall);
            assert(address_stall and "StreamVariable elements must be ports!");
            sv.address_stall = entity->addPortCopy(address_stall, address_stall->getType());
            entity->mapPortToSubComponentPort(dynamic_cast<VHDLInterface::Port*>(sv.address_stall), outputComponent, address_stall);
          }
          streams.push_back(sv);
          found = true;
        }
      }
      if( !found )
      {
        INTERNAL_ERROR("Stream " << cur_output_stream->getName() << " is listed in stream order call, but output streams are as follows:\n");
        for(std::vector<OutputControllerPass::OutputStream>::iterator OSI = getAnalysis<OutputControllerPass>().output_streams.begin(); OSI != getAnalysis<OutputControllerPass>().output_streams.end(); ++OSI)
        {
          if( OSI->stream->value )
          {
            INTERNAL_ERROR("     " << OSI->stream->value->getName() << "\n");
          }
          else
            INTERNAL_ERROR("     NULL\n");
        }
      }
      assert( found and "Could not find stream listed in stream order call!" );
    }
  }
}

void VHDLOutputPass::finalizeVHDLInterface(DFFunction* f)
{
  if( f->getFunctionType() == ROCCC::MODULE )
  {     
    for(std::vector<VHDLInterface::Port*>::iterator PI = outputComponent->getDeclaration()->getPorts().begin(); PI != outputComponent->getDeclaration()->getPorts().end(); ++PI)
    {
      if( (*PI)->getLLVMValue() == NULL or outputComponent->isMapped(*PI) )
        continue;
      if( (*PI)->getType() == VHDLInterface::Port::OUTPUT )
      {
        if( entity->getPort( (*PI)->getLLVMValue() ).size() == 1 )
        {
          if( !entity->getPort( (*PI)->getLLVMValue() ).at(0)->isOwned() )
          {
            entity->mapPortToSubComponentPort(entity->getPort( (*PI)->getLLVMValue() ).at(0), outputComponent, *PI);
            continue;
          }
        }
        if( entity->findSignal( (*PI)->getLLVMValue() ).size() == 1 )
        {
          if( !entity->findSignal( (*PI)->getLLVMValue() ).at(0)->isOwned() )
          {
            outputComponent->map(entity->findSignal( (*PI)->getLLVMValue() ).at(0), *PI);
            continue;
          }
        }
      }
      else
      {
        if( entity->getPort( (*PI)->getLLVMValue() ).size() == 1 )
        {
          entity->mapPortToSubComponentPort(entity->getPort( (*PI)->getLLVMValue() ).at(0), outputComponent, *PI);
          continue;
        }
        if( entity->findSignal( (*PI)->getLLVMValue() ).size() == 1 )
        {
          outputComponent->map(entity->findSignal( (*PI)->getLLVMValue() ).at(0), *PI);
          continue;
        }
      }
      INTERNAL_WARNING("Could not map " << outputComponent->getName() << "->" << (*PI)->getName() << "!(llvm::value " << (*PI)->getLLVMValue()->getName() << ")\n");
    }
    for(std::vector<VHDLInterface::Port*>::iterator PI = inputComponent->getDeclaration()->getPorts().begin(); PI != inputComponent->getDeclaration()->getPorts().end(); ++PI)
    {
      if( (*PI)->getLLVMValue() == NULL or inputComponent->isMapped(*PI) )
        continue;
      if( entity->getPort( (*PI)->getLLVMValue() ).size() == 1 )
      {
        if( !entity->getPort( (*PI)->getLLVMValue() ).at(0)->isOwned() )
        {
          entity->mapPortToSubComponentPort(entity->getPort( (*PI)->getLLVMValue() ).at(0), inputComponent, *PI);
          continue;
        }
      }
      if( entity->findSignal( (*PI)->getLLVMValue() ).size() == 1 )
      {
        if( !entity->findSignal( (*PI)->getLLVMValue() ).at(0)->isOwned() )
        {
          inputComponent->map(entity->findSignal( (*PI)->getLLVMValue() ).at(0), *PI);
          continue;
        }
      }
      INTERNAL_WARNING("Could not map " << inputComponent->getName() << "->" << (*PI)->getName() << "!\n");
    }
  }
}

// This is the entry point to our pass and where all of our work gets done
bool VHDLOutputPass::runOnFunction(Function& f)
{
  CurrentFile::set(__FILE__);
  //resetSizeMap();
  
  if (f.isDeclaration() || f.getDFFunction() == NULL)
  {
    return false ; // No modification and nothing to do.
  }
  DFFunction* df = f.getDFFunction() ;
  
  if (df->getFunctionType() == ROCCC::BLOCK)
  {
    OutputBlock(df) ;
  }
  else if (df->getFunctionType() == ROCCC::MODULE)
  {
    OutputModule(df) ;
  }
  else
  {
    llvm::cerr << "Unsupported function type." << std::endl ;
    assert(0) ;
  }
  // Nothing has changed in the internal representation, so return false
  return false ; 
}

void VHDLOutputPass::OutputBlock(DFFunction* f)
{
  // Write out a vhdl file the exact same way we output a system

  OutputModule(f) ;
}

int getActualPipelineLevel(DFBasicBlock* currentNode)
{
  assert(currentNode);
  int level = currentNode->getPipelineLevel() + currentNode->getDelay() - 1;
  if( CallInst* CI = dynamic_cast<CallInst*>(currentNode->getFirstNonPHI()) )
  {
    if( isROCCCFunctionCall(CI, ROCCCNames::InvokeHardware) )
    {
      level += 1;
    }
  }
  return level;
}

class VHDLArray : public VHDLInterface::Signal {
  class ArrayElement : public VHDLInterface::Variable {
    std::string name;
    VHDLInterface::Value* index;
    VHDLArray* parent;
  public:
    ArrayElement(std::string n, int s, VHDLInterface::Value* i, VHDLArray* p) : Variable(s, p->getLLVMValue()), name(n), index(i), parent(p)
    {
      assert(i);
      assert(p);
    }
    virtual std::string getInternalName() //from Value
    {
      return name;
    }
    virtual std::string generateCode(int size)
    {
      std::stringstream ss;
      ss << getName() << "(conv_integer(" << index->generateCode(parent->getAddressSize()) << "))";
      return ss.str();
    }
    virtual void setReadFrom()
    {
      index->setReadFrom();
      parent->setReadFrom();
    }
    virtual void setWrittenTo()
    {
      index->setReadFrom();
      parent->setWrittenTo();
    }
  };
  std::vector<int> values;
public:
  VHDLArray(std::string n, int size, Variable::T* v=NULL) : VHDLInterface::Signal(n,size,v)
  {
  }
  virtual std::string generateCode(int size) //from Value
  {
    assert(0 and "Cannot generate a ConstArray!");
  }
  virtual std::string generateDeclarationCode() //from Signal
  {
    Variable::declare();
    std::stringstream ss;
    assert( values.size() > 0 );
    ss << "type " << getName() << "_type is array (0 to " << values.size()-1 << ") of std_logic_vector(" << getSize()-1 << " downto 0) ;\n";
    ss << "signal " << getName() << " : " << getName() << "_type := (\n";
    int count = 0;
    for(std::vector<int>::iterator DI = values.begin(); DI != values.end(); ++DI, ++count)
    {
       if( DI != values.begin() )
         ss << ",\n";
       ss << "    conv_std_logic_vector(" << *DI << ", " << getSize() << ")";
    }
    ss << ") ;\n";
    return ss.str();
  }
  virtual VHDLInterface::Value* generateResetValue() //from Variable
  {
    assert(0 and "Cannot reset a ConstArray!");
  }
  VHDLInterface::Variable* getElement(VHDLInterface::Value* v)
  {
    return new ArrayElement(getName(), getSize(), v, this);
  }
  void addElement(int v)
  {
    values.push_back(v);
  }
  int getNumElements()
  {
    return values.size();
  }
  int getAddressSize()
  {
    return std::ceil(std::log(getNumElements()) / std::log(2));
  }
};

VHDLArray* GetMemoryTable(llvm::Value* memory, VHDLInterface::Entity* entity)
{
  assert(memory);
  if( entity->findSignal(memory).size() == 0 )
  {
    VHDLArray* ret = entity->createSignal<VHDLArray>(memory->getName(), getSizeInBits(memory), memory);
    VHDLInterface::Attribute* ramstyle = entity->getAttribute("syn_ramstyle");
    if( !ramstyle )
      ramstyle = entity->addAttribute(new VHDLInterface::VHDLAttribute("syn_ramstyle", "string"));
    ret->addAttribute(ramstyle, "\"block_ram\"");
    for(Value::use_iterator UI = memory->use_begin(); UI != memory->use_end(); ++UI)
    {
      if( isROCCCFunctionCall(dynamic_cast<CallInst*>(*UI), ROCCCNames::InternalLUTDeclaration) )
      {
        CallInst* CI = dynamic_cast<CallInst*>(*UI);
        assert(CI);
        assert(CI->getNumOperands() >= 3);
        ConstantInt* numElem_c = dynamic_cast<ConstantInt*>(CI->getOperand(2)) ;
        assert(numElem_c);
        unsigned int numElems = numElem_c->getValue().getSExtValue();
        for(unsigned int OP = 3; OP < CI->getNumOperands(); ++OP)
        {
          ConstantInt* elem = dynamic_cast<ConstantInt*>(CI->getOperand(OP)) ;
          assert(elem);
          ret->addElement(elem->getValue().getSExtValue());
        }
        for(unsigned int OP = ret->getNumElements(); OP < numElems; ++OP)
        {
          ret->addElement(0);
        }
        for(unsigned int OP = ret->getNumElements(); OP < pow(2, std::ceil(std::log(ret->getNumElements()) / std::log(2))); ++OP)
        {
          ret->addElement(0);
        }
        return ret;
      }
    }
  }
  return dynamic_cast<VHDLArray*>(entity->findSignal(memory).at(0));
}


void createMemoryRead(llvm::Value* memory, VHDLInterface::Entity* entity)
{

}
void createMemoryWrite(llvm::Value* memory, VHDLInterface::Entity* entity)
{

}

llvm::Value* getDefaultLoadPreviousInitValue(llvm::Value* V)
{
  if( !V )
    return NULL;
  for(llvm::Value::use_iterator UI = V->use_begin(); UI != V->use_end(); ++UI)
  {
    if(CallInst* CI = dynamic_cast<CallInst*>(*UI))
    {
      if( isROCCCFunctionCall(CI, ROCCCNames::LoadPreviousInitValue) )
      {
        assert( CI->getNumOperands() == 3 and "LoadPreviousInitValue() needs 2 arguments!" );
        assert( CI->getOperand(1) == V and "First argument to LoadPreviousInitValue() must be register, not constant!" );
        return CI->getOperand(2);
      }
    }
  }
  return NULL;
}

void createLUTExternalWrite(VHDLArray* array, VHDLInterface::Entity* parent)
{
  //also add a process to handle the writing of the memory, which necessitates tons of new ports
  VHDLInterface::Port* write_clk = parent->addPort(array->getName() + "_wclk", 1, VHDLInterface::Port::INPUT);
  VHDLInterface::Port* write_wr = parent->addPort(array->getName() + "_wr", 1, VHDLInterface::Port::INPUT);
  VHDLInterface::Port* write_addr = parent->addPort(array->getName() + "_waddr", array->getAddressSize(), VHDLInterface::Port::INPUT);
  VHDLInterface::Port* write_data = parent->addPort(array->getName() + "_wdata", array->getSize(), VHDLInterface::Port::INPUT);
  //set the readable names of the newly created ports to the name of the LUT
  VHDLInterface::Attribute* readable_name = parent->getAttribute("readable_name");
  if( !readable_name )
    readable_name = parent->addAttribute(new VHDLInterface::Attribute("readable_name"));
  write_clk->addAttribute(readable_name, array->getName());
  write_wr->addAttribute(readable_name, array->getName());
  write_addr->addAttribute(readable_name, array->getName());
  write_data->addAttribute(readable_name, array->getName());
  //set the port type of the newly created ports
  VHDLInterface::Attribute* port_type = parent->getAttribute("port_type");
  if( !port_type )
    port_type = parent->addAttribute(new VHDLInterface::Attribute("port_type"));
  write_clk->addAttribute(port_type, "LUT_CROSS_CLK");
  write_wr->addAttribute(port_type, "LUT_ENABLE_ACCESS");
  write_addr->addAttribute(port_type, "LUT_ADDRESS");
  write_data->addAttribute(port_type, "LUT_CHANNEL");
  //create the new process
  VHDLInterface::MultiStatementProcess* p2 = parent->createProcess<VHDLInterface::MultiStatementProcess>(write_clk);
  p2->setHasReset(false);
  //if write enable, write to the memory location mem[addr] <= data
  p2->addStatement(new VHDLInterface::IfStatement(p2, VHDLInterface::Wrap(write_wr) == VHDLInterface::ConstantInt::get(1), new VHDLInterface::AssignmentStatement(array->getElement(write_addr), write_data, p2)));
}

class VHDLProcess : public VHDLInterface::Process {
  // Instructions for outputting individual instructions
  VHDLInterface::Statement* ProcessInstructions(DFBasicBlock* b) ;
  VHDLInterface::Statement* ProcessInstruction(Instruction* i) ;
  VHDLInterface::Statement* ProcessCallInstruction(CallInst* i) ;
  VHDLInterface::Statement* ProcessUnaryInstruction(UnaryInstruction* i) ;
  VHDLInterface::Statement* ProcessCmpInstruction(CmpInst* i) ;
  VHDLInterface::Statement* ProcessBinaryInstruction(BinaryOperator* i) ;
  VHDLInterface::Statement* ProcessZeroExtendInstruction(ZExtInst* i) ;
  VHDLInterface::Statement* ProcessSignExtendInstruction(SExtInst* i) ;
  // For printing out individual variable names
  VHDLInterface::VWrap LLVMValueToVHDLValue(Value* v) ;
  VHDLInterface::MultiStatement* ms;
  std::vector<VHDLInterface::Variable*> trigger_variables;
  std::vector<VHDLInterface::Variable*> load_variables;
  VHDLInterface::ComponentDefinition* inputComponent;
  VHDLInterface::ComponentDefinition* outputComponent;
  VHDLInterface::Condition* stall_condition;
  VHDLInterface::Port* outputComponent_stall_internal;
  VHDLInterface::Variable* internal_stall;
  VHDLInterface::Signal* activeStates;
  VHDLInterface::Wrap getLevelTriggerValue(DFFunction* f, int level)
  {
    VHDLInterface::Wrap trigger(NULL);
    if( level > activeStates->getSize() )
    {
      INTERNAL_WARNING("Level is greater than activeStates!\n");
      trigger = VHDLInterface::Wrap(getParent()->getStandardPorts().inputReady);
    }
    else if( level == activeStates->getSize() )
    {
      if ( f->getFunctionType() == ROCCC::MODULE )
        trigger = VHDLInterface::Wrap(getParent()->getVariableMappedTo(inputComponent, inputComponent->getDeclaration()->getStandardPorts().outputReady));
      else
        trigger = VHDLInterface::Wrap(getParent()->getStandardPorts().inputReady);
    }
    else
    {
      if( activeStates->getSize() > 1 )
        trigger = VHDLInterface::BitRange::get(activeStates, level, level);
      else
        trigger = VHDLInterface::Wrap(activeStates);
    }
    assert( trigger != NULL );
    return trigger;
  }
  VHDLInterface::CWrap getConditionForPipelineStage(DFFunction* f, int level)
  {
    VHDLInterface::Wrap trigger = getLevelTriggerValue(f, level);
    return (trigger == VHDLInterface::ConstantInt::get(1) and VHDLInterface::CWrap(stall_condition));
  }
  VHDLInterface::Statement* getConditionalLevelStatement(DFFunction* f, int level, VHDLInterface::Statement* inst)
  {
    return new VHDLInterface::IfStatement(this, getConditionForPipelineStage(f,level), inst);
  }
public:
  VHDLProcess(VHDLInterface::Entity* e) : Process(e), ms(NULL), stall_condition(NULL), outputComponent_stall_internal(NULL), internal_stall(NULL), activeStates(NULL) {}
  void initialize(DFFunction* f, VHDLInterface::ComponentDefinition* ic, VHDLInterface::ComponentDefinition* oc, VHDLInterface::Port* ocsi)
  {
    ms = new VHDLInterface::MultiStatement(this);
    inputComponent = ic;
    outputComponent = oc;
    outputComponent_stall_internal = ocsi;
    
    VHDLInterface::Signal* stall_previous = getParent()->createSignal<VHDLInterface::Signal>("stall_previous", 1, NULL); //FIXME: should the stall previous actually be a shift buffer, or is it correctly only stalling on the and of the previous and current stall?
    if( outputComponent )
    {
      internal_stall = getParent()->getVariableMappedTo(outputComponent, outputComponent_stall_internal);
    }
    else
    {
      internal_stall = getParent()->getStandardPorts().stall;
    }
    ms->addStatement(new VHDLInterface::AssignmentStatement(stall_previous, internal_stall, this));
    stall_condition = (VHDLInterface::Wrap(internal_stall) != VHDLInterface::ConstantInt::get(1)) or (VHDLInterface::Wrap(stall_previous) == VHDLInterface::ConstantInt::get(0));
    
    trigger_variables.push_back(getParent()->getStandardPorts().outputReady);
    ms->addStatement(new VHDLInterface::CommentStatement(this, "BEGIN ACTIVESTATES SHIFT"));
    assert(f->getDelay() > 0 and "Error with graph. Are your outputs connected to your inputs?");
    activeStates = getParent()->createSignal<VHDLInterface::Signal>("activeStates", f->getDelay(), NULL);
    VHDLInterface::Value* input_ready = NULL;
    VHDLInterface::AssignmentStatement* active_states_shift = new VHDLInterface::AssignmentStatement(activeStates, this);
    if ( f->getFunctionType() == ROCCC::MODULE )
      input_ready = getParent()->getVariableMappedTo(inputComponent, inputComponent->getDeclaration()->getStandardPorts().outputReady);
    else
      input_ready = getParent()->getStandardPorts().inputReady;
    if( activeStates->getSize() > 1 )
      active_states_shift->addCase(bitwise_concat(input_ready, VHDLInterface::BitRange::get(activeStates, activeStates->getSize()-1, 1)), stall_condition);
    else
      active_states_shift->addCase(input_ready, stall_condition);
    ms->addStatement(active_states_shift);
    ms->addStatement(new VHDLInterface::CommentStatement(this, "END ACTIVESTATES SHIFT"));
    //This is TERRIBAD
    //add the init_inputscalars - this is done here because the input_scalar call may not connect to the
    //  sink, for example when the input scalars are only used as loop ending conditions, and they will
    //  will not normally be connected in that situation.
    //also add the systolic nexts, these suffer the same problem that the input scalars do
    //also also add the block that contains the induction variables
    //also, process the InputStreams and OutputStreams call instructions
    for(Function::iterator BB = f->begin(); BB != f->end(); ++BB)
    {
      DFBasicBlock* dfbb = BB->getDFBasicBlock();
      assert(dfbb);
      if( dfbb->isMarked() )
        continue;
      for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
      {
        CallInst* CI = dynamic_cast<CallInst*>(&*II);
        if( dynamic_cast<PHINode*>(&*II) )
        {
          dfbb->mark();
          VHDLInterface::Statement* inst = ProcessInstruction(&*II) ;
          if( inst != NULL )
          {
            int level = getActualPipelineLevel(dfbb);
            if( level == -1 )
              level = 1;
            ms->addStatement(getConditionalLevelStatement(f, level, inst));
          }
        }
        else if( isROCCCFunctionCall(CI, ROCCCNames::InputScalar) )
        {
          dfbb->mark();
          VHDLInterface::Statement* inst = ProcessCallInstruction(CI) ;
          if( inst != NULL )
          {
            ms->addStatement(getConditionalLevelStatement(f, f->getDelay(), inst));
          }
        }
        else if( isROCCCFunctionCall(CI, ROCCCNames::DebugScalarOutput) )
        {
          dfbb->mark();
          VHDLInterface::Statement* inst = ProcessCallInstruction(CI) ;
          if( inst != NULL )
          {
            assert( "DebugScalarOutput should not create vhdl statement inside process!" );
          }
        }
        else if( isROCCCFunctionCall(CI, ROCCCNames::SystolicNext) )
        {
          dfbb->mark();
          int level = -1;
          assert( CI->getNumOperands() == 3 and "Incorrect number of arguments to storeNext!" );
          Instruction* defIncInst = getDefinitionInstruction(dynamic_cast<Instruction*>(CI->getOperand(2)), CI->getParent());
          if( defIncInst )
          {
            level = defIncInst->getParent()->getDFBasicBlock()->getPipelineLevel()-1;
          }
          if( level < 0 )
          {
            INTERNAL_ERROR("Systolic next has no effect!\n");
            assert( CI->getNumOperands() == 3 and "Incorrect number of arguments to storeNext!" );
            VHDLInterface::Wrap lhs = LLVMValueToVHDLValue(CI->getOperand(1));
            ms->addStatement(new VHDLInterface::AssignmentStatement(lhs, VHDLInterface::ConstantInt::get(0), this));
          }
          else
          {
            INTERNAL_MESSAGE("Adding " << *CI << " with level " << level << "\n");
            VHDLInterface::Statement* inst = ProcessCallInstruction(CI) ;
            ms->addStatement(getConditionalLevelStatement(f, level, inst));
          }          
        }
        else if( isROCCCFunctionCall(CI, ROCCCNames::LUTRead) )
        {
          assert( CI->getNumOperands() == 3+1 and "Incorrect number of arguments to LUTRead!" );
          //create a memory that this LUTRead call will be accessing
          llvm::Value* lutArray = CI->getOperand(1);
          VHDLArray* array = GetMemoryTable(lutArray, getParent());
          assert(array);
          //create a process that reads from the memory
          VHDLInterface::MultiStatementProcess* p = getParent()->createProcess<VHDLInterface::MultiStatementProcess>();
          p->setHasReset(false);
          VHDLInterface::Wrap lhs = LLVMValueToVHDLValue(CI);
          VHDLInterface::Wrap addr = LLVMValueToVHDLValue(CI->getOperand(2));
          //add the assignment val <= mem[address] to the new process, with the pipeline level condition of course!
          VHDLInterface::AssignmentStatement* assign = new VHDLInterface::AssignmentStatement(lhs, array->getElement(addr), p);
          int level = getActualPipelineLevel(dfbb);
            if( level < 0 )
            {
              p->addStatement(new VHDLInterface::CommentStatement(p, "Level went negative; setting to 1."));
              level = 1;
            }
          p->addStatement(new VHDLInterface::IfStatement(p, getConditionForPipelineStage(f,level), assign));
        }
        else if( isROCCCFunctionCall(CI, ROCCCNames::LUTWrite) )
        {
          assert( CI->getNumOperands() == 4+1 and "Incorrect number of arguments to LUTWrite!" );
          //create a memory that this LUTWrite call will be accessing
          llvm::Value* lutArray = CI->getOperand(1);
          VHDLArray* array = GetMemoryTable(lutArray, getParent());
          assert(array);
          //create a process that writes to the memory
          VHDLInterface::MultiStatementProcess* p = getParent()->createProcess<VHDLInterface::MultiStatementProcess>();
          p->setHasReset(false);
          VHDLInterface::Wrap addr = LLVMValueToVHDLValue(CI->getOperand(2));
          VHDLInterface::Wrap rhs = LLVMValueToVHDLValue(CI->getOperand(3));
          //add the assignment mem[addr] <= rhs to the new process, with the pipeline level condition of course!
          VHDLInterface::AssignmentStatement* assign = new VHDLInterface::AssignmentStatement(array->getElement(addr), rhs, p);
          int level = -1;
          Instruction* addrInst = getDefinitionInstruction(dynamic_cast<Instruction*>(CI->getOperand(2)), CI->getParent());
          Instruction* datInst = getDefinitionInstruction(dynamic_cast<Instruction*>(CI->getOperand(3)), CI->getParent());
          if( addrInst or datInst )
            level = 100000;
          if( addrInst )
          {
            level = std::min(level, getActualPipelineLevel(addrInst->getParent()->getDFBasicBlock())-1);
          }
          if( datInst )
          {
            level = std::min(level, getActualPipelineLevel(datInst->getParent()->getDFBasicBlock())-1);
          }
          if( level < 0 )
            {
              p->addStatement(new VHDLInterface::CommentStatement(p, "Level went negative; setting to 1."));
              level = 1;
            }
          p->addStatement(new VHDLInterface::IfStatement(p, getConditionForPipelineStage(f,level), assign));
        }
      }
    }
    std::list<DFBasicBlock*> toPrint ;
    toPrint.push_back(f->getSink()) ;
    while (!toPrint.empty())
    {
      DFBasicBlock* currentNode = toPrint.front() ;
      toPrint.pop_front() ;
      if (currentNode->isMarked())
      {
        continue ;
      }
      currentNode->mark() ;
      if( currentNode->getPipelineLevel() <= f->getDelay() and
          currentNode->getPipelineLevel() >= 0 )
      {
        VHDLInterface::Statement* inst = ProcessInstructions(currentNode);
        if( inst != NULL )
        {
          int level = getActualPipelineLevel(currentNode);
          std::stringstream level_name;
          level_name << currentNode->getName() << "_" << level;
          currentNode->setName(level_name.str());
          ms->addStatement(getConditionalLevelStatement(f, level, inst));
        }
      }
      // Get all of the others
      pred_iterator findPreds = pred_begin(currentNode) ;
      while (findPreds != pred_end(currentNode))
      {
        DFBasicBlock* nextPred = (*findPreds)->getDFBasicBlock() ;
        assert(nextPred != NULL) ;
        if (!nextPred->isMarked())
        {
          toPrint.push_back(nextPred) ;
        }
        ++findPreds ;
      }
    }
    //finally, we might have datapath elements that use the loop induction variables; these need
    //  to be registered at the same time the inputs to the datapath are ready, ie at inputReady
    std::vector<Value*> indexes = f->getROCCCLoopInfo().indexes;
    for(std::vector<Value*>::iterator LIV = indexes.begin(); LIV != indexes.end(); ++LIV)
    {
      assert( *LIV );
      VHDLInterface::Variable* port = getParent()->getVariableMappedTo(inputComponent, inputComponent->getDeclaration()->getPort(*LIV).at(0));
      ms->addStatement(getConditionalLevelStatement(f, activeStates->getSize(), new VHDLInterface::AssignmentStatement(getParent()->findSignal(*LIV).at(0), port, this)));
    }
  }
  std::string generateSteadyState(int level)
  {
    assert( ms and "Must call initialize()!" );
    VHDLInterface::MultiStatement* reset_statement = new VHDLInterface::MultiStatement(this);
    for(std::vector<VHDLInterface::Variable*>::iterator TSI = trigger_variables.begin(); TSI != trigger_variables.end(); ++TSI)
    {
      reset_statement->addStatement( new VHDLInterface::AssignmentStatement(*TSI, (*TSI)->generateResetValue(), this) );
    }
    VHDLInterface::MultiStatement* load_statements = new VHDLInterface::MultiStatement(this);
    for(std::vector<VHDLInterface::Variable*>::iterator TSI = load_variables.begin(); TSI != load_variables.end(); ++TSI)
    {
      load_statements->addStatement( new VHDLInterface::AssignmentStatement(*TSI, (*TSI)->generateResetValue(), this) );
    }
    std::stringstream ss;
    if( inputComponent )
    {
      reset_statement->addStatement(new VHDLInterface::IfStatement(this, VHDLInterface::Wrap(getParent()->getVariableMappedTo(inputComponent, inputComponent->getDeclaration()->getStandardPorts().outputReady)) == VHDLInterface::ConstantInt::get(1),load_statements));
    }
    ss << reset_statement->generateCode(level);
    ss << ms->generateCode(level);
    return ss.str();
  }
};

void VHDLOutputPass::OutputModule(DFFunction* f)
{
  //output the generated entity
  char buff[1024];
  if( !getcwd(buff, 1024) )
  {
    llvm::cout << "Could not get current directory!\n";
    assert(0);
    exit(0);
  }
  std::string outputDirectory(buff);
  outputDirectory += "/vhdl/";
  std::string outputFileName ; 
  outputFileName = f->getName() ;
  outputFileName += ".vhdl" ;
  std::string outputFullPath = outputDirectory + outputFileName;
  std::ofstream fout;
  fout.open(outputFullPath.c_str(), std::ios_base::out) ;
  if (!fout)
  {
    llvm::cerr << "Cannot open output file:" << outputFullPath << std::endl ;
    assert(0) ;
  }
  llvm::cout << "Writing " << (f->getFunctionType() ? "module" : "system") << " to \'" << outputFullPath << "\'\n";
  Database::FileInfoInterface::addFileInfo(Database::getCurrentID(), Database::FileInfo(outputFileName, Database::FileInfo::VHDL_SOURCE, outputDirectory));
  LOG_MESSAGE2("VHDL Generation", "Datapath", "Datapath written to <a href=\'" << outputFullPath << "\'>" << outputFileName << "</a>.\n");

  setupVHDLInterface(f);
  VHDLProcess* p = entity->createProcess<VHDLProcess>();
  p->initialize(f, inputComponent, outputComponent, getAnalysis<OutputControllerPass>().stall_internal);
  finalizeVHDLInterface(f);
  fout << entity->generateCode();
  fout.close() ;
}

std::vector<llvm::Instruction*> getUsedVars(Instruction* i)
{
  assert(i);
  std::map<llvm::Value*, bool> allVarsUsedMap;
  for(User::op_iterator OP = i->op_begin(); OP != i->op_end(); ++OP)
  {
    allVarsUsedMap[OP->get()] = true;
  }
  std::vector<llvm::Instruction*> allVarsUsed;
  for(std::map<llvm::Value*, bool>::iterator MI = allVarsUsedMap.begin(); MI != allVarsUsedMap.end(); ++MI)
  {
    Instruction* inst = dynamic_cast<Instruction*>(MI->first);
    if( inst )
    {
      if( MI->second == true and !isDefinition(i, inst) )
        allVarsUsed.push_back( inst );
    }
  }
  return allVarsUsed;
}
std::vector<llvm::Instruction*> getLiveInVars(Instruction* i)
{
  assert(i);
  int curLevel = i->getParent()->getDFBasicBlock()->getPipelineLevel();
  std::vector<llvm::Instruction*> allVarsUsed = getUsedVars(i);
  std::vector<llvm::Instruction*> liveInVars;
  for(std::vector<llvm::Instruction*>::iterator MI = allVarsUsed.begin(); MI != allVarsUsed.end(); ++MI)
  {
    Instruction* inst = dynamic_cast<Instruction*>(*MI);
    if( inst )
    {
      Instruction* definition = getDefinitionInstruction(inst, i->getParent());
      int definitionLevel = definition->getParent()->getDFBasicBlock()->getPipelineLevel();
      if( curLevel < definitionLevel and !isDefinition(i, inst) )
        liveInVars.push_back( inst );
    }
  }
  return liveInVars;
}

VHDLInterface::Statement* VHDLProcess::ProcessInstruction(Instruction* i) 
{
  assert(i);
  std::vector<llvm::Instruction*> allVarsUsed = getUsedVars(i);
  std::vector<llvm::Instruction*> liveInVars = getLiveInVars(i);
  VHDLInterface::MultiStatement* ms = new VHDLInterface::MultiStatement(this);
  if (CallInst* CI = dynamic_cast<CallInst*>(i))
  {
    if ( !isROCCCFunctionCall(CI, ROCCCNames::BoolSelect) )
    {
      if(isROCCCInputStream(CI))
      { 
      }
      else if(isROCCCFunctionCall(CI, ROCCCNames::LoadPrevious))
      {
      }
      else if(isROCCCFunctionCall(CI, ROCCCNames::SystolicPrevious))
      {
      }
      else if(isROCCCFunctionCall(CI, ROCCCNames::FeedbackScalar))
      {
      }
      else if(isROCCCFunctionCall(CI, ROCCCNames::LUTRead))
      {
      }
      else if(isROCCCFunctionCall(CI, ROCCCNames::LUTWrite))
      {
      }
      else if(isROCCCFunctionCall(CI, ROCCCNames::InternalLUTDeclaration))
      {
      }
      else if(isROCCCFunctionCall(CI, ROCCCNames::LUTOrder))
      {
      }
      else if(liveInVars.size() != allVarsUsed.size())
      {
        INTERNAL_ERROR("Call instruction " << *i << " has non live-in operands!\n");
        INTERNAL_MESSAGE("Used vars:\n");
        for(std::vector<llvm::Instruction*>::iterator UVI = allVarsUsed.begin(); UVI != allVarsUsed.end(); ++UVI)
        {
          INTERNAL_MESSAGE(**UVI);
        }
        INTERNAL_MESSAGE("Live-in vars:\n");
        for(std::vector<llvm::Instruction*>::iterator LVI = liveInVars.begin(); LVI != liveInVars.end(); ++LVI)
        {
          INTERNAL_MESSAGE(**LVI);
        }
        assert( 0 and "Call inst cannot have non live-in vars!") ;
      }
      return ProcessCallInstruction(dynamic_cast<CallInst*>(i));
    }
  }
  for(std::vector<llvm::Instruction*>::iterator VI = liveInVars.begin(); VI != liveInVars.end(); ++VI)
  {
    std::string copyName = (*VI)->getName() + "_registered";
    llvm::Instruction* load = NULL;
    if( getParent()->findSignal(copyName).size() > 0 )
    {
      load = dynamic_cast<llvm::Instruction*>(getParent()->findSignal(copyName).at(0)->getLLVMValue());
    }
    else
    {
      load = dynamic_cast<Instruction*>(getCopyOfValue(*VI));
      load->setName(copyName);
      DFBasicBlock::Create("", i->getParent()->getParent())->addInstruction(load);
      load->getParent()->getDFBasicBlock()->setPipelineLevel((*VI)->getParent()->getDFBasicBlock()->getPipelineLevel());
      load->getParent()->getDFBasicBlock()->setDataflowLevel((*VI)->getParent()->getDFBasicBlock()->getPipelineLevel());
    }
    i->replaceUsesOfWith(*VI, load);
    
    VHDLInterface::Value* loadSignal = LLVMValueToVHDLValue(load);
    if( loadSignal == NULL )
      loadSignal = getParent()->createSignal<VHDLInterface::Signal>(load->getName(), getSizeInBits(load), load);
    if( !getParent()->getAttribute("syn_keep") )
      getParent()->addAttribute(new VHDLInterface::VHDLAttribute("syn_keep", "boolean"));
    if(dynamic_cast<VHDLInterface::Variable*>(loadSignal))
      dynamic_cast<VHDLInterface::Variable*>(loadSignal)->addAttribute(getParent()->getAttribute("syn_keep"), "true");
    ms->addStatement(new VHDLInterface::AssignmentStatement(loadSignal, LLVMValueToVHDLValue(*VI), this));
  }
  if (CallInst* CI = dynamic_cast<CallInst*>(i))
  {
    if ( isROCCCFunctionCall(CI, ROCCCNames::BoolSelect) )
    {
      assert( CI->getNumOperands() == 4 and "Incorrect number of arguments to BoolSelect!" );
      VHDLInterface::Wrap lhs = LLVMValueToVHDLValue(CI);
      VHDLInterface::Wrap true_val = LLVMValueToVHDLValue(CI->getOperand(1));
      VHDLInterface::Wrap false_val = LLVMValueToVHDLValue(CI->getOperand(2));
      VHDLInterface::Wrap cond = LLVMValueToVHDLValue(CI->getOperand(3));
      VHDLInterface::AssignmentStatement* ret = getParent()->createSynchronousStatement(lhs);
      ret->addCase(true_val, cond == VHDLInterface::ConstantInt::get(1));
      ret->addCase(false_val);
    }
    else
    {
      assert(0 and "Should have handled non boolSelect calls at this point!");
    }
  }
  else if (dynamic_cast<UnaryInstruction*>(i) != NULL)
  {
    VHDLInterface::Statement* s = ProcessUnaryInstruction(dynamic_cast<UnaryInstruction*>(i));
    if( s )
      ms->addStatement( s );
  }
  else if (dynamic_cast<CmpInst*>(i) != NULL)
  {
    VHDLInterface::Statement* s = ProcessCmpInstruction(dynamic_cast<CmpInst*>(i));
    if( s )
      ms->addStatement( s );
  }
  else if (dynamic_cast<BinaryOperator*>(i) != NULL)
  {
    VHDLInterface::Statement* s = ProcessBinaryInstruction(dynamic_cast<BinaryOperator*>(i));
    if( s )
      ms->addStatement( s );
  }
  else if (dynamic_cast<PHINode*>(i) != NULL)
  {
  }
  else
  {
    std::stringstream ss;
    ss << *i;
    ms->addStatement(new VHDLInterface::CommentStatement(this, ss.str()));
  }
  if( ms->isEmpty() )
    return NULL;
  return ms;
}

VHDLInterface::Statement* VHDLProcess::ProcessZeroExtendInstruction(ZExtInst* i)
{
  getParent()->createSynchronousStatement(LLVMValueToVHDLValue(i), LLVMValueToVHDLValue(i->getOperand(0)));
  return NULL;
}

// Currently, sign extension is the same as zero extension
//  This is only correct in half of the cases.
VHDLInterface::Statement* VHDLProcess::ProcessSignExtendInstruction(SExtInst* i)
{
  getParent()->createSynchronousStatement(LLVMValueToVHDLValue(i), LLVMValueToVHDLValue(i->getOperand(0)));
  return NULL;
}

class LibraryComponentOwner : public VHDLInterface::ValueOwner {
public:
  virtual std::string getName(); //from ValueOwner
};
std::string LibraryComponentOwner::getName() //from ValueOwner
{
  return "LIBRARY_COMPONENT_OWNER";
}
//TODO: rewrite this using DatabaseInterface::getEntry(std::string)
VHDLInterface::ComponentDeclaration* getDeclarationForComponent(std::string componentName, CallInst* CI, VHDLInterface::Entity* e, std::string instanceName)
{
  DatabaseInterface* coreLibrary = DatabaseInterface::getInstance();
  std::list<LibraryEntry*>::iterator libIter = coreLibrary->getCores().begin() ;
  while (libIter != coreLibrary->getCores().end())
  {
    if (componentName == (*libIter)->getName())
    {
      VHDLInterface::ComponentDeclaration* cd = new VHDLInterface::ComponentDeclaration((*libIter)->getName());
      std::list<Database::Port*> p = (*libIter)->getAllPorts();
      unsigned int i = 2;
      for( std::list<Database::Port*>::iterator PI = p.begin(); PI != p.end(); ++PI)
      {
        llvm::Value* v = NULL;
        if( (*PI)->isDebug() )
        {
          int size = (*PI)->getBitSize();
          Value* ptr = new AllocaInst(IntegerType::get(size), 0, "", CI->getParent()->getTerminator());
          Value* result = new LoadInst(ptr, componentName + "_" + instanceName + "_" + (*PI)->getName(), CI->getParent()->getTerminator());
          VHDLInterface::Signal* s = e->createSignal<VHDLInterface::Signal>(result->getName(), getSizeInBits(result), result);
          VHDLInterface::Port* p = e->addPort(result->getName(), getSizeInBits(result), VHDLInterface::Port::OUTPUT, result);
          //make sure we set the attribute output_type
          VHDLInterface::Attribute* port_type = e->getAttribute("port_type");
          if( !port_type )
            port_type = e->addAttribute(new VHDLInterface::Attribute("port_type"));
          p->addAttribute(port_type, "DEBUG");
          e->createSynchronousStatement(p, s);
          v = result;
          Function* debugFunc = CI->getParent()->getParent()->getParent()->getFunction(ROCCCNames::DebugScalarOutput);
          if( !debugFunc )
          {
            std::vector<const Type*> paramTypes;
            paramTypes.push_back( v->getType() );
            debugFunc = Function::Create(FunctionType::get(Type::VoidTy, paramTypes, false),
            			   (GlobalValue::LinkageTypes)0,
            			   ROCCCNames::DebugScalarOutput, 
            			   CI->getParent()->getParent()->getParent() );
          }
        	//now create the actual call instruction
        	DFBasicBlock* newBB = DFBasicBlock::Create("debugBB", CI->getParent()->getParent());
        	CI->getParent()->getDFBasicBlock()->AddUse(newBB);
          std::vector<Value*> valArgs;
          valArgs.push_back(v);
        	CallInst::Create( debugFunc,
                            valArgs.begin(),
                  			    valArgs.end(),
                   			    "" ,
                  			    newBB->getTerminator());
        }
        else
        {
          assert( i < CI->getNumOperands() && "Mismatched number of arguments!" );
          v = CI->getOperand(i);
          ++i;
        }
        cd->addPort( (*PI)->getName(), (*PI)->getBitSize(), (*PI)->isOutput()?VHDLInterface::Port::OUTPUT:VHDLInterface::Port::INPUT, v, false );
      }
      for(std::vector<VHDLInterface::Port*>::iterator PI = cd->getPorts().begin(); PI != cd->getPorts().end(); ++PI)
      {
        static LibraryComponentOwner LIBRARY_COMPONENT_OWNER;
        LIBRARY_COMPONENT_OWNER.add(*PI);
      }
      return cd;
    }
  ++libIter ;
  }
  assert(libIter != coreLibrary->getCores().end()) ;
  return NULL;
}

//TODO: this could search for an "allow retiming" function and return that value,
//          or use a configuration flag, or something similar
bool ROCCCAllowRetiming()
{
  return false;
}

// Call instructions must be treated specially, because of all the ROCCC
//  intrinsics that we are trying to output.
// Note: This function should be called only when we are outputting a
//  pipeline stage.
VHDLInterface::Statement* VHDLProcess::ProcessCallInstruction(CallInst* CI)
{
  VHDLInterface::Attribute* allow_retiming = getParent()->getAttribute("syn_allow_retiming");
  if( !allow_retiming and ROCCCAllowRetiming() )
  {
    allow_retiming = getParent()->addAttribute(new VHDLInterface::VHDLAttribute("syn_allow_retiming", "boolean"));
  }
  if( isROCCCFunctionCall(CI, ROCCCNames::InvokeHardware) )
  {
    static int u = 0;
    std::string componentName = getComponentNameFromCallInst( CI );    
    std::stringstream name;
    name << "U" << u++;
    VHDLInterface::ComponentDeclaration* cd = getDeclarationForComponent(componentName, CI, getParent(), name.str());
    
    VHDLInterface::ComponentDefinition* cde = getParent()->createComponent(name.str(), cd);
    cde->map(getParent()->getStandardPorts().clk, cd->getStandardPorts().clk);
    cde->map(getParent()->getStandardPorts().rst, cd->getStandardPorts().rst);
    cde->map(internal_stall, cd->getStandardPorts().stall);
    VHDLInterface::Signal* load = getParent()->createSignal<VHDLInterface::Signal>(name.str() + "_load", 1);
    cde->map(load, cd->getStandardPorts().inputReady);
    trigger_variables.push_back(load);
    DatabaseInterface* coreLibrary = DatabaseInterface::getInstance();
    for(std::vector<VHDLInterface::Port*>::iterator PI = cd->getPorts().begin(); PI != cd->getPorts().end(); ++PI)
    {
      if( (*PI)->getLLVMValue() != NULL )
      {
        VHDLInterface::Variable* val = LLVMValueToVHDLValue((*PI)->getLLVMValue());
        if( dynamic_cast<VHDLInterface::NamedConstant*>(val) )
          dynamic_cast<VHDLInterface::NamedConstant*>(val)->resize((*PI)->getSize());
        if( val->getSize() != (*PI)->getSize() )
        {
          if( coreLibrary->LookupEntry(componentName).getType() != LibraryEntry::INT_DIV and
              coreLibrary->LookupEntry(componentName).getType() != LibraryEntry::INT_TO_FP and
              coreLibrary->LookupEntry(componentName).getType() != LibraryEntry::FP_TO_INT and 
              val->getSize() < (*PI)->getSize() )
          {
            INTERNAL_ERROR("Cannot map value " << val->getName() << " of size " << val->getSize() << " to port " << cd->getName() << "_" << (*PI)->getName() << " of size " << (*PI)->getSize() << " because mapped values must be larger than or equal to the port's size!\n");
            assert(0 and "Incorrectly mapping a value to a larger port!");
          }
          else
          {
            llvm::Value* extend_llvm_val = getCopyOfValue(val->getLLVMValue());
            DFBasicBlock::Create("", CI->getParent()->getParent())->addInstruction(dynamic_cast<Instruction*>(extend_llvm_val)); 
            VHDLInterface::Signal* extend = getParent()->createSignal<VHDLInterface::Signal>( name.str() + "_" + (*PI)->getName() + "_resize", (*PI)->getSize(), extend_llvm_val);
            if( (*PI)->getType() == VHDLInterface::Port::INPUT )
              getParent()->createSynchronousStatement(extend, val);
            else
              getParent()->createSynchronousStatement(val, extend);
            val = extend;
          }
        }
        cde->map(val, *PI);
      }
    }
    return new VHDLInterface::AssignmentStatement(load, VHDLInterface::ConstantInt::get(1), this);
  }
  else if (isROCCCFunctionCall(CI, ROCCCNames::InputScalar))
  {
    VHDLInterface::MultiStatement* ms = new VHDLInterface::MultiStatement(this);
    for(unsigned int j = 1 ; j < CI->getNumOperands() ; ++j)
    {
      Value* v = CI->getOperand(j);
      assert( v );
      VHDLInterface::Port* p = getParent()->addPort(getValueName(v), getSizeInBits(v), VHDLInterface::Port::INPUT, v);
      if( allow_retiming )
        p->addAttribute(allow_retiming, "false");
      if( allow_retiming )
        getParent()->findSignal(v).at(0)->addAttribute(allow_retiming, "false");
      ms->addStatement(new VHDLInterface::AssignmentStatement(getParent()->findSignal(v).at(0), p, this));
    }
    return ms;
  }  
  else if (isROCCCFunctionCall(CI, ROCCCNames::OutputScalar))
  {
    VHDLInterface::MultiStatement* ms = new VHDLInterface::MultiStatement(this);
    if(  outputComponent ) //its a system
    {
      ms->addStatement(new VHDLInterface::AssignmentStatement(getParent()->getStandardPorts().outputReady, VHDLInterface::ConstantInt::get(1), this));
      trigger_variables.push_back(getParent()->getVariableMappedTo(outputComponent, outputComponent->getDeclaration()->getStandardPorts().inputReady));
      ms->addStatement(new VHDLInterface::AssignmentStatement(getParent()->getVariableMappedTo(outputComponent, outputComponent->getDeclaration()->getStandardPorts().inputReady), VHDLInterface::ConstantInt::get(1), this));
    }
    else //its a module
    {
      ms->addStatement(new VHDLInterface::AssignmentStatement(getParent()->getStandardPorts().outputReady, VHDLInterface::ConstantInt::get(1), this));
      ms->addStatement(new VHDLInterface::AssignmentStatement(getParent()->getSignalMappedToPort(getParent()->getStandardPorts().done), VHDLInterface::ConstantInt::get(1), this));
    }
    for(unsigned int j = 1 ; j < CI->getNumOperands() ; ++j)
    {
      Value* v = CI->getOperand(j);
      assert( v );
      VHDLInterface::Port* p = getParent()->addPort(getValueName(v), getSizeInBits(v), VHDLInterface::Port::OUTPUT, v);
      if( allow_retiming )
        p->addAttribute(allow_retiming, "false");
      if( LLVMValueToVHDLValue(v) == NULL )
      {
        INTERNAL_ERROR("Cannot find signal for " << *v << "\n");
      }
      assert( LLVMValueToVHDLValue(v) != NULL and "Fatal error, signal not found! Could it have gotten removed due to constant propogation?" );
      ms->addStatement(new VHDLInterface::AssignmentStatement(p, LLVMValueToVHDLValue(v), this));
      if(VHDLInterface::Signal* s = dynamic_cast<VHDLInterface::Signal*>(&*LLVMValueToVHDLValue(v)) )
      {
        if( allow_retiming )
          s->addAttribute(allow_retiming, "false");
      }
    }
    return ms;
  }
  else if (isROCCCFunctionCall(CI, ROCCCNames::DebugScalarOutput))
  {
    Value* v = CI->getOperand(1);
    assert( v );
    VHDLInterface::Port* p = getParent()->addPort(getValueName(v) + "_debug", getSizeInBits(v), VHDLInterface::Port::OUTPUT, v);
    if( allow_retiming )
      p->addAttribute(allow_retiming, "false");
    VHDLInterface::Attribute* port_type = getParent()->getAttribute("port_type");
    if( !port_type )
      port_type = getParent()->addAttribute(new VHDLInterface::Attribute("port_type"));
    p->addAttribute(port_type, "DEBUG");
    if( LLVMValueToVHDLValue(v) == NULL )
    {
      INTERNAL_ERROR("Cannot find signal for " << *v << "\n");
    }
    assert( LLVMValueToVHDLValue(v) != NULL and "Fatal error, signal not found! Could it have gotten removed due to constant propogation?" );
    
    // This is what makes the Asynchronous statement that connects the 
    //  debug port to the actual port...
    getParent()->createSynchronousStatement(p, LLVMValueToVHDLValue(v));

    // Check if there is a watchpoint associated with this
    Value* validValue = CI->getOperand(3) ;
    assert(validValue != NULL) ;
    llvm::ConstantInt* validInt = dynamic_cast<llvm::ConstantInt*>(validValue);
    if (validInt != NULL && validInt->equalsInt(1))
    {
      // If there is, add it
      std::stringstream watchPointStream ;
      watchPointStream << "watchPointHit" << numWatchpoints ;
      VHDLInterface::Signal* watchpointHit = 
	getParent()->createSignal<VHDLInterface::Signal>(watchPointStream.str(), 1); 
      VHDLInterface::AssignmentStatement* watchpointStmt = 
	getParent()->createSynchronousStatement(watchpointHit);

      Value* compareValue = CI->getOperand(2) ;
      assert(compareValue != NULL) ;
      llvm::ConstantInt* validInt = 
	dynamic_cast<llvm::ConstantInt*>(compareValue) ;
      assert(validInt != NULL) ;
      
      // watchpointHit = '1' when (variable = value)
      watchpointStmt->addCase(VHDLInterface::ConstantInt::get(1),
			      LLVMValueToVHDLValue(v) == 
			      VHDLInterface::ConstantInt::get(validInt->getValue().getLimitedValue())) ;
      
      // else '0'
      watchpointStmt->addCase(VHDLInterface::ConstantInt::get(0)) ;
      ++numWatchpoints ;

      // Remove the last case (which should be the default) and replace
      //  with two more
      watchedStatement->removeLastCase() ;      
      watchedStatement->addCase(VHDLInterface::ConstantInt::get(1),
				VHDLInterface::Wrap(watchpointHit) == VHDLInterface::ConstantInt::get(1)) ;
      watchedStatement->addCase(VHDLInterface::ConstantInt::get(0)) ;

      allWatchpoints.push_back(watchpointHit) ;
    }
    
    return NULL;
  }
  else if ( isROCCCOutputStream(CI) )
  {
    VHDLInterface::MultiStatement* ms = new VHDLInterface::MultiStatement(this);
    int dim = getROCCCStreamDimension(CI);
    //the call instruction is of the form:
    //ROCCC_output_fifoN(BUFF_NAME, counter1, . . . counterN,
    //                   output1, index1, . . . indexN,
    //                   . . .
    //                   outputZ, index1, . . . indexN );
    // where N is the dimension, and Z can be any value.
    for(unsigned int j = 1+dim+1 ; j+dim < CI->getNumOperands() ; j+=dim+1)
    {
      Value* v = CI->getOperand(j);
      assert( v );
      if( outputComponent->getDeclaration()->getPort(v).size() <= 0 )
      {
        INTERNAL_ERROR(*v);
      }
      assert( outputComponent->getDeclaration()->getPort(v).size() > 0 );
      VHDLInterface::Variable* p = getParent()->getVariableMappedTo(outputComponent, outputComponent->getDeclaration()->getPort(v).at(0));
      ms->addStatement(new VHDLInterface::AssignmentStatement(p, getParent()->findSignal(v).at(0), this));
    }
    trigger_variables.push_back(getParent()->getVariableMappedTo(outputComponent, outputComponent->getDeclaration()->getStandardPorts().inputReady));
    ms->addStatement(new VHDLInterface::AssignmentStatement(getParent()->getVariableMappedTo(outputComponent, outputComponent->getDeclaration()->getStandardPorts().inputReady), VHDLInterface::ConstantInt::get(1), this));
    return ms;
  }
  else if ( isROCCCInputStream(CI) )
  {
    VHDLInterface::MultiStatement* ms = new VHDLInterface::MultiStatement(this);
    int dim = getROCCCStreamDimension(CI);
    //the call instruction is of the form:
    //ROCCC_input_fifoN(BUFF_NAME, counter1, . . . counterN,
    //                   output1, index1, . . . indexN,
    //                   . . .
    //                   outputZ, index1, . . . indexN );
    // where N is the dimension, and Z can be any value.
    for(unsigned int j = 1+dim+1 ; j+dim < CI->getNumOperands() ; j+=dim+1)
    {
      Value* v = CI->getOperand(j);
      assert( v );
      VHDLInterface::Variable* p = getParent()->getVariableMappedTo(inputComponent, inputComponent->getDeclaration()->getPort(v).at(0));
      ms->addStatement(new VHDLInterface::AssignmentStatement(getParent()->findSignal(v).at(0), p, this));
    }
    return ms;    
  }
  else if ( isROCCCFunctionCall(CI, ROCCCNames::BoolSelect) )
  {
    assert( CI->getNumOperands() == 4 and "Incorrect number of arguments to BoolSelect!" );
    VHDLInterface::Wrap lhs = LLVMValueToVHDLValue(CI);
    VHDLInterface::Wrap true_val = LLVMValueToVHDLValue(CI->getOperand(1));
    VHDLInterface::Wrap false_val = LLVMValueToVHDLValue(CI->getOperand(2));
    VHDLInterface::Wrap cond = LLVMValueToVHDLValue(CI->getOperand(3));
    VHDLInterface::AssignmentStatement* ret = new VHDLInterface::AssignmentStatement(lhs, this);
    ret->addCase(true_val, cond == VHDLInterface::ConstantInt::get(1));
    ret->addCase(false_val);
    return ret;
  }
  else if ( isROCCCFunctionCall(CI, ROCCCNames::SystolicNext) or
            isROCCCFunctionCall(CI, ROCCCNames::StoreNext) )
  {
    assert( CI->getNumOperands() == 3 and "Incorrect number of arguments to storeNext!" );
    VHDLInterface::Wrap lhs = LLVMValueToVHDLValue(CI->getOperand(1));
    VHDLInterface::Wrap rhs = LLVMValueToVHDLValue(CI->getOperand(2));
    VHDLInterface::Signal* load = getParent()->createSignal<VHDLInterface::Signal>(lhs.val->getName() + "_load", 1);
    trigger_variables.push_back(load);
    VHDLInterface::MultiStatement* ms = new VHDLInterface::MultiStatement(this);
    ms->addStatement(new VHDLInterface::AssignmentStatement(lhs, rhs, this));
    ms->addStatement(new VHDLInterface::AssignmentStatement(load, VHDLInterface::ConstantInt::get(1), this));
    return ms;
  }
  else if ( isROCCCFunctionCall(CI, ROCCCNames::SystolicPrevious) )
  {
    assert( CI->getNumOperands() == 3 and "Incorrect number of arguments to SystolicPrevious!" );
    //set up the on_first_value
    VHDLInterface::Signal* is_initialized = getParent()->createSignal<VHDLInterface::Signal>(CI->getOperand(2)->getName()+"_is_initialized", 1);
    //set up the initialization value
    llvm::Value* defaultInitVal = getDefaultLoadPreviousInitValue(CI->getOperand(2));
    VHDLInterface::Wrap init_val(NULL);
    if( defaultInitVal )
    {
      init_val = LLVMValueToVHDLValue(defaultInitVal);
    }
    else
    {
      INTERNAL_WARNING("No initial value specified for " << getValueName(CI->getOperand(2)) << ", generating input port for initial value!\n");
      VHDLInterface::Port* init = getParent()->getDeclaration()->addPort(getValueName(CI->getOperand(1))+"_init", getSizeInBits(CI->getOperand(1)), VHDLInterface::Port::INPUT, CI->getOperand(1));
      VHDLInterface::Attribute* port_name = getParent()->getAttribute("readable_name");
      if( !port_name )
        port_name = getParent()->addAttribute(new VHDLInterface::Attribute("readable_name"));
      init->addAttribute(port_name, init->getInternalName());
      init_val = VHDLInterface::Wrap(init);
    }
    //create the assignment statement
    std::stringstream ss;
    ss << *CI;
    VHDLInterface::MultiStatement* ms = new VHDLInterface::MultiStatement(this);
    ms->addStatement(new VHDLInterface::CommentStatement(this, ss.str()));
    VHDLInterface::Wrap lhs = LLVMValueToVHDLValue(CI->getOperand(1));
    VHDLInterface::Wrap rhs = LLVMValueToVHDLValue(CI->getOperand(2));
    VHDLInterface::AssignmentStatement* assign = new VHDLInterface::AssignmentStatement(lhs, this);
    assign->addCase(init_val, VHDLInterface::Wrap(is_initialized) == VHDLInterface::ConstantInt::get(0));
    assign->addCase(rhs);
    ms->addStatement(assign);
    ms->addStatement(new VHDLInterface::AssignmentStatement(is_initialized, VHDLInterface::ConstantInt::get(1), this));
    return ms;
  }
  else if ( isROCCCFunctionCall(CI, ROCCCNames::SummationFeedback) )
  {
    assert( CI->getNumOperands() == 3 and "Incorrect number of arguments to SummationFeedback!" );
    //set up the on_first_value
    VHDLInterface::Signal* is_initialized = getParent()->createSignal<VHDLInterface::Signal>(CI->getOperand(1)->getName()+"_is_initialized", 1);
    //set up the initialization value
    llvm::Value* defaultInitVal = getDefaultLoadPreviousInitValue(CI->getOperand(1));
    VHDLInterface::Wrap init_val(NULL);
    if( defaultInitVal )
    {
      init_val = LLVMValueToVHDLValue(defaultInitVal);
    }
    else
    {
      INTERNAL_WARNING("No initial value specified for " << getValueName(CI->getOperand(1)) << ", generating input port for initial value!\n");
      VHDLInterface::Port* init = getParent()->getDeclaration()->addPort(getValueName(CI->getOperand(1))+"_init", getSizeInBits(CI->getOperand(1)), VHDLInterface::Port::INPUT, CI->getOperand(1));
      VHDLInterface::Attribute* port_name = getParent()->getAttribute("readable_name");
      if( !port_name )
        port_name = getParent()->addAttribute(new VHDLInterface::Attribute("readable_name"));
      init->addAttribute(port_name, init->getInternalName());
      init_val = VHDLInterface::Wrap(init);
    }
    //create the assignment statement
    std::stringstream ss;
    ss << *CI;
    VHDLInterface::MultiStatement* ms = new VHDLInterface::MultiStatement(this);
    ms->addStatement(new VHDLInterface::CommentStatement(this, ss.str()));
    VHDLInterface::Wrap lhs = LLVMValueToVHDLValue(CI->getOperand(1));
    VHDLInterface::Wrap rhs = LLVMValueToVHDLValue(CI->getOperand(2));
    VHDLInterface::AssignmentStatement* assign = new VHDLInterface::AssignmentStatement(lhs, this);
    assign->addCase(init_val+rhs, VHDLInterface::Wrap(is_initialized) == VHDLInterface::ConstantInt::get(0));
    assign->addCase(lhs + rhs);
    ms->addStatement(assign);
    ms->addStatement(new VHDLInterface::AssignmentStatement(is_initialized, VHDLInterface::ConstantInt::get(1), this));
    return ms;
  }
  else if ( isROCCCFunctionCall(CI, ROCCCNames::LUTRead) )
  {
   //do nothing, because we already handled it in the setup phase
  }
  else if ( isROCCCFunctionCall(CI, ROCCCNames::LUTWrite) )
  {
   //do nothing, because we already handled it in the setup phase
  }
  else
  {
    INTERNAL_WARNING(*CI << " is not handled!\n");
  }
  std::stringstream ss;
  ss << *CI;
  return new VHDLInterface::CommentStatement(this, ss.str());
}

VHDLInterface::Statement* VHDLProcess::ProcessUnaryInstruction(UnaryInstruction* i)
{
  if (dynamic_cast<ZExtInst*>(i) != NULL)
  {
    return ProcessZeroExtendInstruction(dynamic_cast<ZExtInst*>(i)) ;
  }
  if (dynamic_cast<SExtInst*>(i) != NULL)
  {
    return ProcessSignExtendInstruction(dynamic_cast<SExtInst*>(i)) ;
  }
  if (dynamic_cast<BitCastInst*>(i) != NULL or
      dynamic_cast<TruncInst*>(i) != NULL)
  {  
    assert(LLVMValueToVHDLValue(i));
    assert(LLVMValueToVHDLValue(i->getOperand(0)));
    getParent()->createSynchronousStatement(LLVMValueToVHDLValue(i), LLVMValueToVHDLValue(i->getOperand(0)));
    return NULL;
  }
  std::stringstream ss;
  ss << *i;
  return new VHDLInterface::CommentStatement(this, ss.str());
}

VHDLInterface::Statement* VHDLProcess::ProcessCmpInstruction(CmpInst* i) 
{
  VHDLInterface::Wrap lhs = LLVMValueToVHDLValue(i->getOperand(0));
  VHDLInterface::Wrap rhs = LLVMValueToVHDLValue(i->getOperand(1));
  VHDLInterface::Condition* comp;
  switch (i->getPredicate())
  {
  case ICmpInst::ICMP_EQ: // equal
    {
      comp = (lhs == rhs);
    }
    break ;
  case ICmpInst::ICMP_NE: // Not equal
    {
      comp = (lhs != rhs);
    }
    break ;
  case ICmpInst::ICMP_UGT: // Greater than (unsigned)
    {
      comp = (lhs > rhs);
    }
    break ;
  case ICmpInst::ICMP_UGE: // Greater than or equal (unsigned)
    {
      comp = (lhs >= rhs);
    }
    break ;
  case ICmpInst::ICMP_ULT: // Less than (unsigned)
    {
      comp = (lhs < rhs);
    }    
    break ;
  case ICmpInst::ICMP_ULE: // Less than or equal (unsigned)
    {
      comp = (lhs <= rhs);
    }
    break ;
  case ICmpInst::ICMP_SGT: // Greater than (signed)
    {
      comp = SGT(lhs, rhs);
    }
    break ;
  case ICmpInst::ICMP_SGE: // Greater than or equal (signed)
    {
      comp = SGTE(lhs, rhs);
    }
    break ;
  case ICmpInst::ICMP_SLT: // Less than (signed)
    {
      comp = SLT(lhs, rhs);
    }
    break ;
  case ICmpInst::ICMP_SLE: // Less than or equal (signed)
    {
      comp = SLTE(lhs, rhs);
    }
    break ;
  default:
    {
      INTERNAL_ERROR("Unknown comparison " << *i);
      assert( 0 and "Unknown operator!" );
    }
  }
  VHDLInterface::AssignmentStatement* ret = getParent()->createSynchronousStatement(LLVMValueToVHDLValue(i));
  ret->addCase(VHDLInterface::ConstantInt::get(1), comp);
  ret->addCase(VHDLInterface::ConstantInt::get(0));
  //return ret;
  return NULL;
}

VHDLInterface::Statement* VHDLProcess::ProcessBinaryInstruction(BinaryOperator* i)
{
  VHDLInterface::AssignmentStatement* ret = getParent()->createSynchronousStatement( LLVMValueToVHDLValue(i));
  VHDLInterface::Wrap lhs = LLVMValueToVHDLValue(i->getOperand(0));
  VHDLInterface::Wrap rhs = LLVMValueToVHDLValue(i->getOperand(1));
  if( isCopyValue(i) )
  {
    ret->addCase( lhs );
    return NULL;
  }
  switch(i->getOpcode())
  {
    case BinaryOperator::Add:
    {
 	    ret->addCase( lhs + rhs );
    }
    break ;
    case BinaryOperator::Sub:
    {
  	  ret->addCase( lhs - rhs );
    }
    break ;
    case BinaryOperator::Mul:
    {
  	  ret->addCase( lhs * rhs );
    }
    break;
    case BinaryOperator::Shl:  // Shift left
    {
      ConstantInt* ci = dynamic_cast<ConstantInt*>(i->getOperand(1));
      assert( ci and "Shift left must have a constant shift value!" );
      int rhs_value = static_cast<int>(ci->getValue().getSExtValue());
      VHDLInterface::Variable* vlhs = dynamic_cast<VHDLInterface::Variable*>(lhs.val);
      assert( vlhs and "Shift left can only be done on a variable value!" );
      ret->addCase( VHDLInterface::shift_left_logical(vlhs, rhs_value) );
    }
    break ;
    case BinaryOperator::LShr: // Shift right logical
    {
      ConstantInt* ci = dynamic_cast<ConstantInt*>(i->getOperand(1));
      assert( ci and "Shift right must have a constant shift value!" );
      int rhs_value = static_cast<int>(ci->getValue().getSExtValue());
      VHDLInterface::Variable* vlhs = dynamic_cast<VHDLInterface::Variable*>(lhs.val);
      assert( vlhs and "Shift right can only be done on a variable value!" );
      ret->addCase( VHDLInterface::shift_right_logical(vlhs, rhs_value) );
    }
    break ;
    case BinaryOperator::AShr: // Shift right arithmetic
    {
      ConstantInt* ci = dynamic_cast<ConstantInt*>(i->getOperand(1));
      assert( ci and "Shift right must have a constant shift value!" );
      int rhs_value = static_cast<int>(ci->getValue().getSExtValue());
      VHDLInterface::Variable* vlhs = dynamic_cast<VHDLInterface::Variable*>(lhs.val);
      assert( vlhs and "Shift right can only be done on a variable value!" );
      ret->addCase( VHDLInterface::shift_right_arithmetic(vlhs, rhs_value) );
    }
    break ;
    case BinaryOperator::And:
    {
	    ret->addCase( lhs & rhs );
    }
    break ;
    case BinaryOperator::Or:
    {
  	  ret->addCase( lhs | rhs );
    }
    break ;
  case BinaryOperator::Xor:
    {
  	  ret->addCase( lhs ^ rhs );
    }
    break ;
  default:
    {
      INTERNAL_ERROR("Unknown binary operation " << *i);
      assert( 0 and "Unknown binary operation!" );
    }
  }
  return NULL;
}

VHDLInterface::VWrap VHDLProcess::LLVMValueToVHDLValue(Value* v)
{
  return getLLVMValueToVHDLValue(v, getParent());
}

VHDLInterface::Statement* VHDLProcess::ProcessInstructions(DFBasicBlock* b)
{
  VHDLInterface::MultiStatement*  ms = NULL;
  BasicBlock::iterator instIter = b->begin() ;
  while(instIter != b->end())
  {
    if (!(*instIter).isTerminator())
    {
      VHDLInterface::Statement* s = ProcessInstruction(&(*instIter));
      if( s )
      {
        if( ms == NULL )
          ms = new VHDLInterface::MultiStatement(this);
        ms->addStatement( s ) ;
      }
    }
    ++instIter ;
  }
  return ms;
}
