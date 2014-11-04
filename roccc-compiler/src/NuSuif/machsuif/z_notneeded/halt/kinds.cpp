/* file "halt/kinds.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "halt/kinds.h"
#endif

#include <machine/machine.h>

#include <halt/kinds.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

/**
 ** Data structures and functions associated with the user-defined
 ** instrumentation procedures.  Some of the work done below is
 ** specific to the SUIF substrate.
 **/

// vectors for procedure names and symbols
Vector<char*> halt_proc_names;
Vector<ProcSym*> halt_proc_syms;

// Define the string names.
void
init_halt_proc_names()
{
    using namespace halt;

    halt_proc_names.resize(LAST_HALT_KIND + 1, NULL);

    halt_proc_names[STARTUP] =	"_halt_startup";
    halt_proc_names[CBR]     =	"_record_cbr";
    halt_proc_names[MBR]     =	"_record_mbr";
    halt_proc_names[ENTRY]    =	"_record_entry";
    halt_proc_names[EXIT]     =	"_record_exit";
    halt_proc_names[SETJMP]  =	"_record_setjmp";
    halt_proc_names[LONGJMP] =	"_record_longjmp";
    halt_proc_names[LOAD]    =	"_record_load";
    halt_proc_names[STORE]   =	"_record_store";
    halt_proc_names[BLOCK]	     =	"_record_block";
    halt_proc_names[CYCLE]   =	"_record_cycle";
}

// Build the corresponding symbols.  Call after init_halt_proc_names.
void
init_halt_proc_syms()
{
    halt_proc_syms.resize(LAST_HALT_KIND + 1, NULL);

    for (int i = 0; i <= LAST_HALT_KIND; i++) {
	halt_proc_syms[i] = create_procedure_symbol
	    (the_suif_env, create_procedure_type(the_suif_env, ""),
	     halt_proc_names[i], true, NULL);
    }
}

// ** OPI function **
// Install all of the procedure symbols into the external symbol table.
// Skip any NULL entries in halt_proc_syms.
void
install_halt_proc_syms()
{
    SymTable *st = external_sym_table();
    for (int i = halt_proc_syms.size() - 1; i >= 0; --i)
	if (halt_proc_syms[i])
	    st->append_symbol_table_object(halt_proc_syms[i]);
}

// ** OPI function **
ProcSym *
halt_proc_sym(int instrumentation_kind)
{
    claim((unsigned)instrumentation_kind < halt_proc_syms.size(),
	  "Instrumentation kind out of range");
    return halt_proc_syms[instrumentation_kind];
}
