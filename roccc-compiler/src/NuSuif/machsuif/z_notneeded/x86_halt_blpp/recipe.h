/* file "x86_halt_blpp/recipe.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef X86_HALT_BLPP_RECIPE_H
#define X86_HALT_BLPP_RECIPE_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "x86_halt_blpp/recipe.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <halt/halt.h>
#include <x86_halt/x86_halt.h>


void init_halt_blpp_recipes_x86();

class HaltRecipeX86PathSumInit : public HaltRecipeX86 {
  public:
    virtual void operator()(HaltLabelNote, InstrHandle, CfgNode*,
			    const NatSet *before, const NatSet *after);
};

class HaltRecipeX86PathSumIncr : public HaltRecipeX86 {
  public:
    virtual void operator()(HaltLabelNote, InstrHandle, CfgNode*,
			    const NatSet *before, const NatSet *after);
};

class HaltRecipeX86PathSumRead : public HaltRecipeX86 {
  public:
    virtual void operator()(HaltLabelNote, InstrHandle, CfgNode*,
			    const NatSet *before, const NatSet *after);
    virtual void restore_state(NatSet *saved_reg_set);

  private:
    long next_path_sum;
};

#endif /* X86_HALT_BLPP_RECIPE_H */
