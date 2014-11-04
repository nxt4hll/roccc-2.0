/* file "cfg2il/cfg2il.h" -- Convert from Cfg to InstrList form */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef CFG2IL_CFG2IL_H
#define CFG2IL_CFG2IL_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "cfg2il/cfg2il.h"
#endif

#include <machine/machine.h>

class Cfg2il {
  public:
    Cfg2il() { }

    void initialize() { }
    void do_opt_unit(OptUnit*);
    void finalize() { }
};

#endif /* CFG2IL_CFG2IL_H */
