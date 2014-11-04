#include "rocccLibrary/VHDLComponents/LoopInductionVariableHandler.h"

#include "llvm/Constants.h"
#include "llvm/Instructions.h"
#include "rocccLibrary/ROCCCNames.h"
#include "rocccLibrary/InductionVariableInfo.h"

LoopInductionVariableHandler::LIVData::LIVData() : endValue(NULL), stepSize(0), counter(NULL), counterEndValue(NULL)
{
}
void LoopInductionVariableHandler::LIVData::setEndValue(llvm::Value* v)
{
  endValue = v;
  if( endValueIsInteger() )
  {
    counterEndValue = VHDLInterface::ConstantInt::get(getEndValueAsInteger());
  }
}
bool LoopInductionVariableHandler::LIVData::endValueIsInfinity()
{
  return isROCCCFunctionCall(dynamic_cast<llvm::CallInst*>(endValue), ROCCCNames::InfiniteLoopCondition);
}
bool LoopInductionVariableHandler::LIVData::endValueIsInteger()
{
  return dynamic_cast<llvm::ConstantInt*>(endValue);
}
int LoopInductionVariableHandler::LIVData::getEndValueAsInteger()
{
  assert(endValueIsInteger());
  return dynamic_cast<llvm::ConstantInt*>(endValue)->getValue().getSExtValue();
}

std::vector<llvm::Value*> LoopInductionVariableHandler::getOutermostLIVs()
{
  std::map<llvm::Value*, bool> is_child;
  for(std::map<llvm::Value*,LIVData>::iterator IDI = indexData.begin(); IDI != indexData.end(); ++IDI)
  {
    for(std::vector<llvm::Value*>::iterator CI = IDI->second.children.begin(); CI != IDI->second.children.end(); ++CI)
    {
      is_child[*CI] = true;
    }
  }
  std::vector<llvm::Value*> ret;
  for(std::vector<llvm::Value*>::iterator II = indexes.begin(); II != indexes.end(); ++II)
  {
    if( !is_child[*II] )
      ret.push_back(*II);
  }
  return ret;
}
VHDLInterface::CWrap LoopInductionVariableHandler::getResetCondition(llvm::Value* liv)
{
  if( indexData[liv].endValueIsInfinity() )
    return VHDLInterface::ConstCondition::get(false);
  return VHDLInterface::CWrap(VHDLInterface::Wrap(indexData[liv].counter) + VHDLInterface::ConstantInt::get(indexData[liv].stepSize) >= VHDLInterface::Wrap(indexData[liv].counterEndValue));
}
LoopInductionVariableHandler::LoopInductionVariableHandler() : window_reset_condition(NULL), increment_statement(NULL)
{
}
llvm::Value* LoopInductionVariableHandler::getInnermostLIV()
{
  for(std::map<llvm::Value*,LIVData>::iterator IDI = indexData.begin(); IDI != indexData.end(); ++IDI)
  {
    if(IDI->second.children.size() == 0)
      return IDI->first;
  }
  assert(0 and "Could not find innermostLIV!");
}
void LoopInductionVariableHandler::setIndexes(std::vector<llvm::Value*> i)
{
  indexes = i;
  for(std::vector<llvm::Value*>::iterator IVI = i.begin(); IVI != i.end(); ++IVI)
  {
    indexData[*IVI].stepSize = getIVStepSize(*IVI);
  }
}
void LoopInductionVariableHandler::setChildren(std::map<llvm::Value*,std::vector<llvm::Value*> > lc)
{
  for(std::map<llvm::Value*,std::vector<llvm::Value*> >::iterator LCI = lc.begin(); LCI != lc.end(); ++LCI)
  {
    indexData[LCI->first].children = LCI->second;
  }
  for(std::map<llvm::Value*,std::vector<llvm::Value*> >::iterator LCI = lc.begin(); LCI != lc.end(); ++LCI)
  {
    llvm::Value* ALIVI = LCI->first;
    std::vector<llvm::Value*> processList(indexData[ALIVI].children.begin(), indexData[ALIVI].children.end());
    //also, set the children of our direct children as children
    while( !processList.empty() )
    {
      llvm::Value* CI = processList.back();
      processList.pop_back();
      for(std::vector<llvm::Value*>::iterator NI = indexData[CI].children.begin(); NI != indexData[CI].children.end(); ++NI)
      {
        processList.push_back(*NI);
        indexData[ALIVI].children.push_back(*NI);
      }
    }
  }
}
void LoopInductionVariableHandler::setEndValues(std::map<llvm::Value*,llvm::Value*> ev)
{
  for(std::map<llvm::Value*,llvm::Value*>::iterator EVI = ev.begin(); EVI != ev.end(); ++EVI)
  {
    indexData[EVI->first].setEndValue(EVI->second);
  }
}
void LoopInductionVariableHandler::setVHDLInterface(llvm::Value* livValue, VHDLInterface::Variable* count, VHDLInterface::Value* end)
{
  indexData[livValue].counter = count;
  if( end != NULL )
    indexData[livValue].counterEndValue = end;
}
VHDLInterface::Variable* LoopInductionVariableHandler::getLIVVHDLValue(llvm::Value* LIV)
{
  return indexData[LIV].counter;
}
VHDLInterface::Value* LoopInductionVariableHandler::getEndValueVHDLValue(llvm::Value* LIV)
{
  return indexData[LIV].counterEndValue;
}
bool LoopInductionVariableHandler::isEndValueInfinity(llvm::Value* LIV)
{
  return indexData[LIV].endValueIsInfinity();
}

bool LoopInductionVariableHandler::isMultidimensional()
{
  return (indexes.size() > 1) ;
}

VHDLInterface::CWrap LoopInductionVariableHandler::getWindowResetCondition()
{
  if( !window_reset_condition )
    window_reset_condition = getResetCondition(getInnermostLIV());
  return VHDLInterface::CWrap(window_reset_condition);
}
VHDLInterface::CWrap LoopInductionVariableHandler::shouldIncrement(llvm::Value* liv)
{
  VHDLInterface::CWrap inc = not getResetCondition(liv);
  for(std::vector<llvm::Value*>::iterator NI = indexData[liv].children.begin(); NI != indexData[liv].children.end(); ++NI)
  {
    inc = inc and getResetCondition(*NI);
  }
  return inc;
}
VHDLInterface::CWrap LoopInductionVariableHandler::getDoneCondition()
{
  VHDLInterface::CWrap ret = VHDLInterface::ConstCondition::get(true);
  std::vector<llvm::Value*> outermost_livs = indexes;//getOutermostLIVs();
  for(std::vector<llvm::Value*>::iterator OMI = outermost_livs.begin(); OMI != outermost_livs.end(); ++OMI)
  {
    VHDLInterface::CWrap reset_condition = getResetCondition(*OMI);
    ret = ret and reset_condition;
  }
  return ret;
}
VHDLInterface::Statement* LoopInductionVariableHandler::getIncrementStatement(VHDLInterface::Process* owner)
{
  if( !increment_statement )
  {
    VHDLInterface::MultiStatement* ms = new VHDLInterface::MultiStatement(owner);
    for(std::vector<llvm::Value*>::iterator II = indexes.begin(); II != indexes.end(); ++II)
    {
      VHDLInterface::MultiStatement* inc_ms = new VHDLInterface::MultiStatement(owner);
      inc_ms->addStatement( new VHDLInterface::AssignmentStatement(indexData[*II].counter, VHDLInterface::Wrap(indexData[*II].counter) + VHDLInterface::ConstantInt::get(indexData[*II].stepSize), owner) );
      //when you increment, reset the child indexes
      for(std::vector<llvm::Value*>::iterator CI = indexData[*II].children.begin(); CI != indexData[*II].children.end(); ++CI)
      {
        //FIXME: these should check what the start values of the indexes are
        inc_ms->addStatement(new VHDLInterface::AssignmentStatement(indexData[*CI].counter, VHDLInterface::ConstantInt::get(getIVStartValue(*CI)), owner));
      }
      VHDLInterface::IfStatement* inc = new VHDLInterface::IfStatement(owner, shouldIncrement(*II), inc_ms);
      ms->addStatement(inc);
    }
    increment_statement = ms;
  }
  assert( increment_statement );
  assert(increment_statement->getParent() == owner);
  return increment_statement;    
}
std::vector<VHDLInterface::Variable*> getVectorOfVHDLIndexes(LoopInductionVariableHandler* handler, std::vector<llvm::Value*> llvmIndexes)
{
  std::vector<VHDLInterface::Variable*> indexes;
  for(std::vector<llvm::Value*>::iterator ALIVI = llvmIndexes.begin(); ALIVI != llvmIndexes.end(); ++ALIVI)
  {
    indexes.push_back( handler->getLIVVHDLValue(*ALIVI) );
  }
  return indexes;
}
std::vector<VHDLInterface::Value*> getVectorOfVHDLEndValues(LoopInductionVariableHandler* handler, std::vector<llvm::Value*> llvmIndexes)
{
  std::vector<VHDLInterface::Value*> endValues;
  for(std::vector<llvm::Value*>::iterator ALIVI = llvmIndexes.begin(); ALIVI != llvmIndexes.end(); ++ALIVI)
  {
    endValues.push_back( handler->getEndValueVHDLValue(*ALIVI) );
  }
  return endValues;
}
