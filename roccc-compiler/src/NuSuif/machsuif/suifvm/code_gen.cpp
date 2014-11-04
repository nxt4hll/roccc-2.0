/* file "suifvm/code_gen.cc" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "suifvm/code_gen.h"
#endif

#include <machine/machine.h>

#include <suifvm/contexts.h>
#include <suifvm/opcodes.h>
#include <suifvm/code_gen.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace suifvm;

/* --------------- routines for init() --------------- */

/* Report_exception() -- check procedure for enable_exceptions
 * annotation.  Default action is to disable all arithmetic
 * exceptions.  Currently, our code generators only expect to see the
 * enabling of integer arithmetic exceptions.  Code for IEEE exception
 * handling must be created when someone actually cares. */
static bool
Report_exception(OptUnit *unit, char *exc)
{
    if (ListNote<IdString> note = get_note(get_body(unit), k_enable_exceptions))
    {
	int count = note.values_size();

	for (int i = 0; i < count; ++i)
	    if (note.get_value(i).chars() == exc)
		return true;
    }
    return false;
}

/* CodeGen::init() -- This routine initializes some of the
 * variables needed during translation of code for a particular
 * procedure.  The architecture-specific derived classes will add to
 * this initialization.  Some of the information comes from
 * annotations on the procedure's body.  It also does some sanity
 * checks to verify that we can translate the contents of this
 * procedure cleanly.
 *
 * No code is produced; pseudo ops for the beginning of a procedure text
 * segment are produced only when we generate assembly/binary code. */
void
CodeGen::init(OptUnit *unit)
{
    // Store away the procedure definition so that we can check
    // the validity of the other calls.
    cur_unit = unit;

    // Do the work to get the procedure name.
    cur_unit_name = get_name(get_proc_sym(unit)).chars();
    debug(3, "Processing procedure %s", cur_unit_name);

    // To ask any interesting question about a procedure (e.g.,
    // has_varargs), we need a CProcedureType.  We downcast to obtain it,
    // and this will fail when someone extends the system with other
    // concrete procedure types.

    cur_unit_type =
	to<CProcedureType>(get_proc_sym(unit)->get_type()); // may fail

    // Set up the exception flags used during translation.
    report_int_overflow = Report_exception(unit, "int_overflow");
    report_int_divideby0 = Report_exception(unit, "int_divide-by-0");

    // Transfer stack_frame_info properties to variables for use/update
    // during code generation.
    //
    StackFrameInfoNote sfi_note = take_note(unit, k_stack_frame_info);
    claim(!is_null(sfi_note));

    is_leaf = sfi_note.get_is_leaf();
    is_varargs = sfi_note.get_is_varargs();
    max_arg_area = sfi_note.get_max_arg_area();

    set_note(unit, k_stack_frame_info, sfi_note);

    // Some sanity checks:
    // ... check type of items in body
    claim(is_kind_of<InstrList>(get_body(unit)),
	  "unit %s contains a non-InstrList body", cur_unit_name);
    // ... check result type
    claim(!is_kind_of<GroupType>(cur_unit_type->get_result_type()),
	  "found an un-fixed proc that returns a struct");

    // At high debugging verbosity, prepare to print instructions emitted.
    if_debug(6)
	target_printer()->set_file_ptr(stderr);
}


/* --------------- routines for finish() --------------- */
void
CodeGen::finish(OptUnit *unit)
{
    // sanity check
    claim(unit == cur_unit, "CodeGen::finish: argument unit != cur_unit");

    line_note = Note();

    StackFrameInfoNote sfi_note = take_note(unit, k_stack_frame_info);
    claim(!is_null(sfi_note));

    sfi_note.set_is_leaf(is_leaf);
    sfi_note.set_is_varargs(is_varargs);
    sfi_note.set_max_arg_area(max_arg_area);

    set_note(unit, k_stack_frame_info, sfi_note);
}

void
CodeGen::emit(Instr *mi)
{
    claim(!has_note(mi, k_line));

    if (!is_null(line_note))
	set_note(mi, k_line, clone(line_note));
    append(receiver, mi);

    if_debug(6)
	target_printer()->print_instr(mi);
}


/* --------------- translate_*() routines --------------- */

/*
 * NOTE: We have defined each translate function so that it reads the input
 * mi(s) and appends the new mi(s) to the current receiver list.  It is up
 * to the actual translate function invoked whether the input mi is deleted
 * or modified.
 */
void
CodeGen::translate_mi(Instr *mi)
{
    int opcode = get_opcode(mi);

    claim((opcode >= 0) && (opcode <= LAST_SUIFVM_OPCODE));

    if (has_note(mi, k_line))
	line_note = take_note(mi, k_line);

    switch (opcode) {
      case opcode_null:
	translate_null(mi);
	break;
      case opcode_label:
	emit(mi);			// move label without translation
	break;
      case NOP:
	translate_nop(mi);
	break;
      case CVT:
	translate_cvt(mi);
	break;
      case LDA:
	translate_lda(mi);
	break;
      case LDC:
	translate_ldc(mi);
	break;
      case ADD:
	translate_add(mi);
	break;
      case SUB:
	translate_sub(mi);
	break;
      case NEG:
	translate_neg(mi);
	break;
      case MUL:
	translate_mul(mi);
	break;
      case DIV:
	translate_div(mi);
	break;
      case REM:
	translate_rem(mi);
	break;
      case MOD:
	translate_mod(mi);
	break;
      case ABS:
	translate_abs(mi);
	break;
      case MIN:
	translate_min(mi);
	break;
      case MAX:
	translate_max(mi);
	break;
      case NOT:
	translate_not(mi);
	break;
      //case AND:
      case LAND:
      case BAND:
	translate_and(mi);
	break;
      //case IOR:
      case LOR:
      case BIOR:
	translate_ior(mi);
	break;
      //case XOR:
      case BXOR:
	translate_xor(mi);
	break;
      case ASR:
	translate_asr(mi);
	break;
      case LSL:
	translate_lsl(mi);
	break;
      case LSR:
	translate_lsr(mi);
	break;
      case ROT:
	translate_rot(mi);
	break;
      case MOV:
	translate_mov(mi);
	break;
      case LOD:
	translate_lod(mi);
	break;
      case STR:
	translate_str(mi);
	break;
      case MEMCPY:
	translate_memcpy(mi);
	break;
      case SEQ:
	translate_seq(mi);
	break;
      case SNE:
	translate_sne(mi);
	break;
      case SL:
	translate_sl(mi);
	break;
      case SLE:
	translate_sle(mi);
	break;
      case BTRUE:
	translate_btrue(mi);
	break;
      case BFALSE:
	translate_bfalse(mi);
	break;
      case BEQ:
	translate_beq(mi);
	break;
      case BNE:
	translate_bne(mi);
	break;
      case BGE:
	translate_bge(mi);
	break;
      case BGT:
	translate_bgt(mi);
	break;
      case BLE:
	translate_ble(mi);
	break;
      case BLT:
	translate_blt(mi);
	break;
      case JMP:
	translate_jmp(mi);
	break;
      case JMPI:
	translate_jmpi(mi);
	break;
      case MBR:
	translate_mbr(mi);
	break;
      case CAL:
	translate_cal(mi);
	break;
      case RET:
	translate_ret(mi);
	break;
      case ANY:
	translate_any(mi);
	break;
      case MRK:
	translate_mrk(mi);
	break;
      default:
	claim(false, "Unrecognized SUIFvm opcode");
    }
}

/* translate_list() -- This routine assumes that you want to code generate
 * in place.  The routine pops instructions off the top of the list, translates
 * the instruction, and appends the resulting translation (one or more
 * instructions on to the end of the list.  We note the length of the list
 * before translation begins so that we know when to stop. 
 *
 * This routine is independent of the target architecture and thus does
 * not appear in any derived class.
 */
void
CodeGen::translate_list(OptUnit *unit, InstrList *in_place)
{
    // sanity check
    claim(unit == cur_unit, "mismatch between unit in this call and cur_unit");

    receiver = in_place;

    int input_size = size(in_place);

    // translate each input instruction
    for (int i = 0; i < input_size; ++i)
	translate_mi(remove(in_place, start(in_place)));
}

/* translate_instr() -- This routine translates a single machine instruction
 * into its equivalent architecture-specific instructions.  These translations
 * are returned at the end of the InstrList parameter.
 *
 * This routine is independent of the target architecture and thus does
 * not appear in any derived class.
 */
void
CodeGen::translate_instr(OptUnit *unit, Instr *mi, InstrList *output)
{
    // sanity check
    claim(unit == cur_unit, "mismatch between unit in this call and cur_unit");

    receiver = output;

   // translate the input instruction
    translate_mi(mi);
}


/* --------------------------   target_code_gen  ---------------------------- */

CodeGen*
target_code_gen()
{
    return dynamic_cast<SuifVmContext*>(the_context)->target_code_gen();
}
