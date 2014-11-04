/* file "cfg/init.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef CFG_INIT_H
#define CFG_INIT_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "cfg/init.h"
#endif

extern "C" void init_cfg(SuifEnv* suif_env);

extern IdString k_cfg_node;

#endif /* CFG_INIT_H */
