/* file "suifvm/printer.cpp" */

/*
   Copyright (c) 2000 The President and Fellows of Harvard College

   All rights reserved.

   This software is provided under the terms described in
   the "machine/copyright.h" include file.
   */

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "suifvm/printer.h"
#endif

#include <machine/machine.h>

#include <suifvm/opcodes.h>
#include <suifvm/printer.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace suifvm;

PrinterSuifVm::PrinterSuifVm()
{
  int i;
  print_instr_table.resize(LAST_SUIFVM_OPCODE + 1);

  print_instr_table[opcode_null] = &Printer::print_instr_dot;
  print_instr_table[opcode_label] = &Printer::print_instr_label;

  for (i = NOP; i <= SLE; i++)
    print_instr_table[i] = &Printer::print_instr_alm;

  for (i = BTRUE; i <= RET; i++)
    print_instr_table[i] = &Printer::print_instr_cti;

  print_instr_table[ANY] = &Printer::print_instr_alm;
  print_instr_table[MRK] = &Printer::print_instr_dot;

  print_instr_table[AZR] = &Printer::print_instr_alm;

  print_instr_table[BEX] = &Printer::print_instr_alm;
  print_instr_table[BNS] = &Printer::print_instr_alm;
  print_instr_table[MUX] = &Printer::print_instr_alm;
  print_instr_table[FOI] = &Printer::print_instr_alm;
  print_instr_table[PST] = &Printer::print_instr_alm;
  print_instr_table[PFW] = &Printer::print_instr_alm;
  print_instr_table[WCF] = &Printer::print_instr_alm;
  print_instr_table[MIN3] = &Printer::print_instr_alm;
  print_instr_table[MAX3] = &Printer::print_instr_alm;
  print_instr_table[LPR] = &Printer::print_instr_alm;
  print_instr_table[SNX] = &Printer::print_instr_alm;
  print_instr_table[BCMB] = &Printer::print_instr_alm;     
  print_instr_table[BLUT] = &Printer::print_instr_alm;     
  print_instr_table[BSEL] = &Printer::print_instr_alm;     
  print_instr_table[LUT] = &Printer::print_instr_cti;
  print_instr_table[RLD] = &Printer::print_instr_alm;
  print_instr_table[RST] = &Printer::print_instr_alm;    
  print_instr_table[SMB1] = &Printer::print_instr_alm;    
  print_instr_table[SMB2] = &Printer::print_instr_alm;    
  print_instr_table[SFF1] = &Printer::print_instr_alm;    
  print_instr_table[SFF2] = &Printer::print_instr_alm;    
 
  for(i=CUSTOM_START+1; i<CUSTOM_END; i++)
      print_instr_table[i] = &Printer::print_instr_alm;

}


  void
PrinterSuifVm::print_instr(Instr *mi)
{

    // JOHN
    //fprintf(stderr, "PROCESS MI: %s\n", suifvm_opcode_names[mi->get_opcode()]);

    
  int opc = mi->get_opcode();
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


/* Print out a label. */
  void
PrinterSuifVm::print_instr_label(Instr *l)
{
  print_sym(get_label(l));
  fprintf(out, ":");
  print_notes(l);
}


/* Print out a cti instruction.  Print "rd <- rs1, rs2, target" in that
 * order, or "rd <- rs" if indirect jump. */
  void
PrinterSuifVm::print_instr_cti(Instr *bj)
{
  print_opcode(bj);

  // print out destination if it exists
  if (dsts_size(bj)) {
    print_opnd(get_dst(bj, 0));
    fprintf(out, " <- ");		// prepare for target (at least)
  }

  if (get_opcode(bj) == CAL || get_opcode(bj) == LUT) {
    // print direct or indirect target first
    Sym *target = get_target(bj);
    claim((target == NULL) || (srcs_size(bj) > 0 && is_null(get_src(bj, 0))),
        "Ambiguous called procedure");

    if (target != NULL)
      print_sym(get_target(bj));
    else {
      fprintf(out, "(*");
      print_opnd(get_src(bj, 0));
      fprintf(out, ")");
    }

    // now print parameter sources, if any
    fprintf(out, "(");
    for (int i = 1; i < srcs_size(bj); i++) {
      if (i > 1) fprintf(out, ",");
      print_opnd(get_src(bj, i));
    }
    fprintf(out, ")");

  } else if (get_opcode(bj) == RET) {
    // no label, but source (if any) is not indirect target
    if (srcs_size(bj)) {
      //	    claim(srcs_size(bj) == 1);
      //	    print_opnd(get_src(bj, 0));
      for (int i = 1; i < srcs_size(bj); i++) {
        if (i > 1) fprintf(out, ",");
        print_opnd(get_src(bj, i));
      }
    }

  } else if (get_opcode(bj) == JMPI) {
    claim(srcs_size(bj) == 1);
    // source operand for indirect target surrounded by ()'s
    fprintf(out, "(");
    print_opnd(get_src(bj, 0));
    fprintf(out, ")");

  }  else {
    // first print out sources, if any
    for (int i = 0; i < srcs_size(bj); i++) {
      print_opnd(get_src(bj, i));
      fprintf(out, ",");		// prepare for target
    }

    // print out target address
    print_sym(get_target(bj));
  }

  print_notes(bj);
}


/* Print out an ALU or memory instruction. */
  void
PrinterSuifVm::print_instr_alm(Instr *mi)
{
    print_predicate(mi);
  print_opcode(mi);

  // print out "dest <- src1, src2" in that order
  if (dsts_size(mi) > 0) {
    if(dsts_size(mi) == 1) {
      print_opnd(get_dst(mi, 0));
    }
    //	fprintf(out, " <- ");}
  else {
    bool need_comma = false;
    for(int i = 0; i < dsts_size(mi); ++i) {
      if(need_comma) fprintf(out, ",");
      else need_comma = true;
      print_opnd(get_dst(mi, i));
    }
  }

  fprintf(out, " <- ");

}
bool need_comma = false;
for (int i = 0; i < srcs_size(mi); i++) {
  Opnd s = get_src(mi, i);
  if (!is_null(s)) {
    if (need_comma) fprintf(out, ",");
    else need_comma = true;		// next time
    print_opnd(s);
  }
}

print_notes(mi);
}


/* Print out pseudo-op. */
  void
PrinterSuifVm::print_instr_dot(Instr *x)
{
  int i;

  // special case opcode_null so we don't output null lines
  if (get_opcode(x) == opcode_null) {
    print_notes(x);
    return;
  }

  // print out opcode
  fprintf(out, "\t%s\t", suifvm_opcode_names[get_opcode(x)]);

  for (i = 0; i < srcs_size(x); i++) {
    if (i)		// print comma before each src except the first
      fprintf(out, ", ");
    print_opnd(get_src(x, i));
  }

  print_notes(x);
}


/* Prints out the machine opcode. */
  void
PrinterSuifVm::print_opcode(Instr *mi)
{
  fprintf(out, "\t%s\t", suifvm_opcode_names[get_opcode(mi)]);
}


/* Helper routine for print_address_exp. */
  void
PrinterSuifVm::print_sym_disp(Opnd addr_sym, Opnd disp)
{
  // print first operand
  print_sym(get_sym(addr_sym));

  // deal with the displacement
  int d = get_immed_int(disp);

  if (d == 0)
    return;
  if (d > 0)
    fprintf(out, "+%d", d);
  else
    fprintf(out, "%d", d);
}

/* Prints out an effective-address operand.  This routine expects the
 * EA operand as an expression. */
  void
PrinterSuifVm::print_address_exp(Opnd o)
{
  if (BaseDispOpnd bdo = o) {
    // print as 'offset(base)'
    print_opnd(bdo.get_disp());
    fprintf(out, "(");
    print_opnd(bdo.get_base());
    fprintf(out, ")");

  } else if (SymDispOpnd sdo = o) {
    // print as 'symbol+offset' -- offset should be unsigned
    print_sym_disp(sdo.get_addr_sym(), sdo.get_disp());

  } else if (IndexSymDispOpnd isdo = o) {
    // print as 'sym+disp($idx_reg)'
    print_sym_disp(isdo.get_addr_sym(), isdo.get_disp());
    fprintf(out, "(");
    print_opnd(isdo.get_index());
    fprintf(out, ")");

  } else {
    claim(false, "unexpected address expression");
  }
}


/* Prints out an operand. */
  void
PrinterSuifVm::print_opnd(Opnd o)
{
  if (is_null(o)) {
    fprintf(out, "(nullop)");

  } else if (is_var(o)) {
    print_sym(get_var(o));
    fprintf(out, ".");
    fprint(out, get_type(o));

  } else if (is_hard_reg(o)) {
    fprintf(out, "$hr%d.", get_reg(o));
    fprint(out, get_type(o));

  } else if (is_virtual_reg(o)) {
    fprintf(out, "$vr%d.", get_reg(o));
    fprint(out, get_type(o));

  } else if (is_immed_integer(o)) {
    Integer i = get_immed_integer(o);
    fprintf(out, "#");
    if (i.is_c_string_int())
      fputs(i.chars(), out);
    else
      fprintf(out, "%ld", i.c_long());

    fprintf(out, "(");
    fprint(out, get_type(o));
    fprintf(out, ")");

  } else if (is_immed_string(o)) {
    fprintf(out, "\"%s\"", get_immed_string(o).chars());

  } else if (is_addr_sym(o)) {
    print_sym(get_sym(o));
    fprintf(out, ".addr_sym.");
    fprint(out, get_type(o));

  } else if (is_addr_exp(o)) {
    print_address_exp(o);
    fprintf(out, ".addr_exp.");
    fprint(out, get_type(o));

  } else {
    claim(false, "print_opnd() -- unknown kind %d", get_kind(o));
  }
 
  NoteKey k_bitwidth=IdString("BITWIDTH");
  if( has_note(o, k_bitwidth) ) {
    OneNote<IdString> the_note(get_note(o,k_bitwidth));
    IdString the_value = the_note.get_value();
    fprintf(out, "{%s}", the_value.c_str());
  }

}


/*
 * Print an .extern directive, including size in bytes if available.
 * (The type of an external array may give no size.)
 */
  void
PrinterSuifVm::print_extern_decl(VarSym *v)
{
  TypeId t = get_type(v);
  fprintf(out, "\t.extern\t");
  print_sym(v);
  int bit_size = get_bit_size(t);	// -1 if unavailable
  if (bit_size >= 0)
    fprintf(out, " %d", bit_size >> 3);
  putc('\n', out);
}


  void
PrinterSuifVm::print_file_decl(int fnum, IdString fnam)
{
  fprintf(out, "\t.file\t");
  fprintf(out, "%d \"", fnum);		// file no.
  fprintf(out, "%s\"\n", fnam.chars());	// file name
}


  void
PrinterSuifVm::print_global_decl(FileBlock *)
{
  // empty
}


/* Starts a procedure text segment. */
  void
PrinterSuifVm::print_proc_begin(ProcDef *)
{
  fprintf(out, "\t.text_start\n");
}


/* Creates some suifvm-like instructions to start a procedure text
 * segment. */
  void
PrinterSuifVm::print_proc_entry(ProcDef *pd,
    int file_no_for_1st_line)
{
  const char* cur_pname = get_name(pd).chars();

  // If global procedure, then add this pseudo-op.  I.e. procedure is
  // in global symbol table, not in the file symbol table.
  Sym* psym = get_proc_sym(pd);
  if (is_global(psym) && !is_private(psym))
    fprintf(out, "\t.globl\t%s\n", cur_pname);

  // Get file and line no. info for this procedure -- .file note
  // already generated (if necessary) in process_file_op().
  LineNote note = get_note(pd, k_line);
  int first_line_no = note.get_line();
  fprintf(out, "\t.loc\t%d %d\n", file_no_for_1st_line, first_line_no);

  fprintf(out, "\t.ent\t%s\n", cur_pname);

  // The procedure symbol cannot be a label symbol since SUIF does not
  // allow label symbols outside of a procedural context (this is the
  // reason why this routine exists in the first place), and so, we
  // have to generate the procedure label in a funny way.
  fprintf(out, "%s:\n", cur_pname);				/* proc: */
}


/* Ends a procedure text segment. */
  void
PrinterSuifVm::print_proc_end(ProcDef *pd)
{
  fprintf(out, "\t.text_end\t%s\n", (get_name(pd)).chars());
}


/* Helper function for print_var_def(). */
  char *
PrinterSuifVm::size_directive(TypeId t)
{
  switch (get_bit_size(t)) {
    case 8:
      return ".byte";
    case 16:
      return ".2byte";
    case 32:
      if (is_kind_of<FloatingPointType>(t))
        return ".fp_single";
      else
        return ".4byte";
    case 64:
      if (is_kind_of<FloatingPointType>(t))
        return ".fp_double";
      else
        return ".8byte";
    default:
      claim(false, "unexpected size in value block");
  }
  return "ERROR";
}

/* Another helper function for print_var_def().   This function
 * expects that it will need to add a return before printing
 * anything, and it expects that you will add a return after
 * it is done. */
  void
PrinterSuifVm::process_value_block(ValueBlock *vblk)
{
  if (is_kind_of<MultiValueBlock>(vblk)) {
    MultiValueBlock *mvb = (MultiValueBlock *)vblk;
    for (int i = 0; i < subblocks_size(mvb); i++ )
      process_value_block(get_subblock(mvb, i));

  } else if (is_kind_of<RepeatValueBlock>(vblk)) {
    RepeatValueBlock *rvb = (RepeatValueBlock *)vblk;
    int repeat_cnt = get_repetition_count(rvb);

    if (repeat_cnt > 1) {	// insert .repeat pseudo-op
      fprintf(out, "\n\t.repeat\t%d", repeat_cnt);
      cur_directive = NULL;
    }

    // actual data directive
    process_value_block(get_subblock(rvb));

    if (repeat_cnt > 1) {	// insert .endr pseudo-op
      fprintf(out, "\n\t.end_repeat");
      cur_directive = NULL;
    }

  } else if (is_kind_of<ExpressionValueBlock>(vblk)) {
    // This kind of value block wraps integer constants,
    // string constants, and symbols.
    ExpressionValueBlock *xvb = (ExpressionValueBlock *)vblk;
    char *directive = size_directive(get_type(xvb));
    if ((directive != cur_directive) || (cur_opnd_cnt > 20)) {
      fprintf(out, "\n\t%s\t", directive);
      cur_directive = directive;
      cur_opnd_cnt = 0;
    } else
      fprintf(out, ", ");
    Opnd opnd = get_value(xvb); 
    print_opnd(opnd);
    cur_opnd_cnt++;

    if (is_var(opnd)) {
      // always force the start of a new data directive
      cur_directive = NULL;
      cur_opnd_cnt = 0;
    }

  } else if (is_kind_of<UndefinedValueBlock>(vblk)) {
    char *directive = size_directive(get_type(vblk));
    if ((directive != cur_directive) || (cur_opnd_cnt > 20)) {
      fprintf(out, "\n\t%s\t", directive);
      cur_directive = directive;
      cur_opnd_cnt = 0;
    } else
      fprintf(out, ", ");
    putc('0', out);
    cur_opnd_cnt++;

  } else {
    claim(false, "unexpected kind of ValueBlock");
  }
}

/* Generates some reasonable-looking assembly language data
 * statements. */
  void
PrinterSuifVm::print_var_def(VarSym *vsym)
{
  int vsym_size = get_bit_size(get_type(vsym)) >> 3;
  VarDef* vdef = get_def(vsym);

  if ((vdef == NULL)
      || (get_init(vdef) == NULL) // FIXME: shouldn't be happening
      || is_kind_of<UndefinedValueBlock>(get_init(vdef))) {
    // Uninitialized data item.  Indicate scope.
    if (is_global(vsym) && !is_private(vsym))
      fprintf(out, "\t.uninit_globl\t");
    else
      fprintf(out, "\t.uninit_local\t");
    print_sym(vsym);
    fprintf(out, ", %d\n", vsym_size);

  } else {		// initialized data item
    fprintf(out, "\t.data\n");

    // If global symbol, add .globl pseudo-op.  Note that a
    // variable is considered global if it is part of the global
    // or file symbol table.  For suifvm assembly, a datum is
    // global only if it is part of the global symbol table
    // (i.e. is_private() checks for def in file symbol table).
    if (is_global(vsym) && !is_private(vsym)) {
      fprintf(out, "\t.globl\t");
      print_sym(vsym);
      fprintf(out, "\n");
    }

    // Determine alignment and align
    char *alignment_value;
    switch (get_bit_alignment(vdef)) {
      case 8:			// byte aligned
        alignment_value = "byte";
        break;
      case 16:			// halfword aligned
        alignment_value = "2byte";
        break;
      case 32:			// word aligned
        alignment_value = "4byte";
        break;
      case 64:			// double word aligned
        alignment_value = "8byte";
        break;
      case 128:			// quad word aligned
        alignment_value = "16byte";
        break;
      default:
        warn("print_var_def() -- bad alignment for %s",
            get_name(vsym).chars());
        alignment_value = "16byte";
    }
    fprintf(out, "\t.align\t%s\n", alignment_value);

    // Write out the label
    print_sym(vsym);
    fprintf(out, ":");	// return added in process_value_block

    // initialize memory with values
    cur_directive = NULL;
    cur_opnd_cnt = 0;
    process_value_block(get_init(vdef));
    fprintf(out, "\n");
  }
}

#define _DO_PRINT_PREDICATE_ false
void
PrinterSuifVm::print_predicate(Instr *instr)
{
    if(_DO_PRINT_PREDICATE_) {
        fprintf(out, "[pred: ");
        if( has_predicate(instr) ) {
            print_opnd(get_predicate(instr));
        }  else {
            fprintf(out, "1");
        }
        fprintf(out, "]  ");
    }
}
