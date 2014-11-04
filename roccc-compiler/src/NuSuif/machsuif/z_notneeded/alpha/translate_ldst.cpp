/* file "alpha/translate_ldst.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#include <machine/machine.h>
#include <suifvm/suifvm.h>

#include <alpha/init.h>
#include <alpha/opcodes.h>
#include <alpha/code_gen.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace alpha;

// size is expressed in bits
#define MAX_SIZE_FOR_SMALL_STRUCT_MOVE 1024

void
CodeGenAlpha::translate_lod(Instr *mi)
{
    Opnd d = get_dst(mi);
    TypeId dtype = get_type(d);

    if (!is_scalar(dtype)) {
	// For structure loads, we just stack the instruction for later
	// translation.  We emit it after we see if the data is used in a
	// store or call.
	struct_lds.push_back(mi);
    } else {
	set_opcode(mi, opcode_load(dtype));
	emit(mi);
    }
}

void
CodeGenAlpha::translate_str(Instr *mi)
{
    Opnd s = get_src(mi, 0);
    TypeId stype = get_type(s);

    if (!is_scalar(stype)) {
	// find corresponding structure load
	Instr *mi_ld = NULL;
	for (InstrHandle h = struct_lds.begin(); h != struct_lds.end(); ++h)
	    if (get_dst(*h) == s) {
		mi_ld = *h;
		struct_lds.erase(h);
		break;
	    }
	claim(mi_ld);

	expand_struct_move(mi_ld, mi);

    } else {
	set_opcode(mi, opcode_store(stype));
	emit(mi);
    }
}


void
CodeGenAlpha::translate_memcpy(Instr *mi)
{
    Opnd s = get_src(mi, 0);
    Opnd d = get_dst(mi);

    claim(is_addr(s) && is_addr(d));

    TypeId rtype = get_deref_type(s);
    claim(!is_void(rtype));

    Opnd v = opnd_reg(rtype);
    
    if (is_scalar(rtype)) {
	// simple load and store
	Instr *mj;
	mj = new_instr_alm(v, opcode_load(rtype), s);
	emit(mj);
	mj = new_instr_alm(d, opcode_store(rtype), v);
	copy_notes(mi, mj);
	emit(mj);

    } else if (is_array(rtype) || is_record(rtype)) {
	// structure move
	Instr *mi_ld, *mi_st;
	mi_ld = new_instr_alm(v, suifvm::LOD, s);
	mi_st = new_instr_alm(d, suifvm::STR, v);
	copy_notes(mi, mi_st);
	expand_struct_move(mi_ld, mi_st);

    } else
	claim(false, "unexpected type for memcpy instr");

    delete mi;
}


/*
 *  SUIF allows structures to have arbitrary alignment and
 *  size.  This is necessary to be compatible with back end compilers
 *  like gcc that use rules for structures.  It's also generally a
 *  good thing -- if you have a structure containing only single
 *  character field, or even a union of a character and a short int,
 *  it's wasteful to require word alignment and padding.
 *
 *  So we need to implement structure copying that allows for
 *  arbitrary sizes and alignment restrictions.  But we still want
 *  to be able to use large load word/store word instructions whenever
 *  possible for efficiency.  Hence we need to know when a
 *  particular structure has word or even halfword alignment; most
 *  structures, especially the larger ones likely contain at least
 *  some ``int'' or ``float'' fields, or something else with word
 *  alignment, so the common case should be structures with word
 *  alignment.
 *
 *  In SUIF we can always get the alignment restrictions from
 *  the type information of the structure.  Using the alignment
 *  information, we break the structure up into chunks with whichever
 *  of double-words, words, halfwords, or bytes fits the alignment.
 *  Break off as many chunks of that size as possible, then move to
 *  the next smaller size.  This gives an unlimited number of chunks
 *  of the largest allowable size, and then either one or zero each of
 *  the smaller ones.
 *
 *  Now if there's just one chunk, just change the opcode for the
 *  load/store instructions, and we're done.  Otherwise, do as many
 *  as possible up to the point where we might have to deal with
 *  the leftover chunks.
 *
 *  As it turns out, for double-words, words, and halfwords of eight,
 *  four, and two bytes respectively, the total number of chunks is:
 *
 *	struct_size / struct_alignment + leftover
 *
 *	    where
 *
 *	leftover = 0   if   struct_size % struct_alignment == 0
 *		 = 1   if   struct_size % struct_alignment == 1, 2, or 4
 *		 = 2   if   struct_size % struct_alignment == 3, 5, or 6
 *               = 3   if   struct_size % struct_alignment == 7
 *
 *  (struct_size and struct_alignment are in bytes)
 *
 *  Note that the expression for ``leftover'' reduces to:
 *
 *	if (struct_size % struct_alignment < 4)
 *        leftover = (struct_size % struct_alignment + 1) / 2
 *	else
 *        leftover = ((struct_size % struct_alignment - 3) / 2) + 1
 *
 *  Now we can do all but the last three chunks knowing that they
 *  all will consist of the largest chunk size that fits the
 *  alignment.  Then we just have to handle the last few chunks
 *  separately.
 *
 *  Specifics for Alpha 21064: hardware has two load delay slots so
 *  transfer in chunks of three. (This will work also fine for 21164
 *  with only a single load delay slot.)  Note that we must do things
 *  in chunks of three since the leftover (which must be handled
 *  separately) in a 64-bit datapath can be up to three 'chunks' large.
 */

/* expand_struct_move() - performs the block copy of a structure by using
 * the information in a lr and sr code generator pseudo op. It also assumes
 * that the destination reg of the load record instr is a virtual reg that
 * lives in a temporary reg, i.e. not in a parameter or result reg. */
void
CodeGenAlpha::expand_struct_move(Instr *lr, Instr *sr)
{
    Opnd lr_d = get_dst(lr);
    claim(is_virtual_reg(lr_d));
    claim(get_reg(lr_d) == get_reg(get_src(sr, 0)));

    TypeId rec_type = get_type(lr_d);
    int struct_size = get_bit_size(rec_type);
    int struct_alignment = get_bit_alignment(rec_type);

    if (struct_alignment > 64)
	struct_alignment = 64;
    else if (struct_alignment == 0)
	struct_alignment = 8;

    claim((struct_alignment == 8) || (struct_alignment == 16) ||
	  (struct_alignment == 32) || (struct_alignment == 64));

    // do block copy of structure from source to sink
    if (((struct_size == 64) || (struct_size == 32)
    || (struct_size == 16) || (struct_size == 8))
    && (struct_alignment >= struct_size)) {
	// simple ldq and stq, ldl and stl, ldw and stw, or ldb and stb --
	// just change opcodes and type of data register (i.e. cannot
	// transfer a structure in a register)
	TypeId mv_type;

	switch (struct_size) {
	  case 64:
	    set_opcode(lr, LDQ);
	    set_opcode(sr, STQ);
	    mv_type = type_s64;
	    break;
	  case 32:
	    set_opcode(lr, LDL);
	    set_opcode(sr, STL);
	    mv_type = type_s32;
	    break;
	  case 16:
	    set_opcode(lr, LDW);
	    set_opcode(sr, STW);
	    mv_type = type_s16;
	    break;
	  case 8:
	    set_opcode(lr, LDB);
	    set_opcode(sr, STB);
	    mv_type = type_s8;
	    break;
	  default:
	    claim(false);
	}
	// set generic types for move
	Opnd v = opnd_reg(mv_type);
	Opnd lea = clone(get_src(lr, 0));
	if (is_addr_exp(lea)) {
	    set_deref_type(lea, mv_type);
	    set_src(lr, 0, lea);
	}
	set_dst(lr, 0, v);
	emit(lr);

	Opnd sea = clone(get_dst(sr, 0));
	if (is_addr_exp(sea)) {
	    set_deref_type(sea, mv_type);
	    set_src(sr, 0, sea);
	}
	set_src(sr, 0, v);
	emit(sr);

    } else {
	// create placeholder for structure move annotations
	Instr *mi = new_instr_alm(opcode_null);

	ListNote<IdString> begin_note;
	begin_note.append(IdString("BEGIN structure move"));
	set_note(mi, k_comment, begin_note);

	copy_notes(lr, mi);
	copy_notes(sr, mi);
	emit(mi);

	// determine what kind of code to generate
	if (struct_size > MAX_SIZE_FOR_SMALL_STRUCT_MOVE)
	    do_big_struct_move(lr, sr, struct_alignment, struct_size);
	else
	    do_small_struct_move(lr, sr, struct_alignment, struct_size);

	// mark end of the structure move
	mi = new_instr_alm(opcode_null);

	ListNote<IdString> end_note;
	end_note.append(IdString("END structure move"));
	set_note(mi, k_comment, end_note);

	emit(mi);
    }
}


/* do_small_struct_move() -- straight-line code to do structure move.
 * Move chunks in sets of 3 ld/st pairs.  Handle ending cases separately.
 * Can have either 2, 3, 4, or 5 pairs left at end. 
 *
 * This code reuses the lr and sr instruction containers.  Do not
 * delete these heap objects. */
void
CodeGenAlpha::do_small_struct_move(Instr *lr, Instr *sr,
				   int struct_alignment, int struct_size)
{
    Instr *mi;
    Opnd vr0, vr1, vr2, vr3, vr4;

    int bits_in_chunk = struct_alignment;
    int bytes_in_chunk = bits_in_chunk >> 3;

    // shift struct_<<stuff>> from bits to bytes
    int struct_offset = 0;
    struct_size = struct_size >> 3;
    struct_alignment = struct_alignment >> 3;

    int leftover = struct_size % struct_alignment;
    if (leftover < 4) leftover = (leftover + 1) / 2;
    else leftover = ((leftover - 3) / 2) + 1;
    int num_chunks = struct_size / struct_alignment + leftover;
    int num_pairs = (num_chunks - 3) / 3;

    TypeId mv_type;
    int chunk_load_op, chunk_store_op;
    switch (bits_in_chunk) {
      case 64:
	chunk_load_op = LDQ;
	chunk_store_op = STQ;
	mv_type = type_s64;
	break;
      case 32:
	chunk_load_op = LDL;
	chunk_store_op = STL;
	mv_type = type_s32;
	break;
      case 16:
	chunk_load_op = LDW;
	chunk_store_op = STW;
	mv_type = type_s16;
	break;
      case 8:
	chunk_load_op = LDB;
	chunk_store_op = STB;
	mv_type = type_s8;
	break;
      default:
	claim(false);
    }

    // turn structure load into lda for structure load base register
    Opnd vrldb_opd = opnd_reg(type_addr());
    set_opcode(lr, alpha::LDA);
    set_dst(lr, vrldb_opd);
    maybe_use_reg_gp(lr);	// add regs_used note for $gp if needed
    emit(lr);

    // turn structure store into lda for struct store base reg
    Opnd vrstb_opd = opnd_reg(type_addr());
    Opnd ea = get_dst(sr);

    set_opcode(sr, alpha::LDA);
    set_dst(sr, vrstb_opd);
    set_src(sr, 0, ea);
    maybe_use_reg_gp(sr);	// add regs_used note for $gp if needed
    emit(sr);

    // loop to create most of ld/st's -- 3 at a time
    for (int pair_num = 0; pair_num < num_pairs; ++pair_num) {
	vr0 = opnd_reg(mv_type);
	vr1 = opnd_reg(mv_type);
	vr2 = opnd_reg(mv_type);

	// do next 3 transfers
	struct_offset = pair_num * bytes_in_chunk * 3;
	int so0 = struct_offset;
	Opnd so0_opd = opnd_immed(so0, type_s64);
	int so1 = struct_offset + bytes_in_chunk;
	Opnd so1_opd = opnd_immed(so1, type_s64);
	int so2 = struct_offset + (2 * bytes_in_chunk);
	Opnd so2_opd = opnd_immed(so2, type_s64);

	// 3 loads
	mi = new_instr_alm(vr0, chunk_load_op,
			   BaseDispOpnd(vrldb_opd, so0_opd));
	emit(mi);
	mi = new_instr_alm(vr1, chunk_load_op,
			   BaseDispOpnd(vrldb_opd, so1_opd));
	emit(mi);
	mi = new_instr_alm(vr2, chunk_load_op,
			   BaseDispOpnd(vrldb_opd, so2_opd));
	emit(mi);

	// 3 stores
	mi = new_instr_alm(BaseDispOpnd(vrstb_opd, so0_opd),
			   chunk_store_op, vr0);
	emit(mi);
	mi = new_instr_alm(BaseDispOpnd(vrstb_opd, so1_opd),
			   chunk_store_op, vr1);
	emit(mi);
	mi = new_instr_alm(BaseDispOpnd(vrstb_opd, so2_opd),
			   chunk_store_op, vr2);
	emit(mi);
    }

    struct_offset = num_pairs * bytes_in_chunk * 3;

    // up to 5 remaining memory move operations
    int load_op[5], store_op[5];
    int num_ops = 0;
    int chunk_byte_sizes[5];

    int remaining_bytes = struct_size - struct_offset;
    for (; bytes_in_chunk > 0; bytes_in_chunk >>= 1) {
	while (remaining_bytes >= bytes_in_chunk) {
	    chunk_byte_sizes[num_ops] = bytes_in_chunk;
	    ++num_ops;
	    remaining_bytes -= bytes_in_chunk;
	}
    }

    claim((num_ops == 2) || (num_ops == 3) ||
	   (num_ops == 4) || (num_ops == 5));

    for (int op_num = 0; op_num < num_ops; ++op_num) {
	switch (chunk_byte_sizes[op_num]) {
	  case 8:
	    load_op[op_num] = LDQ;
	    store_op[op_num] = STQ;
	    break;
	  case 4:
	    load_op[op_num] = LDL;
	    store_op[op_num] = STL;
	    break;
	  case 2:
	    load_op[op_num] = LDW;
	    store_op[op_num] = STW;
	    break;
	  case 1:
	    load_op[op_num] = LDB;
	    store_op[op_num] = STB;
	    break;
	  default:
            claim(false);
	}
    }

    // Pattern is 3 loads -- 2 stores -- 2 loads -- 3 stores
    // or 3-1-1-3 or 3-3 or 2-2.  Since always start with at
    // least 2 loads, do this then specialize.
    vr0 = opnd_reg(mv_type);
    vr1 = opnd_reg(mv_type);
    int fso0 = struct_offset;
    Opnd fso0_opd = opnd_immed(fso0, type_s64);
    int fso1 = struct_offset + chunk_byte_sizes[0];
    Opnd fso1_opd = opnd_immed(fso1, type_s64);

    // 2 loads
    mi = new_instr_alm(vr0, load_op[0], BaseDispOpnd(vrldb_opd, fso0_opd));
    emit(mi);
    mi = new_instr_alm(vr1, load_op[1], BaseDispOpnd(vrldb_opd, fso1_opd));
    emit(mi);

    if (num_ops == 2) {
	// 2 stores
	mi = new_instr_alm(BaseDispOpnd(vrstb_opd, fso0_opd), store_op[0], vr0);
	emit(mi);
	mi = new_instr_alm(BaseDispOpnd(vrstb_opd, fso1_opd), store_op[1], vr1);
	emit(mi);

    } else {
	// common pattern is now 3 loads and at least 1 store
	vr2 = opnd_reg(mv_type);
	int fso2 = struct_offset + chunk_byte_sizes[0] + chunk_byte_sizes[1];
	Opnd fso2_opd = opnd_immed(fso2, type_s64);

	// 1 more load
	mi = new_instr_alm(vr2, load_op[2], BaseDispOpnd(vrldb_opd, fso2_opd));
	emit(mi);

	// 1 store
	mi = new_instr_alm(BaseDispOpnd(vrstb_opd, fso0_opd), store_op[0], vr0);
	emit(mi);

	// finish off the specialization
	if (num_ops == 3) {
	    // 2 stores
	    mi = new_instr_alm(BaseDispOpnd(vrstb_opd, fso1_opd),
			       store_op[1], vr1);
	    emit(mi);
	    mi = new_instr_alm(BaseDispOpnd(vrstb_opd, fso2_opd),
			       store_op[2], vr2);
	    emit(mi);

	} else if (num_ops == 4) {
	    vr3 = opnd_reg(mv_type);
	    int fso3 = struct_offset + chunk_byte_sizes[0]
		+ chunk_byte_sizes[1] + chunk_byte_sizes[2];
	    Opnd fso3_opd = opnd_immed(fso3, type_s64);

	    // 1 load
	    mi = new_instr_alm(vr3, load_op[3],
			       BaseDispOpnd(vrldb_opd, fso3_opd));
	    emit(mi);

	    // 3 stores
	    mi = new_instr_alm(BaseDispOpnd(vrstb_opd, fso1_opd),
			       store_op[1], vr1);
	    emit(mi);
	    mi = new_instr_alm(BaseDispOpnd(vrstb_opd, fso2_opd),
			       store_op[2], vr2);
	    emit(mi);
	    mi = new_instr_alm(BaseDispOpnd(vrstb_opd, fso3_opd),
			       store_op[3], vr3);
	    emit(mi);

	} else if (num_ops == 5) {
	    vr3 = opnd_reg(mv_type);
	    vr4 = opnd_reg(mv_type);
	    int fso3 = struct_offset + chunk_byte_sizes[0]
		+ chunk_byte_sizes[1] + chunk_byte_sizes[2];
	    Opnd fso3_opd = opnd_immed(fso3, type_s64);
	    int fso4 = fso3 + chunk_byte_sizes[3];
	    Opnd fso4_opd = opnd_immed(fso4, type_s64);

	    // another store
	    mi = new_instr_alm(BaseDispOpnd(vrstb_opd, fso1_opd),
			       store_op[1], vr1);
	    emit(mi);

	    // 2 loads
	    mi = new_instr_alm(vr3, load_op[3],
			       BaseDispOpnd(vrldb_opd, fso3_opd));
	    emit(mi);
	    mi = new_instr_alm(vr4, load_op[4],
			       BaseDispOpnd(vrldb_opd, fso4_opd));
	    emit(mi);

	    // 3 stores
	    mi = new_instr_alm(BaseDispOpnd(vrstb_opd, fso2_opd),
			       store_op[2], vr2);
	    emit(mi);
	    mi = new_instr_alm(BaseDispOpnd(vrstb_opd, fso3_opd),
			       store_op[3], vr3);
	    emit(mi);
	    mi = new_instr_alm(BaseDispOpnd(vrstb_opd, fso4_opd),
			       store_op[4], vr4);
	    emit(mi);
	}
    }
}


/* do_big_struct_move() -- looping code to do structure move.  Move
 * code in three quad-word chunks at a time.  Handle alignment cases
 * at end carefully!
 *
 * Note that the Alpha compiler seems to generate the same .comm
 * code as we do for these big structures, and they always assume
 * that .comm structures are double-word aligned.  I'll make the
 * same assumption and remove the need for any pre-alignment checks
 * or code. 
 *
 * This code reuses the lr and sr instruction containers.  Do not
 * delete these heap objects. */
void
CodeGenAlpha::do_big_struct_move(Instr *lr, Instr *sr,
				 int struct_alignment, int struct_size)
{
    Instr *mi;
    Opnd vr0, vr1, vr2;	 	// transfer registers

    // warning about alignment
    if (struct_alignment != 64)
	warn("Assuming large structure is double-word aligned");

    // calculate what needs to be done
    struct_size = struct_size >> 3;
    int quadword_leftover = struct_size & 7;
    struct_size -= quadword_leftover;
    struct_size = struct_size >> 3;		// bytes to quadwords
    int loop_leftover = struct_size % 3;
    struct_size -= loop_leftover;
    int loop_cnt = struct_size / 3;

    // create init code for loop

    // turn structure load into lda for structure load base register
    Opnd vrldb_opd = opnd_reg(type_addr());
    set_opcode(lr, alpha::LDA);
    set_dst(lr, vrldb_opd);
    maybe_use_reg_gp(lr);	// add regs_used note for $gp if needed
    emit(lr);

    // turn structure store into lda for struct store base reg
    Opnd vrstb_opd = opnd_reg(type_addr());
    Opnd ea = get_dst(sr);

    set_opcode(sr, alpha::LDA);
    set_dst(sr, vrstb_opd);
    set_src(sr, 0, ea);
    maybe_use_reg_gp(sr);	// add regs_used note for $gp if needed
    emit(sr);

    // init loop index reg
    Opnd vr_i_opd = opnd_reg(type_u64);
    Opnd lc_opd = opnd_immed(loop_cnt, type_u64);
    mi = new_instr_alm(vr_i_opd, LDIQ, lc_opd);
    emit(mi);

    // core of the loop -- ldq/stq x3 and pointer updates

    // create loop label
    LabelSym *ll = new_unique_label(the_suif_env, cur_unit);
    mi = new_instr_label(ll);
    emit(mi);

    vr0 = opnd_reg(type_u64);
    vr1 = opnd_reg(type_u64);
    vr2 = opnd_reg(type_u64);

    // 3 quadword loads
    mi = new_instr_alm(vr0, LDQ,
		       BaseDispOpnd(vrldb_opd, opnd_immed_0_u64, type_u64));
    emit(mi);
    mi = new_instr_alm(vr1, LDQ, 
		       BaseDispOpnd(vrldb_opd, opnd_immed_8_u64, type_u64));
    emit(mi);
    mi = new_instr_alm(vr2, LDQ, 
		       BaseDispOpnd(vrldb_opd, opnd_immed_16_u64, type_u64));
    emit(mi);

    // 3 quadword stores
    mi = new_instr_alm(BaseDispOpnd(vrstb_opd, opnd_immed_0_u64, type_u64),
		       STQ, vr0);
    emit(mi);
    mi = new_instr_alm(BaseDispOpnd(vrstb_opd, opnd_immed_8_u64, type_u64),
		       STQ, vr1);
    emit(mi);
    mi = new_instr_alm(BaseDispOpnd(vrstb_opd, opnd_immed_16_u64, type_u64),
		       STQ, vr2);
    emit(mi);

    // load and store base register updates
    mi = new_instr_alm(vrldb_opd, ADDQ, vrldb_opd, opnd_immed_24_u64);
    emit(mi);
    mi = new_instr_alm(vrstb_opd, ADDQ, vrstb_opd, opnd_immed_24_u64);
    emit(mi);

    // index register update and branch
    mi = new_instr_alm(vr_i_opd, SUBQ, vr_i_opd, opnd_immed_1_u64);
    emit(mi);
    mi = new_instr_cti(alpha::BNE, ll, vr_i_opd);
    emit(mi);

    // include last one or two double-word transfers, if needed
    claim(loop_leftover < 3);
    int o = 0;			// index reg for leftover offset
    Opnd o_opd;
    if (loop_leftover > 0) {
	Opnd vr3 = opnd_reg(type_u64);
	o_opd = opnd_immed(o, type_u64);
	mi = new_instr_alm(vr3, LDQ,
			   BaseDispOpnd(vrldb_opd, o_opd, type_u64));
	emit(mi);
	mi = new_instr_alm(BaseDispOpnd(vrstb_opd, o_opd, type_u64),
			   STQ, vr3);
	emit(mi);
	o += 8;
    }
    if (loop_leftover == 2) {
	Opnd vr4 = opnd_reg(type_u64);
	o_opd = opnd_immed(o, type_u64);
	mi = new_instr_alm(vr4, LDQ,
			   BaseDispOpnd(vrldb_opd, o_opd, type_u64));
	emit(mi);
	mi = new_instr_alm(BaseDispOpnd(vrstb_opd, o_opd, type_u64),
			   STQ, vr4);
	emit(mi);
	o += 8;
    }

    // post-alignment code -- finish off the transfer
    if (quadword_leftover > 0) {
	int zap_mask = (1 << quadword_leftover) - 1;
	Opnd z_opd = opnd_immed(zap_mask, type_u64);
	Opnd vr5 = opnd_reg(type_u64);
	Opnd vr6 = opnd_reg(type_u64);
	o_opd = opnd_immed(o, type_u64);

	mi = new_instr_alm(vr5, LDQ,
			   BaseDispOpnd(vrldb_opd, o_opd, type_u64));
	emit(mi);				// ldq src_struct

	mi = new_instr_alm(vr5, ZAPNOT, vr5, z_opd);
	emit(mi);				// zapnot

	mi = new_instr_alm(vr6, LDQ,
			   BaseDispOpnd(vrstb_opd, o_opd, type_u64));
	emit(mi);				// ldq dst_struct

	mi = new_instr_alm(vr6, ZAP, vr6, z_opd);
	emit(mi);				// zap

	mi = new_instr_alm(vr6, BIS, vr5, vr6);
	emit(mi);				// bis

	mi = new_instr_alm(BaseDispOpnd(vrstb_opd, o_opd, type_u64),
			   STQ, vr6);
	emit(mi);				// stq dst_struct
    }
}
