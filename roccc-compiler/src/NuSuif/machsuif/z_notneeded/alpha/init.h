/* file "alpha/init.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef ALPHA_INIT_H
#define ALPHA_INIT_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "alpha/init.h"
#endif

extern "C" void init_alpha(SuifEnv*);

// string constants
extern IdString k_alpha;
extern IdString k_gprel_init;		// Note for GP-relative symbol inits
extern IdString k_hint;			// Note for branch hints
extern IdString k_reloc;			// Note for relocations
extern IdString k_next_free_reloc_num;	// Note for reloc numbering

extern IdString k_gp_home;		// Name of local home var for $gp

extern IdString k_OtsDivide64;		// Object-time-system functions..
extern IdString k_OtsDivide64Unsigned;	// ..for integer divide and remainder
extern IdString k_OtsDivide32;
extern IdString k_OtsDivide32Unsigned;
extern IdString k_OtsRemainder64;
extern IdString k_OtsRemainder64Unsigned;
extern IdString k_OtsRemainder32;
extern IdString k_OtsRemainder32Unsigned;

// common operands
extern Opnd opnd_immed_0_u64, opnd_immed_1_u64, opnd_immed_8_u64,
    opnd_immed_16_u64, opnd_immed_24_u64;

#endif /* ALPHA_INIT_H */
