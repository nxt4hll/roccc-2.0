/* file "ex4/ex4.h" -- Cookbook example #4 */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef EX4_EX4_H
#define EX4_EX4_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "ex4/ex4.h"
#endif


class Ex4 {
  public:
    Ex4() { }

    void initialize();
    void do_opt_unit(OptUnit*);
    void finalize();

    // set pass options
    void set_reserved_reg(IdString n)	{ reserved_reg_name = n; }

  protected:
    // pass-option variables
    IdString reserved_reg_name;	// name of register to reserve (optional)

    // initialization variables
    int reserved_reg;		// its abstract number
    TypeId reserved_reg_type;	// its generic type
    Opnd reserved_reg_opnd;	// it as an register operand

    // markers for inserted instructions
    NoteKey k_reserved_reg_load;
    NoteKey k_store_reserved_reg;
};

#endif /* EX4_EX4_H */
