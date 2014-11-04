/* file "alpha_halt/contexts.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>

#ifndef ALPHA_HALT_CONTEXT_H
#define ALPHA_HALT_CONTEXT_H

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "alpha_halt/contexts.h"
#endif

#include <machine/machine.h>
#include <halt/halt.h>
#include <alpha_halt/recipe.h>


class HaltContextAlpha : public HaltContext {
  public:
    HaltContextAlpha();
    virtual ~HaltContextAlpha();

    virtual void halt_begin_unit(OptUnit*);
    virtual HaltRecipe* halt_recipe(int) const;

  protected:
    SaveArea *save_area;
};


class AlphaHaltContext : public virtual Context,
			 public virtual MachineContextAlpha,
			 public virtual SuifVmContextAlpha,
			 public virtual HaltContextAlpha
{ };

#endif /* ALPHA_HALT_CONTEXT_H */
