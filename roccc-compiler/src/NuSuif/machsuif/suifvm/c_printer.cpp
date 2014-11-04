/* file "suifvm/c_printer.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "suifvm/cprinter.h"
#endif

#include <machine/machine.h>

#include <suifvm/opcodes.h>
#include <suifvm/c_printer.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace suifvm;

CPrinterSuifVm::CPrinterSuifVm()
{
    int i;
    print_instr_table = new print_instr_f[LAST_SUIFVM_OPCODE + 1];

    print_instr_table[opcode_null] = &CPrinter::print_instr_dot;
    print_instr_table[opcode_label] = &CPrinter::print_instr_label;

    for (i = NOP; i <= SLE; i++)
	print_instr_table[i] = &CPrinter::print_instr_alm;

    for (i = BTRUE; i <= RET; i++)
	print_instr_table[i] = &CPrinter::print_instr_cti;

    print_instr_table[ANY] = &CPrinter::print_instr_alm;
    print_instr_table[MRK] = &CPrinter::print_instr_dot;

    print_instr_table[AZR] = &CPrinter::print_instr_alm;

    print_instr_table[BEX] = &CPrinter::print_instr_alm;
    print_instr_table[BNS] = &CPrinter::print_instr_alm;
    print_instr_table[MUX] = &CPrinter::print_instr_alm;    
    print_instr_table[FOI] = &CPrinter::print_instr_alm;    
    print_instr_table[PST] = &CPrinter::print_instr_alm;    
    print_instr_table[PFW] = &CPrinter::print_instr_alm;    
    print_instr_table[WCF] = &CPrinter::print_instr_alm;    

    print_instr_table[MIN3] = &CPrinter::print_instr_alm;
    print_instr_table[MAX3] = &CPrinter::print_instr_alm;
    print_instr_table[LPR] = &CPrinter::print_instr_alm;
    print_instr_table[SNX] = &CPrinter::print_instr_alm;
    print_instr_table[LUT] = &CPrinter::print_instr_cti;
    print_instr_table[RLD] = &CPrinter::print_instr_alm;
    print_instr_table[RST] = &CPrinter::print_instr_alm;    
    print_instr_table[SMB1] = &CPrinter::print_instr_alm;    
    print_instr_table[SMB2] = &CPrinter::print_instr_alm;    
    print_instr_table[SFF1] = &CPrinter::print_instr_alm;    
    print_instr_table[SFF2] = &CPrinter::print_instr_alm;    

     
    for(i=CUSTOM_START+1; i<CUSTOM_END; i++)
        print_instr_table[i] = &CPrinter::print_instr_alm;
}

void
CPrinterSuifVm::print_instr(Instr *mi)
{
    int opc = get_opcode(mi);
    if (opc > LAST_SUIFVM_OPCODE) {
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

/* Local helpers */

static bool
Is_addr_binop(Instr *mi)
{
    return is_pointer(get_type(get_src(mi, 0)))
	&& is_pointer(get_type(get_src(mi, 1)));
}

/* Print out an ALU or memory instruction. */
void
CPrinterSuifVm::print_instr_alm(Instr *mi)
{
    int i;
    int opcode = get_opcode(mi);

    fprintf(out, "\t");	// indent everything

    // handle NOP as a special case
    if (opcode == NOP) {
	fprintf(out, "/* nop */");
	print_notes(mi);
	return;			// don't need a semicolon
    }

    // print out the destination
    if (opcode == STR) {
	putc('*', out);
	print_addr(get_dst(mi), get_type(get_src(mi, 0)), UNARY);
	fprintf(out, " = ");
    } else {
	if (dsts_size(mi)) {
	    claim(dsts_size(mi) == 1);
	    print_opnd(get_dst(mi));
	    fprintf(out, " = ");
	}
    }

    // print out the source(s)
    switch (opcode) {
      case CVT:
	fprintf(out, "(");
	print_type(get_type(get_dst(mi)));
	fprintf(out, ")");
	print_opnd(get_src(mi, 0));
	break;

      case LDA: { 
	  TypeId goal = get_referent_type(get_type(get_dst(mi)));
	  print_addr(get_src(mi, 0), goal, ASSIGN);
	  break;
        }

      case LDC: case MOV: case STR:
	print_opnd(get_src(mi, 0));
	break;

    case ADD: case SUB:
	if (is_pointer(get_type(get_dst(mi)))) {
	    Opnd base = get_src(mi, 0);
	    Opnd disp = get_src(mi, 1);
	    if ((opcode == ADD) && is_pointer(get_type(disp))) {
		base = get_src(mi, 1);
		disp = get_src(mi, 0);
	    }
	    TypeId goal = get_referent_type(get_type(get_dst(mi)));
	    print_addr_disp(base, disp, goal, ASSIGN, suifvm_cnames[opcode]);
	} else if (opcode == SUB && Is_addr_binop(mi)) {
	    print_addr_binop(mi);
	} else {
	    print_opnd(get_src(mi, 0));
	    print_opcode(mi);
	    print_opnd(get_src(mi, 1));
	}
	break;

      case SEQ: case SNE: case SL: case SLE:
	if (Is_addr_binop(mi)) {
	    print_addr_binop(mi);
	    break;
	}				// fall through to simple binary case
      case MUL: case DIV: case REM:

      //case AND: case IOR: case XOR:
      case BAND: case LAND: 
      case LOR: case BIOR: case BXOR: 


      case ASR: case LSL:
	print_opnd(get_src(mi, 0));
	print_opcode(mi);
	print_opnd(get_src(mi, 1));
	break;

     case NEG: case NOT: 
	print_opcode(mi);
	print_opnd(get_src(mi, 0));
	break;

      case ABS:
	print_opcode(mi);
	fprintf(out, "(");
	print_opnd(get_src(mi, 0));
	fprintf(out, ")");
	break;

      case MIN: case MAX:
      case LSR:
	print_opcode(mi);
	fprintf(out, "(");
	print_opnd(get_src(mi, 0));
	fprintf(out, ", ");
	print_opnd(get_src(mi, 1));
	fprintf(out, ")");
	break;

      case LOD:
	putc('*', out);
	print_addr(get_src(mi, 0), get_type(get_dst(mi)), ASSIGN);
	break;

      case ANY:
	{
	    OneNote<IdString> opcode_name_note = get_note(mi, k_instr_opcode);
	    claim(!is_null(opcode_name_note));

	    fprintf(out, "%s(", opcode_name_note.get_value().chars());
	    for (i = 0; i < srcs_size(mi); i++) {
		if (i) fprintf(out, ", ");
		print_opnd(get_src(mi, i));
	    }
	    fprintf(out, ")");
	}
	break;

      case MOD: case ROT: case MEMCPY:
	claim(false, "unimplemented opcode %s", suifvm_opcode_names[opcode]);

      default:
	claim(false, "unexpected opcode %s", suifvm_opcode_names[opcode]);
    }

    fprintf(out, ";");
    print_notes(mi);
}


/* Print CAL as a call statement. */
void
CPrinterSuifVm::print_instr_cal(Instr *mi)
{
    TypeId dtype = type_v0;

    // deal with result, if any
    if (dsts_size(mi)) {
	claim(dsts_size(mi) == 1);
	dtype = get_type(get_dst(mi));
	print_opnd(get_dst(mi));
	fprintf(out, " = ");
    }

    // output callee_address operand
    if (get_target(mi)) {
	// direct call
	claim(srcs_size(mi) == 0 || is_null(get_src(mi, 0)));
	print_sym(get_target(mi));
    } else {
	// indirect call
	fprintf(out, "(*((");
	print_type(dtype);
	fprintf(out, " (*)())");
	print_opnd(get_src(mi, 0));
	fprintf(out, "))");
    }

    // build parameter list
    fprintf(out, "(");
    for (int i = 1; i < srcs_size(mi); i++) {
	if (i > 1)
	    fprintf(out, ", ");
	print_opnd(get_src(mi, i));
    }
    fprintf(out, ");");
}

/* Print MBR as a switch statement. */
void
CPrinterSuifVm::print_instr_mbr(Instr *mi)
{
    fprintf(out, "switch (");
    print_opnd(get_src(mi, 0));
    fprintf(out, ") {\n\t");

    // process cases
    MbrNote note = get_note(mi, k_instr_mbr_tgts);
    claim(!is_null(note));
    int num_cases = note.get_case_count();
    claim(num_cases > 0);

    for (int i = 0; i < num_cases; ++i) {
	fprintf(out, "case %d:\n\tgoto ", note.get_case_constant(i));
	print_sym(note.get_case_label(i));
	fprintf(out, ";\n\t");
    }
    fprintf(out, "default:\n\tgoto ");
    print_sym(note.get_default_label());
    fprintf(out, ";\n\t");

    fprintf(out, "}");
}

/* Print out a cti instruction. */
void
CPrinterSuifVm::print_instr_cti(Instr *mi)
{
    int opcode = get_opcode(mi);

    fprintf(out, "\t");	// indent everything

    // handle call as special case since it has a destination
    if (opcode == CAL || opcode == LUT)
	print_instr_cal(mi);

    else {
	claim(dsts_size(mi) == 0);

	// print out the source(s)
	switch (opcode) {
	  case BTRUE:
	    fprintf(out, "if (");
	    print_opnd(get_src(mi, 0));
	    fprintf(out, ") goto ");
	    print_sym(get_target(mi));
	    fprintf(out, ";");
	    break;

	  case BFALSE:
	    fprintf(out, "if (!");
	    print_opnd(get_src(mi, 0));
	    fprintf(out, ") goto ");
	    print_sym(get_target(mi));
	    fprintf(out, ";");
	    break;

	  case BEQ: case BNE: case BGE:
	  case BGT: case BLE: case BLT:
	    fprintf(out, "if (");
	    if (Is_addr_binop(mi)) {
		print_addr_binop(mi);
	    } else {
		print_opnd(get_src(mi, 0));
		print_opcode(mi);
		print_opnd(get_src(mi, 1));
	    }
	    fprintf(out, ") goto ");
	    print_sym(get_target(mi));
	    fprintf(out, ";");
	    break;

	  case JMP:
	    fprintf(out, "goto ");
	    print_sym(get_target(mi));
	    fprintf(out, ";");
	    break;

	  case MBR:
	    print_instr_mbr(mi);
	    break;

	  case RET:
	    if (srcs_size(mi)) {
		claim(srcs_size(mi) == 1);
		fprintf(out, "return ");
		print_opnd(get_src(mi, 0));
		fprintf(out, ";");
	    } else {
		fprintf(out, "return;");
	    }
	    break;

	  case JMPI:
	    claim(false, "unimplemented opcode %s",
		  suifvm_opcode_names[opcode]);

	  default:
	    claim(false, "unexpected opcode %s", suifvm_opcode_names[opcode]);
	}
    }
    print_notes(mi);
}


/* Print out pseudo-op. */
void
CPrinterSuifVm::print_instr_dot(Instr *mi)
{
    int opcode = get_opcode(mi);

    // special case opcode_null so we don't output null lines
    if (opcode == opcode_null) {
	print_notes(mi);
	return;
    }

    // print out MRK information
    claim((opcode == MRK) && (dsts_size(mi) == 0) && (srcs_size(mi) == 0));

    Annote *an_line;
    if (omit_unwanted_notes && (an_line = mi->peek_annote(k_line))) {
	fputs("\t\t\t", out);
	print_annote(an_line);
    } else {
	fprintf(out, "\t/* mrk */\t");
	print_notes(mi);
    }
}


/* Prints out the opcode as a C-language operator. */
void
CPrinterSuifVm::print_opcode(Instr *mi)
{
    fprintf(out, " %s ", suifvm_cnames[get_opcode(mi)]);
}


/*
 * print_addr_binop -- Print a binary expression both of whose operands
 * are addresses, i.e, pointer difference or pointer comparison.
 */
void
CPrinterSuifVm::print_addr_binop(Instr *mi)
{
    TypeId referent = get_referent_type(get_type(get_src(mi, 0)));
    if (referent  ==  get_referent_type(get_type(get_src(mi, 1)))
    &&  get_bit_size(referent) == 8) {
	print_opnd(get_src(mi, 0));
	print_opcode(mi);
	print_opnd(get_src(mi, 1));
    } else {
	print_addr(get_src(mi, 0), type_s8, BINARY);
	print_opcode(mi);
	print_addr(get_src(mi, 1), type_s8, UNARY);
    }
}

void
CPrinterSuifVm::print_global_decl(FileBlock *)
{
    fprintf(out, "\n#include <suifvm/c_printer_defs.h>\n\n");
}
