/* file "ia64/opcodes.h" */

/*
    Copyright (c) 2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef IA64_OPCODES_H
#define IA64_OPCODES_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "ia64/opcodes.h"
#endif

/*
 * Define identifiers for the complete set of ia64 assembly opcodes,
 * pseudo-ops, opcode extensions, and code-generator/scheduler pseudo-ops.  
 *
 * To add new opcodes, add a new enum whose identifier values begin beyond
 * the end of the current enum (specified by LAST_IA64_OPCODE_STEM), then
 * redefine LAST_IA64_OPCODE_STEM.  Also, enroll new data into the opcode-
 * indexed data tables.
 */
namespace ia64 {

enum {
    //opcode_null = 0,	defined in machine/opcodes.h
    //opcode_label,	defined in machine/opcodes.h

    // Assembler directive mnemonics
    ALIAS = 2,		// alias declaration directives
    SECALIAS,		// 
    PREDREL,		// assembler annotations
    PREDVECTOR,		// 
    MEMOFFSET,		// 
    ENTRY,		// 
    AUTO,		// assembler modes
    EXPLICIT,		// 
    DEFAULT,		// 
    MSB,		// byte order specification directive
    LSB,		// 
    COMMON,		// common symbol declaration directives
    LCOMM,		// 
    XDATA1,		// cross-section data allocation statements
    XDATA2,		// 
    XDATA4,		// 
    XDATA8,		// 
    XSTRING,		// 
    XSTRINGZ,		// 
    DATA1,		// data-allocation statements data1
    DATA2,		// 
    DATA4,		// 
    DATA8,		// 
    REAL4,		// 
    REAL8,		// 
    REAL10,		// 
    REAL16,		// 
    STRING,		// 
    STRINGZ,		// 
    MII,		// explicit template selection directives
    MFI,		// 
    BBB,		// 
    MLX,		// 
    MIB,		// 
    MMB,		// 
    MMI,		// 
    MBB,		// 
    MFB,		// 
    MMF,		// 
    FILE,		// file symbol declaration directive
    IDENT,		// ident string directive
    INCLUDE,		// include file directive
    HANDLERDATA,	// language specific data directive (NT specific)
    PROC,		// procedure declaration directives
    ENDP,		// 
    RADIX,		// radix indicator directive
    REGSTK,		// register stack directive
    SKIP,		// reserving uninitialized space statements
    ORG,		// 
    ROTR,		// rotating register directives
    ROTP,		// 
    ROTF,		// 
    SECTION,		// section directives
    PUSHSECTION,	// 
    POPSECTION,		// 
    PREVIOUS,		// 
    TEXT,		// 
    DATA,		// 
    SDATA,		// 
    BSS,		// 
    SBSS,		// 
    RODATA,		// 
    COMMENT,		// 
    ALIGN,		// section and data alignment directive
    GLOBAL,		// symbol scope declaration directives
    WEAK,		// 
    LOCAL,		// 
    PROTECTED,		// symbol visibility directives
    HIDDEN,		// 
    TYPE,		// symbol type and size directives
    SIZE,		// 
    LN,			// symbolic debug directive
    BF,			// symbolic debug directive (NT specific)
    EF,			// 
    PROLOGUE,		//
    BODY,		//
    FFRAME,		//
    SAVE,		// 
    RESTORE,		//
    SPILL,		//
    VREGALLOCATABLE,	// virtual register allocation directives
    VREGSAFEACROSSCALLS,// 
    VREGFAMILY,		// 
    VREGVAR,		// 
    VREGUNDEF,		// 

    // Load, store, move, set and exchange (ALM) instructions
    CMPXCHG,	       	// compare and exchange
    LD,		       	// load
    LFETCH,	       	// line prefetch
    LOADRS,	       	// load register stack
    MOV_AR,	       	// move application register
    MOV_BR,	       	// move branch register
    MOV_CR,	       	// move control register
    MOV_FR,	       	// move floating-point register
    MOV_GR,	       	// move general register
    MOV_IMM,	       	// move immediate
    MOV_INDIRECT,      	// move indirect register
    MOV_IP,	       	// move instruction pointer
    MOV_PR,	       	// move predicates
    MOV_PSR,	       	// move processor status register
    MOV_UM,	       	// move user mask
    MOVL,	      	// move long immediate
    ST,		       	// store
    XCHG,	       	// exchange

    // General computational (ALM) instructions
    ADD,	       	// addition
    ADDS,	       	// addition
    ADDL,	       	// addition
    ADDP4,	       	// add pointer
    AND,	       	// logical and
    ANDCM,	       	// and complement
    CMP,	       	// compare
    CMP4,	       	// compare word
    CZX,	       	// compute zero index
    DEP,	       	// deposit
    EXTR,	       	// extract
    FETCHADD,	       	// fetch and add immediate
    MIX,	       	// mix
    MUX,	       	// mux
    OR,		       	// logical or
    PACK,	       	// pack
    PADD,	       	// parallel add
    PAVG,	       	// parallel average
    PAVGSUB,	       	// parallel average subtract
    PCMP,	       	// parallel compare
    PMAX,	       	// parallel maximum
    PMIN,	       	// parallel minimum
    PMPY,	       	// parallel multiply
    PMPYSHR,	       	// parallel multiply and shift right
    POPCNT,	       	// population count
    PSAD,	       	// parallel sum of absolute difference
    PSHL,	       	// parallel shift left
    PSHLADD,	       	// parallel shift left and add
    PSHR,	       	// parallel shift right
    PSHRADD,	       	// parallel shift right and add
    PSUB,	       	// parallel subtract
    SHL,	       	// shift left
    SHLADD,	       	// shift left and add
    SHLADDP4,	       	// shift left and add pointer
    SHR,	       	// shift right
    SHRP,	       	// shift right pair
    SUB,	       	// subtract
    SXT,	       	// sign extend
    TBIT,	       	// test bit
    TNAT,	       	// test NaT
    TPA,	       	// translate to physical address
    TTAG,	       	// translation hashed entry tag
    UNPACK,	       	// unpack
    XMA,	       	// fixed-point multiply add
    XMPY,	       	// fixed-point multiply 
    XOR,	       	// exclusive or
    ZXT,       		// zero	extend (65)

    // Branch (CTI) instructions
    BR,			// branch
    BRL,	       	// branch long
    RFI,	       	// return from interruption

    // Special (ALM)
    ALLOC,	       	// allocate stack frame
    BREAK,	       	// break
    BRP,	       	// branch predict
    BSW,	       	// bank switch
    CHK,	       	// speculation check
    CLRRRB,	       	// clear RRB
    COVER,	       	// cover stack frame
    EPC,	       	// enter privileged code
    FC,		       	// flush cache
    FLUSHRS,	       	// flush register stack
    FWB,	       	// flush write buffers
    INVALA,	       	// invalidate ALAT
    ITC,	       	// insert translation cache
    ITR,	       	// insert translation register
    MF,		       	// memory fence
    NOP,	       	// no operation
    PROBE,	       	// probe access
    PTC_E,	       	// purge translation cache entry
    PTC_G,	      	// purge global translation cache (global)
    PTC_GA,	       	// purge global translation cache (global alat)
    PTC_L,	       	// purge local translation cache 
    PTR,	       	// purge translation register 
    RSM,	       	// reset system mask
    RUM,	       	// reset user mask
    SRLZ,	       	// serialize
    SSM,	       	// set system mask
    SUM,	       	// set user mask
    TAK,	       	// translation access key
    SYNC,	       	// memory synchronization
    THASH,	       	// translation hashed entry address (98)

    // Floating-point load/store (ALM) instructions
    LDF,	       	// floating-point load
    LDFP,	       	// floating-point load pair
    STF,	       	// floating-point store

    // Floating-point computational (ALM) instructions
    FABS,	       	// floating-point absolute value
    FADD,	       	// floating-point add
    FAMAX,	       	// floating-point absolute maximum
    FAMIN,	       	// floating-point absolute minimum
    FAND,	       	// floating-point logical and
    FANDCM,	       	// floating-point and complement
    FCMP,	       	// floating-point compare
    FMA,	       	// floating-point multiply add
    FMAX,	       	// floating-point maximum
    FMERGE,	       	// floating-point merge
    FMIN,	       	// floating-point minimum
    FMIX,	       	// floating-point mix
    FMPY,	       	// floating-point multiply
    FMS,	       	// floating-point multiply subtract
    FNEG,	       	// floating-point negate
    FNEGABS,	       	// floating-point negate absolute value
    FNMA,	       	// floating-point negative multiply add
    FNMPY,	       	// floating-point negative multiply
    FNORM,	       	// floating-point normalize
    FOR,	       	// floating-point logical or
    FPABS,	       	// floating-point absolute value
    FPACK,	       	// floating-point pack
    FPAMAX,	       	// floating-point parallel absolute maximum
    FPAMIN,	       	// floating-point parallel absolute minimum
    FPCMP,	       	// floating-point parallel compare
    FPMA,	       	// floating-point parallel multiply add
    FPMAX,	       	// floating-point parallel maximum
    FPMERGE,	       	// floating-point parallel merge
    FPMIN,	       	// floating-point parallel minimum
    FPMPY,	       	// floating-point parallel multiply
    FPMS,	       	// floating-point parallel multiply subtract
    FPNEG,	       	// floating-point parallel negate
    FPNEGABS,	       	// floating-point parallel negate absolute value
    FPNMA,	       	// floating-point parallel negative multiply add
    FPNMPY,	       	// floating-point parallel negative multiply 
    FPRCPA,	       	// floating-point parallel reciprocal approximation
    FPRSQRTA,	       	// floating-point parallel reciprocal sqrt approximation
    FRCPA,	       	// floating-point reciprocal approximation
    FRSQRTA,	       	// floating-point reciprocal sqrt approximation
    FSELECT,	       	// floating-point select
    FSUB,	       	// floating-point subtract
    FSWAP,	       	// floating-point swap
    FSXT,	       	// floating-point sign extend
    FXOR,	       	// floating-point exclusive or
    GETF,	       	// get floating-point value or exponent or significand
    SETF,	       	// set floating-point value, exponent, or significand

    // Floating-point special (ALM) instructions
    FCHKF,	       	// floating-point check flags
    FCLASS,	       	// floating-point class
    FCLRF,	       	// floating-point clear flags
    FCVT_FX,	       	// convert floating-point to signed integer
    FCVT_FXU,	       	// convert floating-point to unsigned integer
    FCVT_XF,	       	// convert signed integer to floating-point
    FCVT_XUF,	       	// convert unsigned integer to floating-point
    FPCVT_FX,	       	// convert parallel floating-point to integer
    FSETC	       	// floating-point set controls

};

} // namespace ia64

#define LAST_IA64_OPCODE_STEM ia64::FSETC

extern Vector<char*> ia64_opcode_stem_names;

// following used only in ia64/init.cc
void init_ia64_opcode_stem_names();

char *opcode_stem_names_ia64(int opcode);
char *opcode_name_ia64(int opcode);


// Defined opcode completers.

namespace ia64 {
enum {

    // branch type (btype) completers 
    // used by (BR,BRL)
    BTYPE_NONE,		// conditional branch
    BTYPE_COND,		// conditional branch
    BTYPE_CALL,		// conditional procedure call
    BTYPE_RET,		// conditional procedure return
    BTYPE_IA,		// invoke ia-32 instruction set
    BTYPE_CLOOP,	// counted loop branch
    BTYPE_CTOP,		// modulo scheduled counted loop
    BTYPE_CEXIT,	// modulo scheduled counted epilog
    BTYPE_WTOP,		// modulo scheduled while loop
    BTYPE_WEXIT,	// modulo scheduled while epilog

    // branch whether hint (bwh) completers
    // used by (BR,BRL)
    BWH_SPNT,		// static not taken
    BWH_SPTK,		// static taken
    BWH_DPNT,		// dynamic not taken
    BWH_DPTK,		// dynamic taken

    // sequential prefetch hint (ph) completers
    // used by (BR,BRL)
    PH_NONE,		// auto prefetch
    PH_FEW,		// few lines
    PH_MANY,		// many lines

    // branch cache deallocation hint (dh) completers
    // used by (BR,BRL)
    DH_NONE,		// don't deallocate
    DH_CLR,		// deallocate branch information

    // format completer
    // used by (BREAK,NOP)
    _FMT_NONE,		// pseudo-op
    _FMT_I,		// i-unit form
    _FMT_B,		// b-unit form
    _FMT_M,		// m-unit form
    _FMT_F,		// f-unit form
    _FMT_X,		// x-unit form

    // branch return
    // used by (BRP, MOV_BR)
    _RET_NONE,		// blank
    _RET_RET,		// branch is a return

    // ip-relative branch predict whether hint (ipwh) completers
    // used by (BRP)
    IPWH_SPTK,		// presaged branch should be predicted static taken
    IPWH_LOOP,		// presaged branch will be br.cloop br.ctop or br.wtop
    IPWH_EXIT,		// presaged branch will be br.cexit or br.wexit
    IPWH_DPTK,		// presaged branch should be predicted dynamically

    // indirect branch predict whether hint (indwh) completers
    // used by (BRP)
    INDWH_SPTK,		// presaged branch should be predicted static taken
    INDWH_DPTK,		// presaged branch should be predicted dynamically

    // importance hint (ih) completers 
    // used by (BRP, MOV_BR)
    IH_NONE,		// less important
    IH_IMP,		// more important

    // zero/one form indicator
    // used by (BSW)
    _BANK_ZERO,		// bank zero
    _BANK_ONE,		// bank one

    // control/data form indicator
    // used by (CHK)
    _SA_FORM_S,		// control form
    _SA_FORM_A,		// data form

    // i-unit/m-unit form indicator
    // used by (CHK, MOV_AR)
    _IM_FORM_NONE,	// unspecified form
    _IM_FORM_I,		// i-form
    _IM_FORM_M,		// m-form

    // ALAT clear (aclr) completers
    // used by (CHK)
    ACLR_CLR,		// invalidate matching ALAT entry 
    ACLR_NC,		// don't invalidate

    // pred/all form indicator 
    // used by (CLRRRB)
    _CLR_ALL,		// clear all register rename bases
    _CLR_PR,		// just clear CRM.rrb.pr

    // Comparison types (ctype) completers 
    // used by (CMP, CMP4, TBIT, TNAT) 
    CTYPE_NONE,		// default behavior
    CTYPE_UNC,		// initializes preds to zero first
    CTYPE_OR,		// set preds to logical or
    CTYPE_AND,		// set preds to logical and
    CTYPE_ORANDCM,	// set preds to logical or and complement
    CTYPE_ORCM,		// pseudo op
    CTYPE_ANDCM,	// pseudo op
    CTYPE_ANDORCM,	// pseudo op

    // Comparison relations (crel) completers
    // used by (CMP,CMP4)
    CREL_EQ,		// equal
    CREL_NEQ,		// not equal
    CREL_LT,		// signed less than
    CREL_LE,		// signed less than or equal to
    CREL_GT,		// signed greater than
    CREL_GE,		// signed greater than or equal to
    CREL_LTU,		// unsigned less than
    CREL_LEU,		// unsigned less than or equal to
    CREL_GTU,		// unsigned greater than
    CREL_GEU,		// unsigned greater than or equal to

    // size (sz) completers  (no preceeding dot)
    // used by (CMPXCHG, XCHG, LD, ST) 1/2/4/8
    // used by (CZX, MUX, PAVG, PAVGSUB, PMAX, PMIN) 1/2 bytes
    // used by (FETCHADD) 4/8
    // used by (PACK PSHL PSHR) 2/4
    // used by (MIX, PADD, PCMP, PSUB) 1/2/4
    SZ_1, 		// 1 byte accessed
    SZ_2, 		// 2 bytes accessed
    SZ_4, 		// 4 bytes accessed
    SZ_8, 		// 8 bytes accessed

    // compare and exchange semaphore types (sem) completers 
    // used by (CMPXCHG, FETCHADD)
    SEM_ACQ,		// Memory R/W is visible prior to subsq data accesses
    SEM_REL,		// Memory R/W is visible after all prev data accesses

    // left/right form 
    // used by (CZX, FSXT, MIX, PMPY)
    _LR_FORM_L,		// left form
    _LR_FORM_R,		// right form

    // merge/zero form 
    // used by (DEP)
    _MZ_FORM_NONE,	// merge form
    _MZ_FORM_Z,		// zero form

    // signed/unsigned form 
    // used by (EXTR, FCVT_FX, FPCVT_FX, PMAX, PMIN, PMPYSHR, PSHR, SHR)
    _SU_FORM_NONE,	// signed form
    _SU_FORM_U,		// unsigned form

    // trunc form
    // used by (FCVT_FX, FPCVT_FX)
    _ROUND_NONE,	// don't truncate
    _ROUND_TRUNC,	// truncate

    // specified pc mnemonic values (pc) completers
    // used by (FADD,FCVT_XUF,FMA,FMPY,FMS,FNMA,FNMPY,FNORM,FSUB)
    PC_NONE,		// dynamic precision (from pc value in status field)
    PC_S,		// single precision
    PC_D,		// double precision

    // sf mnemonic values (sf) completers 
    // used by (FADD,FAMAX,FAMIN,FCHKF,FCLRF,FCMP,FCVT_FX,FCVT_XUF,FMA,
    // FMAX,FMIN,FMPY,FMS,FNMA,FNMPY,FNORM,FPAMAX,FPAMIN,FPCMP,FPCVT_FX,
    // FPMA,FPMAX,FPMIN,FPMPY,FPMS,FPNMA,FPNMPY,FPRCPA,FPRSQRTA,FRCPA,
    // FRSQRTA,FSETC,FSUB) 
    SF_NONE,		// default (same as s0)
    SF_S0,		// sf0 status field accessed
    SF_S1,		// sf1 status field accessed
    SF_S2,		// sf2 status field accessed
    SF_S3,		// sf3 status field accessed

    // floating point class relations (fcrel) completers
    // used by (FCLASS)
    FCREL_M,		// is a member
    FCREL_NM,		// is not a member

    // floating point comparison types (fctype) completers
    // used by (FCLASS, FCMP)
    FCTYPE_NONE,	// see Vol3: 2-52 for table (normal mode)
    FCTYPE_UNC,		// see Vol3: 2-52 for table

    // floating point classes (fclass9) completers
    // used by (FCLASS)
    FCLASS_NAT,		// NaTVal 		0x0100 @nat
    FCLASS_QNAN,	// Quiet NaN		0x080  @qnan
    FCLASS_SNAN,	// Signaling NaN	0x040  @snan
    FCLASS_POS,		// Positive		0x001  @pos
    FCLASS_NEG,		// Negative		0x002  @neg
    FCLASS_ZERO,	// Zero			0x004  @zero
    FCLASS_UNORM,	// Unnormalized		0x008  @unorm
    FCLASS_NORM,	// Normalized		0x010  @norm
    FCLASS_INF,		// Infinity		0x020  @inf

    // floating point comparison relations (frel) completers
    // used by (FCMP)
    FREL_EQ,		// equal
    FREL_LT,		// less than
    FREL_LE,		// less than or equal
    FREL_GT,		// greater than
    FREL_GE,		// greater than or equal
    FREL_UNORD,		// unordered
    FREL_NEQ,		// not equal
    FREL_NLT,		// not less than
    FREL_NLE,		// not less than or equal
    FREL_NGT,		// not greater than
    FREL_NGE,		// not greater than or equal
    FREL_ORD,		// ordered

    // neg/sign/signexp form
    // used by (FMERGE, FPMERGE)
    _SIGN_NS,		// negated sign form
    _SIGN_S,		// sign form
    _SIGN_SE,		// sign exponent form

    // mix l/r/lr form
    // used by (FMIX)
    _MIX_L,		// mix left form
    _MIX_R,		// mix right form
    _MIX_LR,		// mix left-right form
 
    // swap form
    // used by (FSWAP)
    _SWAP_NONE,		// swap form
    _SWAP_NL,		// negated left swap form
    _SWAP_NR,		// negated right swap form
 
    // single/double/exponent/significand form
    // used by (GETF, SETF)
    _SD_FORM_S,		// single form
    _SD_FORM_D,		// double form
    _SD_FORM_EXP,	// exponent form
    _SD_FORM_SIG,	// significand form

    // entry form
    // used by (INVALA)
    _E_FORM_NONE,	// everything invaladated
    _E_FORM_E,		// matching ALAT entries invaladated

    // instruction/data form
    // used by (ITC, ITR, PTR, SRLZ)
    _ID_FORM_I,		// instruction form
    _ID_FORM_D,		// data form
 
    // Load types (ldtype) completers
    // used by (LD) 
    LDTYPE_NONE,	// normal load
    LDTYPE_S,		// speculative load
    LDTYPE_A,		// advanced load
    LDTYPE_SA,		// speculative advanced load
    LDTYPE_C_NC,	// check load, no clear
    LDTYPE_C_CLR,	// check load, clear
    LDTYPE_C_CLR_ACQ,	// ordered check load, clear
    LDTYPE_ACQ,		// ordered load
    LDTYPE_BIAS,	// biased load
    _LDTYPE_FILL,	// fill (only applicable to ld8 or ldf)

    // Load hints (ldhint) completers
    // used by (LD, CMPXCHG, FETCHADD, LDF, LDFP, XCHG)
    LDHINT_NONE,	// temporal locality, level 1
    LDHINT_NT1,		// no temporal locality, level 1
    LDHINT_NTA,		// no temporal locality, all levels

    // Floating point size (fsz) completers
    // used by (LDF, STF) s/d/e/8
    // used by (LDFP) s/d/8
    _FSZ_NONE,		// not specified
    FSZ_S,		// single precision: 4 bytes
    FSZ_D,		// double precision: 8 bytes
    FSZ_E,		// extended precision: 10 bytes
    _FSZ_8,		// see 2-214: seems wrong, but I'm following the manual

    // exclusive form
    // used by (lfetch)
    _CACHE_NONE,	// cache not marked exclusive
    _CACHE_EXCL,	// cache marked exclusive

    // Floating point load types (fldtype) completers
    // used by (LDF, LDFP)
    FLDTYPE_NONE,	// normal load 
    FLDTYPE_S,		// speculative load 
    FLDTYPE_A,		// advanced load 
    FLDTYPE_SA,		// speculative advanced load 
    FLDTYPE_C_NC,	// check load, no clear
    FLDTYPE_C_CLR,	// check load, clear
    _FLDTYPE_FILL,	// fill (only applicable to ldf)

    // line fault type (lftype) completers
    // used by (LFETCH PROBE)
    LFTYPE_NONE,	// ignore faults
    LFTYPE_FAULT,	// raise faults

    // line prefetch hint (lfhint) completers
    // used by (LFETCH)
    LFHINT_NONE,	// temporal locality, level 1
    LFHINT_NT1,		// no temporal locality, level 1
    LFHINT_NT2,		// no temporal locality, level 2
    LFHINT_NTA,		// no temporal locality, all levels

    // ordering/acceptance form
    // used by (MF)
    _ORD_NONE,		// ordering form
    _ORD_A,		// acceptance form

    // move to branch whether hints (mwh) completers
    // used by (MOV BR)
    MWH_NONE,		// ignore all hints
    MWH_SPTK,		// static taken
    MWH_DPTK,		// dynamic

    // indirect register file (ireg) completers
    // used by (MOV INDIRECT)
    IREG_CPUID,		// processor identification register
    IREG_DBR,		// data breakpoint register
    IREG_IBR,		// instruction breakpoint register
    IREG_PKR,		// protection key register
    IREG_PMC,		// performance monitor configuration register
    IREG_PMD,		// performance monitor data register
    IREG_RR,		// region register

    // mux permutations for 8-bit elements (mbtype) completers
    // used by (MUX)
    MBTYPE_REV,		// reverse the order of the bytes
    MBTYPE_MIX,		// perform a mix operation on the two halves of r2
    MBTYPE_SHUF,	// perform a shuffle operation on the two halves of r2
    MBTYPE_ALT,		// perform an alternate oper'n on the two halves of r2
    MBTYPE_BRCST,	// perform a broadcast oper'n on least sig byteof r2

    // saturation form
    // used by (PADD, PSUB) none/sss/uus/uuu
    // used by (PACK) sss/uss
    _SAT_NONE,		// modulo form
    _SAT_SSS,		// sss saturation form
    _SAT_USS,		// unsigned saturation form
    _SAT_UUS,		// uus saturation form
    _SAT_UUU,		// uuu saturation form

    // raz/normal form
    // used by (PAVG)
    _RAZ_NONE,		// normal form
    _RAZ_RAZ,		// round away from zero

    // Pcmp relations (prel) completers
    // used by (PCMP)
    PREL_EQ,		// equal
    PREL_GT,		// greater than

    // read/write form
    // used by (PROBE)
    _RW_FORM_R,		// read form
    _RW_FORM_W,		// write form
    _RW_FORM_RW,	// read/write form


    // store types (sttype) completers
    // used by (ST)
    STTYPE_NONE,	// normal store
    STTYPE_REL,		// ordered store
    _STTYPE_SPILL,	// 8-byte value spilled

    // store hints (sthint) completers
    // used by (ST, STF)
    STHINT_NONE,	// temporal locality, level 1
    STHINT_NTA,		// non-temporal locality, all levels

    // extend size (xsz) completers
    // used by (SXT, ZXT)
    XSZ_1,		// bit position 7
    XSZ_2,		// bit position 15
    XSZ_4,		// bit position 31

    // test bit relations (trel) completers
    // used by (TBIT, TNAT)
    TREL_NZ,		// selected bit == 1
    TREL_Z,		// selected bit == 0

    // low/high/unsigned form
    // used by (UNPACK) l/h
    // used by (XMA, XMPY) l/lu/h/hu
    _LHU_FORM_L,		// low form
    _LHU_FORM_LU,		// low unsigned form
    _LHU_FORM_H,		// high form
    _LHU_FORM_HU,		// high unsigned form

};

} // namespace ia64

#define LAST_IA64_OPCODE_EXT ia64::_LHU_FORM_HU

extern Vector<char*> ia64_opcode_ext_names;

// following used only in ia64/init.cc
void init_ia64_opcode_ext_names();

char *opcode_ext_names_ia64(int opcode);

bool target_implements_ia64(int opcode);

int opcode_line_ia64();
int opcode_ubr_ia64();
int opcode_move_ia64(TypeId);
int opcode_load_ia64(TypeId);
int opcode_store_ia64(TypeId);
bool is_breg(Opnd);
bool is_preg(Opnd);

int opcode_cbr_inverse_ia64(int opcode);

/* MACROS for opcodes */
/*
 * opcode = 32 bits
 *
 * Reg Instrs:
 * ext3(8bits) | ext2(8bits) | ext1(8bits) | stem(8bits)
 *
 * BR Instrs:
 * ext4(6bits) | ext3(6bits) | ext2(6bits) | ext1(6bits) | stem(8bits)
 *
 *
 */

#define EMPTY 0 //For "No Completer"

/* stem is 8 bits, just mask out exts */
#define get_stem(opc) \
        (opc & 0x000000ff)

/* typical opcode has 8bit stem and three 8bit completers */
#define make_opcode(stem, ext1, ext2, ext3) \
        ((ext3 << 24) | (ext2 << 16) | (ext1 << 8) | (stem))

/* br/brl opcode has 8bit stem and four 6bit completers */
#define make_br_opcode(stem, ext1, ext2, ext3, ext4) \
        ((ext4 << 26) | (ext3 << 20) | (ext2 << 14) | (ext1 << 8) | (stem))

/* ext is 8 bits, shift and mask to find it */
#define get_ext(opc, whichext) \
        ((opc >> (whichext*8)) & 0x000000ff)

/* but for br, ext is 6 bits */
#define get_br_ext(opc, whichext) \
        ((opc >> (((whichext-1)*6)+8)) & 0x0000003f)

/* shift out the stem to see if exts are encoded */
#define has_exts(opc) \
        ((opc >> 8) != 0)


#endif /* IA64_OPCODES_H */
