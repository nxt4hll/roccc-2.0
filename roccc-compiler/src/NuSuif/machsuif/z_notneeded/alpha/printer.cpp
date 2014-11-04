/* file "alpha/printer.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "alpha/printer.h"
#endif

#include <machine/machine.h>

#include <alpha/init.h>
#include <alpha/opcodes.h>
#include <alpha/instr.h>
#include <alpha/code_gen.h>
#include <alpha/printer.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace alpha;

PrinterAlpha::PrinterAlpha()
{
    int i;
    print_instr_table.resize(LAST_ALPHA_OPCODE + 1);

    print_instr_table[opcode_null] = &Printer::print_instr_dot;
    print_instr_table[opcode_label] = &Printer::print_instr_label;

    for (i = ENT; i <= X_FLOATING; i++)
	print_instr_table[i] = &Printer::print_instr_dot;

    for (i = LDIL; i <= ZAPNOT; i++)
	print_instr_table[i] = &Printer::print_instr_alm;

    for (i = BR; i <= CALL_PAL; i++)
	print_instr_table[i] = &Printer::print_instr_cti;

    for (i = FETCH; i <= CMPTUN; i++)
	print_instr_table[i] = &Printer::print_instr_alm;

    for (i = FBEQ; i <= FBGE; i++)
	print_instr_table[i] = &Printer::print_instr_cti;

    print_instr_table[MF_FPCR] = &Printer::print_instr_alm;
    print_instr_table[MT_FPCR] = &Printer::print_instr_alm;
}


void
PrinterAlpha::print_instr(Instr *mi)
{
    int opcode = get_opcode(mi);
    if (opcode > LAST_ALPHA_OPCODE) {
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


/* Print out label in Alpha format. */
void
PrinterAlpha::print_instr_label(Instr *l)
{
    print_sym(get_label(l));
    fprintf(out, ":");
    print_reloc(l);
    print_notes(l);
}


/* Print out a cti instruction.  Print "rd, rs1, rs2, target" in that
 * order, or "rd, rs" if indirect jump. */
void
PrinterAlpha::print_instr_cti(Instr *bj)
{
    // print out opcode and any extensions
    print_opcode(bj);

    // print out destination if it exists
    if (dsts_size(bj)) {
	claim(is_reg(get_dst(bj)),
	       "found cti dst opnd that is not a REG");
	print_opnd(get_dst(bj));
	fprintf(out, ",");		// prepare for target (at least)
    }

    if (get_opcode(bj) == CALL_PAL) {
	// only specifier is an immediate
	print_opnd(get_src(bj, 0));

    } else if (is_indirect(bj)) {
	// operands for indirect target surrounded by ()'s
	fprintf(out, "(");
	print_opnd(get_src(bj, 0));
	fprintf(out, ")");
	// then print out the rest of the source registers, if any
	if (srcs_size(bj) > 1) {
	    claim(srcs_size(bj) == 2);
	    print_opnd(get_src(bj, 1));
	}

    } else {
	// first print out source registers, if any
	for (int i = 0; i < srcs_size(bj); i++) {
	    if (!is_null(get_src(bj, i))) {
		print_opnd(get_src(bj, i));
		fprintf(out, ",");	// prepare for target (at least)
	    }
	}

	// print out target address
	print_sym(get_target(bj));
    }

    if (get_opcode(bj) == alpha::RET)
	fprintf(out, ",1");		// print hint

    // print more general hint, if one is attached
    if (OneNote<IdString> note = get_note(bj, k_hint))
	fprintf(out, ",%s", note.get_value().chars());

    print_reloc(bj);
    print_notes(bj);
}


/* Print out an ALU or memory instruction.  Compaq/Digital Alpha
 * instructions that print out their operands in a weird order are
 * handled here. */
void
PrinterAlpha::print_instr_alm(Instr *mi)
{
    // print out opcode and any extensions
    print_opcode(mi);

    // Print out any operands -- some opcodes require special printing 
    // format for the operands.  Check for lda and ldgp since they
    // are printed like a load but do not read memory.
    int opcode = get_opcode(mi);
    if (reads_memory(mi) || (opcode == alpha::LDA) || (opcode == LDAH)
    || (opcode == LDGP)) { 				// is load
	if ((opcode == FETCH) || (opcode == FETCH_M)) {
	    // no destination in prefetch operation
	} else {
	    claim(!is_null(get_dst(mi)));
	    print_opnd(get_dst(mi));
	}
	fprintf(out, ",");

	// print out effective address operand
	claim(srcs_size(mi) > 0);
	print_opnd(get_src(mi, 0));

    } else if (writes_memory(mi)) {			// is store
	claim(dsts_size(mi) && srcs_size(mi));
	print_opnd(get_src(mi, 0));
	fprintf(out, ",");
	print_opnd(get_dst(mi));

    } else if (is_ldc_alpha(mi)) {
	// print out "dest, imm" in that order
	print_opnd(get_dst(mi));
	fprintf(out, ",");
	print_opnd(get_src(mi, 0));

    } else if (is_cmove_alpha(mi)) {
	// don't print out the last source which corresponds to
	// the destination -- needed for correct liveness analysis
	print_opnd(get_src(mi, 0));
	fprintf(out, ",");
	print_opnd(get_src(mi, 1));
	fprintf(out, ",");
	print_opnd(get_dst(mi));

    } else {
	// print out "src1, src2, dest" in that order
	bool need_comma = false;
	for (int i = 0; i < srcs_size(mi); i++) {
	    Opnd s = get_src(mi, i);
	    if (!is_null(s)) {
		if (need_comma) fprintf(out, ",");
		else need_comma = true;		// next time
		print_opnd(s);
	    }
	}
	if (dsts_size(mi) > 0) {
	    if (need_comma) fprintf(out, ",");
	    print_opnd(get_dst(mi));
	}
    }

    print_reloc(mi);
    print_notes(mi);
}


/* Print out pseudo-op in Compaq/Digital Alpha format. */
void
PrinterAlpha::print_instr_dot(Instr *x)
{
    int i;

    // special case opcode_null so we don't output null lines
    if (get_opcode(x) == opcode_null) {
	print_notes(x);
	return;
    }

    claim(is_null(get_note(x, k_instr_opcode_exts)),
	  "print_instr_dot() -- unexpected k_instr_opcode_exts");

    // print out opcode
    fprintf(out, "\t%s\t", alpha_opcode_names[get_opcode(x)]);

    switch (get_opcode(x)) {
      case alpha::FILE:			// .file <file_no> "<file_name>"
	claim(srcs_size(x) == 2);
	fprintf(out, "%d \"", get_immed_int(get_src(x, 0)));
	fprintf(out, "%s\"",  get_immed_string(get_src(x, 1)).chars());
	break;

      case FRAME:			// .frame $sp,framesize,$26
	claim((srcs_size(x) == 3) || (srcs_size(x) == 4));
	claim(get_immed_integer(get_src(x, 0)) == 30);
	fprintf(out, "$sp, %d, $26", get_immed_int(get_src(x, 1)));
	claim(get_immed_integer(get_src(x, 2)) == 26);
	if (srcs_size(x) == 4)
	    fprintf(out, ", %d", get_immed_int(get_src(x, 3)));
	break;

      case MASK: case FMASK:	// .*mask <hex_mask>, <f_offset>
	claim(srcs_size(x) == 2);
	fprintf(out, "0x%08x, ", get_immed_int(get_src(x, 0)));
	fprintf(out, "%d",	  get_immed_int(get_src(x, 1)));
	break;

      case ASCII: case ASCIIZ:	// .ascii* ["<s1>"[, "<s2>"...]]
	for (i = 0; i < srcs_size(x); i++) {
	    // while >1 element in list, print with comma
	    fprintf(out, "\"%s\"", get_immed_string(get_src(x, i)).chars());
	    if (!i)
		fprintf(out, ", ");
	}
	break;

      // Print arguments without commas and strings without quotes
      case ENT: case EXTERN: case LIVEREG:
      case LOC: case VERSTAMP: case VREG:
	for (i = 0; i < srcs_size(x); i++) {
	    Opnd opnd = get_src(x, i);
	    claim(is_immed(opnd));

	    if (is_immed_integer(opnd))
		fprintf(out, "%d", get_immed_int(opnd));
	    else
		fprintf(out, "\"%s\"", get_immed_string(opnd).chars());

	    fprintf(out, " ");
	}
	break;

      // Print arguments with commas and strings without quotes
      default:
	for (i = 0; i < srcs_size(x); i++) {
	    // while >1 element in list, print with comma
	    Opnd opnd = get_src(x, i);
	    claim(is_immed(opnd));

	    if (is_immed_integer(opnd))
		fprintf(out, "%d", get_immed_int(opnd));
	    else
		fprintf(out, "\"%s\"", get_immed_string(opnd).chars());

	    if (!i)
		fprintf(out, ", ");
	}
	break;
    }

    print_reloc(x);
    print_notes(x);
}

/* Prints out the machine opcode and any extensions in Compaq/Digital
 * Alpha-specific syntax. */
void
PrinterAlpha::print_opcode(Instr *mi)
{
    fprintf(out, "\t%s", alpha_opcode_names[get_opcode(mi)]);

    if (ListNote<long> note = get_note(mi, k_instr_opcode_exts)) {
	for (int i = 0; i < note.values_size(); ++i) {
	    int e_op = note.get_value(i);
	    fprintf(out, "%s", alpha_opcode_ext_names[e_op]);
	    /* normally, we'd print the extension separator each time
	     * too, but Alpha has none */
	}
    }
    fprintf(out, "\t");
}


/* Helper routine for print_addr_exp. */
void
PrinterAlpha::print_sym_disp(Sym *s, int d)
{
    // print first operand
    print_sym(s);

    // deal with the displacement
    if (d == 0)
	return;
    fprintf(out, "%+d", d);
}

/* Prints out an effective-address operand in Compaq/Digital
 * Alpha-specific syntax. This routine expects the EA operand as an
 * expression. */
void
PrinterAlpha::print_addr_exp(Opnd o)
{
    if (BaseDispOpnd bdo = o) {
	    // print as 'offset(base_reg)' -- offset already in bytes
	    print_opnd(bdo.get_disp());
	    fprintf(out, "(");
	    print_opnd(bdo.get_base());
	    fprintf(out, ")");

    } else if (SymDispOpnd sdo = o) {
	// print as 'symbol+offset' -- offset should be unsigned
	print_sym_disp(get_sym(sdo.get_addr_sym()),
		       get_immed_int(sdo.get_disp()));

    } else if (IndexSymDispOpnd isdo = o) {
	// print as 'sym+disp($idx_reg)' -- offset already in bytes
	print_sym_disp(get_sym(isdo.get_addr_sym()),
		       get_immed_int(isdo.get_disp()));
	fprintf(out, "(");
	print_opnd(isdo.get_index());
	fprintf(out, ")");

    } else {
	claim(false, "unexpected address expression");
    }
}


/* Prints out an operand in Compaq/Digital Alpha-specific syntax. */
void
PrinterAlpha::print_opnd(Opnd o)
{
    if (is_null(o)) {
	// print nothing

    } else if (is_var(o)) {
	print_sym(get_var(o));

    } else if (is_hard_reg(o)) {
	fprintf(out, "$%s", reg_name(get_reg(o)));

    } else if (is_virtual_reg(o)) {
	fprintf(out, "$vr%d", get_reg(o));

    } else if (is_immed_integer(o)) {
	fprint(out, get_immed_integer(o));

    } else if (is_immed_string(o)) {
	fprintf(out, "%s", get_immed_string(o).chars());

    } else if (is_addr_sym(o)) {
	print_sym(get_sym(o));

    } else if (is_addr_exp(o)) {
	print_addr_exp(o);

    } else {
	claim(false, "print_opnd() -- unknown kind %d", get_kind(o));
    }
}


/* Prints out the relocation information in Alpha-specific syntax.
 * Omit the operand-index field of the annotation. */
void 
PrinterAlpha::print_reloc(Instr *mi)
{
    if (RelocNote note = get_note(mi, k_reloc))
	fprintf(out, "!%s!%d", note.get_name().chars(), note.get_seq());
}

void
PrinterAlpha::print_extern_decl(VarSym *v)
{
    fprintf(out, "\t%s\t", alpha_opcode_names[EXTERN]);
    print_sym(v);

    int bit_size = get_bit_size(get_type(v));	// -1 if unavailable
    if (bit_size >= 0)
	fprintf(out, " %d", bit_size >> 3);
    putc('\n', out);
}


void
PrinterAlpha::print_file_decl(int fnum, IdString fnam)
{
    // If file name is empty, owing to front end bug, comment out the
    // .file directive to avoid upsetting the assembler.
    if (fnam.chars()[0] == '\0')
	fprintf(out, " #");
    fprintf(out, "\t%s\t", alpha_opcode_names[alpha::FILE]);
    fprintf(out, "%d \"", fnum);		// file no.
    fprintf(out, "%s\"\n", fnam.chars());	// file name
}


/* If we inserted relocations, we must disable code reordering by the
 * assembler.  If we do not, it will reorder the instructions
 * including those that define $gp.  Since these instructions depend
 * upon their placement, do not move them! */
void
PrinterAlpha::print_global_decl(FileBlock *fb)
{
    if (is_null(get_note(fb, k_next_free_reloc_num)))
	return;

    // This annotation exists only if we inserted relocations.  So,
    // disallow code reordering and also globally allow for the
    // use of the $at register.
    fprintf(out, "\t%s\t%s\n", alpha_opcode_names[SET], "noreorder");
    fprintf(out, "\t%s\t%s\n", alpha_opcode_names[SET], "noat");
}


/* Starts a procedure text segment. */
void
PrinterAlpha::print_proc_begin(OptUnit *)
{
    fprintf(out, "\t%s\n", alpha_opcode_names[TEXT]);	// .text
    fprintf(out, "\t%s\t%d\n", alpha_opcode_names[ALIGN], 4);// .align 4
}


/* Reads the symbol table information and the annotations associated
 * with the OptUnit.  It creates the correct Alpha
 * instructions to start a procedure text segment. */
void
PrinterAlpha::print_proc_entry(OptUnit *unit,
			       int file_no_for_1st_line)
{
    const char *cur_unit_name = get_name(unit).chars();

    fprintf(out, "\t%s\t%d\n", alpha_opcode_names[ALIGN], 4);// .align 4 

    // If global procedure, then add this pseudo-op.  I.e. procedure is
    // in global symbol table, not in the file symbol table.
    Sym *psym = get_proc_sym(unit);
    if (is_global(psym) && !is_private(psym))
	fprintf(out, "\t%s\t%s\n", alpha_opcode_names[GLOBL],// .globl 
		     cur_unit_name);

    // Get file and line no. info for this procedure -- .file directive
    // already generated (if necessary) in print_file_decl().
    int first_line_no = ((const LineNote&)get_note(unit, k_line)).get_line();
    fprintf(out, "\t%s\t%d %d\n", alpha_opcode_names[LOC],	// .loc 
		 file_no_for_1st_line, first_line_no);

    fprintf(out, "\t%s\t%s\n", alpha_opcode_names[ENT],	// .ent 
		 cur_unit_name);

    // The procedure symbol cannot be a label symbol since SUIF does not
    // allow label symbols outside of a procedural context (this is the
    // reason why this routine exists in the first place), and so, we
    // have to generate the procedure label in a funny way.
    fprintf(out, "%s:\n", cur_unit_name);			// proc: 

    // .frame (and others if not leaf) added to text during code gen
}


/* Outputs the .end pseudo-op. */
void
PrinterAlpha::print_proc_end(OptUnit *unit)
{
    const char *cur_unit_name = get_name(get_proc_sym(unit)).chars();
    fprintf(out, "\t%s\t%s\n", alpha_opcode_names[END],	// .end 
		 cur_unit_name);
}


///////////////////////////////////////////////////////////////
////////// Rest of this file is SUIF-specific /////////////////
///////////////////////////////////////////////////////////////

/* Helper function for print_var_def(). */
int
PrinterAlpha::print_size_directive(TypeId t)
{
    int bits_filled = get_bit_size(t);
    int opcode;

    switch (bits_filled) {
      case 8:
	opcode = BYTE;
	break;
      case 16:
	opcode = WORD;
	break;
      case 32: 
	if (is_floating_point(t))
	    opcode = S_FLOATING;
	else
	    opcode = LONG;
	break;
      case 64:
	if (is_floating_point(t))
	    opcode = T_FLOATING;
	else
	    opcode = QUAD;
	break;
      default:
	claim(false, "unexpected size in value block");
    }

    if ((opcode != cur_opcode) || (cur_opnd_cnt > 20)) {
	fprintf(out, "\n\t%s\t", alpha_opcode_names[opcode]);
	cur_opcode = opcode;
	cur_opnd_cnt = 0;
    } else
	fprintf(out, ", ");

    return bits_filled;
}


int
PrinterAlpha::print_bit_filler(int bit_count)
{
    claim((bit_count % 8) == 0);
    int fill_count = bit_count / 8;
    if (fill_count > 1)
	fprintf(out, "\n\t%s\t%d", alpha_opcode_names[REPEAT], fill_count);
    fprintf(out, "\n\t.byte\t0");
    if (fill_count > 1) {
	fprintf(out, "\n\t%s", alpha_opcode_names[ENDR]);
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
PrinterAlpha::process_value_block(ValueBlock *vblk)
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

	    // Two kinds are possible: generic symbol initializations and
	    // ones done for gp-relative initialization.  We use an
	    // annotation on the value block to signal the second kind.
	    if (is_null(get_note(evb, k_gprel_init))) {
		bits_filled = print_size_directive(type_addr());
	    } else {
		// Alpha-specific initialization -- generate a .gprel32
		// pseudo-op for the vsym.  The addressed symbol is the
		// one whose GP-offset we want.
		fprintf(out, "\n\t%s\t", alpha_opcode_names[GPREL32]);
		claim(delta == 0);
	    }
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
	    fprintf(out, "\n\t%s\t%d", alpha_opcode_names[REPEAT],
		    repeat_cnt);
	    cur_opcode = opcode_null;	// force new data directive
	}
	// actual data directive
	bits_filled = repeat_cnt * process_value_block(rvb->get_sub_block());

	if (repeat_cnt > 1) {		// insert .endr pseudo-op
	    fprintf(out, "\n\t%s", alpha_opcode_names[ENDR]);
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
 *
 * There is a special case though to cover the problem of the front end
 * creating a big array filled with zeros.  If the array is big enough
 * and I tell the assembler to initialize it, the assembler will die.
 * Consequently, I check to see if the data item has only a single
 * annotation, the annotation fills with zeros, and the item is bigger
 * than 1024 bytes in size.  If so, the data item is made a .comm and
 * I rely on UNIX to initialize it to zeros at link time.
 *
 * If the data item is bigger than 64K bytes, I used to (for the MIPS)
 * print a warning that the assembler may die.  This may be irrelevant on
 * the Alpha.
 */
void
PrinterAlpha::print_var_def(VarSym *vsym)
{
    int vsym_size = get_bit_size(get_type(vsym)) >> 3;		// in bytes
    VariableDefinition *vdef = vsym->get_definition();

    if (vdef == NULL
	|| (vdef->get_initialization() == NULL)	// FIXME: shouldn't be possible
	||  is_a<UndefinedValueBlock>(vdef->get_initialization())) {

	// Uninitialized data item.  If global symbol, append to
	// .comm, else static symbol to .lcomm.
	if (is_global(vsym) && !is_private(vsym))
	    fprintf(out, "\t%s\t", alpha_opcode_names[COMM]);
	else
	    fprintf(out, "\t%s\t", alpha_opcode_names[LCOMM]);
	print_sym(vsym);
	fprintf(out, ", %d\n", vsym_size);

    } else {		// initialized data item 

	// Specify data section to place data in. By default, if the
	// data is greater than 512 bytes in size, we use .data,
	// otherwise we use .sdata.
	if (vsym_size > Gnum)
	    fprintf(out, "\t%s\n", alpha_opcode_names[DATA]);
	else
	    fprintf(out, "\t%s\n", alpha_opcode_names[SDATA]);

	// If global symbol, add .globl pseudo-op.  Note that a variable
	// is considered global in SUIF if it is part of the global or
	// file symbol table.  For Alpha assembly, a datum is global only
	// if it is part of the global symbol table (i.e. is_private()
	// checks for def in file symbol table).
	if (is_global(vsym) && !is_private(vsym)) {
	    fprintf(out, "\t%s\t", alpha_opcode_names[GLOBL]);
	    print_sym(vsym);
	    fprintf(out, "\n");
	}

	// Determine alignment and align
	int bit_alignment = vdef->get_bit_alignment();
	int alignment_value;
	switch (bit_alignment) {
	  case 8:			// byte aligned
	    alignment_value = 0;
	    break;
	  case 16:			// halfword aligned
	    alignment_value = 1;
	    break;
	  case 32:			// word aligned
	    // alpha defaults int alignment to double-word alignment
	    alignment_value = 3;
	    break;
	  case 64:			// double word aligned
	    alignment_value = 3;
	    break;
	  case 128:			// quad word aligned
	    alignment_value = 4;
	    break;
	  default:
	    warn("print_var_def() -- bad alignment for %s",
		 get_name(vsym).chars());
	    alignment_value = 4;
	    break;
	}
	// FIXME: This is a workaround for a link-time optimizer bug.  At worst,
	// it can go away when we do our own expansion of unimplemented operations.
	alignment_value = 4;

	fprintf(out, "\t%s\t%d\n", alpha_opcode_names[ALIGN],
		     alignment_value);

	// Disable automatic alignment
	fprintf(out, "\t%s\t%d\t# %s\n", alpha_opcode_names[ALIGN], 0,
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
