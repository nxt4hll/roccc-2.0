/* file "x86/translate_ldst.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#include <machine/machine.h>

#include "init.h"
#include "opcodes.h"
#include "reg_info.h"
#include "code_gen.h"

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

extern void add_regs_defd(Instr*, int l, int h);
extern void add_regs_used(Instr*, int l, int h);

using namespace x86;

// size is expressed in bits
#define MAX_SIZE_FOR_SMALL_STRUCT_MOVE 1024

void
CodeGenX86::translate_lod(Instr *mi)
{
    Opnd d = get_dst(mi);
    TypeId dtype = get_type(d);

    if (!is_scalar(dtype)) {
	// For structure loads, we just stack the instruction for later
	// translation.  We emit it after we see if the data is used in a
	// store or call.
	struct_lds.push_back(mi);

    } else if (is_floating_point(dtype)) {
	Instr *mj = fp_stack.push(get_src(mi,0));
	emit(mj);
	if (is_var(d)) {
	    // write ST to symbol in memory
	    mj = fp_stack.pop(d);
	    emit(mj);
	} else if (is_reg(d)) {
	    // d is reg so rename ST
	    fp_stack.rename_st(get_reg(d));
	}
	copy_notes(mi, mj);
	delete mi;

    } else {
	set_opcode(mi, x86::MOV);
	emit(mi);
    }
}


void
CodeGenX86::translate_str(Instr *mi)
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

    } else if (is_floating_point(stype)) {
	Instr *mj = NULL;
	if (is_var(s)) {
	    // first fld symbol
	    mj = fp_stack.push(s);
	} else if (is_reg(s)) {
	    // s is reg so move vr in fpstack to ST
	    mj = fp_stack.exchange(s);
	}
	if (mj)
	    emit(mj);
	mj = fp_stack.pop(get_dst(mi));
	copy_notes(mi, mj);
	emit(mj);
	delete mi;

    } else {
	set_opcode(mi, x86::MOV);
	emit(mi);
    }
}


void
CodeGenX86::translate_memcpy(Instr *mi)
{
    Opnd s = get_src(mi, 0);
    Opnd d = get_dst(mi);

    claim(is_addr(s) && is_addr(d));

    TypeId rtype = get_deref_type(s);
    claim(!is_void(rtype));

    Opnd v = opnd_reg(rtype);
    
    if (is_scalar(rtype)) {
	// simple load and store
	translate_lod(new_instr_alm(v, suifvm::LOD, s));
	set_opcode(mi, suifvm::STR);
	set_src(mi, 0, v);
	translate_str(mi);

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
 */

/* Expand_struct_move() - performs the block copy of a structure by
 * using the information in lr and sr.  It also assumes that the
 * destination reg of the load record instr is a virtual reg. */
void
CodeGenX86::expand_struct_move(Instr *lr, Instr *sr)
{
    Opnd lr_d = get_dst(lr);
    claim(is_virtual_reg(lr_d));
    claim(get_reg(lr_d) == get_reg(get_src(sr, 0)));

    TypeId rec_type = get_type(lr_d);
    int struct_size = get_bit_size(rec_type);
    int struct_alignment = get_bit_alignment(rec_type);

    if (struct_alignment > 32)
	struct_alignment = 32;
    else if (struct_alignment == 0)
	struct_alignment = 8;

    claim((struct_alignment == 8) || (struct_alignment == 16) ||
	  (struct_alignment == 32));

    // do block copy of structure from source to sink
    if (((struct_size == 32) || (struct_size == 16) || (struct_size == 8))
    && (struct_alignment >= struct_size)) {
	// Simple ld and st MOV's.  This code assumes that we do NOT
	// have to correct variable types.
	set_opcode(lr, x86::MOV);
	emit(lr);
	set_opcode(sr, x86::MOV);
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
 *
 * This code reuses the lr and sr instruction containers.  Do not
 * delete these heap objects. */
void
CodeGenX86::do_small_struct_move(Instr *lr, Instr *sr,
				 int struct_alignment, int struct_size)
{
    Instr *mi;

    int bits_in_chunk = struct_alignment;
    int bytes_in_chunk = bits_in_chunk >> 3;

    // shift struct_<<stuff>> from bits to bytes
    int struct_offset = 0;
    struct_size = struct_size >> 3;
    struct_alignment = struct_alignment >> 3;

    TypeId mv_type;
    switch (bits_in_chunk) {
      case 32:
	mv_type = type_u32;
	break;
      case 16:
	mv_type = type_u16;
	break;
      case 8:
	mv_type = type_u8;
	break;
      default:
	claim(false);
    }

    // turn structure load into lea for structure load base register
    Opnd vrldb_opnd = opnd_reg(type_addr());
    set_opcode(lr, LEA);
    set_dst(lr, vrldb_opnd);
    emit(lr);

    // turn structure store into lea for struct store base reg
    Opnd vrstb_opnd = opnd_reg(type_addr());
    set_opcode(sr, LEA);
    Opnd ea = get_dst(sr);
    set_dst(sr, vrstb_opnd);
    set_src(sr, 0, ea);
    emit(sr);

    // loop to create most of ld/st's
    int num_pairs = struct_size / struct_alignment;
    for (int pair_num = 0; pair_num < num_pairs; ++pair_num) {
	Opnd vr = opnd_reg(mv_type);
	Opnd so_opnd = opnd_immed(struct_offset, type_s32);

	mi = new_instr_alm(vr, x86::MOV, 
			   BaseDispOpnd(vrldb_opnd, so_opnd, mv_type));
	emit(mi);				// load

	mi = new_instr_alm(BaseDispOpnd(vrstb_opnd, so_opnd, mv_type),
			   x86::MOV, vr);
	emit(mi);				// store

	struct_offset += bytes_in_chunk;	// prepare for next move
    }

    // up to 2 remaining memory move operations
    int remaining_bytes = struct_size - struct_offset;
    do_remaining_moves(remaining_bytes, vrldb_opnd, vrstb_opnd, struct_offset);
}


/*
 * do_big_struct_move() -- looping code to do structure move.  For
 * X86 machines, use REP MOVS* to do the structure move.
 *
 * Note that the MOVS* instruction must be seen as both reading and
 * writing registers ECX (count), ESI (source pointer), and EDI
 * (destination pointer).
 *
 * This code reuses the lr and sr instruction containers.  Do not
 * delete these heap objects. 
 */
void
CodeGenX86::do_big_struct_move(Instr *lr, Instr *sr,
			       int struct_alignment, int struct_size)
{
    Instr *mi;
    Opnd ea;

    // turn lr into an instruction that sets ESI = source addr
    ea = get_src(lr, 0);
    set_dst(lr, opnd_reg(ESI, type_addr()));
    if (BaseDispOpnd bdo = ea) {
	if (get_immed_integer(bdo.get_disp()) == 0) {
	    // use MOV to set ESI
	    set_opcode(lr, x86::MOV);
	    set_src(lr, 0, bdo.get_base());
	} else {
	    goto set_esi_using_lea;
	}
    } else {
    set_esi_using_lea:
	set_opcode(lr, LEA);			// ea is already src0
    }
    emit(lr);

    // turn sr into an instruction that sets EDI = dest addr
    ea = get_dst(sr, 0);
    set_dst(sr, opnd_reg(EDI, type_addr()));
    if (BaseDispOpnd bdo = ea) {
	if (get_immed_integer(bdo.get_disp()) == 0) {
	    // use MOV to set EDI
	    set_opcode(sr, x86::MOV);
	    set_src(sr, 0, bdo.get_base());
	} else {
	    goto set_edi_using_lea;
	}
    } else {
    set_edi_using_lea:
	set_opcode(sr, LEA);
	set_src(sr, 0, ea);
    }
    emit(sr);

    // clear the direction flag
    mi = new_instr_alm(opnd_reg_eflags, CLD);
    emit(mi);

    // shift struct_<<stuff>> from bits to bytes and calculate number
    // of aligned chunks to move with REP MOVS
    int bits_in_chunk = struct_alignment;
    struct_size = struct_size >> 3;
    struct_alignment = struct_alignment >> 3;

    // load the number of aligned chunks to move into ECX
    mi = new_instr_alm(opnd_reg(ECX, type_u32), x86::MOV, 
		       opnd_immed(struct_size / struct_alignment, type_u32));
    add_regs_defd(mi, CL, CH);
    emit(mi);

    // select the correct opcode and set the move type
    int movs_op;
    TypeId mv_type;
    switch (bits_in_chunk) {
      case 8:
	movs_op = MOVSB;
	mv_type = type_u8;
	break;
      case 16:
	movs_op = MOVSW;
	mv_type = type_u16;
	break;
      case 32:
	movs_op = MOVSL;
	mv_type = type_u32;
	break;
      default:
	claim(false,
	      "Do_big_struct_move() -- unexpected number of bits_in_chunk %d",
	      bits_in_chunk);
    };

    // Create REP MOVS*.  First set sources ...
    Opnd edi_ea = BaseDispOpnd(opnd_reg(EDI, type_addr()),
			       opnd_immed(0, type_s32), mv_type);
    Opnd esi_ea = BaseDispOpnd(opnd_reg(ESI, type_addr()),
			       opnd_immed(0, type_s32), mv_type);
    mi = new_instr_alm(edi_ea, movs_op, esi_ea,
		       opnd_reg(ECX, type_u32));	// count reg
    set_src(mi, 2, opnd_reg_eflags);			// direction flag
    // ... then set dsts (eflags not written) ...
    set_dst(mi, 1, opnd_reg(EDI, type_addr()));
    set_dst(mi, 2, opnd_reg(ESI, type_addr()));
    set_dst(mi, 3, opnd_reg(ECX, type_u32));
    add_regs_defd(mi, CL, CH);
    add_regs_used(mi, CL, CH);
    // ... finally attach REP prefix.
    ListNote<long> note;
    note.append(REP);
    set_note(mi, k_instr_opcode_exts, note);
    emit(mi);

    // moves to finish leftover pieces
    int remaining_bytes = struct_size - ((struct_size / struct_alignment)
					 * struct_alignment);
    do_remaining_moves(remaining_bytes, opnd_reg(ESI, type_addr()),
		       opnd_reg(EDI, type_addr()), 0);
}


/* Do_remaining_moves() -- We move aligned items, and it sometimes turns
 * out that the structure is not an integral number of aligned items in
 * size.  This routine finishes move whatever number of bytes are leftover
 * after moving all of the aligned items that we possibly can. */
void
CodeGenX86::do_remaining_moves(int remaining_bytes, Opnd vrldb_opnd,
			       Opnd vrstb_opnd, int struct_offset)
{
    Instr *mi;
    Opnd so_opnd = opnd_immed(struct_offset, type_s32);

    if (remaining_bytes >= 2) {		// move halfword-aligned quantity
	Opnd vr = opnd_reg(type_u16);

	mi = new_instr_alm(vr, x86::MOV,
			   BaseDispOpnd(vrldb_opnd, so_opnd, type_u16));
	emit(mi);			// load

	mi = new_instr_alm(BaseDispOpnd(vrstb_opnd, so_opnd, type_u16),
			   x86::MOV, vr);
	emit(mi);			// store

	struct_offset += 2;		// prepare for next move
	remaining_bytes -= 2;
    }

    if (remaining_bytes == 1) {		// move last byte
	Opnd vr = opnd_reg(type_u8);

	mi = new_instr_alm(vr, x86::MOV,
			   BaseDispOpnd(vrldb_opnd, so_opnd, type_u8));
	emit(mi);			// load

	mi = new_instr_alm(BaseDispOpnd(vrstb_opnd, so_opnd, type_u8),
			   x86::MOV, vr);
	emit(mi);			// store

	remaining_bytes -= 1;
    }

    claim(remaining_bytes == 0);	// better be done!
}
