/* file "halt/kinds.h" -- HALT instrumentation kinds */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef HALT_KINDS_H
#define HALT_KINDS_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "halt/kinds.h"
#endif

#include <machine/machine.h>

// Instrumentation kinds.  Inserted by a HALT labelling pass
// and then later expanded by a HALT instrumentation pass.

namespace halt {

enum {
    STARTUP = 0,	// whole-program entry
    CBR,		// conditional branch
    MBR,		// multi-way branch
    ENTRY,		// procedure entry
    EXIT,		// procedure exit
    SETJMP,		// setjmp call
    LONGJMP,		// longjmp call
    LOAD,		// reads memory
    STORE,		// writes memory
    BLOCK,			// basic block
    CYCLE		// cycle count
};
} // namespace halt

#define LAST_HALT_KIND halt::CYCLE


// extensible vectors based on instrumentation kinds
extern Vector<char*> halt_proc_names;
extern Vector<ProcSym*> halt_proc_syms;

// ** OPI functions **
void install_halt_proc_syms();
ProcSym* halt_proc_sym(int instrumentation_kind);

// following functions used only in halt/init.cc
void init_halt_proc_names();
void init_halt_proc_syms();

#endif /* HALT_KINDS_H */
