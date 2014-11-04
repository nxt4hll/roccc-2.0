/* file "x86/printer.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "x86/printer.h"
#endif

#include <machine/machine.h>

#include <x86/init.h>
#include <x86/opcodes.h>
#include <x86/printer.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace x86;

PrinterX86::PrinterX86()
{
    int i;
    print_instr_table.resize(LAST_X86_OPCODE + 1);

    print_instr_table[opcode_null] = &Printer::print_instr_dot;
    print_instr_table[opcode_label] = &Printer::print_instr_label;

    for (i = ALIGN; i <= WORD; i++)
	print_instr_table[i] = &Printer::print_instr_dot;

    for (i = LDS; i <= DAS; i++)
	print_instr_table[i] = &Printer::print_instr_alm;

    for (i = JA; i <= JMP; i++)
	print_instr_table[i] = &Printer::print_instr_cti;

    for (i = CLC; i <= FXAM; i++)
	print_instr_table[i] = &Printer::print_instr_alm;
}


void
PrinterX86::print_instr(Instr *mi)
{
    int opcode = get_opcode(mi);
    if (opcode > LAST_X86_OPCODE) {
	// Found opcode outside range of opcodes defined in this library.
	// Print using user-defined printing routine.
	print_instr_user_defd(mi);
	return;
    }
    // Otherwise use dispatch table.  Of course, the user may have
    // overwritten a table entry to point to the user-defined printing
    // method.
    print_instr_f f = print_instr_table[get_opcode(mi)];
    (this->*f)(mi);
}


/* Print out label in X86 format. */
void
PrinterX86::print_instr_label(Instr *l)
{
    print_sym(get_label(l));
    fprintf(out, ":");		// x86-specific ending
    print_notes(l);
}



/* Print out a cti instruction. */
void
PrinterX86::print_instr_cti(Instr *bj)
{
    // print out opcode and any extensions
    print_opcode(bj);

    if (get_target(bj)) {
	print_sym(get_target(bj));
    } else {
	fprintf(out, "*");
	print_opnd(get_src(bj, 0));
    }

    print_notes(bj);
}


/* Print out an ALU or memory instruction.  Intel x86 instructions
 * that print out their operands in a weird order are handled here. */
void
PrinterX86::print_instr_alm(Instr *mi)
{
    // print out opcode and any extensions
    print_opcode(mi);

    // Print out any operands -- some opcodes require special printing 
    // format for the operands.
    int opcode = get_opcode(mi);

    if (opcode == POP) {
	// print dst on pop
	print_opnd(get_dst(mi));

    } else if ((opcode == IDIV) || (opcode == DIV)) {
	// on integer divide, divisor (src1) is the only explicit operand
	print_opnd(get_src(mi, 1));

    } else if ((opcode == PUSH)
	       || (opcode == FLD) || (opcode == FILD)
	       || (opcode == FXCH)) {
	// on push or on fp load or exchange, dst is implicit; print src0 only
	print_opnd(get_src(mi, 0));

    } else if ((opcode == CBW) || (opcode == CWD) || (opcode == CDQ)
	       || (opcode == CLD) || (opcode == STD)
	       || (opcode == RET)
	       || (opcode == MOVSB) || (opcode == MOVSW) || (opcode == MOVSL)
	       || (opcode == PUSHF) || (opcode == POPF)
	       || (opcode == FCOMPP)|| (opcode == FCHS) || (opcode == FABS)
	       || (opcode == FLDZ)  || (opcode == FLD1)
	       || (opcode == SAHF)) {
	// special cases -- do not print any operands

    } else if (opcode == FSTCW || opcode == FSTSW) {
	print_opnd(get_dst(mi));

    } else if (writes_memory(mi)) {			// is store
	claim(dsts_size(mi) > 0);
	if (   (opcode == SETE)  || (opcode == SETL)
	    || (opcode == SETNE) || (opcode == SETLE)
	    || (opcode == FSTP)  || (opcode == FISTP)
	    || (opcode == FSAVE) || (opcode == FNSAVE)) {
	    // source of data to write is implicit
	} else {
	    claim(srcs_size(mi) > 0);
	    int j = is_two_opnd(mi) ? 1 : 0;		// index of explicit src
	    print_opnd(get_src(mi, j));
	    fprintf(out, ",");
	}
	print_opnd(get_dst(mi));			// EA of cell to write

    } else if (opcode == CMP || opcode == TEST) {
	// in AT&T syntax, src1 comes before (left of) src0
	print_opnd(get_src(mi, 1));
	fprintf(out, ",");
	print_opnd(get_src(mi, 0));

    } else if (opcode == MOV) {
	Opnd s = get_src(mi, 0), d = get_dst(mi);
	if (is_hard_reg(s) && 
	    (get_bit_size(get_type(s)) != get_bit_size(get_type(d)))) {
	    // hack to print source operand not according to type
	    print_opnd(opnd_reg(get_reg(s), get_type(d)));
	} else {
	    print_opnd(s);
	}
	fprintf(out, ",");
	print_opnd(d);

    } else if (opcode == FLDCW || opcode == FFREE || opcode == FRSTOR) {
	print_opnd(get_src(mi, 0));

    } else if (reads_memory(mi)   || opcode == LEA ||
	       opcode == MOVSX || opcode == MOVZX) {
	print_opnd(get_src(mi, 0));
	fprintf(out, ",");
	print_opnd(get_dst(mi));

    } else if (srcs_size(mi) < 2) {
	if (dsts_size(mi)) {
	    // print only dst, src0 is the same as dst
	    print_opnd(get_dst(mi));
	} else if (srcs_size(mi) == 1) {
	    // src0 but no dst
	    print_opnd(get_src(mi, 0));
	} // else print nothing after opcode

    } else {
	// General 2-source-operand case.  Print out "src1, dst" in
	// that order.  Remember that src0 is same as dst.
	claim(dsts_size(mi), "Missed case? Valid src1 but no dst.");
	print_opnd(get_src(mi, 1));
	fprintf(out, ",");
	print_opnd(get_dst(mi));
    }

    print_notes(mi);
}


/* Print out pseudo-ops. */
void
PrinterX86::print_instr_dot(Instr *x)
{
    int i;

    // special case opcode_null so we don't output null lines
    if (get_opcode(x) == opcode_null) {
	print_notes(x);
	return;
    }

    // print out opcode
    fprintf(out, "\t%s\t", x86_opcode_names[get_opcode(x)]);

    switch (get_opcode(x)) {
      case OPCODE_FILE:			// .file "<file_name>" # <file_no>
	claim(srcs_size(x) == 2);
	fprintf(out, "\"%s\"",  get_immed_string(get_src(x, 1)).chars());
	fprintf(out, "# %d", get_immed_int(get_src(x, 0)));
	break;

      case GLOBL:
	fprintf(out, "%s",  get_immed_string(get_src(x, 0)).chars());
	break;

      case ASCII:		// .ascii ["<s1>"[, "<s2>"...]]
	for (i = 0; i < srcs_size(x); i++) {
	    // while >1 element in list, print with comma
	    fprintf(out, "\"%s\"", get_immed_string(get_src(x, i)).chars());
	    if (!i)
		fprintf(out, ", ");
	}
	break;

      // Print arguments with commas
      default:
	for (i = 0; i < srcs_size(x); i++) {
	    // while >1 element in list, print with comma
	    Opnd opnd = get_src(x, i);

	    if (is_var(opnd)) {
		print_opnd(opnd);
	    } else if (is_immed_integer(opnd))
		fprintf(out, "%d", get_immed_int(opnd));
	    else
		fprintf(out, "\"%s\"", get_immed_string(opnd).chars());

	    if (!i)
		fprintf(out, ", ");
	}
	break;
    }

    print_notes(x);
}


/* Prints out the machine opcode and any extensions in Intel
 * X86-specific syntax. */
void
PrinterX86::print_opcode(Instr *mi)
{
    // prefix bytes (aka opcode extensions) go first in x86
    if (ListNote<long> note = get_note(mi, k_instr_opcode_exts)) {
	for (int i = 0; i < note.values_size(); ++i) {
	    int e_op = note.get_value(i);
	    fprintf(out, "%s", x86_opcode_ext_names[e_op]);
	}
    }

    fprintf(out, "\t%s", x86_opcode_names[get_opcode(mi)]);
    print_opcode_type(mi);
    fprintf(out, "\t");
}

void
PrinterX86::print_opcode_type(Instr *mi)
{
    int opcode = get_opcode(mi);

    if (   (opcode <= WORD)  || ((opcode >= JA) && (opcode <= JMP))
	|| (opcode == SETE)  || (opcode == SETL) 
	|| (opcode == SETLE) || (opcode == SETNE) || (opcode == CDQ)
	|| (opcode == CBW)   || (opcode == MOVSL) 
	|| (opcode == MOVSW) || (opcode == MOVSB) || (opcode == CLD)
	|| (opcode == LEAVE) || (opcode == RET)
	|| (opcode == FADDP) || (opcode == FSUBRP)
	|| (opcode == FMULP) || (opcode == FDIVRP)
	|| (opcode == FCOMPP)|| (opcode == FCHS)  || (opcode == FABS)
	|| (opcode == FSTSW) || (opcode == FSTCW) || (opcode == FLDCW)
	|| (opcode == FSAVE) || (opcode == FNSAVE)|| (opcode == FRSTOR)
	|| (opcode == FXCH)  || (opcode == FLDZ)  || (opcode == FLD1)
	|| (opcode == FFREE) || (opcode == SAHF))
	return;

    if (opcode == MOV) {
	if (writes_memory(mi))
	    print_opcode_size(get_type(get_src(mi, 0)));
	else // reads_memory(i) or simple reg-reg move
	    print_opcode_size(get_type(get_dst(mi)));

    } else if ((opcode == MOVSX) || (opcode == MOVZX)) {
	/* These opcodes need a size for the source and a size 
	 * for the destination. */
	if (reads_memory(mi)) {
	    // unwrap ptr type for source effective address
	    TypeId t_s0 = get_deref_type(get_src(mi, 0));
	    claim(t_s0 != NULL);
	    print_opcode_size(t_s0);
	} else {
	    // movsx and movzx do not ever write memory
	    print_opcode_size(get_type(get_src(mi, 0)));
	}
	print_opcode_size(get_type(get_dst(mi)));

    } else if (opcode == FILD) {
	// unwrap ptr type for memory effective address
	TypeId t_s0 = get_deref_type(get_src(mi, 0));
	claim(t_s0 != NULL);
	print_opcode_size(t_s0);

    } else if (   (opcode == FIST) || (opcode == FISTP)
	       || (opcode == FST)  || (opcode == FSTP)) {
	// unwrap ptr type for memory effective address
	TypeId t_dst = get_deref_type(get_dst(mi));
	claim(t_dst != NULL);
	print_opcode_size(t_dst);

    } else if ((opcode == FLD)) {
	if (reads_memory(mi)) {
	    // unwrap ptr type for memory effective address
	    TypeId t_src = get_deref_type(get_src(mi, 0));
	    claim(t_src != NULL);
	    print_opcode_size(t_src);
	}
    } else if (opcode == CMP || opcode == TEST) {
	print_opcode_size(get_type(get_src(mi, 0)));

    } else if (dsts_size(mi) == 0 || is_null(get_dst(mi))) {
	if (reads_memory(mi)) {
	    // unwrap ptr type of source effective address
	    TypeId t_s0 = get_deref_type(get_src(mi, 0));
	    claim(t_s0 != NULL);
	    print_opcode_size(t_s0);
	} else {
	    print_opcode_size(get_type(get_src(mi, 0)));
	}

    } else
	print_opcode_size(get_type(get_dst(mi)));
}

/* print_opcode_size() -- print the "l", "w", or "b" after an ALU opcode
 * required by the AT&T x86 assembler syntax.   This routine also handles
 * "s" and "l" size specifiers for FP opcodes. */
void
PrinterX86::print_opcode_size(TypeId t)
{
    int size = get_bit_size(t);

    if (is_floating_point(t))
	switch (size) {
	  case 64:
	    fprintf(out, "l");
	    break;
	  case 32:
	    fprintf(out, "s");
	    break;
	  default:
	    claim(false, "unexpected fp size (%d)", size);
	}
    else
	switch (size) {
	  case 32:
	    fprintf(out, "l");
	    break;
	  case  8:
	    fprintf(out, "b");
	    break;
	  case 16:
	    fprintf(out, "w");
	    break;
	  case 64:
	    fprintf(out, "ll");
	    break;
	  default:
	    claim(false, "unexpected int size (%d)", size);
	}
}


/* Prints out an operand in X86-specific syntax. */
void
PrinterX86::print_opnd(Opnd o)
{
    if (is_null(o)) {
	// print nothing

    } else if (is_var(o)) {
	print_sym(get_var(o));

    } else if (is_hard_reg(o)) {
	int sz = get_bit_size(get_type(o));
	int reg = get_reg(o);

	// A GPR register operand may have been downsized (never upsized)
	// by giving it a type narrower than the operand to which its
	// register number was assigned.  If the narrowing is enough to
	// cause a name change, extract the two-letter suffix of the given
	// reg name (e.g., `ax' from `eax' or `ax') and either print that
	// (if the value has word width) or combine its first letter with
	// `l' to name a low-order byte subregister.

	if (   sz > 16
	    || sz == reg_width(reg)
	    || !reg_allocables()->contains(reg))
	    fprintf(out, "%%%s", reg_name(reg));
	else {
	    const char *suffix =
		reg_name(reg) + (int)(reg_width(reg) == 32);
	    if (sz > 8)
		fprintf(out, "%%%s", suffix);
	    else
		fprintf(out, "%%%c%c", *suffix, 'l');
	}
    } else if (is_virtual_reg(o)) {
	fprintf(out, "$vr%d", get_reg(o));

    } else if (is_immed_integer(o)) {
	fputc('$', out);
	fprint(out, get_immed_integer(o));

    } else if (is_immed_string(o)) {
	fprintf(out, "$%s", get_immed_string(o).chars());

    } else if (is_addr_sym(o)) {
	print_sym(get_sym(o));

    } else if (is_addr_exp(o)) {
	print_addr_exp(o);

    } else {
	claim(false, "print_opnd() -- unknown kind %d", get_kind(o));
    }
}


/* Helper routines for print_addr_exp. */
void
PrinterX86::print_sym_disp(Sym *s, int d)
{
    // print first operand
    print_sym(s);

    // deal with the displacement
    if (d == 0)
	return;
    fprintf(out, "%+d", d);
}

void
PrinterX86::print_disp(Opnd d)
{
    if (is_immed_integer(d))
	fprintf(out, "%d", get_immed_int(d));
    else if (is_addr_sym(d))
	print_sym(get_sym(d));
    else
	claim(false);
}


/* Prints out an effective-address operand in X86-specific
 * syntax. This routine expects the EA operand as an expression.  
 *
 * All of the index-reg-based forms are printed as
 * 'disp(base_reg,index_reg,scale)'.  Both the scale factor and the
 * displacement are already in bytes.  The printing of scale and disp
 * do not use print_opnd because we don't want a '$' in front of
 * either one. */
void
PrinterX86::print_addr_exp(Opnd o)
{
    if (BaseDispOpnd bdo = o) {
	Opnd base = bdo.get_base();
	if (is_reg(base)) {
	    if (base == opnd_reg_const0) {
		// Special case since x86 doesn't have a const0 register.
		// Just print the displacement (an absolute address.)
		print_opnd(bdo.get_disp());
	    } else {
		// Print as 'offset(base_reg)' -- offset already in bytes.
		// Don't use print_opnd because we don't want a '$' in
		// front of the offset.  Omit a zero displacement to
		// simplify the instruction.
		Opnd disp = bdo.get_disp();
		if (!is_immed_integer(disp) || get_immed_integer(disp) != 0)
		    print_disp(disp);
		fprintf(out,"(");
		print_opnd(base);
		fprintf(out,")");
	    }

	} else {
	    /* print as 's0+s1' */
	    claim(is_var(base));
	    print_sym_disp(get_var(base), get_immed_int(bdo.get_disp()));
	}

    } else if (SymDispOpnd sdo = o) {
	// print as 'symbol+offset' -- offset should be unsigned
	print_sym_disp(get_sym(sdo.get_addr_sym()),
		       get_immed_int(sdo.get_disp()));

    } else if (IndexScaleDispOpnd isdo = o) {
	print_disp(isdo.get_disp());
	fprintf(out, "(,");
	print_opnd(isdo.get_index());
	fprintf(out, ",%d)", get_immed_int(isdo.get_scale()));

    } else if (BaseIndexDispOpnd bido = o) {
	print_disp(bido.get_disp());
	fprintf(out, "(");
	print_opnd(bido.get_base());
	fprintf(out, ",");
	print_opnd(bido.get_index());
	fprintf(out, ",0)");

    } else if (BaseIndexScaleDispOpnd bisdo = o) {
	print_disp(bisdo.get_disp());
	fprintf(out, "(");
	print_opnd(bisdo.get_base());
	fprintf(out, ",");
	print_opnd(bisdo.get_index());
	fprintf(out, ",%d)", get_immed_int(bisdo.get_scale()));

    } else {
	claim(false, "unexpected address expression");
    }
}


void
PrinterX86::print_global_decl(FileBlock *fb)
{
    // do nothing
}


void
PrinterX86::print_extern_decl(VarSym *v)
{
    // do nothing
}


void
PrinterX86::print_file_decl(int fnum, IdString fnam)
{
    fprintf(out, "\t%s\t", x86_opcode_names[OPCODE_FILE]);
    fprintf(out, "\"%s\"", fnam.chars());	// file name
    fprintf(out, "\t# %d\n", fnum);		// file no.
}


/* Starts a procedure text segment. */
void
PrinterX86::print_proc_begin(OptUnit *)
{
    fprintf(out, "\t%s\n", x86_opcode_names[TEXT]);		/* .text */
    fprintf(out, "\t%s\t%d\n", x86_opcode_names[ALIGN], 2);	/* .align 2 */
}


/* Reads the symbol table information and the annotations associated
 * with the OptUnit.  It creates the correct X86 instructions to
 * start a procedure text segment. */
void
PrinterX86::print_proc_entry(OptUnit *unit, int file_no_for_1st_line)
{
    Sym *psym = get_proc_sym(unit);
    const char *cur_unit_name = get_name(psym).chars();

    fprintf(out, "\t%s\t%d\n", x86_opcode_names[ALIGN], 2);	/* .align 2 */

    // If global procedure, then add this pseudo-op.  I.e. procedure is
    // in global symbol table, not in the file symbol table.
    if (is_global(psym) && !is_private(psym))
	fprintf(out, "\t%s\t%s\n", x86_opcode_names[GLOBL],	/* .globl */
		     cur_unit_name);

    // Get file and line no. info for this procedure -- .file directive
    // already generated (if necessary) in print_file_decl().
    int first_line_no = ((const LineNote&)get_note(unit, k_line)).get_line();
    fprintf(out, "\t%s\t%d %d\n", x86_opcode_names[LOC],	/* .loc */
		 file_no_for_1st_line, first_line_no);

    // The procedure symbol cannot be a label symbol since SUIF does not
    // allow label symbols outside of a procedural context (this is the
    // reason why this routine exists in the first place), and so, we
    // have to generate the procedure label in a funny way.
    fprintf(out, "%s:\n", cur_unit_name);			/* proc: */

    // rest (if any) added to text during code gen
}


void
PrinterX86::print_proc_end(OptUnit *unit)
{
    // do nothing
}


///////////////////////////////////////////////////////////////
////////// Rest of this file is SUIF-specific /////////////////
///////////////////////////////////////////////////////////////

/* Helper function for print_var_def(). */
int
PrinterX86::print_size_directive(TypeId t)
{
    DataType *dt = unqualify_data_type(t);
    int opcode;
    int bits_filled = dt->get_bit_size().c_int();

    switch (bits_filled) {
      case 8:
	opcode = BYTE;
	break;
      case 16:
	opcode = WORD;
	break;
      case 32: 
	if (is_a<FloatingPointType>(t))
	    opcode = FLOAT;
	else
	    opcode = LONG;
	break;
      case 64:
	claim(is_a<FloatingPointType>(t));
	opcode = DOUBLE;
	break;
      default:
	claim(false, "unexpected size in value block");
    }

    if ((opcode != cur_opcode) || (cur_opnd_cnt > 20)) {
	fprintf(out, "\n\t%s\t", x86_opcode_names[opcode]);
	cur_opcode = opcode;
	cur_opnd_cnt = 0;
    } else
	fprintf(out, ", ");

    return bits_filled;
}


int
PrinterX86::print_bit_filler(int bit_count)
{
    claim((bit_count % 8) == 0);
    int fill_count = bit_count / 8;
    if (fill_count > 1)
	fprintf(out, "\n\t%s\t%d", x86_opcode_names[REPEAT], fill_count);
    fprintf(out, "\n\t%s\t0", x86_opcode_names[BYTE]);
    if (fill_count > 1) {
	fprintf(out, "\n\t%s", x86_opcode_names[ENDR]);
	cur_opcode = opcode_null;	// force new data directive
    }
    return bit_count;
}

/* Another helper function for print_var_def().   This function
 * expects that it will need to add a return before printing
 * anything, and it expects that you will add a return after
 * it is done.
 * It returns the number of bits of initialization data that it
 * generates.
 */
int
PrinterX86::process_value_block(ValueBlock *vblk)
{
    int bits_filled = 0;

    if (is_a<ExpressionValueBlock>(vblk)) {
	// An ExpressionValueBlock either holds a constant value or a
	// simple expression representing either an address relative to
	// a symbol or a pointer generated by conversion from an integer
	// constant (presumably zero).

	ExpressionValueBlock *evb = (ExpressionValueBlock*)vblk;
	Expression *exp = evb->get_expression();

	if (is_a<IntConstant>(exp)) {
	    IntConstant *ic = (IntConstant*)exp;
	    bits_filled = print_size_directive(ic->get_result_type());
	    fprint(out, ic->get_value());
	    cur_opnd_cnt++;
	}
	else if (is_a<FloatConstant>(exp)) {
	    FloatConstant *fc = (FloatConstant*)exp;
	    bits_filled = print_size_directive(fc->get_result_type());
	    fputs(fc->get_value().c_str(), out);
	    cur_opnd_cnt++;
	}
	else {
	    // We use non-constant ExpressionValueBlocks to hold a symbolic
	    // address, optionally with an offset, or a pointer initializer
	    // consisting of a `convert' expression.
	    // A SymbolAddressExpression represents a plain symbolic
	    // address.
	    // An address obtained by conversion from another type is a
	    // unary expression with opcode "convert".
	    // A symbol+delta is represented by an add whose first operand
	    // is the load-address and whose second is an IntConstant
	    // expression.
	    // No other kind of "expression" is recognized.

	    if (is_a<UnaryExpression>(exp)) {
		UnaryExpression *ux = (UnaryExpression*)exp;
		claim(ux->get_opcode() == k_convert,
		      "unexpected unary expression in expression block");
		DataType *tt = ux->get_result_type();	// target type
		claim(is_kind_of<PointerType>(tt),
		      "unexpected target type when converting initializer");
		exp = ux->get_source();
		if (is_a<IntConstant>(exp)) {
		    IntConstant *ic = (IntConstant*)exp;
		    bits_filled = print_size_directive(tt);
		    fprint(out, ic->get_value());
		    cur_opnd_cnt++;
		    return bits_filled;
		}
		// Fall through to handle symbol-relative address, on the
		// assumption that the front end validated the conversion.
	    }
	    SymbolAddressExpression *sax;
	    long delta;
	    if (is_a<BinaryExpression>(exp)) {
		BinaryExpression *bx = (BinaryExpression*)exp;
		claim(bx->get_opcode() == k_add,
		      "unexpected binary expression in expression block");
		Expression *s1 = bx->get_source1();
		Expression *s2 = bx->get_source2();
		sax = to<SymbolAddressExpression>(s1);
		delta = to<IntConstant>(s2)->get_value().c_long();
	    } else if (is_a<SymbolAddressExpression>(exp)) {
		sax = (SymbolAddressExpression*)exp;
		delta = 0;
	    } else {
		claim(false, "unexpected kind of expression block");
	    }
	    Sym *sym = sax->get_addressed_symbol();

	    // symbol initialization
	    bits_filled = print_size_directive(type_ptr);
	    print_sym(sym);
	    if (delta != 0) {
		fprintf(out, "%+ld", delta);
	    }
	    // always force the start of a new data directive
	    cur_opcode = opcode_null;
	    cur_opnd_cnt = 0;
	}
    }

    else if (is_a<MultiValueBlock>(vblk)) {
	MultiValueBlock *mvb = (MultiValueBlock*)vblk;
	for (int i = 0; i < mvb->get_sub_block_count(); i++ ) {
	    int offset = mvb->get_sub_block(i).first.c_int();
	    claim(offset >= bits_filled);

	    if (bits_filled < offset)		// pad to offset
		bits_filled += print_bit_filler(offset - bits_filled);
	    bits_filled += process_value_block(mvb->get_sub_block(i).second);
	}
	int all_bits = get_bit_size(mvb->get_type());
	if (all_bits > bits_filled)
	    bits_filled += print_bit_filler(all_bits - bits_filled);

    } else if (is_a<RepeatValueBlock>(vblk)) {
	// Simplifying assumption: We now expect the front-end
	// to remove initialization code of large arrays of zeros.
	RepeatValueBlock *rvb = (RepeatValueBlock*)vblk;
	int repeat_cnt = rvb->get_num_repetitions();

	if (repeat_cnt > 1) {		// insert .repeat pseudo-op
	    fprintf(out, "\n\t%s\t%d", x86_opcode_names[REPEAT], repeat_cnt);
	    cur_opcode = opcode_null;	// force new data directive
	}
	// actual data directive
	bits_filled = repeat_cnt * process_value_block(rvb->get_sub_block());

	if (repeat_cnt > 1) {		// insert .endr pseudo-op
	    fprintf(out, "\n\t%s", x86_opcode_names[ENDR]);
	    cur_opcode = opcode_null;	// force new data directive
	}

    } else if (is_a<UndefinedValueBlock>(vblk)) {
	bits_filled += print_bit_filler(get_bit_size(vblk->get_type()));

    } else {
	claim(false, "unexpected kind of ValueBlock");
    }
    return bits_filled;
}

/*
 * print_var_def() -- Generates assembly language data statements.
 * Data objects are put into the sdata section, data section, comm
 * section, or the lcomm section.  If the data object is global, I
 * also specify a .globl pseudo-op.
 */
void
PrinterX86::print_var_def(VarSym *vsym)
{
    int vsym_size = get_bit_size(get_type(vsym)) >> 3;	// in bytes
    VariableDefinition *vdef = vsym->get_definition();

    if (vdef == NULL
	|| (vdef->get_initialization() == NULL)	// FIXME: shouldn't be possible
	||  is_a<UndefinedValueBlock>(vdef->get_initialization())) {

	// Uninitialized data item.  If global symbol, append to
	// .comm, else static symbol to .lcomm.
	if (is_global(vsym) && !is_private(vsym))
	    fprintf(out, "\t%s\t", x86_opcode_names[COMM]);
	else
	    fprintf(out, "\t%s\t", x86_opcode_names[LCOMM]);
	print_sym(vsym);
	fprintf(out, ", %d\n", vsym_size);

    } else {		// initialized data item 

	// Specify data section to place data in. On x86, always use
	// .data section.
	fprintf(out, "\t%s\n", x86_opcode_names[DATA]);

	// If global symbol, add .globl pseudo-op.  Note that a variable
	// is considered global in SUIF if it is part of the global or
	// file symbol table.  For X86 assembly, a datum is global only
	// if it is part of the global symbol table (i.e. is_private()
	// checks for def in file symbol table).
	if (is_global(vsym) && !is_private(vsym)) {
	    fprintf(out, "\t%s\t", x86_opcode_names[GLOBL]);
	    print_sym(vsym);
	    fprintf(out, "\n");
	}

	// Determine alignment and align
	int bit_alignment = get_bit_alignment(vdef);
	// If vdef's alignment is zero (shouldn't be!), use that in vsym's type
	if (bit_alignment == 0)
	    bit_alignment = get_bit_alignment(get_type(vsym));
	int alignment_value;
	switch (bit_alignment) {
	  case 8:			// byte aligned
	    alignment_value = 0;
	    break;
	  case 16:			// halfword aligned
	    alignment_value = 1;
	    break;
	  case 32:			// word aligned
	    alignment_value = 2;
	    break;
	  case 64:			// double word aligned
	    alignment_value = 3;
	    break;
	  case 128:			// quad word aligned
	    alignment_value = 4;
	    break;
	  default:
	    claim(false, "bad alignment for %s", get_name(vsym).chars());
	}
	fprintf(out, "\t%s\t%d\n", x86_opcode_names[ALIGN], alignment_value);

	// Disable automatic alignment
	fprintf(out, "\t%s\t%d\t# %s\n", x86_opcode_names[ALIGN], 0,
		      "disable automatic alignment");

	// Write out the label
	print_sym(vsym);
	fprintf(out, ":");		// return added in process_value_block

	// initialize memory with values
	cur_opcode = opcode_null;
	cur_opnd_cnt = 0;
	process_value_block(vdef->get_initialization());
	fprintf(out, "\n");
   }
}
