/* file "x86_halt/contexts.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef X86_HALT_CONTEXT_H
#define X86_HALT_CONTEXT_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "x86_halt/contexts.h"
#endif

#include <machine/machine.h>
#include <halt/halt.h>
#include <x86/x86.h>


class HaltContextX86 : public HaltContext {
  public:
    HaltContextX86();
    virtual ~HaltContextX86();

    virtual HaltRecipe *halt_recipe(int) const;
};

class X86HaltContext : public virtual X86Context,
		       public virtual HaltContextX86
{ };

#endif /* X86_HALT_CONTEXT_H */
