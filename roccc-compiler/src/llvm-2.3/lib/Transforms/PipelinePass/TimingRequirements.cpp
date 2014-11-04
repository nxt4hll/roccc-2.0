#include "TimingRequirements.h"

#include <string>
#include <fstream>
#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/ROCCCNames.h"
#include "rocccLibrary/DFBasicBlock.h"
#include "rocccLibrary/CopyValue.h"
#include "rocccLibrary/PipelineBlocks.h"
#include "rocccLibrary/MessageLogger.h"

using namespace Pipelining;
using namespace llvm;

/*
TimingInfo is a class that provides an easy interface to a timing
configuration file.
*/
class TimingInfo {
  std::map<std::string, int> timing;
  float operationsPerPipelineStage;
  int getDelayOfType(std::string type)
  {
    std::map<std::string, int>::iterator t = timing.find(type);
    if( t == timing.end() )
    {
      timing[type] = 1;
      assert(timing.find(type) != timing.end());
      INTERNAL_WARNING("Could not find " << type << " delay! Setting to " << timing[type] << "!\n");
      return timing[type];
    }
    return t->second;
  }
public:
  TimingInfo() : operationsPerPipelineStage(1.0)
  {
    std::ifstream f(".ROCCC/.timingInfo");
    if (!f)
    {
      INTERNAL_SERIOUS_WARNING("Could not open timing information!\n");
      return;
    }
    while( !f.eof() )
    {
      std::string name;
      int value = -1;
      f >> name;
      //allow comments
      if( name.find("#") == 0 or name.find("//") == 0 or name.find("--") == 0 )
      {
        std::string temp;
        std::getline(f, temp);
      }
      else if( name.find("OperationsPerPipelineStage") == 0 )
      {
        f >> operationsPerPipelineStage;
      }
      else
      {
        f >> value;
      }
      if( name != "" and value >= 0 )
      {
        timing[name] = value;
      }
    }
  }
  int getCopyDelay()
  {
    return getDelayOfType("Copy");
  }
  int getAddDelay()
  {
    return getDelayOfType("Add");
  }
  int getSubDelay()
  {
    return getDelayOfType("Sub");
  }
  int getMulDelay()
  {
    return getDelayOfType("Mult");
  }
  int getShiftDelay()
  {
    return getDelayOfType("Shift");
  }
  int getAndDelay()
  {
    return getDelayOfType("AND");
  }
  int getOrDelay()
  {
    return getDelayOfType("OR");
  }
  int getXorDelay()
  {
    return getDelayOfType("XOR");
  }
  int getCmpDelay()
  {
    return getDelayOfType("Compare");
  }
  int getMuxDelay()
  {
    return getDelayOfType("Mux");
  }
  int getMaximumDelay()
  {
    int max = 0;
    for(std::map<std::string, int>::iterator TMI = timing.begin(); TMI != timing.end(); ++TMI)
    {
      if( TMI->second > max and TMI->first != "MaxFanoutRegistered" )
        max = TMI->second;
    }
    return max;
  }
  float getOperationsPerPipelineStage()
  {
    return operationsPerPipelineStage;
  }
};

/*
Sum up the delay of each instruction in a BasicBlock, given a desiredDelay
*/
int getBasicBlockDelay(DFBasicBlock* BB, int desiredDelay)
{
  TimingInfo t;
  int totalDelay = 0;
  for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
  {
    if ( CallInst* CI = dynamic_cast<CallInst*>(&*II) )
    {
      //setting to DesiredDelay means it will always have its own pipeline level
      if( isROCCCFunctionCall(CI, ROCCCNames::InvokeHardware) )
      {
        totalDelay += desiredDelay;
      }
      //dont register the input
      else if (isROCCCFunctionCall(CI, ROCCCNames::InputScalar))
      {
        totalDelay += t.getCopyDelay();
      }  
      else if ( isROCCCInputStream(CI) )
      {
        totalDelay += t.getCopyDelay();
      }
      //but register the output
      else if (isROCCCFunctionCall(CI, ROCCCNames::OutputScalar))
      {
        totalDelay += desiredDelay;
      }
      else if (isROCCCFunctionCall(CI, ROCCCNames::FeedbackScalar))
      {
        totalDelay += desiredDelay;
      }
      else if ( isROCCCOutputStream(CI) )
      {
        totalDelay += desiredDelay;
      }
      //boolSelects are muxes
      else if ( isROCCCFunctionCall(CI, ROCCCNames::BoolSelect) )
      {
        totalDelay += t.getMuxDelay();
      }
      //anything systolic or store related needs its own pipeline level
      else if ( isROCCCFunctionCall(CI, ROCCCNames::SystolicNext) or
                isROCCCFunctionCall(CI, ROCCCNames::StoreNext) )
      {
        totalDelay += desiredDelay;
      }
      else if( isROCCCFunctionCall(CI, ROCCCNames::LoadPrevious) )
      {
        totalDelay += desiredDelay;
      }
      else if( isROCCCFunctionCall(CI, ROCCCNames::SystolicPrevious) )
      {
        totalDelay += t.getCopyDelay();
      }
      else if( isROCCCFunctionCall(CI, ROCCCNames::SummationFeedback) )
      {
        totalDelay += desiredDelay;
      }
      else if( isROCCCFunctionCall(CI, ROCCCNames::LUTRead) )
      {
        totalDelay += desiredDelay;
      }
      else if( isROCCCFunctionCall(CI, ROCCCNames::LUTWrite) )
      {
        totalDelay += desiredDelay;
      }
      else if( isROCCCFunctionCall(CI, ROCCCNames::LUTOrder) )
      {
        totalDelay += 0;
      }
      else if( isROCCCFunctionCall(CI, ROCCCNames::InternalLUTDeclaration) )
      {
        totalDelay += 0;
      }
      else
        INTERNAL_SERIOUS_WARNING("Unknown call inst " << *CI);
    }
    else if (dynamic_cast<UnaryInstruction*>(&*II) != NULL)
    {
      if (dynamic_cast<ZExtInst*>(&*II) != NULL)
      {
        assert(0 and "We should have no zero extends by this point!");
        totalDelay += t.getCopyDelay();
      }
      else if (dynamic_cast<SExtInst*>(&*II) != NULL)
      {
        totalDelay += t.getCopyDelay();
      }
      else if (dynamic_cast<LoadInst*>(&*II) != NULL)
      {
      }
      else if (dynamic_cast<BitCastInst*>(&*II) != NULL)
      {
        totalDelay += t.getCopyDelay();
      }
      else if( dynamic_cast<TruncInst*>(&*II) != NULL )
      {
        totalDelay += t.getCopyDelay();
      }
      else
        INTERNAL_SERIOUS_WARNING("Unknown unary operator " << *II);
    }
    else if (dynamic_cast<CmpInst*>(&*II) != NULL)
    {
      totalDelay += t.getCmpDelay();
    }
    else if( isCopyValue(&*II) )
    {
      totalDelay += t.getCopyDelay();
    }
    else if( BinaryOperator* i = dynamic_cast<BinaryOperator*>(&*II) )
    {
      switch(i->getOpcode())
      {
        case BinaryOperator::Add:
        {
          totalDelay += t.getAddDelay();
        }
        break ;
        case BinaryOperator::Sub:
        {
      	    totalDelay += t.getSubDelay();
        }
        break ;
        case BinaryOperator::Mul:
        {
      	    totalDelay += t.getMulDelay();
        }
        break;
        case BinaryOperator::Shl:  // Shift left
        {
          totalDelay += t.getShiftDelay();
        }
        break ;
        case BinaryOperator::LShr: // Shift right logical
        {
          totalDelay += t.getShiftDelay();
        }
        break;
        case BinaryOperator::AShr: // Shift right arithmetic
        {
          totalDelay += t.getShiftDelay();
        }
        break ;
        case BinaryOperator::And:
        {
          totalDelay += t.getAndDelay();
        }
        break ;
        case BinaryOperator::Or:
        {
          totalDelay += t.getOrDelay();
        }
        break ;
        case BinaryOperator::Xor:
        {
          totalDelay += t.getXorDelay();
        }
        break ;
        default:
        {
          INTERNAL_ERROR("Unknown binary operation " << *i);
          assert( 0 and "Unknown binary operation!" );
        }
      }
    }
    else if( dynamic_cast<TerminatorInst*>(&*II) )
    {
    }
    else if( dynamic_cast<PHINode*>(&*II) )
    {
    }
    else
      INTERNAL_SERIOUS_WARNING("Unknown operation " << *II);
  }
  if( totalDelay > desiredDelay )
  {
    //INTERNAL_SERIOUS_WARNING("Total delay of " << *BB << " is " << totalDelay << ", which is greater than desired delay, " << desiredDelay << "! Setting total to desired!\n");
    totalDelay = desiredDelay;
  }
  return totalDelay;
}

std::map<llvm::Function*,TimingRequirements*> TimingRequirements::instances;

TimingRequirements::TimingRequirements(llvm::Function* f) : desiredDelay(0)
{
  TimingInfo t;
  //calculate the desiredDelay, based on the operations per pipeline stage
  //and the average delay of the used operations
  int totalOperations = 0;
  int totalDelay = 0;
  std::map<DFBasicBlock*, bool> blocks = getPipelineBlocks(*f);
  for(std::map<DFBasicBlock*, bool>::iterator BB = blocks.begin(); BB != blocks.end(); ++BB)
  {
    if( BB->second )
    {
      totalDelay += ::getBasicBlockDelay(BB->first, t.getMaximumDelay());
      ++totalOperations;
    }
  }
  float averageDelay = float(totalDelay) / float(totalOperations);
  desiredDelay = std::ceil(averageDelay * t.getOperationsPerPipelineStage());
  //LOG_MESSAGE2("Pipelining", "Timing Requirements", "averageDelay = " << totalDelay << " / " << totalOperations << " = " << averageDelay << "; operationsPerPipelineStage = " << t.getOperationsPerPipelineStage() << "; desiredDelay = " << desiredDelay << "\n");
}
TimingRequirements* TimingRequirements::getCurrentRequirements(llvm::Function* f)
{
  if( instances.find(f) == instances.end() )
  {
    TimingRequirements* tr = new TimingRequirements(f);
    instances[f] = tr;
  }
  return instances[f];
  
}
int TimingRequirements::getDesiredDelay()
{
  return desiredDelay;
}
int TimingRequirements::getBasicBlockDelay(llvm::BasicBlock* BB)
{
  if( timingValues.find(BB) == timingValues.end() )
  {
    timingValues[BB] = ::getBasicBlockDelay(BB->getDFBasicBlock(), desiredDelay);
  }
  return timingValues[BB];
}
void TimingRequirements::setBasicBlockDelay(llvm::BasicBlock* BB, int delay)
{
  timingValues[BB] = delay;
}
