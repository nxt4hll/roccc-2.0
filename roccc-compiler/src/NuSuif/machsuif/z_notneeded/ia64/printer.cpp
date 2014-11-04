/* file "ia64/printer.cpp" */

/*
    Copyright (c) 2002 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "ia64/printer.h"
#endif

#include <machine/machine.h>

#include <ia64/init.h>
#include <ia64/opcodes.h>
#include <ia64/instr.h>
#include <ia64/code_gen.h>
#include <ia64/reg_info.h>
#include <ia64/printer.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace ia64;

PrinterIa64::PrinterIa64()
{
    int i;
    print_instr_table.resize(LAST_IA64_OPCODE_STEM + 1);

    print_instr_table[opcode_null] = &Printer::print_instr_dot;
    print_instr_table[opcode_label] = &Printer::print_instr_label;

    for (i = ALIAS; i <= VREGUNDEF; i++)
	print_instr_table[i] = &Printer::print_instr_dot;

    for (i = CMPXCHG; i <= ZXT; i++)
	print_instr_table[i] = &Printer::print_instr_alm;

    for (i = BR; i <= BRL; i++)
	print_instr_table[i] = &Printer::print_instr_cti;

    for (i = RFI; i <= FSETC; i++)
	print_instr_table[i] = &Printer::print_instr_alm;
}


void
PrinterIa64::print_instr(Instr *mi)
{
    int opcode = get_opcode(mi);
    int stem = get_stem(opcode);
    if (stem > LAST_IA64_OPCODE_STEM) {
	// Found opcode outside range of opcodes defined in this library.
	// Print using user-defined printing routine.
	print_instr_user_defd(mi);
	return;
    }
    // Otherwise use dispatch table.  Of course, the user may have
    // overwritten a table entry to point to the user-defined printing
    // method.
    print_instr_f f = print_instr_table[stem];
    (this->*f)(mi);
}


/* Print out label in Ia64 format. */
void
PrinterIa64::print_instr_label(Instr *l)
{
    print_sym(get_label(l));
    fprintf(out, ":");
    print_notes(l);
}


/* Print out a cti instruction. 
 *	(Intel Manual Vol.3 2-9)
 * (qp) br.btype.bwh.ph.dh 	target25
 * (qp) br.btype.bwh.ph.dh 	b1 = target25
 *      br.btype.bwh.ph.dh 	target25
 *      br.ph.dh 		target25
 * (qp) br.btype.bwh.ph.dh 	b1
 * (qp) br.btype.bwh.ph.dh 	b1 = b2
 *      br.ph.dh 		b1
 */
void
PrinterIa64::print_instr_cti(Instr *bj)
{
    // print the qualifying predicate (if one exists)
    print_pred(bj);

    // print out opcode and any extensions
    print_opcode(bj);

    // print out destination if it exists
    if (dsts_size(bj)) {
	claim(is_breg(get_dst(bj)), 
	       "found cti dst opnd that is not a BREG");
	print_opnd(get_dst(bj));
	fprintf(out, " = ");		// prepare for target (at least)
    }

    // print out target address
    if (get_target(bj) != NULL) {
        print_sym(get_target(bj));
	fprintf(out, "#");
    }
    else {
        claim (is_breg(get_src(bj, 0)),
		"found cti src opnd that is not a BREG");
	print_opnd(get_src(bj, 0));
    }

    // FIXME: thwart bug in assembler's .auto scheduling by forcing a
    // stop after each CTI instruction.
    fprintf(out, " ;;");

    print_notes(bj);
}


/* Print out an ALU or memory instruction. Fortunately, instruction 
 * operand order is very regular. 
 * (pred) opcode.completerlist destlist = srclist */
void
PrinterIa64::print_instr_alm(Instr *mi)
{
    // print out qualifying predicate, opcode and any completers
    print_pred(mi);
    print_opcode(mi);

    bool need_comma = false;
    int i = 0;

    // First handle dst operands
    if (dsts_size(mi)) {
        need_comma = false;
        for (i = 0; i<dsts_size(mi); i++) {
            if (need_comma) fprintf(out, ", ");
            print_opnd(get_dst(mi, i));
            if (get_stem(get_opcode(mi)) == LD) break; //special case hidden op
            if (get_stem(get_opcode(mi)) == ST) break; //special case hidden op
            need_comma = true;
        }
        fprintf(out, "\t= ");
    } else if (get_stem(get_opcode(mi))==MOV_PR) {
        fprintf(out, "pr\t= "); //implied operand hack
    }

    // Now handle source operands
    need_comma = false;
    if (srcs_size(mi)) {
        int endVal = srcs_size(mi);
        if (has_qpred(mi)) endVal--; // Ignore the qp
        for (i=0; i<endVal; i++) {
            if (need_comma) fprintf(out, ", ");
            print_opnd(get_src(mi, i));
            need_comma = true;
        }
    } else if (get_stem(get_opcode(mi))==MOV_PR) {
        fprintf(out, "pr"); //implied operand hack
    }

    // FIXME: thwart bug in assembler's .auto scheduling by forcing a
    // stop after each ALM instruction.
    fprintf(out, " ;;");

    print_notes(mi);
}


/* Print out assembler directives in Ia64 format. */
void
PrinterIa64::print_instr_dot(Instr *x)
{
    int opcode = get_opcode(x);
    int stem = get_stem(opcode);

    // special case opcode_null so we don't output null lines
    if (stem == opcode_null) {
	print_notes(x);
	return;
    }
    else if (stem == opcode_label) {
        claim(srcs_size(x) == 1);
	fprintf(out, "%s:",  get_immed_string(get_src(x, 1)).chars());
	print_notes(x);
	return;
    }

    // print out opcode
    fprintf(out, "\t%s\t", ia64_opcode_stem_names[stem]);

    switch (stem) {

      case ia64::FILE:			// .file <file_no> "<file_name>"
	claim(srcs_size(x) == 2);
	fprintf(out, "%d \"", get_immed_int(get_src(x, 0)));
	fprintf(out, "%s\"",  get_immed_string(get_src(x, 1)).chars());
	break;

      case ia64::LN:			// .ln <line_no> 
	claim(srcs_size(x) == 2);
	fprintf(out, "%d", get_immed_int(get_src(x, 1)));
	break;

      // Print arguments without commas and strings without quotes
      //  ... but strings are followed by a pound sign 
      case ia64::GLOBAL:
	for (int i = 0; i < srcs_size(x); i++) {
	    Opnd opnd = get_src(x, i);
	    claim(is_immed(opnd));

	    if (is_immed_integer(opnd))
		fprintf(out, "%d", get_immed_int(opnd));
	    else
		fprintf(out, "%s#", get_immed_string(opnd).chars());

	    fprintf(out, " ");
	}
	break;

      // Print arguments with commas and strings without quotes
      // Pretty much everything follows this standard :)
      default:
	for (int i = 0; i < srcs_size(x); i++) {
	    if (i > 0)		// comma before each argument after first
		fprintf(out, ", ");

	    Opnd opnd = get_src(x, i);
	    if (is_reg(opnd))
		print_opnd(opnd);
	    else {
		claim(is_immed(opnd));

		if (is_immed_integer(opnd))
		    fprintf(out, "%d", get_immed_int(opnd));
		else
		    fprintf(out, "\"%s\"", get_immed_string(opnd).chars());
	    }
	}
	break;
    }
    print_notes(x);
}

/* Prints out the qualifying predicate on the instruction (if one exists) in
 * Ia64-specific syntax. */
void
PrinterIa64::print_pred(Instr *mi)
{
    if (is_predicated(mi)) {
        fprintf(out, "(" );
        print_opnd(get_qpred(mi));
        fprintf(out, ")" );
    }
    fprintf(out, "\t" );
}


/* Prints out the machine opcode and any extensions in 
 * Ia64-specific syntax. */
void
PrinterIa64::print_opcode(Instr *mi)
{
    int i;
    int opc = get_opcode(mi);
    int stem = get_stem(opc);
    claim(stem < LAST_IA64_OPCODE_STEM);
    fprintf(out, "%s", ia64_opcode_stem_names[stem]);
 
    if(has_exts(opc)) {
      if (stem==BR || stem==BRL || stem==BRP) {
        //Print four completers
        for (i=0; i<4; i++) {
	  int ext = get_br_ext(opc, (i+1));
          claim(ext < LAST_IA64_OPCODE_EXT);
          fprintf(out, "%s", ia64_opcode_ext_names[ext]);
        }
      }
      else {
        //Print three completers
        for (i=0; i<3; i++) {
	  int ext = get_ext(opc, (i+1));
          claim(ext < LAST_IA64_OPCODE_EXT);
          fprintf(out, "%s", ia64_opcode_ext_names[ext]);
        }
      }
    }
    fprintf(out, "\t");
}


/* Prints out an address expression in IA64 syntax */
void PrinterIa64::print_addr_exp(Opnd o)
{

    if (BaseDispOpnd bdo = o) {
        fprintf(out, "[");
        print_opnd(bdo.get_base());
        fprintf(out, "]");

        Opnd disp = bdo.get_disp(); 
        claim(disp == opnd_immed_0_u64, "Non-zero displacement!");
    }
    else if (SymDispOpnd sdo = o) {
        Opnd disp = sdo.get_disp(); 
        claim(disp == opnd_immed_0_u64, "Non-zero displacement!");
        print_opnd(sdo.get_addr_sym());
    }
    else claim(false, "unexpected address expression");
}

/* Prints out an address symbol operand in IA64 syntax.
 * Local symbols will later be translated, but global symbols
 * require the @ltoff tag.
 */
void PrinterIa64::print_addr_sym(Opnd o)
{
    Sym *s = get_sym(o);
    if (is_kind_of<VarSym>(s) && is_auto(static_cast<VarSym*>(s))) {
        fprintf(out, "@localsym(");
	print_sym(s);
        fprintf(out, "#)");
    }
    else {
        fprintf(out, "@ltoff(");
	print_sym(s);
        fprintf(out, "#)");
    }
}

/* Prints out an operand in Ia64 syntax. */
void
PrinterIa64::print_opnd(Opnd o)
{
    if (is_null(o)) {
	// print nothing
    } 
    else if (is_var(o)) {
	fprintf(out, "sym:");
	print_sym(get_var(o));
    } 
    else if (is_hard_reg(o)) {
	fprintf(out, "%s", reg_name(get_reg(o)));
    } 
    else if (is_virtual_reg(o)) {
	fprintf(out, "vr%d", get_reg(o));
    } 
    else if (is_immed_integer(o)) {
	fprint(out, get_immed_integer(o));
    } 
    else if (is_immed_string(o)) {
	fprintf(out, "%s", get_immed_string(o).chars());
    } 
    else if (is_addr_sym(o)) {
	print_addr_sym(o);
    } 
    else if (is_addr_exp(o)) {
	print_addr_exp(o);
    } 
    else {
	claim(false, "print_opnd() -- unknown kind %d", get_kind(o));
    }
}


void
PrinterIa64::print_extern_decl(VarSym *v)
{
	//do nothing
}

void
PrinterIa64::print_global_decl(FileBlock *fb)
{
	//do nothing
}


void
PrinterIa64::print_file_decl(int fnum, IdString fnam)
{
    // If file name is empty, owing to front end bug, comment out the
    // .file directive to avoid upsetting the assembler.
    if (fnam.chars()[0] == '\0')
	fprintf(out, " //");
    fprintf(out, "\t%s\t", ia64_opcode_stem_names[ia64::FILE]);
    //fprintf(out, "%d ", fnum);			// file number
    fprintf(out, "\"%s\"\n", fnam.chars());	// file name

    // FIXME: thwart bug in assembler's .auto scheduling by avoiding
    // .auto and emitting an explicit stop after each instruction.
//  fprintf(out, "\t%s\n", ia64_opcode_stem_names[AUTO]);	// .auto
}


/* Starts a procedure text segment. */
void
PrinterIa64::print_proc_begin(OptUnit *)
{
    fprintf(out, "\t%s\n", ia64_opcode_stem_names[TEXT]);	// .text
}


/* Reads the symbol table information and the annotations associated
 * with the OptUnit.  It creates the correct Ia64
 * instructions to start a procedure text segment. */
void
PrinterIa64::print_proc_entry(OptUnit *unit,
			       int file_no_for_1st_line)
{
    const char *cur_unit_name = get_name(unit).chars();

    fprintf(out, "\t%s\t%d\n", ia64_opcode_stem_names[ALIGN], 16);// .align 16 

    // If global procedure, then add this pseudo-op.  I.e. procedure is
    // in global symbol table, not in the file symbol table.
    Sym *psym = get_proc_sym(unit);
    if (is_global(psym) && !is_private(psym))
	fprintf(out, "\t%s\t%s#\n", ia64_opcode_stem_names[GLOBAL],// .global 
		     cur_unit_name);

    // add this pseudo-op to announce procedure
    fprintf(out, "\t%s\t%s#\n", ia64_opcode_stem_names[PROC],// .proc 
		     cur_unit_name);

    // The procedure symbol cannot be a label symbol since SUIF does not
    // allow label symbols outside of a procedural context (this is the
    // reason why this routine exists in the first place), and so, we
    // have to generate the procedure label in a funny way.
    fprintf(out, "%s:\n", cur_unit_name);			// procname: 

    // rest (if any) added to text during code gen
}


/* Outputs the .end pseudo-op. */
void
PrinterIa64::print_proc_end(OptUnit *unit)
{
    const char *cur_unit_name = get_name(get_proc_sym(unit)).chars();
    fprintf(out, "\t%s\t%s#\n", ia64_opcode_stem_names[ENDP],cur_unit_name);
}


///////////////////////////////////////////////////////////////
////////// Rest of this file is SUIF-specific /////////////////
///////////////////////////////////////////////////////////////

/* Helper function for print_var_def(). */
int
PrinterIa64::print_size_directive(TypeId t)
{

    int bits_filled = get_bit_size(t);
    int opcode;

    if (is_floating_point(t)) {
      switch (bits_filled) {
        case 32: 
	  opcode = REAL4;
	  break;
        case 64:
	  opcode = REAL8;
	  break;
        case 80:
	  opcode = REAL10;
	  break;
        case 128:
	  opcode = REAL16;
	  break;
        default:
	  claim(false, "unexpected size in value block");
      }
    }
    else {
      switch (bits_filled) {
        case 8:
	  opcode = DATA1;
	  break;
        case 16:
	  opcode = DATA2;
	  break;
        case 32: 
	  opcode = DATA4;
	  break;
        case 64:
	  opcode = DATA8;
	  break;
        default:
	  claim(false, "unexpected size in value block");
      }
    }

    fprintf(out, "\n\t%s\t", ia64_opcode_stem_names[opcode]);
    return bits_filled;

}


int
PrinterIa64::print_bit_filler(int bit_count)
{

    claim((bit_count % 8) == 0);
    int fill_count = bit_count / 8;
    for (int i=0; i<fill_count; i++) {
        fprintf(out, "\n\t%s\t0", ia64_opcode_stem_names[DATA1]);
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
PrinterIa64::process_value_block(ValueBlock *vblk)
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
	}
	else if (is_a<FloatConstant>(exp)) {
	    FloatConstant *fc = (FloatConstant*)exp;
	    bits_filled = print_size_directive(fc->get_result_type());
	    fputs(fc->get_value().c_str(), out);
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
	    fprintf(out, "#");
	    if (delta != 0) {
		fprintf(out, "%+ld", delta);
	    }
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

	// actual data directive
        for (int i=0; i<repeat_cnt; i++) {
	    bits_filled += process_value_block(rvb->get_sub_block());
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
PrinterIa64::print_var_def(VarSym *vsym)
{

    int vsym_size = get_bit_size(get_type(vsym)) >> 3;		// in bytes
    VariableDefinition *vdef = vsym->get_definition();

    // Determine alignment
    int bit_alignment = vdef->get_bit_alignment();
    int alignment_value;
    switch (bit_alignment) {
        case 8:			// byte aligned
	    alignment_value = 1;
	    break;
        case 16:			// halfword aligned
	    alignment_value = 2;
	    break;
        case 32:			// word aligned
	    alignment_value = 4;
	    break;
        case 64:			// double word aligned
	    alignment_value = 8;
	    break;
        case 128:			// quad word aligned
	    alignment_value = 16;
	    break;
        default:
	    warn("print_var_def() -- bad alignment for %s",
		 get_name(vsym).chars());
	    alignment_value = 8;
	    break;
    }

    if (vdef == NULL
	|| (vdef->get_initialization() == NULL)	// FIXME: shouldn't be possible
	||  is_a<UndefinedValueBlock>(vdef->get_initialization())) {

	// Uninitialized data item.  If global symbol, append to
	// .comm, else static symbol to .lcomm.
	if (is_global(vsym) && !is_private(vsym))
	    fprintf(out, "\t%s\t", ia64_opcode_stem_names[COMMON]);
	else
	    fprintf(out, "\t%s\t", ia64_opcode_stem_names[LCOMM]);
	print_sym(vsym);
	fprintf(out, "#,%d,%d\n", vsym_size, alignment_value);

    } else {		// initialized data item 

	// If global symbol, add .globl pseudo-op.  Note that a variable
	// is considered global in SUIF if it is part of the global or
	// file symbol table.  For Alpha assembly, a datum is global only
	// if it is part of the global symbol table (i.e. is_private()
	// checks for def in file symbol table).
	if (is_global(vsym) && !is_private(vsym)) {
	    fprintf(out, "\t%s\t", ia64_opcode_stem_names[GLOBAL]);
	    print_sym(vsym);
	    fprintf(out, "#\n"); 
	}

	// Specify data section to place data in. By default, if the
	// data is greater than 512 bytes in size, we use .data,
	// otherwise we use .sdata.
	if (vsym_size > Gnum)
	    fprintf(out, "\t%s\n", ia64_opcode_stem_names[DATA]);
	else
	    fprintf(out, "\t%s\n", ia64_opcode_stem_names[SDATA]);

	// .align
	fprintf(out, "\t%s\t%d\n", ia64_opcode_stem_names[ALIGN],
		     alignment_value);

        // .type sym#,@object
	fprintf(out, "\t%s\t", ia64_opcode_stem_names[TYPE]);
	print_sym(vsym);
	fprintf(out, "#,@object\n");

        //FIXME: .size sym#,<total_size> This doesn't always match gcc
	fprintf(out, "\t%s\t", ia64_opcode_stem_names[SIZE]);
	print_sym(vsym);
	fprintf(out, "#,%d\n", vsym_size);

	// Write out the label
	print_sym(vsym);
	fprintf(out, ":");		// return added in process_value_block

	// initialize memory with values
	process_value_block(vdef->get_initialization());
	fprintf(out, "\n");
   }
}
