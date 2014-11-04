/* file "halt_blpp/kinds.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "halt_blpp/kinds.h"
#endif

#include <machine/machine.h>
#include <halt/halt.h>
#include <halt_blpp/kinds.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace halt;

// Define the string names.
void
init_halt_blpp_proc_names()
{
    halt_proc_names.resize(LAST_HALT_KIND+1, NULL);
    halt_proc_names[PATH_SUM_READ] = "_record_path_sum_read";
}

// Build the corresponding symbols.  Call after init_halt_blpp_proc_names.
void
init_halt_blpp_proc_syms()
{
    halt_proc_syms.resize(LAST_HALT_KIND + 1, NULL);
    halt_proc_syms[PATH_SUM_READ] =
	create_procedure_symbol(the_suif_env,
				create_procedure_type(the_suif_env, ""),
				halt_proc_names[PATH_SUM_READ], true, NULL);
}
