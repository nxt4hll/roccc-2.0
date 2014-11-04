/* file "machine/opcodes.cpp" */

/*
   Copyright (c) 2000 The President and Fellows of Harvard College

   All rights reserved.

   This software is provided under the terms described in
   the "machine/copyright.h" include file.
   */

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "machine/opcodes.h"
#endif

#include <machine/substrate.h>
#include <machine/contexts.h>
#include <machine/opcodes.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif


  int
opcode_line()
{
  return dynamic_cast<MachineContext*>(the_context)->opcode_line();
}

  int
opcode_ubr()
{
  return dynamic_cast<MachineContext*>(the_context)->opcode_ubr();
}

  int
opcode_move(TypeId type)
{
  return dynamic_cast<MachineContext*>(the_context)->opcode_move(type);
}

  int
opcode_load(TypeId type)
{
  return dynamic_cast<MachineContext*>(the_context)->opcode_load(type);
}

  int
opcode_store(TypeId type)
{
  return dynamic_cast<MachineContext*>(the_context)->opcode_store(type);
}

  int
opcode_cbr_inverse(int opcode)
{
  return
    dynamic_cast<MachineContext*>(the_context)->opcode_cbr_inverse(opcode);
}


  bool
target_implements(int opcode)
{
  return
    dynamic_cast<MachineContext*>(the_context)->target_implements(opcode);
}

  char*
opcode_name(int opcode)
{
  return dynamic_cast<MachineContext*>(the_context)->opcode_name(opcode);
}

Map<IdString,int> name_to_opcode;

  int
opcode_from_name(const char *name)
{
  static bool map_ready;
  if (!map_ready)
  {
    for (int i = 2; target_implements(i); ++i)
      name_to_opcode[opcode_name(i)] = i;
    map_ready = true;
  }

  Map<IdString,int>::iterator it = name_to_opcode.find(name);
  claim(it != name_to_opcode.end(), "Not a valid opcode name: '%s'", name);

  return (*it).second;
}
