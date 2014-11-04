#ifndef _SIZE_IN_BITS_H__
#define _SIZE_IN_BITS_H__

#include "llvm/Value.h"

int getSizeInBits(llvm::Value*);

//void resetSizeMap();

void setSizeInBits(llvm::Value* val, int size);

#endif
