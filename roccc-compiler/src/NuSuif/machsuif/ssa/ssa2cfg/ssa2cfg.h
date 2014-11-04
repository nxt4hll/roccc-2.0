/* file "ssa2cfg/ssa2cfg.h" -- Convert from Cfg to InstrList form */

/*
    Copyright (c) 2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef SSA2CFG_SSA2CFG_H
#define SSA2CFG_SSA2CFG_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "ssa2cfg/ssa2cfg.h"
#endif

#include <machine/machine.h>

class Ssa2cfg {
  public:
    Ssa2cfg() { }

    void initialize() { }
    void do_opt_unit(OptUnit*);
    void finalize() { }

    void set_restore_orig_names(bool ron) { restore_orig_names = ron; }

  protected:
    bool restore_orig_names;
};

#endif /* SSA2CFG_SSA2CFG_H */
