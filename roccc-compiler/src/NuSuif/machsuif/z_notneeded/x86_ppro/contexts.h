/* file "x86_ppro/contexts.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef X86_PPRO_CONTEXT_H
#define X86_PPRO_CONTEXT_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "x86_ppro/contexts.h"
#endif

#include <machine/machine.h>
#include <x86/x86.h>


class SuifVmContextX86PPro : public SuifVmContextX86 {
  public:
    CodeGen* target_code_gen() const;
};

class MachineContextX86PPro : public MachineContextX86 {
  public:
    Printer* target_printer() const;

    bool is_cmove(Instr*) const;
};

class X86PProContext : public virtual Context,
		       public virtual MachineContextX86PPro,
		       public virtual SuifVmContextX86PPro
{ };



#endif /* X86_PPRO_CONTEXT_H */
