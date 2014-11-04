#include "common/system_specific.h"
#include "common/suif_hash_map.h"
#include "print_subsystem.h"

#include "iokernel/meta_class.h"
#include "iokernel/object_factory.h"
#include "iokernel/object_wrapper.h"
#include "common/formatted.h"
#include "suif_object.h"
#include "module_subsystem.h"
#include "module.h"

#include "suif_env.h"

#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

PrintSubSystem::PrintSubSystem(SuifEnv* suif) :
  SubSystem(suif),
  _string_map(0)
{
  suif->set_print_subsystem(this);

  // Initialize a small size at least!
}


PrintSubSystem::~PrintSubSystem() {
  if (_string_map) {
    for (StringMap::iterator iter = _string_map->begin();
	 iter != _string_map->end(); iter++) {
      delete (*iter).second;
    }
    delete(_string_map);
  }
}


/*
 * This function uses the 
 * SuifObject::print(FormattedText&) method
 * for printing suif objects. Otherwise
 * it prints an error message
 */
static void print_default( ostream& output, 
			   const ObjectWrapper &obj)
{
  if (is_kind_of_suif_object_meta_class(obj.get_meta_class()))
    {
      FormattedText text;
      ( (SuifObject*)obj.get_address()) -> print(text);
      output << text.get_value() << std::endl;
    }
  else
    {
      output << "No available printer for a non-SuifObject";
    }
  return;
}
  


void PrintSubSystem::print( const LString &style, 
			    ostream& output, 
			    const ObjectWrapper &obj) const
{
  ModuleSubSystem *ms = get_suif_env()->get_module_subsystem();
  if (style == emptyLString
      || !ms
      || !ms->is_available(style)
      || !ms->is_initialized(style))
    {
      print_default(output, obj);
      return;
    }
  Module *module = ms->retrieve_module(style);
  LString print_iface = "print";
  if (!module->supports_interface(print_iface))
    {
      print_default(output, obj);
      return;
    }
  PrintDispatch pfunc = (PrintDispatch)module->get_interface(print_iface);
  if (pfunc)
    {
      (*pfunc)(module, output, obj);
    }
  else
    {
      output << "installed style" 
	     << style.c_str() << "has no valid print function" << std::endl;
    }
}

void PrintSubSystem::print( const LString &style, 
			    ostream& output, 
			    SuifObject *obj) const
{
  if (obj == NULL) return;
  print(style, output, ObjectWrapper(obj));
}

String PrintSubSystem::print_to_string(const LString &style,
					       const ObjectWrapper &obj) const
{
  //ostrstream str
  std::stringstream str;
  print(style, str, obj);
  return(String(str.str().c_str()));//, str.pcount()));
}

String PrintSubSystem::print_to_string(const LString &style,
				       SuifObject *obj) const
{
  //ostrstream str;
  std::stringstream str;
  print(style, str, ObjectWrapper(obj));
  return(String(str.str().c_str())); //, str.pcount()));
}

/* Interface where the default style is used
 * the default im
 */
void PrintSubSystem::print(ostream& output, 
		   const ObjectWrapper &obj) const
{
  print(get_default_print_style(), output, obj);
}


void PrintSubSystem::print(FILE *fp, const ObjectWrapper &obj) const
{
  std::cerr << "UNSUPPORTED: PRINTSUBSYSTEM::PRINT(FILE*): LINE(" << __LINE__ << ") " 
    << "FILE(" << __FILE__ << ")" << std::endl;

  std::ofstream str("z.PrintSubSystem.out"); //fileno(fp));
//   std::filebuf buf;//(fp);//, std::ios_base::out);
//   buf.attach(fileno(fp));
//   //str.
//   str.setbuf(rdbuf(&buf); 
//   dynamic_cast<std::ostream*>(&str)->rdbuf( &buf);
  print(get_default_print_style(), str, obj);
}

String PrintSubSystem::print_to_string(const ObjectWrapper &obj) const
{
  std::stringstream str;
  //ostrstream str;
  print(get_default_print_style(), str, obj);
  return(String(str.str().c_str())); //, str.pcount()));
}

void PrintSubSystem::print( const ObjectWrapper &obj ) const
{
  print(get_default_print_style(), get_default_stream(), obj);
}
void PrintSubSystem::print(SuifObject *obj) const
{
  if (obj == NULL) return;
  print(get_default_print_style(), get_default_stream(), ObjectWrapper(obj));
}
String PrintSubSystem::print_to_string(SuifObject *obj) const
{
  if (obj == NULL) return("");
  //ostrstream str;
  std::stringstream str;

  print(get_default_print_style(), str, ObjectWrapper(obj));
  return(String(str.str().c_str())); //, str.pcount()));
}
  


ostream& PrintSubSystem::get_default_stream() const {
  return std::cerr;
}


LString PrintSubSystem::get_default_print_style() const {
  return(_style);
}
void PrintSubSystem::set_default_print_style(const LString &style) {
  _style = style;
}


PrintSubSystem::PrintSubSystem(const PrintSubSystem &other) :
  SubSystem(other._suif_env)
{
}
PrintSubSystem& PrintSubSystem::operator=(const PrintSubSystem &other) {
  suif_assert(0); return(*this);
}

PrintStringRepository *PrintSubSystem::
retrieve_string_repository(const LString &style)
{
  if (_string_map == NULL)
    _string_map = new StringMap;
  StringMap::iterator iter = _string_map->find(style);
  if (iter == _string_map->end())
    // (*_string_map)[style] = new PrintStringRepository;
    _string_map->enter_value(style, new PrintStringRepository);
	   
  //  return((*_string_map)[style]);
  return _string_map->lookup(style);
}

//-------------------------------------------------------------------
// Print String repository
//-------------------------------------------------------------------

PrintStringRepository::PrintStringRepository() :
  _print_full(new suif_hash_map<LString, String>),
  _print_ref(new suif_hash_map<LString, String>)
{}
PrintStringRepository::~PrintStringRepository() 
{
  delete _print_ref;
  delete _print_full;
}
  
String PrintStringRepository::get_print_ref_string(const LString &key) const
{
  if (!has_print_ref_string(key)) return emptyString;
  //  return((*_print_ref)[key]);
  return _print_ref->lookup(key);
}

String PrintStringRepository::get_print_string(const LString &key) const
{
  if (!has_print_string(key)) return emptyString;
  //  return((*_print_full)[key]);
  return _print_full->lookup(key);
}
void PrintStringRepository::set_print_ref_string(const LString &key,
						 const String &value)
{
  //  (*_print_ref)[key] = value;
  _print_ref->enter_value(key, value);
}
void PrintStringRepository::set_print_string(const LString &key,
					       const String &value) {
  //  (*_print_full)[key] = value;
  _print_full->enter_value(key, value);
}

void PrintStringRepository::set_print_string_by_table(const PrintSpecClass *table,
						      size_t size) {
  for (size_t i = 0; i < size; i++) {
    String spec(table[i].print_spec);
    set_print_string(table[i].class_name, spec);
  }
}

void PrintStringRepository::set_print_ref_string_by_table(const PrintSpecClass *table,
							  size_t size) {
  for (size_t i = 0; i < size; i++) {
    set_print_ref_string(table[i].class_name,
			 table[i].print_spec);
  }
}

bool PrintStringRepository::has_print_ref_string(const LString &key) const
{
  return(_print_ref->find(key) != _print_ref->end());
}
bool PrintStringRepository::has_print_string(const LString &key) const
{
  return(_print_full->find(key) != _print_full->end());
}
