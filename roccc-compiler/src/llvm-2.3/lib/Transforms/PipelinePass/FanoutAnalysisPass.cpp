// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

This pass is responsible. It does not drink in excess, and always has a 
ride home that does not involve driving drunk. It sets aside a portion of its
paycheck for future emergencies, such as car repair and medical costs, and
saves for the future. This pass is responsible.

*/

#include "llvm/Pass.h"
#include "rocccLibrary/DFFunction.h"
#include "llvm/Support/CFG.h"

#include <map>
#include <algorithm>
#include <fstream>
#include <cmath>

#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/MessageLogger.h"
#include "rocccLibrary/PipelineBlocks.h"
#include "TimingRequirements.h"
#include "rocccLibrary/GetValueName.h"
#include "rocccLibrary/DefinitionInst.h"
#include "rocccLibrary/CopyValue.h"

namespace llvm
{
  class UnregisteredFanoutAnalysisPass : public FunctionPass
  {
  private:
  public:
    static char ID ;
    UnregisteredFanoutAnalysisPass() ;
    ~UnregisteredFanoutAnalysisPass() ;
    virtual bool runOnFunction(Function& b) ;
  } ;
}

using namespace llvm ;

char UnregisteredFanoutAnalysisPass::ID = 0 ;

static RegisterPass<UnregisteredFanoutAnalysisPass> X ("fanoutAnalysis", 
					"Guarantees high-fanout instructions are registered.");

UnregisteredFanoutAnalysisPass::UnregisteredFanoutAnalysisPass() : FunctionPass((intptr_t)&ID) 
{
  ; // Nothing in here
}

UnregisteredFanoutAnalysisPass::~UnregisteredFanoutAnalysisPass()
{
  ; // Nothing to delete either
}

bool UnregisteredFanoutAnalysisPass::runOnFunction(Function& f)
{
  CurrentFile::set(__FILE__);
  bool changed = false ;
  if (f.isDeclaration() || f.getDFFunction() == NULL)
  {
    return changed ;
  }
  unsigned int MAX_FANOUT = 50;
  std::ifstream file(".ROCCC/.timingInfo");
  if (!file)
  {
    INTERNAL_SERIOUS_WARNING("Could not open timing information file for max fanout!\n");
  }
  else
  {
    while( !file.eof() )
    {
      std::string name;
      int value = -1;
      file >> name;
      //allow comments
      if( name.find("#") == 0 or name.find("//") == 0 or name.find("--") == 0 )
      {
        std::string temp;
        std::getline(file, temp);
      }
      else
      {
        file >> value;
      }
      if( name == "MaxFanoutRegistered" and value >= 0 )
      {
        MAX_FANOUT = value;
      }
    }
  }
  LOG_MESSAGE2("Pipelining", "Fanout Analysis", "In order to reduce the negative effect on frequency that a high unregistered fanout can have, operations with a fanout greater than " << MAX_FANOUT << " will be registered.\n");
  
  
  std::map<DFBasicBlock*, bool> allBlocks = getPipelineBlocks(f);
  for(std::map<DFBasicBlock*,bool>::iterator BBI = allBlocks.begin(); BBI != allBlocks.end(); ++BBI)
  {
    if( BBI->second )
    {
      for( BasicBlock::iterator II = BBI->first->begin(); II != BBI->first->end(); ++II )
      {
        unsigned int realFanout = 0;
        for(Instruction::use_iterator UI = II->use_begin(); UI != II->use_end(); ++UI)
        {
          if( dynamic_cast<Instruction*>(*UI) and
              allBlocks.find(dynamic_cast<Instruction*>(*UI)->getParent()->getDFBasicBlock()) != allBlocks.end() and
              allBlocks.find(dynamic_cast<Instruction*>(*UI)->getParent()->getDFBasicBlock())->second != false and
              !isDefinition(dynamic_cast<Instruction*>(*UI), II) )
          {
            ++realFanout;
          }
        }
        if( realFanout > MAX_FANOUT )
        {
          LOG_MESSAGE2("Pipelining", "Fanout Analysis", getValueName(II) << " has fanout of " << realFanout << "; "
                                                        "Putting " << getValueName(II) << " into own pipeline stage.\n");
          Pipelining::TimingRequirements* timing = Pipelining::TimingRequirements::getCurrentRequirements(&f);
          timing->setBasicBlockDelay(BBI->first, timing->getDesiredDelay());
        }
      }
    }
  }
  
  return changed ;
}

namespace llvm
{
  class FanoutTreePass : public FunctionPass
  {
  private:
  public:
    static char ID ;
    FanoutTreePass() ;
    ~FanoutTreePass() ;
    virtual bool runOnFunction(Function& b) ;
  } ;
}

using namespace llvm ;

char FanoutTreePass::ID = 0 ;

static RegisterPass<FanoutTreePass> X2 ("fanoutTree", 
					"Creates a tree of copies for operations with high fanout.");

FanoutTreePass::FanoutTreePass() : FunctionPass((intptr_t)&ID) 
{
  ; // Nothing in here
}

FanoutTreePass::~FanoutTreePass()
{
  ; // Nothing to delete either
}

bool FanoutTreePass::runOnFunction(Function& f)
{
  CurrentFile::set(__FILE__);
  bool changed = false ;
  if (f.isDeclaration() || f.getDFFunction() == NULL)
  {
    return changed ;
  }
  unsigned int MAX_FANOUT = 50;
  std::ifstream file(".ROCCC/.optlo");
  if (!file)
  {
    INTERNAL_SERIOUS_WARNING("Could not open timing information file for max tree fanout!\n");
  }
  else
  {
    while( !file.eof() )
    {
      std::stringstream line;
      {
        std::string l2;
        getline(file, l2);
        line << l2;
      }
      std::string name;
      int value = -1;
      line >> name;
      //allow comments
      if( name.find("#") != 0 and name.find("//") != 0 and name.find("--") != 0 )
      {
        line >> value;
      }
      if( name == "FanoutTreeGeneration" and value >= 0 )
      {
        MAX_FANOUT = value;
      }
    }
  }
  LOG_MESSAGE2("Pipelining", "Fanout Analysis", "In order to reduce the negative effect on frequency that a high fanout can have, operations with a fanout greater than " << MAX_FANOUT << " will have a tree of copies created.\n");
  
  
  std::map<DFBasicBlock*, bool> allBlocks = getPipelineBlocks(f);
  for(std::map<DFBasicBlock*,bool>::iterator BBI = allBlocks.begin(); BBI != allBlocks.end(); ++BBI)
  {
    if( BBI->second )
    {
      for( BasicBlock::iterator II = BBI->first->begin(); II != BBI->first->end(); ++II )
      {
        std::vector<Instruction*> realUses;
        for(Instruction::use_iterator UI = II->use_begin(); UI != II->use_end(); ++UI)
        {
          if( dynamic_cast<Instruction*>(*UI) and
              allBlocks.find(dynamic_cast<Instruction*>(*UI)->getParent()->getDFBasicBlock()) != allBlocks.end() and
              allBlocks.find(dynamic_cast<Instruction*>(*UI)->getParent()->getDFBasicBlock())->second != false and
              !isDefinition(dynamic_cast<Instruction*>(*UI), II) )
          {
            realUses.push_back(dynamic_cast<Instruction*>(*UI));
          }
        }
        if( realUses.size() > MAX_FANOUT )
        {
          double log_val = std::log(realUses.size()) / std::log(MAX_FANOUT);
          int depth = std::ceil(log_val);
          int fanout = std::ceil(std::pow(realUses.size(), 1.0/depth));
          LOG_MESSAGE2("Pipelining", "Fanout Analysis", getValueName(II) << " has a fanout of " << realUses.size() << "; a copy tree of depth " << depth << " with a fanout of " << fanout << " at each level will be created.\n");
          for(int d = 0; d < depth; ++d)
          {
            std::vector<Instruction*> newUses;
            int curFanout = 0;
            for(std::vector<Instruction*>::iterator UI = realUses.begin(); UI != realUses.end(); ++UI)
            {
              if( curFanout == 0 )
              {
                Instruction* copy = dynamic_cast<Instruction*>(getCopyOfValue(II));
                newUses.push_back(copy);
                DFBasicBlock* copyBlock = DFBasicBlock::Create("fanoutTreeBlock", &f);
                copyBlock->addInstruction(copy);
                //Pipelining::TimingRequirements* timing = Pipelining::TimingRequirements::getCurrentRequirements(&f);
                //timing->setBasicBlockDelay(copyBlock, timing->getDesiredDelay());
              }
              assert(newUses.begin() != newUses.end());
              assert(newUses.back());
              assert(newUses.back()->getParent());
              assert(newUses.back()->getParent()->getDFBasicBlock());
              assert(*UI);
              assert((*UI)->getParent());
              assert((*UI)->getParent()->getDFBasicBlock());
              (*UI)->replaceUsesOfWith(II, newUses.back());
              II->getParent()->getDFBasicBlock()->RemoveUse((*UI)->getParent()->getDFBasicBlock());
              newUses.back()->getParent()->getDFBasicBlock()->AddUse((*UI)->getParent()->getDFBasicBlock());
              ++curFanout;
              if( curFanout >= fanout )
              {
                curFanout = 0;
              }
            }
            realUses = newUses;
          }
        }
      }
    }
  }
  
  return changed ;
}

