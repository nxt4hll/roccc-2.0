#ifndef _DEFINITION_INST_DOT_H__
#define _DEFINITION_INST_DOT_H__

#include "llvm/Instruction.h"
#include "llvm/BasicBlock.h"

// If the predecessor instruction (passed in as i) defines the value
//  (passed in as v), then return true.  Otherwise return false.
bool isDefinition(llvm::Instruction* i, llvm::Value* v);

//given an instruction, i, and a starting basic block BB,
//searches the preds of BB to see if any of the instructions in there define i
llvm::Instruction* getDefinitionInstruction(llvm::Instruction* i, llvm::BasicBlock* BB);

#endif

