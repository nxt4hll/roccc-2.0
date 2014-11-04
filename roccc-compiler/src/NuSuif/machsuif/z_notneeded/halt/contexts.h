/* file "halt/contexts.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef HALT_CONTEXT_H
#define HALT_CONTEXT_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "halt/contexts.h"
#endif

#include <halt/recipe.h>

void halt_begin_unit(OptUnit*);
void halt_end_unit  (OptUnit*);
HaltRecipe *halt_recipe(int);

class HaltContext {
  public:
    HaltContext() { }
    virtual ~HaltContext() { }

    virtual void halt_begin_unit(OptUnit*) { }
    virtual void halt_end_unit  (OptUnit*) { }
    virtual HaltRecipe *halt_recipe(int) const = 0;
};

#endif	/* HALT_CONTEXT_H */
