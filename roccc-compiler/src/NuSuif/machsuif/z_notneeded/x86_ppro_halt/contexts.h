/* file "x86_ppro_halt/contexts.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef X86_PPRO_HALT_CONTEXT_H
#define X86_PPRO_HALT_CONTEXT_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "x86_ppro_halt/contexts.h"
#endif

class X86PProHaltContext : public virtual Context,
			   public virtual MachineContextX86PPro,
			   public virtual SuifVmContextX86PPro,
			   public virtual HaltContextX86
{ };

#endif /* X86_PPRO_HALT_CONTEXT_H */
