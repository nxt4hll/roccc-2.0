/* file "x86/code_fin.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef X86_CODE_FIN_H
#define X86_CODE_FIN_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "x86/code_fin.h"
#endif

class CodeFinX86 : public CodeFin {    
  public:
    virtual void init(OptUnit*);
    virtual void layout_frame();
    virtual void analyze_opnd(Opnd);
    virtual Opnd replace_opnd(Opnd);
    virtual void make_header_trailer();

  protected:
    // Subfunction for replace_opnd
    Opnd frame_addr(Opnd orig, Opnd adr_sym, int delta);

    int framesize;
    int frameoffset;
    int localoffset;
    NatSetDense saved_reg_set;		// saved-regs used in GPR register file
};

#endif /* X86_CODE_FIN_H */
