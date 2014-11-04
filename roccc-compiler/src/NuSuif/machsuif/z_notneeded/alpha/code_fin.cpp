/* file "alpha/code_fin.cpp" */

/*
   Copyright (c) 2000 The President and Fellows of Harvard College

   All rights reserved.

   This software is provided under the terms described in
   the "machine/copyright.h" include file.
   */

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "alpha/code_fin.h"
#endif

#include <ctype.h>		// (isdigit)
#include <machine/machine.h>

#include <alpha/init.h>
#include <alpha/opcodes.h>
#include <alpha/reg_info.h>
#include <alpha/code_fin.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace alpha;

  void
CodeFinAlpha::init(OptUnit *unit)
{
  CodeFin::init(unit);

  for (int c = CLASS_GPR; c <= CLASS_FPR; ++c)
    saved_reg_set[c].remove_all();
}

/*
 * analyze_opnd -- Accumulate the sets of SAV-convention registers
 * actually used in the program.  Identify and record non-parameter
 * automatic variables used in address operands.  (Parameters are handled
 * separately in layout_frame.)  Virtual and symbolic registers should have
 * been eliminated before code finalization.
 */
  void
CodeFinAlpha::analyze_opnd(Opnd opnd)
{
  if (is_hard_reg(opnd)) {
    int r = get_reg(opnd);
    if (reg_callee_saves_alpha()->contains(r))	    
      saved_reg_set[((r > LAST_GPR) && (r <= LAST_FPR))
        ? CLASS_FPR : CLASS_GPR].insert(r);
  } else if (is_addr_sym(opnd)) {
    VarSym *v = (VarSym*)get_sym(opnd);
    if (is_a<VarSym>(v) &&
        is_auto(v) &&
        frame_map.count(v) == 0) {
      claim(!is_a<ParameterSymbol>(v));
      frame_map[v] = 0;	// add v to map, but with no frame offset yet
    }
  } else {
    claim(!is_virtual_reg(opnd), "unallocated virtual register");
    claim(!is_var(opnd),	     "unallocated symbolic register");
  }
}

/*
 * layout_frame -- Assign stack frame locations to each variable
 * that needs one.  Store the offsets in frame_map.
 */
  void
CodeFinAlpha::layout_frame()
{
  debug(4, "... determine offsets from $sp for variables");

  frameoffset = 0;

  SymTable *st = cur_unit->get_symbol_table();
  Iter<SymbolTableObject*> iter = st->get_symbol_table_object_iterator();
  for ( ; iter.is_valid(); iter.next()) {
    SymbolTableObject *sto = iter.current();

    if (is_a<VarSym>(sto)) {
      VarSym *v = (VarSym*)sto;

      // If v is a fictitious variable, invented to represent the
      // stack home of a register passed varargs parameter, just
      // plug its frame offset into frame_map.
      //
      if (OneNote<long> note = get_note(v, k_stdarg_offset)) {
        frame_map[v] = note.get_value();
        continue;
      }
      Map<Sym*,int>::iterator v_handle = frame_map.find(v);
      if (v_handle == frame_map.end())
        continue;				// v never used

      // Here v is an automatic variable, other than a parameter,
      // that is actually used in the program.
      // Allocate frame space in multiples of 4 bytes.  Otherwise,
      // must use a different ld/st_to/from_stack routine.  However,
      // align locals larger than 4 bytes on 8-byte boundaries.
      //
      int v_size = get_bit_size(get_type(v)) >> 3;// in bytes
      if (v_size <= 4) {
        frameoffset -= 4;
      } else {
        frameoffset &= -8;			// next 8-byte boundary
        frameoffset -= (v_size + 3) & -4;	// alloc a multiple of 4
      }

      // Adjust the offset to account for space allocated for
      // parameters at the top of the stack frame, if the procedure
      // has parameters.
      //
      int realoffset = frameoffset;
      if (get_formal_param_count(cur_unit)) {
        if (is_varargs) realoffset -= 96;
        else realoffset -= 48;
      }
      (*v_handle).second = realoffset;		// update frame_map
    }
  }

  claim((-frameoffset & 3) == 0);	// better be aligned on 4-byte boundary

  // Process parameter list in order.  Set running offset p_offset for
  // parameters to reflect the fact that parameters passed in registers
  // are actually saved in our stack frame.
  //
  int p_offset = -48;
  debug(4, "... determine offsets from $sp for parameters");

  bool do_stdarg_int_half = true;

  for (int i = 0; i < get_formal_param_count(cur_unit); i++) {
    VarSym *p = get_formal_param(cur_unit, i);
    int p_size = get_bit_size(get_type(p)) >> 3;	// in bytes

    // set $sp offset -- always on 8-byte boundary
    claim(frame_map.count(p) == 0);
    const char *intarg = "_stdintarg"; 
    const char *fparg = "_stdfparg"; 
    if (strcmp(get_name(p).chars(), "va_alist") == 0) {

      // If this is a varargs proc (rather than a stdarg one),
      // skip the va_alist parameter since it takes 0 space.
      claim(is_varargs);
      frame_map[p] = p_offset;

    } else if (strncmp(get_name(p).chars(), intarg, strlen(intarg)) == 0) {

      // handle integer part of vararg/stdarg unnamed parameters
      claim(do_stdarg_int_half); 
      frame_map[p] = p_offset;
      do_stdarg_int_half = false;		// no offset update yet

    } else if (strncmp(get_name(p).chars(), fparg, strlen(fparg)) == 0) {

      // handle fp part of vararg/stdarg unnamed parameters
      claim(!do_stdarg_int_half); 
      frame_map[p] = p_offset-48;		// fp offset
      p_offset += p_size;			// update running offset
      do_stdarg_int_half = true;

    } else {
      frame_map[p] = p_offset;
      p_offset += p_size;			// update running offset
    } 

    // Pad out p_offset to 4-byte boundary then to 8-byte boundary.
    // Why?  Cause we're too lazy to really change the code from
    // the way mgen worked.
    //
    if (int p_offset_mod_4 = (p_offset & 3))
      p_offset += (4 - p_offset_mod_4);
    if (p_offset & 7) p_offset += 4;
  }
  claim(do_stdarg_int_half);

  // Calculate the final framesize and frameoffset values.
  //
  framesize   = -frameoffset;
  localoffset = -frameoffset;

  // Local space should be a multiple of 16 bytes.  Should already be
  // aligned on a 4-byte boundary.
  // 
  claim((framesize & 3) == 0);

  if (int framesize_mod_16 = (framesize & 15)) {
    int bytes_to_add = 16 - framesize_mod_16;
    framesize   += bytes_to_add;
    frameoffset -= bytes_to_add;
    localoffset += bytes_to_add;
  }

  if (!is_leaf 
      ||  !saved_reg_set[CLASS_GPR].is_empty()
      ||  !saved_reg_set[CLASS_FPR].is_empty()) {
    // Leave room in stack frame for return address, even in some cases
    // when we don't save and restore it.  For any procedure using
    // saved registers, the ALPHA assembler assumes that $ra will be
    // saved and restored.
    framesize   += 8;
    frameoffset -= 8;

    // Include space for the saved registers used
    //
    for (int c = CLASS_GPR; c <= CLASS_FPR; ++c)
      if (!saved_reg_set[c].is_empty()) {
        int bytes_to_add = (saved_reg_set[c].size() * 8);
        framesize   += bytes_to_add;
        frameoffset -= bytes_to_add;
      }

    // If this is not a leaf procedure, mark return address register to
    // be saved and restored.
    if (!is_leaf)
      saved_reg_set[CLASS_GPR].insert(RA);
  }

  // Assure that framesize and frameoffset are multiples of 16 bytes
  if (framesize & 15) {
    framesize   += 8;
    frameoffset -= 8;
  }

  // Include argument-build space if there are any calls in current proc
  if (!is_leaf) framesize += max_arg_area;

  // Assure that framesize is an 8-byte multiple
  //						FIXME: not 16-byte multiple?
  if (framesize & 7) framesize += 4;

  // Include space for parameter dump, if necessary
  if (get_formal_param_count(cur_unit)) {
    if (is_varargs) {
      /* include space for twelve 8-byte items */
      framesize	+= 96;
      frameoffset -= 96;
      localoffset += 96;
    } else {
      /* include space for six 8-byte items */
      framesize	+= 48;
      frameoffset -= 48;
      localoffset += 48;
    }
  }
}

/*
 * replace_opnd -- If argument is an address that uses a stack variable,
 * return a replacement frame address that indexes from the stack pointer.
 * Otherwise, return the original operand.
 */
  Opnd
CodeFinAlpha::replace_opnd(Opnd opnd)
{
  if (is_addr_sym(opnd)) {
    return frame_addr(opnd, opnd, 0);
  } else if (SymDispOpnd sdo = opnd) {
    int offset = get_immed_int(sdo.get_disp());
    return frame_addr(opnd, sdo.get_addr_sym(), offset);
  }
  return opnd;
}

/*
 * frame_addr -- Subfunction of replace_opnd taking an address-symbol
 * operand and a `delta' relative to that symbol.  If the symbol has a
 * frame slot, return a replacement address that indexes from the stack
 * pointer.  The displacement in this address combines the symbol-relative
 * delta with the slot displacement of the symbol.  The latter is the sum
 * of the frame size and the `frame offset' of the symbol, a number stored
 * in frame_map that may be positive (for stack-passed formal parameter
 * symbols) or negative (for other locals).  The combined $sp displacement
 * should be non-negative.
 *
 * If the symbol is not in the frame, return the orig(inal) operand.
 */
  Opnd
CodeFinAlpha::frame_addr(Opnd orig, Opnd adr_sym, int delta)
{
  VarSym *v = (VarSym*)get_sym(adr_sym);
  if (frame_map.count(v) == 0)
    return orig;

  delta += frame_map[v] + framesize;
  claim(delta >= 0);

  Opnd disp = opnd_immed(delta, type_s32);
  return BaseDispOpnd(opnd_reg_sp, disp, get_deref_type(orig));
}

/*
 * make_header_trailer() -- Build the instruction lists for the procedure
 * entry and exits.  Note that not all of the header is built here--it is
 * completed in printmachine.  The header instructions are placed on a list
 * in reverse order so that we can later "pop" them easily into the
 * instruction stream after the proc_entry point.  The trailer instructions
 * are also placed on a list, to be inserted before returns marked with
 * k_incomplete_proc_exit notes.
 */
  void
CodeFinAlpha::make_header_trailer()
{
  Instr *mi;

  clear(header());				// begin collecting header code

  // create -- 				ldgp $gp,0($pv)
  mi = new_instr_alm(opnd_reg_gp, LDGP,
      BaseDispOpnd(opnd_reg_pv, opnd_immed(0, type_s32)));
  header().push_front(mi);

  // create, if needed --			lda $sp,-framesize($sp)
  if (framesize) {
    Opnd ea = BaseDispOpnd(opnd_reg_sp, opnd_immed(-framesize, type_s32));
    mi = new_instr_alm(opnd_reg_sp, LDA, ea);
    header().push_front(mi);
  }

  // Store general-purpose saved registers that need saving
  int sr_offset = 0;				// saved-reg running offset
  int os;

  if (   !saved_reg_set[CLASS_GPR].is_empty()
      || !saved_reg_set[CLASS_FPR].is_empty())
  {
    // If there are any GPR or FPR saved regs, $ra must have a stack home
    // and must be marked in a .mask pseudo-op, even in a leaf procedure.
    // (In a leaf procedure, we don't actually save and restore it, though.)
    //
    unsigned gp_saved_reg_mask = 1<<26;	// $ra == $26
    sr_offset = 8;

    // Generate stores for GP saved registers and compose a mask for the
    // .mask pseudo-op.  Only registers with encoding less than 32 go into
    // the latter, though a simulated register set may have more than 32
    // per bank.
    //
    NatSetIter gpsrs(saved_reg_set[CLASS_GPR].iter());
    for ( ; gpsrs.is_valid(); gpsrs.next()) {
      int r = gpsrs.current();

      if (r == RA)
        os = 0;
      else {
        os = sr_offset;
        sr_offset += 8;			// bytes per register
      }
      Opnd disp =
        opnd_immed(framesize + frameoffset + os, type_s32);

      mi = new_instr_alm(BaseDispOpnd(opnd_reg_sp, disp),
          STQ, opnd_reg(r, type_addr()));
      header().push_front(mi);

      const char *name = reg_name(r);
      claim(isdigit(name[0]));
      int encoding = atoi(name);
      if (encoding < 32)
        gp_saved_reg_mask |= (1 << encoding); 
    }

    // generate integer .mask pseudo-op
    mi = new_instr_dot(MASK,
        opnd_immed(gp_saved_reg_mask, type_u32), 
        opnd_immed(frameoffset, type_s32));
    header().push_front(mi);
  }

  // sanity check: still 8-byte aligned
  claim(((framesize + frameoffset + sr_offset) & 7) == 0);

  // Store floating-point saved registers that need saving and compose
  // mask for .fmask pseudo-op.  sr_offset continues from where last GP
  // saved-reg left off.
  //
  unsigned fp_saved_reg_mask = 0;

  NatSetIter fpsrs(saved_reg_set[CLASS_FPR].iter());
  for ( ; fpsrs.is_valid(); fpsrs.next()) {
    int r = fpsrs.current();
    Opnd disp =
      opnd_immed(framesize + frameoffset + sr_offset, type_s32);
    mi = new_instr_alm(BaseDispOpnd(opnd_reg_sp, disp, type_f64),
        STT, opnd_reg(r, type_f64));
    header().push_front(mi);
    sr_offset += 8;				// bytes per register

    const char *name = reg_name(r);
    claim(isdigit(name[1]));		// skip leading "f"
    int encoding = atoi(name + 1);
    if (encoding < 32)
      fp_saved_reg_mask |= (1 << encoding); 
  }

  if (fp_saved_reg_mask) {			// generate .fmask pseudo-op
    mi = new_instr_dot(FMASK,
        opnd_immed(fp_saved_reg_mask, type_u32), 
        opnd_immed(frameoffset, type_s32));
    header().push_front(mi);
  }

  mi = new_instr_dot(FRAME);
  set_src(mi, 0, opnd_immed(30, type_s32));	// $sp
  set_src(mi, 1, opnd_immed(framesize, type_s32));
  set_src(mi, 2, opnd_immed(26, type_s32));	// $ra
  set_src(mi, 3, opnd_immed(localoffset, type_s32));
  header().push_front(mi);				// .frame ...

  mi = new_instr_dot(PROLOGUE, opnd_immed(1, type_s32));
  header().push_front(mi);				// .prologue 1

  // Add a header instruction to save $gp if this is not a leaf procedure
  // and if there are occurrences of the local home variable for $gp.  Do
  // the mapping to the frame location manually, since header instructions
  // are not assumed to need operand replacement.
  //
  if (!is_leaf) {
    VarSym *gp_home = lookup_local_var(k_gp_home);
    if((gp_home != NULL) && (frame_map.count(gp_home) > 0)) {
      int delta = frame_map[gp_home] + framesize;
      claim(delta >= 0);

      Opnd dst = BaseDispOpnd(opnd_reg_sp, opnd_immed(delta, type_s32));
      mi = new_instr_alm(dst, STQ, opnd_reg_gp);
      header().push_front(mi);			// stq $gp,__gp_home__
    }
  }
  // Create trailer instructions for procedure exit.  Restore saved regs.
  //
  clear(trailer());
  sr_offset = 8;				// Start running offset past $ra

  NatSetIter gpsrs(saved_reg_set[CLASS_GPR].iter());
  for ( ; gpsrs.is_valid(); gpsrs.next()) {
    int r = gpsrs.current();

    if (r == RA)
      os = 0;
    else {
      os = sr_offset;
      sr_offset += 8;			// bytes per register
    }
    Opnd disp =
      opnd_immed(framesize + frameoffset + os, type_s32);

    mi = new_instr_alm(opnd_reg(r, type_u64),
        LDQ, BaseDispOpnd(opnd_reg_sp, disp, type_u64));
    trailer().push_back(mi);
  }

  claim(((framesize + frameoffset + sr_offset) & 7) == 0);  // 8-byte aligned

  // Load floating-point saved registers.  sr_offset continues from where
  // last GP saved reg left off.
  //
  for (fpsrs = saved_reg_set[CLASS_FPR].iter();
      fpsrs.is_valid(); fpsrs.next()) {
    int r = fpsrs.current();
    Opnd disp =
      opnd_immed(framesize + frameoffset + sr_offset, type_s32);
    mi = new_instr_alm(opnd_reg(r, type_f64),
        LDT, BaseDispOpnd(opnd_reg_sp, disp, type_f64));
    trailer().push_back(mi);
    sr_offset += 8;				// bytes per register
  }

  // create, if needed --			lda $sp,framesize($sp)
  if (framesize) {
    Opnd disp = opnd_immed(framesize, type_s32);
    mi = new_instr_alm(opnd_reg_sp, LDA,
        BaseDispOpnd(opnd_reg_sp, disp));
    trailer().push_back(mi);
  }
}
