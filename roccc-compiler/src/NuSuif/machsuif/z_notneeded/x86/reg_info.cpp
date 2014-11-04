/* file "x86/reg_info.cpp" */

/*
    Copyright (c) 2000-2002 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "x86/reg_info.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>

#include <x86/opcodes.h>
#include <x86/instr.h>
#include <x86/reg_info.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace x86;

static Vector<const char*>  reg_name_table;
static Vector<int>	    reg_width_table;
static Vector<NatSetDense>  reg_aliases_table;

static void
define_reg(unsigned reg, const char* name, int width, int count, ...)
{
    claim(reg == reg_name_table.size());
    claim(reg == reg_width_table.size());
    claim(reg == reg_aliases_table.size());

    reg_name_table.push_back(IdString(name).chars());
    reg_width_table.push_back(width);
    reg_aliases_table.push_back(NatSetDense());
    reg_aliases_table[reg].insert(reg);
    va_list ap;
    va_start(ap, count);
    for ( ; count > 0; --count)
	reg_aliases_table[reg].insert(va_arg(ap, int));
    va_end(ap);
}

static Vector<NatSetDense> class_members_table;

static void
define_class(RegClassId id, int count, ...)
{
    claim((unsigned)id == class_members_table.size());

    class_members_table.push_back(NatSetDense());
    va_list ap;
    va_start(ap, count);
    for ( ; count > 0; --count)
	class_members_table[id].insert(va_arg(ap, int));
    va_end(ap);
}

/*
 * This function fills in the arrays indexed by abstract register number.
 */
void
init_x86_reg_tables()
{
    define_reg(EAX,          "eax", 32, 3, AX, AL, AH);
    define_reg(EBX,          "ebx", 32, 3, BX, BL, BH);
    define_reg(ECX,          "ecx", 32, 3, CX, CL, CH);
    define_reg(EDX,          "edx", 32, 3, DX, DL, DH);

    define_reg(AX,            "ax", 16, 3, EAX, AL, AH);
    define_reg(BX,            "bx", 16, 3, EBX, BL, BH);
    define_reg(CX,            "cx", 16, 3, ECX, CL, CH);
    define_reg(DX,            "dx", 16, 3, EDX, DL, DH);

    define_reg(AL,            "al",  8, 2, EAX, AX);
    define_reg(BL,            "bl",  8, 2, EBX, BX);
    define_reg(CL,            "cl",  8, 2, ECX, CX);
    define_reg(DL,            "dl",  8, 2, EDX, DX);

    define_reg(AH,            "ah",  8, 2, EAX, AX);
    define_reg(BH,            "bh",  8, 2, EBX, BX);
    define_reg(CH,            "ch",  8, 2, ECX, CX);
    define_reg(DH,            "dh",  8, 2, EDX, DX);
                                    
    define_reg(ESI,          "esi", 32, 1, SI);
    define_reg(EDI,          "edi", 32, 1, DI);
    define_reg(SI,            "si", 16, 1, ESI);
    define_reg(DI,            "di", 16, 1, EDI);
                                    
    define_reg(CONST0,        "--", 32, 0);
    define_reg(ESP,          "esp", 32, 0);
    define_reg(EBP,          "ebp", 32, 0);
                                    
    define_reg(ES,            "es", 16, 0);
    define_reg(CS,            "cs", 16, 0);
    define_reg(SS,            "ss", 16, 0);
    define_reg(DS,            "ds", 16, 0);
    define_reg(FS,            "fs", 16, 0);
    define_reg(GS,            "gs", 16, 0);
                                       
    define_reg(EFLAGS,    "eflags", 32, 0);
    define_reg(EIP,          "eip", 32, 0); // instr ptr
    define_reg(FPCR,        "fpcr", 16, 0); // FP control
    define_reg(FPFLAGS,     "fpsr", 16, 0); // FP status
    define_reg(FPTW,        "fptw", 16, 0); // FP tag word
                                       
    define_reg(FP0,           "st", 80, 0);
    define_reg(FP0 + 1,    "st(1)", 80, 0);
    define_reg(FP0 + 2,    "st(2)", 80, 0);
    define_reg(FP0 + 3,    "st(3)", 80, 0);
    define_reg(FP0 + 4,    "st(4)", 80, 0);
    define_reg(FP0 + 5,    "st(5)", 80, 0);
    define_reg(FP0 + 6,    "st(6)", 80, 0);
    define_reg(FP0 + 7,    "st(7)", 80, 0);

    define_class(CLASS_EXI, 6, EAX, EBX, ECX, EDX, ESI, EDI);
    define_class(CLASS_XI,  6,  AX,  BX,  CX,  DX,  SI,  DI);
    define_class(CLASS_EX,  4, EAX, EBX, ECX, EDX);
    define_class(CLASS_X,   4,  AX,  BX,  CX,  DX);
    define_class(CLASS_LH,  8,  AL,  AH,  BL,  BH,  CL,  CH,  DL,  DH);
    define_class(CLASS_EI,  2,			   ESI, EDI);
    define_class(CLASS_I,   2,			    SI,  DI);
}

int
reg_count_x86()
{
    return LAST_X86_REG + 1;
}

const char*
reg_name_x86(int reg)
{
    claim((unsigned)reg < reg_name_table.size());
    return reg_name_table[reg];
}

int
reg_width_x86(int reg)
{
    claim((unsigned)reg < reg_width_table.size());
    return reg_width_table[reg];
}

const NatSet*
reg_aliases_x86(int reg)
{
    claim((unsigned)reg < reg_aliases_table.size());
    return &reg_aliases_table[reg];
}

const NatSet*
reg_members_x86(RegClassId id)
{
    static NatSetDense empty;

    if (id == REG_CLASS_NONE)
	return &empty;

    claim((unsigned)id < class_members_table.size());
    return &class_members_table[id];
}

const NatSet*
reg_allocables_x86(bool maximals)
{
    static bool ready = false;
    static NatSetDense all;
    static NatSetDense max;

    if (!ready) {
	ready = true;
	all += *reg_caller_saves_x86(false);
	all += *reg_callee_saves_x86(false);
	max += *reg_caller_saves_x86(true);
	max += *reg_callee_saves_x86(true);
    }
    return (maximals) ? &max : &all;
}

const NatSet*
reg_caller_saves_x86(bool maximals)
{
    static bool ready = false;
    static NatSetDense all;
    static NatSetDense max;

    if (!ready) {
	ready = true;
	all.insert(EAX);
	all.insert(AX);
	all.insert(AL);
	all.insert(AH);

	all.insert(ECX);
	all.insert(CX);
	all.insert(CL);
	all.insert(CH);

	all.insert(EDX);
	all.insert(DX);
	all.insert(DL);
	all.insert(DH);

	max.insert(EAX);
	max.insert(ECX);
	max.insert(EDX);
    }
    return (maximals) ? &max : &all;
}

/*
 * FIXME:
 *
 * When assigning the callee-saves registers, we should give preference to the
 * SI and DI resources before BX and friends.  This is because we'd like for
 * word/doubleword candidates to be assigned if possible to the registers that
 * only accept words/doublewords, leaving BL and BH available for byte
 * candidates.
 *
 * By demanding a set, rather than a sequence, here, the interface is forcing us
 * to lose useful order info.
 */
const NatSet*
reg_callee_saves_x86(bool maximals)
{
    static bool ready = false;
    static NatSetDense all;
    static NatSetDense max;

    if (!ready) {
	all.insert(ESI);
	all.insert(SI);

	all.insert(EDI);
	all.insert(DI);

	all.insert(EBX);
	all.insert(BX);
	all.insert(BL);
	all.insert(BH);

	max.insert(ESI);
	max.insert(EDI);
	max.insert(EBX);

	ready = true;
    }
    return (maximals) ? &max : &all;
}

int
reg_maximal_x86(int reg)
{
    static unsigned long submaximals;
    if (submaximals == 0)
	submaximals = (1<<AL) | (1<<AH) | (1<<AX)
		    | (1<<BL) | (1<<BH) | (1<<BX)
		    | (1<<CL) | (1<<CH) | (1<<CX)
		    | (1<<DL) | (1<<DH) | (1<<DX)
		    | (1<<SI) | (1<<DI);
    if (submaximals & (1<<reg)) {
	if (reg == AL || reg == AH || reg == AX)
	    return EAX;
	if (reg == BL || reg == BH || reg == BX)
	    return EBX;
	if (reg == CL || reg == CH || reg == CX)
	    return ECX;
	if (reg == DL || reg == DH || reg == DX)
	    return EDX;
	if (reg == SI)
	    return ESI;
	if (reg == DI)
	    return EDI;
	claim(false);
    }
    return reg;
}

InstrHandle
reg_fill_x86(Opnd dst, Opnd src, InstrHandle marker)
{
    Instr  *filler = new_instr_alm(dst, opcode_load(get_type(dst)), src);
    CfgNode *block = get_parent_node(*marker);

    claim(block != NULL, "reg_fill must be used on a CFG instruction");

    return insert_before(block, marker, filler);
}

InstrHandle
reg_spill_x86(Opnd dst, Opnd src, InstrHandle marker)
{
    Instr  *spiller = new_instr_alm(dst, opcode_store(get_type(src)), src);
    CfgNode *block  = get_parent_node(*marker);

    claim(block != NULL, "reg_spill must be used on a CFG instruction");

    return insert_after(block, marker, spiller);
}

int
reg_class_count_x86()
{
    return LAST_X86_CLASS + 1;
}

/*
 * Produce the ID of the intersection of two classes.
 */
RegClassId
reg_class_intersection_x86(RegClassId c1, RegClassId c2)
{
    const int nclasses = LAST_X86_CLASS + 1;

    const RegClassId EXI = CLASS_EXI;
    const RegClassId  XI = CLASS_XI;
    const RegClassId  EX = CLASS_EX;
    const RegClassId   X = CLASS_X;
    const RegClassId  LH = CLASS_LH;
    const RegClassId  EI = CLASS_EI;
    const RegClassId   I = CLASS_I;
    const RegClassId  __ = REG_CLASS_NONE;

    static RegClassId intersections[nclasses][nclasses] =
	//   EXI    XI	  EX	 X    LH    EI	   I

        { {  EXI,   __,   EX,   __,   __,   EI,   __  }, // EXI
          {   __,   XI,   __,    X,   __,   __,    I  }, //  XI
          {   EX,   __,   EX,   __,   __,   __,   __  }, // EXI
          {   __,    X,   __,    X,   __,   __,   __  }, //   X
          {   __,   __,   __,   __,   LH,   __,   __  }, //  LH
          {   EI,   __,   __,   __,   __,   EI,   __  }, //  EI
          {   __,    I,   __,   __,   __,   __,    I  }  //   I
	};

    if (c1 == REG_CLASS_NONE || c2 == REG_CLASS_NONE)
	return REG_CLASS_NONE;

    if (c1 == REG_CLASS_ANY)
	return c2;
    if (c2 == REG_CLASS_ANY)
	return c1;

    claim(c1 < nclasses && c2 < nclasses);

    return intersections[c1][c2];
}



/* -------------------  Implementation of reg_classify  ----------------- */

/*
 * Return a ptr to the entry for opnd in class_map, provided catalog is
 * willing to enroll opnd.  Otherwise, return NULL.
 */
static RegClassId*
class_map_entry(Opnd opnd, OpndCatalog *catalog, RegClassMap *class_map)
{
    int id = -1;
    catalog->enroll(opnd, &id);
    if (id < 0)
	return NULL;
    maybe_expand(*class_map, id, REG_CLASS_ANY);
    return &(*class_map)[id];
}

/*
 * Helper class for classify_opnd and reg_classify_x86: classify
 * address-register operands found in an address expression.
 */
class ClassifyAddr : public OpndFilter {
  public:
    ClassifyAddr(OpndCatalog *catalog, RegClassMap *class_map)
	: catalog(catalog), class_map(class_map) { }
    Opnd operator()(Opnd opnd, InOrOut)
    {
	if (RegClassId *entry = class_map_entry(opnd, catalog, class_map))
	    *entry = reg_class_intersection_x86(*entry, CLASS_EXI);
	return opnd;
    }

  protected:
    OpndCatalog *catalog;
    RegClassMap *class_map;
};

/*
 * Classify an operand based on its bit width (or its identity, if a hard
 * reg).  If it's an address expression, classify any register-candidate
 * suboperands as addresses.  Return class for opnd's type even if it is
 * not a register candidate.
 *
 * The argument operand is located from its containing instruction and a
 * position indicator which is -1 for dst0 and a non-negative src index
 * otherwise.
 *
 * The optional output parameter entry_ptr, if non-NULL, is used to store
 * indirectly a pointer to the entry in class_map for this operand.  This
 * pointer is NULL for address expressions and other operands that are not
 * assimilated by the operand catalog.
 */

enum { DST0 = -1, SRC0 = 0, SRC1 = 1 };

static RegClassId
classify_opnd(Instr *instr, int pos, OpndCatalog *catalog,
	      RegClassMap *class_map, RegClassId **entry_ptr = NULL)
{
    TypeId type;
    RegClassId *entry;
    RegClassId class_id = REG_CLASS_ANY;
    Opnd opnd = (pos < 0) ? get_dst(instr) : get_src(instr, pos);

    if (is_addr(opnd)) {
	entry = NULL;
	type = get_deref_type(opnd);

	if (is_addr_exp(opnd)) {
	    ClassifyAddr classify_addr(catalog, class_map);
	    map_opnds(opnd, classify_addr);
	}
    } else {
	entry = class_map_entry(opnd, catalog, class_map);
	if (is_hard_reg(opnd)) {

	    // We can't rely on the type of a hard register operand to give
	    // its bit width because register numbers are sometimes turned
	    // into operands without knowledge of the register semantics.

	    type = NULL;
	    switch (get_reg(opnd)) {
	      case EAX: case EBX: case ECX: case EDX:
		class_id = CLASS_EX;
		break;
	      case  AX: case  BX: case  CX: case  DX:
		class_id = CLASS_X;
		break;
	      case  AL: case  BL: case  CL: case  DL:
	      case  AH: case  BH: case  CH: case  DH:
		class_id = CLASS_LH;
		break;
	      case ESI: case EDI:
		class_id = CLASS_EI;
		break;
	      case  SI: case  DI:
		class_id = CLASS_I;
		break;
	      default:
		class_id = REG_CLASS_NONE;
	    }
	} else {
	    type = get_type(opnd);
	}
    }
    if (type) {
	int width = get_bit_size(type);
	class_id = (width == 32) ? CLASS_EXI :
		   (width == 16) ? CLASS_XI  :
		   (width ==  8) ? CLASS_LH  : REG_CLASS_ANY;
    }
    if (entry) {
	*entry = reg_class_intersection_x86(*entry, class_id);
	claim(*entry != REG_CLASS_NONE);
    }
    if (entry_ptr)
	*entry_ptr = entry;
    return class_id;
}



/*
 * classify_move:  classify MOV instruction
 *
 * Classify source and destination, then check for the following special
 * case.  When the destination has the 8-bit CLASS_LH and the source does
 * not, we constrain the source have CLASS_EX if it had CLASS_EXI or
 * CLASS_X if it had CLASS_XI.
 *
 * This is necessary because the code generator uses a plain MOV
 * instruction when it converts from a wider integral value to a narrower
 * one, as in
 *
 *    cvt s.32 -> s.8.
 *
 * It relies on the assembly printer to pick the right assembly opcode for
 * the destination width, so that the high-order part of wide value is
 * effectively trimmed by the MOV.
 *
 * E.g., the C statement:
 *
 *    putchar((char)(maxbits | block_compress));
 *
 * gives rise to:
 *
 *    movl      maxbits,$vr1
 *    orl       block_compress,$vr1
 *    movb      $vr1,$vr2
 *    movsbl    $vr2,$vr3
 *
 * Here $vr1 is a 32-bit candidate is being used as the source of an 8-bit
 * move in order to convert it to type char by extracting its low-order
 * byte.
 *
 * When the destination type is 16 bits wide, this hack places no
 * constraint on the class of the source operand (e.g., $vr1).  For an
 * 8-bit destination, the source is required to be in (E)AX, (E)BX, (E)CX,
 * or (E)DX.  Hence, we restrict the source operand to CLASS_EX or CLASS_X,
 * whichever is appropriate to its prior class.
 */
void
classify_move(Instr *instr, OpndCatalog *catalog, RegClassMap *class_map)
{
    RegClassId dst_id = classify_opnd(instr, DST0, catalog, class_map);

    RegClassId *src_entry;
    RegClassId src_id =
	classify_opnd(instr, SRC0, catalog, class_map, &src_entry);

    if ((src_entry != NULL) && (dst_id == CLASS_LH)) {
	if (src_id == CLASS_EXI)
	    *src_entry = CLASS_EX;
	else if (src_id == CLASS_XI)
	    *src_entry = CLASS_X;
    }
}

/*
 * reg_classify_x86
 *
 * Classify the operands of instr using logic derived from that of the
 * PrinterX86::print_opcode method.
 */
void
reg_classify_x86(Instr *instr, OpndCatalog *catalog, RegClassMap *class_map)
{
    switch (get_opcode(instr)) {
      case MOV:	  classify_move(instr, catalog, class_map); return;
      case CMP:
      case TEST:
      case SAR:
      case SHL:
      case SHR:	  classify_opnd(instr, SRC0, catalog, class_map);
		  classify_opnd(instr, SRC1, catalog, class_map);  return;
      case SETE:
      case SETNE:
      case SETL:
      case SETLE: classify_opnd(instr, DST0, catalog, class_map);  return;

      case CALL:  classify_opnd(instr, SRC0, catalog, class_map);  return;
      default:
	if (dsts_size(instr) == 0) {
	    if (srcs_size(instr) > 0)
		classify_opnd(instr, SRC0, catalog, class_map);
	    return;
	}
	classify_opnd(instr, DST0, catalog, class_map);

	if (is_two_opnd_x86(instr)) {
	    if (srcs_size(instr) >= 2)
		classify_opnd(instr, SRC1, catalog, class_map);
	} else if (srcs_size(instr) >= 1) {
	    classify_opnd(instr, SRC0, catalog, class_map);
	}
    }
}

/*
 * reg_choice_x86
 *
 * We break down the possible arguments as follows:
 *
 * o  `class_id' will be one of EXI, EX, XI, X, LH.  The other two classes
 *    are not for register candidates.
 *
 * o  `pool' can be exactly the caller_save set (TMP), or exactly the
 *    callee_save set (SAV), or an ad hoc selection from those sets (ANY).
 *
 * With five classes and three pools, we have fifteen cases.
 */

enum { MEMO_TMP_EXI = 0,
       MEMO_TMP_XI,
       MEMO_TMP_EX,
       MEMO_TMP_X,
       MEMO_TMP_LH,
       MEMO_SAV_EXI,
       MEMO_SAV_XI,
       MEMO_SAV_EX,
       MEMO_SAV_X,
       MEMO_SAV_LH,
       MEMO_ANY_EXI,
       MEMO_ANY_XI,
       MEMO_ANY_EX,
       MEMO_ANY_X,
       MEMO_ANY_LH,
       NUMBER_MEMO_CASES,
};

Vector<int> memo_members[NUMBER_MEMO_CASES];
int         memo_rotor  [NUMBER_MEMO_CASES];

/*
 * Helpers init_memo and init_memos fill the caches containing the
 * register sequences for the fifteen cases described above.
 */

void
init_memo(const NatSet *conv_pool, const NatSet *class_members,
	  int conv_case, int any_case)
{
    for (NatSetIter it = conv_pool->iter(); it.is_valid(); it.next()) {
	int reg = it.current();

	if (class_members->contains(reg)) {
	    memo_members[conv_case].push_back(reg);
	    memo_members[any_case] .push_back(reg);
	}
    }
}

void
init_memos()
{
    const NatSet *tmp_pool = reg_caller_saves_x86();
    const NatSet *sav_pool = reg_callee_saves_x86();

    init_memo(tmp_pool, reg_members_x86(CLASS_EXI), MEMO_TMP_EXI, MEMO_ANY_EXI);
    init_memo(tmp_pool, reg_members_x86(CLASS_XI),  MEMO_TMP_XI,  MEMO_ANY_XI);
    init_memo(tmp_pool, reg_members_x86(CLASS_EX),  MEMO_TMP_EX,  MEMO_ANY_EX);
    init_memo(tmp_pool, reg_members_x86(CLASS_X),   MEMO_TMP_X,	  MEMO_ANY_X);
    init_memo(tmp_pool, reg_members_x86(CLASS_LH),  MEMO_TMP_LH,  MEMO_ANY_LH);
    init_memo(sav_pool, reg_members_x86(CLASS_EXI), MEMO_SAV_EXI, MEMO_ANY_EXI);
    init_memo(sav_pool, reg_members_x86(CLASS_XI),  MEMO_SAV_XI,  MEMO_ANY_XI);
    init_memo(sav_pool, reg_members_x86(CLASS_EX),  MEMO_SAV_EX,  MEMO_ANY_EX);
    init_memo(sav_pool, reg_members_x86(CLASS_X),   MEMO_SAV_X,	  MEMO_ANY_X);
    init_memo(sav_pool, reg_members_x86(CLASS_LH),  MEMO_SAV_LH,  MEMO_ANY_LH);
}

int
reg_choice_x86(RegClassId class_id, const NatSet *pool,
	       const NatSet *excluded, bool rotate)
{
    claim(pool != NULL);
    if (class_id == REG_CLASS_NONE || class_id == REG_CLASS_ANY)
	return -1;

    claim((unsigned)class_id <= CLASS_EI);		// FIXME: allow EI and I

    static bool memoized = false;

    if (!memoized) {
	memoized = true;
	init_memos();
    }

    int memo_case;
    
    if (pool == reg_caller_saves_x86()) {
	memo_case = MEMO_TMP_EXI;
	pool = NULL;
    }
    else if (pool == reg_callee_saves_x86()) {
	memo_case = MEMO_SAV_EXI;
	pool = NULL;
    }
    else {
	memo_case = MEMO_ANY_EXI;
    }

    memo_case += class_id;

    Vector<int> &members = memo_members[memo_case];
    unsigned size = members.size();

    int start = rotate ? memo_rotor[memo_case] : 0;

    int k = start;
    do {
	int reg = members[k];
	k = (k + 1) % size;

	// reg must be in the given register pool and not the excluded set...
	if ((pool != NULL && !pool->contains(reg)) || excluded->contains(reg))
	    continue;				// ... else try next reg

	// success: if using round-robin, remember where to start next time
	if (rotate)
	    memo_rotor[memo_case] = k;
	return reg;

    } while (k != start);

    return -1;
}
