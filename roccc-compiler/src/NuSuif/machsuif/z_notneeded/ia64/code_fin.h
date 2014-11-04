/* file "ia64/code_fin.h" */

/*
    Copyright (c) 2002 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef IA64_CODE_FIN_H
#define IA64_CODE_FIN_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "ia64/code_fin.h"
#endif

#include <machine/machine.h>
#include <ia64/reg_info.h>

class CodeFinIa64 : public CodeFin {
  public:
    virtual void init(OptUnit*);
    virtual void layout_frame();
    virtual void analyze_opnd(Opnd);
    virtual Opnd replace_opnd(Opnd);
    virtual void make_header_trailer();
    virtual void finalize();

  protected:
    // Subfunction for replace_opnd
    Opnd frame_addr(Opnd orig, Opnd adr_sym, int delta);

    int frame_size;		// size of memory-stack frame
    int frame_offset;		// running offset from SP of memory locals
    int save_area_size;		// size of memory area for saving registers
    int va_area_size;		// size of memory area for dumping unnamed args

    int max_in_reg;		// highest incoming argument reg for proc
    int max_local_reg;		// highest mentioned non-out GR stacked reg
    int max_out_reg;		// highest mentioned out reg

    VarSym *va_first_var;	// made-up var to be bound to 1st unnamed vararg

				// saved-regs used in each register file
    NatSetDense saved_reg_set[LAST_IA64_CLASS - ia64::CLASS_GR + 1];
};

#endif /* IA64_CODE_FIN_H */
