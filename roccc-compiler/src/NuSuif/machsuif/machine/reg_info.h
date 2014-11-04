/* file "machine/reg_info.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef MACHINE_REG_INFO_H
#define MACHINE_REG_INFO_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "machine/reg_info.h"
#endif

#include <machine/substrate.h>
#include <machine/machine_ir.h>
#include <machine/nat_set.h>
#include <machine/opnd.h>

typedef int RegClassId;

typedef Vector<RegClassId> RegClassMap;

int reg_count();
const char* reg_name(int reg);
int reg_width(int reg);
int reg_maximal(int reg);
const NatSet* reg_aliases(int reg);
const NatSet* reg_allocables(bool maximals = false);
const NatSet* reg_caller_saves(bool maximals = false);
const NatSet* reg_callee_saves(bool maximals = false);

void reg_info_print(FILE*);

const RegClassId REG_CLASS_ANY  = -1;	// universal class
const RegClassId REG_CLASS_NONE = -2;	// empty class

int reg_class_count();
const NatSet* reg_members(RegClassId);
RegClassId reg_class_intersection(RegClassId, RegClassId);

void reg_classify(Instr*, OpndCatalog*, RegClassMap*);
int reg_choice(RegClassId, const NatSet *pool, const NatSet *excluded,
	       bool rotate);

InstrHandle reg_fill (Opnd dst, Opnd src, InstrHandle marker,
                      bool post_reg_alloc = false);
InstrHandle reg_spill(Opnd dst, Opnd src, InstrHandle marker,
                      bool post_reg_alloc = false);

extern int reg_lookup(const char *name);


#endif /* MACHINE_REG_INFO_H */
