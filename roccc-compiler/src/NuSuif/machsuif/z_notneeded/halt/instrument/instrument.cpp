/* file "instrument/instrument.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "instrument/instrument.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <bvd/bvd.h>
#include <halt/halt.h>

#include <instrument/instrument.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

//  Local helpers

void print_basic_blocks(FILE*, Cfg*, Liveness*, int limit);
void print_nat_set(FILE*, NatSet*, int limit);


void
InstrumentPass::initialize()
{
    install_halt_proc_syms();
}

// This routine sets up the operand catalog so that the slot id
// of a hard register is equal to its abstract register number.
// We do this so that we don't have to keep a separate mapping
// from slots ids to abstract register numbers.
void
InstrumentPass::warm_catalog(OpndCatalog *cat)
{
    unsigned int num_hard_regs = reg_count();

    int slot;
    for(unsigned int i = 0; i < num_hard_regs; i++)
	cat->enroll(opnd_reg(i, type_v0), &slot);
}

void
InstrumentPass::do_opt_unit(OptUnit *unit)
{
    IdString name = get_name(get_proc_sym(unit));
    debug(2, "Processing procedure \"%s\"", name.chars());
    bool is_main_proc = (strcmp(name.chars(), main_name) == 0);
    HaltRecipe* recipe;

    // this pass requires a unit's body to be a CFG
    claim(is_kind_of<Cfg>(get_body(unit)), "Body is not in CFG form");
    cfg = static_cast<Cfg*>(get_body(unit));

#ifdef CANONICALIZE_BUG_FIXED
    // Establish CFG preferences: nodes are maximal basic blocks and
    // not broken at calls, and the initial layout is retained.
    canonicalize(cfg, true, false, false);
#endif

    // build structures used by live-var analysis
    opnd_catalog = new RegSymCatalog(true, is_hard_reg);
    warm_catalog(opnd_catalog);			// map abstract regs to slots
    du_analyzer = new DefUseAnalyzer(opnd_catalog);
    liveness = new Liveness(cfg, opnd_catalog, du_analyzer);

    if_debug(4){
	fprintf(stderr, "Operand catalog:  --------------------------------\n");
	opnd_catalog->print(stderr);
	fprintf(stderr, "\n");
	print_basic_blocks(stderr, cfg, liveness, opnd_catalog->size());
    }

    halt_begin_unit(unit);

    // As we scan the instructions of a block, we want liveness info at the
    // current point of scan.  Variable `live' is the set of slot numbers
    // representing live entities at each point.  It is initialized at the
    // bottom of each block and updated for each instruction in the
    // backward scan over the block.
    for (int i = 0; i < nodes_size(cfg); ++i) {
	CfgNode *b = get_node(cfg, i);
	debug(6, "visiting cfg node %d", i);

	NatSetDense liveb, livea;	// live before and after instr
	liveb = *liveness->out_set(b);	// live set at block exit
	livea = *liveness->out_set(b);

	InstrHandle h = last(b);	// start at bottom of block
	int bsize = size(b);
	for(int i = 0; i < bsize; i++, livea = liveb) {
	    Instr *mi = *h;

	    // extract defs/uses and calculate the "live-before" set
	    du_analyzer->analyze(mi);
	    liveb -= *du_analyzer->defs_set();
	    liveb += *du_analyzer->uses_set();

	    // Bump the instruction handle so we don't analyze the
	    // instructions that we're about to insert (assuming
	    // we find a HALT annotation.
	    InstrHandle cur_h = h;
	    h--;

	    // process any HALT annotations
	    while(has_note(mi, k_halt)) {
		HaltLabelNote n = take_note(mi, k_halt);
		long k = n.get_kind();
		debug(4, "inserting instrumentation kind %ld\n", k);

		recipe = halt_recipe(k);
		if (recipe)
		    (*recipe)(n, cur_h, b, &liveb, &livea);
	    }

	    // If this is the main function of the program, instrument
	    // STARTUP.  This comes after processing HALT notes so that
	    // startup code will be inserted before other instrumentation.
	    if (is_main_proc && has_note(mi, k_proc_entry)) {
		debug(6, "labeling program startup");

		// Skip header instrs that might have been added by fin.
		InstrHandle entry = cur_h;

		for (InstrHandle h2 = entry; ++h2 != end(b); /* */)
		    if (has_note(*h2, k_header_trailer))
			entry = h2;
		    else
			break;

		HaltLabelNote note(halt::STARTUP, 0);
		recipe = halt_recipe(halt::STARTUP);
		if (recipe)
		    (*recipe)(note, entry, b, &liveb, &livea);
	    }
	}
    }

    halt_end_unit(unit);

    delete opnd_catalog;
    delete du_analyzer;
    delete liveness;

    // simplify the CFG if possible
    while(optimize_jumps(cfg)		||
	  merge_node_sequences(cfg)	||
	  remove_unreachable_nodes(cfg))
    { }
}

/*  -------------------------  Debugging helpers  -------------------------  */

void
print_basic_blocks(FILE *fp, Cfg *cfg, Liveness *liveness, int limit)
{
    fprintf(fp, "\nBasic blocks:\n");

    NatSetDense live_set;
    target_printer()->set_omit_unwanted_notes(true);

    for (CfgNodeHandle h = nodes_start(cfg);
	 h != nodes_end(cfg); ++h) {

	CfgNode *cnode = *h;

	fprintf(fp, "Block %u  -----------------------------------------\n",
		get_number(cnode));

	live_set.remove_all();
	live_set += *liveness->in_set(cnode);

	fprintf(fp, "Live coming in: ");
	print_nat_set(fp, &live_set, limit);
	putc('\n', fp);

	InstrHandle h = start(cnode);		// move forward thru instrs
	for (int i = 0; i < size(cnode); ++i, ++h) {
	    fprintf(fp, "[%lx]: ", (unsigned long)*h);
	    fprint (fp, *h);
	}

	live_set.remove_all();
	live_set += *liveness->out_set(cnode);

	fprintf(fp, "Live going out: ");
	print_nat_set(fp, &live_set, limit);
	putc('\n', fp);
   }
}

void
print_nat_set(FILE *fp, NatSet *ns, int limit)
{
    NatSetIter nsi = ns->iter();
    int e1 = limit, e2;
    char separator = '\0';

    for ( ; nsi.is_valid() && (e2 = nsi.current()) < limit; nsi.next()) {
	if ((separator != '-' && e1 < limit)
	 || (separator == '-' && e1 != e2 - 1)) {
	    fprintf(fp, "%.1s%d", &separator, e1);
	    separator = (e1 == e2 - 1) ? '-' : ',';
	}
	e1 = e2;
    }
    if (e1 < limit)
	fprintf(fp, "%.1s%d", &separator, e1);
}
