#ifndef _INDUCTION_VARIABLE_INFO_DOT_H__
#define _INDUCTION_VARIABLE_INFO_DOT_H__

#include "llvm/Value.h"

int getIVStepSize(llvm::Value* inst);
int getIVStartValue(llvm::Value* inst);
int getIVEndValue(llvm::Value* inst);

#endif
