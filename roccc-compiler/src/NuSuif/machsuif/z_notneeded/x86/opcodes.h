/* file "x86/opcodes.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef X86_OPCODES_H
#define X86_OPCODES_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "x86/opcodes.h"
#endif

/* Define enumerations for the complete set of x86 assembly opcodes,
 * pseudo-ops, opcode extensions, and the code generator/scheduler
 * pseudo-ops.  
 *
 * To modify the contents of the opcode enumeration, you add new enum
 * constants to current end of the enum (specified by LAST_X86_OPCODE),
 * redefine LAST_X86_OPCODE, and you enroll new data into the opcode-indexed
 * data tables. */

namespace x86 {
enum {
    // opcode_null = 0,		defined in machine/opcodes.h
    // opcode_label,		defined in machine/opcodes.h

    // Pseudo-opcode (Dot) instructions
    ALIGN = 2,          // alignment location counter
    ASCII,              // assemble strings into successive locs
    BYTE,               // assemble expressions into succ. bytes
    COMM,               // global common symbol
    DATA,               // subsequent data to data section
    DOUBLE,             // initialize 64-bit floating-point numbers
    ENDR,               // end of repeat
    OPCODE_FILE,        // source file name
    FLOAT,              // initialize 32-bit floating-point numbers
    GLOBL,              // makes a name external
    LCOMM,              // makes name's data type bss
    LOC,                // specifies source and line number
    LONG,               // assemble expressions into succ. 32-bits
    REPEAT,             // begin repeat directive
    TEXT,               // subsequent code to text section
    WORD,               // assemble expressions into succ. 16-bits

    // Load, move, store, set and exchange (ALM) instructions
    LDS,                // load full pointer (DS)
    LEA,                // load effective address
    LES,                // load full pointer (ES)       
    LFS,                // load full pointer (FS)
    LGS,                // load full pointer (GS)
    LSS,                // load full pointer (SS)
    LODS,               // load string
    LODSB,              // load string (byte)
    LODSD,              // load string (doubleword)
    LODSW,              // load string (word)
    MOV,                // load/store operation
    MOVS,               // move data from string to string
    MOVSB,              // move data from string to string (byte)
    MOVSD,              // move data from string to string (doubleword)
    MOVSL,              // move data from string to string [synonym of movsd]
    MOVSW,              // move data from string to string (word)
    MOVSX,              // move with sign-extend (MOVSX in Intel syntax)
    MOVZX,              // move with zero-extend (MOVZX in Intel syntax)
    SETA,               // set if above
    SETAE,              // set if above or equal
    SETB,               // set if below
    SETBE,              // set if below or equal
    SETC,               // set if carry
    SETE,               // set if equal
    SETG,               // set if greater
    SETGE,              // set if greater or equal
    SETL,               // set if less
    SETLE,              // set if less or equal
    SETNA,              // set if not above
    SETNAE,             // set if not above or equal
    SETNB,              // set if not below
    SETNBE,             // set if not below or equal
    SETNC,              // set if not carry
    SETNE,              // set if not equal
    SETNG,              // set if not greater
    SETNGE,             // set if not greater or equal
    SETNL,              // set if not less
    SETNLE,             // set if not less or equal
    SETNO,              // set if not overflow
    SETNP,              // set if not parity
    SETNS,              // set if not sign
    SETNZ,              // set if not zero
    SETO,               // set if overflow
    SETP,               // set if parity
    SETPE,              // set if parity even
    SETPO,              // set if parity odd
    SETS,               // set if sign
    SETZ,               // set if zero
    STOS,               // store string data
    STOSB,              // store string data (byte)
    STOSD,              // store string data (doubleword)
    STOSW,              // store string data (word)
    XCHG,               // exchange
    XLAT,               // table look-up translation
    XLATB,              // table look-up translation (no operand)

    // General computational (ALM) instructions
    ADC,                // add with carry
    ADD,                // add
    AND,                // and
    BSF,                // bit scan forward
    BSR,                // bit scan reverse
    BSWAP,              // byte swap (for little/big endian conversion
    BT,                 // bit test
    BTC,                // bit test and complement
    BTR,                // bit test and reset
    BTS,                // bit test and set
    CBW,                // convert byte to word
    CDQ,                // convert doubleword to quadword
    CMP,                // compare
    CMPS,               // compare string operands
    CMPSB,              // compare string operands (byte)
    CMPSW,              // compare string operands (word)
    CMPSD,              // compare string operands (doubleword)
    CMPXCHG,            // compare and exchange
    CWD,                // convert word to doubleword
    CWDE,               // convert word to doubleword (alternate)
    DEC,                // decrement
    DIV,                // unsigned divide
    IDIV,               // signed divide
    IMUL,               // signed multiply
    INC,                // increment
    MUL,                // unsigned multiply
    NEG,                // negation
    NOT,                // one's complement negation
    OR,                 // or
    RCL,                // rotate left (with carry flag)
    RCR,                // rotate right (with carry flag)
    ROL,                // rotate left
    ROR,                // rotate right
    SAL,                // shift left
    SAR,                // shift right
    SBB,                // subtract with borrow
    SHL,                // shift left
    SHR,                // shift right
    SCAS,               // compare string data
    SCASB,              // compare string data (byte)
    SCASD,              // compare string data (doubleword)
    SCASW,              // compare string data (word)
    SHLD,               // double precision shift left
    SHRD,               // double precision shift right
    SUB,                // subtract
    TEST,               // bitwise compare
    XADD,               // exchange and add
    XOR,                // exclusive or

    // BCD math (ALM) instructions
    AAA,                // ASCII adjust after addition
    AAD,                // ASCII adjust before division
    AAM,                // ASCII adjust after multiplication
    AAS,                // ASCII adjust after subtraction
    DAA,                // decimal adjust after addition
    DAS,                // decimal adjust after subtraction

    // Branch (CTI) instructions
    JA,                 // jump if above
    JAE,                // jump if above or equal
    JB,                 // jump if below
    JBE,                // jump if below or equal
    JC,                 // jump if carry
    JE,                 // jump if equal
    JG,                 // jump if greater
    JGE,                // jump if greater or equal
    JL,                 // jump if less
    JLE,                // jump if less or equal
    JNA,                // jump if not above
    JNAE,               // jump if not above or equal
    JNB,                // jump if not below
    JNBE,               // jump if not below or equal
    JNC,                // jump if not carry
    JNE,                // jump if not equal
    JNG,                // jump if not greater
    JNGE,               // jump if not greater or equal
    JNL,                // jump if not less
    JNLE,               // jump if not less or equal
    JNO,                // jump if not overflow
    JNP,                // jump if not parity
    JNS,                // jump if not sign
    JNZ,                // jump if not zero
    JO,                 // jump if overflow
    JP,                 // jump if parity
    JPE,                // jump if parity even
    JPO,                // jump if parity odd
    JS,                 // jump if sign
    JZ,                 // jump if zero

    JCXZ,               // jump if CX = 0
    JECXZ,              // jump if ECX = 0
    LOOP,               // loop
    LOOPE,              // loop if equal
    LOOPNE,             // loop if not equal
    LOOPNZ,             // loop if not zero
    LOOPZ,              // loop if zero

    // Jump (CTI) instructions
    CALL,               // call procedure
    JMP,                // jump

    // Flag manipulation (ALM) instructions
    CLC,                // clear carry flag
    CLD,                // clear direction flag
    CLI,                // clear interrupt flag
    CLTS,               // clear task-switched flag
    CMC,                // complement carry flag
    POPF,               // pop flags register
    POPFD,              // pop flags register (32-bit)
    PUSHF,              // push flags register
    PUSHFD,             // push flags register (32-bit)
    LAHF,               // load flags into AH
    SAHF,               // store AH in flags
    STC,                // set carry flag
    STD,                // set direction flag
    STI,                // set interrupt flag

    // Memory management (ALM)
    ARPL,               // adjust RPL field of selector
    BOUND,              // check array index against bounds
    LAR,                // load selector access rights
    LGDT,               // load global descriptor table register
    LIDT,               // load interrupt descriptor table register
    LLDT,               // load local descriptor table register
    LSL,                // load segment limit
    SGDT,               // store global descriptor table register
    SIDT,               // store interrupt descriptor table register
    SLDT,               // store local descriptor table register
    VERR,               // verify segment for reading
    VERW,               // verify segment for writing

    // Stack (ALM)
    ENTER,              // make stack frame for procedure parameters
    LEAVE,              // leave procedure stack frame
    POP,                // pop from stack
    POPA,               // pop all registers from stack
    POPAD,              // pop all registers from stack (32-bit)
    PUSH,               // push onto stack
    PUSHA,              // push all registers onto stack
    PUSHAD,             // push all registers onto stack (32-bit)
    RET,                // return from procedure

    // Special (ALM)
    HLT,                // halt the processor
    IN,                 // input from port
    INS,                // input from port to string
    INSB,               // input from port to string (byte)
    INSD,               // input from port to string (doubleword)
    INSW,               // input from port to string (word)
    INT,                // interrupt
    INTO,               // interrupt 4 on overflow
    INVD,               // invalidate cache
    INVLPG,             // invalidate TLB entry
    IRET,               // return from interrupt
    IRETD,              // return from interrupt (32-bit)
    LMSW,               // load machine status word
    LTR,                // load task register
    NOP,                // no operation
    OUT,                // output to port
    OUTS,               // output string to port
    OUTSB,              // output string to port (byte)
    OUTSD,              // output string to port (doubleword)
    OUTSW,              // output string to port (word)
    SMSW,               // store machine status word
    STR,                // store task register
    WAIT,               // wait for pending numeric exceptions
    WBINVD,             // write-back and invalidate cache

    // Floating-point load immediate (ALM) instructions
    FLD1,               // load 1
    FLDL2T,             // load log2 of 10
    FLDL2E,             // load log2 of e
    FLDPI,              // load pi
    FLDLG2,             // load log 2
    FLDLN2,             // load ln 2
    FLDZ,               // load 0
    FXCH,               // exchange registers

    // Floating-point load/store (ALM) instructions
    FBLD,               // load BCD
    FBSTP,              // store binary coded decimal and pop
    FILD,               // load integer
    FIST,               // store integer
    FISTP,              // store integer and pop
    FLD,                // load real
    FST,                // store real
    FSTP,               // store real and pop

    // Floating-point computational (ALM) instructions
    F2XM1,              // compute 2^n - 1
    FABS,               // absolute value
    FADD,               // add
    FADDP,              // add and pop
    FCHS,               // change sign
    FCOM,               // compare
    FCOMP,              // compare and pop
    FCOMPP,             // compare and pop twice
    FCOS,               // cosine
    FDIV,               // divide
    FDIVP,              // divide and pop
    FDIVR,              // reverse divide
    FDIVRP,             // reverse divide and pop
    FIADD,              // add integer
    FICOM,              // compare integer
    FICOMP,             // compare integer and pop
    FIDIV,              // divide integer
    FIDIVR,             // reverse divide integer
    FISUB,              // subtract integer
    FISUBR,             // reverse subtract integer
    FMUL,               // multiply
    FMULP,              // multiply and pop
    FIMUL,              // multiply integer
    FPATAN,             // partial arctangent
    FPREM,              // partial remainder (backward compatibility)
    FPREM1,             // partial remainder (IEEE)
    FPTAN,              // partial tangent
    FRNDINT,            // round to integer
    FSCALE,             // scale (multiple/divide by powers of 2)
    FSIN,               // sine
    FSINCOS,            // sine and cosine
    FSQRT,              // square root
    FSUB,               // subtract
    FSUBP,              // subtract and pop
    FSUBR,              // reverse subtract
    FSUBRP,             // reverse subtract and pop
    FTST,               // compare with 0
    FUCOM,              // unordered compare real
    FUCOMP,             // unordered compare real and pop
    FUCOMPP,            // unordered compare real and pop twice
    FXTRACT,            // extract exponent and mantissa
    FYL2X,              // y * log2 x
    FYL2XP1,            // y * log2(x + 1)

    // Floating-point special (ALM) instructions
    FCLEX,              // clear exceptions
    FDECSTP,            // decrement stack-top pointer
    FFREE,              // free floating-point register
    FINCSTP,            // increment stack-top pointer
    FINIT,              // initialize FPU
    FLDCW,              // load control word
    FLDENV,             // load FPU environment
    FNCLEX,             // clear exceptions (w/o error checks)
    FNINIT,             // initialize FPU (w/o error checks)
    FNOP,               // no operation
    FNSAVE,             // store FPU state (w/o error checks)
    FNSTCW,             // store control word (w/o error checks)
    FNSTENV,            // store FPU environment (w/o error checks)
    FNSTSW,             // store status word (w/o error checks)
    FRSTOR,             // restore FPU state
    FSAVE,              // store FPU state
    FSTCW,              // store control word
    FSTENV,             // store FPU environment
    FSTSW,              // store status word
    FWAIT,              // wait
    FXAM                // examine object type
};
} // namespace x86

#define LAST_X86_OPCODE x86::FXAM

extern Vector<char*> x86_opcode_names;

// following used only in x86/init.cc
void init_x86_opcode_names();

char *opcode_name_x86(int opcode);


// Defined opcode extenders.

namespace x86 {
enum {
    LOCK,               // ensure exclusive access

    REP,                // repeat string operation
    REPE,               // ... if equal
    REPNE,              // ... if not equal
    REPNZ,              // ... if not zero
    REPZ,               // ... if zero
};
} // namespace x86

#define LAST_X86_OPCODE_EXT x86::REPZ

extern Vector<char*> x86_opcode_ext_names;

// following used only in x86/init.cc
void init_x86_opcode_ext_names();


extern Vector<int> x86_invert_table;

// following used only in x86/init.cc
void init_x86_invert_table();


bool target_implements_x86(int opc);

int opcode_line_x86();
int opcode_ubr_x86();

int opcode_move_x86(TypeId);
int opcode_load_x86(TypeId);
int opcode_store_x86(TypeId);

int opcode_cbr_inverse_x86(int opcode);

#endif /* X86_OPCODES_H */
