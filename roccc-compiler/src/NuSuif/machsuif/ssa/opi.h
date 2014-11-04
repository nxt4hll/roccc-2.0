/* file "ssa/opi.h" */

/*
    Copyright (c) 2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef SSA_OPI_H
#define SSA_OPI_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "ssa/opi.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <ssa/ssa_ir.h>


/*
 * Flags for new_ssa_cfg.
 *
 * BUILD_MINIMAL_FORM, BUILD_SEMI_PRUNED_FORM, and BUILD_PRUNED_FORM are
 * mutually exclusive.  Minimal form ignores liveness in placing phi-nodes.
 * Pruned form only inserts phi-nodes at merge points where the defined
 * value is known to be live.  Semi-pruned form is a compromise that
 * doesn't require full-blown liveness analysis.  It places phi-nodes only
 * for names known to be live on entry to _some_ basic block.  ("Minimal"
 * form is a terrible name for the form that inserts zillions of unneeded
 * phi-nodes.  Avoid it.)
 *
 * BUILD_DEF_USE_CHAINS directs the library to record def-use mappings,
 * which are available through the `use_chain' method of class SsaCore.
 *
 * FOLD_COPIES causes the transformation into SSA form to eliminate "move"
 * instructions involving SSA names by propagating the source of a move to
 * the occurrences of its destination name.
 *
 * OMIT_USELESS_PHI_NODES causes a phi-node to be omitted if every path to
 * its basic block has the same SSA value for the original name.
 *
 * PRINT_WARNINGS turns some warnings that are suppressed by default.
 */
namespace ssa {

    enum {
        BUILD_MINIMAL_FORM     = 1<<0,
        BUILD_SEMI_PRUNED_FORM = 1<<1,
        BUILD_PRUNED_FORM      = 1<<2,
        BUILD_DEF_USE_CHAINS   = 1<<3,
        FOLD_COPIES            = 1<<4,
        OMIT_USELESS_PHI_NODES = 1<<5,
        PRINT_WARNINGS         = 1<<6
    };

} // namespace ssa

class Operation;

SsaCfg* new_ssa_cfg(Cfg*, OptUnit*, unsigned flags = 0);
Cfg* restore(SsaCfg*, bool not_old_names = false);

Cfg* get_cfg(SsaCfg*);

bool has_minimal_form(SsaCfg*);
bool has_semi_pruned_form(SsaCfg*);
bool has_pruned_form(SsaCfg*);

bool has_def_use_chains(SsaCfg*);
bool has_copies_folded(SsaCfg*);
bool has_useless_phi_nodes_folded(SsaCfg*);

int get_loc_count(SsaCfg*);			// number of original locations
int get_value_count(SsaCfg*);			// number of SSA defs

const OpndCatalog* get_value_catalog(SsaCfg*);
Opnd get_value_name(SsaCfg*, int value_id);	// map value_id to SSA name
CfgNode* get_def_block(SsaCfg*, int value_id);	// block holding value's definer

Operation get_unique_def(SsaCfg*, Opnd value_name);
const List<Operation>& get_def_use_chain(SsaCfg*, Opnd value_name);

typedef List<PhiNode*>::iterator PhiHandle;

const List<PhiNode*>& get_phi_nodes(SsaCfg*, CfgNode*);
PhiHandle append_phi_node(SsaCfg*, CfgNode*, PhiNode*);
PhiNode* remove_phi_node(SsaCfg*, CfgNode*, PhiNode*);

void record_def(SsaCfg*, Opnd old_name, Operation, int dst_pos = 0);
void record_use_swap(SsaCfg*, Opnd out, Opnd in, Operation);
void record_all_uses(SsaCfg*, Operation);

void fprint(FILE*, SsaCfg*);

PhiNode* new_phi_node(int src_count);
int srcs_size(PhiNode*);
Opnd get_src(PhiNode*, int pos);
void set_src(PhiNode*, int pos, Opnd);
Opnd get_dst(PhiNode*);
void set_dst(PhiNode*, Opnd);
bool get_is_useless(PhiNode*);
void set_is_useless(PhiNode*, bool);
CfgNode* get_parent_node(PhiNode*);
void map_opnds(PhiNode*, OpndFilter&);

/*
 * An ``operation'' is either a phi-node or an instruction that defines one
 * or more values.
 */

class Operation
{
  public:
    Operation() :
	instr_handle(one_null_instr.begin()),
	phi_handle  (one_null_phi.begin())
      { }

    Operation(InstrHandle ih) { set_instr_handle(ih); }
    Operation(PhiHandle ph) { set_phi_handle (ph); }

    PhiHandle get_phi_handle() const { return phi_handle; }
    InstrHandle get_instr_handle() const { return instr_handle; }

    void set_instr_handle(InstrHandle ih)
	{ instr_handle = ih; phi_handle = one_null_phi.begin(); }
    void set_phi_handle(PhiHandle ph)
	{ phi_handle = ph; instr_handle = one_null_instr.begin(); }

    bool is_instr_handle() const
	{ return *phi_handle == NULL && *instr_handle != NULL; }
    bool is_phi_handle() const
	{ return *phi_handle != NULL && *instr_handle == NULL; }
    bool is_null() const
	{ return *phi_handle == NULL && *instr_handle == NULL; }

    static list<Instr*>   one_null_instr;
    static list<PhiNode*> one_null_phi;

  protected:
    InstrHandle instr_handle;
    PhiHandle phi_handle;
};

Opnd get_dst(Operation, int pos = 0);
CfgNode* get_parent_node(Operation);
void map_opnds(Operation, OpndFilter&);

bool has_note (Operation, NoteKey);
Note get_note (Operation, NoteKey);
Note take_note(Operation, NoteKey);
void set_note (Operation, NoteKey, const Note&);


#endif /* SSA_OPI_H */
