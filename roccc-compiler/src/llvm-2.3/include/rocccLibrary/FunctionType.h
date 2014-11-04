#include "llvm/Function.h"

namespace ROCCC {

enum FunctionType { ERROR, BLOCK, MODULE, SYSTEM };

/*
Get the function type of the given function
*/
FunctionType ROCCCFunctionType( llvm::Function* F );

}
