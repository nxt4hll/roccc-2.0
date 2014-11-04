/* file "alpha_halt/recipe.h" */

/*
    Copyright (c) 1998-2000 The President and Fellows of Harvard University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file.
*/

#ifndef ALPHA_HALT_RECIPE_H
#define ALPHA_HALT_RECIPE_H

#include <machine/machine.h>
#include <halt/halt.h>

extern Vector<HaltRecipe*> halt_recipes_alpha;

class SaveArea;

void init_halt_recipes_alpha(SaveArea*);
void clear_halt_recipes_alpha();

/*
 * Class SaveArea manages the per-opt-unit local array variable that's used
 * for saving and restoring registers at instrumentation points.
 *
 * A single instance, created by init_alpha_halt, is shared among all the
 * recipes.  Its constructor fixes the length (number of registers that can
 * be saved), but leaves the array variable symbol field NULL.
 *
 * The accessor methods return the current array-variable symbol (get_var)
 * and the length of the array it represents (get_length).  The mutator
 * method (refresh_var) obtains a variable symbol in the current OptUnit of
 * the desired length and stores it in the SaveArea instance.
 */

class SaveArea {
  public:
    SaveArea(int length) : var(NULL), length(length) { }

    VarSym* get_var() const { return var; }
    int get_length() const { return length; }
    void refresh_var() { var = new_empty_table_var(type_p64, length); }

  protected:
    VarSym *var;
    int length;
};

class HaltRecipeAlpha : public HaltRecipe { // still an abstract class
  public:
    HaltRecipeAlpha(SaveArea *save_area) : save_area(save_area) { }
    virtual ~HaltRecipeAlpha() { }

  protected:
    // target-specific helpers for use in operator()
    virtual void static_args(HaltLabelNote);
    virtual void build_save_set(NatSet *save, const NatSet *live);
    virtual void setup_stack();
    virtual void save_state(NatSet*);
    virtual void insert_args();
    virtual void insert_call(ProcSym*);
    virtual void clean_args();
    virtual void restore_state(NatSet*);
    virtual void destroy_stack();


    // Helper specific to Alpha

    Opnd save_area_addr(int pos) const;

    SaveArea *save_area;
};

class HaltRecipeAlphaStartup : public HaltRecipeAlpha {
  public:
    HaltRecipeAlphaStartup(SaveArea *save_area)
	: HaltRecipeAlpha(save_area) { }
    virtual ~HaltRecipeAlphaStartup() { }

    // target and instrumentation-kind specific handler
    virtual void operator()(HaltLabelNote, InstrHandle, CfgNode *,
			    const NatSet *before, const NatSet *after);
};

class HaltRecipeAlphaCbr : public HaltRecipeAlpha {
  public:
    HaltRecipeAlphaCbr(SaveArea *save_area)
	: HaltRecipeAlpha(save_area) { }
    virtual ~HaltRecipeAlphaCbr() { }

    // target and instrumentation-kind specific handler
    virtual void operator()(HaltLabelNote, InstrHandle, CfgNode *,
			    const NatSet *before, const NatSet *after);

    // helper routines
    virtual Opnd evaluate_cond(InstrHandle);
};

class HaltRecipeAlphaEntry : public HaltRecipeAlpha {
  public:
    HaltRecipeAlphaEntry(SaveArea *save_area)
	: HaltRecipeAlpha(save_area) { }
    virtual ~HaltRecipeAlphaEntry() { }

    // target and instrumentation-kind specific handler
    virtual void operator()(HaltLabelNote, InstrHandle, CfgNode *,
			    const NatSet *before, const NatSet *after);
};

class HaltRecipeAlphaExit : public HaltRecipeAlpha {
  public:
    HaltRecipeAlphaExit(SaveArea *save_area)
	: HaltRecipeAlpha(save_area) { }
    virtual ~HaltRecipeAlphaExit() { }

    // target and instrumentation-kind specific handler
    virtual void operator()(HaltLabelNote, InstrHandle, CfgNode *,
			    const NatSet *before, const NatSet *after);
};

class HaltRecipeAlphaMbr : public HaltRecipeAlpha {
  public:
    HaltRecipeAlphaMbr(SaveArea *save_area)
	: HaltRecipeAlpha(save_area) { }
    virtual ~HaltRecipeAlphaMbr() { }

    // target and instrumentation-kind specific handler
    virtual void operator()(HaltLabelNote, InstrHandle, CfgNode *,
			    const NatSet *before, const NatSet *after);
};

class HaltRecipeAlphaLoad : public HaltRecipeAlpha {
  public:
    HaltRecipeAlphaLoad(SaveArea *save_area)
	: HaltRecipeAlpha(save_area) { }
    virtual ~HaltRecipeAlphaLoad() { }

    // target and instrumentation-kind specific handler
    virtual void operator()(HaltLabelNote, InstrHandle, CfgNode *,
			    const NatSet *before, const NatSet *after);

    // helper routines
    virtual int num_bytes_ld(InstrHandle);
};


class HaltRecipeAlphaStore : public HaltRecipeAlpha {
  public:
    HaltRecipeAlphaStore(SaveArea *save_area)
	: HaltRecipeAlpha(save_area) { }
    virtual ~HaltRecipeAlphaStore() { }

    // target and instrumentation-kind specific handler
    virtual void operator()(HaltLabelNote, InstrHandle, CfgNode *,
			    const NatSet *before, const NatSet *after);

    // helper routines
    virtual int num_bytes_st(InstrHandle);
};

class HaltRecipeAlphaSetjmp : public HaltRecipeAlpha {
  public:
    HaltRecipeAlphaSetjmp(SaveArea *save_area)
	: HaltRecipeAlpha(save_area) { }
    virtual ~HaltRecipeAlphaSetjmp() { }

    // target and instrumentation-kind specific handler
    virtual void operator()(HaltLabelNote, InstrHandle, CfgNode *,
			    const NatSet *before, const NatSet *after);
};

class HaltRecipeAlphaLongjmp : public HaltRecipeAlpha {
  public:
    HaltRecipeAlphaLongjmp(SaveArea *save_area)
	: HaltRecipeAlpha(save_area) { }
    virtual ~HaltRecipeAlphaLongjmp() { }

    // target and instrumentation-kind specific handler
    virtual void operator()(HaltLabelNote, InstrHandle, CfgNode *,
			    const NatSet *before, const NatSet *after);
};

class HaltRecipeAlphaBlock : public HaltRecipeAlpha {
  public:
    HaltRecipeAlphaBlock(SaveArea *save_area)
	: HaltRecipeAlpha(save_area) { }
    virtual ~HaltRecipeAlphaBlock() { }

    // target and instrumentation-kind specific handler
    virtual void operator()(HaltLabelNote, InstrHandle, CfgNode *,
			    const NatSet *before, const NatSet *after);
};

class HaltRecipeAlphaCycle : public HaltRecipeAlpha {
  public:
    HaltRecipeAlphaCycle(SaveArea *save_area)
	: HaltRecipeAlpha(save_area) { }
    virtual ~HaltRecipeAlphaCycle() { }

    // target and instrumentation-kind specific handler
    virtual void operator()(HaltLabelNote, InstrHandle, CfgNode *,
			    const NatSet *before, const NatSet *after);
};


#endif	/* ALPHA_HALT_RECIPE_H */
