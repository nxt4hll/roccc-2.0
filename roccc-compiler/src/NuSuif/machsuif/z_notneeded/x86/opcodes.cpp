/* file "x86/opcodes.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "x86/opcodes.h"
#endif

#include <machine/machine.h>

#include <x86/opcodes.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace x86;

// -------- Opcodes --------

Vector<char *> x86_opcode_names(LAST_X86_OPCODE + 1);

void
init_x86_opcode_names()
{
    x86_opcode_names[opcode_null]  = "";
    x86_opcode_names[opcode_label] = "";

    // Pseudo-opcode (dot) instructions
    x86_opcode_names[ALIGN]        = ".align";
    x86_opcode_names[ASCII]        = ".ascii";
    x86_opcode_names[BYTE]         = ".byte";
    x86_opcode_names[COMM]         = ".comm";
    x86_opcode_names[DATA]         = ".data";
    x86_opcode_names[DOUBLE]       = ".double";
    x86_opcode_names[ENDR]         = ".endr";
    x86_opcode_names[OPCODE_FILE]  = ".file";
    x86_opcode_names[FLOAT]        = ".float";
    x86_opcode_names[GLOBL]        = ".globl";
    x86_opcode_names[LCOMM]        = ".lcomm";
    x86_opcode_names[LOC]          = "# .loc";
    x86_opcode_names[LONG]         = ".long";
    x86_opcode_names[REPEAT]       = ".rept";
    x86_opcode_names[TEXT]         = ".text";
    x86_opcode_names[WORD]         = ".word";

    // Load, move, store, set and exchange (ALM) instructions
    x86_opcode_names[LDS]          = "lds";
    x86_opcode_names[LEA]          = "lea";
    x86_opcode_names[LES]          = "les";
    x86_opcode_names[LFS]          = "lfs";
    x86_opcode_names[LGS]          = "lgs";
    x86_opcode_names[LSS]          = "lss";
    x86_opcode_names[LODS]         = "lods";
    x86_opcode_names[LODSB]        = "lodsb";
    x86_opcode_names[LODSD]        = "lodsd";
    x86_opcode_names[LODSW]        = "lodsw";
    x86_opcode_names[MOV]          = "mov";
    x86_opcode_names[MOVS]         = "movs";
    x86_opcode_names[MOVSB]        = "movsb";
    x86_opcode_names[MOVSD]        = "movsd";
    x86_opcode_names[MOVSL]        = "movsl"; // AT&T-style name for `movsd'
    x86_opcode_names[MOVSW]        = "movsw";
    x86_opcode_names[MOVSX]        = "movs";
    x86_opcode_names[MOVZX]        = "movz";
    x86_opcode_names[SETA]         = "seta";
    x86_opcode_names[SETAE]        = "setae";
    x86_opcode_names[SETB]         = "setb";
    x86_opcode_names[SETBE]        = "setbe";
    x86_opcode_names[SETC]         = "setc";
    x86_opcode_names[SETE]         = "sete";
    x86_opcode_names[SETG]         = "setg";
    x86_opcode_names[SETGE]        = "setge";
    x86_opcode_names[SETL]         = "setl";
    x86_opcode_names[SETLE]        = "setle";
    x86_opcode_names[SETNA]        = "setna";
    x86_opcode_names[SETNAE]       = "setnae";
    x86_opcode_names[SETNB]        = "setnb";
    x86_opcode_names[SETNBE]       = "setnbe";
    x86_opcode_names[SETNC]        = "setnc";
    x86_opcode_names[SETNE]        = "setne";
    x86_opcode_names[SETNG]        = "setng";
    x86_opcode_names[SETNGE]       = "setnge";
    x86_opcode_names[SETNL]        = "setnl";
    x86_opcode_names[SETNLE]       = "setnle";
    x86_opcode_names[SETNO]        = "setno";
    x86_opcode_names[SETNP]        = "setnp";
    x86_opcode_names[SETNS]        = "setns";
    x86_opcode_names[SETNZ]        = "setnz";
    x86_opcode_names[SETO]         = "seto";
    x86_opcode_names[SETP]         = "setp";
    x86_opcode_names[SETPE]        = "setpe";
    x86_opcode_names[SETPO]        = "setpo";
    x86_opcode_names[SETS]         = "sets";
    x86_opcode_names[SETZ]         = "setz";
    x86_opcode_names[STOS]         = "stos";
    x86_opcode_names[STOSB]        = "stosb";
    x86_opcode_names[STOSD]        = "stosd";
    x86_opcode_names[STOSW]        = "stosw";
    x86_opcode_names[XCHG]         = "xchg";
    x86_opcode_names[XLAT]         = "xlat";
    x86_opcode_names[XLATB]        = "xlatb";

    // General computational (ALM) instructions
    x86_opcode_names[ADC]          = "adc";
    x86_opcode_names[ADD]          = "add";
    x86_opcode_names[AND]          = "and";
    x86_opcode_names[BSF]          = "bsf";
    x86_opcode_names[BSR]          = "bsr";
    x86_opcode_names[BSWAP]        = "bswap";
    x86_opcode_names[BT]           = "bt";
    x86_opcode_names[BTC]          = "btc";
    x86_opcode_names[BTR]          = "btr";
    x86_opcode_names[BTS]          = "bts";
    x86_opcode_names[CBW]          = "cbtw";
    x86_opcode_names[CDQ]          = "cltd";
    x86_opcode_names[CMP]          = "cmp";
    x86_opcode_names[CMPS]         = "cmps";
    x86_opcode_names[CMPSB]        = "cmpsb";
    x86_opcode_names[CMPSW]        = "cmpsw";
    x86_opcode_names[CMPSD]        = "cmpsd";
    x86_opcode_names[CMPXCHG]      = "cmpxchg";
    x86_opcode_names[CWD]          = "cwtd";
    x86_opcode_names[CWDE]         = "cwtl";
    x86_opcode_names[DEC]          = "dec";
    x86_opcode_names[DIV]          = "div";
    x86_opcode_names[IDIV]         = "idiv";
    x86_opcode_names[IMUL]         = "imul";
    x86_opcode_names[INC]          = "inc";
    x86_opcode_names[MUL]          = "mul";
    x86_opcode_names[NEG]          = "neg";
    x86_opcode_names[NOT]          = "not";
    x86_opcode_names[OR]           = "or";
    x86_opcode_names[RCL]          = "rcl";
    x86_opcode_names[RCR]          = "rcr";
    x86_opcode_names[ROL]          = "rol";
    x86_opcode_names[ROR]          = "ror";
    x86_opcode_names[SAL]          = "sal";
    x86_opcode_names[SAR]          = "sar";
    x86_opcode_names[SBB]          = "sbb";
    x86_opcode_names[SHL]          = "shl";
    x86_opcode_names[SHR]          = "shr";
    x86_opcode_names[SCAS]         = "scas";
    x86_opcode_names[SCASB]        = "scasb";
    x86_opcode_names[SCASD]        = "scasd";
    x86_opcode_names[SCASW]        = "scasw";
    x86_opcode_names[SHLD]         = "shld";
    x86_opcode_names[SHRD]         = "shrd";
    x86_opcode_names[SUB]          = "sub";
    x86_opcode_names[TEST]         = "test";
    x86_opcode_names[XADD]         = "xadd";
    x86_opcode_names[XOR]          = "xor";

    // BCD math (ALM) instructions
    x86_opcode_names[AAA]          = "aaa";
    x86_opcode_names[AAD]          = "aad";
    x86_opcode_names[AAM]          = "aam";
    x86_opcode_names[AAS]          = "aas";
    x86_opcode_names[DAA]          = "daa";
    x86_opcode_names[DAS]          = "das";

    // Branch (mi_cti) instructions
    x86_opcode_names[JA]           = "ja";
    x86_opcode_names[JAE]          = "jae";
    x86_opcode_names[JB]           = "jb";
    x86_opcode_names[JBE]          = "jbe";
    x86_opcode_names[JC]           = "jc";
    x86_opcode_names[JE]           = "je";
    x86_opcode_names[JG]           = "jg";
    x86_opcode_names[JGE]          = "jge";
    x86_opcode_names[JL]           = "jl";
    x86_opcode_names[JLE]          = "jle";
    x86_opcode_names[JNA]          = "jna";
    x86_opcode_names[JNAE]         = "jnae";
    x86_opcode_names[JNB]          = "jnb";
    x86_opcode_names[JNBE]         = "jnbe";
    x86_opcode_names[JNC]          = "jnc";
    x86_opcode_names[JNE]          = "jne";
    x86_opcode_names[JNG]          = "jng";
    x86_opcode_names[JNGE]         = "jnge";
    x86_opcode_names[JNL]          = "jnl";
    x86_opcode_names[JNLE]         = "jnle";
    x86_opcode_names[JNO]          = "jno";
    x86_opcode_names[JNP]          = "jnp";
    x86_opcode_names[JNS]          = "jns";
    x86_opcode_names[JNZ]          = "jnz";
    x86_opcode_names[JO]           = "jo";
    x86_opcode_names[JP]           = "jp";
    x86_opcode_names[JPE]          = "jpe";
    x86_opcode_names[JPO]          = "jpo";
    x86_opcode_names[JS]           = "js";
    x86_opcode_names[JZ]           = "jz";

    x86_opcode_names[JCXZ]         = "jcxz";
    x86_opcode_names[JECXZ]        = "jecxz";
    x86_opcode_names[LOOP]         = "loop";
    x86_opcode_names[LOOPE]        = "loope";
    x86_opcode_names[LOOPNE]       = "loopne";
    x86_opcode_names[LOOPNZ]       = "loopnz";
    x86_opcode_names[LOOPZ]        = "loopz";

    // Jump (mi_cti) instructions
    x86_opcode_names[CALL]         = "call";
    x86_opcode_names[JMP]          = "jmp";

    // Flag manipulation (ALM) instructions
    x86_opcode_names[CLC]          = "clc";
    x86_opcode_names[CLD]          = "cld";
    x86_opcode_names[CLI]          = "cli";
    x86_opcode_names[CLTS]         = "clts";
    x86_opcode_names[CMC]          = "cmc";
    x86_opcode_names[POPF]         = "popf";
    x86_opcode_names[POPFD]        = "popfd";
    x86_opcode_names[PUSHF]        = "pushf";
    x86_opcode_names[PUSHFD]       = "pushfd";
    x86_opcode_names[LAHF]         = "lahf";
    x86_opcode_names[SAHF]         = "sahf";
    x86_opcode_names[STC]          = "stc";
    x86_opcode_names[STD]          = "std";
    x86_opcode_names[STI]          = "sti";

    // Memory management (ALM)
    x86_opcode_names[ARPL]         = "arpl";
    x86_opcode_names[BOUND]        = "bound";
    x86_opcode_names[LAR]          = "lar";
    x86_opcode_names[LGDT]         = "lgdt";
    x86_opcode_names[LIDT]         = "lidt";
    x86_opcode_names[LLDT]         = "lldt";
    x86_opcode_names[LSL]          = "lsl";
    x86_opcode_names[SGDT]         = "sgdt";
    x86_opcode_names[SIDT]         = "sidt";
    x86_opcode_names[SLDT]         = "sldt";
    x86_opcode_names[VERR]         = "verr";
    x86_opcode_names[VERW]         = "verw";

    // Stack (ALM)
    x86_opcode_names[ENTER]        = "enter";
    x86_opcode_names[LEAVE]        = "leave";
    x86_opcode_names[POP]          = "pop";
    x86_opcode_names[POPA]         = "popa";
    x86_opcode_names[POPAD]        = "popad";
    x86_opcode_names[PUSH]         = "push";
    x86_opcode_names[PUSHA]        = "pusha";
    x86_opcode_names[PUSHAD]       = "pushad";
    x86_opcode_names[RET]          = "ret";

    // Special (ALM)
    x86_opcode_names[HLT]          = "hlt";
    x86_opcode_names[IN]           = "in";
    x86_opcode_names[INS]          = "ins";
    x86_opcode_names[INSB]         = "insb";
    x86_opcode_names[INSD]         = "insd";
    x86_opcode_names[INSW]         = "insw";
    x86_opcode_names[INT]          = "int";
    x86_opcode_names[INTO]         = "into";
    x86_opcode_names[INVD]         = "invd";
    x86_opcode_names[INVLPG]       = "invlpg";
    x86_opcode_names[IRET]         = "iret";
    x86_opcode_names[IRETD]        = "iretd";
    x86_opcode_names[LMSW]         = "lmsw";
    x86_opcode_names[LTR]          = "ltr";
    x86_opcode_names[NOP]          = "nop";
    x86_opcode_names[OUT]          = "out";
    x86_opcode_names[OUTS]         = "outs";
    x86_opcode_names[OUTSB]        = "outsb";
    x86_opcode_names[OUTSD]        = "outsd";
    x86_opcode_names[OUTSW]        = "outsw";
    x86_opcode_names[SMSW]         = "smsw";
    x86_opcode_names[STR]          = "str";
    x86_opcode_names[WAIT]         = "wait";
    x86_opcode_names[WBINVD]       = "binvd";

    // Floating-point load immediate (ALM) instructions
    x86_opcode_names[FLD1]         = "fld1";
    x86_opcode_names[FLDL2T]       = "fldl2t";
    x86_opcode_names[FLDL2E]       = "fldl2e";
    x86_opcode_names[FLDPI]        = "fldpi";
    x86_opcode_names[FLDLG2]       = "fldlg2";
    x86_opcode_names[FLDLN2]       = "fldln2";
    x86_opcode_names[FLDZ]         = "fldz";
    x86_opcode_names[FXCH]         = "fxch";

    // Floating-point load/store (ALM) instructions
    x86_opcode_names[FBLD]         = "fbld";
    x86_opcode_names[FBSTP]        = "fbstp";
    x86_opcode_names[FILD]         = "fild";
    x86_opcode_names[FIST]         = "fist";
    x86_opcode_names[FISTP]        = "fistp";
    x86_opcode_names[FLD]          = "fld";
    x86_opcode_names[FST]          = "fst";
    x86_opcode_names[FSTP]         = "fstp";

    // Floating-point computational (ALM) instructions
    x86_opcode_names[F2XM1]        = "f2xm1";
    x86_opcode_names[FABS]         = "fabs";
    x86_opcode_names[FADD]         = "fadd";
    x86_opcode_names[FADDP]        = "faddp";
    x86_opcode_names[FCHS]         = "fchs";
    x86_opcode_names[FCOM]         = "fcom";
    x86_opcode_names[FCOMP]        = "fcomp";
    x86_opcode_names[FCOMPP]       = "fcompp";
    x86_opcode_names[FCOS]         = "fcos";
    x86_opcode_names[FDIV]         = "fdiv";
    x86_opcode_names[FDIVP]        = "fdivp";
    x86_opcode_names[FDIVR]        = "fdivr";
    x86_opcode_names[FDIVRP]       = "fdivrp";
    x86_opcode_names[FIADD]        = "fiadd";
    x86_opcode_names[FICOM]        = "ficom";
    x86_opcode_names[FICOMP]       = "ficomp";
    x86_opcode_names[FIDIV]        = "fidiv";
    x86_opcode_names[FIDIVR]       = "fidivr";
    x86_opcode_names[FISUB]        = "fisub";
    x86_opcode_names[FISUBR]       = "fisubr";
    x86_opcode_names[FMUL]         = "fmul";
    x86_opcode_names[FMULP]        = "fmulp";
    x86_opcode_names[FIMUL]        = "fimul";
    x86_opcode_names[FPATAN]       = "fpatan";
    x86_opcode_names[FPREM]        = "fprem";
    x86_opcode_names[FPREM1]       = "fprem1";
    x86_opcode_names[FPTAN]        = "fptan";
    x86_opcode_names[FRNDINT]      = "frndint";
    x86_opcode_names[FSCALE]       = "fscale";
    x86_opcode_names[FSIN]         = "fsin";
    x86_opcode_names[FSINCOS]      = "fsincos";
    x86_opcode_names[FSQRT]        = "fsqrt";
    x86_opcode_names[FSUB]         = "fsub";
    x86_opcode_names[FSUBP]        = "fsubp";
    x86_opcode_names[FSUBR]        = "fsubr";
    x86_opcode_names[FSUBRP]       = "fsubrp";
    x86_opcode_names[FTST]         = "ftst";
    x86_opcode_names[FUCOM]        = "fucom";
    x86_opcode_names[FUCOMP]       = "fucomp";
    x86_opcode_names[FUCOMPP]      = "fucompp";
    x86_opcode_names[FXTRACT]      = "fxtract";
    x86_opcode_names[FYL2X]        = "fyl2x";
    x86_opcode_names[FYL2XP1]      = "fyl2xp1";

    // Floating-point special (ALM) instructions
    x86_opcode_names[FCLEX]        = "fclex";
    x86_opcode_names[FDECSTP]      = "fdecstp";
    x86_opcode_names[FFREE]        = "ffree";
    x86_opcode_names[FINCSTP]      = "fincstp";
    x86_opcode_names[FINIT]        = "finit";
    x86_opcode_names[FLDCW]        = "fldcw";
    x86_opcode_names[FLDENV]       = "fldenv";
    x86_opcode_names[FNCLEX]       = "fnclex";
    x86_opcode_names[FNINIT]       = "fninit";
    x86_opcode_names[FNOP]         = "fnop";
    x86_opcode_names[FNSAVE]       = "fnsave";
    x86_opcode_names[FNSTCW]       = "fnstcw";
    x86_opcode_names[FNSTENV]      = "fnstenv";
    x86_opcode_names[FNSTSW]       = "fnstsw";
    x86_opcode_names[FRSTOR]       = "frstor";
    x86_opcode_names[FSAVE]        = "fsave";
    x86_opcode_names[FSTCW]        = "fstcw";
    x86_opcode_names[FSTENV]       = "fstenv";
    x86_opcode_names[FSTSW]        = "fstsw";
    x86_opcode_names[FWAIT]        = "fwait";
    x86_opcode_names[FXAM]         = "fxam";
}

char *opcode_name_x86(int opcode)
{
    return x86_opcode_names[opcode];
}


// -------- Opcode extensions --------

Vector<char *> x86_opcode_ext_names(LAST_X86_OPCODE_EXT + 1);

void
init_x86_opcode_ext_names()
{
    x86_opcode_ext_names[LOCK]  = "lock";

    x86_opcode_ext_names[REP]   = "rep";
    x86_opcode_ext_names[REPE]  = "repe";
    x86_opcode_ext_names[REPNE] = "repne";
    x86_opcode_ext_names[REPNZ] = "repnz";
    x86_opcode_ext_names[REPZ]  = "repz";
}


// -------- Conditional-branch opcode inversions --------

Vector<int> x86_invert_table(LAST_X86_OPCODE + 1);

/* We waste lots of space and set only the entries that we care about. */
void
init_x86_invert_table()
{
    for (int i = 0; i < LAST_X86_OPCODE; i++)
	x86_invert_table[i] = -1;

    x86_invert_table[JA]     = JNA;
    x86_invert_table[JAE]    = JNAE;
    x86_invert_table[JB]     = JNB;
    x86_invert_table[JBE]    = JNBE;
    x86_invert_table[JC]     = JNC;
    x86_invert_table[JE]     = JNE;
    x86_invert_table[JG]     = JNG;
    x86_invert_table[JGE]    = JNGE;
    x86_invert_table[JL]     = JNL;
    x86_invert_table[JLE]    = JNLE;
    x86_invert_table[JO]     = JNO;
    x86_invert_table[JP]     = JNP;
    x86_invert_table[JPE]    = JPO;
    x86_invert_table[JS]     = JNS;
    x86_invert_table[JZ]     = JNZ;
    
    x86_invert_table[LOOP]   = JECXZ;
    x86_invert_table[LOOPE]  = LOOPNE;
    x86_invert_table[LOOPZ]  = LOOPNZ;

    x86_invert_table[JNA]    = JA;
    x86_invert_table[JNAE]   = JAE;
    x86_invert_table[JNB]    = JB;
    x86_invert_table[JNBE]   = JBE;
    x86_invert_table[JNC]    = JC;
    x86_invert_table[JNE]    = JE;
    x86_invert_table[JNG]    = JG;
    x86_invert_table[JNGE]   = JGE;
    x86_invert_table[JNL]    = JL;
    x86_invert_table[JNLE]   = JLE;
    x86_invert_table[JNO]    = JO;
    x86_invert_table[JNP]    = JP;
    x86_invert_table[JPO]    = JPE;
    x86_invert_table[JNS]    = JS;
    x86_invert_table[JNZ]    = JZ;
    
    x86_invert_table[JECXZ]  = LOOP;
    x86_invert_table[LOOPNE] = LOOPE;
    x86_invert_table[LOOPNZ] = LOOPZ;
}; 

// -------- Other OPI functions --------

bool
target_implements_x86(int opcode)
{
    return (unsigned)opcode <= (unsigned)LAST_X86_OPCODE;
}


// -------- Code generation helper routines --------

int opcode_cbr_inverse_x86(int opcode)
{
    int invop = x86_invert_table[opcode];
    claim(invop >= 0);
    return invop;
}


int
opcode_line_x86()
{
    return LOC;
}

int
opcode_ubr_x86()
{
    return JMP;
}

int
opcode_load_x86(TypeId t)
{
    switch (get_bit_size(t)) {
      case 8:
      case 16:
	claim(!is_floating_point(t));
	return MOV;

      case 32:
	return (is_floating_point(t)) ? FLD : MOV;

      case 64:
	claim(is_floating_point(t),
	       "opcode_load_x86() -- can only load 64-bit FP numbers");
	return FLD;

      default:
	fprint(stderr, t);
	claim(false, "opcode_load_x86() -- unexpected load size");
    }
    return opcode_null;
}

int
opcode_store_x86(TypeId t)
{
    switch (get_bit_size(t)) {
      case 8:
      case 16:
	claim(!is_floating_point(t));
	return MOV;

      case 32:
	return (is_floating_point(t)) ? FSTP : MOV;

      case 64:
	claim(is_floating_point(t),
	       "opcode_store_x86() -- can only store 64-bit FP numbers");
	return FSTP;

      default:
	fprint(stderr, t);
	claim(false, "opcode_store_x86() -- unexpected store size");
    }
    return opcode_null;
}

int
opcode_move_x86(TypeId t)
{
    switch (get_bit_size(t)) {
      case 8:
      case 16:
	claim(!is_floating_point(t));
	return MOV;

      case 32:
	return (is_floating_point(t)) ? FSTP : MOV;

      case 64:
	claim(is_floating_point(t),
	       "opcode_move_x86() -- can only move 64-bit FP numbers");
	return FLD;

      default:
	claim(false, "opcode_move_x86() -- unexpected size for move");
    }
    return opcode_null;
}
