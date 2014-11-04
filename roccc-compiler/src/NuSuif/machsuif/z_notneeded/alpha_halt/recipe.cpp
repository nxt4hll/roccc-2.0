/* file "alpha_halt/recipe.cpp" */

/*  Copyright (c) 1995-2000 The President and Fellows of Harvard University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. 
*/

#include <common/suif_copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "alpha_halt/recipe.h"
#endif

#include <machine/machine.h>
#include <halt/halt.h>
#include <alpha/alpha.h>

#include "recipe.h"

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace alpha;
using namespace halt;

Vector<HaltRecipe*> halt_recipes_alpha;

void
init_halt_recipes_alpha(SaveArea *save_area)
{
    halt_recipes_alpha.resize(LAST_HALT_KIND + 1, NULL);

    halt_recipes_alpha[STARTUP]	  = new HaltRecipeAlphaStartup(save_area);
    halt_recipes_alpha[CBR]	  = new HaltRecipeAlphaCbr(save_area);
    halt_recipes_alpha[ENTRY]	  = new HaltRecipeAlphaEntry(save_area);
    halt_recipes_alpha[EXIT]	  = new HaltRecipeAlphaExit(save_area);
    halt_recipes_alpha[MBR]	  = new HaltRecipeAlphaMbr(save_area);
    halt_recipes_alpha[LOAD]	  = new HaltRecipeAlphaLoad(save_area);
    halt_recipes_alpha[STORE]	  = new HaltRecipeAlphaStore(save_area);
    halt_recipes_alpha[SETJMP]	  = new HaltRecipeAlphaSetjmp(save_area);
    halt_recipes_alpha[LONGJMP]	  = new HaltRecipeAlphaLongjmp(save_area);
    halt_recipes_alpha[BLOCK]	  = new HaltRecipeAlphaBlock(save_area);
    halt_recipes_alpha[CYCLE]	  = new HaltRecipeAlphaCycle(save_area);
}

void
clear_halt_recipes_alpha()
{
    for (unsigned i = 0; i < halt_recipes_alpha.size(); i++)
	if (halt_recipes_alpha[i])
	    delete halt_recipes_alpha[i];
}


/**
 ** HaltRecipeAlpha methods
 **/

/*
 * Insert all static arguments into the argument list.  Start with the
 * unique id of the instrumentation point, and follow with any optional
 * static args.
 */
void
HaltRecipeAlpha::static_args(HaltLabelNote note)
{
    args.push_back(opnd_immed(note.get_unique_id(), type_s64));

    int size_static_args = note.get_size_static_args();
    for (int i = 0; i < size_static_args; ++i)
	args.push_back(opnd_immed(note.get_static_arg(i), type_s64));
}

// Converts the set of live registers into a set of registers to 
// be saved before the instrumentation call.
void
HaltRecipeAlpha::build_save_set(NatSet *save, const NatSet *live)
{
    save->insert(GP);		// $gp always needs saving, but may not appear live

    for(NatSetIter iter(live->iter()); iter.is_valid(); iter.next()) {
	int r = iter.current();

	save->insert(r);
    }
    save->remove(SP);		// $sp never needs saving
    save->remove(CONST0_GPR);	// $31 (const0) never needs saving
    save->remove(CONST0_FPR);	// $f31 never needs saving
}

void
HaltRecipeAlpha::setup_stack()
{
    debug(2, "%s:setup_stack", __FILE__);
    claim(size(instr_pot[SETUP]) == 0);

    // nothing to do on alpha
}

void
HaltRecipeAlpha::save_state(NatSet *saved_reg_set)
{
    debug(2, "%s:save_state", __FILE__);
    claim(size(instr_pot[SAVE]) == 0);

    int regs_to_save = saved_reg_set->size();

    for (NatSetIter rs(saved_reg_set->iter()); rs.is_valid(); rs.next()) {
	int r = rs.current();
	Opnd addr = save_area_addr(--regs_to_save);
	Instr *mi;

	if (r >= CONST0_GPR && r <= LAST_GPR)
	    mi = new_instr_alm(addr, STQ, opnd_reg(r, type_s64));
        else if (r > LAST_GPR)
	    mi = new_instr_alm(addr, STT, opnd_reg(r, type_s64));
//
// FIXME: the above is a hack: we assume that all GP regs come before all
// FP regs.  There needs to be some better means of check a register's
// bank, that still allows alpha-library extenders the freedom to play with
// classes.

//	else
//	    claim(false, "Unexpected reg bank in saved-reg set");
	append(instr_pot[SAVE], mi);
    }
}

void
HaltRecipeAlpha::insert_args()
{
    // args list contains immeds and regs - put immeds into regs 

    debug(2, "%s:insert_args", __FILE__);
    claim(size(instr_pot[ARGS]) == 0);
    // assume no more than 6 args to analysis routine
    claim(args.size() <= 6);

    // We're about to move the arg values represented by operands args[i],
    // for i = 0,...,|args|-1, into the hardware arg registers a0,a1,...,
    // in that order.  If it happens that operand arg[i] is actually the
    // arg register aj, for j < i, then we will clobber the value of arg[i]
    // before it can be transferred.  We must detect this problem and move
    // arg[i] to an unused register.

    const NatSet *pool = reg_allocables();
    NatSetDense used_regs;

    for(unsigned i = 0; i < args.size(); i++)
	if (is_hard_reg(args[i]))
	    used_regs.insert(get_reg(args[i]));

    // The vulnerable arg values are those args[i] such that i > 0 and
    // args[i] is a register in the range [a0,ai).  Make sure that regs in
    // [a0,ai) are marked used by the time args[i] is considered.

    for (int i = 1; (unsigned)i < args.size(); i++) {
	used_regs.insert(ARG0_GPR + i - 1);

	Opnd arg = args[i];
	if (is_hard_reg(arg)) {
	    int reg = get_reg(arg);

	    // If arg[i] will not be clobbered before being MOVed, skip it.
	    if (reg < ARG0_GPR || reg >= ARG0_GPR + i)
		continue;

      	    // Else find an unused temp reg and move the args[i] value there.
	    NatSetIter it = pool->iter();
	    for (/* */; it.is_valid(); it.next()) {
		int k = it.current();

		if (!used_regs.contains(k)) {
		    Opnd temp = opnd_reg(k, get_type(arg));
		    append(instr_pot[ARGS], new_instr_alm(temp, MOV, arg));
		    args[i] = temp;
		    used_regs.insert(k);
		    break;
		}
	    }
	    claim(it.is_valid(), "Too few registers to shuffle args");
	}
    }	
    
    // Put each args[i] into ARG0_GPR + i, unless it's there already.
    for (int i = 0; (unsigned)i < args.size(); i++) {
	Opnd arg = args[i];
	if (is_hard_reg(arg) && get_reg(arg) == ARG0_GPR + i)
	    continue;

	if (is_reg(arg) || is_immed(arg) || is_var(arg)) {
	    Opnd ai = opnd_reg(ARG0_GPR+i, get_type(arg));

	    append(instr_pot[ARGS], new_instr_alm(ai, MOV, arg));

	    // if reg do as above, if addressSymbol (is_addr(opnd)), make an 
	    // LDA instr with same dst.

	} else {
	    claim(false, "Analysis routine argument imMOVable");
	}
    }
}

void
HaltRecipeAlpha::insert_call(ProcSym *ps)
{
    debug(2, "%s:insert_call", __FILE__);
    claim(size(instr_pot[CALL]) == 0);

    Instr *mi;
    mi = new_instr_alm(opnd_reg_pv, LDA, opnd_addr_sym(ps));
    append(instr_pot[CALL], mi);
    mi = new_instr_cti(opnd_reg_ra, JSR, NULL, opnd_reg_pv);
    append(instr_pot[CALL], mi);

    // Set of registers possibly defined at the point of call.  To be
    // attached to the call as a regs_defd note for use by DFA.  Defined
    // registers include all those in the caller-saved convention, plus the
    // assembler temporaries, plus $gp.
    static NatSetDense regs_defd;
    if (regs_defd.size() == 0) {
	regs_defd = *reg_caller_saves();
	regs_defd.insert(GP);
	for (int i = ASM_TMP0; i <= LAST_ASM_TMP; ++i)
	    regs_defd.insert(i);
    }
    set_note(mi, k_regs_defd, NatSetNote(&regs_defd));
}

void
HaltRecipeAlpha::clean_args()
{
    debug(2, "%s:clean_args", __FILE__);
    claim(size(instr_pot[CLEAN]) == 0);

    args.resize(0);

    // This function is supposed to clean up whatever may have been done when
    // the arguments to the analysis routine were set up for the call. 
    // In x86 this involved popping those args off the stack, but for alpha 
    // the arguments are set up by being placed in certain registers, which 
    // will be returned to their original state by the restore_state function.

}

void
HaltRecipeAlpha::restore_state(NatSet *saved_reg_set)
{
    debug(2, "%s:restore_state", __FILE__);
    claim(size(instr_pot[RESTORE]) == 0);

    int regs_to_restore = saved_reg_set->size();

    for(NatSetIter rs(saved_reg_set->iter()); rs.is_valid(); rs.next()) {
	int r = rs.current();
	Opnd addr = save_area_addr(--regs_to_restore);
	Instr* mi;

	if (r >= CONST0_GPR && r <= LAST_GPR)
	    mi = new_instr_alm(opnd_reg(r, type_s64), LDQ, addr);
	else if (r > LAST_GPR)
	    mi = new_instr_alm(opnd_reg(r, type_s64), LDT, addr);
//
// See note about hack in HaltRecipeAlpha::save_state()

//	else
//	    claim(false, "Unexpected reg bank in saved-reg set");
	append(instr_pot[RESTORE], mi);
    }
}

void
HaltRecipeAlpha::destroy_stack()
{
    debug(2, "%s:destroy_stack", __FILE__);
    claim(size(instr_pot[DESTROY]) == 0);

    // nothing to do on alpha
}

/*
 * Produce the address operand of a slot in the save area for the current
 * opt unit.  The `pos' index is in register-sized words.  If negative, add
 * it to the length of the save area to obtain the actual slot number.
 */
Opnd
HaltRecipeAlpha::save_area_addr(int pos) const
{
    VarSym *var = save_area->get_var();
    claim(var != NULL, "Register-save area not initialized");

    if (pos < 0)
	pos = save_area->get_length() + pos;
    claim((unsigned)pos < (unsigned)save_area->get_length(),
	  "Bad save-area position");

    return SymDispOpnd(opnd_addr_sym(var), opnd_immed(8 * pos, type_s32));
}


/**
 ** HaltRecipeAlpha* operator() implementations
 **/

// instrument_start_alpha
void
HaltRecipeAlphaStartup::operator()(HaltLabelNote note, InstrHandle h, CfgNode *n,
				   const NatSet *before, const NatSet *after)
{
    debug(2, "%s:HaltRecipeAlphaStart", __FILE__);

    // build args list -- no dynamic arguments
    static_args(note);

    follow_recipe(STARTUP, after);
    insert_instrs(AFTER, n, h);
}

// instrument_branch_alpha
void
HaltRecipeAlphaCbr::operator()(HaltLabelNote note, InstrHandle h, CfgNode *n,
			       const NatSet *before, const NatSet *after)
{
    debug(2, "%s:HaltRecipeAlphaCbr", __FILE__);

    Opnd cond = evaluate_cond(h);
    
    args.push_back(cond);	// dynamic arg
    static_args(note);

    follow_recipe(CBR, before);
    insert_instrs(BEFORE, n, h);
}

// Helper function to evaluate the condition of a branch instruction
Opnd
HaltRecipeAlphaCbr::evaluate_cond(InstrHandle h)
{
    int opcode = get_opcode(*h);
    Opnd src = get_src(*h, 0);

    Opnd a1 = opnd_reg(ARG0_GPR + 1, type_u64);
    Opnd int_zero = opnd_reg_const0;
    Opnd float_zero = opnd_reg(CONST0_FPR, type_f64);

    switch (opcode) {
 	// For integer CBRs, insert a corresponding compare that realizes the
	// branch condition in arg register a1 (0 => fall-through, 1 => taken).
      case BR:
      case BSR:
	claim(false, "This branch is unconditional");
	return a1;
      case BEQ:
	append(instr_pot[KIND], new_instr_alm(a1, CMPEQ, src, int_zero));
	return a1;     
      case BGE:	
	// a >= 0   <=>   0 <= a
	append(instr_pot[KIND], new_instr_alm(a1, CMPLE, int_zero, src));
	return a1;
      case BGT:
	// a > 0   <=>   0 < a
	append(instr_pot[KIND], new_instr_alm(a1, CMPLT, int_zero, src));	
	return a1;
      case BLBC:
      case BLBS:
	claim(false, "BLBC/BLBS branch evaluation not yet implemented");
      case BLE:
	append(instr_pot[KIND], new_instr_alm(a1, CMPLE, src, int_zero));	
	return a1;
      case BLT:
	append(instr_pot[KIND], new_instr_alm(a1, CMPLT, src, int_zero));	
	return a1;
      case BNE:
	// a != 0  <=>  0 < (unsigned)a
	append(instr_pot[KIND], new_instr_alm(a1, CMPULT, int_zero, src));
	return a1;
    }

    // If the source operand of a floating cbr is a hard register, then it
    // will automatically be saved and restored around the instrumentation
    // point by virtue of its liveness there; thus, we can use it as a
    // temporary.  If not, register allocation must be coming later on, so
    // we use a fresh virtual register as a temporary.

    claim(is_hard_reg(src) || is_virtual_reg(src) || is_var(src));
    Opnd tmp = (is_hard_reg(src)) ? src : opnd_reg(get_type(src));

    bool invert = false;			// maybe invert sense of compare

    switch (opcode) {
      case FBNE:
	invert = true;				// fall through to case FBEQ
      case FBEQ:
	append(instr_pot[KIND], new_instr_alm(tmp, CMPTEQ, float_zero, src));
	break;
      case FBLT:
	append(instr_pot[KIND], new_instr_alm(tmp, CMPTLT, src, float_zero));
	break;
      case FBLE:
	append(instr_pot[KIND], new_instr_alm(tmp, CMPTLE, src, float_zero));
	break;
      case FBGT:
	// a > 0   <=>   0 < a
	append(instr_pot[KIND], new_instr_alm(tmp, CMPTLT, float_zero, src));
	break;
      case FBGE:
	// a >= 0   <=>   0 <= a
	append(instr_pot[KIND], new_instr_alm(tmp, CMPTLE, float_zero, src));
	break;
      default:
	claim(false, "Unexpected instruction marked as conditional branch");
    }

    // Here the result of a floating compare is in `tmp'.  It is a true
    // zero to signify false, and is otherwise a non-zero floating value.
    // Emit the following instructions to move the value to $a1 and convert
    // it to integer 1 if non-zero:
    //
    //	stt	tmp,save_area_last
    //	ldq	$a1,save_area_last
    //	cmovne	$a1,1,$a1
    //
    //  where save_area_last is the last slot of the register-save area.
    //
    //  For the FBNE case, an additional instruction is required to invert the
    //  low-order bit of $a1.  (That's because there's no CMPTNE instruction.)

    append(instr_pot[KIND], new_instr_alm(save_area_addr(-1), STT, tmp));
    append(instr_pot[KIND], new_instr_alm(a1, LDQ, save_area_addr(-1)));

    append(instr_pot[KIND], new_instr_alm(a1, CMOVNE, a1, opnd_immed_1_u64));
    if (invert)
	append(instr_pot[KIND], new_instr_alm(a1, XOR, a1, opnd_immed_1_u64));
    return a1;
}

// instrument_entry_alpha
void
HaltRecipeAlphaEntry::operator()(HaltLabelNote note, InstrHandle h, CfgNode *n,
				const NatSet *before, const NatSet *after)
{
    debug(2, "%s:HaltRecipeAlphaEntry", __FILE__);

    // build args list -- no dynamic arguments
    static_args(note);

    follow_recipe(ENTRY, after);
    insert_instrs(AFTER, n, h);
}

// instrument_exit_alpha
void
HaltRecipeAlphaExit::operator()(HaltLabelNote note, InstrHandle h, CfgNode *n,
			       const NatSet *before, const NatSet *after)
{
    debug(2, "%s:HaltRecipeAlphaExit", __FILE__);

    // build args list -- no dynamic arguments
    static_args(note);

    follow_recipe(EXIT, before);
    insert_instrs(BEFORE, n, h);
}

// instrument_mbr_alpha
//
// The instrumented instruction is not the actual multiway branch.  It is
// an earlier instruction that computes the target address for the branch.
// (It is identified by the k_mbr_target_def annotation.)
//
// For Alpha, it is an S8ADDQ instruction whose first source operand holds
// the dispatch-table index that is to be passed to _record_mbr.

void
HaltRecipeAlphaMbr::operator()(HaltLabelNote note, InstrHandle h, CfgNode *n,
			     const NatSet *before, const NatSet *after)
{
    debug(2, "%s:HaltRecipeAlphaMbr", __FILE__);
    claim(has_note(*h, k_mbr_target_def) || has_note(*h, k_mbr_index_def));

    claim(get_opcode(*h) == S8ADDQ, "Can't recognize mbr index calculation");

    Opnd index = get_src(*h, 0);

    args.push_back(index);	// dynamic arg
    static_args(note);

    follow_recipe(MBR, before);
    insert_instrs(BEFORE, n, h);
}


// instrument_load_alpha
void
HaltRecipeAlphaLoad::operator()(HaltLabelNote note, InstrHandle h, CfgNode *n,
			      const NatSet *before, const NatSet *after)
{
    debug(2, "%s:instrument_load_alpha", __FILE__);
    
    Instr* mi;
    Opnd ea_opnd, bytes_opnd;

    // After live regs have been saved, but before args are prepared,
    // put the effective address in a temp reg.

    ea_opnd = opnd_reg(TMP0_GPR, type_ptr);
    mi = new_instr_alm(ea_opnd, LDA, get_src(*h, 0));
    append(instr_pot[KIND], mi);

    // The number of bytes to be loaded is 1st dynamic arg.
    // The effective address is the 2nd dynamic arg.
    bytes_opnd = opnd_immed(num_bytes_ld(h), type_u32);
    args.push_back(bytes_opnd);
    args.push_back(ea_opnd);

    static_args(note);
    
    follow_recipe(LOAD, after);
    insert_instrs(AFTER, n, h); 
}

// instrument_store_alpha
void
HaltRecipeAlphaStore::operator()(HaltLabelNote note, InstrHandle h, CfgNode *n,
			       const NatSet *before, const NatSet *after)
{
    debug(2, "%s:instrument_store_alpha", __FILE__);

    Instr* mi;
    Opnd ea, tmp_opnd, bytes_opnd; 

    ea = get_dst(*h);

    tmp_opnd = opnd_reg(TMP0_GPR, type_ptr); 
    mi = new_instr_alm(tmp_opnd, LDA, ea); 
    append(instr_pot[KIND], mi);
    
    // find # bytes stored and create immed
    bytes_opnd = opnd_immed(num_bytes_st(h), type_u32);

    // # bytes moved (dyn. arg. #1)
    args.push_back(bytes_opnd);
    // effective address (dynamic arg. #2)
    args.push_back(tmp_opnd);

    static_args(note);

    follow_recipe(STORE, after);
    insert_instrs(AFTER, n, h);
}

// helper functions for load and store instrumentation
int 
HaltRecipeAlphaLoad::num_bytes_ld (InstrHandle h) 
{
    Opnd src;
    
    for (int i=0; i<srcs_size(*h); i++) {
	src = get_src(*h, i);
	if (is_addr(src))
	    return (get_bit_size(get_deref_type(src)) / 8);
    }
    return 0;
}

int 
HaltRecipeAlphaStore::num_bytes_st (InstrHandle h) {

    Opnd dst;
    
    for (int i=0; i<dsts_size(*h); i++) {
	dst = get_dst(*h, i);
	if (is_addr(dst))
	    return (get_bit_size(get_deref_type(dst)) / 8);
    }	
    return 0;
}	

// instrument_setjmp_alpha
void
HaltRecipeAlphaSetjmp::operator()(HaltLabelNote note, InstrHandle h, CfgNode *n,
				  const NatSet *before, const NatSet *after)
{
    debug(2, "%s:instrument_setjmp_alpha", __FILE__);
    
    // one dynamic arg: the jmp_buf arg of the setjmp call
    args.push_back(opnd_reg(ARG0_GPR, type_u64));

    static_args(note);

    follow_recipe(SETJMP, before);
    insert_instrs(BEFORE, n, h);
}

// instrument_longjmp_alpha
void
HaltRecipeAlphaLongjmp::operator()
    (HaltLabelNote note, InstrHandle h, CfgNode *n,
     const NatSet *before, const NatSet *after)
{
    debug(2, "%s:instrument_long_jmp_alpha", __FILE__);

    // two dynamic args: the jmp_buf and status args to the longjmp call
    args.push_back(opnd_reg(ARG0_GPR,     type_u64));
    args.push_back(opnd_reg(ARG0_GPR + 1, type_u64));
    
    static_args(note);
    
    follow_recipe(LONGJMP, before);
    insert_instrs(BEFORE, n, h);
}

// instrument_block_alpha
void
HaltRecipeAlphaBlock::operator()(HaltLabelNote note, InstrHandle h, CfgNode *n,
				 const NatSet *before, const NatSet *after)
{
    debug(2, "%s:instrument_block_alpha", __FILE__);

    // There are no dynamic arguments to _record_block.
    static_args(note);

    follow_recipe(BLOCK, after);
    insert_instrs(AFTER, n, h);
}


// instrument_cycle_alpha
void
HaltRecipeAlphaCycle::operator()(HaltLabelNote note, InstrHandle h, CfgNode *n,
				 const NatSet *before, const NatSet *after)
{
    debug(2, "%s:instrument_cycle_alpha", __FILE__);

    // There are no dynamic args to _record_cycle.
    static_args(note);

    follow_recipe(CYCLE, after);
    insert_instrs(AFTER, n, h);
}
