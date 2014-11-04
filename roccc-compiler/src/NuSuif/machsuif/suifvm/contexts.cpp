/* file "suifvm/contexts.cpp" */

/*
   Copyright (c) 2000 The President and Fellows of Harvard College

   All rights reserved.

   This software is provided under the terms described in
   the "machine/copyright.h" include file.
   */

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "suifvm/contexts.h"
#endif

#include <machine/machine.h>

#include <suifvm/instr.h>
#include <suifvm/opcodes.h>
#include <suifvm/code_gen.h>
#include <suifvm/printer.h>
#include <suifvm/c_printer.h>
#include <suifvm/contexts.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

/* ----------------------------  SuifVmContext  ----------------------------- */


SuifVmContext::SuifVmContext()
{
  cached_code_gen = NULL;
}

SuifVmContext::~SuifVmContext()
{
  delete cached_code_gen;
}

/* ------------------------  MachineContextSuifVm  -------------------------- */

TypeId
MachineContextSuifVm::type_addr() const
{
  TargetInformationBlock *target_info = 
    find_target_information_block(the_suif_env);

  int size = target_info->get_pointer_size().c_int();

  if (size == 64)
    return type_p64;
  else if (size == 32)
    return type_p32;
  else
    claim(false, "Unexpected target pointer size");
  return 0;
}

Printer*
MachineContextSuifVm::target_printer() const
{
  if (cached_printer)
    return cached_printer;

  cached_printer = (Printer*) new PrinterSuifVm;
  return cached_printer;
}


CPrinter*
MachineContextSuifVm::target_c_printer() const
{
  if (cached_c_printer)
    return cached_c_printer;

  cached_c_printer = new CPrinterSuifVm;
  return cached_c_printer;
}

// FIXME: eliminate extra function calls below

bool
MachineContextSuifVm::is_ldc(Instr *mi) const
{
  return is_ldc_suifvm(mi);
}

bool
MachineContextSuifVm::is_move(Instr *mi) const
{
  return is_move_suifvm(mi);
}

bool
MachineContextSuifVm::is_cmove(Instr *mi) const
{
  return is_cmove_suifvm(mi);
}

bool
MachineContextSuifVm::is_line(Instr *mi) const
{
  return is_line_suifvm(mi);
}

bool
MachineContextSuifVm::is_ubr(Instr *mi) const
{
  return is_ubr_suifvm(mi);
}

bool
MachineContextSuifVm::is_cbr(Instr *mi) const
{
  return is_cbr_suifvm(mi);
}

bool
MachineContextSuifVm::is_call(Instr *mi) const
{
  return is_call_suifvm(mi);
}

bool
MachineContextSuifVm::is_return(Instr *mi) const
{
  return is_return_suifvm(mi);
}

bool
MachineContextSuifVm::is_binary_exp(Instr *mi) const
{
  return is_binary_exp_suifvm(mi);
}

bool
MachineContextSuifVm::is_unary_exp(Instr *mi) const
{
  return is_unary_exp_suifvm(mi);
}

bool
MachineContextSuifVm::is_commutative(Instr *mi) const
{
  return is_commutative_suifvm(mi);
}

bool
MachineContextSuifVm::is_two_opnd(Instr *mi) const
{
  return false;		// no two-operand instrs
}

bool
MachineContextSuifVm::reads_memory(Instr *mi) const
{
  return reads_memory_suifvm(mi);
}

bool
MachineContextSuifVm::writes_memory(Instr *mi) const
{
  return writes_memory_suifvm(mi);
}

bool
MachineContextSuifVm::is_builtin(Instr *mi) const
{
  return is_builtin_suifvm(mi);
}

char*
MachineContextSuifVm::opcode_name(int opcode) const
{
  return opcode_name_suifvm(opcode);
}

bool
MachineContextSuifVm::target_implements(int opcode) const
{
  return target_implements_suifvm(opcode);
}

int
MachineContextSuifVm::opcode_line() const
{
  return opcode_line_suifvm();
}

int
MachineContextSuifVm::opcode_ubr() const
{
  return opcode_ubr_suifvm();
}

int
MachineContextSuifVm::opcode_move(TypeId type) const
{
  return opcode_move_suifvm(type);
}

int
MachineContextSuifVm::opcode_load(TypeId type) const
{
  return opcode_load_suifvm(type);
}

int
MachineContextSuifVm::opcode_store(TypeId type) const
{
  return opcode_store_suifvm(type);
}

int
MachineContextSuifVm::opcode_cbr_inverse(int opcode) const
{
  return opcode_cbr_inverse_suifvm(opcode);
}
