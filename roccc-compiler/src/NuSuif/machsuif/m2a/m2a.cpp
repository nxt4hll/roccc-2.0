/* file "m2a/m2a.cc" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "m2a/m2a.h"
#endif

#include <machine/machine.h>

#include <m2a/m2a.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif


/* Purpose of program: This program translates a Machine-SUIF IR file 
 * into an ASCII assembly-language file for the current target. */

/*
 * Class file_list - manages source file names and numbers
 */
struct file_e {
    IdString fname;		/* source file name */
    int file_number;		/* assigned file number */
    file_e *next;

    file_e(IdString f, int n) 
	{ fname = f; file_number = n; next = NULL; }
    ~file_e() {}
};

class FileList {
    file_e *flist;		/* linked list of source files */
    int next_file_number;	/* next file number to give out */

  public:
    int enter(IdString);
    int get_file_number(IdString);
    bool contains(IdString);

    FileList() { flist = NULL; next_file_number = 2; }
    ~FileList() {}
};

static FileList filelist;	/* table of source file names */


M2a::M2a()
{
    out = NULL;
}


void
M2a::do_file_block(FileBlock *fb)
{
    printer = target_printer();
    // output annotations on file block
    printer->set_file_ptr(out);
    printer->print_notes(fb);

    // allow for special assembler directives at top of file
    printer->print_global_decl(fb);

    // output data pseudo-ops
    debug(3, "Processing globals for %s", (get_name(fb)).chars());
    process_sym_table(external_sym_table());
    process_sym_table(file_set_sym_table());
    process_sym_table(get_sym_table(fb));
}


void
M2a::do_proc_def(ProcDef *pd)
{
    printer->set_omit_unwanted_notes(!print_all_notes);

    // do the work to get the procedure name
    const char *cur_pname = (get_name(pd)).chars();
    debug(3, "Processing procedure %s", cur_pname);

    // print the procedure symbol table
    process_sym_table(get_sym_table(pd));

    // create a list to hold any .file op's found in the middle
    // of the text segment -- cannot print them there
    List<IdString> file_strings;

    // ensure that the procedure's body is in instruction list form
    claim(is_kind_of<InstrList>(get_body(pd)), "Body is not an InstrList");
    InstrList *mil = static_cast<InstrList*>(get_body(pd));

    // .file pseudo op processing
    LineNote note = get_note(pd, k_line);
    claim(!is_null(note), "Missing k_line on ProcDef");
    int this_file_number;
    IdString this_file = note.get_file();
    IdString last_file;				// file from last line instr
    int last_line = -1;				// line   "    "    "    "

    if (!filelist.contains(this_file)) {
	// enter file into table and output .file pseudo op
	this_file_number = filelist.enter(this_file);
	printer->print_file_decl(this_file_number, this_file);
/********
	  if (want_stabs) {
	  Stabs_begin_file(this_file, output_fd);
	  Stabs_types(fileset->globals(), output_fd);
	  Stabs_types(fse->symtab(), output_fd);
	  }
********/
    } else {
	// in-table so already .file already exists, but need
	// this_file_number for print_proc_entry
	this_file_number = filelist.get_file_number(this_file);
    }

    // generate pseudo ops to begin procedure text segment
    debug(2, "Printing procedure \"%s\"", cur_pname);
    printer->print_proc_begin(pd);

    // output procedure body
    for (InstrHandle mi_h = start(mil); mi_h != end(mil); ) {
	Instr *mi = *mi_h;
	bool defer = false;			// deferring current instr

	// check for procedure entry point
	if (has_note(mi, k_proc_entry)) {
	    // generate pseudo ops to start procedure text segment
	    claim(is_null(mi));
	    printer->print_proc_entry(pd, this_file_number);
/********
	    if (want_stabs) {
		Stabs_types(cur_psym->block()->proc_syms(), output_fd);
		Stabs_block_begin_label(output_fd);
	    }
********/
	    ++mi_h;
	    continue;
	}
	if (has_note(mi, k_line) && !is_line(mi)) {
	    LineNote note = take_note(mi, k_line);
	    IdString note_file = note.get_file();
	    if (note_file.is_empty())			// work around CFE bug:
		note_file = last_file;			// treat empty file name
	    if (last_line != note.get_line()  ||	// just like last_file
		last_file != note_file) {
		defer = true;
		mi = new_instr_dot(opcode_line());
		set_note(mi, k_line, note);
	    }
	}
	if (is_line(mi)) {
	    LineNote note = take_note(mi, k_line);
	    IdString note_file = note.get_file();
	    int	     note_line = note.get_line();

	    if (note_file.is_empty())			// work around afore-
		note_file = last_file;			// mentioned CFE bug

	    // first finish translation of line pseudo op
	    if (!filelist.contains(note_file)) {
		// Add to table.  However, we canNOT immediately print
		// .file op -- breaks assembler.  Since .file ops are
		// ignored by the most assemblers anyway, wait and output
		// the file op at the end of the procedure.
		filelist.enter(note_file);
		file_strings.push_back(note_file);
	    }
/********
	    if (want_stabs)
		Stabs_line_label((*il)[0].integer(), output_fd);
********/
	    last_file = note_file;
	    last_line = note_line;
	    int file_number = filelist.get_file_number(note_file);
	    set_src(mi, 0, opnd_immed(file_number, type_u32));
	    set_src(mi, 1, opnd_immed(note_line, type_u32));
	}
	printer->print_instr(mi);

	if (defer) {
	    defer = false;
	    delete mi;				// temporary line instr
	} else {
	    ++mi_h;
	}
    }
    // generate pseudo ops to end procedure text segment
/********
    if (want_stabs)
	Stabs_block_end_label(output_fd);
********/
    printer->print_proc_end(pd);

/********
    // TODO - move this to pmstabs
    if (want_stabs) {
	Stabs_finish_proc(cur_psym, output_fd);
	Stabs_symtab(cur_psym->block()->proc_syms(), output_fd);
    }
********/

    // any extra file op's to print?
    while (file_strings.size()) {
	this_file = file_strings.front();
	file_strings.pop_front();
	int this_file_number = filelist.get_file_number(this_file);
	printer->print_file_decl(this_file_number, this_file);
    }
}

void 
M2a::process_sym_table(SymTable *st)
{
    Iter<SymbolTableObject*> iter = st->get_symbol_table_object_iterator();
    while (iter.is_valid()) {
	SymbolTableObject *the_sto = iter.current();
	if (is_kind_of<VarSym>(the_sto)) {
	    VarSym *v = (VarSym*)(the_sto); 

	    if (is_external(v) && !is_defined(v)) {
		// externally-defined symbols need to be identified in
		// assembly code
		printer->print_extern_decl(v);

	    } else if (!is_auto(v)) {
		printer->print_var_def(v);
	    }

	} else if (is_kind_of<ProcSym>(the_sto)) {
	    printer->print_proc_decl((ProcSym*)(the_sto));

	} else if (is_kind_of<LabelSym>(the_sto) || is_kind_of<Type>(the_sto)) {
	    // do nothing

	} else
	    claim(is_kind_of<IrOpnd>(the_sto),
		  "process_sym_table: unexpected kind of sto"); 

	iter.next();
    }
}

/*
 * Definitions for class FileList
 */

/* FileList::enter() -- create an entry for file and return the
 * file number.  Note that the file list is maintained as a LIFO
 * stack, and that file numbers are assigned sequentially. */
int
FileList::enter(IdString name)
{
    claim(!contains(name));
    file_e *new_fp = new file_e(name, next_file_number);
    next_file_number++;
    new_fp->next = flist;
    flist = new_fp;
    return flist->file_number;
}

/* FileList::get_file_number() - returns the file number for the given file
   name */
int
FileList::get_file_number(IdString name)
{
    file_e *fp = flist;
    while (fp && (fp->fname != name)) fp = fp->next;
    claim(fp, "get_file_number() -- filename not in list");
    return fp->file_number;
}

/* FileList::contains() -- returns true if filename is in file list */
bool
FileList::contains(IdString f)
{
    file_e *fp = flist;
    while (fp && (fp->fname != f)) fp = fp->next;
    if (fp) return true;
    return false;
}
