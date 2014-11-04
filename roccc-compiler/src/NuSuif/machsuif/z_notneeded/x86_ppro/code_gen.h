/* file "x86_ppro/code_gen.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef X86_PPRO_CODE_GEN_H
#define X86_PPRO_CODE_GEN_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "x86_ppro/code_gen.h"
#endif

#include <machine/machine.h>
#include <x86/x86.h>

class CodeGenX86PPro : public CodeGenX86 {    
  protected:
    virtual void translate_abs(Instr*);

    // helper routines
    virtual void translate_fp(int opc, Instr*);

  public:
    CodeGenX86PPro();
};

#endif /* X86_PPRO_CODE_GEN_H */
