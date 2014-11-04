#include "rocccLibrary/ROCCCNames.h"

#include "llvm/GlobalValue.h"
#include "llvm/Function.h"

#include <sstream>

using namespace llvm;

bool isROCCCFunctionCall(llvm::CallInst* inst, std::string name)
{
  if( !inst )
    return false;
  return ( inst->getCalledFunction()->getName().find(name) == 0 );
}

std::string getComponentNameFromCallInst( CallInst* compIter )
{
  assert( isROCCCFunctionCall(compIter, ROCCCNames::InvokeHardware) and "Not a component call!" );
  Constant* tmpCast = dynamic_cast<Constant*>(compIter->getOperand(1)) ;
  assert(tmpCast and "Component name must be a constant!");
  return tmpCast->getStringValue() ;
}

bool isROCCCInputStream(llvm::CallInst* CI)
{
  return isROCCCFunctionCall(CI, ROCCCNames::InputFifo) or
         isROCCCFunctionCall(CI, ROCCCNames::InputSmartBuffer);
}
bool isROCCCOutputStream(llvm::CallInst* CI)
{
  return isROCCCFunctionCall(CI, ROCCCNames::OutputFifo) or
         isROCCCFunctionCall(CI, ROCCCNames::OutputSmartBuffer);
}
int getROCCCStreamDimension(llvm::CallInst* CI)
{
  std::string name = CI->getCalledFunction()->getName();
  assert( isROCCCInputStream(CI) or isROCCCOutputStream(CI) );
  std::stringstream ss;
  if(name.substr(0,ROCCCNames::InputFifo.length()) == ROCCCNames::InputFifo)
  {
    ss << name.substr(ROCCCNames::InputFifo.length());
  }
  if(name.substr(0,ROCCCNames::InputSmartBuffer.length()) == ROCCCNames::InputSmartBuffer)
  {
    ss << name.substr(ROCCCNames::InputSmartBuffer.length());
  }
  if(name.substr(0,ROCCCNames::OutputFifo.length()) == ROCCCNames::OutputFifo)
  {
    ss << name.substr(ROCCCNames::OutputFifo.length());
  }
  if(name.substr(0,ROCCCNames::OutputSmartBuffer.length()) == ROCCCNames::OutputSmartBuffer)
  {
    ss << name.substr(ROCCCNames::OutputSmartBuffer.length());
  }
  int val = 0;
  ss >> val;
  return val;
}

bool ROCCCNames::isROCCCFunction(llvm::CallInst* CI)
{
  return( isROCCCFunctionCall(CI, FunctionType) or
          isROCCCFunctionCall(CI, StateScalar) or
          isROCCCFunctionCall(CI, PortOrder) or
          isROCCCFunctionCall(CI, ModuleStructName) or
          isROCCCFunctionCall(CI, IntrinsicType) or
          isROCCCFunctionCall(CI, MaximizePrecision) or
          isROCCCFunctionCall(CI, InvokeHardware) or
          isROCCCFunctionCall(CI, InternalLUTDeclaration) or
          isROCCCFunctionCall(CI, LUTRead) or
          isROCCCFunctionCall(CI, LUTWrite) or
          isROCCCFunctionCall(CI, LUTOrder) or
          isROCCCFunctionCall(CI, SystolicNext) or
          isROCCCFunctionCall(CI, StoreNext) or
          isROCCCFunctionCall(CI, LoadPrevious) or
          isROCCCFunctionCall(CI, SystolicPrevious) or
          isROCCCFunctionCall(CI, InputScalar) or
          isROCCCFunctionCall(CI, OutputScalar) or
          isROCCCFunctionCall(CI, FeedbackScalar) or
          isROCCCFunctionCall(CI, InputStream) or
          isROCCCFunctionCall(CI, OutputStream) or
          isROCCCFunctionCall(CI, BoolSelect) or
          isROCCCFunctionCall(CI, TripleVote) or
          isROCCCFunctionCall(CI, StreamSplitter) or
          isROCCCFunctionCall(CI, StreamDoubleVote) or
          isROCCCFunctionCall(CI, StreamTripleVote) or
          isROCCCFunctionCall(CI, SummationFeedback) or
          isROCCCFunctionCall(CI, InputFifo) or
          isROCCCFunctionCall(CI, InputSmartBuffer) or
          isROCCCFunctionCall(CI, OutputFifo) or
          isROCCCFunctionCall(CI, OutputSmartBuffer) or
          isROCCCFunctionCall(CI, VariableSize) or
          isROCCCFunctionCall(CI, VariableName) or
          isROCCCFunctionCall(CI, VariableSigned) or
          isROCCCFunctionCall(CI, LoadPreviousInitValue) or
          isROCCCFunctionCall(CI, DebugScalarOutput) or
          isROCCCFunctionCall(CI, InductionVariableStartValue) or
          isROCCCFunctionCall(CI, InductionVariableEndValue) or
          isROCCCFunctionCall(CI, InductionVariableStepSize) or
          isROCCCFunctionCall(CI, InfiniteLoopCondition) or
          isROCCCFunctionCall(CI, NumberOfOutstandingMemoryRequests) or
          isROCCCFunctionCall(CI, NumberOfDataChannels) or
          isROCCCFunctionCall(CI, NumberOfAddressChannels)
        );
}
const std::string ROCCCNames::FunctionType = "ROCCCFunctionType";
const std::string ROCCCNames::StateScalar = "ROCCC_state_scalar";
const std::string ROCCCNames::PortOrder = "ROCCCPortOrder";
const std::string ROCCCNames::ModuleStructName = "ROCCCModuleStructName";
const std::string ROCCCNames::IntrinsicType = "ROCCCIntrinsicType";
const std::string ROCCCNames::MaximizePrecision = "ROCCCMaximizePrecision";
const std::string ROCCCNames::InvokeHardware = "ROCCCInvokeHardware";
const std::string ROCCCNames::InternalLUTDeclaration = "ROCCCInternalLUTDeclaration";
const std::string ROCCCNames::LUTRead = "ROCCCLUTLookup";
const std::string ROCCCNames::LUTWrite = "ROCCCLUTStore";
const std::string ROCCCNames::LUTOrder = "ROCCCLUTs";
const std::string ROCCCNames::SystolicNext = "ROCCC_systolicNext";
const std::string ROCCCNames::StoreNext = "ROCCC_storeNext";
const std::string ROCCCNames::LoadPrevious = "ROCCC_loadPrevious";
const std::string ROCCCNames::SystolicPrevious = "ROCCCSystolicPrevious";
const std::string ROCCCNames::InputScalar = "ROCCC_init_inputscalar";
const std::string ROCCCNames::OutputScalar = "ROCCC_output_C_scalar";
const std::string ROCCCNames::FeedbackScalar = "ROCCCFeedbackScalar";
const std::string ROCCCNames::InputStream = "ROCCCInputStreams";
const std::string ROCCCNames::OutputStream = "ROCCCOutputStreams";
const std::string ROCCCNames::BoolSelect = "ROCCC_boolsel";
const std::string ROCCCNames::DoubleVote = "ROCCCDoubleVote";
const std::string ROCCCNames::TripleVote = "ROCCCTripleVote";
const std::string ROCCCNames::StreamSplitter = "ROCCCSplitter";
const std::string ROCCCNames::StreamDoubleVote = "ROCCCStreamsDoubleVote";
const std::string ROCCCNames::StreamTripleVote = "ROCCCStreamsTripleVote";
const std::string ROCCCNames::SummationFeedback = "ROCCCSummation";
const std::string ROCCCNames::InputFifo = "ROCCCInputFifo";
const std::string ROCCCNames::InputSmartBuffer = "ROCCCInputSB";
const std::string ROCCCNames::OutputFifo = "ROCCCOutputFifo";
const std::string ROCCCNames::OutputSmartBuffer = "ROCCCOutputSB";
const std::string ROCCCNames::ConvertFloatToFloat = "ROCCCFPToFP";
const std::string ROCCCNames::ConvertFloatToInt = "ROCCCFPToInt";
const std::string ROCCCNames::ConvertIntToFloat = "ROCCCIntToFP";
const std::string ROCCCNames::ConvertIntToInt = "ROCCCIntToInt";
const std::string ROCCCNames::VariableSize = "ROCCCSize";
const std::string ROCCCNames::VariableName = "ROCCCName";
const std::string ROCCCNames::VariableSigned = "ROCCCSigned";
const std::string ROCCCNames::LoadPreviousInitValue = "ROCCCLoadPreviousInit";
const std::string ROCCCNames::DebugScalarOutput = "ROCCCDebugScalarOutput";
const std::string ROCCCNames::InductionVariableStartValue = "ROCCCStartValue";
const std::string ROCCCNames::InductionVariableEndValue = "ROCCCEndValue";
const std::string ROCCCNames::OutputInductionVariableEndValue = "ROCCCEndValueOutput";
const std::string ROCCCNames::InductionVariableStepSize = "ROCCCStep";
const std::string ROCCCNames::InfiniteLoopCondition = "ROCCCInfinity";
const std::string ROCCCNames::NumberOfOutstandingMemoryRequests = "ROCCCNumMemReq";
const std::string ROCCCNames::NumberOfDataChannels = "ROCCCNumDataChannels";
const std::string ROCCCNames::NumberOfAddressChannels = "ROCCCNumAddressChannels";

