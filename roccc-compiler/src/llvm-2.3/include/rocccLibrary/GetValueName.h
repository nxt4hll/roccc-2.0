#ifndef _GET_VALUE_NAME_DOT_H__
#define _GET_VALUE_NAME_DOT_H__

#include "llvm/Value.h"

#include <string>

std::string getValueName(llvm::Value* inst);

void setValueName(llvm::Value* inst, std::string name);

#endif
