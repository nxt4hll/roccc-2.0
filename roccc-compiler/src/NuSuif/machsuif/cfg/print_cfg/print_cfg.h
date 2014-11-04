/* file "print_cfg/print_cfg.h" -- print out intermiedate Cfg form */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef PRINT_CFG_PRINT_CFG_H
#define PRINT_CFG_PRINT_CFG_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "print_cfg/print_cfg.h"
#endif

#include <machine/machine.h>

class PrintCfg {
  public:
    PrintCfg() { }

    void initialize() { }
    void do_opt_unit(OptUnit*);
    void finalize() { }

    // set pass options 
    void set_show_layout(bool sl)                { show_layout = sl; }
    void set_show_code(bool sc)             { show_code = sc; }

  protected:
    bool show_layout;
    bool show_code;
};

#endif /* PRINT_CFG_PRINT_CFG_H */
