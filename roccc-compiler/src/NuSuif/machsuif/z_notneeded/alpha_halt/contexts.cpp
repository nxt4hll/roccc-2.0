/* file "alpha_halt/contexts.cpp" */

/*  
    Copyright (c) 2000 The President and Fellows of Harvard University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>

#pragma implementation "alpha_halt/contexts.h"

#include <machine/machine.h>
#include <alpha/alpha.h>
#include <halt/halt.h>

#include <alpha_halt/recipe.h>
#include <alpha_halt/contexts.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

HaltContextAlpha::HaltContextAlpha()
{
    // The size of the save area is a number of registers.  We never need
    // to save the two CONST0 regs or the stack pointer reg.  But we
    // sometimes need an extra place in the save area for transferring FPR
    // conditions to the GPR file.

    save_area = new SaveArea(reg_count_alpha() - 3 + 1);
    init_halt_recipes_alpha(save_area);
}

HaltContextAlpha::~HaltContextAlpha()
{
    // FIXME: may be called after the halt_recipes_alpha vector is already gone.
    //    clear_halt_recipes_alpha();

    delete save_area;
}

void
HaltContextAlpha::halt_begin_unit(OptUnit *unit)
{
    save_area->refresh_var();
}

HaltRecipe *
HaltContextAlpha::halt_recipe(int ik) const
{
    return halt_recipes_alpha[ik];
}
