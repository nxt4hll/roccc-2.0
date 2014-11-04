/* file "m2c/m2c.cc" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "m2c/m2c.h"
#endif

#include <machine/machine.h>

#include <m2c/m2c.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif


/* Purpose of program: This program translates a Machine-SUIF IR file 
 * to C.
 */


/* -------- filter definitions -------- */

class VrFilter : public OpndFilter {
  private:
    TypeId *_vr_table;

  public:
    VrFilter(TypeId *vr_table) { _vr_table = vr_table; }
    virtual ~VrFilter() {}

    virtual Opnd operator()(Opnd, InOrOut);
};


/* Routine just records what virtual registers it has seen. */
Opnd
VrFilter::operator()(Opnd o, InOrOut t)
{
    if (!is_virtual_reg(o)) return o;

    int vr = get_reg(o);
    if (_vr_table[vr]) {
	// seen already, just perform sanity check
	claim(_vr_table[vr] == get_type(o));
    } else {
	_vr_table[vr] = get_type(o);
    }

    return o;
}


/* -------- implementations of M2c methods -------- */

M2c::M2c()
{
    out = NULL;
    vr_table = NULL;
}

void
M2c::do_file_block(FileBlock *fb)
{
    printer = target_c_printer();
    printer->clear();
    printer->set_file_ptr(out);
    printer->set_omit_unwanted_notes(!print_all_notes);

    // output annotations on file block
    printer->print_notes(fb);

    fprintf(out, "/* Generated automatically by Machine SUIF */\n");

    // allow for special directives at top of file
    printer->print_global_decl(fb);

    // output global declarations
    debug(3, "Processing globals for %s", (get_name(fb)).chars());
    process_sym_table(external_sym_table());
    process_sym_table(file_set_sym_table());
    process_sym_table(get_sym_table(fb));

    while (!postponed_vars.empty()) {
	VarSym *v = postponed_vars.front();
	postponed_vars.pop_front();
	printer->print_var_def(v, false);
    }
}

void
M2c::do_proc_def(ProcDef *pd)
{
    const char *cur_pname = (get_name(pd)).chars();
    debug(3, "Processing procedure %s", cur_pname);

    // create a list to hold any .file op's found in the middle
    // of the text segment -- cannot print them there
    List<IdString> file_strings;

    claim(is_kind_of<InstrList>(get_body(pd)), "Body is not an InstrList");
    cur_body = static_cast<InstrList*>(get_body(pd));

    printer->print_proc_begin(pd);

    // print the procedure symbol table
    process_sym_table(pd->get_symbol_table());

    // walk the instruction list once to record all vr defs
    cur_handle = start(cur_body);
    process_vr_decls(pd);
    cur_handle = start(cur_body);

    Instr *mi = *cur_handle;
    if (mi->peek_annote(k_proc_entry) == NULL) {
	// Entry point to procedure is not the first instruction,
	// generate goto to instruction with k_proc_entry note.
	fprintf(out, "\n\tgoto %s_entry_pt;\n", cur_pname);
    } else {
	// simple entry point
	claim(is_null(mi));
	++cur_handle;	
    }

    // output procedure body
    while (cur_handle != end(cur_body)) {
	mi = *cur_handle;

	// do some work on a non-simple procedure entry point
	if (mi->peek_annote(k_proc_entry) != NULL) {
	    // generate label for earlier goto
	    claim(is_null(mi));
	    fprintf(out, "\n%s_entry_pt:\n", cur_pname);
	}
	printer->print_instr(mi);
	++cur_handle;
    }
    // procedure body must not end with a label
    if (is_label(mi))
	fputs("\t/* empty statement */;\n", out);
    fprintf(out, "}\t/* end of %s */\n\n", cur_pname);
}

void 
M2c::process_sym_table(SymTable *st)
{
    bool st_is_private = is_private(st);

    Iter<SymbolTableObject*> iter = st->get_symbol_table_object_iterator();
    for (/* iter */; iter.is_valid(); iter.next()) {
	SymbolTableObject *the_sto = iter.current();
	if (is_kind_of<VariableSymbol>(the_sto)) {
	    if (is_kind_of<ParameterSymbol>(the_sto))
		continue;			// don't shadow an arg!

	    VarSym *v = to<VariableSymbol>(the_sto); 

	    if (!is_global(st)) {
		printer->print_var_def(v, false);
	    }
	    else if (v->get_definition() != NULL) {
		postponed_vars.push_back(v);
		printer->print_var_def(v, true);
	    }
	    else {
		claim(is_external(v));
		fprintf(out, "extern ");
		printer->print_sym_decl(v);
		fprintf(out, ";\n");
	    }
	} else if (is_kind_of<ProcedureSymbol>(the_sto)) {
	    if (st_is_private) fputs("static ", out);
	    printer->print_proc_decl(to<ProcedureSymbol>(the_sto));

	} else if (is_kind_of<Type>(the_sto)) {
	    if (is_kind_of<EnumeratedType>(the_sto) ||
		(is_kind_of<GroupType>(the_sto) &&
		 to<GroupType>(the_sto)->get_is_complete())) {
		printer->print_type(to<Type>(the_sto));
		fprintf(out, ";\n");
	    }
	}
    }
}


/* This routine walks the procedure's instruction list, records what
 * virtual registers are encounters and what are their types, and then
 * produces a bunch of declarations for these virtual registers. */
void 
M2c::process_vr_decls(ProcDef *pd)
{
    int i;

    // initialize the vr_table
    OneNote<long> note = get_note(pd, k_vr_count);
    int vr_count = note.get_value();
    vr_table = new TypeId[vr_count];
    for (i = 0; i < vr_count; i++)
	vr_table[i] = NULL;
    VrFilter filter(vr_table);

    // record the type for each vr found as a destination
    while (cur_handle != end(cur_body)) {
	map_dst_opnds(*cur_handle, filter);
	++cur_handle;
    }

    // print declaration for each vr used in procedure
    fprintf(out, "\n\n/* Virtual register declarations */\n");

    for (i = 0; i < vr_count; i++) {
	if (vr_table[i]) {
	    char buf[1024];
	    sprintf(buf, " _vr%d", i);
	    printer->print_sym_decl(buf, vr_table[i]);
	    fputs(";\n", out);
	}
    }
    fprintf(out, "\n");

    delete [] vr_table;
}
