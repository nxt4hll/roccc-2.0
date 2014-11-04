/* file "x86_halt/contexts.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "x86_halt/contexts.h"
#endif

#include <halt/halt.h>

#include <x86_halt/contexts.h>
#include <x86_halt/recipe.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

HaltContextX86::HaltContextX86()
{
    init_halt_recipes_x86();
}

HaltContextX86::~HaltContextX86()
{
    // FIX ME! The next line is commented just because of
    // a problem in the order of deletion
    // clear_halt_recipes_x86();
}

HaltRecipe*
HaltContextX86::halt_recipe(int ik) const
{
    return halt_recipes_x86[ik];
}
