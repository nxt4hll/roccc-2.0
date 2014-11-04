/* file "make_2opnd/make_2opnd.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef MAKE_2OPND_MAKE_2OPND_H
#define MAKE_2OPND_MAKE_2OPND_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "make_2opnd/make_2opnd.h"
#endif

/*
 * make_2opnd ensures that every instruction satisfying the is_two_opnd
 * predicate has equal first source and destination operands.
 */

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <bvd/bvd.h>

class Make_2opnd {
  public:
    Make_2opnd() { }

    void initialize();
    void do_opt_unit(OptUnit*);
    void finalize() { }

    // set pass options
    void set_debug_arg(int da)		{ debuglvl = da; }

  protected:

    // Per-pass variables
    List<IdString> files;		// Names of source files in file set

    // Per-unit variables
    Cfg			*unit_cfg;
};

#endif /* MAKE_2OPND_MAKE_2OPND_H */
