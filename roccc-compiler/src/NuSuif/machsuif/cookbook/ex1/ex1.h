/* file "ex1/ex1.h" -- Cookbook example #1 */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef EX1_EX1_H
#define EX1_EX1_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "ex1/ex1.h"
#endif


// This defines the substrate-independent OPI class.
class Ex1 {
  public:
    Ex1() { }

    void initialize() { printf("Running initialize()\n"); }
    void do_opt_unit(OptUnit*);
    void finalize() { printf("Running finalize()\n"); }
};

#endif /* EX1_EX1_H */
