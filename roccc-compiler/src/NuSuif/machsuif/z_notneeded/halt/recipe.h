/* file "halt/recipe.h" */

/*
    Copyright (c) 2000-2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef HALT_RECIPE_H
#define HALT_RECIPE_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "halt/recipe.h"
#endif

#include <machine/machine.h>
#include <halt/kinds.h>
#include <halt/note.h>
#include <cfg/cfg.h>

namespace halt {
enum {
    SETUP = 0,	// code to build stack frame
    SAVE,	// save necessary register state
    KIND,	// create kind-specific arguments
    ARGS,	// argument setup
    CALL,	// call to user-provided instrumentation routine
    CLEAN,	// clean-up argument storage
    RESTORE,	// restore saved register state
    DESTROY,	// code to destroy the stack frame
    RECIPE_SIZE
};
} // namespace halt

namespace halt {
enum InsertPoint {
    BEFORE,
    AFTER
};
} // namespace halt

class HaltRecipe {
  public:
    HaltRecipe() { prepare_pot(); }
    virtual ~HaltRecipe() { /* scrub_pot(); */ } // (SUIF dtor-ordering problem)

    // insert target- and kind-specific instrumentation for one point
    virtual void
	operator()(HaltLabelNote note, InstrHandle handle, CfgNode *block,
		   const NatSet *live_before, const NatSet *live_after) = 0;
  protected:
    InstrList *instr_pot[halt::RECIPE_SIZE];    // code snippets for this...
						// ...instrumentation point
    Vector<Opnd> args;                          // args to analysis routine

    // target-independent helpers
    void prepare_pot();                         // initialize instr_pot
    void scrub_pot();                           // clean up instr_pot
    void follow_recipe(int kind,                // fill instr_pot by calling...
		       const NatSet *live);     // ... target-specific methods
    void insert_instrs(halt::InsertPoint,       // pour instr_pot into stream...
		       CfgNode*, InstrHandle);  // ...at indicated point

    // target-specific helper methods for use in operator()
    virtual void static_args(HaltLabelNote) = 0;
    virtual void build_save_set(NatSet *save, const NatSet *live) = 0;
    virtual void setup_stack() = 0;
    virtual void save_state(NatSet *save) = 0;
    virtual void insert_args() = 0;
    virtual void insert_call(ProcSym*) = 0;
    virtual void clean_args() = 0;
    virtual void restore_state(NatSet *save) = 0;
    virtual void destroy_stack() = 0;    
};

// This routine fills in each of the InstrLists.  The caller
// should have already set up args (both dynamic and static).
inline void
HaltRecipe::follow_recipe(int unique_id, const NatSet *live)
{
    NatSetDense save_set;
    build_save_set(&save_set, live);

    insert_args();
    clean_args();

    insert_call(halt_proc_sym(unique_id));

    save_state(&save_set);
    restore_state(&save_set);

    setup_stack();		// save_state may record the size of the state
    destroy_stack();
}
#endif /* HALT_RECIPE_H */
