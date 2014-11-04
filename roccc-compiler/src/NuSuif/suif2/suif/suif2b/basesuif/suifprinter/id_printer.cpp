/**
  * file : id_printer.cpp
  *
  */


#include "id_printer.h"
#include "suifkernel/cascading_map.h"
#include "basicnodes/basic_constants.h"
#include "basicnodes/basic.h"
#include "suifnodes/suif.h"
#include "iokernel/object_wrapper.h"
#include "suifkernel/module_subsystem.h"
#include "basicnodes/basic_constants.h"
#include <iostream>

using namespace std;

static String to_hex_address(const Object* curAddr)
{
   char addr_str[256];
   sprintf(addr_str, "0x%p", (void*)curAddr);
   return String(addr_str);
}





void IdPrinter::print_id(Module *,
			 ostream &str,
			 const ObjectWrapper &obw)
{
  Object* obj = obw.get_object();
  if (obj != 0) {  
    if (is_kind_of<SymbolTableObject>(obj)) {
      SymbolTableObject *sobj = to<SymbolTableObject>(obj);
      if (sobj->get_name().length() > 0)
	str << "<" << to<SymbolTableObject>(obj)->get_name() << ">_";
    }
    str << (obj->getClassName()).c_str() << "_" << to_hex_address(obj);
    return;
  }
  str << "Addr[" << obw.get_address() << "]";
}



IdPrinter::IdPrinter(SuifEnv *s, const LString &name) :
  Module(s, name)
{}


Module* IdPrinter::clone(void) const
{
  return const_cast<IdPrinter*>(this);
}

void IdPrinter::initialize() {
  Module::initialize();
  set_interface("print", (Address)IdPrinter::print_id);
}


