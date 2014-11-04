#include "mc_style.h"
#include "suifkernel/cascading_map.h"
#include "basicnodes/basic_constants.h"
#include "basicnodes/basic.h"
#include "suifnodes/suif.h"
#include "iokernel/object_wrapper.h"
#include "suifkernel/module_subsystem.h"
#include "basicnodes/basic_constants.h"

#include "iokernel/pointer_meta_class.h"
#include "iokernel/string_meta_class.h"
#include "iokernel/lstring_meta_class.h"
#include "common/i_integer.h"

// #ifndef MSVC
// #include <iostream>
// #else
// #include <iostream.h>
// #endif
#include <iostream>

using namespace std;

LString MCPrintStyleModule::get_class_name() {
  static LString name("mcprint");
  return(name);
}

void MCPrintStyleModule::initialize() {
  Module::initialize();
  
 set_interface("print", (Address)MCPrintStyleModule::print_dispatch);

}

void MCPrintStyleModule::print_dispatch(Module *module,
				       ostream &str,
				       const ObjectWrapper &obj)
{
  MCPrintStyleModule *pm = (MCPrintStyleModule*)module;
  
  pm->print_it(str, obj, "");
}

#if 0
static String obj_to_string(const ObjectWrapper &obj) {
  String result = obj.get_meta_class()->get_instance_name;
  result += "@";
  result += obj.get_address();
  return(result);
}
#endif

static void obj_print(ostream &str, const ObjectWrapper &obj) {
  LString name;
  
  if (obj.get_meta_class() != NULL) {
    name = obj.get_meta_class()->get_instance_name();

    static LString lstring_lstring = "LString";
    static LString string_lstring = "String";
    static LString iinteger_lstring = "IInteger";

    if (name == lstring_lstring) {
      LString *s = (LString*)obj.get_address();
      cerr << '\'';
      cerr << *s;
      cerr << '\'';
      return;
    }
    if (name == string_lstring) {
      String *s = (String*)obj.get_address();
      cerr << '\'';
      cerr << *s;
      cerr << '\'';
      return;
    }
    if (name == iinteger_lstring) {
      IInteger *s = (IInteger*)obj.get_address();
      cerr << (*s).to_String();
      return;
    }
  }

  // wouldn't it be nice to have a printer...
  cerr << name << " @ " << obj.get_address();
}

static ostream & operator<<(ostream &str, const ObjectWrapper &obj) {
  obj_print(str, obj);
  return(str);
}

void MCPrintStyleModule::print_it(ostream &str,
				    const ObjectWrapper &obj,
				    String indent)
{
  String result;
  str << indent;
  str << obj;
  //  obj_print(str, obj);
  str << "\n";

  bool has_one = false;

  indent += " ";
  // First iterate over the referenced pointers
  Iterator *iter = obj.get_meta_class()->get_iterator(obj, Iterator::Referenced);
  for (; iter && iter->is_valid(); iter->next()) {
    FieldWrapper field = iter->current_field();
    // ObjectWrapper field_obj = field.get_object();
    //    if (is_kind_of<PointerMetaClass>(field_obj.get_meta_class()))
    //      continue;
    has_one = true;
    str << indent;
    LString fieldname = field.get_field_name();
    if (fieldname != emptyLString)
      {
	str << fieldname;
	str << ": ";
      }
    str << field.get_object();
    str << "\n";
  }
  delete iter;


  iter = obj.get_meta_class()->get_iterator(obj, Iterator::Owned);
  for (; iter && iter->is_valid(); iter->next()) {
    // Next recursively print owned things
    FieldWrapper field = iter->current_field();
    // ObjectWrapper field_obj = field.get_object();
    //    if (is_kind_of<PointerMetaClass>(field_obj.get_meta_class())) {
    //      print_it(str, field_obj, indent);
    //      continue;
    //    }
    
    has_one = true;
    LString fieldname = field.get_field_name();
    if (fieldname != emptyLString)
      {
	str << indent;
	str << fieldname;
	str << ":= \n";
      }
    {
      String new_indent = indent + " ";
      print_it(str, field.get_object(), new_indent);
    }
  }
  delete iter;
  //  if (has_one)
  //    str << "\n";
}

MCPrintStyleModule::MCPrintStyleModule(SuifEnv *s, const LString &name) :
  Module(s, name)
{

}
Module *MCPrintStyleModule::clone() const {
  return((Module*)this);
}

