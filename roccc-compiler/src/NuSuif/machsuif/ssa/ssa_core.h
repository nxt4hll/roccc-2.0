/* file "ssa/ssa_core.h" -- transformers to and from SSA form */

/*
    Copyright (c) 2000-2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef SSA_SSA_CORE_H
#define SSA_SSA_CORE_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "ssa/ssa_core.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <cfa/cfa.h>
#include <bvd/bvd.h>
#include <ssa/ssa_ir.h>
#include <ssa/opi.h>

/*
 * Literature:   This implementation adheres quite closely to that developed by
 * the Massively Scalar Compiler Project at Rice University.  (See
 * <URL:http://www.cs.rice.edu/MSCP/>.)  A description of the Rice innovations
 * in SSA transformation is at <URL:http://www.cs.rice.edu/~harv/ssa.ps>.
 *
 *
 * Terminology:
 *
 * location:  A conventional variable or register, suitable for SSA
 * 	transformation.
 *
 * name:   (An Opnd.)  Either an original location operand suitable for SSA
 * 	replacement (an "old" name, or location name), or the SSA replacement
 * 	itself (a "new" name, or value name).
 *
 * id:	An integer that identifies a name.  There are location ids for
 * 	old names and value ids for SSA names.
 */


extern IdString k_phi_nodes;		// key for attaching phi-nodes to blocks

/*
 * Classes defined in ssa_core.cpp
 */
class LivenessInfo;
class PhiSrcMapEntry;
class NameStackEntry;
class CopyData;
class DominanceChild;
class AssignmentListEntry;

/*
 * An instance of class SsaCore gives access to all information about the
 * SSA form of a CFG.
 *
 * Method `build' transforms a CFG into SSA form.  It takes a simple bitset
 * of flags (see below) that control options.
 *
 * Method `restore' transforms the CFG to conventional form.  Depending on
 * optimizations performed at build time or by the client, it may or may
 * not be valid to restore the original names.  If argument `not_old_names'
 * is true, restoration of original names is suppressed.
 *
 * Method `loc_count' returns the number of original names replaced by SSA
 * names.  Method `value_count' returns the number of SSA names.
 *
 * Method `value_catalog' returns a map from SSA names to their integer
 * ids.  (OpndCatalog is defined in the `machine' library.)  The catalog's
 * `inverse' method implements the opposite map from ids to names.
 *
 * The `get_unique_def' and `get_def_use_chain' methods give use-def and
 * def-use "chains" for the SSA form. (In the latter case, the
 * corresponding option flags must have been passed to `build'.)  In
 * single-assignment form, the def for each use is unique, and so there's
 * not an actual use-def chain as such.
 *
 * Method `get_phi_nodes' maps from a basic block to the set of phi-nodes
 * for the block.  Method `get_def_block' gives the block for any
 * operation, whether instruction or phi-node.
 *
 * Method `record_use_swap' takes note of a replacement (`in' replacing
 * `out') among the use occurrences of SSA names in an instruction or
 * phi-node.  It doesn't alter the instruction or phi-node; it just updates
 * internal records (e.g., def-use chains) to reflect the swap.
 *
 *   NB: If the name being swapped in is a newly-created SSA name, its
 *   definition must already have been processed by record_def before
 *   record_use_swap is called.
 *
 * Method `record_all_uses' takes note of all the use occurrences within a
 * new instruction or phi-node.  It modifies internal records (e.g.,
 * def-use chains) to reflect insertion of the instruction or phi-node into
 * the code.
 *
 * Method `record_def' takes note of a new definition, in the form either
 * of an instruction or a phi-node.  Its first argument is the original
 * (non-SSA) name for the definee.  It does not change the instruction or
 * phi-node, but looks there to find the SSA name defined.  In the
 * instruction case, the method also receives the position of the relevant
 * destination in the instruction.
 *
 * There are also print methods for debugging.
 */
class SsaCore
{
  public:
    SsaCore(SsaCfg *ssa = NULL, bool is_ssa_cfg = false, bool is_deflated = false);
    ~SsaCore();

    void build(unsigned flags = 0);
						// transform to SSA form
    Cfg* restore(bool not_old_names = false);	// restore conventional form

    int get_loc_count() /* const */;		// number of original locations
    int get_value_count() /* const */;		// number of SSA defs (plus one)

    OpndCatalog* get_value_catalog() /* const */;
						// numbering map for SSA defs
    Opnd get_value_name(int value_id) /* const */;
						// map value_id to SSA name
    Operation get_unique_def(Opnd) /* const */;	// defining operation for value
    const List<Operation>&			// all operations that use value
	get_def_use_chain(Opnd) /* const */;

    const List<PhiNode*>& get_phi_nodes(CfgNode*) const;
						// all phi-nodes of basic block
    PhiNode* remove_phi_node(CfgNode*, PhiNode*);
						// remove phi from block
    PhiHandle append_phi_node(CfgNode*, PhiNode*);
						// attach phi-node to block
    CfgNode* get_def_block(int value_id)/* const */;
						// block holding value's definer

    void record_use_swap(Opnd out, Opnd in, Operation);
						// note use swap (in for out)
    void record_all_uses(Operation);		// note all uses in new operation
    void record_def(Opnd old_name, Operation, int dst_pos = 0);
						// note new SSA def for old_name

    void print_phi_nodes(FILE*, CfgNode*);
    void print_name_map(FILE*);
    void print(FILE*);

    void inflate();				// Expand to ram-resident form
    void deflate();				// Discard ram-only data
    void maybe_inflate();			// Inflate only if necessary

    OptUnit* get_unit() const;
    void set_unit(OptUnit *unit);

    Cfg* get_cfg() const;
    void set_cfg(Cfg *cfg);

    bool get_build_minimal_form() const;
    void set_build_minimal_form(bool build_minimal_form);

    bool get_build_semi_pruned_form() const;
    void set_build_semi_pruned_form(bool build_semi_pruned_form);

    bool get_build_pruned_form() const;
    void set_build_pruned_form(bool build_pruned_form);

    bool get_build_def_use_chains() const;
    void set_build_def_use_chains(bool build_def_use_chains);

    bool get_fold_copies() const;
    void set_fold_copies(bool fold_copies);

    bool get_omit_useless_phi_nodes() const;
    void set_omit_useless_phi_nodes(bool omit_useless_phi_nodes);

    bool get_report_undefined_locs() const;
    void set_report_undefined_locs(bool report_undefined_locs);

  protected:
    SsaCfg *ssa;			// file-resident part of SSA form
    bool is_ssa_cfg;			// true while CFG is in SSA form
    bool is_deflated;			// true => inflate() needs to be called

    unsigned _loc_count;		// virtual and symbolic registers
    unsigned def_count;			// number of original defs plus one
    unsigned block_count;		// basic blocks

    RegSymCatalog *old_opnd_catalog;	// original opnd to its numeric id
    RegSymCatalog *new_opnd_catalog;	// ssa opnd to numeric id

    PhiSrcMapEntry **phi_src_map;
    CfgNode ***edge_maps;		// edge_maps[b][j] = src j's pred of b

    Vector<Operation> def_table;	// def_table[value_id] = value's definer
    Vector<List<Operation> > def_use_chains;
					// def_use_chains[value_id] = user list
    LivenessInfo *live_in;
    LivenessInfo *live_out;

    //  The following four are temporarily unavailable publicly.

    bool get_keep_live_in_info() const;
    void set_keep_live_in_info(bool keep_live_in_info);

    bool get_keep_live_out_info() const;
    void set_keep_live_out_info(bool keep_live_out_info);

    void print_def_use_chains(FILE*);

    int enroll_old_opnd(Opnd opnd);
    int lookup_old_opnd(Opnd opnd) const;
    int enroll_new_opnd(Opnd opnd);
    int lookup_new_opnd(Opnd opnd) const;
    int (SsaCore::* enroll_opnd)(Opnd);	// One of enroll_{old,new}_opnd

    void record_def_either(Opnd old_name, Opnd new_name, Operation, CfgNode*);

    void record_liveness_info(NatSet **sets, bool live_out_info);
    void push_name(NameStackEntry **stack,
		   NatSet *pushed, int *node_list_pointer,
		   unsigned old_name_number, Opnd new_name);
    void rename_locs(CfgNode *block, NameStackEntry **name_stacks);
    void rewrite_liveness_info(LivenessInfo *set, NameStackEntry **name_stacks);
    int find_root(unsigned value_id, unsigned *loc_ids);
    int phi_src_compare(PhiNode *phi_node, unsigned *loc_ids);
    bool cfg_edges_split();
    void replace_phi_nodes(bool lost_copy_risk);
    void insert_copies(CfgNode *block, CopyData *copy_stats);
    void build_dominator_tree();
    void analyze_liveness();
    void rebuild_2ndary_maps(CfgNode *block);
    void set_comment(Instr*, IdString comment);

    Opnd get_old_name(int old_id) const;
    Opnd get_new_name(int new_id) const;
    int  get_old_id(int new_id) const;
    void set_old_id(int new_id, int old_id);
    int  get_formal_value_id(int pos) const;
    void set_formal_value_id(int pos, int value_id);

    void resize_old_ids(int size);
    void resize_old_names(int size);
    void resize_new_names(int size);
    void resize_formal_value_ids(int size);

    DominanceInfo *dominator_analysis;
    DominanceChild **dominator_children;
    bool build_minimal_form, build_semi_pruned_form, build_pruned_form,
	build_def_use_chains, fold_copies, omit_useless_phi_nodes,
	report_undefined_locs, keep_live_in_info, keep_live_out_info;
    NatSet **private_live_in;
    NatSet **private_live_out;
    AssignmentListEntry **assignments;
    NatSet *global_locs;
    NatSet **phi_node_uses;
    NatSet *pushed;
    unsigned *phi_srcs_indices;
    bool replaced_phi_nodes;
    NatSet *dst_list;
    NatSet *worklist;

    // Classes for mapping the operands of instructions and phi-nodes

    friend class CountOpnds;
    friend class AddToLiveIn;
    friend class AddToCurKilled;
    friend class AddUsesToGlobal;
    friend class AddDefsToKilled;
    friend class FormAssignmentLists;
    friend class ReplaceUsedLocs;
    friend class RenameDefinedLocs;
    friend class RecordUses;
    friend class RecordDefs;
    friend class ChangeNames;
    friend class RewriteTheUse;
    friend class ReplaceSourceNames;
    friend class RestoreRegisters;
};

#endif /* SSA_SSA_CORE_H */
