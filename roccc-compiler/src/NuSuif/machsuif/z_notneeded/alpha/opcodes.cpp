/* file "alpha/opcodes.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "alpha/opcodes.h"
#endif

#include <machine/machine.h>

#include <alpha/opcodes.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace alpha;

// -------- Opcodes --------

Vector<char*> alpha_opcode_names(LAST_ALPHA_OPCODE + 1);

void
init_alpha_opcode_names()
{
    alpha_opcode_names[opcode_null] = "";
    alpha_opcode_names[opcode_label] = "";

    // Pseudo-opcode (mi_dot) instructions
    alpha_opcode_names[AENT]	      = ".aent";
    alpha_opcode_names[ALIAS]	      = ".alias";
    alpha_opcode_names[ALIGN]	      = ".align";
    alpha_opcode_names[ASCII]	      = ".ascii";
    alpha_opcode_names[ASCIIZ]	      = ".asciiz";
    alpha_opcode_names[BGNB]	      = ".bgnb";
    alpha_opcode_names[BYTE]	      = ".byte";
    alpha_opcode_names[COMM]	      = ".comm";
    alpha_opcode_names[DATA]	      = ".data";
    alpha_opcode_names[D_FLOATING]    = ".d_floating";
    alpha_opcode_names[DOUBLE]	      = ".double";
    alpha_opcode_names[EDATA]	      = ".edata";
    alpha_opcode_names[EFLAG]	      = ".eflag";
    alpha_opcode_names[END]	      = ".end";
    alpha_opcode_names[ENDB]	      = ".endb";
    alpha_opcode_names[ENDR]	      = ".endr";
    alpha_opcode_names[ENT]	      = ".ent";
    alpha_opcode_names[ERR]	      = ".err";
    alpha_opcode_names[EXTENDED]      = ".extended";
    alpha_opcode_names[EXTERN]	      = ".extern";
    alpha_opcode_names[F_FLOATING]    = ".f_floating";
    alpha_opcode_names[alpha::FILE]   = ".file";
    alpha_opcode_names[FLOAT]	      = ".float";
    alpha_opcode_names[FMASK]	      = ".fmask";
    alpha_opcode_names[FNOP]	      = "fnop";
    alpha_opcode_names[FRAME]	      = ".frame";
    alpha_opcode_names[G_FLOATING]    = ".g_floating";
    alpha_opcode_names[GJSRLIVE]      = ".gjsrlive";
    alpha_opcode_names[GJSRSAVED]     = ".gjsrsaved";
    alpha_opcode_names[GLOBL]	      = ".globl";
    alpha_opcode_names[GPREL32]	      = ".gprel32";
    alpha_opcode_names[GRETLIVE]      = ".gretlive";
    alpha_opcode_names[DOTLAB]	      = ".lab";
    alpha_opcode_names[LCOMM]	      = ".lcomm";
    alpha_opcode_names[LIVEREG]	      = ".livereg";
    alpha_opcode_names[LOC]	      = ".loc";
    alpha_opcode_names[LONG]	      = ".long";
    alpha_opcode_names[MASK]	      = ".mask";
    alpha_opcode_names[NOALIAS]	      = ".noalias";
    alpha_opcode_names[NOP]	      = "nop";
    alpha_opcode_names[OPTION]	      = ".option";
    alpha_opcode_names[PROLOGUE]      = ".prologue";
    alpha_opcode_names[QUAD]	      = ".quad";
    alpha_opcode_names[RDATA]	      = ".rdata";
    alpha_opcode_names[REPEAT]	      = ".repeat";
    alpha_opcode_names[SAVE_RA]	      = ".save_ra";
    alpha_opcode_names[SDATA]	      = ".sdata";
    alpha_opcode_names[SET]	      = ".set";
    alpha_opcode_names[S_FLOATING]    = ".s_floating";
    alpha_opcode_names[SPACE]	      = ".space";
    alpha_opcode_names[STRUCT]	      = ".struct";
    alpha_opcode_names[TEXT]	      = ".text";
    alpha_opcode_names[T_FLOATING]    = ".t_floating";
    alpha_opcode_names[UGEN]	      = ".ugen";
    alpha_opcode_names[UNOP]	      = "unop";
    alpha_opcode_names[VERSTAMP]      = ".verstamp";
    alpha_opcode_names[VREG]	      = ".vreg";
    alpha_opcode_names[WEAKEXT]	      = ".weakext";
    alpha_opcode_names[WORD]	      = ".word";
    alpha_opcode_names[X_FLOATING]    = ".x_floating";
    
    // Load immediate (mi_alm) instructions
    alpha_opcode_names[LDIL]	      = "ldil";
    alpha_opcode_names[LDIQ]	      = "ldiq";

    // Load (mi_alm) instructions
    alpha_opcode_names[LDA]	      = "lda";
    alpha_opcode_names[LDAH]	      = "ldah";
    alpha_opcode_names[LDGP]	      = "ldgp";
    alpha_opcode_names[LDB]	      = "ldb";
    alpha_opcode_names[LDBU]	      = "ldbu";
    alpha_opcode_names[LDW]	      = "ldw";
    alpha_opcode_names[LDWU]	      = "ldwu";
    alpha_opcode_names[LDL]	      = "ldl";
    alpha_opcode_names[LDL_L]	      = "ldl_l";
    alpha_opcode_names[LDQ]	      = "ldq";
    alpha_opcode_names[LDQ_L]	      = "ldq_l";
    alpha_opcode_names[LDQ_U]	      = "ldq_u";
    alpha_opcode_names[ULDW]	      = "uldw";
    alpha_opcode_names[ULDWU]	      = "uldwu";
    alpha_opcode_names[ULDL]	      = "uldl";
    alpha_opcode_names[ULDQ]	      = "uldq";

    // Store (mi_alm) instructions
    alpha_opcode_names[STB]	      = "stb";
    alpha_opcode_names[STW]	      = "stw";
    alpha_opcode_names[STL]	      = "stl";
    alpha_opcode_names[STL_C]	      = "stl_c";
    alpha_opcode_names[STQ]	      = "stq";
    alpha_opcode_names[STQ_C]	      = "stq_c";
    alpha_opcode_names[STQ_U]	      = "stq_u";
    alpha_opcode_names[USTW]	      = "ustw";
    alpha_opcode_names[USTL]	      = "ustl";
    alpha_opcode_names[USTQ]	      = "ustq";

    // General computational (mi_alm) instructions
    alpha_opcode_names[ABSL]	      = "absl";
    alpha_opcode_names[ABSQ]	      = "absq";
    alpha_opcode_names[ADDL]	      = "addl";
    alpha_opcode_names[ADDLV]	      = "addlv";
    alpha_opcode_names[ADDQ]	      = "addq";
    alpha_opcode_names[ADDQV]	      = "addqv";
    alpha_opcode_names[AND]	      = "and";
    alpha_opcode_names[ANDNOT]	      = "andnot";
    alpha_opcode_names[BIC]	      = "bic";
    alpha_opcode_names[BIS]	      = "bis";
    alpha_opcode_names[CLR]	      = "clr";
    alpha_opcode_names[CMOVEQ]	      = "cmoveq";
    alpha_opcode_names[CMOVNE]	      = "cmovne";
    alpha_opcode_names[CMOVLT]	      = "cmovlt";
    alpha_opcode_names[CMOVLE]	      = "cmovle";
    alpha_opcode_names[CMOVGT]	      = "cmovgt";
    alpha_opcode_names[CMOVGE]	      = "cmovge";
    alpha_opcode_names[CMOVLBC]	      = "cmovlbc";
    alpha_opcode_names[CMOVLBS]	      = "cmovlbs";
    alpha_opcode_names[CMPEQ]	      = "cmpeq";
    alpha_opcode_names[CMPLT]	      = "cmplt";
    alpha_opcode_names[CMPLE]	      = "cmple";
    alpha_opcode_names[CMPULT]	      = "cmpult";
    alpha_opcode_names[CMPULE]	      = "cmpule";
    alpha_opcode_names[DIVL]	      = "divl";
    alpha_opcode_names[DIVLU]	      = "divlu";
    alpha_opcode_names[DIVQ]	      = "divq";
    alpha_opcode_names[DIVQU]	      = "divqu";
    alpha_opcode_names[EQV]	      = "eqv";
    alpha_opcode_names[MOV]	      = "mov";
    alpha_opcode_names[MULL]	      = "mull";
    alpha_opcode_names[MULLV]	      = "mullv";
    alpha_opcode_names[MULQ]	      = "mulq";
    alpha_opcode_names[MULQV]	      = "mulqv";
    alpha_opcode_names[NEGL]	      = "negl";
    alpha_opcode_names[NEGLV]	      = "neglv";
    alpha_opcode_names[NEGQ]	      = "negq";
    alpha_opcode_names[NEGQV]	      = "negqv";
    alpha_opcode_names[NOT]	      = "not";
    alpha_opcode_names[OR]	      = "or";
    alpha_opcode_names[ORNOT]	      = "ornot";
    alpha_opcode_names[REML]	      = "reml";
    alpha_opcode_names[REMLU]	      = "remlu";
    alpha_opcode_names[REMQ]	      = "remq";
    alpha_opcode_names[REMQU]	      = "remqu";
    alpha_opcode_names[S4ADDL]	      = "s4addl";
    alpha_opcode_names[S4ADDQ]	      = "s4addq";
    alpha_opcode_names[S8ADDL]	      = "s8addl";
    alpha_opcode_names[S8ADDQ]	      = "s8addq";
    alpha_opcode_names[S4SUBL]	      = "s4subl";
    alpha_opcode_names[S4SUBQ]	      = "s4subq";
    alpha_opcode_names[S8SUBL]	      = "s8subl";
    alpha_opcode_names[S8SUBQ]	      = "s8subq";
    alpha_opcode_names[SETXL]	      = "setxl";
    alpha_opcode_names[SLL]	      = "sll";
    alpha_opcode_names[SRA]	      = "sra";
    alpha_opcode_names[SRL]	      = "srl";
    alpha_opcode_names[SUBL]	      = "subl";
    alpha_opcode_names[SUBLV]	      = "sublv";
    alpha_opcode_names[SUBQ]	      = "subq";
    alpha_opcode_names[SUBQV]	      = "subqv";
    alpha_opcode_names[UMULH]	      = "umulh";
    alpha_opcode_names[XOR]	      = "xor";
    alpha_opcode_names[XORNOT]	      = "xornot";

    // Byte-manipulation (mi_alm) instructions
    alpha_opcode_names[CMPBGE]	      = "cmpbge";
    alpha_opcode_names[EXTBL]	      = "extbl";
    alpha_opcode_names[EXTWL]	      = "extwl";
    alpha_opcode_names[EXTLL]	      = "extll";
    alpha_opcode_names[EXTQL]	      = "extql";
    alpha_opcode_names[EXTWH]	      = "extwh";
    alpha_opcode_names[EXTLH]	      = "extlh";
    alpha_opcode_names[EXTQH]	      = "extqh";
    alpha_opcode_names[INSBL]	      = "insbl";
    alpha_opcode_names[INSWL]	      = "inswl";
    alpha_opcode_names[INSLL]	      = "insll";
    alpha_opcode_names[INSQL]	      = "insql";
    alpha_opcode_names[INSWH]	      = "inswh";
    alpha_opcode_names[INSLH]	      = "inslh";
    alpha_opcode_names[INSQH]	      = "insqh";
    alpha_opcode_names[MSKBL]	      = "mskbl";
    alpha_opcode_names[MSKWL]	      = "mskwl";
    alpha_opcode_names[MSKLL]	      = "mskll";
    alpha_opcode_names[MSKQL]	      = "mskql";
    alpha_opcode_names[MSKWH]	      = "mskwh";
    alpha_opcode_names[MSKLH]	      = "msklh";
    alpha_opcode_names[MSKQH]	      = "mskqh";
    alpha_opcode_names[ZAP]	      = "zap";
    alpha_opcode_names[ZAPNOT]	      = "zapnot";

    // Branch (mi_cti) instructions
    alpha_opcode_names[BR]	      = "br";
    alpha_opcode_names[BEQ]	      = "beq";
    alpha_opcode_names[BEQ_N]	      = "beq_N";
    alpha_opcode_names[BEQ_T]	      = "beq_T";
    alpha_opcode_names[BGE]	      = "bge";
    alpha_opcode_names[BGE_N]	      = "bge_N";
    alpha_opcode_names[BGE_T]	      = "bge_T";
    alpha_opcode_names[BGT]	      = "bgt";
    alpha_opcode_names[BGT_N]	      = "bgt_N";
    alpha_opcode_names[BGT_T]	      = "bgt_T";
    alpha_opcode_names[BLBC]	      = "blbc";
    alpha_opcode_names[BLBC_N]	      = "blbc_N";
    alpha_opcode_names[BLBC_T]	      = "blbc_T";
    alpha_opcode_names[BLBS]	      = "blbs";
    alpha_opcode_names[BLBS_N]	      = "blbs_N";
    alpha_opcode_names[BLBS_T]	      = "blbs_T";
    alpha_opcode_names[BLE]	      = "ble";
    alpha_opcode_names[BLE_N]	      = "ble_N";
    alpha_opcode_names[BLE_T]	      = "ble_T";
    alpha_opcode_names[BLT]	      = "blt";
    alpha_opcode_names[BLT_N]	      = "blt_N";
    alpha_opcode_names[BLT_T]	      = "blt_T";
    alpha_opcode_names[BNE]	      = "bne";
    alpha_opcode_names[BNE_N]	      = "bne_N";
    alpha_opcode_names[BNE_T]	      = "bne_T";
    alpha_opcode_names[BSR]	      = "bsr";

    // Jump (mi_cti) instructions
    alpha_opcode_names[JMP]	      = "jmp";
    alpha_opcode_names[JSR]	      = "jsr";
    alpha_opcode_names[JSR_COROUTINE] = "jsr_coroutine";
    alpha_opcode_names[RET]	      = "ret";

    // Special (mi_alm or mi_cti) instructions
    alpha_opcode_names[CALL_PAL]      = "call_pal";
    alpha_opcode_names[FETCH]	      = "fetch";
    alpha_opcode_names[FETCH_M]	      = "fetch_m";
    alpha_opcode_names[RPCC]	      = "rpcc";
    alpha_opcode_names[TRAPB]	      = "trapb";
    alpha_opcode_names[EXCB]	      = "excb";
    alpha_opcode_names[MB]	      = "mb";
    alpha_opcode_names[WMB]	      = "wmb";

    // Floating point load (mi_alm) instructions
    alpha_opcode_names[LDF]	      = "ldf";
    alpha_opcode_names[LDG]	      = "ldg";
    alpha_opcode_names[LDS]	      = "lds";
    alpha_opcode_names[LDT]	      = "ldt";

    // Floating point load immediate (mi_alm) instructions
    alpha_opcode_names[LDIF]	      = "ldif";
    alpha_opcode_names[LDID]	      = "ldid";
    alpha_opcode_names[LDIG]	      = "ldig";
    alpha_opcode_names[LDIS]	      = "ldis";
    alpha_opcode_names[LDIT]	      = "ldit";

    // Floating point store (mi_alm) instructions
    alpha_opcode_names[STF]	      = "stf";
    alpha_opcode_names[STG]	      = "stg";
    alpha_opcode_names[STS]	      = "sts";
    alpha_opcode_names[STT]	      = "stt";

    // Floating point computational (mi_alm) instructions
    alpha_opcode_names[ADDF]	      = "addf";
    alpha_opcode_names[ADDG]	      = "addg";
    alpha_opcode_names[ADDS]	      = "adds";
    alpha_opcode_names[ADDT]	      = "addt";
    alpha_opcode_names[FCMOVEQ]	      = "fcmoveq";
    alpha_opcode_names[FCMOVNE]	      = "fcmovne";
    alpha_opcode_names[FCMOVLT]	      = "fcmovlt";
    alpha_opcode_names[FCMOVLE]	      = "fcmovle";
    alpha_opcode_names[FCMOVGT]	      = "fcmovgt";
    alpha_opcode_names[FCMOVGE]	      = "fcmovge";
    alpha_opcode_names[CPYS]	      = "cpys";
    alpha_opcode_names[CPYSN]	      = "cpysn";
    alpha_opcode_names[CPYSE]	      = "cpyse";
    alpha_opcode_names[CVTQL]	      = "cvtql";
    alpha_opcode_names[CVTLQ]	      = "cvtlq";
    alpha_opcode_names[CVTGQ]	      = "cvtgq";
    alpha_opcode_names[CVTTQ]	      = "cvttq";
    alpha_opcode_names[CVTQF]	      = "cvtqf";
    alpha_opcode_names[CVTQG]	      = "cvtqg";
    alpha_opcode_names[CVTQS]	      = "cvtqs";
    alpha_opcode_names[CVTQT]	      = "cvtqt";
    alpha_opcode_names[CVTDG]	      = "cvtdg";
    alpha_opcode_names[CVTGD]	      = "cvtgd";
    alpha_opcode_names[CVTGF]	      = "cvtgf";
    alpha_opcode_names[CVTTS]	      = "cvtts";
    alpha_opcode_names[CVTST]	      = "cvtst";
    alpha_opcode_names[DIVF]	      = "divf";
    alpha_opcode_names[DIVG]	      = "divg";
    alpha_opcode_names[DIVS]	      = "divs";
    alpha_opcode_names[DIVT]	      = "divt";
    alpha_opcode_names[FABS]	      = "fabs";
    alpha_opcode_names[FCLR]	      = "fclr";
    alpha_opcode_names[FMOV]	      = "fmov";
    alpha_opcode_names[MULF]	      = "mulf";
    alpha_opcode_names[MULG]	      = "mulg";
    alpha_opcode_names[MULS]	      = "muls";
    alpha_opcode_names[MULT]	      = "mult";
    alpha_opcode_names[FNEG]	      = "fneg";
    alpha_opcode_names[NEGF]	      = "negf";
    alpha_opcode_names[NEGG]	      = "negg";
    alpha_opcode_names[NEGS]	      = "negs";
    alpha_opcode_names[NEGT]	      = "negt";
    alpha_opcode_names[SUBF]	      = "subf";
    alpha_opcode_names[SUBG]	      = "subg";
    alpha_opcode_names[SUBS]	      = "subs";
    alpha_opcode_names[SUBT]	      = "subt";

    // Floating point relational (mi_alm) instructions
    alpha_opcode_names[CMPGEQ]	      = "cmpgeq";
    alpha_opcode_names[CMPGLT]	      = "cmpglt";
    alpha_opcode_names[CMPGLE]	      = "cmpgle";
    alpha_opcode_names[CMPTEQ]	      = "cmpteq";
    alpha_opcode_names[CMPTLT]	      = "cmptlt";
    alpha_opcode_names[CMPTLE]	      = "cmptle";
    alpha_opcode_names[CMPTUN]	      = "cmptun";

    // Floating point control (mi_cti) instructions
    alpha_opcode_names[FBEQ]	      = "fbeq";
    alpha_opcode_names[FBNE]	      = "fbne";
    alpha_opcode_names[FBLT]	      = "fblt";
    alpha_opcode_names[FBLE]	      = "fble";
    alpha_opcode_names[FBGT]	      = "fbgt";
    alpha_opcode_names[FBGE]	      = "fbge";

    // Special floating point (mi_alm) instructions
    alpha_opcode_names[MF_FPCR]	      = "mf_fpcr";
    alpha_opcode_names[MT_FPCR]	      = "mt_fpcr";
}

char *opcode_name_alpha(int opcode)
{
    return alpha_opcode_names[opcode];
}


// -------- Opcode extensions --------

Vector<char*> alpha_opcode_ext_names(LAST_ALPHA_OPCODE_EXT + 1);

void
init_alpha_opcode_ext_names()
{
    // VAX/IEEE rounding mode qualifiers
    alpha_opcode_ext_names[ROUND_NORMAL]  = "";
    alpha_opcode_ext_names[ROUND_CHOPPED] = "c";
    alpha_opcode_ext_names[ROUND_P_INF]	  = "d";
    alpha_opcode_ext_names[ROUND_M_INF]	  = "m";

    // VAX/IEEE trap modes
    alpha_opcode_ext_names[TRAP_NONE]	  = "";
    alpha_opcode_ext_names[TRAP_U]	  = "u";
    alpha_opcode_ext_names[TRAP_S]	  = "s";
    alpha_opcode_ext_names[TRAP_SU]	  = "su";
    alpha_opcode_ext_names[TRAP_SUI]	  = "sui";

    // VAX/IEEE convert-to-integer trap modes
    alpha_opcode_ext_names[ITRAP_NONE]	  = "";
    alpha_opcode_ext_names[ITRAP_V]	  = "v";
    alpha_opcode_ext_names[ITRAP_S]	  = "s";
    alpha_opcode_ext_names[ITRAP_SV]	  = "sv";
    alpha_opcode_ext_names[ITRAP_SVI]	  = "svi";
}


// -------- Conditional-branch opcode inversions --------

Vector<int> alpha_invert_table(LAST_ALPHA_OPCODE + 1);

/* We waste lots of space and set only the entries that we care about. */
void
init_alpha_invert_table()
{
    for (int i = 0; i < LAST_ALPHA_OPCODE; i++)
	alpha_invert_table[i] = -1;

    alpha_invert_table[BEQ]  = BNE;
    alpha_invert_table[BGE]  = BLT;
    alpha_invert_table[BGT]  = BLE;
    alpha_invert_table[BLBC] = BLBS;
    alpha_invert_table[BLBS] = BLBC;
    alpha_invert_table[BLE]  = BGT;
    alpha_invert_table[BLT]  = BGE;
    alpha_invert_table[BNE]  = BEQ;

    alpha_invert_table[FBEQ] = FBNE;
    alpha_invert_table[FBNE] = FBEQ;
    alpha_invert_table[FBLT] = FBGE;
    alpha_invert_table[FBLE] = FBGT;
    alpha_invert_table[FBGT] = FBLE;
    alpha_invert_table[FBGE] = FBLT;
}; 

// -------- Other OPI functions --------

bool
target_implements_alpha(int opcode)
{
    return (unsigned)opcode <= (unsigned)LAST_ALPHA_OPCODE;
}


// -------- Code generation helper routines --------

int opcode_cbr_inverse_alpha(int opcode)
{
    int invop = alpha_invert_table[opcode];
    claim(invop >= 0);
    return invop;
}


int
opcode_line_alpha()
{
    return LOC;
}

int
opcode_ubr_alpha()
{
    return BR;
}

int
opcode_load_alpha(TypeId t)
{
    switch (get_bit_size(t)) {
      case 8:
	claim(!is_floating_point(t));
	if (is_integral(t))
	    return is_signed(t) ? LDB : LDBU;
	else
	    return LDBU;

      case 16:
	claim(!is_floating_point(t));
	if (is_integral(t))
	    return is_signed(t) ? LDW : LDWU;
	else
	    return LDWU;

      case 32:
	return (is_floating_point(t)) ? LDS : LDL;

      case 64:
	return (is_floating_point(t)) ? LDT : LDQ;

      default:
	fprint(stderr, t);
	claim(false, "opcode_load_alpha() -- unexpected load size");
    }
    return opcode_null;
}

int
opcode_store_alpha(TypeId t)
{
    switch (get_bit_size(t)) {
      case 8:
	claim(!is_floating_point(t));
	return STB;

      case 16:
	claim(!is_floating_point(t));
	return STW;

      case 32:
	return (is_floating_point(t)) ? STS : STL;

      case 64:
	return (is_floating_point(t)) ? STT : STQ;

      default:
	fprint(stderr, t);
	claim(false, "opcode_store_alpha() -- unexpected store size");
    }
    return opcode_null;
}

int
opcode_move_alpha(TypeId t)
{
    switch (get_bit_size(t)) {
      case 8:
      case 16:
	claim(!is_floating_point(t));
	return MOV;

      case 32:
      case 64:
	return (is_floating_point(t)) ? FMOV : MOV;

      default:
	fprint(stderr, t);
	claim(false, "opcode_move_alpha() -- unexpected size for move");
    }
    return opcode_null;
}
