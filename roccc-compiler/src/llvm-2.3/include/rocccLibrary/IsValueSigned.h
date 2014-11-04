#ifndef _GET_VALUE_SIGNED_DOT_H__
#define _GET_VALUE_SIGNED_DOT_H__

#include "llvm/Value.h"

bool isValueSigned(llvm::Value* inst);
bool isValueUnsigned(llvm::Value* inst);

//true sets it as signed, false sets it as unsigned
void setValueSigned(llvm::Value* inst, bool);

#endif
