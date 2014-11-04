/* file "suifvm/opcodes.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "suifvm/opcodes.h"
#endif

#include <machine/machine.h>
#include <suifvm/opcodes.h>


using namespace suifvm;

Vector<char*> suifvm_opcode_names(LAST_SUIFVM_OPCODE + 1);
Vector<char*> suifvm_cnames(LAST_SUIFVM_OPCODE + 1);

void
init_suifvm_opcode_names()
{
    suifvm_opcode_names[opcode_null] = "";
    suifvm_opcode_names[opcode_label] = "";

    suifvm_opcode_names[NOP]	= "nop";
    suifvm_opcode_names[CVT]	= "cvt";
    suifvm_opcode_names[LDA]	= "lda";
    suifvm_opcode_names[LDC]	= "ldc";
    suifvm_opcode_names[ADD]	= "add";
    suifvm_opcode_names[SUB]	= "sub";
    suifvm_opcode_names[NEG]	= "neg";
    suifvm_opcode_names[MUL]	= "mul";
    suifvm_opcode_names[DIV]	= "div";
    suifvm_opcode_names[REM]	= "rem";
    suifvm_opcode_names[MOD]	= "mod";
    suifvm_opcode_names[ABS]	= "abs";
    suifvm_opcode_names[MIN]	= "min";
    suifvm_opcode_names[MAX]	= "max";
    suifvm_opcode_names[NOT]	= "not";
    // Will be removed in next iteration
    //suifvm_opcode_names[AND]	= "and";
    //suifvm_opcode_names[IOR]	= "ior";
    //suifvm_opcode_names[XOR]	= "xor";
    // =============
    suifvm_opcode_names[ASR]	= "asr";
    suifvm_opcode_names[LSL]	= "lsl";
    suifvm_opcode_names[LSR]	= "lsr";
    suifvm_opcode_names[ROT]	= "rot";
    suifvm_opcode_names[MOV]	= "mov";
    suifvm_opcode_names[LOD]	= "lod";
    suifvm_opcode_names[STR]	= "str";
    suifvm_opcode_names[MEMCPY] = "memcpy";
    suifvm_opcode_names[SEQ]	= "seq";
    suifvm_opcode_names[SNE]	= "sne";
    suifvm_opcode_names[SL]	    = "sl";
    suifvm_opcode_names[SLE]	= "sle";
    
   

    suifvm_opcode_names[BTRUE]	= "btrue";
    suifvm_opcode_names[BFALSE] = "bfalse";
    suifvm_opcode_names[BEQ]	= "beq";
    suifvm_opcode_names[BNE]	= "bne";
    suifvm_opcode_names[BGE]	= "bge";
    suifvm_opcode_names[BGT]	= "bgt";
    suifvm_opcode_names[BLE]	= "ble";
    suifvm_opcode_names[BLT]	= "blt";
    suifvm_opcode_names[JMP]	= "jmp";
    suifvm_opcode_names[JMPI]	= "jmpi";
    suifvm_opcode_names[MBR]	= "mbr";
    suifvm_opcode_names[CAL]	= "cal";
    suifvm_opcode_names[RET]	= "ret";
    suifvm_opcode_names[ANY]	= "any";
    suifvm_opcode_names[MRK]	= "mrk";
    suifvm_opcode_names[BEX]	= "bex";
    suifvm_opcode_names[BNS]	= "bns";
    suifvm_opcode_names[MUX]	= "mux";
    suifvm_opcode_names[FOI]	= "foi";
    suifvm_opcode_names[PST]	= "pst";
    suifvm_opcode_names[PFW]	= "pfw";
    suifvm_opcode_names[WCF]	= "wcf";
    suifvm_opcode_names[BCMB]	= "bcmb";
    suifvm_opcode_names[BLUT]	= "blut";
    suifvm_opcode_names[BSEL]	= "bsel";
    suifvm_opcode_names[AZR]	= "azr";
    suifvm_opcode_names[LPR]	= "lpr";
    suifvm_opcode_names[SNX]	= "snx";      
    suifvm_opcode_names[LUT]	= "lut";
    suifvm_opcode_names[RLD]	= "rld";
    suifvm_opcode_names[RST]	= "rst";        

    suifvm_opcode_names[SMB1]	= "smb1";        
    suifvm_opcode_names[SMB2]	= "smb2";        
    suifvm_opcode_names[SFF1]	= "sff1";
    suifvm_opcode_names[SFF2]	= "sff2";

    suifvm_opcode_names[SATADD] = "sat_add";
    suifvm_opcode_names[SATCLAMP] = "sat_clamp";

  
    // ##############################
    suifvm_opcode_names[RB_SMB1D] = "rb_smb1d";
    suifvm_opcode_names[RB_SMB2D] = "rb_smb2d";

    suifvm_opcode_names[WB_SFF1D] = "wb_sff1d";
    suifvm_opcode_names[WB_SFF2D] = "wb_sff2d";
    // ##############################

    suifvm_opcode_names[MAX2]	= "max2";
    suifvm_opcode_names[MAX3]	= "max3";
    suifvm_opcode_names[MIN2]	= "min2";
    suifvm_opcode_names[MIN3]	= "min3";


    suifvm_opcode_names[LAND]	= "Land"; // Logical AND  (if both operators are true types, input and output types are of "u1")
    suifvm_opcode_names[BAND]	= "Band"; // Bitwise AND  (operators are same)

    suifvm_opcode_names[LOR] = "Lor";   // logical or
    suifvm_opcode_names[BIOR] = "Bior"; // bitwise incluseive OR
    suifvm_opcode_names[BXOR] = "Bxor"; // bitwise excluseive OR

    // ##############################
    suifvm_opcode_names[CUST_CAM] = "cust_cam";
    suifvm_opcode_names[CUST_MUX] = "cust_mux";
    // ##############################

    suifvm_opcode_names[INSCALAR] = "inscalar";
    suifvm_opcode_names[OUTSCALAR] = "outscalar";
    suifvm_opcode_names[STATESCALAR] = "statescalar";

    // bytelut look up
    suifvm_opcode_names[GENBLUT] = "gen_bytelut";
    suifvm_opcode_names[LKUPBLUT] = "bytelut_lkup";

    // generic ipcore
    suifvm_opcode_names[GIPCORE] = "gipcore";
    suifvm_opcode_names[GIPCORE_PP] = "gipcore_PP";

    suifvm_opcode_names[FP_ACCUM] = "fp_acc";
    suifvm_opcode_names[FP_NEG] = "fp_neg";

    // add missing stuff
    suifvm_opcode_names[SG]    = "sg";
    suifvm_opcode_names[SGE]    = "sge";
 

}


char *
opcode_name_suifvm(int opc)
{
    return suifvm_opcode_names[opc];
}


void
init_suifvm_cnames()
{
    suifvm_cnames[opcode_null] = "*not_used*";
    suifvm_cnames[opcode_label] = "*not_used*";

    suifvm_cnames[NOP]	  = "*not_used*";
    suifvm_cnames[CVT]	  = "*not_used*";
    suifvm_cnames[LDA]	  = "*not_used*";
    suifvm_cnames[LDC]	  = "*not_used*";
    suifvm_cnames[ADD]	  = " + ";
    suifvm_cnames[SUB]	  = " - ";
    suifvm_cnames[NEG]	  = "-";
    suifvm_cnames[MUL]	  = " * ";
    suifvm_cnames[DIV]	  = " / ";
    suifvm_cnames[REM]	  = " % ";
    suifvm_cnames[MOD]	  = "_MOD";
    suifvm_cnames[ABS]	  = "_ABS";
//    suifvm_cnames[MIN]	  = "_MIN";
//    suifvm_cnames[MAX]	  = "_MAX";
    suifvm_cnames[NOT]	  = "~";
//    suifvm_cnames[AND]	  = " & ";
//    suifvm_cnames[IOR]	  = " | ";
//    suifvm_cnames[XOR]	  = " ^ ";
    suifvm_cnames[ASR]	  = " >> ";
    suifvm_cnames[LSL]	  = " << ";
    suifvm_cnames[LSR]	  = "_LSR";
    suifvm_cnames[ROT]	  = "_ROT";
    suifvm_cnames[MOV]	  = "*not_used*";
    suifvm_cnames[LOD]	  = "*not_used*";
    suifvm_cnames[STR]	  = "*not_used*";
    suifvm_cnames[MEMCPY] = "*not_used*"; 
    suifvm_cnames[SEQ]	  = " == ";
    suifvm_cnames[SNE]	  = " != ";
    suifvm_cnames[SL]	  = " < ";
    suifvm_cnames[SLE]	  = " <= ";
    suifvm_cnames[BTRUE]  = "*not_used*";
    suifvm_cnames[BFALSE] = "*not_used*";
    suifvm_cnames[BEQ]	  = " == ";
    suifvm_cnames[BNE]	  = " != ";
    suifvm_cnames[BGE]	  = " >= ";
    suifvm_cnames[BGT]	  = " > ";
    suifvm_cnames[BLE]	  = " <= ";
    suifvm_cnames[BLT]	  = " < ";
    suifvm_cnames[JMP]	  = "*not_used*";
    suifvm_cnames[JMPI]	  = "*not_used*";
    suifvm_cnames[MBR]	  = "*not_used*";
    suifvm_cnames[CAL]	  = "*not_used*";
    suifvm_cnames[RET]	  = "*not_used*";
    suifvm_cnames[ANY]	  = "*not_used*";
    suifvm_cnames[MRK]	  = "*not_used*";
    suifvm_cnames[BEX]	  = "*not_used*";
    suifvm_cnames[BNS]	  = "*not_used*";
    suifvm_cnames[MUX]	  = "*not_used*";
    suifvm_cnames[FOI]	  = "*not_used*";
    suifvm_cnames[PST]	  = "*not_used*";
    suifvm_cnames[PFW]	  = "*not_used*";
    suifvm_cnames[WCF]	  = "*not_used*";
    suifvm_cnames[BCMB]	  = "*not_used*";
    suifvm_cnames[BLUT]	  = "*not_used*";
    suifvm_cnames[BSEL]	  = "*not_used*";    
    suifvm_cnames[AZR]	  = "*not_used*";
    suifvm_cnames[MIN3]	  = "*not_used*";
    suifvm_cnames[MAX3]	  = "*not_used*";
    suifvm_cnames[LPR]	  = "*not_used*";
    suifvm_cnames[SNX]	  = "*not_used*";           
    suifvm_cnames[LUT]	  = "*not_used*";
    suifvm_cnames[RLD]	  = "*not_used*";
    suifvm_cnames[RST]	  = "*not_used*";       
    suifvm_cnames[SMB1]	  = "*not_used*";       
    suifvm_cnames[SMB2]	  = "*not_used*";       
    suifvm_cnames[SFF1]	  = "*not_used*";       
    suifvm_cnames[SFF2]	  = "*not_used*";       

    for(int i=CUSTOM_START; i<= CUSTOM_END; i++)
        suifvm_cnames[i] = "*not_used*";
}

bool
target_implements_suifvm(int opc)
{
    return true;
}

int
opcode_line_suifvm()
{
    return MRK;
}

int
opcode_ubr_suifvm()
{
    return JMP;
}

int
opcode_move_suifvm(TypeId)
{
    // type doesn't matter for suifvm
    return MOV;
}

int
opcode_load_suifvm(TypeId)
{
    // type doesn't matter for suifvm
    return LOD;
}

int opcode_store_suifvm(TypeId)
{
    // type doesn't matter for suifvm
    return STR;
}

int opcode_cbr_inverse_suifvm(int opc)
{
    if (opc == BFALSE)
	return BTRUE;
    else if (opc == BTRUE)
	return BFALSE;
    else if (opc == BEQ)
	return BNE;
    else if (opc == BNE)
	return BEQ;
    else if (opc == BGE)
	return BLT;
    else if (opc == BGT)
	return BLE;
    else if (opc == BLE)
	return BGT;
    else if (opc == BLT)
	return BGE;

    claim(false, "opcode is not a cbr");
    return opcode_null;
}
