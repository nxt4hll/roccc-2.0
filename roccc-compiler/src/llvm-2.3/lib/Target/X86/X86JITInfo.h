//===- X86JITInfo.h - X86 implementation of the JIT interface  --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the X86 implementation of the TargetJITInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef X86JITINFO_H
#define X86JITINFO_H

#include "llvm/Function.h"
#include "llvm/Target/TargetJITInfo.h"

namespace llvm {
  class X86TargetMachine;

  class X86JITInfo : public TargetJITInfo {
    X86TargetMachine &TM;
    intptr_t PICBase;
  public:
    explicit X86JITInfo(X86TargetMachine &tm) : TM(tm) {useGOT = 0;}

    /// replaceMachineCodeForFunction - Make it so that calling the function
    /// whose machine code is at OLD turns into a call to NEW, perhaps by
    /// overwriting OLD with a branch to NEW.  This is used for self-modifying
    /// code.
    ///
    virtual void replaceMachineCodeForFunction(void *Old, void *New);

    /// emitGlobalValueLazyPtr - Use the specified MachineCodeEmitter object to
    /// emit a lazy pointer which contains the address of the specified ptr.
    virtual void *emitGlobalValueLazyPtr(const GlobalValue* GV, void *ptr,
                                         MachineCodeEmitter &MCE);

    /// emitFunctionStub - Use the specified MachineCodeEmitter object to emit a
    /// small native function that simply calls the function at the specified
    /// address.
    virtual void *emitFunctionStub(const Function* F, void *Fn,
                                   MachineCodeEmitter &MCE);

    /// getPICJumpTableEntry - Returns the value of the jumptable entry for the
    /// specific basic block.
    virtual intptr_t getPICJumpTableEntry(intptr_t BB, intptr_t JTBase);

    /// getLazyResolverFunction - Expose the lazy resolver to the JIT.
    virtual LazyResolverFn getLazyResolverFunction(JITCompilerFn);

    /// relocate - Before the JIT can run a block of code that has been emitted,
    /// it must rewrite the code to contain the actual addresses of any
    /// referenced global symbols.
    virtual void relocate(void *Function, MachineRelocation *MR,
                          unsigned NumRelocs, unsigned char* GOTBase);

    /// setPICBase / getPICBase - Getter / setter of PICBase, used to compute
    /// PIC jumptable entry.
    void setPICBase(intptr_t Base) { PICBase = Base; }
    intptr_t getPICBase() const { return PICBase; }
  };
}

#endif
