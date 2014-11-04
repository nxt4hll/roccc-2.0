/* file "raga/raga.h" */

/*
    Copyright (c) 1995-99 The President and Fellows of Harvard University

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef RAGA_RAGA_H
#define RAGA_RAGA_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "raga/raga.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <cfa/cfa.h>
#include <bvd/bvd.h>

#include <raga/libra.h>

/*
 * Raga is a graph coloring register allocator that uses the method of
 * George and Appel [ToPLaS, May 1996].  It runs after any of the machsuif
 * code generators (e.g., agen) and must be followed by a code finalization
 * pass (e.g., afin).
 */

class RagaNode;
class MoveInfo;
typedef List<RagaNode*> NodeList;
typedef NodeList::iterator NodeListHandle;

typedef enum { NOSTATE, INITIAL, COLORED, PRECOLORED, COALESCED, SPILLED,
	       SIMPLIFY_WORK, FREEZE_WORK, SPILL_WORK, SELECT_STACKED,
	       ACTIVE, WORKLIST, CONSTRAINED, FROZEN }
  WorkState;

class Raga {
  public:
    void initialize();
    void do_opt_unit(OptUnit*);
    void finalize();

    // set pass options
    void set_file_name(const char *fn)	{ file_name = fn; }
    void set_debug_arg(int da)		{ debug_arg = da; }
    void set_reserve_t0(bool rt0)	{ reserve_t0 = rt0; }
    void set_note_spills(bool ns)	{ note_spills = ns; }
    void set_min_caller_saved(bool mcs) { min_caller_saved = mcs; }
    void set_just_coalesce(bool jc)	{ just_coalesce = jc; }
    void set_time_total(bool tt)	{ time_total = tt; }
    void set_time_each(bool te)		{ time_each = te; }
#ifdef NEIGHBOR_CENSUS
    void set_neighbor_census(bool nc)	{ neighbor_census = nc; }
#endif
    void add_debug_proc(String name)	{ debug_procs.insert(name); }

    bool has_reg_traits(Opnd);	// FIXME: should not have to be public

  protected:

    // Pass-option variables
    const char *file_name;	// Source file name
    int debug_arg;		// Debugging level
    bool reserve_t0;		// Don't allocate 0th GPR temp register
    bool note_spills;		// Annotate spill/reload instructions
    bool min_caller_saved;	// Don't do round-robin temp assignment
    bool just_coalesce;		// Coalesce but don't assign registers
    bool time_total;		// Measure total allocation time for file
    bool time_each;		// Measure allocation time for each proc
#ifdef NEIGHBOR_CENSUS
    bool neighbor_census;	// Maintain neighbor counts by class
#endif
    Set<IdString> debug_procs;	// Procedures that need debugging output

    // Per-file variables

    Vector<Vector<int> > *displacements;
				// Effect of neighbor on candidate class
    Vector<Vector<NatSetDense> > *aliases_in_class;
				// Reg's aliases that are also in class
    int class_count;		// Cache for reg_class_count()
    Vector<int> class_size;	// Map class ID c to reg_members(c).size()

    // Per-unit variables
    OptUnit *cur_unit;
    AnyBody *cur_body;
    Cfg *unit_cfg;
    RegSymCatalog *opnd_catalog;
    DefUseAnalyzer *du_analyzer;
    RegPartition *reg_partition;
    Liveness *liveness;
    HashMap<VarSym*,RagaNode> *volatiles_map;
    DominanceInfo *dominance_info;
    NaturalLoopInfo *loop_info;

    bool is_leaf;		// true if leaf procedure
    bool is_varargs;		// true if varargs procedure
				// dictionary of register candidates
    Vector<RagaNode*> node_table;
    RegClassMap opnd_classes;	// map from opnd index to class set

    // Helpers
    bool set_operand_bits(Opnd, NatSet*);
    RagaNode* record_reg(Opnd);
    int var_or_reg_node_id(Opnd);
    int var_node_id(VarSym*);
//  bool has_reg_traits(Opnd);  // (shoved up into the public interface)
    bool has_reg_candidate_traits(Opnd);
    void allocate_registers();
    void gather_candidates(Instr*);
    Opnd gather_var(Opnd, bool, InstrHandle);
    Opnd gather_reg(Opnd, bool, InstrHandle);
    Opnd spill_volatile(Opnd, bool, InstrHandle);
    void note_memory_EA(VarSym*);
    RagaNode* new_node_table_entry(Opnd, int id);
    void enter_hard_reg_nodes();

    // Principal functions used in coloring (names derived from G&A pseudocode)
    void assign_registers();	// Entry to coloring machinery
    void liveness_analysis();
    void init_coloring();
    void reinit_coloring();		// Prepare for each round of coloring
    void Main();			// Main loop; recolors till spills cease
#ifndef NO_LEUNG_GEORGE
    bool
#else
    void
#endif
	 add_edge(RagaNode*, RagaNode*);
    void build();		// Build interference graph
    void entry_interferences(CfgNode*); // Simulate parameter-candidate defs
    bool move_related(RagaNode*); // Is operand of coalesceable move
    void mk_worklist();		// Triage initial nodes to worklists
    void map_adjacent(RagaNode*, void (Raga::*)(RagaNode*, RagaNode*));
    void simplify();
    void decrement_degree(RagaNode*, RagaNode*);
    void enable_moves_adjacent(RagaNode*, bool hr_moves_only = false);
    void add_work_list(RagaNode*);
    bool is_in_conflict(RagaNode*, RagaNode*);
    bool adjacent_ok(RagaNode*, RagaNode*, MoveInfo*);
    bool adjacent_conservative(RagaNode*, RagaNode*, MoveInfo*);
    RagaNode *get_alias(RagaNode*);
    void combine(RagaNode*, RagaNode*);
    void coalesce();
    void transfer_edge(RagaNode*, RagaNode*);
    void freeze();
    void freeze_moves(RagaNode*);
    void select_spill();
    int spin(int color, int delta, int bound1, int bound2);
    void assign_colors();
    void propagate_alias_colors();
    void rewrite_program();
    void finish();			// G & A's `finalize'

    // Coloring helpers
    void add_squeeze(RagaNode*, RagaNode*);	// Helper for add_edge
    void relax_squeeze(RagaNode*, RagaNode*);	// Helper for decrement_degree

    // Variables used in coloring
    unsigned old_node_count;		// node_count at start of coloring round
    unsigned move_mentioned_count;	// Number of reg candidates in moves

    unsigned node_count;		// Number of active node_table entries

    // Node work lists
    RagaNode *initial;
    RagaNode *simplify_worklist;
    RagaNode *freeze_worklist;
    RagaNode *spill_worklist;
    RagaNode *spilled_nodes;
    RagaNode *coalesced_nodes;
    RagaNode *colored_nodes;
    RagaNode *select_stack;

    // Move-instruction work lists
    MoveInfo *coalesced_moves;
    MoveInfo *constrained_moves;
    MoveInfo *frozen_moves;
    MoveInfo *worklist_moves;
    MoveInfo *active_moves;

    AdjacencyMatrix adj_set;

    Vector<MoveInfo*> node_moves[2];	// Per opnd: movee_index -> move list

    NodeList current_spillees;		// Spillees of current instruction

    // Utilities local to coloring
    void eliminate_move_instr(MoveInfo*);
    void add_node_move(RagaNode*, MoveInfo*, int link_index);
    MoveInfo* pop_node_move(RagaNode*);
    bool unlink_node_move(RagaNode*, MoveInfo*, int link_index);
    void remove_node_move(RagaNode*, MoveInfo*);
    void push_move(MoveInfo*, MoveInfo*&, WorkState);
    MoveInfo* pop_move(MoveInfo*&, WorkState);
    void remove_move(MoveInfo*, MoveInfo*&, WorkState);
    void push_node(RagaNode*, RagaNode*&, WorkState);
    RagaNode* pop_node(RagaNode*&, WorkState);
    void remove_node(RagaNode*, RagaNode*&, WorkState);
    int var_or_reg_assigned_reg(Opnd);
    void clear_current_spillees(Instr* = NULL);	// FIXME: unneeded
    void classify_after_spill(Instr* = NULL);
    Opnd replace_and_maybe_spill(Opnd, bool is_src, InstrHandle);
    Opnd replace_and_spill(RagaNode*, bool is_src, InstrHandle,
			   bool true_spill);
    void register_temps_and_classify_after_spill(Instr* = NULL);
    void maybe_load_params();
    Opnd substitute_register(Opnd, bool, InstrHandle);
    Opnd substitute_alias(Opnd, bool, InstrHandle);

    // Debugging utilities for coloring
    void print_interferences(FILE*);
    void print_node_table_entry(FILE*, int);
    void print_node_table_segment(FILE*, int, int, char*);
    void print_basic_blocks(FILE *fp);
    void print_nat_set(FILE*, NatSet*, int, int (*)(int) = NULL);
    int spin_by_color_shift(int);
    void print_assignment(FILE*, RagaNode*, int);
    void print_coalesced_nodes(FILE*);

    // Instrumentation of coloring
    long unit_time;		// coloring time (usec) for this unit
    long total_time;		// coloring time (usec) for this file
#ifdef METER
    unsigned edge_count;	// number of interference graph edges
    unsigned move_count;	// number of possibly-coalesceable moves
    long coalesce_time;		// time spent this coloring round in coalesce()
    long combine_time;		// time spent this coloring round in combine()
    long george_test_time;	// time spent in adjacent_ok()
    long briggs_test_time;	// time spent in adjacent_conservative()
    long enable_moves_time;	// time spent in enable_moves_once()
    long select_spill_time;	// time spent in select_spill()
    long add_edge_time;		// time spent in add_edge()
    unsigned visit_count;	// number of neighbor visits in coalesce()
    unsigned combine_count;	// number of moves actually coalesced
    unsigned briggs_test_count;	// number of calls on adjacent_conservative()
    unsigned george_test_count;	// number of calls on adjacent_ok()
    unsigned enable_moves_count;// number of calls on enable_moves_once()
    unsigned select_spill_count;// number of calls on select_spill()
    unsigned test_spill_count;	// number of nodes tested on select_spill()
    unsigned add_edge_count;	// number of calls on add_edge()

    void print_coalesce_counts();
#endif // METER

    void print_unit_stats();	// print measurements for this unit
#ifdef NEIGHBOR_CENSUS
    void validate_census();	// check class_degree maps against reality
#endif

};


/* RagaNode -- Entry type for the table of register candidates, which
 * include the precolored (hardware) registers, plus virtual registers and
 * local variables.  These are called "nodes" by G & A because they become
 * nodes in the interference graph.
 */
#ifdef RACELL_BUG_FIXED
class RagaNode : public RaCell {
  public:
#else
class RagaNode {
  public:
    unsigned id;
    float frequency;		// weighted occurrence count
    int color;			// -1 or index in "color" map
    Opnd opnd;			// Corresponding HR, VR, or LV operand
    RagaNode *temp;		// Point-life VR: holds this value if spilled
#endif
    RagaNode()		: opnd(opnd_null()) { init(); }
    RagaNode(Opnd opnd) : opnd(opnd)	    { init(); }

    void init();		// Initialize all fields except `opnd'

    WorkState state;		// Usually indicates membership in a worklist
    RagaNode *prev;		// Endogenous, doubly-linked list ..
    RagaNode *next;		// ..  in prev and next fields
    int squeeze;		// Placement constraint from all neighbors
    float antisqueeze;		// Benefit to neighbors of node's removal
#ifdef NEIGHBOR_CENSUS
    Vector<int>class_degree;	// Map class to count of neighbors of that class
#endif
#ifndef IG_INCLUDES_HR
    NatSetDense excluded;	// Registers not available to this node
#endif
    RegClassId class_id;	// ID of register class for this candidate
    NodeList adj_list;		// Nodes that interfere with this LV or VR
    int movee_index;		// Move id if move-related; -1 otherwise
    RagaNode *alias;		// For COALESCED nodes:  node coalesced into
};

class MoveLink {
  public:
    MoveInfo *prev;
    MoveInfo *next;
};

class MoveInfo {
  public:
    MoveInfo(InstrHandle h, RagaNode *d, RagaNode *s) {
	state = INITIAL;
	handle = h;
	node[0] = d;
	node[1] = s;
	link[0].prev = link[0].next = NULL;
	link[1].prev = link[1].next = NULL;
	prev = next = NULL;
#ifdef METER
	coalesce_count = 0;
#endif
	hi = 0;
    }

    WorkState state;		// Usually indicates membership in a worklist
    InstrHandle handle;		// Handle on the move within its CFG node
    RagaNode *node[2];		// Aliases of operands of the move
    MoveLink link[2];		// Links for lists of moves of the two operands
    MoveInfo *prev;		// Endogenous, doubly-linked list ..
    MoveInfo *next;		// .. in prev and next fields
#ifdef METER
    unsigned long coalesce_count; // Number of attempts to coalesce this move
#endif
    int hi;			// <= to # of hi-degree nodes blocking coalesce
};

extern NoteKey k_cycle_count;	// Annotation key for instrumentation
extern NoteKey k_spill_code;	// Annotation key for spill code added

#endif /* RAGA_RAGA_H */
