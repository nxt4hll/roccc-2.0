#include "common/system_specific.h"
#include "print_utils.h"
#include "iokernel/cast.h"
#include "basicnodes/basic.h"
#include <stdio.h>

static String to_hex_address(const Object* curAddr)
{
   char addr_str[256];
   sprintf(addr_str, "0x%p", (void*)curAddr);
   return String(addr_str);
}


// Return a human readable string that identifies the object.
//
String to_id_string(const Object *obj)
{
  if (obj == NULL) return String("NULL");
  String name;
  if (is_kind_of<SymbolTableObject>(obj)) {
    SymbolTableObject *sobj = to<SymbolTableObject>(obj);
    if (sobj->get_name().length() > 0)
      name += String("<") + to<SymbolTableObject>(obj)->get_name() + ">_";
  }
  name += (obj->getClassName()).c_str();
  name += String("_") + to_hex_address(obj);
  return name;
}

