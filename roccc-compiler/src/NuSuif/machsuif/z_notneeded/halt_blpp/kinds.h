/* file "halt_blpp/kinds.h" -- extra HALT instrumentation kinds */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef HALT_BLPP_KINDS_H
#define HALT_BLPP_KINDS_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "halt_blpp/kinds.h"
#endif

#include <halt/halt.h>

// Extra instrumentation kinds needed by path profiling

namespace halt {
    enum {
	PATH_SUM_INIT = LAST_HALT_KIND + 1, // initialize path sum
	PATH_SUM_INCR,			    // adjust path sum
	PATH_SUM_READ,			    // use path sum to record path
    };
} // namespace halt

#undef LAST_HALT_KIND
#define LAST_HALT_KIND halt::PATH_SUM_READ

void init_halt_blpp_proc_names();
void init_halt_blpp_proc_syms();

#endif /* HALT_BLPP_KINDS_H */
