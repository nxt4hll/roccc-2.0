/* file "cfg2ssa/cfg2ssa.h" -- Convert from Cfg to InstrList form */

/*
    Copyright (c) 2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef CFG2SSA_CFG2SSA_H
#define CFG2SSA_CFG2SSA_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "cfg2ssa/cfg2ssa.h"
#endif

#include <machine/machine.h>

class Cfg2ssa {
  public:
    Cfg2ssa() { }

    void initialize() { }
    void do_opt_unit(OptUnit*);
    void finalize() { }

    void set_build_flags(unsigned bf)	{ build_flags = bf; }

  protected:
    unsigned build_flags;
};

#endif /* CFG2SSA_CFG2SSA_H */
