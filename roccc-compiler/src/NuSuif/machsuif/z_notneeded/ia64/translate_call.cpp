/* file "ia64/translate_call.cpp" */

/*
    Copyright (c) 2002 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#include <machine/machine.h>
#include <suifvm/suifvm.h>

#include <ia64/code_gen.h>
#include <ia64/opcodes.h>
#include <ia64/reg_info.h>
#include <ia64/instr.h>
#include <ia64/init.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace ia64;

/*
 * This module contains the routines to translate SUIF call and return
 * instructions (CALL and RET) into the appropriate IA-64 assembly
 * language instructions and assembler pseudo-ops.
 *
 */

/*
 * PROCEDURE:	Translate_call
 *
 * DESCRIPTION:
 *
 * Translates a suifvm call instruction into the corresponding
 * IA-64 assembly instructions.  This routine first
 * generates IA-64 assembly instructions to setup the parameters to be
 * passed, both in the parameter registers and on the stack.  It then
 * generates the call instruction, and finally moves the return value
 * (if any) to the correct abstract register.
 *
 */
void
CodeGenIa64::translate_cal(Instr *cal)
{
    Instr *mi;

    int argi; // argument index
    int num_args = srcs_size(cal) - 1;

    // The number of argument regs is the same for GR and FR banks
    int num_arg_regs = 8;

    // Stack argument area needed by this call.  Starts negative to
    // reflect the fact that the first 8 arguments are passed in
    // registers and saved in callee's stack frame space.
    int arg_area = -num_arg_regs * 8;

    // Abstract numbers of next free FPR and GPR arg registers, resp.
    int next_freeAR_fpr = FR_ARG0,
        next_freeAR_gpr = GR_OUT0;

    // Set of registers possibly clobbered by a call.  To be attached to
    // the call as a regs_defd note for use by DFA.  Killed registers are
    // those in the caller-saved convention.
    static NatSetDense regs_killed;
    if (regs_killed.size() == 0)
        regs_killed = *reg_caller_saves();

    // Set of registers implicitly occupied by actual args at the point of
    // call.  To be added to the call as a regs_used note for use by DFA.
    // The set is filled in by the loop below.
    NatSetDense regs_used;

    // The callee is represented either as a symbol in the target field of
    // the call or as a pointer-valued operand, the first source of the
    // call.
    Sym *target = get_target(cal);	// null if callee isn't known
    Opnd callee_addr;			// address of callee if unknown
    TypeId callee_type = NULL;

    if (target == NULL) {
      callee_addr = get_src(cal, 0);
      callee_type = get_type(callee_addr);
      if (is_pointer(callee_type))
	  callee_type = get_referent_type(callee_type);
    }
    else {
      callee_type = get_type(to<ProcSym>(target));
    }

    // Determine whether callee has a fixed-length parameter list.
    // Otherwise, it must be assumed to accept unnamed arguments.
    bool has_fixed_args = false;

    if (!is_kind_of<CProcedureType>(callee_type))
      warn("Can't resolve called-procedure type");
    else {
      CProcedureType  *c_proc_type = static_cast<CProcedureType*>(callee_type);
      has_fixed_args = c_proc_type->get_arguments_known() &&
		      !c_proc_type->get_has_varargs();
    }

    // Plow through the parameters, creating argument build
    // instructions (or information) as we go.
    for (argi = 1; argi <= num_args; argi++) {
        Opnd arg_opnd = get_src(cal, argi);
        TypeId arg_type = get_type(arg_opnd);

        // Perform offset and argument area size calculations -- arg_area
        // should always remain aligned on a 64-bit boundary.
        claim((arg_area & 7) == 0);             // verify word aligned
        int arg_size = get_bit_size(arg_type) >> 3;
        int arg_offset = arg_area;
        arg_area += arg_size;

        // Pad out arg_area to 64-bit boundary
        int bytes_over_boundary;
        if ((bytes_over_boundary = (arg_area & 7)))
            arg_area += (8 - bytes_over_boundary);

        // Parameter build instructions -- 4 possible scenarios
        //  (1) structure (or union) operand;
        //  (2) FP operand and parameter registers available;
        //  (3) integer operand and parameter registers available;
        //  (4) no parameter registers available.

        // case 1 -- struct move with/without parameter registers
        if (is_record(arg_type)) {

            Instr *mi_ld = NULL;
            // find or generate corresponding structure load
            for (InstrHandle h = struct_lds.begin(); h != struct_lds.end(); ++h)
                if (get_dst(*h) == arg_opnd) {
                    mi_ld = *h;
                    struct_lds.erase(h);
                    break;
                }

            if (mi_ld == NULL) {
                // need to create structure load
                VarSym *v = get_var(arg_opnd);
                arg_opnd = opnd_reg(get_type(arg_opnd));
                mi_ld = new_instr_alm(arg_opnd, suifvm::LOD, opnd_addr_sym(v));
            }

            // generate the struct store into parameter space
            Opnd argo_opnd = opnd_immed(arg_offset+16, type_s64);// scratch area
            Instr *mi_st = new_instr_alm(BaseDispOpnd(opnd_reg_sp, argo_opnd),
                                         suifvm::STR, arg_opnd);

            if ((next_freeAR_gpr - GR_OUT0) >= num_arg_regs)
                expand_struct_move(mi_ld, mi_st);
            else {
                // Struct move requiring parameter registers.  Here we
                // don't use the helper (expand_struct_move) used by
                // translate_str and translate_memcpy, but do our own
                // version of it.  Too hard to redefine the structure move.
                int delta = expand_struct_move2(mi_ld, mi_st,
                                                next_freeAR_gpr, &regs_used);
                next_freeAR_gpr += delta;
            }

        }

        // case 2 -- FP operand to go in FR registers (f8-f15)
        else if (is_floating_point(arg_type)
         && ((next_freeAR_gpr - GR_OUT0) < num_arg_regs)) {
            // move parameter value into FP parameter register
            int opc = make_opcode(MOV_FR, EMPTY,EMPTY,EMPTY);
            Opnd areg_opnd = opnd_reg(next_freeAR_fpr, type_f64);
            mi = new_instr_alm(areg_opnd, opc, arg_opnd);
            emit(mi);
	    
            // update regs_used set for annotation
            regs_used.insert(next_freeAR_fpr);

	    // if this might be a varargs param, duplicate it in GR bank
	    if (!has_fixed_args) {
		Opnd dupe = opnd_reg(next_freeAR_gpr, type_u64);
		opc = make_opcode(GETF, _SD_FORM_D,EMPTY,EMPTY);
		mi = new_instr_alm(dupe, opc, areg_opnd);
		emit(mi);
	    }

            // update next_freeAR
            next_freeAR_gpr += 1; //gpr skips over this slot
            next_freeAR_fpr += 1; //but fpr only increments if used
        }

        // case 3 -- integer operand to go in GR registers (out0-out7)
       else if (!is_floating_point(arg_type)
        && ((next_freeAR_gpr - GR_OUT0) < num_arg_regs)) {
            // move parameter value into GP parameter register
            // FIXME - Assumes a register source - what is correct?
            int opc = make_opcode(MOV_GR, EMPTY,EMPTY,EMPTY);
            Opnd areg_opnd = opnd_reg(next_freeAR_gpr, type_s64);
            mi = new_instr_alm(areg_opnd, opc, arg_opnd);
            emit(mi);

            // update regs_used set for annotation
            regs_used.insert(next_freeAR_gpr);

            // update next_freeAR
            next_freeAR_gpr += 1;
       }

        // case 4 -- put parameter on stack, skipping over the caller's
	//	     16-byte scratch area
       else {
            int s_opcode = opcode_store(arg_type);
            claim((next_freeAR_gpr - GR_OUT0) >= num_arg_regs);

            // add vr = offset, sp
            Opnd argo_opnd = opnd_immed(arg_offset + 16, type_s64);
            Opnd stackptr = opnd_reg(type_p64);
            int addopc = make_opcode(ADD, EMPTY,EMPTY,EMPTY);
            emit(new_instr_alm(stackptr, addopc, argo_opnd, opnd_reg_sp));

            // st [vr] = arg
            Opnd stackAddr = BaseDispOpnd(stackptr, opnd_immed_0_u64, arg_type);
            emit(new_instr_alm(stackAddr, s_opcode, arg_opnd));
       }
    }

    // Save away gp
    Opnd gpSav = opnd_reg(type_u64);
    int opc = make_opcode(MOV_GR, EMPTY,EMPTY,EMPTY);
    emit( new_instr_alm(gpSav, opc, opnd_reg_gp));
    regs_used.insert(gpSav); // FIXME?
    
    // Generate the actual call instruction 
    opc = make_br_opcode(BR, BTYPE_CALL,BWH_SPTK,PH_MANY,DH_NONE);
    if (target == NULL) {
      Opnd bsrc = opnd_reg(type_br);
      int movOpc = make_opcode(MOV_BR, EMPTY,EMPTY,EMPTY);
      emit( new_instr_alm(bsrc, movOpc, callee_addr));
      emit( mi = new_instr_cti(opnd_br_ra, opc, target, bsrc));
    }
    else {
      emit( mi = new_instr_cti(opnd_br_ra, opc, target));
    }

    // Generate reload of $gp
    opc = make_opcode(MOV_GR, EMPTY,EMPTY,EMPTY);
    emit( new_instr_alm(opnd_reg_gp, opc, gpSav));
 
    NatSetDense regs_defd = regs_killed; // for k_regs_defd annotation

    // Generate copy of return values, if necessary
    for (int i=0; i < (dsts_size(cal)); i++) {
        Opnd d = get_dst(cal, i);
        TypeId d_type = get_type(d);

        int opcode, reg;
        if (is_floating_point(d_type)) {
            opcode = make_opcode(MOV_FR, EMPTY,EMPTY,EMPTY);
            reg = FR_ARG0 + i;
        } else if (is_record(d_type)) {
            claim(false, "Need to implement return structs in IA-64"); //FIXME
        } else {
            opcode = make_opcode(MOV_GR, EMPTY,EMPTY,EMPTY);
            reg = GR_RET0 + i;
        }
        emit(new_instr_alm(d, opcode, opnd_reg(reg, d_type)));
        regs_defd.insert(reg);
    }
    set_note(mi, k_regs_defd, NatSetNote(&regs_defd));

    // Update per-procedure variables
    claim((arg_area & 7) == 0);         // better be aligned
    is_leaf = false;
    if (max_arg_area < arg_area)
        max_arg_area = arg_area;
    delete cal;

}



/*
 * PROCEDURE:	Translate_return()
 *
 * DESCRIPTION:
 *
 * Translates RET suifvm instructions.  The finishing pass does the
 * actual appending of instructions that restore registers.  The printing
 * routines insert the hint bits.
 */
void
CodeGenIa64::translate_ret(Instr *ret)
{

    TypeId r_type = NULL;

    // Translate RET, if return values exist then we need to place them in
    // correct hard return registers.
    for (int i=0; i<srcs_size(ret); i++) {
        Opnd r_opnd = get_src(ret, i);
        r_type = get_type(r_opnd);
        int opc;

        claim(srcs_size(ret) <= 4); /* FIXME - rest should go on stack */
	/* FIXME: Cover case where return values goes in given pointer */

        if (is_floating_point(r_type)) {
            Opnd dstReg = opnd_reg((FR_ARG0 + i), r_type);
            opc = make_opcode(MOV_FR, EMPTY,EMPTY,EMPTY);
            emit(new_instr_alm(dstReg, opc, r_opnd));
        }
        else if (is_void(r_type)) {
            r_type = NULL;
        }
        else {
            Opnd dstReg = opnd_reg((GR_RET0 + i), r_type);
            opc = make_opcode(MOV_GR, EMPTY,EMPTY,EMPTY);
            emit(new_instr_alm(dstReg, opc, r_opnd));
        }

        // Note the register used as being live.
        if (r_type) {
            NatSetDense regs_used;
            if (is_floating_point(r_type))
                regs_used.insert(FR_ARG0 + i);
            else
                regs_used.insert(GR_RET0 + i);
            set_note(ret, k_regs_used, NatSetNote(&regs_used));
        }
    }

    // Now make the return into an IA64 return
    set_opcode(ret,make_br_opcode(ia64::BR,BTYPE_RET,BWH_SPTK,PH_MANY,DH_NONE));
    set_src(ret, 0, opnd_br_ra);

    // remember that this is a return (and not a switch or something else)
    ListNote<IrObject*> note;
    if (r_type)
        note.set_value(0, r_type);
    set_note(ret, k_instr_ret, note);

    // remember that we need to restore the saved regs and update the
    // stack pointer during the finishing pass
    set_note(ret, k_incomplete_proc_exit, note_flag());

    emit(ret);                          // ret $ra
}

/* Ugly code required to move part or all of a parameter structure
 * using the parameter registers.  This routine returns the number
 * of argument registers consumed. */
int
CodeGenIa64::expand_struct_move2(Instr *lr, Instr *sr, int next_freeAR_gpr,
				 NatSet *regs_used)
{
    int first_AR = next_freeAR_gpr;

    // determine structure alignment
    TypeId rec_type = get_type(get_dst(lr));
    int rec_size = get_bit_size(rec_type) >> 3; // in bytes
    int struct_alignment = get_bit_alignment(rec_type);

    if (struct_alignment == 0) struct_alignment = 8;

    TypeId mv_type;
    int sload_op, sstore_op, increment_value;
    switch (struct_alignment) {
      case 128:
        sload_op = make_opcode(LDFP, FSZ_D,FLDTYPE_NONE,LDHINT_NONE);
        sstore_op = make_opcode(STF, FSZ_D,STTYPE_NONE,STHINT_NONE);
        increment_value = 16;    // in bytes
        claim(false); //mv_type = type_s128; FIXME:doesn't exist
        break;
      case 80:
        sload_op = make_opcode(LDF, FSZ_E,FLDTYPE_NONE,LDHINT_NONE);
        sstore_op = make_opcode(STF, FSZ_E,STTYPE_NONE,STHINT_NONE);
        increment_value = 10;    // in bytes
        mv_type = type_f80;
        break;
      case 64:
        sload_op = make_opcode(LD, SZ_8,LDTYPE_NONE,LDHINT_NONE);
        sstore_op = make_opcode(ST, SZ_8,STTYPE_NONE,STHINT_NONE);
        increment_value = 8;    // in bytes
        mv_type = type_s64;
        break;
      case 32:
        sload_op = make_opcode(LD, SZ_4,LDTYPE_NONE,LDHINT_NONE);
        sstore_op = make_opcode(ST, SZ_4,STTYPE_NONE,STHINT_NONE);
        increment_value = 4;    // in bytes
        mv_type = type_s32;
        break;
      case 16:
        sload_op = make_opcode(LD, SZ_2,LDTYPE_NONE,LDHINT_NONE);
        sstore_op = make_opcode(ST, SZ_2,STTYPE_NONE,STHINT_NONE);
        increment_value = 2;    // in bytes
        mv_type = type_s16;
        break;
      case 8:
        sload_op = make_opcode(LD, SZ_1,LDTYPE_NONE,LDHINT_NONE);
        sstore_op = make_opcode(ST, SZ_1,STTYPE_NONE,STHINT_NONE);
        increment_value = 1;    // in bytes
        mv_type = type_s8;
        break;
      default:
        claim(false);
    }

    // create placeholder for structure move annotations
    Instr *mi = new_instr_alm(opcode_null);

    ListNote<IdString> begin_note;
    begin_note.append(IdString("BEGIN structure move"));
    set_note(mi, k_comment, begin_note);

    copy_notes(lr, mi);
    copy_notes(sr, mi);
    emit(mi);

    // turn structure load into lda for structure load base register
    Opnd vrldb_opnd = opnd_reg(type_addr());
    set_dst(lr, vrldb_opnd);
    translate_lda(lr);

    // load into parameter registers while possible
    int r_i = 0;                // record index in bytes
    int num_arg_regs = GR_LAST_ARG - GR_ARG0 + 1;
    while (((next_freeAR_gpr - GR_ARG0) < num_arg_regs)
           && (r_i < rec_size)) {
        Opnd areg_opnd = opnd_reg(next_freeAR_gpr, type_s64);

        if (struct_alignment == 64) {
            // easy load
            Opnd ae = BaseDispOpnd(vrldb_opnd, opnd_immed_0_u64);
            Opnd upd = opnd_immed(64, type_u64);
            int opc = make_opcode(LD, SZ_8,LDTYPE_NONE,LDHINT_NONE);
            mi = new_instr_alm(areg_opnd, opc, ae, upd); 
            append_dst(mi, vrldb_opnd); 	//Pseudo 2nd dst
            emit(mi);

        } else {
            // gotta love potentially unaligned loads
            claim(false, "Unaligned load not implemented"); //FIXME
        }

        // update regs_used set for annotation
        regs_used->insert(next_freeAR_gpr);

        // update indices
        next_freeAR_gpr += 1;           // update next_freeAR
        r_i += 8;                       // increment in bytes
    }

    // finished with parameter register loads.  Do we have any
    // more structure to move?  If so, dump it onto the stack.
    if (r_i < rec_size) {
        // create base register for stores
        Opnd vrstb_opnd = opnd_reg(type_addr());
        mi = new_instr_alm(vrstb_opnd, suifvm::LDA,
                           BaseDispOpnd(opnd_reg_sp, opnd_immed_0_u64));
        translate_lda(mi);

        // perform load followed by store, repeatedly
        int s_i = 0;
        while (r_i < rec_size) {
            Opnd vr_opnd = opnd_reg(mv_type);
            Opnd incVal = opnd_immed(increment_value, type_u64);

            // load
            Opnd ae = BaseDispOpnd(vrldb_opnd, opnd_immed_0_u64);
            mi = new_instr_alm(vr_opnd, sload_op, ae, incVal);
            append_dst(mi, vrldb_opnd);
            emit(mi);

            // store
            ae = BaseDispOpnd(vrstb_opnd, opnd_immed_0_u64);
            mi = new_instr_alm(ae, sstore_op, vr_opnd, incVal);
            append_dst(mi, vrstb_opnd);
            emit(mi);

            // update indices
            r_i += increment_value;
            s_i += increment_value;
        }
    }

    // mark end of the structure move
    mi = new_instr_alm(opcode_null);

    ListNote<IdString> end_note;
    end_note.append(IdString("END structure move"));
    set_note(mi, k_comment, end_note);

    emit(mi);

    return next_freeAR_gpr - first_AR;

}
