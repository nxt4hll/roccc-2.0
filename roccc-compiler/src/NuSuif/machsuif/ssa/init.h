/* file "ssa/init.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef SSA_INIT_H
#define SSA_INIT_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "ssa/init.h"
#endif

extern IdString k_phi_nodes;			// key for attaching phi-nodes to blocks

extern "C" void init_ssa(SuifEnv*);

#endif /* SSA_INIT_H */
