#ifndef _LOOP_INDUCTION_VARIABLE_HANDLER_H__
#define _LOOP_INDUCTION_VARIABLE_HANDLER_H__

#include <vector>
#include "llvm/Value.h"
#include "rocccLibrary/VHDLInterface.h"
#include <map>

class LoopInductionVariableHandler {
  struct LIVData {
    std::vector<llvm::Value*> children;
    llvm::Value* endValue;
    int stepSize;
    VHDLInterface::Variable* counter;
    VHDLInterface::Value* counterEndValue;
    LIVData();
    void setEndValue(llvm::Value*);
    bool endValueIsInfinity();
    bool endValueIsInteger();
    int getEndValueAsInteger();
  };
  std::vector<llvm::Value*> indexes ;
  std::map<llvm::Value*,LIVData> indexData;
  VHDLInterface::Condition* window_reset_condition;
  VHDLInterface::Statement* increment_statement;
  std::vector<llvm::Value*> getOutermostLIVs();
  //Given a LIV value liv, get the condition that it should be reset
  VHDLInterface::CWrap getResetCondition(llvm::Value* liv);
public:
  LoopInductionVariableHandler();
  llvm::Value* getInnermostLIV();
  //setIndexes() also sets the step size using getIVStepSize()
  void setIndexes(std::vector<llvm::Value*> i);
  void setChildren(std::map<llvm::Value*,std::vector<llvm::Value*> > lc);
  //The end values can either be llvm::Constant, ROCCCInfinity() calls,
  //  or variables. If they are non-constant, getEndValueVHDLValue() returns NULL.
  void setEndValues(std::map<llvm::Value*,llvm::Value*> ev);
  //If end is NULL, setEndValues() must have already been called.
  void setVHDLInterface(llvm::Value* livValue, VHDLInterface::Variable* count, VHDLInterface::Value* end = NULL);
  VHDLInterface::Variable* getLIVVHDLValue(llvm::Value* LIV);
  //Returns NULL if the end value is either infinity or a variable
  VHDLInterface::Value* getEndValueVHDLValue(llvm::Value* LIV);
  //This is used to determine whether the end value is infinity
  bool isEndValueInfinity(llvm::Value* LIV);

  // This is used to determine if the loop induction variables are multi-dim
  //  or not.
  bool isMultidimensional() ;

  //get statements
  //Get the innermost LIV's reset condition
  VHDLInterface::CWrap getWindowResetCondition();
  //Is the window at the edge, ie should we reset the window?
  VHDLInterface::CWrap shouldIncrement(llvm::Value* liv);
  //Get the condition under which we are done
  VHDLInterface::CWrap getDoneCondition();
  //Get the statement for incrementing the LIVs by 1.
  VHDLInterface::Statement* getIncrementStatement(VHDLInterface::Process* owner);
};

std::vector<VHDLInterface::Variable*> getVectorOfVHDLIndexes(LoopInductionVariableHandler* handler, std::vector<llvm::Value*> llvmIndexes);
std::vector<VHDLInterface::Value*> getVectorOfVHDLEndValues(LoopInductionVariableHandler* handler, std::vector<llvm::Value*> llvmIndexes);

#endif
