/* file "ia64/init.h" */

/*
    Copyright (c) 2002 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef IA64_INIT_H
#define IA64_INIT_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "ia64/init.h"
#endif

#include <machine/machine.h>

extern "C" void init_ia64(SuifEnv*);

// string constants
extern IdString k_ia64;
extern IdString k_va_first_unnamed;	// Note on special first-vararg VarSym*

// special types used for marking PR and BR register candidates
extern TypeId type_pr, type_br, type_f80;
extern bool is_type_pr(TypeId);
extern bool is_type_br(TypeId);

//FIXME - Alpha run-time routines.  Similar IA64 routines may be needed.
extern IdString k_divdi3;	// Object-time-system functions..
extern IdString k_divsf3;	
extern IdString k_moddi3;	// ..for integer divide and remainder
extern IdString k_memcpy;	

// common operands
extern Opnd opnd_immed_0_u64, opnd_immed_1_u64, opnd_immed_neg1_u64,
	opnd_immed_8_u64, opnd_immed_16_u64, opnd_immed_24_u64;

#endif /* IA64_INIT_H */
