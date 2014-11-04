// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

This file contains string constants for the names of ROCCC functions,
along with some basic functions that operate on those names.

*/

#ifndef _ROCCC_NAMES_DOT_H__
#define _ROCCC_NAMES_DOT_H__

#include <string>
#include "llvm/Instructions.h"

bool isROCCCFunctionCall(llvm::CallInst* inst, std::string name);
std::string getComponentNameFromCallInst( llvm::CallInst* CI );
bool isROCCCInputStream(llvm::CallInst* CI);
bool isROCCCOutputStream(llvm::CallInst* CI);
int getROCCCStreamDimension(llvm::CallInst* CI);

class ROCCCNames {
  ROCCCNames();//DO NOT IMPLEMENT
public:
static bool isROCCCFunction(std::string);
static bool isROCCCFunction(llvm::CallInst*);
//function level information
static const std::string FunctionType;
static const std::string StateScalar;
static const std::string PortOrder;
static const std::string ModuleStructName;
static const std::string IntrinsicType;
//compiler settings
static const std::string MaximizePrecision;
//callable instruction types
static const std::string InvokeHardware;
static const std::string InternalLUTDeclaration;
static const std::string LUTRead;
static const std::string LUTWrite;
static const std::string LUTOrder;
static const std::string SystolicNext;
static const std::string StoreNext;
static const std::string LoadPrevious;
static const std::string SystolicPrevious;
static const std::string InputScalar;
static const std::string OutputScalar;
static const std::string FeedbackScalar;
static const std::string InputStream;
static const std::string OutputStream;
static const std::string BoolSelect;
static const std::string DoubleVote;
static const std::string TripleVote;
static const std::string StreamSplitter;
static const std::string StreamDoubleVote;
static const std::string StreamTripleVote;
static const std::string SummationFeedback;
static const std::string InputFifo;
static const std::string InputSmartBuffer;
static const std::string OutputFifo;
static const std::string OutputSmartBuffer;
//type/size conversion functions
static const std::string ConvertFloatToFloat;
static const std::string ConvertFloatToInt;
static const std::string ConvertIntToFloat;
static const std::string ConvertIntToInt;
//generic variable information
static const std::string VariableSize;
static const std::string VariableName;
static const std::string VariableSigned;
//loadPrevious specific control
static const std::string LoadPreviousInitValue;
//debug functions
static const std::string DebugScalarOutput;
//stream information
static const std::string InductionVariableStartValue;
static const std::string InductionVariableEndValue;
static const std::string OutputInductionVariableEndValue;
static const std::string InductionVariableStepSize;
static const std::string InfiniteLoopCondition;
static const std::string NumberOfOutstandingMemoryRequests;
static const std::string NumberOfDataChannels;
static const std::string NumberOfAddressChannels;
};

#endif
