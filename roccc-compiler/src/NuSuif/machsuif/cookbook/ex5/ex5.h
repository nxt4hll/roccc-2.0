/* file "ex5/ex5.h" -- Cookbook example #5 */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef EX5_EX5_H
#define EX5_EX5_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "ex5/ex5.h"
#endif


class Ex5 {
  public:
    Ex5() { }

    void initialize();
    void do_opt_unit(OptUnit*);
    void finalize();

  protected:
    // markers for inserted instructions
    NoteKey k_reserved_reg_load;
    NoteKey k_store_reserved_reg;
};

#endif /* EX5_EX5_H */
