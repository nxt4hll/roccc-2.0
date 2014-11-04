/* file "machine/cprinter.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "machine/cprinter.h"
#endif

#include <ctype.h>		// (isdigit)
#include <machine/substrate.h>
#include <machine/machine_ir.h>
#include <machine/problems.h>
#include <machine/init.h>
#include <machine/util.h>
#include <machine/instr.h>
#include <machine/opnd.h>
#include <machine/contexts.h>
#include <machine/note.h>
#include <machine/c_printer.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

/* Static variables */
NoteKey k_type_defined;		// key for note marking a defined type


class Declarator
{
public:
  Declarator() { }
  virtual void print(FILE*, bool, char * = "") const { }
  virtual ~Declarator(){}
};

const Declarator empty_decl;

class SymDeclarator : public Declarator {
  const char *_name;
public:
  SymDeclarator(Sym *sym) : _name(get_name(sym).chars()) { }
  SymDeclarator(char *name) : _name(name) { }
  virtual void print(FILE *out, bool, char *shim = "") const { 
    fprintf(out, "%s%s", shim, _name); 
  }
  virtual ~SymDeclarator(){}
};

enum { INDIRECT = false, DIRECT = true };

class PointerDeclarator : public Declarator {
  const Declarator &_decl;
  const char *_qual;
public:
  PointerDeclarator(const Declarator &decl, const char *qual)
    : _decl(decl), _qual(qual) { }
  virtual void print(FILE *out, bool is_direct, char *shim = "") const {
    fputs(shim, out);
    if (is_direct) putc('(', out);
    fprintf(out, "*%s", _qual);
    _decl.print(out, INDIRECT);
    if (is_direct) putc(')', out);
  }
  virtual ~PointerDeclarator(){}
};

const PointerDeclarator empty_pointer_decl(empty_decl, "");

class ArrayDeclarator : public Declarator {
  const Declarator &_decl;
  int _dimension;
public:
  ArrayDeclarator(const Declarator &decl, int dimension)
    : _decl(decl), _dimension(dimension) { }
  
  virtual void print(FILE *out, bool, char *shim = "") const {
    fputs(shim, out);
    _decl.print(out, DIRECT);
    if (_dimension < 0)
      fprintf(out, "[]");
    else
      fprintf(out, "[%d]", _dimension);
  }
  
  virtual ~ArrayDeclarator(){}
};

class FunctionDeclarator : public Declarator {
  const Declarator &_decl;
public:
  FunctionDeclarator(const Declarator &decl) : _decl(decl) { }
  
  virtual void print(FILE *out, bool, char *shim = "") const {
    fputs(shim, out);
    _decl.print(out, DIRECT);
    fprintf(out, "()");
  }
  
  virtual ~FunctionDeclarator(){}
};


/* Local helpers */

static TypeId get_sym_type(Sym*);
static bool are_similar(TypeId , TypeId);
static bool is_const(Type*);
static bool is_volatile(Type*);
static Integer get_integer(Expression*);

CPrinter::CPrinter()
{
    out = NULL;
    print_instr_table = NULL;

    omit_unwanted_notes = true;
    k_type_defined = "type_defined";
    next_type_tag = 0;
}

CPrinter::~CPrinter()
{
    clear();
}

void
CPrinter::clear()
{
    next_type_tag = 0;

    while (!noted_types.empty()) {
	TypeId t = noted_types.front();
	noted_types.pop_front();
	take_note(t, k_type_defined);
    }
}

/* Print out a label. */
void
CPrinter::print_instr_label(Instr *l)
{
    print_sym(get_label(l));
    fprintf(out, ":");
    print_notes(l);
}

void
CPrinter::print_notes(FileBlock *fb)
{
    print_annote(fb->peek_annote(k_target_lib));

    for (Iter<Annote*> iter(fb->get_annote_iterator());
	 iter.is_valid();
	 iter.next())
    {
	Annote *an = iter.current();
	if (an->get_name() == k_history)
	    print_annote(an);
	iter.next();
    }
}

void
CPrinter::print_notes(Instr *instr)
{
    if (!has_notes(instr)) {
	fprintf(out, "\n");
	return;
    }

    if (Annote *an_comment = instr->peek_annote(k_comment)) {
	// print real comment first
	print_annote(an_comment);
    } else {
	// end current line containing instruction
	fprintf(out, "\n");
    }

    Iter<Annote*> iter(instr->get_annote_iterator());
    while (iter.is_valid()) {
	Annote *an = iter.current();
	if (!omit_unwanted_notes ||
	    nonprinting_notes.find(an->get_name()) == nonprinting_notes.end())
	    print_annote(an);
	iter.next();
    }
}


/*
 * Prints out an effective-address operand.  The operand is either an
 * address operand (symbol or expression), or a register or variable
 * with pointer type.
 */
void
CPrinter::print_addr(Opnd o, TypeId goal, int context)
{
    if (!is_addr(o)) {
	claim(is_pointer(get_type(o)));
	TypeId deref_type = get_referent_type(get_type(o));
	bool cast = (goal != NULL && goal != deref_type);
	if (cast && context > UNARY)
	    putc('(', out);
	if (cast) print_pointer_cast(goal);
	print_opnd(o);
	if (cast && context > UNARY)
	    putc(')', out);

    } else if (is_addr_sym(o)) {
	Sym *as = get_sym(o);
	TypeId ast = get_sym_type(as);

	// an array-of-T can be used like a pointer-to-T
	if (is_kind_of<ArrayType>(ast) && (goal == to<ArrayType>(ast)->get_element_type()))
	    print_sym(as);
	else {
	    if (goal == NULL)		// when in doubt, cast to (void *)
		goal = type_v0;
	    if (context > UNARY)	// both cast-expr and &-expr are unary
		putc('(', out);
	    if (goal != NULL && ast != goal)
		print_pointer_cast(goal);
	    putc('&', out);
	    print_sym(as);
	    if (context > UNARY)
		putc(')', out);
	}
    } else {
	if (BaseDispOpnd bdo = o) {
	    Opnd disp = bdo.get_disp();
	    
	    if (!is_immed_integer(disp) || (get_immed_int(disp) != 0)) {
		print_addr_disp(bdo.get_base(), disp, goal, context, "+");
	    } else {
		claim(are_similar(goal, get_deref_type(bdo)));
		print_opnd(bdo.get_base());
	    }
	}
	else if (SymDispOpnd sdo = o) {
	    Opnd disp = sdo.get_disp();

	    if (!is_immed_integer(disp) || (get_immed_int(disp) != 0)) {
		print_addr_disp(sdo.get_addr_sym(), disp, goal, context, "+");
	    } else {
		print_addr(sdo.get_addr_sym(), goal, context);
	    }
	} else {
	    claim(false, "Unexpected address expression");
	}
    }
}

/* Print out an operand. */
void
CPrinter::print_opnd(Opnd o)
{
    if (is_null(o)) {
	fprintf(out, "/* nullop */");

    } else if (is_var(o)) {
	print_sym(get_var(o));

    } else if (is_hard_reg(o)) {
	fprintf(out, "_hr%d", get_reg(o));

    } else if (is_virtual_reg(o)) {
	fprintf(out, "_vr%d", get_reg(o));

    } else if (is_immed(o)) {
	print_immed(o);

    } else if (is_addr(o)) {
	print_addr(o);

    } else {
	claim(false, "print_opnd() -- unknown kind %d", get_kind(o));
    }
}

void
CPrinter::print_sym(Sym *s)
{
    claim(s);
    fprintf(out, "%s", get_name(s).chars());
}

void
CPrinter::print_sym_decl(Sym *s)
{
    claim(s);
    print_decl(get_sym_type(s), SymDeclarator(s));
}

void
CPrinter::print_sym_decl(char *s, TypeId t)
{
    claim(s);
    print_decl(t, SymDeclarator(s));
}


/* Print a procedure declaration. */
void
CPrinter::print_proc_decl(ProcSym *p)
{
    print_sym_decl(p);
    fprintf(out, ";\n");
}


/* Prints the procedure implementation header. */
void
CPrinter::print_proc_begin(ProcDef *pd)
{
    ProcSym *psym = get_proc_sym(pd);
    const char *cur_pname = get_name(psym).chars();

    // if static procedure, then add this storage class
    if (is_private(psym))
	fprintf(out, "static ");

    print_type(get_result_type(get_type(psym)));

    fprintf(out, "\n%s(", cur_pname);

    // print out formal parameters
    for (int i = 0; i < get_formal_param_count(pd); i++) {
	VarSym *param = get_formal_param(pd, i);
	if (i > 0) 
	    fprintf(out, ", ");
	print_sym_decl(param);
    }

    fprintf(out, ")\n{\n");
}

/*
 * A helper function for print_var_def().  Add a return after calling this.
 */
bool
CPrinter::process_value_block(ValueBlock *vblk, TypeId)
{
    if (is_kind_of<UndefinedValueBlock>(vblk)) {
	return false;

    } else if (is_kind_of<ExpressionValueBlock>(vblk)) {
	ExpressionValueBlock *xvb = static_cast<ExpressionValueBlock*>(vblk);
	TypeId t = xvb->get_expression()->get_result_type();

	if (is_pointer(t))
	    print_pointer_cast(get_referent_type(t));
	print_opnd(get_value(xvb));
	return true;

    } else if (is_kind_of<MultiValueBlock>(vblk)) {
	MultiValueBlock *mvb = (MultiValueBlock*)vblk;
	int nsubblocks = subblocks_size(mvb);

	putc('{', out);
	for (int i = 0; i < nsubblocks; ++i) {
	  if (   process_value_block(get_subblock(mvb, i), 0)
	      && (i < nsubblocks - 1))
	    fputs(", ", out);
	}
	putc('}', out);
	return true;
	    
    } else if (is_kind_of<RepeatValueBlock>(vblk)) {
	RepeatValueBlock *rvb = static_cast<RepeatValueBlock*>(vblk);
	claim(is_kind_of<UndefinedValueBlock>(get_subblock(rvb)));
	return false;

    } else {
	claim(false, "Unexpected kind of ValueBlock");
    }
    return false;			// never reached
}

/*
 * Generate a C variable definitions.  If no_init is true, ignore any
 * initializer.  This is to allow predeclarations at top level so that
 * forward references are legal.
 */
void
CPrinter::print_var_def(VarSym *vsym, bool no_init)
{
    VariableDefinition *vdef = vsym->get_definition();

    if (   no_init
	|| vdef == NULL
	|| (vdef->get_initialization() == NULL)	// FIXME: shouldn't be possible
	|| is_kind_of<UndefinedValueBlock>(vdef->get_initialization()))
    {
	// Uninitialized data item.  Indicate scope.
	if (is_private(vsym))
	    fprintf(out, "static ");
	print_sym_decl(vsym);
	fprintf(out, ";\n");

    } else {		// initialized data item 

	// If static symbol, add static modifier.  Note that a variable
	// is considered global in SUIF if it is part of the global or
	// file symbol table.
	if (is_private(vsym) || !is_global(vsym))
	    fprintf(out, "static ");

	print_sym_decl(vsym);

	// initialize memory with values
	fputs(" = ", out);
	process_value_block(vdef->get_initialization(), 0); // 2nd arg ignored
	fputs(";\n", out);
   }
}


/* Print a type from a SUIF symbol table in C syntax. */
void
CPrinter::print_type(TypeId t)
{
    print_decl(t, empty_decl);
}

void
CPrinter::print_addr_disp(Opnd pointer, Opnd disp,
			  TypeId goal, int context, char *op)
{
    bool cast = (goal != NULL && get_bit_size(goal) != 8);
    int shape = cast ? UNARY : BINARY;

    if (context > shape)
	putc('(', out);
    if (!cast) {
	print_addr(pointer, goal ? goal : type_s8, BINARY);
	fprintf(out, " %s ", op);
	print_opnd(disp);
    } else {
	print_pointer_cast(goal);
	print_addr_disp(pointer, disp, type_s8, UNARY, op);
    }
    if (context > shape)
	putc(')', out);
}

/*
 * Useful for printing annotations as comments.  Expects that the
 * annotation is either a BrickAnnote or a GeneralAnnote that has no
 * associated value.
 */
void
CPrinter::print_annote(Annote *annote)
{
    claim(annote != NULL);
    IdString name = annote->get_name();

    if (name != k_comment)
	fprintf(out, "/* %s", name.chars());

    if (is_kind_of<BrickAnnote>(annote)) {
	BrickAnnote *an = (BrickAnnote *)(annote);
	char *separator = ": ";

	for (Iter<SuifBrick*> iter = an->get_brick_iterator();
	     iter.is_valid(); iter.next())
	{
	    fputs(separator, out);
	    separator = ", ";

	    SuifBrick *brick = iter.current();
	    if (is_a<IntegerBrick>(brick)) {
		Integer i = ((IntegerBrick*)iter.current())->get_value();
		if (i.is_c_string_int())
		    fputs(i.chars(), out);
		else
		    fprintf(out, "%ld", i.c_long());
	    }
	    else if (is_a<StringBrick>(brick)) {
		String string = ((StringBrick*)iter.current())->get_value();
		print_string_literal(string);
	    }
	    else {
		claim(is_a<SuifObjectBrick>(brick));
		SuifObject *so = ((SuifObjectBrick*)brick)->get_object();
		if (is_kind_of<Type>(so))
		    fprint(out, (TypeId)so);
		else {
		    const char *kind = so ? get_class_name(so) : "NULL";
		    fprintf(out, "<<<%s object>>>", kind);
		}
	    }
	}
    } else {
	claim(is_kind_of<GeneralAnnote>(annote), "Unexpected kind of Annote");
    }
    if (name != k_comment)
	fputs("]", out);
    fputs(" */\n", out);
}

void
CPrinter::print_pointer_cast(TypeId referent)
{
    putc('(', out);
    print_decl(referent, empty_pointer_decl);
    putc(')', out);
}

void
CPrinter::print_decl(TypeId t, const Declarator &decl)
{
    TypeId ut = unqualify_type(t);
    if (is_pointer(ut)) {
	const char *q = is_const(t) ? " const" : is_volatile(t) ? " volatile" : "";
	print_decl(get_referent_type(ut), PointerDeclarator(decl, q));
    } else if (is_kind_of<ArrayType>(ut)) {
	ArrayType *at = to<ArrayType>(ut);
	Integer lb = get_integer(at->get_lower_bound());
	Integer ub = get_integer(at->get_upper_bound());

	claim(lb == 0);
	      
	int dimension = (ub.is_c_int() ? ub.c_int() + 1 : -1);
	ArrayDeclarator array_decl(decl, dimension);
	print_decl(at->get_element_type(), array_decl);

    } else if (is_kind_of<ProcedureType>(ut)) {
	print_decl(get_result_type(ut), FunctionDeclarator(decl));

    } else {
	if (is_record(ut))
	    print_group_def(ut);
	else if (is_enum(ut))
	    print_enum_def(ut);
	else
	    print_atomic_type(t);
	decl.print(out, INDIRECT, " ");   // leading space if decl isn't empty
    }
}

bool
CPrinter::print_type_ref(TypeId t, const char *keyword)
{
    IdString name_tag = get_name(t);

    ListNote<IdString> note = get_note(t, k_type_defined);
    bool def_printed = !is_null(note);

    if (def_printed) {
	if (name_tag == empty_id_string)
	    name_tag = note.get_value(0);
    } else {
	note = ListNote<long>();
	if (name_tag == empty_id_string) {
	    char buffer[20];
	    sprintf(buffer, "__%d", next_type_tag++);
	    name_tag = buffer;
	    note.set_value(0, name_tag);
	}
	set_note(t, k_type_defined, note);
    }
    fprintf(out, "%s %s", keyword, name_tag.chars());

    return def_printed;
}

void
CPrinter::print_group_def(TypeId t)
{
    if (print_type_ref(t, is_struct(t) ? "struct" : "union"))
	return;

    GroupType *gt = to<GroupType>(t);
    GroupSymbolTable *gst = gt->get_group_symbol_table();

    fputs(" { ", out);

    Iter<SymbolTableObject*> iter = gst->get_symbol_table_object_iterator();
    for ( ; iter.is_valid(); iter.next()) {
	FieldSymbol *fs = to<FieldSymbol>(iter.current());

	print_decl(get_type(fs), SymDeclarator(fs));
	fputs("; ", out);
    }
    putc('}', out);
}

void
CPrinter::print_enum_def(TypeId t)
{
    if (print_type_ref(t, "enum"))
	return;

    EnumeratedType *et = to<EnumeratedType>(t);
    fputs(" { ", out);
    for (int i = 0; i < et->get_case_count(); ++i)
	fprintf(out, "%s%s = %d", (i ? ", " : ""),
		     et->get_case(i).first.c_str(),
		     et->get_case(i).second.c_int());
    putc('}', out);
}

void
CPrinter::print_atomic_type(TypeId t)
{
    if (is_const(t))
	fprintf(out, "const ");
    else if (is_volatile(t))
	fprintf(out, "volatile ");

    if (is_void(t))
	fprintf(out, "void");

    else if (is_boolean(t))
	fprintf(out, "_BOOL");

    else if (is_integral(t)) {
	int size = get_bit_size(t);

	if (!is_signed(t))
	    fprintf(out, "unsigned ");

	if (size == 8)
	    fprintf(out, "char");
	else if (size == 16)
	    fprintf(out, "short");
	else if (size == 32)
	    fprintf(out, "int");
	else if (size == 64)
	    fprintf(out, "long");
	else
	    claim(false, "unexpected size (%d) for integral type", size);

    } else if (is_floating_point(t)) {
	int size = get_bit_size(t);
	if (size == 32)
	    fprintf(out, "float");
	else if (size == 64)
	    fprintf(out, "double");
	else if (size > 64)
	    fprintf(out, "long double");
	else
	    claim(false, "unexpected size (%d) for floating-point type", size);
    }
}

void
CPrinter::print_immed(Opnd o)
{
    TypeId t = get_type(o);

    if (!t) {
	claim(is_immed_string(o));
	print_string_literal(get_immed_string(o));
    } else {
	if (is_pointer(t))
	    print_pointer_cast(get_referent_type(t));
	if (is_immed_string(o))
	    fputs(get_immed_string(o).chars(), out);
	else {
	    Integer i = get_immed_integer(o);
	    if (i.is_c_long())
		fprintf(out, "%ld", i.c_long());
	    else
		fputs(i.c_string_int(), out);
	}
    }
}

void
CPrinter::print_string_literal(IdString literal)
{
    putc('"', out);
    for (const char *p = literal.chars(); *p != '\0'; ++p)
    {
	if (*p == '"' || *p == '\\')
	    putc('\\', out);
	putc(*p, out);
    }
    putc('"', out);
}

/* -----------------------------  Helpers  ------------------------------- */

static TypeId 
get_sym_type(Sym *s)
{
    if (is_kind_of<VarSym>(s))
	return get_type(static_cast<VarSym*>(s));
    else if (is_kind_of<ProcSym>(s))
	return get_type(static_cast<ProcSym*>(s));
    else
	claim(false, "Can't obtain type of symbol");
    return NULL;
}

bool
are_similar(TypeId t1, TypeId t2)
{
    TypeId ut1 = unqualify_type(t1), ut2 = unqualify_type(t2);
    return ut1->getClassName() == ut2->getClassName()
	&& get_bit_size(ut1) == get_bit_size(ut2);
}

static bool
is_const(Type *t)
{
    return is_kind_of<QualifiedType>(t)
	&& static_cast<QualifiedType*>(t)->has_qualification_member(k_const);
}

static bool
is_volatile(Type *t)
{
    return is_kind_of<QualifiedType>(t)
	&& static_cast<QualifiedType*>(t)->has_qualification_member(k_volatile);
}

static Integer
get_integer(Expression *x)
{
    claim(is_kind_of<IntConstant>(x));
    return static_cast<IntConstant*>(x)->get_value();
}

CPrinter*
target_c_printer()
{
    return dynamic_cast<MachineContext*>(the_context)->target_c_printer();
}
