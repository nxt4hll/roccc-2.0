/* file "machine/opcodes.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef MACHINE_OPCODES_H
#define MACHINE_OPCODES_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "machine/opcodes.h"
#endif

#include <machine/substrate.h>

bool target_implements(int opcode);

char* opcode_name(int opcode);

int opcode_move(TypeId);
int opcode_load(TypeId);
int opcode_store(TypeId);

int opcode_line();
int opcode_ubr();

int opcode_cbr_inverse(int opcode);

const int opcode_null = 0;
const int opcode_label = 1;

int opcode_from_name(const char *name);

#endif /* MACHINE_OPCODES_H */
