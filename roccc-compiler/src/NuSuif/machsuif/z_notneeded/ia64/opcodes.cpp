/* file "ia64/opcodes.cpp" */

/*
    Copyright (c) 2002 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "ia64/opcodes.h"
#endif

#include <machine/machine.h>

#include <ia64/opcodes.h>
#include <ia64/init.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace ia64;

// -------- Opcodes --------

Vector<char*> ia64_opcode_stem_names(LAST_IA64_OPCODE_STEM + 1);

void
init_ia64_opcode_stem_names()
{
    ia64_opcode_stem_names[opcode_null] = "";
    ia64_opcode_stem_names[opcode_label] = "";

    // Assembler directive mnemonics
    ia64_opcode_stem_names[ALIAS]	      = ".alias";
    ia64_opcode_stem_names[SECALIAS]	      = ".secalias";
    ia64_opcode_stem_names[PREDREL]	      = ".pred.rel";
    ia64_opcode_stem_names[PREDVECTOR]     = ".pred.vector";
    ia64_opcode_stem_names[MEMOFFSET]      = ".mem.offset";
    ia64_opcode_stem_names[ENTRY]          = ".entry";
    ia64_opcode_stem_names[AUTO]	      = ".auto";
    ia64_opcode_stem_names[EXPLICIT]	      = ".explicit";
    ia64_opcode_stem_names[DEFAULT]	      = ".default";
    ia64_opcode_stem_names[MSB]	      = ".msb";
    ia64_opcode_stem_names[LSB]	      = ".lsb";
    ia64_opcode_stem_names[COMMON]	      = ".common";
    ia64_opcode_stem_names[LCOMM]	      = ".lcomm";
    ia64_opcode_stem_names[XDATA1]	      = ".xdata1";
    ia64_opcode_stem_names[XDATA2]	      = ".xdata2";
    ia64_opcode_stem_names[XDATA4]	      = ".xdata4";
    ia64_opcode_stem_names[XDATA8]	      = ".xdata8";
    ia64_opcode_stem_names[XSTRING]	      = ".xstring";
    ia64_opcode_stem_names[XSTRINGZ]	      = ".xstringz";
    ia64_opcode_stem_names[DATA1]	      = "data1";
    ia64_opcode_stem_names[DATA2]	      = "data2";
    ia64_opcode_stem_names[DATA4]	      = "data4";
    ia64_opcode_stem_names[DATA8]	      = "data8";
    ia64_opcode_stem_names[REAL4]	      = "real4";
    ia64_opcode_stem_names[REAL8]	      = "real8";
    ia64_opcode_stem_names[REAL10]	      = "real10";
    ia64_opcode_stem_names[REAL16]	      = "real16";
    ia64_opcode_stem_names[STRING]	      = "string";
    ia64_opcode_stem_names[STRINGZ]	      = "stringz";
    ia64_opcode_stem_names[MII]	      = ".mii";
    ia64_opcode_stem_names[MFI]	      = ".mfi";
    ia64_opcode_stem_names[BBB]	      = ".bbb";
    ia64_opcode_stem_names[MLX]	      = ".mlx";
    ia64_opcode_stem_names[MIB]	      = ".mib";
    ia64_opcode_stem_names[MMB]	      = ".mmb";
    ia64_opcode_stem_names[MMI]	      = ".mmi";
    ia64_opcode_stem_names[MBB]	      = ".mbb";
    ia64_opcode_stem_names[MFB]	      = ".mfb";
    ia64_opcode_stem_names[MMF]	      = ".mmf";
    ia64_opcode_stem_names[ia64::FILE]	      = ".file";
    ia64_opcode_stem_names[IDENT]	      = ".ident";
    ia64_opcode_stem_names[INCLUDE]	      = ".include";
    ia64_opcode_stem_names[HANDLERDATA]    = ".handlerdata";
    ia64_opcode_stem_names[PROC]	      = ".proc";
    ia64_opcode_stem_names[ENDP]	      = ".endp";
    ia64_opcode_stem_names[RADIX]	      = ".radix";
    ia64_opcode_stem_names[REGSTK]	      = ".regstk";
    ia64_opcode_stem_names[SKIP]	      = ".skip";
    ia64_opcode_stem_names[ORG]	      = ".org";
    ia64_opcode_stem_names[ROTR]	      = ".rotr";
    ia64_opcode_stem_names[ROTP]	      = ".rotp";
    ia64_opcode_stem_names[ROTF]	      = ".rotf";
    ia64_opcode_stem_names[SECTION]	      = ".section";
    ia64_opcode_stem_names[PUSHSECTION]    = ".pushsection";
    ia64_opcode_stem_names[POPSECTION]     = ".popsection";
    ia64_opcode_stem_names[PREVIOUS]	      = ".previous";
    ia64_opcode_stem_names[TEXT]	      = ".text";
    ia64_opcode_stem_names[DATA]	      = ".data";
    ia64_opcode_stem_names[SDATA]	      = ".sdata";
    ia64_opcode_stem_names[BSS]	      = ".bss";
    ia64_opcode_stem_names[SBSS]	      = ".sbss";
    ia64_opcode_stem_names[RODATA]	      = ".rodata";
    ia64_opcode_stem_names[COMMENT]	      = ".comment";
    ia64_opcode_stem_names[ALIGN]	      = ".align";
    ia64_opcode_stem_names[GLOBAL]	      = ".global";
    ia64_opcode_stem_names[WEAK]	      = ".weak";
    ia64_opcode_stem_names[LOCAL]	      = ".local";
    ia64_opcode_stem_names[PROTECTED]      = ".protected";
    ia64_opcode_stem_names[HIDDEN]	      = ".hidden";
    ia64_opcode_stem_names[TYPE]	      = ".type";
    ia64_opcode_stem_names[SIZE]	      = ".size";
    ia64_opcode_stem_names[LN]	      = ".ln";
    ia64_opcode_stem_names[BF]	      = ".bf";
    ia64_opcode_stem_names[EF]	      = ".ef";
    ia64_opcode_stem_names[PROLOGUE]	= ".prologue";
    ia64_opcode_stem_names[BODY]	= ".body";
    ia64_opcode_stem_names[FFRAME]	= ".fframe";
    ia64_opcode_stem_names[SAVE]	= ".save";
    ia64_opcode_stem_names[RESTORE]	= ".restore";
    ia64_opcode_stem_names[SPILL]	= ".spill";
    ia64_opcode_stem_names[VREGALLOCATABLE]	      = ".vreg.allocatable";
    ia64_opcode_stem_names[VREGSAFEACROSSCALLS]    = ".vreg.safe_across_calls";
    ia64_opcode_stem_names[VREGFAMILY]     = ".vreg.family";
    ia64_opcode_stem_names[VREGVAR]	      = ".vreg.var";
    ia64_opcode_stem_names[VREGUNDEF]      = ".vreg.undef";

    // Load, store, move, set and exchange (ALM) instructions
    ia64_opcode_stem_names[CMPXCHG]        = "cmpxchg";
    ia64_opcode_stem_names[LD]             = "ld"; 
    ia64_opcode_stem_names[LFETCH]         = "lfetch";
    ia64_opcode_stem_names[LOADRS]         = "loadrs";
    ia64_opcode_stem_names[MOV_AR]         = "mov";
    ia64_opcode_stem_names[MOV_BR]         = "mov";
    ia64_opcode_stem_names[MOV_CR]         = "mov";
    ia64_opcode_stem_names[MOV_FR]         = "mov";
    ia64_opcode_stem_names[MOV_GR]         = "mov";
    ia64_opcode_stem_names[MOV_IMM]        = "mov";
    ia64_opcode_stem_names[MOV_INDIRECT]   = "mov";
    ia64_opcode_stem_names[MOV_IP]         = "mov";
    ia64_opcode_stem_names[MOV_PR]         = "mov";
    ia64_opcode_stem_names[MOV_PSR]        = "mov";
    ia64_opcode_stem_names[MOV_UM]         = "mov";
    ia64_opcode_stem_names[MOVL]           = "movl";
    ia64_opcode_stem_names[ST]             = "st";
    ia64_opcode_stem_names[XCHG]           = "xchg";

    // General computational (ALM) instructions
    ia64_opcode_stem_names[ADD]	      = "add";
    ia64_opcode_stem_names[ADDS]	      = "adds";
    ia64_opcode_stem_names[ADDL]	      = "addl";
    ia64_opcode_stem_names[ADDP4]	      = "addp4";
    ia64_opcode_stem_names[AND]     	      = "and";
    ia64_opcode_stem_names[ANDCM]          = "andcm";
    ia64_opcode_stem_names[CMP]            = "cmp";
    ia64_opcode_stem_names[CMP4]           = "cmp4";
    ia64_opcode_stem_names[CZX]            = "czx";
    ia64_opcode_stem_names[DEP]            = "dep";
    ia64_opcode_stem_names[EXTR]           = "extr";
    ia64_opcode_stem_names[FETCHADD]       = "fetchadd";
    ia64_opcode_stem_names[MIX]            = "mix";
    ia64_opcode_stem_names[MUX]            = "mux";
    ia64_opcode_stem_names[OR]             = "or";
    ia64_opcode_stem_names[PACK]           = "pack";
    ia64_opcode_stem_names[PADD]           = "padd";
    ia64_opcode_stem_names[PAVG]           = "pavg";
    ia64_opcode_stem_names[PAVGSUB]        = "pavgsub";
    ia64_opcode_stem_names[PCMP]           = "pcmp";
    ia64_opcode_stem_names[PMAX]           = "pmax";
    ia64_opcode_stem_names[PMIN]           = "pmin";
    ia64_opcode_stem_names[PMPY]           = "pmpy2";
    ia64_opcode_stem_names[PMPYSHR]        = "pmpyshr2";
    ia64_opcode_stem_names[POPCNT]         = "popcnt";
    ia64_opcode_stem_names[PSAD]           = "psad1";
    ia64_opcode_stem_names[PSHL]           = "pshl";
    ia64_opcode_stem_names[PSHLADD]        = "pshladd2";
    ia64_opcode_stem_names[PSHR]           = "pshr";
    ia64_opcode_stem_names[PSHRADD]        = "pshradd2";
    ia64_opcode_stem_names[PSUB]           = "psub";
    ia64_opcode_stem_names[SHL]            = "shl";
    ia64_opcode_stem_names[SHLADD]         = "shladd";
    ia64_opcode_stem_names[SHLADDP4]       = "shladdp4";
    ia64_opcode_stem_names[SHR]            = "shr";
    ia64_opcode_stem_names[SHRP]           = "shrp";
    ia64_opcode_stem_names[SUB]            = "sub";
    ia64_opcode_stem_names[SXT]            = "sxt";
    ia64_opcode_stem_names[TBIT]           = "tbit";
    ia64_opcode_stem_names[TNAT]           = "tnat";
    ia64_opcode_stem_names[TPA]            = "tpa";
    ia64_opcode_stem_names[TTAG]           = "ttag";
    ia64_opcode_stem_names[UNPACK]         = "unpack";
    ia64_opcode_stem_names[XMA]            = "xma";
    ia64_opcode_stem_names[XMPY]           = "xmpy";
    ia64_opcode_stem_names[XOR]            = "xor";
    ia64_opcode_stem_names[ZXT]            = "zxt";

    // Branch (CTI) instructions
    ia64_opcode_stem_names[BR]             = "br";
    ia64_opcode_stem_names[BRL]            = "brl";
    ia64_opcode_stem_names[RFI]            = "rfi";

   // Special (ALM) 
    ia64_opcode_stem_names[ALLOC]   	      = "alloc";       
    ia64_opcode_stem_names[BREAK]          = "break";
    ia64_opcode_stem_names[BRP]            = "brp";
    ia64_opcode_stem_names[BSW]            = "bsw";
    ia64_opcode_stem_names[CHK]            = "chk";
    ia64_opcode_stem_names[CLRRRB]         = "clrrrb";
    ia64_opcode_stem_names[COVER]          = "cover";
    ia64_opcode_stem_names[EPC]            = "epc";
    ia64_opcode_stem_names[FC]             = "fc";
    ia64_opcode_stem_names[FLUSHRS]        = "flushrs";
    ia64_opcode_stem_names[FWB]            = "fwb";
    ia64_opcode_stem_names[INVALA]         = "invala";
    ia64_opcode_stem_names[ITC]            = "itc";
    ia64_opcode_stem_names[ITR]            = "itr";
    ia64_opcode_stem_names[MF]             = "mf";
    ia64_opcode_stem_names[NOP]            = "nop";
    ia64_opcode_stem_names[PROBE]          = "probe";
    ia64_opcode_stem_names[PTC_E]          = "ptc.e";
    ia64_opcode_stem_names[PTC_G]          = "ptc.g";
    ia64_opcode_stem_names[PTC_GA]         = "ptc.ga";
    ia64_opcode_stem_names[PTC_L]          = "ptc.l";
    ia64_opcode_stem_names[PTR]            = "ptr";
    ia64_opcode_stem_names[RSM]            = "rsm";
    ia64_opcode_stem_names[RUM]            = "rum";
    ia64_opcode_stem_names[SRLZ]           = "srlz";
    ia64_opcode_stem_names[SSM]            = "ssm";
    ia64_opcode_stem_names[SUM]            = "sum";
    ia64_opcode_stem_names[TAK]            = "tak";
    ia64_opcode_stem_names[SYNC]           = "sync.i";
    ia64_opcode_stem_names[THASH]          = "thash";

    // Floating-point load/store (ALM) instructions
    ia64_opcode_stem_names[LDF]            = "ldf"; 
    ia64_opcode_stem_names[LDFP]           = "ldfp"; 
    ia64_opcode_stem_names[STF]            = "stf";

    // Floating-point computational (ALM) instructions
    ia64_opcode_stem_names[FABS]           = "fabs";
    ia64_opcode_stem_names[FADD]           = "fadd";
    ia64_opcode_stem_names[FAMAX]          = "famax";
    ia64_opcode_stem_names[FAMIN]          = "famin";
    ia64_opcode_stem_names[FAND]           = "fand";
    ia64_opcode_stem_names[FANDCM]         = "fandcm";
    ia64_opcode_stem_names[FCMP]           = "fcmp";
    ia64_opcode_stem_names[FMA]            = "fma";
    ia64_opcode_stem_names[FMAX]           = "fmax";
    ia64_opcode_stem_names[FMERGE]         = "fmerge";
    ia64_opcode_stem_names[FMIN]           = "fmin";
    ia64_opcode_stem_names[FMIX]           = "fmix";
    ia64_opcode_stem_names[FMPY]           = "fmpy";
    ia64_opcode_stem_names[FMS]            = "fms";
    ia64_opcode_stem_names[FNEG]           = "fneg";
    ia64_opcode_stem_names[FNEGABS]        = "fnegabs";
    ia64_opcode_stem_names[FNMA]           = "fnma";
    ia64_opcode_stem_names[FNMPY]          = "fnmpy";
    ia64_opcode_stem_names[FNORM]          = "fnorm";
    ia64_opcode_stem_names[FOR]            = "for";
    ia64_opcode_stem_names[FPABS]          = "fpabs";
    ia64_opcode_stem_names[FPACK]          = "fpack";
    ia64_opcode_stem_names[FPAMAX]         = "fpamax";
    ia64_opcode_stem_names[FPAMIN]         = "fpamin";
    ia64_opcode_stem_names[FPCMP]          = "fpcmp";
    ia64_opcode_stem_names[FPMA]           = "fpma";
    ia64_opcode_stem_names[FPMAX]          = "fpmax";
    ia64_opcode_stem_names[FPMERGE]        = "fpmerge";
    ia64_opcode_stem_names[FPMIN]          = "fpmin";
    ia64_opcode_stem_names[FPMPY]          = "fpmpy";
    ia64_opcode_stem_names[FPMS]           = "fpms";
    ia64_opcode_stem_names[FPNEG]          = "fpneg";
    ia64_opcode_stem_names[FPNEGABS]       = "fpnegabs";
    ia64_opcode_stem_names[FPNMA]          = "fpnma";
    ia64_opcode_stem_names[FPNMPY]         = "fpnmpy";
    ia64_opcode_stem_names[FPRCPA]         = "fprcpa"; 
    ia64_opcode_stem_names[FPRSQRTA]       = "fprsqrta";
    ia64_opcode_stem_names[FRCPA]          = "frcpa"; 
    ia64_opcode_stem_names[FRSQRTA]        = "frsqrta";
    ia64_opcode_stem_names[FSELECT]        = "fselect";
    ia64_opcode_stem_names[FSUB]           = "fsub";
    ia64_opcode_stem_names[FSWAP]          = "fswap";
    ia64_opcode_stem_names[FSXT]           = "fsxt";
    ia64_opcode_stem_names[FXOR]           = "fxor";
    ia64_opcode_stem_names[GETF]           = "getf"; 
    ia64_opcode_stem_names[SETF]           = "setf";

    // Floating-point special (ALM) instructions
    ia64_opcode_stem_names[FCHKF]          = "fchkf";
    ia64_opcode_stem_names[FCLASS]         = "fclass";
    ia64_opcode_stem_names[FCLRF]          = "fclrf";
    ia64_opcode_stem_names[FCVT_FX]        = "fcvt.fx";
    ia64_opcode_stem_names[FCVT_FXU]       = "fcvt.fxu";
    ia64_opcode_stem_names[FCVT_XF]        = "fcvt.xf";
    ia64_opcode_stem_names[FCVT_XUF]       = "fcvt.xuf";
    ia64_opcode_stem_names[FPCVT_FX]       = "fpcvt.fx";
    ia64_opcode_stem_names[FSETC]          = "fsetc";
}

char *opcode_stem_names_ia64(int stem)
{
    claim((stem <= LAST_IA64_OPCODE_STEM), "Invalid stem code!");
    return ia64_opcode_stem_names[stem];
}


// -------- Opcode extensions --------

Vector<char*> ia64_opcode_ext_names(LAST_IA64_OPCODE_EXT + 1);

void
init_ia64_opcode_ext_names()
{
    // branch type (btype) completers
    ia64_opcode_ext_names[BTYPE_NONE]  	= "";
    ia64_opcode_ext_names[BTYPE_COND] 	= ".cond";
    ia64_opcode_ext_names[BTYPE_CALL] 	= ".call";
    ia64_opcode_ext_names[BTYPE_RET] 	= ".ret";
    ia64_opcode_ext_names[BTYPE_IA] 	= ".ia";
    ia64_opcode_ext_names[BTYPE_CLOOP] 	= ".cloop";
    ia64_opcode_ext_names[BTYPE_CTOP] 	= ".ctop";
    ia64_opcode_ext_names[BTYPE_CEXIT] 	= ".cexit";
    ia64_opcode_ext_names[BTYPE_WTOP] 	= ".wtop";
    ia64_opcode_ext_names[BTYPE_WEXIT] 	= ".wexit";

    // branch whether hint (bwh) completers
    ia64_opcode_ext_names[BWH_SPNT] 	= ".spnt";
    ia64_opcode_ext_names[BWH_SPTK] 	= ".sptk";
    ia64_opcode_ext_names[BWH_DPNT] 	= ".dpnt";
    ia64_opcode_ext_names[BWH_DPTK] 	= ".dptk";

    // sequential prefetch hint (ph) completers
    ia64_opcode_ext_names[PH_NONE]  	= "";
    ia64_opcode_ext_names[PH_FEW] 	= ".few";
    ia64_opcode_ext_names[PH_MANY] 	= ".many";

    // branch cache deallocation hint (dh) completers
    ia64_opcode_ext_names[DH_NONE]  	= "";
    ia64_opcode_ext_names[DH_CLR] 	= ".clr";

    // format mnemonics
    ia64_opcode_ext_names[_FMT_NONE]  	= "";
    ia64_opcode_ext_names[_FMT_I]  	= ".i";
    ia64_opcode_ext_names[_FMT_B]  	= ".b";
    ia64_opcode_ext_names[_FMT_M]  	= ".m";
    ia64_opcode_ext_names[_FMT_F]  	= ".f";
    ia64_opcode_ext_names[_FMT_X]  	= ".x";

    // branch return mnemonics
    ia64_opcode_ext_names[_RET_NONE]  	= "";
    ia64_opcode_ext_names[_RET_RET]  	= ".ret";

    // ip-relative branch predict whether hint (ipwh) completers
    ia64_opcode_ext_names[IPWH_SPTK]	= ".sptk";
    ia64_opcode_ext_names[IPWH_LOOP]	= ".loop";
    ia64_opcode_ext_names[IPWH_EXIT]	= ".exit";
    ia64_opcode_ext_names[IPWH_DPTK]	= ".dptk";

    // indirect branch predict whether hint (indwh) completers
    ia64_opcode_ext_names[INDWH_SPTK]	= ".sptk";
    ia64_opcode_ext_names[INDWH_DPTK]	= ".dptk";

    // importance hint (dh) completers
    ia64_opcode_ext_names[IH_NONE]	= "";
    ia64_opcode_ext_names[IH_IMP]	= ".imp";

    // zero/one form indicators
    ia64_opcode_ext_names[_BANK_ZERO]	= ".0";
    ia64_opcode_ext_names[_BANK_ONE]	= ".1";

    // control/data form indicators
    ia64_opcode_ext_names[_SA_FORM_S]= ".s";
    ia64_opcode_ext_names[_SA_FORM_A]= ".a";

    // i-unit/m-unit form indicators
    ia64_opcode_ext_names[_IM_FORM_NONE]= "";
    ia64_opcode_ext_names[_IM_FORM_I]= ".i";
    ia64_opcode_ext_names[_IM_FORM_M]= ".m";

    // ALAT clear (aclr) completers
    ia64_opcode_ext_names[ACLR_CLR]	= ".clr";
    ia64_opcode_ext_names[ACLR_NC]	= ".nc";

    // all/pred form indicator
    ia64_opcode_ext_names[_CLR_ALL]	= "";
    ia64_opcode_ext_names[_CLR_PR]	= ".pr";

    // comparison type (ctype) completers
    ia64_opcode_ext_names[CTYPE_NONE]	= "";
    ia64_opcode_ext_names[CTYPE_UNC]	= ".unc";
    ia64_opcode_ext_names[CTYPE_OR]	= ".or";
    ia64_opcode_ext_names[CTYPE_AND]	= ".and";
    ia64_opcode_ext_names[CTYPE_ORANDCM]= ".or.andcm";
    ia64_opcode_ext_names[CTYPE_ORCM]	= ".orcm";
    ia64_opcode_ext_names[CTYPE_ANDCM]	= ".andcm";
    ia64_opcode_ext_names[CTYPE_ANDORCM]= ".and.orcm";

    // Comparison relations (crel) completers
    ia64_opcode_ext_names[CREL_EQ]	= ".eq";
    ia64_opcode_ext_names[CREL_NEQ]	= ".ne";
    ia64_opcode_ext_names[CREL_LT]	= ".lt";
    ia64_opcode_ext_names[CREL_LE]	= ".le";
    ia64_opcode_ext_names[CREL_GT]	= ".gt";
    ia64_opcode_ext_names[CREL_GE]	= ".ge";
    ia64_opcode_ext_names[CREL_LTU]	= ".ltu";
    ia64_opcode_ext_names[CREL_LEU]	= ".leu";
    ia64_opcode_ext_names[CREL_GTU]	= ".gtu";
    ia64_opcode_ext_names[CREL_GEU]	= ".geu";

    // size (sz) completers
    ia64_opcode_ext_names[SZ_1]		= "1";
    ia64_opcode_ext_names[SZ_2]		= "2";
    ia64_opcode_ext_names[SZ_4]		= "4";
    ia64_opcode_ext_names[SZ_8]		= "8";

    // compare and exchange semaphore types (sem) completers
    ia64_opcode_ext_names[SEM_ACQ]	= ".acq";
    ia64_opcode_ext_names[SEM_REL]	= ".rel";

    // left/right byte form
    ia64_opcode_ext_names[_LR_FORM_L]	= ".l";
    ia64_opcode_ext_names[_LR_FORM_R]	= ".r";

    // merge/zero form
    ia64_opcode_ext_names[_MZ_FORM_NONE]= "";
    ia64_opcode_ext_names[_MZ_FORM_Z]	= ".z";

    // signed/unsigned form
    ia64_opcode_ext_names[_SU_FORM_NONE]= "";
    ia64_opcode_ext_names[_SU_FORM_U]	= ".u";

    // trunc form
    ia64_opcode_ext_names[_ROUND_NONE]= "";
    ia64_opcode_ext_names[_ROUND_TRUNC]	= ".trunc";

    // specified pc mnemonic values (pc) completers
    ia64_opcode_ext_names[PC_NONE]	= "";
    ia64_opcode_ext_names[PC_S]		= ".s";
    ia64_opcode_ext_names[PC_D]		= ".d";

    // sf mnemonic values (sf) completers
    ia64_opcode_ext_names[SF_NONE]	= "";
    ia64_opcode_ext_names[SF_S0]	= ".s0";
    ia64_opcode_ext_names[SF_S1]	= ".s1";
    ia64_opcode_ext_names[SF_S2]	= ".s2";
    ia64_opcode_ext_names[SF_S3]	= ".s3";

    // floating point class relations (fcrel) completers
    ia64_opcode_ext_names[FCREL_M]	= ".m";
    ia64_opcode_ext_names[FCREL_NM]	= ".nm";

    // floating point comparison types (fctype) completers
    ia64_opcode_ext_names[FCTYPE_NONE]	= "";
    ia64_opcode_ext_names[FCTYPE_UNC]	= ".unc";

    // floating point classes (fclass9) completers
    ia64_opcode_ext_names[FCLASS_NAT]	= "@nat";
    ia64_opcode_ext_names[FCLASS_QNAN]	= "@qnan";
    ia64_opcode_ext_names[FCLASS_SNAN]	= "@snan";
    ia64_opcode_ext_names[FCLASS_POS]	= "@pos";
    ia64_opcode_ext_names[FCLASS_NEG]	= "@neg";
    ia64_opcode_ext_names[FCLASS_ZERO]	= "@zero";
    ia64_opcode_ext_names[FCLASS_UNORM]	= "@unorm";
    ia64_opcode_ext_names[FCLASS_NORM]	= "@norm";
    ia64_opcode_ext_names[FCLASS_INF]	= "@inf";

    // floating point comparison relations (frel) completers
    ia64_opcode_ext_names[FREL_EQ]	= ".eq";
    ia64_opcode_ext_names[FREL_LT]	= ".lt";
    ia64_opcode_ext_names[FREL_LE]	= ".le";
    ia64_opcode_ext_names[FREL_GT]	= ".gt";
    ia64_opcode_ext_names[FREL_GE]	= ".ge";
    ia64_opcode_ext_names[FREL_UNORD]	= ".unord";
    ia64_opcode_ext_names[FREL_NEQ]	= ".neq";
    ia64_opcode_ext_names[FREL_NLT]	= ".nlt";
    ia64_opcode_ext_names[FREL_NLE]	= ".nle";
    ia64_opcode_ext_names[FREL_NGT]	= ".ngt";
    ia64_opcode_ext_names[FREL_NGE]	= ".nge";
    ia64_opcode_ext_names[FREL_ORD]	= ".ord";

    // neg/sign/signexp form
    ia64_opcode_ext_names[_SIGN_NS]	= ".ns";
    ia64_opcode_ext_names[_SIGN_S]	= ".s";
    ia64_opcode_ext_names[_SIGN_SE]	= ".se";

    // mix l/r/lr form
    ia64_opcode_ext_names[_MIX_L]	= ".l";
    ia64_opcode_ext_names[_MIX_R]	= ".r";
    ia64_opcode_ext_names[_MIX_LR]	= ".lr";

    // swap form
    ia64_opcode_ext_names[_SWAP_NONE]	= "";
    ia64_opcode_ext_names[_SWAP_NL]	= ".nl";
    ia64_opcode_ext_names[_SWAP_NR]	= ".nr";

    // single/double/exponent/significand form
    ia64_opcode_ext_names[_SD_FORM_S]	= ".s";
    ia64_opcode_ext_names[_SD_FORM_D]	= ".d";
    ia64_opcode_ext_names[_SD_FORM_EXP]	= ".exp";
    ia64_opcode_ext_names[_SD_FORM_SIG]	= ".sig";

    // entry form
    ia64_opcode_ext_names[_E_FORM_NONE]	= "";
    ia64_opcode_ext_names[_E_FORM_E]	= ".e";

    // instruction/data form
    ia64_opcode_ext_names[_ID_FORM_I]	= ".i";
    ia64_opcode_ext_names[_ID_FORM_D]	= ".d";

    // Load types (ldtype) completers
    ia64_opcode_ext_names[LDTYPE_NONE]	= "";
    ia64_opcode_ext_names[LDTYPE_S]	= ".s";
    ia64_opcode_ext_names[LDTYPE_A]	= ".a";
    ia64_opcode_ext_names[LDTYPE_SA]	= ".sa";
    ia64_opcode_ext_names[LDTYPE_C_NC]	= ".c.nc";
    ia64_opcode_ext_names[LDTYPE_C_CLR]	= ".c.clr";
    ia64_opcode_ext_names[LDTYPE_C_CLR_ACQ]= ".c.clr.acq";
    ia64_opcode_ext_names[LDTYPE_ACQ]	= ".acq";
    ia64_opcode_ext_names[LDTYPE_BIAS]	= ".bias";
    ia64_opcode_ext_names[_LDTYPE_FILL]	= ".fill";

    // Load hints (ldhint) completers
    ia64_opcode_ext_names[LDHINT_NONE]	= "";
    ia64_opcode_ext_names[LDHINT_NT1]	= ".nt1";
    ia64_opcode_ext_names[LDHINT_NTA]	= ".nta";

    // Floating point size (fsz) completers
    ia64_opcode_ext_names[_FSZ_NONE]	= "";
    ia64_opcode_ext_names[FSZ_S]	= "s";
    ia64_opcode_ext_names[FSZ_D]	= "d";
    ia64_opcode_ext_names[FSZ_E]	= "e";
    ia64_opcode_ext_names[_FSZ_8]	= "8";

    // Exclusive form
    ia64_opcode_ext_names[_CACHE_NONE]	= "";
    ia64_opcode_ext_names[_CACHE_EXCL]	= ".excl";

    // Floating point load types (fldtype) completers
    ia64_opcode_ext_names[FLDTYPE_NONE]	= "";
    ia64_opcode_ext_names[FLDTYPE_S]	= ".s";
    ia64_opcode_ext_names[FLDTYPE_A]	= ".a";
    ia64_opcode_ext_names[FLDTYPE_SA]	= ".sa";
    ia64_opcode_ext_names[FLDTYPE_C_NC]	= ".c.nc";
    ia64_opcode_ext_names[FLDTYPE_C_CLR]= ".c.clr";
    ia64_opcode_ext_names[_FLDTYPE_FILL]= ".fill";

    // line fault type (lftype) completers
    ia64_opcode_ext_names[LFTYPE_NONE]	= "";
    ia64_opcode_ext_names[LFTYPE_FAULT]	= ".fault";

    // line prefetch hint (lfhint) completers
    ia64_opcode_ext_names[LFHINT_NONE]	= "";
    ia64_opcode_ext_names[LFHINT_NT1]	= ".nt1";
    ia64_opcode_ext_names[LFHINT_NT2]	= ".nt2";
    ia64_opcode_ext_names[LFHINT_NTA]	= ".nta";

    // ordering/acceptance form
    ia64_opcode_ext_names[_ORD_NONE]	= "";
    ia64_opcode_ext_names[_ORD_A]	= ".a";

    // move to branch whether hints (mwh) completers
    ia64_opcode_ext_names[MWH_NONE]	= "";
    ia64_opcode_ext_names[MWH_SPTK]	= ".sptk";
    ia64_opcode_ext_names[MWH_DPTK]	= ".dptk";

    // indirect register file (ireg) mnemonics
    ia64_opcode_ext_names[IREG_CPUID]	= "cpuid";
    ia64_opcode_ext_names[IREG_DBR]	= "dbr";
    ia64_opcode_ext_names[IREG_IBR]	= "ibr";
    ia64_opcode_ext_names[IREG_PKR]	= "pkr";
    ia64_opcode_ext_names[IREG_PMC]	= "pmc";
    ia64_opcode_ext_names[IREG_PMD]	= "pmd";
    ia64_opcode_ext_names[IREG_RR]	= "rr";

    // mux permutations for 8-bit elements (mbtype) completers
    ia64_opcode_ext_names[MBTYPE_REV]	= "@rev";
    ia64_opcode_ext_names[MBTYPE_MIX]	= "@mix";
    ia64_opcode_ext_names[MBTYPE_SHUF]	= "@shuf";
    ia64_opcode_ext_names[MBTYPE_ALT]	= "@alt";
    ia64_opcode_ext_names[MBTYPE_BRCST]	= "@brcst";

    // saturation form
    ia64_opcode_ext_names[_SAT_NONE]	= "";
    ia64_opcode_ext_names[_SAT_SSS]	= ".sss";
    ia64_opcode_ext_names[_SAT_USS]	= ".uss";
    ia64_opcode_ext_names[_SAT_UUS]	= ".uus";
    ia64_opcode_ext_names[_SAT_UUU]	= ".uuu";

    // normal/raz form
    ia64_opcode_ext_names[_RAZ_NONE]	= "";
    ia64_opcode_ext_names[_RAZ_RAZ]	= ".raz";

    // Pcmp relations (prel) completers
    ia64_opcode_ext_names[PREL_EQ]	= ".eq";
    ia64_opcode_ext_names[PREL_GT]	= ".gt";

    // read/write form
    ia64_opcode_ext_names[_RW_FORM_R]	= ".r";
    ia64_opcode_ext_names[_RW_FORM_W]	= ".w";
    ia64_opcode_ext_names[_RW_FORM_RW]	= ".rw";

    // store types (sttype) completers
    ia64_opcode_ext_names[STTYPE_NONE]	= "";
    ia64_opcode_ext_names[STTYPE_REL]	= ".rel";
    ia64_opcode_ext_names[_STTYPE_SPILL]= ".spill";

    // store hints (sthint) completers
    ia64_opcode_ext_names[STHINT_NONE]	= "";
    ia64_opcode_ext_names[STHINT_NTA]	= ".nta";

    // extend size (xsz) completers
    ia64_opcode_ext_names[XSZ_1]	= "1";
    ia64_opcode_ext_names[XSZ_2]	= "2";
    ia64_opcode_ext_names[XSZ_4]	= "4";

    // test bit relations (trel) completers
    ia64_opcode_ext_names[TREL_NZ]	= ".nz";
    ia64_opcode_ext_names[TREL_Z]	= ".z";

    // low/high/unsigned form
    ia64_opcode_ext_names[_LHU_FORM_L]	= ".l";
    ia64_opcode_ext_names[_LHU_FORM_LU]	= ".lu";
    ia64_opcode_ext_names[_LHU_FORM_H]	= ".h";
    ia64_opcode_ext_names[_LHU_FORM_HU]	= ".hu";

}

char *opcode_ext_names_ia64(int ext)
{ 
    claim((ext <= LAST_IA64_OPCODE_EXT), "Invalid extension code!");
    return ia64_opcode_ext_names[ext];
}

char *opcode_name_ia64(int opcode) {

    /* FIXME - Essentially, this will never be called! */
    /*   but when it is, we shouldn't use malloc - bad idea */

    char *thisName = (char *)malloc(sizeof(char)*50);
    int stem = get_stem(opcode);
    strcpy(thisName, opcode_stem_names_ia64(stem));
    if ((stem==ia64::BR)||(stem==ia64::BRL)||(stem==ia64::BRP)) {
        int i; int ext;
        for(i=0; i<4; i++) {
            ext = get_br_ext(opcode, (i+1));
            strcat(thisName, opcode_ext_names_ia64(ext));
        }
    }
    else {
        int i; int ext;
        for(i=0; i<3; i++) {
            ext = get_ext(opcode, (i+1));
            strcat(thisName, opcode_ext_names_ia64(ext));
        }
    }
    return thisName;
}


// -------- Other OPI functions --------

bool
target_implements_ia64(int opcode)
{
    return true;
}


// -------- Code generation helper routines --------

int opcode_cbr_inverse_ia64(int opcode)
{
    /* FIXME: We don't invert opcodes, invert the predicates! */
    claim(false, "opcodes.cpp -- can't invert branches without predicates");
    return opcode_null;
}


int
opcode_line_ia64()
{
    return make_opcode(LN, EMPTY,EMPTY,EMPTY);
}

int
opcode_ubr_ia64()
{
    return make_br_opcode(BR, BTYPE_NONE,BWH_SPTK,PH_NONE,DH_NONE);
}

int
opcode_load_ia64(TypeId t)
{
    /* FIXME: IA-64 lets you special-case spills and fills,
	but I haven't been able to determine them here */

    switch (get_bit_size(t)) {
      case 8: // 1-byte 
	claim(!is_floating_point(t));
	return (make_opcode(LD, SZ_1, LDTYPE_NONE, LDHINT_NONE));

      case 16: // 2-bytes
	claim(!is_floating_point(t));
	return (make_opcode(LD, SZ_2, LDTYPE_NONE, LDHINT_NONE));

      case 32: // 4-bytes
	return (is_floating_point(t)) ? 
	  (make_opcode(LDF, FSZ_S, FLDTYPE_NONE, LDHINT_NONE)) : 
	  (make_opcode(LD, SZ_4, LDTYPE_NONE, LDHINT_NONE));

      case 64: // 8-bytes
	return (is_floating_point(t)) ? 
	  (make_opcode(LDF, FSZ_D, FLDTYPE_NONE, LDHINT_NONE)) : 
	  (make_opcode(LD, SZ_8, LDTYPE_NONE, LDHINT_NONE)) ; 

      case 80: // 10-bytes
	claim(is_floating_point(t));
	return make_opcode(LDF, FSZ_E, FLDTYPE_NONE, LDHINT_NONE);

      case 128: // 16-bytes
	return (is_floating_point(t)) ? 
	  (make_opcode(LDFP, FSZ_D, FLDTYPE_NONE, LDHINT_NONE)) :
	  (make_opcode(LDFP, _FSZ_8, FLDTYPE_NONE, LDHINT_NONE));

      default:
	fprint(stderr, t);
	claim(false, "opcode_load_ia64() -- unexpected load size");
    }
    return opcode_null;
}

int
opcode_store_ia64(TypeId t)
{
    /* FIXME: IA-64 lets you special-case spills and fills,
	but I haven't been able to determine them here */

    switch (get_bit_size(t)) {
      case 8:
	claim(!is_floating_point(t));
	return make_opcode(ST, SZ_1, STTYPE_NONE, STHINT_NONE);

      case 16:
	claim(!is_floating_point(t));
	return make_opcode(ST, SZ_2, STTYPE_NONE, STHINT_NONE);

      case 32:
	return (is_floating_point(t)) ? 
	  (make_opcode(STF, FSZ_S, STTYPE_NONE, STHINT_NONE)) :
	  (make_opcode(ST, SZ_4, STTYPE_NONE, STHINT_NONE));

      case 64:
	return (is_floating_point(t)) ?
	  (make_opcode(STF, FSZ_D, STTYPE_NONE, STHINT_NONE)) :
	  (make_opcode(ST, SZ_8, STTYPE_NONE, STHINT_NONE));

      case 80:
	claim(is_floating_point(t));
	return make_opcode(STF, FSZ_E, STTYPE_NONE, STHINT_NONE);

      case 128:
	claim(is_floating_point(t));
	return make_opcode(STF, FSZ_E, STTYPE_NONE, STHINT_NONE);

      default:
	fprint(stderr, t);
	claim(false, "opcode_store_ia64() -- unexpected store size");
    }
    return opcode_null;
}

int
opcode_move_ia64(TypeId t)
{
    return (is_floating_point(t) ?
	(make_opcode(MOV_FR, EMPTY,EMPTY,EMPTY)) :
	(make_opcode(MOV_GR, EMPTY,EMPTY,EMPTY)));
}

bool
is_breg(Opnd opnd)
{
  return is_type_br(get_type(opnd));
}

bool
is_preg(Opnd opnd)
{
  return is_type_pr(get_type(opnd));
}
