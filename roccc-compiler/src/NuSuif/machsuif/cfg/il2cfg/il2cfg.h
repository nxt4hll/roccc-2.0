/* file "il2cfg/il2cfg.h" -- Convert from InstrList to Cfg form */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef IL2CFG_IL2CFG_H
#define IL2CFG_IL2CFG_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "il2cfg/il2cfg.h"
#endif

#include <machine/machine.h>

class Il2cfg {
  public:
    Il2cfg() { }

    void initialize() { }
    void do_opt_unit(OptUnit*);
    void finalize() { }

    // set pass options
    void set_keep_layout(int kl)		{ keep_layout = kl; }
    void set_break_at_call(int bac)		{ break_at_call = bac; }
    void set_break_at_instr(int bai)		{ break_at_instr = bai; }

  protected:
    // Pass-option variables
    bool keep_layout;		// flags to cfg creator
    bool break_at_call;
    bool break_at_instr;
};

#endif /* IL2CFG_IL2CFG_H */
