/* file "raga/raga.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "raga/raga.h"
#endif

#include <time.h>
#include <sys/time.h>
#include <math.h>

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <cfa/cfa.h>
#include <bvd/bvd.h>

#include <raga/libra.h>
#include <raga/raga.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif


/*
 * Register Allocation a la George & Appel  (ToPLaS, May 1996)
 */


#ifdef PERTURB_FREQ
/*
 * Generate random numbers with "standard normal" distribution, i.e.,
 * Gaussian with zero mean and unity standard deviation.
 *
 * Lifted from <URL:http://www.taygeta.com/random/gaussian.html>.
 *
 */
double
normal_random()
{
    double x1, x2, w, y1;
    static double y2;
    static int use_last = 0;

    if (use_last)		        /* use value from previous call */
    {
	y1 = y2;
	use_last = 0;
    } else {
	do {
	    x1 = 2.0 * drand48() - 1.0;
	    x2 = 2.0 * drand48() - 1.0;
	    w = x1 * x1 + x2 * x2;
	} while (w >= 1.0);

	w = sqrt((-2.0 * log(w) ) / w);
	y1 = x1 * w;
	y2 = x2 * w;
	use_last = 1;
    }
    return(y1);
}
#endif // PERTURB_FREQ

void
RagaNode::init()
{
#ifndef RACELL_BUG_FIXED
    id = 0;
    frequency = 0.0;
    color = 0;
    temp = 0;
#endif
    state = NOSTATE;
    prev = NULL;
    next  = NULL;
    squeeze = 0;
    clear(adj_list);
    movee_index = -1;
    alias = NULL;
}

void
Raga::initialize()
{
    if (debug_procs.empty())
	debuglvl = debug_arg;
    debug(1, "Debug level is %d", debuglvl);
    debug(1, "Processing source file %s", file_name);

    ::note_spills = note_spills;	// flag to libra

    opnd_classes.resize(1000);

    if (time_total)
	total_time = 0L;

    displacements = class_displacements();
    aliases_in_class = aliases_modulo_class();
    class_count = reg_class_count();
    class_size.resize(class_count);
    for (int i = 0; i < class_count; ++i)
	class_size[i] = reg_members(i)->size();
}

void
Raga::do_opt_unit(OptUnit *unit)
{
    cur_unit = unit;
    IdString name = get_name(get_proc_sym(unit));
    if (debug_procs.find(name) != debug_procs.end())
	debuglvl = debug_arg;
    debug(2, "Translating procedure \"%s\"", name.chars());

    if_debug(3)
	audit_opnds(stderr, "Initial ");

    // Get procedure attributes: is_leaf, is_varargs
    StackFrameInfoNote sfi_note = get_note(unit, k_stack_frame_info);
    claim(!is_null(sfi_note), "Missing stack_frame_info");

    is_leaf = sfi_note.get_is_leaf();
    is_varargs = sfi_note.get_is_varargs();

    // This pass requires a unit's body to be a CFG.  Be sure that a
    // preceding pass has left it in that form.

    claim(is_kind_of<Cfg>(get_body(cur_unit)), "Body is not in CFG form");
    unit_cfg = static_cast<Cfg*>(get_body(cur_unit));
    if_debug(5)
	fprint(stderr, unit_cfg, false, true);		// no layout, just code

    // Compute liveness information for the current procedure's registers
    // and potentially register-resident variables.  Here
    //
    //   opnd_catalog	maps operands of interest to bit-vector slots,
    //   du_analyzer	identifies the defs/uses of interest in a single
    //			instruction, and
    //   liveness	holds per-basic-block liveness results.

    opnd_catalog = NULL;
    du_analyzer = NULL;
    liveness = NULL;		// computed during coloring

    //   volatiles_map  maps volatile vars to non-candidate "nodes"
    //			used in spill_volatile
    //   dominance_info	is used for finding natural loops
    //   loop_info	is used for computing potential spill costs

    volatiles_map = NULL;
    dominance_info = NULL;
    loop_info = NULL;

    spill_load_count = spill_store_count = 0;

    allocate_registers();

    debug(2, "Spills: %d loads and %d stores",
	  spill_load_count, spill_store_count);

    delete opnd_catalog;
    delete du_analyzer;
    delete liveness;
    delete volatiles_map;
    delete dominance_info;
    delete loop_info;

    if_debug(5) {
	fprintf(stderr, "\nFinal CFG:\n");
	fprint(stderr, unit_cfg, false, true);		// no layout, just code
    }
    if_debug(3)
	audit_opnds(stderr, "Final ");
    if (debug_procs.find(name) != debug_procs.end())
	debuglvl = 0;
}

void
Raga::finalize()
{
    if (time_total)
	fprintf(stderr, "%-32s%9.3f seconds\n\n", file_name,
		(double)total_time / 1.0e6);
    delete displacements;
    delete aliases_in_class;
}


NoteKey k_cycle_count;			// Annotation key for instrumentation
NoteKey k_spill_code;			// Annotation key for spill code added

// FIXME: the following statics represent a hack to allow a method of class Raga
// to be used as a (non-member) functional argument.

static Raga *this_raga;
static bool
has_reg_traits_fn(Opnd opnd)
{
    return this_raga->has_reg_traits(opnd);
}

void
Raga::allocate_registers()
{
    struct timeval start_all, end_all; 
    if (time_total)
	gettimeofday(&start_all, NULL); 

    node_table.clear();
    node_count = 0;

    delete opnd_catalog;
    this_raga = this;
    opnd_catalog = new RegSymCatalog(debuglvl > 0, has_reg_traits_fn);

    for (int i = opnd_classes.size(); i > 0; /* */)
	opnd_classes[--i] = REG_CLASS_ANY;

    enter_hard_reg_nodes();

    volatiles_map = new HashMap<VarSym*,RagaNode>(512);

    dominance_info = new DominanceInfo(unit_cfg);
    dominance_info->find_dominators();

    loop_info = new NaturalLoopInfo(dominance_info);
    loop_info->find_natural_loops();

    scan_edit_code(this, unit_cfg, &Raga::gather_candidates,
		   &Raga::classify_after_spill, NULL, &Raga::gather_var,
		   &Raga::gather_reg, &Raga::spill_volatile,
		   &Raga::note_memory_EA);
    clear_current_spillees();			// FIXME: no longer needed

#ifdef PERTURB_FREQ
    // Perturb occurrence frequencies by a randomly-chosen
    // factor close to one.

    node_count = node_table.size();

    for (size_t i = 0; i < node_count; ++i)
	if (RagaNode *e = node_table[i])
	    e->frequency *= exp(0.2 * normal_random());
#endif // PERTURB_FREQ

    if_debug(6) {
	printf("Operand catalog:\n");
	opnd_catalog->print();
	printf("\n");
    }

    delete du_analyzer;
    reg_partition = new RegPartition(opnd_catalog);
    du_analyzer = new DefUseAnalyzer(opnd_catalog, reg_partition);

    assign_registers();

    for (int i = node_table.size() - 1; i >= 0; i--)
	delete node_table[i];

    if (time_total) {
	gettimeofday(&end_all, NULL); 
	unit_time =
	    ((long)end_all.tv_sec  - start_all.tv_sec) * 1000000
	  + ((long)end_all.tv_usec - start_all.tv_usec);
	total_time += unit_time;
    }
    print_unit_stats();
}

/*
 * Collect and classify the register candidates in an instruction.
 *
 * This is done at the instruction level, rather than in a function that is
 * mapped over operands, because reg_classify needs to look at the whole
 * instruction in order to assign a class set to each register candidate.
 *
 * An operand is a valid candidate if it is acceptable to the operand catalog.
 *
 * Use the target-specific reg_classify function to assign a register class
 * set to each valid candidate.
 */
void
Raga::gather_candidates(Instr *instr)
{
    reg_classify(instr, opnd_catalog, &opnd_classes);
    clear_current_spillees();			// FIXME: no longer needed
}

/*
 * gather_var() - Make sure that a local-variable register candidate has an
 * entry in node_table.
 *
 * Make a node_table entry for a previously-unseen variable.
 */
Opnd
Raga::gather_var(Opnd opnd, bool, InstrHandle ih)
{
    VarSym *v = get_var(opnd);

    int node_id = var_node_id(v);
    claim(node_id >= 0);

    RagaNode *e;
    if ((size_t)node_id >= node_table.size()
	|| (e = node_table[node_id]) == NULL)
	e = new_node_table_entry(opnd, node_id);

    // Increase occurrence "count" by 8^(loop depth).
    e->frequency +=
	ldexp(1.0, 3 * loop_info->loop_depth(get_parent_node(*ih)));

    return opnd;
}

/* gather_reg() - Make sure that a register operand (hard or virtual) has
 * an entry in node_table.
 *
 * Make a node_table entry for a previously-unseen register.
 */
Opnd
Raga::gather_reg(Opnd opnd, bool, InstrHandle ih)
{
    RagaNode *e = record_reg(opnd);

    // If opnd is a register candidate, increase its occurrence "count" by
    // 8^(loop depth).
    if (e != NULL)
	e->frequency +=
	    ldexp(1.0, 3 * loop_info->loop_depth(get_parent_node(*ih)));
    return opnd;
}

/*
 * Spill a "volatile" variable, i.e. one that is not local or whose
 * address-taken attribute is true.
 *
 * Since the usual mechanism for avoiding redundant temps for the same
 * spillee in a single instruction depends on the existence of a node-table
 * entry, we use a fake entry that actually lives in volatiles_map.
 *
 * This code should be merged with part of replace_and_maybe_spill.  A
 * difference between the two is that spilled volatiles don't count as
 * producing spill code, since they aren't spilled because of failure to
 * find colors.
 *
 * NB: each newly-inserted spill temp must get classified correctly after
 * this routine has been used to replace an operand of the original
 * instruction.  The current_spillees list can be used to detect when such
 * a spill has occurred.
 */
Opnd
Raga::spill_volatile(Opnd opnd, bool is_src, InstrHandle h)
{
    VarSym *var = get_var(opnd);
    HashMap<VarSym*,RagaNode>::iterator it = volatiles_map->find(var);

    RagaNode &n = (it == volatiles_map->end())
		      ? volatiles_map->enter_value(var, RagaNode(opnd)).second
		      : (*it).second;

    return replace_and_spill(&n, is_src, h, false);	// not a candidate spill
}

void
Raga::note_memory_EA(VarSym *v)
{
    /*			// FIXME:  really needed?
	if (!vsym_used(v))
	    vsym_update_usage(v);
    */
}

// liveness_analysis() -- Invoked at the start of each coloring pass.
void
Raga::liveness_analysis()
{
    delete liveness;
    liveness = new Liveness(unit_cfg, opnd_catalog, du_analyzer);
    claim((size_t)opnd_catalog->num_slots() >= node_table.size());
}

RagaNode*
Raga::record_reg(Opnd opnd)
{
    int node_id = -1;
    opnd_catalog->enroll(opnd, &node_id);

    if (node_id < 0)
	return NULL;    

    RagaNode *e;
    if ((size_t)node_id >= node_table.size()
	|| (e = node_table[node_id]) == NULL)
    {
	e = new_node_table_entry(opnd, node_id);
    }
    else if (is_virtual_reg(opnd)
	     && get_bit_size(get_type(opnd)) > get_bit_size(get_type(e->opnd)))
    {
	// Be sure the RagaNode for a VR reflects the type of the widest value
	// it's expected to hold.  If it is spilled, this becomes the type of
	// its stack home.
	e->opnd = opnd;
    }
    return e;
}

/*
 * has_reg_traits(opnd)
 *
 * Return true iff operand is either an assignable register or is suitable
 * for assignment to a register.
 */
bool
Raga::has_reg_traits(Opnd opnd)
{
    if (is_hard_reg(opnd))
#if !defined(WIGLESS) || !defined(ALIGN_WIGLESS_IDS)
	return reg_allocables()->contains(get_reg(opnd));
#else
	return reg_allocables()->contains(get_reg(opnd))
	    || reg_widths()[get_reg(opnd)] == 8;
#endif
    return has_reg_candidate_traits(opnd);
}

/*
 * has_reg_candidate_traits(opnd)
 *
 * Return true iff operand is either a virtual register or a local variable
 * whose address is never taken.
 */
bool
Raga::has_reg_candidate_traits(Opnd opnd)
{
    if (is_var(opnd))
	return is_auto(get_var(opnd)) && !is_addr_taken(get_var(opnd));
    return is_virtual_reg(opnd);
}

int
Raga::var_or_reg_node_id(Opnd opnd)
{
    int node_id = -1;

    if (opnd_catalog->lookup(opnd, &node_id)) {
	claim(node_id >= 0);
	return node_id;
    }
    return -1;			// negative means no entry
}

int
Raga::var_node_id(VarSym *v)
{
    return var_or_reg_node_id(opnd_var(v));
}



RagaNode*
Raga::new_node_table_entry(Opnd opnd, int id)
{
    if ((size_t)id >= opnd_classes.size()) {
	warn("reg_classify missed a valid register candidate: id = %d", id);
	opnd_classes.resize(id+1, REG_CLASS_ANY);
    }
    maybe_expand(node_table, id, (RagaNode*)NULL);
    RagaNode *e = new RagaNode;
    node_table[id] = e;
    e->id = id;
    e->class_id = opnd_classes[id];
    e->frequency = 0.0;
    e->squeeze = 0;
    e->antisqueeze = 0.0;
#ifdef NEIGHBOR_CENSUS
    if (neighbor_census)
	e->class_degree.resize(class_count, 0);
#endif
    e->state = INITIAL;
    e->color = -1;
    e->opnd = opnd;
    e->next = e->prev = NULL;
    e->temp = NULL;
    clear(e->adj_list);
    e->movee_index = -1;
    e->alias = NULL;
    return e;
}

/*
 * enter_hard_reg_nodes
 *
 * Put entries in node_table for the allocatable regs.
 */
void
Raga::enter_hard_reg_nodes()
{
    const NatSet *regs = reg_allocables();
    Instr *dummy = new_instr_alm(opcode_null);

    for (NatSetIter it = regs->iter(); it.is_valid(); it.next()) {
	Opnd opnd = opnd_reg(it.current(), type_v0);
	int node_id = -1;
	bool fresh = opnd_catalog->enroll(opnd, &node_id);

	claim(fresh && (size_t)node_id == node_table.size());
	new_node_table_entry(opnd, node_id);
	set_src(dummy, 0, opnd);
	reg_classify(dummy, opnd_catalog, &opnd_classes);

	// FIXME: hack to align x86 HR-node IDs
#if defined(WIGLESS) && defined(ALIGN_WIGLESS_IDS)
	if (reg_widths()[it.current()] != 8)
	    continue;

	opnd = opnd_reg(it.current() + 4, type_v0); // corresponding H register
	node_id = -1;
	fresh = opnd_catalog->enroll(opnd, &node_id);

	claim(fresh && (size_t)node_id == node_table.size());
	new_node_table_entry(opnd, node_id);
#endif
    }
    delete dummy;
}

void
Raga::print_unit_stats()
{
    if (time_each) {
	IdString name = get_name(get_proc_sym(cur_unit));
	fprintf(stderr, "  %-30s", name.chars());

	fprintf(stderr, "%9.3f seconds\n", (double)unit_time / 1.0e6);
    }
}
