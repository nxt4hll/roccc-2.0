/* file "halt/recipe.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "halt/recipe.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>

#include <halt/kinds.h>
#include <halt/recipe.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace halt;

void
HaltRecipe::prepare_pot()
{
    for (int i = 0; i < RECIPE_SIZE; i++)
	instr_pot[i] = new_instr_list();
}

void
HaltRecipe::scrub_pot()
{
    for (int i = 0; i < RECIPE_SIZE; i++) {
	claim(size(instr_pot[i]) == 0);
	delete instr_pot[i];
    }
}

void
HaltRecipe::insert_instrs(InsertPoint location, CfgNode *n, InstrHandle h)
{
    debug(2, "%s:insert_instrs", __FILE__);

    InstrList *il = new_instr_list();

    for (int i = 0; i < RECIPE_SIZE; i++) {
	while (size(instr_pot[i]) > 0)
	    append(il, remove(instr_pot[i], start(instr_pot[i])));
    }

    if (location == BEFORE) {
	while (size(il) > 0) {
	    Instr *mi = remove(il, start(il));
	    insert_before(n, h, mi);
	    if_debug(4) {
		fprintf(stderr, "  inserting: ");
		fprint(stderr, mi);
	    }
	}
    } else {
	while (size(il) > 0) {
	    Instr *mi = remove(il, last(il));
	    insert_after(n, h, mi);
	    if_debug(4) {
		fprintf(stderr, "  inserting: ");
		fprint(stderr, mi);
	    }
	}
    }

    delete il;
}
