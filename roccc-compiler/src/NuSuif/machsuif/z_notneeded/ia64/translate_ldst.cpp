/* file "ia64/translate_ldst.cpp" */

/*
    Copyright (c) 2002 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#include <machine/machine.h>
#include <suifvm/suifvm.h>

#include <ia64/init.h>
#include <ia64/opcodes.h>
#include <ia64/reg_info.h>
#include <ia64/code_gen.h>
#include <ia64/instr.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace ia64;

// size is expressed in bits
#define MAX_SIZE_FOR_SMALL_STRUCT_MOVE 128


void
CodeGenIa64::translate_lod(Instr *mi)
{
    Opnd src = get_src(mi, 0);
    Opnd dst = get_dst(mi);
    TypeId dtype = get_type(dst);
    int dsize;
    claim(is_addr(src));

    if (!is_scalar(dtype)) {
	// For structure loads, we just stack the instruction for later
	// translation.  We emit it after we see if the data is used in a
	// store or call.
	struct_lds.push_back(mi);
    } else {
	// For scalar loads, check the source address.  If the source is
	// the address of a local symbol, then emit
	//
	//   add tmp = ADDR_SYM, sp
	//
	// If it's the address of a global symbol, then emit
	//
	//   addl tmp = ADDR_SYM, gp
	//   ld8  tmp = [tmp]
	//
	// In either case, change the original load's source to [tmp],
	// represented as a base-disp address expression having zero
	// displacement.

	if (is_addr_sym(src)) {
	    Opnd tmp = opnd_reg(dtype);
	    set_src(mi, 0, BaseDispOpnd(tmp, opnd_immed_0_u64));

	    VarSym *var = static_cast<VarSym*>(get_sym(src));
	    claim(is_kind_of<VarSym>(var), "Unexpected kind of load address");
	    if (is_auto(var)) {
		int add = make_opcode(ADD, EMPTY,EMPTY,EMPTY);
		Instr *mj = new_instr_alm(tmp, add, src, opnd_reg_gp);
		emit(mj);
	    } else {
		int addl = make_opcode(ADDL, EMPTY,EMPTY,EMPTY);
		Instr *mj = new_instr_alm(tmp, addl, src, opnd_reg_gp);
		emit(mj);
		int ld8 = make_opcode(LD, SZ_8,LDTYPE_NONE,LDHINT_NONE);
		mj = new_instr_alm(tmp, ld8, BaseDispOpnd(tmp, opnd_immed_0_u64));
		emit(mj);
	    }
	}
	// For scalar loads, change the opcode and emit.  A signed, integral
	// subword value must be sign-extended after the load.

	set_opcode(mi, opcode_load(dtype));
	emit(mi);
	if (is_integral(dtype) && is_signed(dtype) &&
	    ((dsize = get_bit_size(dtype)) < 64)) {
	    int xsz = (dsize == 32) ? XSZ_4 :
		      (dsize == 16) ? XSZ_2 :
		      (dsize ==  8) ? XSZ_1 :
		      -1;
	    claim(xsz != -1, "Unexpected bit size as load destination");
	    int sxt_xsz = make_opcode(SXT,xsz, EMPTY,EMPTY);
	    emit(new_instr_alm(dst, sxt_xsz, dst));
	}
    }
}

void
CodeGenIa64::translate_str(Instr *mi)
{
    Opnd s = get_src(mi, 0);
    Opnd d = get_dst(mi);
    TypeId stype = get_type(s);
    claim(is_addr(d));

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
	// For scalar stores, check the destination address.  If it's the
	// address of a local symbol, then emit
	//
	//   add tmp = ADDR_SYM, sp
	//
	// If it's the address of a global symbol, then emit
	//
	//   addl tmp = ADDR_SYM, gp
	//   ld8  tmp = [tmp]
	//
	// In either case, change the store destination to [tmp], represented as
	// a base-disp address expression having zero displacement.

	if (is_addr_sym(d)) {
	    Opnd tmp = opnd_reg(stype);
	    set_dst(mi, 0, BaseDispOpnd(tmp, opnd_immed_0_u64));

	    VarSym *var = static_cast<VarSym*>(get_sym(d));
	    claim(is_kind_of<VarSym>(var), "Unexpected kind of store address");
	    if (is_auto(var)) {
		int add = make_opcode(ADD, EMPTY,EMPTY,EMPTY);
		Instr *mj = new_instr_alm(tmp, add, d, opnd_reg_gp);
		emit(mj);
	    } else {
		int addl = make_opcode(ADDL, EMPTY,EMPTY,EMPTY);
		Instr *mj = new_instr_alm(tmp, addl, d, opnd_reg_gp);
		emit(mj);
		int ld8 = make_opcode(LD, SZ_8,LDTYPE_NONE,LDHINT_NONE);
		mj = new_instr_alm(tmp, ld8, BaseDispOpnd(tmp, opnd_immed_0_u64));
		emit(mj);
	    }
	}
	set_opcode(mi, opcode_store(stype));
	emit(mi);
    }
}


void
CodeGenIa64::translate_memcpy(Instr *mi)
{
    Opnd s = get_src(mi, 0);
    Opnd d = get_dst(mi);

    claim(is_addr(s) && is_addr(d));

    TypeId rtype = get_deref_type(s);
    claim(!is_void(rtype));

    Opnd v = opnd_reg(rtype);
    
    if (is_scalar(rtype)) {
	// Simple load and store, using translate_lod() to make sure the
	// load is followed by sign extension if necessary.

	translate_lod(new_instr_alm(v, suifvm::LOD, s));
	Instr *mj = new_instr_alm(d, opcode_store(rtype), v);
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
	claim(false, "Unexpected type for memcpy instr");

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
 */

/* expand_struct_move() - performs the block copy of a structure by using
 * the information in a lr and sr code generator pseudo op. It also assumes
 * that the destination reg of the load record instr is a virtual reg that
 * lives in a temporary reg, i.e. not in a parameter or result reg. */
void
CodeGenIa64::expand_struct_move(Instr *lr, Instr *sr)
{
    Opnd lr_d = get_dst(lr);
    claim(is_virtual_reg(lr_d));
    claim(get_reg(lr_d) == get_reg(get_src(sr, 0)));

    TypeId rec_type = get_type(lr_d);
    int struct_size = get_bit_size(rec_type);
    int struct_alignment = get_bit_alignment(rec_type);

    if (struct_alignment == 0) struct_alignment = 8;

    claim((struct_alignment == 8) || (struct_alignment == 16) ||
          (struct_alignment == 32) || (struct_alignment == 64) ||
          (struct_alignment == 80));

    // do block copy of structure from source to sink
#if 0				// FIXME: problems with single-step transfer:
				// (1) seems bogus to move random bit pattern
				// through the FPU (80-bit struct); (2) the sr
				// instr may have a base-disp dst addr with a
				// non-zero disp from SP.
    if (((struct_size == 80)
    || (struct_size == 64) || (struct_size == 32) 
    || (struct_size == 16) || (struct_size == 8))
    && (struct_alignment >= struct_size)) {

        TypeId mv_type;
        switch (struct_size) {
          case 80:
            set_opcode(lr,make_opcode(ia64::LDF,FSZ_E,LDTYPE_NONE,LDHINT_NONE));
            set_opcode(sr,make_opcode(ia64::STF,FSZ_E,STTYPE_NONE,STHINT_NONE));
            mv_type = type_f80;
            break;
          case 64:
            set_opcode(lr, make_opcode(ia64::LD, SZ_8,LDTYPE_NONE,LDHINT_NONE));
            set_opcode(sr, make_opcode(ia64::ST, SZ_8,STTYPE_NONE,STHINT_NONE));
            mv_type = type_s64;
            break;
          case 32:
            set_opcode(lr, make_opcode(ia64::LD, SZ_4,LDTYPE_NONE,LDHINT_NONE));
            set_opcode(sr, make_opcode(ia64::ST, SZ_4,STTYPE_NONE,STHINT_NONE));
            mv_type = type_s32;
            break;
          case 16:
            set_opcode(lr, make_opcode(ia64::LD, SZ_2,LDTYPE_NONE,LDHINT_NONE));
            set_opcode(sr, make_opcode(ia64::ST, SZ_2,STTYPE_NONE,STHINT_NONE));
            mv_type = type_s16;
            break;
          case 8:
            set_opcode(lr, make_opcode(ia64::LD, SZ_1,LDTYPE_NONE,LDHINT_NONE));
            set_opcode(sr, make_opcode(ia64::ST, SZ_1,STTYPE_NONE,STHINT_NONE));
            mv_type = type_s8;
            break;
          default:
            claim(false);
        }

        // set generic types for move
        Opnd v = opnd_reg(mv_type);
        set_dst(lr, 0, v);
        emit(lr);

        set_src(sr, 0, v);
        emit(sr);

    }
    else
#endif
    {
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
 * The lr and sr instruction containers are deleted in a parent routine.
 */
void
CodeGenIa64::do_small_struct_move(Instr *lr, Instr *sr,
				  int struct_alignment, int struct_size)
{
    int bits_in_chunk = struct_alignment;
    int bytes_in_chunk = bits_in_chunk >> 3;

    // shift struct_<<stuff>> from bits to bytes
    int struct_offset = 0;
    struct_size = struct_size >> 3;
    struct_alignment = struct_alignment >> 3;

    TypeId mv_type;
    int opcode_ld, opcode_st;
    switch (bits_in_chunk) {
      case 64:
        opcode_ld = make_opcode(ia64::LD, SZ_8,EMPTY,EMPTY);
        opcode_st = make_opcode(ia64::ST, SZ_8,EMPTY,EMPTY);
        mv_type = type_s64;
        break;
      case 32:
        opcode_ld = make_opcode(ia64::LD, SZ_4,EMPTY,EMPTY);
        opcode_st = make_opcode(ia64::ST, SZ_4,EMPTY,EMPTY);
        mv_type = type_s32;
        break;
      case 16:
        opcode_ld = make_opcode(ia64::LD, SZ_2,EMPTY,EMPTY);
        opcode_st = make_opcode(ia64::ST, SZ_2,EMPTY,EMPTY);
        mv_type = type_s16;
        break;
      case 8:
        opcode_ld = make_opcode(ia64::LD, SZ_1,EMPTY,EMPTY);
        opcode_st = make_opcode(ia64::ST, SZ_1,EMPTY,EMPTY);
        mv_type = type_s8;
        break;
      default:
        claim(false);
    }
    
    // struct load and store addresses
    Opnd addr_ld = get_src(lr, 0);
    Opnd addr_st = get_dst(sr, 0);

    int num_pairs = struct_size / struct_alignment;
    Opnd delta = opnd_immed(bytes_in_chunk, type_s64);	// ld/st addr increment

    // Initialize running ptrs to the loaded and stored structs.  In each
    // case, if the address is already in base-displacement form, assume
    // that it's relative to SP, and generate
    //
    //    add  ptr = disp, base
    //
    // If the address is that of a local struct variable, use
    //
    //    add  ptr = var_addr, sp
    //
    // For the address of a global struct variable, use
    //
    //    addl ptr = var_addr, gp
    //    ld8  ptr = [ptr]

    Opnd ptr_ld    = opnd_reg(type_p64);
    Opnd ptr_st    = opnd_reg(type_p64);

    Opnd at_ptr_ld = BaseDispOpnd(ptr_ld, opnd_immed_0_u64);
    Opnd at_ptr_st = BaseDispOpnd(ptr_st, opnd_immed_0_u64);

    if (BaseDispOpnd bd_ld = addr_ld) {
	int add = make_opcode(ADD, EMPTY,EMPTY,EMPTY);
	emit(new_instr_alm(ptr_ld, add, bd_ld.get_disp(), bd_ld.get_base()));
    } else {
	claim(is_addr_sym(addr_ld), "Unexpected kind of struct load address");

	VarSym *var = static_cast<VarSym*>(get_sym(addr_ld));
	claim(is_kind_of<VarSym>(var));

	bool is_local = is_auto(var);
	Opnd base = is_local ? opnd_reg_sp : opnd_reg_gp;
	int add = make_opcode(is_local ? ADD : ADDL, EMPTY,EMPTY,EMPTY);

	emit(new_instr_alm(ptr_ld, add, addr_ld, base));

	if (!is_local) {
	    int ld8 = make_opcode(LD, SZ_8,LDTYPE_NONE,LDHINT_NONE);
	    emit(new_instr_alm(ptr_ld, ld8, clone(at_ptr_ld)));
	}
    }

    if (BaseDispOpnd bd_st = addr_st) {
	int add = make_opcode(ADD, EMPTY,EMPTY,EMPTY);
	emit(new_instr_alm(ptr_st, add, bd_st.get_disp(), bd_st.get_base()));
    } else {
	claim(is_addr_sym(addr_st), "Unexpected kind of struct store address");

	VarSym *var = static_cast<VarSym*>(get_sym(addr_st));
	claim(is_kind_of<VarSym>(var));

	bool is_local = is_auto(var);
	Opnd base = is_local ? opnd_reg_sp : opnd_reg_gp;
	int add = make_opcode(is_local ? ADD : ADDL, EMPTY,EMPTY,EMPTY);

	emit(new_instr_alm(ptr_st, add, addr_st, base));

	if (!is_local) {
	    int ld8 = make_opcode(LD, SZ_8,LDTYPE_NONE,LDHINT_NONE);
	    emit(new_instr_alm(ptr_st, ld8, clone(at_ptr_st)));
	}
    }

    for (int pair_num = 0; pair_num < num_pairs; ++pair_num) {
	Opnd chunk = opnd_reg(mv_type);		// chunk transfer tmp

        Instr *mi = new_instr_alm(chunk, opcode_ld, clone(at_ptr_ld), delta);
        set_dst(mi, 1, ptr_ld);			// implied 2nd dst
        emit(mi);                               // load

        mi = new_instr_alm(clone(at_ptr_st), opcode_st, chunk, delta);
        set_dst(mi, 1, ptr_st);			// implied 2nd dst
        emit(mi);                               // store

        struct_offset += bytes_in_chunk;        // for tracking leftovers
    }

    // handle leftover bytes
    int remaining_bytes = struct_size - struct_offset;
    if (remaining_bytes) {
	do_remaining_moves(remaining_bytes, at_ptr_ld, at_ptr_st);
    }
}


/* do_big_struct_move() -- looping code to do structure move.  Move
 * code in three quad-word chunks at a time.  Handle alignment cases
 * at end carefully!
 *
 * This code reuses the lr and sr instruction containers.  Do not
 * delete these heap objects. */
void
CodeGenIa64::do_big_struct_move(Instr *lr, Instr *sr,
				int struct_alignment, int struct_size)
{
  /* We'll just call dst = memcpy(dst, src, sz); */

  Opnd src = get_src(lr, 0);
  Opnd dst = get_dst(sr);

  //Since src and dst will be in BaseDisp form, get their base
  claim(is_addr_exp(src), "Invalid src opnd for ld");
  claim(is_addr_exp(dst), "Invalid dst opnd for st");
  src = ((BaseDispOpnd)src).get_base();
  dst = ((BaseDispOpnd)dst).get_base();

  // move the size into a register for the call 
  Opnd sz = opnd_reg(type_s64);
  translate_ldc(new_instr_alm(sz, suifvm::LDC, 
	opnd_immed(struct_size>>3, type_s64)));  /* Cvt struct_size to bytes*/

  TypeId type = find_proc_type(type_p64, type_p64, type_u64); 
  ProcSym *sym = find_proc_sym(type, k_memcpy);

  Instr *call = new_instr_cti(dst, suifvm::CAL, sym, opnd_null(), dst); 
  set_src(call, 2, src);
  set_src(call, 3, sz);
  translate_cal(call);
}

/* Do_remaining_moves() -- We move aligned items, and it sometimes turns
 * out that the structure is not an integral number of aligned items in
 * size.  This routine finishes moving whatever bytes are leftover
 * after all possible aligned items have been copied.
 */
void
CodeGenIa64::do_remaining_moves(int remaining_bytes, Opnd at_ptr_ld,
				Opnd at_ptr_st)
{
    claim(is_base_disp(at_ptr_ld) && is_base_disp(at_ptr_st));

    Opnd delta = opnd_immed_8_u64;
    int opcode_ld = make_opcode(LD, SZ_1, EMPTY,EMPTY);
    int opcode_st = make_opcode(ST, SZ_1, EMPTY,EMPTY);
    Opnd ptr_ld = ((BaseDispOpnd)at_ptr_ld).get_base();
    Opnd ptr_st = ((BaseDispOpnd)at_ptr_st).get_base();

    // move the rest one byte at a time
    for (int i = 0; i < remaining_bytes; i++) {
        Opnd byte = opnd_reg(type_u8);

	Instr *mi = new_instr_alm(byte, opcode_ld, clone(at_ptr_ld), delta);
        set_dst(mi, 1, ptr_ld);			// implied 2nd dst
        emit(mi);

        mi = new_instr_alm(clone(at_ptr_st), opcode_st, byte, delta);
        set_dst(mi, 1, ptr_st);			// implied 2nd dst
        emit(mi);
    }
}
