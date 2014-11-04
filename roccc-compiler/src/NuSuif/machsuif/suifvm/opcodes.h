/* file "suifvm/opcodes.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef SUIFVM_OPCODES_H
#define SUIFVM_OPCODES_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "suifvm/opcodes.h"
#endif

#include <machine/machine.h>

namespace suifvm {
    enum {			// SUIFvm opcodes
        NOP = 2,
        CVT, LDA, LDC,
        ADD, SUB, NEG,
        MUL, DIV, REM, MOD,
        ABS, 
        MIN, MAX,
        NOT, 
        ASR, LSL, LSR, ROT,
        MOV, LOD, STR, MEMCPY,
        SEQ, SNE, 
        SL, SLE,
        BTRUE, BFALSE,
        BEQ, BNE, BGE, BGT, BLE, BLT, 
        JMP, JMPI, MBR, CAL,RET,
        ANY, MRK, 
        BEX, BNS, MUX, FOI, PST, PFW, WCF, 
        BCMB, BLUT, BSEL, AZR,
        LPR, SNX, RLD, RST, SMB1, SMB2, SFF1, SFF2, LUT, 

        // John's custom instructions
        CUSTOM_START,
        SATADD, 
        SATCLAMP, 

        SG, SGE,          // were missing, (sg = set if greater than, sge =  set if greater than or equal)

        // ##############################
        // NEW UPDATE  
        // ##############################
        RB_SMB1D, 
        RB_SMB2D, 

        WB_SFF1D, 
        WB_SFF2D,

        // MIN and MAX will be deprecated to MIN2 and MAX2 in preprocess
        MIN2, MIN3, 
        MAX2, MAX3, 

        //AND, IOR, XOR, //have been replaced with the following:
        LAND, // Logical AND
        BAND, // Bitwise AND
        LOR,  // Logical OR
        BIOR, // Bitwise IOR
        BXOR, // Bitwise XOR
        // ##############################
        
        // custom cam,mux
        CUST_CAM, 
        CUST_MUX,

        // ##############################
        
        INSCALAR,          // input scalar 
        OUTSCALAR,         // output scalar
        STATESCALAR,       // state scalar added for ROCCC2.0

        GENBLUT,           // create a byte lut
        LKUPBLUT,          // access to a lut (look up byte lut)

        GIPCORE,            // generic ipcore
        GIPCORE_PP,         // generic ipcore pipeline propagate instruction

        FP_ACCUM,           // floating point accumulator
        FP_NEG,             // floating point negation

        CUSTOM_END 
    };
} // namespace suifvm

#define LAST_SUIFVM_OPCODE suifvm::CUSTOM_END

extern Vector<char*> suifvm_opcode_names;
extern Vector<char*> suifvm_cnames;
void init_suifvm_opcode_names();
void init_suifvm_cnames();

bool target_implements_suifvm(int opc);
char *opcode_name_suifvm(int opc);

int opcode_line_suifvm();
int opcode_ubr_suifvm();
int opcode_move_suifvm(TypeId);
int opcode_load_suifvm(TypeId);
int opcode_store_suifvm(TypeId);

int opcode_cbr_inverse_suifvm(int opc);

#endif /* SUIFVM_OPCODES_H */
