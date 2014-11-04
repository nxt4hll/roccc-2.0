/* file "alpha/code_fin.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef ALPHA_CODE_FIN_H
#define ALPHA_CODE_FIN_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "alpha/code_fin.h"
#endif

class CodeFinAlpha : public CodeFin {    
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
    NatSetDense saved_reg_set[2];	// saved-regs used in each register file
};

#endif /* ALPHA_CODE_FIN_H */
