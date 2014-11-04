/* file "summarize/summarize.cc" */

/*
    Copyright (c) 2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "summarize/summarize.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <halt/halt.h>

#include <summarize/summarize.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif


int find_branch_id(CfgNode*, int max_event_id);
void summarize_branch(char tag, FILE*, CfgNode*, HaltLabelNote&,
		      int max_event_id);

Summarize::Summarize()
{
    out = NULL;
}

/*
 * Scan the whole file to compute the upper bound (max + 1) on the HALT
 * event ids for CBR, MBR, ENTRY, and EXIT events.
 */

void
Summarize::initialize()
{
    DefinitionBlock *db = the_file_block->get_definition_block();

    max_event_id = 0;
    for (Iter<ProcedureDefinition*> iter = db-> get_procedure_definition_iterator();
	 iter.is_valid();
	 iter.next())
    {
	OptUnit *unit = iter.current();
	claim(is_kind_of<Cfg>(get_body(unit)), "Body is not in CFG form");
	Cfg *unit_cfg = static_cast<Cfg*>(get_body(unit));
	debug(5, "Procedure %s", get_name(unit).chars());
	if_debug(5)
	    fprint(stdout, unit_cfg, true, true); // layout and code

	for (int i = 0; i < nodes_size(unit_cfg); ++i)
	{
	    CfgNode *block = get_node(unit_cfg, i);
	    for (InstrHandle h = start(block); h != end(block); ++h)
		if (HaltLabelNote note = get_note(*h, k_halt))
		    switch (note.get_kind())
		    {
		      case halt::CBR:
		      case halt::MBR:
		      case halt::ENTRY:
		      case halt::EXIT:
		      {
			  long id = note.get_unique_id();
			  if (max_event_id <= id)
			      max_event_id =  id + 1;
		      }
		    }
	}
    }
    fprintf(out, "%d\n", max_event_id);
}

void
Summarize::do_opt_unit(OptUnit *unit)
{
    const char *unit_name = get_name(unit).chars();
    LineNote note = get_note(unit, k_line);
    claim(!is_null(note), "Missing `line' annotation on unit `%s'", unit_name);

    const char *file_name = note.get_file().chars();

    debug(1, "Processing procedure `%s' from file `%s'", unit_name, file_name);

    // This pass requires a unit's body to be a CFG.  Be sure that a
    // preceding pass has left it in that form.

    claim(is_kind_of<Cfg>(get_body(unit)), "Body is not in CFG form");
    Cfg *unit_cfg = static_cast<Cfg*>(get_body(unit));
    if_debug(5)
	fprint(stdout, unit_cfg, false, true);		// no layout, just code

	for (int i = 0; i < nodes_size(unit_cfg); ++i)
	{
	    CfgNode *block = get_node(unit_cfg, i);
	    for (InstrHandle h = start(block); h != end(block); ++h)
		if (HaltLabelNote note = get_note(*h, k_halt))
		    switch (note.get_kind())
		    {
		      case halt::CBR:
			summarize_branch('c', out, block, note, max_event_id);
			break;
		      case halt::MBR:
			summarize_branch('m', out, block, note, max_event_id);
			break;
		      case halt::ENTRY:
			fprintf(out, "p %ld 1 %d %s:%s\n",
				note.get_unique_id(),
				find_branch_id(block, max_event_id),
				file_name,
				unit_name);
			break;
		      case halt::EXIT:
			fprintf(out, "r %ld 0\n", note.get_unique_id());
			break;
		    }
	}

}

/*
 * find_branch_id
 *
 * Find the first branch point following `block' and return the HALT id of
 * that branch "event", which must be less than `max_event_id'.
 *
 * The tricky case is when a cycle of CFG blocks contains no branch points.
 * We recognize this situation when we find an impossible edge from one of
 * the blocks reached from `block'.  In that case, we return -1.
 */
int
find_branch_id(CfgNode *block, int max_event_id)
{
    NatSetDense visited;
    bool seen_impossible = false;
    visited.insert(get_number(block));
    do {
	if (ends_in_cbr(block) || ends_in_mbr(block) || ends_in_return(block))
	{
	    // For CBR and EXIT, halt note is right on the CTI.  For MBR,
	    // it's on the k_mbr_target_def instruction, the one that
	    // develops the dispatch address.

	    InstrHandle cti_handle = get_cti_handle(block);
	    claim(cti_handle != end(block));

	    InstrHandle h = cti_handle;
	    for (/* */; !has_note(*h, k_halt) && h != start(block); --h)
	    { }

	    if (HaltLabelNote note = get_note(*h, k_halt))
	    {
		int id = note.get_unique_id();
		claim(0 <= id && id < max_event_id, "Branch id out of range");
		return id;
	    }
	    claim(false, "Unable to determine unique id for branch");
	}
	
	claim(ends_in_ubr(block) || ends_in_call(block) || !ends_in_cti(block));

	CfgNodeHandle sh = succs_start(block);
	CfgNode *succ = *sh;

	for (int i = 0; sh != succs_end(block); ++i, ++sh) {
	    if (is_impossible_succ(block, i))
		seen_impossible = true;
	}
	visited.insert(get_number(block));
	block = succ;
    } while (!visited.contains(get_number(block)));
    claim(seen_impossible);
    return -1;
}


void
summarize_branch(char tag, FILE *out, CfgNode *block, HaltLabelNote &note,
		 int max_event_id)
{
    int nsuccs = 0;

    for (CfgNodeHandle sh = succs_start(block); sh != succs_end(block); ++sh)
    {
	claim (*sh != NULL, "Null successor node");
	if (is_abnormal_succ(block, nsuccs++))
	    break;
    }

    fprintf(out, "%c %ld %d ", tag, note.get_unique_id(), nsuccs); 

    for (CfgNodeHandle sh = succs_start(block); nsuccs-- > 0; ++sh)
	fprintf(out, "%d ", find_branch_id(*sh, max_event_id));
    fprintf(out, "\n"); 
}
