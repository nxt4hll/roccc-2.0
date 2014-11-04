/* file "ex2/ex2.h" -- Cookbook example #2 */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef EX2_EX2_H
#define EX2_EX2_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "ex2/ex2.h"
#endif


class Ex2 {
  public:
    Ex2() { }

    void initialize() { printf("Running initialize()\n"); }
    void do_opt_unit(OptUnit*);
    void finalize() { printf("Running finalize()\n"); }
};

#endif /* EX2_EX2_H */
