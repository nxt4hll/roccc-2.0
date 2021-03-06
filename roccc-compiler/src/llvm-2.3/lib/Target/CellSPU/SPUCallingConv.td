//===- SPUCallingConv.td - Calling Conventions for CellSPU ------*- C++ -*-===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// This describes the calling conventions for the STI Cell SPU architecture.
//
//===----------------------------------------------------------------------===//

/// CCIfSubtarget - Match if the current subtarget has a feature F.
class CCIfSubtarget<string F, CCAction A>
 : CCIf<!strconcat("State.getTarget().getSubtarget<PPCSubtarget>().", F), A>;

//===----------------------------------------------------------------------===//
// Return Value Calling Convention
//===----------------------------------------------------------------------===//

// Return-value convention for Cell SPU: Everything can be passed back via $3:
def RetCC_SPU : CallingConv<[
  CCIfType<[i32], CCAssignToReg<[R3]>>,
  CCIfType<[i64], CCAssignToReg<[R3]>>,
  CCIfType<[f32, f64], CCAssignToReg<[R3]>>,
  CCIfType<[v16i8, v8i16, v4i32, v2i64, v4f32, v2f64], CCAssignToReg<[R3]>>
]>;


//===----------------------------------------------------------------------===//
// CellSPU Argument Calling Conventions
// FIXME
//===----------------------------------------------------------------------===//
/*
def CC_SPU : CallingConv<[
  // The first 8 integer arguments are passed in integer registers.
  CCIfType<[i32], CCAssignToReg<[R3, R4, R5, R6, R7, R8, R9, R10]>>,
  CCIfType<[i64], CCAssignToReg<[X3, X4, X5, X6, X7, X8, X9, X10]>>,
  
  // SPU can pass back arguments in all 
  CCIfType<[f32, f64], CCIfSubtarget<"isMachoABI()",
           CCAssignToReg<[F1, F2, F3, F4, F5, F6, F7, F8,F9,F10,F11,F12,F13]>>>,
  // Other sub-targets pass FP values in F1-10.
  CCIfType<[f32, f64], CCAssignToReg<[F1, F2, F3, F4, F5, F6, F7, F8, F9,F10]>>,
           
  // The first 12 Vector arguments are passed in altivec registers.
  CCIfType<[v16i8, v8i16, v4i32, v4f32],
              CCAssignToReg<[V2, V3, V4, V5, V6, V7, V8, V9, V10,V11,V12,V13]>>
/*
  // Integer/FP values get stored in stack slots that are 8 bytes in size and
  // 8-byte aligned if there are no more registers to hold them.
  CCIfType<[i32, i64, f32, f64], CCAssignToStack<8, 8>>,
  
  // Vectors get 16-byte stack slots that are 16-byte aligned.
  CCIfType<[v16i8, v8i16, v4i32, v2i64, v4f32, v2f64],
              CCAssignToStack<16, 16>>*/
]>;
 */
