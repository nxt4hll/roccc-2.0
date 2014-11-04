#ifndef _COPY_VALUE_DOT_H__
#define _COPY_VALUE_DOT_H__

#include "llvm/Value.h"

llvm::Value* getCopyOfValue(llvm::Value* inst);

bool isCopyValue(llvm::Value* inst);

#endif
