/* file "exp_alpha/exp_alpha.cpp" */

/*
    Copyright (c) 2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "exp_alpha/exp_alpha.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <alpha/alpha.h>

#include <exp_alpha/exp_alpha.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif


/*
 * This program expands Alpha macro assembly language instructions
 * into the real sequence of machine instructions for a particular
 * version of the Alpha architecture.
 */

using namespace alpha;

// Some useful constants for immediate expansion
#define MAX_u8BIT 255
#define MIN_u8BIT (0)
#define MAX_15BIT 32767
#define MIN_15BIT (-32768)
#define MAX_31BIT ((long)(1UL<<31)-1)
#define MIN_31BIT ((long)-(1UL<<31))


//////////////////  Helper classes for scanning/editing code  /////////////////

class SpillVolatilesOpnd : public OpndFilter
{
  public:
    SpillVolatilesOpnd(ExpAlpha &exp_alpha, InstrHandle ih)
	: exp_alpha(exp_alpha), ih(ih) { }
    Opnd operator()(Opnd, InOrOut);

  protected:
    ExpAlpha &exp_alpha;
    InstrHandle ih;
};

class SpillVolatilesInstr : public InstrFilter
{
  public:
    SpillVolatilesInstr(ExpAlpha &exp_alpha) : exp_alpha(exp_alpha) { }
    void operator()(InstrHandle ih)
    {
	SpillVolatilesOpnd spill_volatiles_opnd(exp_alpha, ih);
	map_opnds(*ih, spill_volatiles_opnd);
    }
  protected:
    ExpAlpha &exp_alpha;
};

/*
 * If opnd is a non-local or address-taken variable symbol, return a new
 * virtual register after inserting a load (store) of that VR from (to)
 * that symbol as an effective address.  Parameter in_or_out is IN (OUT)
 * if the variable is an input (destination) operand, and therefore should
 * be loaded before (stored after) the original instruction.
 */
Opnd
SpillVolatilesOpnd::operator()(Opnd opnd, InOrOut in_or_out)
{
    if (!is_var(opnd))
	return opnd;

    VarSym *v = get_var(opnd);

    if (is_auto(v) && !is_addr_taken(v))
	return opnd;

    TypeId type = get_type(v);
    Opnd vr = opnd_reg(type);
    Opnd ea = opnd_addr_sym(v);

    if (in_or_out == IN)
	exp_alpha.insert_before(ih, new_instr_alm(vr, opcode_load(type),  ea));
    else
	exp_alpha.insert_after (ih, new_instr_alm(ea, opcode_store(type), vr));

    return vr;
}

class MaybeExpandInstr : public InstrFilter
{
  public:
    MaybeExpandInstr(ExpAlpha &exp_alpha) : exp_alpha(exp_alpha) { }
    void operator()(InstrHandle);

  protected:
    ExpAlpha &exp_alpha;
};

enum { IS_STORE = false, IS_LOAD = true };

void
MaybeExpandInstr::operator()(InstrHandle ih)
{
    // If we were using the $at register in the last expansion, mark
    // that we're done now (before the next mi).

    if (exp_alpha.noat_is_set) {
	exp_alpha.insert_before(ih, new_instr_dot(SET, opnd_immed("at")));
	exp_alpha.noat_is_set = false;
    }

    // We expand instructions using primitives at the next lower level in
    // the expansion hierarchy, and then call the expansion routines for
    // that next lower level.  Since an expansion may add and/or remove
    // instructions, each of the following cases must be carefully ordered
    // and self-contained.

    // FIXME: don't expand byte/word instrs for the later Alpha implementations.

    int opcode = get_opcode(*ih);

    if (exp_alpha.exp_sublong_ld_st &&
	((opcode == LDB) || (opcode == LDBU) ||
	 (opcode == LDW) || (opcode == LDWU)))
    {
	exp_alpha.expand_byteword_memory_instr(ih, IS_LOAD);
    }
    else if (exp_alpha.exp_sublong_ld_st &&
	     ((opcode == STB) || (opcode == STW)))
    {
	exp_alpha.expand_byteword_memory_instr(ih, IS_STORE);
    }
    else if (opcode == LDA || reads_memory(*ih))
    {
	exp_alpha.expand_memory_instr(ih, IS_LOAD);
    }
    else if (writes_memory(*ih))
    {
	exp_alpha.expand_memory_instr(ih, IS_STORE);
    }
    else if ((opcode == LDIL) || (opcode == LDIQ) ||
	     (opcode == LDIF) || (opcode == LDID) ||
	     (opcode == LDIG) || (opcode == LDIS) ||
	     (opcode == LDIT))
    {
	exp_alpha.expand_ldi_immed(ih);
    }
    else if (opcode == LDGP)
    {
	exp_alpha.expand_ldgp_instr(ih);
    }
#if 0
    else if ((opcode == ABSL) || (opcode == ABSQ))
    {
	exp_alpha.expand_abs_instr(*ih, ih);
    }
    else if (opcode == JSR)
    {
	exp_alpha.expand_jsr_instr(*ih, ih);
    }
    else if ((opcode == MULQ) || (opcode == MULL))
    {
	exp_alpha.expand_mul_instr(*ih, ih);
    }
    else
    {
	// check any remaining immediate fields
	exp_alpha.expand_immed_opnds(*ih, ih);
    }
#endif /* 0 */
}


////////////////////  Public methods  ///////////////////

void
ExpAlpha::initialize()
{
    debug(1, "Debug level is %d", debuglvl);

    if (OneNote<long> note = get_note(the_file_block, k_next_free_reloc_num))
	reloc_seq = note.get_value();
    else
	reloc_seq = 1;

    noat_is_set = false;

    // We only need to insert `.set noat' assembler directives if we're
    // using $at for expansions (instead of virtual registers) _and_ we're
    // not inserting relocations.

    insert_set_noat = (!use_virtual_regs && !insert_reloc);
}

void
ExpAlpha::do_opt_unit(OptUnit *unit)
{
    IdString name = get_name(get_proc_sym(unit));
    debug(1, "Processing procedure \"%s\"", name.chars());

    // This pass accepts either a CFG or an InstrList.

    unit_instr_list = NULL;
    unit_cfg = NULL;
    AnyBody *body = get_body(unit);

    if (is_kind_of<Cfg>(body))
    {
	unit_cfg = (Cfg*)body;
	if_debug(5)
	    fprint(stdout, unit_cfg, false, true);	// no layout, just code
    }
    else
	unit_instr_list = to<InstrList>(body);

    // Optionally spill any non-local or address-taken variables.

    if (spill_volatiles)
    {
	SpillVolatilesInstr spill_volatiles_instr(*this);
	map_instrs(spill_volatiles_instr);
    }

    MaybeExpandInstr maybe_expand_instr(*this);
    map_instrs(maybe_expand_instr);

}

void
ExpAlpha::map_instrs(InstrFilter &filter)
{
    if (unit_instr_list != NULL)
	for (InstrHandle ih = start(unit_instr_list);
	     ih != end(unit_instr_list); /* */)
	    filter(ih++);
    else
	for (CfgNodeHandle nh = start(unit_cfg); nh != end(unit_cfg); ++nh)
	{
	    cur_cfg_node = *nh;
	    for (InstrHandle ih = start(*nh); ih != end(*nh); /* */)
		filter(ih++);
	}
}

void
ExpAlpha::insert_before(InstrHandle ih, Instr *instr)
{
    if (unit_instr_list != NULL)
	::insert_before(unit_instr_list, ih, instr);
    else
	::insert_before(cur_cfg_node, ih, instr);
}

void
ExpAlpha::insert_after(InstrHandle ih, Instr *instr)
{
    if (unit_instr_list != NULL)
	::insert_after(unit_instr_list, ih, instr);
    else
	::insert_after(cur_cfg_node, ih, instr);
}

Instr*
ExpAlpha::remove(InstrHandle ih)
{
    if (unit_instr_list != NULL)
	return ::remove(unit_instr_list, ih);
    else
	return ::remove(cur_cfg_node, ih);
}


///////////////////   Expand synthsized memory operations   ////////////////////

void
attach_reloc(Instr *instr, const char *name, int seq, int opnd_pos)
{
    set_note(instr, k_reloc, RelocNote(name, seq, opnd_pos));
}

/*
 * expand_byteword_memory_instr
 *
 * Insert the appropriate instruction sequences for the unimplemented byte
 * and word memory operations.
 */
void
ExpAlpha::expand_byteword_memory_instr(InstrHandle &ih, bool is_load)
{
    InstrHandle mih = ih;
    Instr *mi = *mih, *mj;

    int opcode = get_opcode(mi);
    Opnd d = get_dst(mi);	/* for loads */
    Opnd s = get_src(mi, 0);	/* for stores */

    TypeId u64 = type_u64;
    Opnd o_r23 = (use_virtual_regs ? opnd_reg(u64) : opnd_reg(ASM_TMP0+1, u64));
    Opnd o_r24 = (use_virtual_regs ? opnd_reg(u64) : opnd_reg(ASM_TMP0+2, u64));
    Opnd o_at  = (use_virtual_regs ? opnd_reg(u64) : opnd_reg(ASM_TMP0+0, u64));

    maybe_set_noat(ih);

    // Get the effective address of the memory location into a register.

    Opnd ea = (is_load ? get_src(mi, 0) : get_dst(mi));
    claim(is_addr(ea));

    emit(ih, new_instr_alm(o_at, LDA, ea));		     // lda $at,ea

    // Deal with any expansions required for EA calculation.
    expand_memory_instr(ih, IS_LOAD);

    // Get the data via an unaligned load.  Copy the notes from the
    // original memory instruction to this LDQ_U.  However, erase
    // a reloc note, if any.  Failure to do so can lead to bogus
    // link-time transformations.
    ea = BaseDispOpnd(o_at, opnd_immed_0_u64);
    mj = new_instr_alm(o_r23, LDQ_U, ea);		     // ldq_u $23,0($at)
    copy_notes(mi, mj);
    take_note(mj, k_reloc);				     // erase reloc note
    emit(ih, mj);

    // specialize depending upon the type of memory operation
    if (is_load)
    {							// load
	// determine appropriate type of extract
	int opcode1;
	if ((opcode == LDB) || (opcode == LDBU))
	    opcode1 = EXTBL;
	else
	    opcode1 = EXTWL;

	// prepare for potentially necessary sign extend
	int dist = 0;
	if (opcode == LDB)
	    dist = 56;
	else if (opcode == LDW)
	    dist = 48;

	// do extract
	mj = new_instr_alm(d, opcode1, o_r23, o_at);	// ext*l $23,$at,d
	emit(ih, mj);

	// done with unsigned loads, shift to sign extend signed loads
	if (dist > 0)
	{
	    Opnd o_dist = opnd_immed(dist, type_u32);

	    mj = new_instr_alm(o_r23, SLL, d, o_dist);	// sll d,dist,$23
	    emit(ih, mj);

	    mj = new_instr_alm(d, SRA, o_r23, o_dist);	// sra $23,dist,d
	    emit(ih, mj);
	}
    }
    else
    {							// store
	// determine appropriate opcode for insert and mask operations
	int ins, msk;
	if (opcode == STB) {
	    ins = INSBL;
	    msk = MSKBL;
	} else {					// STW
	    ins = INSWL;
	    msk = MSKWL;
	}

	// do insert and mask of appropriate size
	mj = new_instr_alm(o_r24, ins, s, o_at);	// ins*l s,$at,$24
	emit(ih, mj);
	mj = new_instr_alm(o_r23, msk, o_r23, o_at);	// msk*l $23,$at,$23
	emit(ih, mj);

	// or data together
	mj = new_instr_alm(o_r23, BIS, o_r23, o_r24);	// bis $23,$24,$23
	emit(ih, mj);

	// Do actual store using a STQ_U.  Attach annotes from original
	// memory op to this STQ_U too.  However, erase a reloc note, if
	// any.  Otherwise we may get bogus link-time transformations.

	ea = BaseDispOpnd(o_at, opnd_immed_0_u64);
	mj = new_instr_alm(ea, STQ_U, o_r23);		// stq_u $23,0($at)
	copy_notes(mi, mj);
	take_note(mj, k_reloc);				// erase reloc note
	emit(ih, mj);
    }

    // delete original instruction
    delete remove(mih);
}


/* expand_memory_instr() -- inserts the appropriate instruction sequences
 * for loads/stores of non-local (i.e. non-stack) symbols.  Also, calls
 * routine to expand the immediate fields of EA calculations.
 */
void
ExpAlpha::expand_memory_instr(InstrHandle &ih, bool is_load)
{
    InstrHandle mih = ih;
    Instr *mi = *mih;

    Opnd ea = (is_load ? get_src(mi, 0) : get_dst(mi));
    bool is_addr_sym_ea = is_addr_sym(ea);

    // Only have to check immediate field if EA has no address symbol.
    if (!is_addr_sym_ea && !is_sym_disp(ea)) {
	expand_ea_calc(ih, is_load);
	return;
    }

    // Only continue if we're allowed to use relocations.
    if (!insert_reloc)
	return;

    Sym *s = get_sym(is_addr_sym_ea ? ea : SymDispOpnd(ea).get_addr_sym());

    // Only expand global-variable and procedure symbols.
    if ( !is_kind_of<ProcSym>(s) &&
	(!is_kind_of<VarSym>(s) || is_auto(static_cast<VarSym*>(s))))
	return;

    // Now we know we want to expand the instruction:
    //
    // If it's a "simple" LDA, i.e., one whose source is a plain address
    // symbol, then replace it with a load via $gp instead.
    // 
    // Otherwise, insert an instruction that loads temp register with the
    // address of the symbol.  Then change the original load/store to use
    // the temp register as a base, offset by any displacement from the
    // original EA.

    bool simple_lda = is_addr_sym_ea && get_opcode(mi) == LDA;
    int ea_offset =
	is_addr_sym_ea ? 0 : get_immed_int(SymDispOpnd(ea).get_disp());

    // Decide which address register to use.
    Opnd o_temp;
    if (simple_lda)
	o_temp = get_dst(mi);
    else
	o_temp = use_virtual_regs
		     ? opnd_reg(type_ptr)
		     : opnd_reg(ASM_TMP0, type_ptr);
    if (!simple_lda)
	maybe_set_noat(ih);
    Instr *mj;

    // Create new EA: sym($gp)
    IndexSymDispOpnd new_ea(opnd_reg_gp, opnd_addr_sym(s), opnd_immed_0_u64);

    // Insert new instr to precede original:  ldq $temp, sym($gp)!literal!nnn
    // (For the time being, it is actually _after_ the original.)  Transfer
    // notes in case we drop the original.

    mj = new_instr_alm(o_temp, LDQ, new_ea);
    attach_reloc(mj, "literal", reloc_seq, 0);
    copy_notes(mi, mj);
    emit(ih, mj);

    remove(mih);					// extract original

    if (simple_lda) {
	delete mi;					// don't need original
	reloc_seq += 1;
    }
    else
    {
	// Change EA in original instruction to offset($temp) where offset
	// comes from offset field of symaddr.  Also, expand immediates to
	// ensure that the offset fits in the signed 16-bit literal field.

	Opnd o_offset = opnd_immed(ea_offset >> 3, type_s16);
	Opnd new_ea = BaseDispOpnd(o_temp, o_offset);
	if (is_load)
	    set_src(mi, 0, new_ea);
	else
	    set_dst(mi, new_ea);

	// Optionally attach a !lituse_base!nnn relocation note.  We can
	// only attach this annotation because o_temp is unmodified between
	// the !literal!nnn definition point and this !lituse_base!nnn
	// point.  The call to expand_ea_calc may well insert instructions
	// in this range, but it will not overwrite o_temp.

	if (insert_lituse)
	    attach_reloc(mi, "lituse_base", reloc_seq, 0);

	reloc_seq += 1;
	emit(ih, mi);					// reinsert original
	expand_ea_calc(ih, is_load);
    }
}

/*
 * expand_ea_calc() -- Finds and rewrites effective-address calculations
 * that have immediate values that are too big for the literal fields.  The
 * immediates have to fit within a signed 16-bit field.  Does nothing
 * unless the EA is a base+displacement form.  (We postpone dealing with
 * address symbols and symbol+displacement addresses until the symbol is
 * expanded into base+offset form.  For index+symbol+displacement
 * addresses, we assume that the displacement is never out of range, since
 * this form is only used for relocations.)
 */
void
ExpAlpha::expand_ea_calc(InstrHandle &ih, bool is_load)
{
    InstrHandle mih = ih;
    Instr *mi = *mih;

    BaseDispOpnd ea = (is_load ? get_src(mi, 0) : get_dst(mi));
    if (is_null(ea))
	return;						// not base+disp EA

    Integer v = get_immed_integer(ea.get_disp());

    long ival;
    TypeId val_type;
    if (v.is_c_int())
    {
	ival = (long)(v.c_int());
	val_type = type_s32;
    }
    else if (v.is_c_unsigned_int())
    {
	ival = (long)(v.c_unsigned_int());
	val_type = type_u32;
    }
    else if (v.is_c_long())
    {
	ival = (long)(v.c_long());
	val_type = type_s64;
    }
    else if (v.is_c_unsigned_long())
    {
	ival = (long)(v.c_unsigned_long());
	val_type = type_u64;
    }
    else {
	claim(false, "expand_ea_calc() -- unexpected immediate kind");
    }

    if ((ival >= MIN_15BIT) && (ival <= MAX_15BIT))
    {
	// Value is in range for 15-bit immediates.  Leave the EA alone.
	return;
    }

    // decide which temp register to use
    Opnd o_temp = use_virtual_regs
		      ? opnd_reg(type_s64)
		      : opnd_reg(ASM_TMP0, type_s64);

    // If using hard regs, we need to use a different assembler
    // temp from the routine that called us.  Make sure!
    if (is_reg(ea.get_base()) && (get_reg(o_temp) == get_reg(ea.get_base())))
    {
	claim(!use_virtual_regs);
	int next_asm_tmp = get_reg(ea.get_base()) + 1;
	claim(next_asm_tmp <= LAST_ASM_TMP);
	o_temp = opnd_reg(next_asm_tmp, type_s64);
    }

    maybe_set_noat(ih);
    Instr *mj;

    // Create a load-immediate of the big-immed and then expand it.
    mj = new_instr_alm(o_temp, LDIQ, opnd_immed(v, val_type));
    emit(ih, mj);					// ldiq $at,big_immed
    expand_ldi_immed(ih);

    mj = new_instr_alm(o_temp, ADDQ, ea.get_base(), o_temp);
    emit(ih, mj);					// addq $b,$at,$at

    // Move the original instruction past the new ones just "emitted".
    remove(mih);
    emit(ih, mi);

    // Fix its EA calculation to reflect the changes.
    ea.set_base(o_temp);
    ea.set_disp(opnd_immed_0_u64);			// zero immed field
    if (is_load)
	set_src(mi, 0, ea);
    else
	set_dst(mi, ea);
}

///////////////   ldgp expansion   ////////////////

/* expand_ldgp_instr() -- expands an ldgp instruction into a sequence
 * of ldah, lda. */
void
ExpAlpha::expand_ldgp_instr(InstrHandle &ih)
{
    Instr *mi = *ih, *mj;

    // check if we're allowed to use relocations
    if (!insert_reloc)
	return;

    // Create instruction: ldah $gp, ea!gpdisp!nnn
    // where ea is copied from original instruction.  We
    // copy notes to be sure to add "header/trailer" note.

    mj = new_instr_alm(opnd_reg_gp, LDAH, get_src(mi, 0));
    copy_notes(mi, mj);
    attach_reloc(mj, "gpdisp", reloc_seq, 0);
    insert_before(ih, mj);

    // change original ldgp instruction to lda $gp,0($gp)!gpdisp!
    set_opcode(mi, LDA);
    set_dst(mi, opnd_reg_gp);
    set_src(mi, 0, BaseDispOpnd(opnd_reg_gp, opnd_immed_0_u64));
    attach_reloc(mi, "gpdisp", reloc_seq, 0);
    reloc_seq += 1;
}

#if 0

///////////////   Absolute-value expansion   ////////////////

/* expand_abs_instr() -- inserts the appropriate instruction sequence
 * for unimplemented absolute-value operations. */
void
ExpAlpha::expand_abs_instr(Instr *mi, InstrHandle ih)
{
    debug(6,".. Expanding an abs* instruction");

    // first expand any immediates in the instruction
    expand_immed_opnds(mi, ih);

    Instr *mj;
    tree_node_list_e *tnle = mi->parent()->list_e();

    Opnd d = mi->dst_op();
    Opnd s = mi->src_op(0);
    Opnd o_r31(REG_const0, type_unsigned);

    int opcode = mi->opcode();
    int sub_opcode = (opcode == ABSL) ? SUBLV : SUBQV;

    // set dst = negate of src
    mj = new_instr_alm(sub_opcode, d, o_r31, s);		// sub*v $31,s,d
    tnl->insert_before(new tree_instr(mj), tnle);

    // check if src needed to be negated, if not overwrite with orig src
    mi->set_opcode(CMOVGT);
    mi->set_src_op(1, s);				// cmovgt s,s,d
    mi->set_num_srcs(3);
    mi->set_src_op(2, d);	/* Since d is conditionally written,
                                 * we need to make d look live up to
				 * this instruction! */

    // note that we change the original ABS* instruction
    immed_list *iml = new immed_list();
    iml->append(immed("abs instruction"));
    mi->append_comment(iml);
}


///////////////   jsr expansion   ////////////////

/* expand_jsr_instr() -- inserts the appropriate instruction sequence
 * to expand jsr instructions.
 * Does not insert a ldgp after the call because agen does that. */
void
ExpAlpha::expand_jsr_instr(Instr *mi, InstrHandle ih)
{
    // check if we're allowed to use relocations
    if (!insert_reloc) return;

    // no work needed for indirect call
    if (((mi_bj *)mi)->is_indirect()) return;
    // if k_reloc note exists, no work is needed
    if (mi->peek_annote(k_reloc)) return;

    Instr *mj;
    tree_node_list_e *tnle = mi->parent()->list_e();

    Opnd o_pv(REG(GPR,ASM_TMP,4), type_ptr);
    Opnd o_ra(REG_ra, type_ptr);

    sym_node *func_sym = ((mi_bj *)mi)->target();

    // create instruction: ldq $27, proc($gp)!literal!
    mj = new_instr_alm(LDQ, o_pv,
		   operand(New_ea_indexed_symaddr(opnd_reg_gp, func_sym, 0)));
    attach_reloc(mj, "literal", reloc_seq, 0);
    tnl->insert_before(new tree_instr(mj), tnle);

    // change jsr instruction to jsr $ra, ($27), proc!lituse_jsr!
    mi->set_dst(o_ra);
    mi->set_src_op(0, o_pv);
    mi->append_annote(k_hint, new immed_list(immed(func_sym)));
    ((mi_bj *)mi)->set_target(NULL);

    if (insert_lituse) {
	attach_reloc(mi, "lituse_jsr", reloc_seq, 0);
    }
    reloc_seq += 1;
}


///////////////   multiply expansion   ////////////////

/* Variables for expand_mul_instr
 * it constructs a sequence of steps or instructions which performs
 *  a multiplication by a constant */

typedef struct { int opcode; int s0, s1; } mult_step;
mult_step step[100];
int num_step = 0;

void
insert_step(int opcode, int s0, int s1) {
    step[num_step].op = opcode;
    step[num_step].s0 = s0;
    step[num_step].s1 = s1;
    num_step += 1;
}

enum mult_ops {
    MO_ADD,  MO_SUB,  MO_SLL, MO_S4ADD,  MO_S4SUB,  MO_S8ADD,  MO_S8SUB };
int mult_q_opcode[] = {
    ADDQ, SUBQ, SLL, S4ADDQ, S4SUBQ, S8ADDQ, S8SUBQ };
int mult_l_opcode[] = {
    ADDL, SUBL, SLL, S4ADDL, S4SUBL, S8ADDL, S8SUBL };


/* operands can be the multiplicand, the accumulator register,
 * zero, or an immediate value < 100 */
enum mult_step_oper { MSO_SRC=100, MSO_DST, MSO_ZERO };

// test whether a string pattern occurs at the end of a string
int
Match_pattern(char *bit, char *pat)
{
    int bit_len = strlen(bit);
    int pat_len = strlen(pat);
    return (!strcmp(bit+bit_len-pat_len, pat));
}

/* Generate_mult_seq() -- generate a sequence of instructions
 * which achieve a multiplication by an integer constant */
void
Generate_mult_seq(char *bits)
{
    int num_bits = strlen(bits), matched_bits = 0, x;
    if (Match_pattern(bits,"1") && num_bits==1) {
	matched_bits = 1;
    }
    else if (Match_pattern(bits,"11") && num_bits==2) {
	insert_step(MO_S4SUB, MSO_SRC, MSO_SRC);
	matched_bits = 2;
    }
    else if (Match_pattern(bits,"111") && num_bits==3) {
	insert_step(MO_S8SUB, MSO_SRC, MSO_SRC);
	matched_bits = 3;
    }
    else if (Match_pattern(bits,"011")) {
	insert_step(MO_S4SUB, MSO_DST, MSO_SRC);
	bits[num_bits-3] = '1';
	matched_bits = 2;
    }
    else if (Match_pattern(bits,"0111")) {
	insert_step(MO_S8SUB, MSO_DST, MSO_SRC);
	bits[num_bits-4] = '1';
	matched_bits = 3;
    }
    else if (Match_pattern(bits,"1111")) {
	int count=1;
	while (count<num_bits && bits[num_bits-count-1]=='1') count++;
	insert_step(MO_SUB, MSO_DST, MSO_SRC);
	insert_step(MO_SLL, MSO_DST, count);
	bits[num_bits-count-1] = '1';
	matched_bits = count;
    }

    else if (Match_pattern(bits,"10")) {
	insert_step(MO_ADD, MSO_DST, MSO_DST);
	matched_bits = 1;
    }
    else if (Match_pattern(bits,"100")) {
	insert_step(MO_S4ADD, MSO_DST, MSO_ZERO);
	matched_bits = 2;
    }
    else if (Match_pattern(bits,"1000")) {
	insert_step(MO_S8ADD, MSO_DST, MSO_ZERO);
	matched_bits = 3;
    }
    else if (Match_pattern(bits,"0000")) {
	int count=1;
	while (bits[num_bits-count-1]=='0') count++;
	insert_step(MO_SLL, MSO_DST, count);
	matched_bits = count;
    }

    else if (Match_pattern(bits,"101")) {
	insert_step(MO_S4ADD, MSO_DST, MSO_SRC);
	matched_bits = 2;
    }
    else if (Match_pattern(bits,"1001")) {
	insert_step(MO_S8ADD, MSO_DST, MSO_SRC);
	matched_bits = 3;
    }
    else if (Match_pattern(bits,"0001")) {
	int count = 1;
	while (count<num_bits && bits[num_bits-count-1]=='0') count++;
	insert_step(MO_ADD, MSO_DST, MSO_SRC);
	insert_step(MO_SLL, MSO_DST, count);
	matched_bits = count;
    }

    assert_msg((matched_bits>0),("constant multiplication: "
				 "no rule applies! (bug in aexp.cc)"));

    // delete bits that have been processed
    for (x=num_bits-1; x>=num_bits-matched_bits; x--)
	bits[x] = 0;
    // if any bits are left, process them.
    if (num_bits-matched_bits > 0)
	Generate_mult_seq(bits);
}


void
ExpAlpha::expand_mul_instr(Instr *mi, InstrHandle ih)
{
    Instr *mj;
    tree_node_list_e *tnle = mi->parent()->list_e();
    Opnd o_src0 = mi->src_op(0);
    Opnd o_src1 = mi->src_op(1);
    Opnd o_dst = mi->dst_op();
    Opnd o_zero = operand(REG_const0, type_unsigned);
    bool is_quad = ((int)mi->opcode() == MULQ);

    // which of the two source operands is a constant int ?
    int value = 0, value0, value1;
    bool src0_con = o_src0.is_const_int(&value0);
    bool src1_con = o_src1.is_const_int(&value1);
    // proceed if one of the source operands is a constant
    if (!src0_con && !src1_con) return;
    // complain if both source operands are constants
    assert(!src0_con || !src1_con);
    value = src0_con ? value0 : value1;

    // start with empty sequence of instructions
    num_step = 0;
    // if value < 0, multiply by abs(value) and then negate the result
    if (value < 0) {
	value = -value;
	insert_step(MO_SUB, MSO_ZERO, MSO_DST);
    }

    // convert value to bit sequence and count 1s
    char bits[40];
    int bit_pos, tmp_val, num_bit, num_ones=0;
    for (num_bit=0; (1<<num_bit)<=value; num_bit++) ;
    for (tmp_val=value, bit_pos=num_bit-1;
	 tmp_val > 0;
	 tmp_val >>= 1, bit_pos--) {
	bits[bit_pos] = (tmp_val & 1) ? '1' : '0';
	num_ones += (tmp_val & 1);
    }
    bits[num_bit] = 0;

    // If the value is too big and would require too many steps,
    // use a multiply machine instruction.  This requires moving
    // the large immediate into a register.
    if ((value > 0x1000f) && (num_ones > 6) && (num_bit-num_ones > 6)) {
	expand_opr_immed(mi, tnl, (src0_con ? 0 : 1) );
	return;
    }

    // Ok, we actually want to do the multiply in a sequence of steps.
    if (o_src0.is_immed()) o_src0.instr()->remove();
    if (o_src1.is_immed()) o_src1.instr()->remove();

    // Load the non-constant source operand into a temporary register.
    Opnd o_noncon = src0_con ? mi->src_op(1) : mi->src_op(0);
    Opnd o_srcval = operand((use_virtual_regs ? NEW_VREG : REG(GPR,ASM_TMP,0)),
		       type_signed_long);

    if (insert_set_noat && !noat_is_set) {
	// specify that we're going to use the $at register
	mj = new mi_xx(SET, immed("noat"));
	tnl->insert_before(new tree_instr(mj), tnle);
	noat_is_set = true;
    }

    // initialize
    mj = new_instr_alm(MOV, o_srcval, o_noncon);
    tnl->insert_before(new tree_instr(mj), tnle);

    Generate_mult_seq(bits);

    if (step[num_step-1].s0==MSO_DST) step[num_step-1].s0 = MSO_SRC;
    if (step[num_step-1].s1==MSO_DST) step[num_step-1].s1 = MSO_SRC;

    for (int x=num_step-1; x>=0; x--) {
	Opnd step_src0, step_src1;
	int step_opcode = is_quad ? mult_q_opcode[step[x].op] :
	    mult_l_opcode[step[x].op];

	switch (step[x].s0) {
	  case MSO_SRC:
	    step_src0 = o_srcval; break;
	  case MSO_DST:
	    step_src0 = o_dst; break;
	  case MSO_ZERO:
	    step_src0 = o_zero; break;
	  default:
	    assert_msg(false,("multiplication step has wrong src0"));
	}

	switch (step[x].s1) {
	  case MSO_SRC:
	    step_src1 = o_srcval; break;
	  case MSO_DST:
	    step_src1 = o_dst; break;
	  case MSO_ZERO:
	    step_src1 = o_zero; break;
	  default:
	    immed ival(step[x].s1);
	    step_src1 = Immed_operand(ival, type_unsigned);
	}

	mj = new_instr_alm(step_opcode, o_dst, step_src0, step_src1);
	tnl->insert_before(new tree_instr(mj), tnle);
    }

    // delete the original mul instruction
    tnl->remove(tnle);
    delete mi->parent();

    // upon return, original mi parameter is a bad pointer
}


///////////////   Expanding large immediates   /////////////////////

/* expand_immed_opnds() -- Finds immediate values in operands that
 * don't fit into the instruction's immediate field and replaces them
 * with register operands.  The ldil and ldiq instructions have a signed
 * immediate field of 16-bits while all other integer operate instructions
 * have only an unsigned 8-bit immediate field. */
void
ExpAlpha::expand_immed_opnds(Instr *mi, InstrHandle ih)
{
    unsigned s, src_idx;
    bool found = false;

    // determine if mi has a 'simple' immediate operand
    for (s = 0; s < mi->num_srcs(); s++) {
	Opnd o_src = mi->src_op(s);
	if (o_src.is_immed() && !Is_ea_operand(o_src)) {
	    assert_msg(!found, ("expand_immed_opnds() -- "
				"found instruction with multiple immediates"));
	    src_idx = s;
	    found = true;
	}
    }

    int opcode = mi->opcode();
    if (found) {
	if ((opcode == LDIL) || (opcode == LDIQ) || (opcode == LDIF)
	||  (opcode == LDID) || (opcode == LDIG) || (opcode == LDIS)
	||  (opcode == LDIT)) {
	    assert(src_idx == 0);
	    expand_ldi_immed(mi, tnl);
	} else {
	    expand_opr_immed(mi, tnl, src_idx);
	}
    }
}


void
ExpAlpha::expand_opr_immed(Instr *mi, InstrHandle ih, int src_idx)
{
    long ival;
    Opnd o_src = mi->src_op(src_idx);
    assert(o_src.is_immed());
    if (o_src.immediate().is_integer()) {
	ival = (long)(o_src.immediate().integer());
    } else if (o_src.immediate().is_unsigned_int()) {
	ival = (long)(o_src.immediate().unsigned_int());
    } else if (o_src.immediate().is_long_int()) {
	ival = (long)(o_src.immediate().long_int());
    } else if (o_src.immediate().is_unsigned_long()) {
	ival = (long)(o_src.immediate().unsigned_long());
    } else {
	assert_msg(false, ("Expand_opr_immed() -- "
			   "unexpected immediate type (%c)",
			   o_src.immediate().kind()));
    }
    TypeId ival_type = (o_src.instr())->result_type();

    if ((ival >= MIN_u8BIT) && (ival <= MAX_u8BIT)) {
	// Value is in range for unsigned 8-bit immediates.  Leave it in the
	// instruction as an immediate operand.
	//
	// Since we're unsigned here, SUIF will never use too big
	// representation to hold the immediate value.
	return;
    }

    debug(4,".. Expanding an operate immediate (%d)", ival);

    // Decide which temp register to use.
    Opnd o_temp;
    tree_node_list_e *tnle = mi->parent()->list_e();

    o_temp = operand((use_virtual_regs ? NEW_VREG : REG(GPR,ASM_TMP,0)),
		     type_signed_long);
    if (insert_set_noat && !noat_is_set) {
	// specify that we're going to use the $at register
	Instr *mj = new mi_xx(SET, immed("noat"));
	tnl->insert_before(new tree_instr(mj), tnle);
	noat_is_set = true;
    }

    // Put the immediate into a ldiq instruction and then expand
    // that instruction.
    instruction *lit_ldc = mi->src_op(src_idx).instr();
    lit_ldc->remove();
    int ldi_opcode;
    if (ival_type->size() == SIZE_OF_WORD)
	ldi_opcode = LDIL;
    else
	ldi_opcode = LDIQ;
    Instr *mj = new_instr_alm(ldi_opcode, o_temp, operand(lit_ldc));
    tnl->insert_before(new tree_instr(mj), tnle);
    expand_ldi_immed(mj, tnl);

    // Replace immediate src operand with o_temp
    mi->set_src_op(src_idx, o_temp);
}

#endif /* 0 */

void
ExpAlpha::expand_ldi_immed(InstrHandle &ih)
{
    InstrHandle mih = ih;
    Instr *mi = *mih;

    bool is_integer, is_binary_integer;
    long ival;
    const char *ival_string, *fval;
    Opnd o_src = get_src(mi, 0);
    TypeId val_type;

    if (is_immed_integer(o_src))
    {
	is_integer = is_binary_integer = true;
	Integer v = get_immed_integer(o_src);
	if (v.is_c_int())
	{
	    ival = (long)(v.c_int());
	    val_type = type_s32;
	}
	else if (v.is_c_unsigned_int())
	{
	    ival = (long)(v.c_unsigned_int());
	    val_type = type_u32;
	}
	else if (v.is_c_long())
	{
	    ival = (long)(v.c_long());
	    val_type = type_s64;
	}
	else if (v.is_c_unsigned_long())
	{
	    ival = (long)(v.c_unsigned_long());
	    val_type = type_u64;
	}
	else if (v.is_c_string_int())
	{
	    is_binary_integer = false;
	    ival_string = v.c_string_int();
	    val_type = v.is_negative() ? type_s64 : type_u64;
	}
	else
	{
	    claim(false, "expand_ldi_immed() -- "
		         "unexpected integer-immediate kind");
	}
    }
    else if (is_immed_string(o_src) &&
	     is_floating_point(val_type = get_type(o_src)))
    {
	is_integer = is_binary_integer = false;
	fval = get_immed_string(o_src).chars();
    }
    else
    {
	claim(false, "expand_ldi_immed() -- unexpected immediate kind");
    }

    if (is_binary_integer && (ival >= MIN_15BIT) && (ival <= MAX_15BIT)) {
	// Value is in range for 15-bit immediates.  Leave it in the
	// instruction as an immediate operand.

	if ((val_type == type_s64) || (val_type == type_u64))
	    // Big representation isn't necessary; use compact representation
	    // to eliminate the annoying assembler warning messages.
	    set_src(mi, 0, opnd_immed(ival, type_s16));
	return;
    }

    if (is_binary_integer)
	debug(4,".. Expanding an integer ldi immediate (%d)", ival);
    else if (is_integer)
	debug(4,".. Expanding an integer ldi immediate (%s)", ival_string);
    else
	debug(4,".. Expanding a FP ldi immediate (%s)", fval);

    Instr *mj;

    // Decide which temp register to use.  If the destination operand is a
    // writeable hard register (i.e. not CONST0), use it.  Otherwise, use a
    // vreg or $at.  Note that the FP literal expansion needs only an
    // integer temporary register to hold the address of the literal in
    // memory.
    Opnd o_temp;
    if (is_hard_reg(get_dst(mi)) && (get_reg(get_dst(mi)) != CONST0_GPR))
	o_temp = get_dst(mi);
    else
	o_temp = use_virtual_regs
		     ? opnd_reg(type_u64)
		     : opnd_reg(ASM_TMP0, type_u64);
    maybe_set_noat(ih);

    if (is_binary_integer && (ival >= MIN_31BIT) && (ival <= MAX_31BIT))
    {
	// value can be assembled in two instructions
	long hi = ival >> 16, lo = ival & 0x0000ffff;
	bool add_one_to_hi = false;

	// Now, lo can only be positive given the AND-operation above.
	// Below, we'll want the signed 16-bit representation.  Because
	// lo is signed and combined with hi via an add, we need to adjust
	// hi if lo is negative to account for the carry.  Note that hi
	// is always signed because ival was signed.
	if (lo > MAX_15BIT) {
	    lo -= 65536;	// make lo signed
	    hi += 1;		// adjust for carry
	}
	if (hi > MAX_15BIT) {
	    // Ooops, cannot add in extra one since hi then overflows.
	    // Instead, add the one in explicitly in the instruction stream.
	    hi -= 1;
	    add_one_to_hi = true;
	}

	assert(hi != 0);	// if so, fit into 16-bit immediate!

	// create ldah
	Opnd o_reg0 = opnd_reg(CONST0_GPR, type_u32);
	mj = new_instr_alm(o_temp, LDAH,
			   BaseDispOpnd(o_reg0, opnd_immed(hi, type_s32)));
	emit(ih, mj);

	if (lo != 0)
	{
	    // create lda (add without exception on overflow)
	    mj = new_instr_alm(o_temp, LDA,
			       BaseDispOpnd(o_temp, opnd_immed(lo, type_s32)));
	    emit(ih, mj);
	}
	if (add_one_to_hi)
	{
	    mj = new_instr_alm(o_temp, LDAH,
			       BaseDispOpnd(o_temp, opnd_immed_1_u64));
	    emit(ih, mj);
	}
	// Remember mj for ldi optimization below!
    }
    else
    {
	// Immediate is too large.  We just put it in the .lit4/.lit8
	// area and load it in when needed.  For integer literals, we
	// fill the literal area with unique values.  To do this, we
	// follow a naming convention for the VarSyms destined for the
	// integer literal area.  There is no support in basesuif for
	// the specification of literal constants.  We don't do any
	// such optimization for the FP literal values.
	int lit_size = get_bit_size(val_type);

	// generate stylized var_sym name
	char lit_name[35];
	if (is_binary_integer) {
	    sprintf(lit_name, "__lit8_0x%lx", ival);
	} else if (is_integer) {
	    sprintf(lit_name, "__lit8_%s", ival_string);
	} else {
	    sprintf(lit_name, "__lit%d_%cpfp", lit_size >> 3,
		    (lit_size == 32) ? 's' : 'd');
	}

	// FIXME:  should be in file symbol table, not local
	VarSym *lit_var = new_unique_var(o_src, lit_name);

	// Build relocatable load instruction to get value out of lit*
	// area.  We reuse code in expand_memory_instr().
	int ld_opcode;
	switch (get_opcode(mi)) {
	  case LDIL:
	    ld_opcode = LDL; break;
	  case LDIQ:
	    ld_opcode = LDQ; break;
	  case LDIS:			// ieee sp
	    ld_opcode = LDS; break;
	  case LDIT:			// ieee fp
	    ld_opcode = LDT; break;
	  case LDIF:
	    ld_opcode = LDF; break;
	  case LDID: case LDIG:
	    ld_opcode = LDG; break;
	  default:
	    claim(false, "expand_ldi_immed() -- "
			 "unexpected opcode: %s", opcode_name_alpha(ld_opcode));
	}
	mj = new_instr_alm(o_temp, ld_opcode, opnd_addr_sym(lit_var));
	emit(ih, mj);
	expand_memory_instr(ih, IS_LOAD);

	// Remember mj for ldi optimization below!
    }

    // The last instruction from the expansion can take the place
    // of the ldi* instruction.  To accomplish this, we simply replace
    // the destination of the last expanded instruction with the
    // destination of the ldi*.
    assert(mj);
    set_dst(mj, get_dst(mi));

    // Then we delete the original instruction mi, now unnecessary.
    delete remove(mih);
}

void
ExpAlpha::finalize()
{ 
    set_note(the_file_block, k_next_free_reloc_num, OneNote<long>(reloc_seq));
}

void
ExpAlpha::emit(InstrHandle &ih, Instr *mi)
{
    insert_after(ih, mi);
    ++ih;
}

/*
 * Emit a `.set noat' directive if one is needed.
 */
void
ExpAlpha::maybe_set_noat(InstrHandle &ih)
{
    if (insert_set_noat && !noat_is_set)
    {
	emit(ih, new_instr_dot(SET, opnd_immed("noat")));
	noat_is_set = true;
    }
}
