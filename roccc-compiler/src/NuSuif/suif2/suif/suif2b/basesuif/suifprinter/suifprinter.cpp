#include "common/system_specific.h"
#include "common/suif_hash_map.h"
#include <ctype.h>
#include "suifprinter.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/suif_object.h"
#include "suifkernel/suif_env.h"
#include "iokernel/meta_class.h"
#include "iokernel/aggregate_meta_class.h"
#include "iokernel/list_meta_class.h"
#include "iokernel/pointer_meta_class.h"
#include "iokernel/stl_meta_class.h"
#include "basicnodes/basic.h"
#include "iokernel/field_description.h"

/* These are used for passing wrappers */
#include "iokernel/object_wrapper.h"
#include "iokernel/pointer_wrapper.h"
#include "iokernel/aggregate_wrapper.h"

/* These are used for safe checking */
#include "iokernel/i_integer_meta_class.h"
#include "iokernel/integer_meta_class.h"
#include "iokernel/lstring_meta_class.h"
#include "iokernel/string_meta_class.h"

#include "print_strings.h"
#include "cprint_style.h"
#include "id_printer.h"
#include "mc_style.h"

#ifdef MSVC
#include <stdlib.h>
#include <string.h>
#endif

using namespace std;

//----------------------------------------------------------------------------
// Object Tags
//----------------------------------------------------------------------------

ObjectTags::ObjectTags() 
  : _tags(new TagMap),
    _next_tag(0)
{
}
ObjectTags::~ObjectTags() {
  delete _tags;
}

size_t ObjectTags::retrieve_tag(const ObjectWrapper &obj)
{
  TagMap::iterator iter = _tags->find(obj.get_address());
  if (iter == _tags->end())
    {
      // (*_tags)[obj.get_address()] = _next_tag;
      _tags->enter_value(obj.get_address(), _next_tag);
      _next_tag++;
      return(_next_tag-1);
    }
  else
    {
      return((*iter).second);
    }
}
size_t ObjectTags::get_tag(const ObjectWrapper &obj)
{
  TagMap::iterator iter = _tags->find(obj.get_address());
  suif_assert_message(iter != _tags->end(),
		      ("invalid tag retrieval"));
  return((*iter).second);
}
bool ObjectTags::has_tag(const ObjectWrapper &obj)
{
  TagMap::iterator iter = _tags->find(obj.get_address());
  return(iter != _tags->end());
}





const int istep = 2;
//----------------------------------------------------------------------------
// Module Stuff
//----------------------------------------------------------------------------
LString SuifPrinterModule::ClassName("SuifPrinterModule");

//----------------------------------------------------------------------------
extern "C" void EXPORT init_suifprinter_extras(SuifEnv* s)
{
  PrintSubSystem *print_sys = s->get_print_subsystem();
  PrintStringRepository *psub = 
    print_sys->retrieve_string_repository("print_suif");
  psub->set_print_string_by_table(defaultPrintStrings,
				  PRINT_SIZE);
  psub->set_print_ref_string_by_table(defaultPrintRefStrings,
				      PRINT_REF_SIZE);
}
//----------------------------------------------------------------------------
extern "C" void EXPORT init_suifprinter(SuifEnv* suif)
{
  suif->require_module("suifpasses");
  // These are needed by the cprint style
  suif->require_module("basicnodes");
  suif->require_module("suifnodes");

  ModuleSubSystem* mSubSystem = suif->get_module_subsystem();
  if (!mSubSystem->retrieve_module(SuifPrinterModule::ClassName)) {
    Module *m = new SuifPrinterModule(suif);
    mSubSystem -> register_module(m);
    suif->get_print_subsystem()->set_default_print_style(m->get_module_name());

    m = new CPrintStyleModule(suif);
    mSubSystem -> register_module(m);
    init_suifprinter_extras(suif);

    m = new IdPrinter(suif);
    mSubSystem -> register_module(m);

    m = new MCPrintStyleModule(suif);
    mSubSystem -> register_module(m);
  }
}

/*
// helper for outputting a metaclass/address pair
static void output_obj(ostream &output, const ObjectWrapper &obj)
{
  output << obj.get_meta_class()->get_instance_name();
  //  output << '(' << obj.get_meta_class()->get_meta_class_id() << ")";
  output << '<' << obj.get_address() << '>';
}
*/
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
SuifPrinterModule::SuifPrinterModule(SuifEnv* suif)
  : Module(suif, "print_suif"), 
  _print_all(false), _use_print_string(true),
  _use_print_ref_string(true), _initialized(false),
  output(cout),
  //_print_inits(new list<print_init_fn>) { init(); }
  _print_inits(new list<print_init_fn>) { }
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void SuifPrinterModule::initialize()
{
   Module::initialize();
   // register as a listener
   ModuleSubSystem *ms = get_suif_env()->get_module_subsystem();
   ms->register_interface_listener(this, "print_init");
   //   ms->register_interface_producer(this, "print");
   set_interface("print", (Address)SuifPrinterModule::print_dispatch);
   
   init();
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void
SuifPrinterModule::init()
{
  _pr = _suif_env->get_print_subsystem();
  if (!_pr) {
     cerr << "SuifPrinterModule: There is no PrintSubSystem!!!\n";
     exit(1);
  }
  _string_table = _pr->retrieve_string_repository(get_module_name());

  /*
  // Now read in from a standard set of print_strings pre-defined for
  // suifnodes and basicnodes.
  int size = sizeof(defaultPrintStrings)/sizeof(PrnStrClass);

  {
    for (int i = 0; i < size; ++i) {
      _string_table->set_print_string(defaultPrintStrings[i].class_name,
				      defaultPrintStrings[i].prn_str);
    }
  }
  size = sizeof(defaultPrintRefStrings)/sizeof(PrnStrClass);
  for (int i = 0; i < size; ++i) {
     _string_table->set_print_ref_string(defaultPrintRefStrings[i].class_name,
					 defaultPrintRefStrings[i].prn_str);
  }
  */
  /*
 // Now do the deferred initialization
  for (list<print_init_fn>::iterator iter = _print_inits->begin();
       iter != _print_inits->end();
       iter++) {
    print_init_fn fn = (*iter);
    (*fn)(_suif_env);
  }
  */
  _initialized = true;

}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void
SuifPrinterModule::interface_object_created(Module *producer,
                                            const LString &interface_name)
{
  Address addr = producer->get_interface(interface_name);
  // Here's the scary part.  Convert it.
  if (addr == 0) return;
  print_init_fn fn = (print_init_fn) addr;
  _print_inits->push_back(fn);
  if (_initialized) {
    (*fn)(_suif_env);
  }
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void SuifPrinterModule::print_dispatch(Module *module,
				       ostream &str,
				       const ObjectWrapper &obj)
{
  SuifPrinterModule *pm = (SuifPrinterModule*)module;
  pm->print(str, obj);
}


void SuifPrinterModule::execute()
{
  print();
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

Module *SuifPrinterModule::clone() const { return(Module*)this; }

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void
SuifPrinterModule::print(ostream& output, const SuifObject* s, const MetaClass* type)
{
  print(output, (const Address)s, type, 0);
}

void
SuifPrinterModule::print(ostream& output, const ObjectWrapper &obj)
{
  print(output, obj.get_address(), obj.get_meta_class(), 0);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
int list_separator = ' ';
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

bool
SuifPrinterModule::parse_and_print(ostream& output, const ObjectWrapper &obj,
				   const LString &name, const String &str, 
				   int _indent, int deref = 0)
{
  const Address what = obj.get_address();
  const MetaClass *type = obj.get_meta_class();
  //  ObjectWrapper obj(what, type);
  //output << "str:deref = " << deref << endl;
  int l_sep = list_separator;
  if (str.length() == 0) {
    return false;
  }
  if (deref)
    _indent -= istep;
  int str_length = str.length();
  bool need_indent = false;
  bool first_field = true;
  for (int i = 0; i < str_length; ++i) {
    if (str[i] != '%') {
      // If there are less than 2 extra stars don't print anything but the
      // fields.
      if (deref < 2) {
	// Need to check for \n and take care of indentation here
	switch(str[i]) {
	case '\n':
	  output << str[i];
	  if (str[i+1]) {
	    need_indent = true;
	    indent(output, _indent);
	  }
	  break;
	case '\t': indent(output, istep); break;
	case '\b': _indent -= istep; break;
	default: output << str[i];
	}
      }
    }
    else {
      ++i;
      if (str[i] == '%') {
	// Double % means print out a '%'
	output << '%';
      }
      else {
	// This has to be cleaned up a bit ...
	//int field_deref = deref?deref-1:0;
	int field_deref = 0;
	char buff[256];
	int j = 0;
	char c = str[i++];
	if (c == '*') {
	  ++field_deref;
	  while ((c = str[i++]) == '*')
	    ++ field_deref;
	}
	while (isalnum(c) || c == '_') {
	  buff[j++] = c;
	  c = str[i++];
	}
	i -= 2;
	buff[j] = 0;
	// Now retrieve the particular field and print it.
	if (!strcmp(buff, "Cl")) {
	  output << type->get_instance_name() << '('
		 << type->get_meta_class_id() << ") ";
	}
	else if (!strcmp(buff, "ii")) { // Deal with printing IInteger
	  IInteger *ii = (IInteger *)what;
	  output << ii->to_String().c_str();
	}
	else if (!strcmp(buff, "i")) { // Deal with printing int
	  output << *(int*)what;
	}
	else if (!strcmp(buff, "f")) { // float
	  output << *(float*)what;
	}
	else if (!strcmp(buff, "d")) { // double
	  output << *(double*)what;
	}
	else if (!strcmp(buff, "c")) { // char
	  output << *(char*)what;
	}
	else if (!strcmp(buff, "b")) { // byte
	  output << (int)*(char*)what;
	}
	else if (!strcmp(buff, "B")) { // bool
	  output << *(bool*)what;
	}
	else if (!strcmp(buff, "ls")) { // Deal with printing LStrings
	  LString str = *(LString*) what;
	  output << str;
	}
	else if (!strcmp(buff, "s")) { // Deal with printing Strings
	  String str = *(String*) what;
	  output << str;
	}
	else if (!strcmp(buff, "n")) { // Deal with name of field
	  if (!deref)
	    output << name;
	}
	else if (!strcmp(buff, "P")) {
	  if (obj.is_null())
	    output << "NULL";
	  else {
	    PointerWrapper ptr_obj(obj);
	    ObjectWrapper base_obj = ptr_obj.dereference();
	    if (ptr_obj.get_meta_class()->is_owning_pointer()) {
	      size_t ref = retrieve_tag(obj.get_object());
	      output << "t" << ref<< ": ";
	      print2(output, base_obj, emptyLString, 
		     _indent+istep, field_deref);
	    }
	    else {
	      print_pointer(output, ptr_obj, emptyLString, _indent, deref);
	    }
	  }
	}
	else if (!strcmp(buff, "R")) { // print the ref #
	  if (!what)
	    output << "NULL";
	  else {
	    //  PointerMetaClass *p = (PointerMetaClass*) type;
	    //  const Address baseAddr = *(Address*) type;
	    //ObjectWrapper obj(what, type);
	    size_t ref = retrieve_tag(obj);
	    output << "t" << ref<< ": ";
	  }
	}
	else if (!strcmp(buff, "LS")) {
	  list_separator = ' ';
	}
	else if (!strcmp(buff, "LN")) {
	  list_separator = '\n';
	}
	else if (!strcmp(buff, "LC")) {
	  list_separator = ',';
	}
	else if (!strcmp(buff, "ANNOTES")) {
	  // Special CASE for handling ANNOTATIONS
	  AggregateWrapper agg(obj);
	  LString field_name("_annotes");
	  FieldDescription *f = agg.get_field_description(field_name);
	  if (!f)
	    cerr << type->get_meta_class(what)->get_class_name()
		 << ":No field '" << field_name << "' found to print!!!\n";
	  else {
	    // Now we need to get the field offset and increment 'what'
	    if (field_deref != 0)
	      cerr << "Extra '*' for %ANNOTES\n";
	    FieldWrapper field = agg.get_field(field_name);
	    if (need_indent) {
	      indent(output, istep);
	      need_indent = false;
	    }
	    char old_sep = list_separator;
	    list_separator = '\n';
	    print2(output, 
		   field.get_object(),
		   field_name, _indent+istep,
		   1);
	    list_separator = old_sep;
	  }
	}
	else if (j) {
	  // Retrieve the field mentioned
	  // The following cast works as we should reach here only if it
	  // is not an elementary or pointer type.
	  AggregateWrapper agg(obj);
	  char *bf = buff;
	  LString field_name(bf);
	  FieldDescription *f = agg.get_field_description(field_name);
	  if (!f)
	    cerr << type->get_meta_class(what)->get_class_name()
		 << ":No field '" << field_name << "' found to print!!!\n";
	  else {
	    // Now we need to get the field offset and increment 'what'
	    if (deref)
	      if (!first_field) output << ' ';
	      else first_field = false;
	    FieldWrapper field = agg.get_field(field_name);
	    //char *f_add = (char*)what + f->get_offset();
	    //indent(output, _indent+istep);
	    if (need_indent) {
	      indent(output, istep);
	      need_indent = false;
	    }
	    if (deref && !field_deref)
	      field_deref = deref - 1;
	    //output << "\tstr:field_deref = " << field_deref << endl;
	    print2(output, 
		   field.get_object(),
		   field_name, _indent+istep,
		   field_deref);
	  }
	}
      }
    }
  }
  list_separator = l_sep;
  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool
SuifPrinterModule::print_elementary(ostream& output, 
				    const ObjectWrapper &obj,
				    const LString &name, 
				    int _indent,int deref = 0)
{
  const Address what = obj.get_address();
  const MetaClass* type = obj.get_meta_class();
  //output << "elem:deref = " << deref << endl;
  if (!deref) {
    output << type->get_instance_name()
	   << '(' << type->get_meta_class_id() << ')';
    if (name)
      output << ' ' << name << ' ';
  }
  if (what) {
    if (type->isKindOf("String"))
      output <<'[' << *(String*)what <<"] ";
    else
      output <<'[' << *(int*)what <<"] ";
  }
  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool
SuifPrinterModule::print_aggregate(ostream& output, 
				   const AggregateWrapper &obj,
				   const LString &name, 
				   int _indent,int deref = 0)
{
  const Address what = obj.get_address();
  const MetaClass* type  = obj.get_meta_class();
  //output << "aggr:deref = " << deref << endl;
  int field_deref = deref?deref-1:0;
  if (!deref) {
    output << type->get_instance_name()
	   << '(' << type->get_meta_class_id() << ')';
    if (name)
      output << ' ' << name << ' ';
    //output << "{\n";
    output << ":\n";
  }
  else if (_indent)
    _indent -= istep;
  Iterator* it = (Iterator*) type->get_iterator(what);
  bool last_printed = true;
  bool nl_needed = false;
  if (deref) last_printed = false;
  for (; it->is_valid(); it->next())
    {
      ObjectWrapper obj(it->current(), it->current_meta_class());
      if (!obj.is_null()) { // used to be if (obj.get_meta_class()) 
	if (last_printed) {
	  if (nl_needed) output << endl;
	  indent(output, _indent+istep);
	}
	if (last_printed = print2(output, 
				  obj,
				  it->current_name(), 
				  _indent+istep, field_deref))
	  nl_needed = true;
      }
    }
  // output << endl;
  delete it;
  //indent(output, _indent);
  //output << "}\n";
  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

/*
bool
SuifPrinterModule::print_pointer(ostream& output, const PointerWrapper &obj,
				 const LString &name, 
				 int _indent,int deref = 0)
{
  return(print_pointer(output, obj.get_address(), obj.get_meta_class(),
		       name, _indent, deref));
}
*/
bool
SuifPrinterModule::print_pointer(ostream& output, 
				 const PointerWrapper &ptr_obj,
				 const LString &name, 
				 int _indent,int deref = 0)
{
  const Address what = ptr_obj.get_address();
  const PointerMetaClass* type = ptr_obj.get_meta_class();
  //output << "ptr:deref = " << deref << endl;
  int field_deref = deref ? deref-1 : 0;

  //   PointerMetaClass *p = (PointerMetaClass*) type;
  //   const Address curAddr = *(Address*) what;
  //PointerWrapper ptr_obj(what, type);
  ObjectWrapper pobj = ptr_obj.get_object();

  ObjectWrapper base_obj = ptr_obj.dereference();
  //   suif_assert(base_obj.get_meta_class() ==
  //	       ptr_obj.get_meta_class()->get_base_type()->get_meta_class(*(Address*) what));

   
  if (!print_all())
    if (base_obj.is_null())
      return false;



  String name_with_addr = String("*") + name;
  //   LString new_name = "*";
  //   LString new_name_str = new_name + name;
  char addr_str[256];
  sprintf(addr_str, " <%p>", base_obj.get_address());
  //   String name_with_addr = new_name_str + addr_str;
  name_with_addr += addr_str;

  if (base_obj.is_null())
    {
      output << type->get_instance_name();
      output << '(' << type->get_meta_class_id() << ")";
      if (name)
	output << ' ' << name << ' ';
      output << " = NULL;";
      return true;
    }

  /*
   * for the "print_all" style
   */
  if (print_all()) 
    {
      if (ptr_obj.get_meta_class()->is_owning_pointer()) 
	{
	  output << type->get_instance_name();
	  output << '(' << type->get_meta_class_id() << ")";
	  output << '<' << what << '>';
	  print2(output, base_obj, emptyLString, _indent, field_deref);
	} else {
	  if (deref) {
	    output << name_with_addr <<":";
	  } else {
	    output << type->get_instance_name();
	    output << ' ' << name_with_addr;
	    output << " (ref)";
	  }
	}
      return true;
    }

  /*
   * This is the short printing
   */
  if (ptr_obj.get_meta_class()->is_owning_pointer()) 
    {
      if (!base_obj.is_null()) 
	{
	  if (!print_all())
	    print2(output, base_obj,
		   name_with_addr, _indent, field_deref);
	  else {
	    output << base_obj.get_meta_class()->get_instance_name();
	    output << '(' << type->get_meta_class_id() << ")";
	    output << '<' << what << '>';
	    print2(output, base_obj, emptyLString, _indent, field_deref);
	  }
	}
    } else {
      //      MetaClassId id = baseType->get_meta_class_id();
      String ref_str = 
	get_print_ref_string(base_obj.get_meta_class()->get_instance_name());

      //ObjectWrapper obj(what, baseType);
      if (has_tag(pobj))
	{
	  size_t ref = get_tag(pobj);
	  output << "@t" << ref;
	  if (use_print_ref_string() && ref_str != emptyString) {
            output<<':';
            return parse_and_print(output, base_obj, emptyLString, ref_str,
                                   _indent, field_deref);
	  }
	}
      else 
	{
	  //ObjectWrapper obj(curAddr, baseType);
	  if (has_tag(base_obj))
	    {
	      size_t ref = get_tag(base_obj);
	      output << "@t" << ref;
	      if (use_print_ref_string() && ref_str != emptyString) 
		{
		  output<<':';
		  return parse_and_print(output, base_obj, emptyLString, 
					 ref_str,
					 _indent, field_deref);
		}
	    }
	  else 
	    {
	      // No tag on the Pointer or the Object.
	      // Here see if there should have been a tag but just hasn't been
	      // seen yet. If so, we can declare one now which will be used later.
	      if (use_print_ref_string() && ref_str != emptyString) 
		{
		  //ObjectWrapper obj(curAddr, baseType);
		  size_t ref = retrieve_tag(base_obj);
		  output << "@t" << ref;
		  output <<':';
		  return parse_and_print(output, base_obj,
					 emptyLString, ref_str,
					 _indent, field_deref);
		}
	      else
		{
		  output << type->get_instance_name();
		  output << ' ' << name_with_addr;
		  output << " (ref)";
		}
	    }
	}
    }
  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool
SuifPrinterModule::print_list(ostream& output, 
			      const ObjectWrapper &obj,
			      const LString &name, 
			      int _indent, int deref = 0)
{
  const Address what = obj.get_address();
  const MetaClass* type = obj.get_meta_class(); 
  //output << "list:deref = " << deref << endl;
  int l_sep = list_separator;
  int field_deref = deref?deref-1:0;
  ListMetaClass *p = (ListMetaClass*) type;
  ListIterator* it = (ListIterator*) p->get_iterator(what);
  //  bool verbose_list = false;
  if (!print_all() && !it->is_valid()) {
      if (list_separator != '\n')
	output << "<None>";
    return true;
  }
  size_t length = it->length();
  if (!deref) {
    if (print_all()) {
      output << type->get_instance_name()
	     << '(' << type->get_meta_class_id() << ") ";
      if (name)
	output << ' ' << name << ' ';
    }
    else // if (length != 1)
      {
	output <<"list ";
	if (name)
	  output << ' ' << name << ' ';
      }
  }
  else if (_indent > 0)
    _indent -= istep;
  {
    if (print_all())
      output << '[' << length << "]:";
    if (it->current()) {
      int num_elts = 0;
      bool last_printed = true;
      bool nl_needed = false;
      if (deref && length == 1)
	last_printed = false;
      else {
	//output << endl;
      }
      //output << '{';
      //indent(output, istep);
      //while (it->is_valid()) {
      for (; it->is_valid();  it->next())
	{
	  list_separator = l_sep;
	  ObjectWrapper element(it->current(),
				it->current_meta_class());
	  //	    const MetaClass* currentMetaClass = it->current_meta_class();
	  //	    const Address curAddr = it->current();
	  if (!element.is_null()) {
	    //	    if (curAddr) {
	    if (last_printed &&
		nl_needed) {
	      output << (char)list_separator;
	      if (list_separator != ' ') {
		if (list_separator == '\n')
		  indent(output, _indent+istep);
		else if (list_separator == ',')
		  output << ' ';
	      }
	    }
	    char buff[100] = { 0 };
	    if (print_all())
	      sprintf(buff,"[%d]", num_elts++);
	    last_printed = print2(output, element,
				  buff, _indent+istep, field_deref);
	    nl_needed = last_printed;
	  }
	}
      if (last_printed &&
	  nl_needed) {
	if (list_separator == '\n') {
	  output << (char)list_separator;
	  indent(output, _indent+istep);
	}
      }
    } else
      output <<" {0x0} ";
  }
  delete it;
  list_separator = l_sep;
  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool
SuifPrinterModule::print_stl(ostream& output, 
			     const ObjectWrapper &obj,
			     const LString &name, int _indent,int deref = 0)
{
  const Address what = obj.get_address();
  const MetaClass* type = obj.get_meta_class();
   int field_deref = deref?deref-1:0;
   output << type->get_instance_name()
              << '(' << type->get_meta_class_id() << ") ";
   if (name)
         output << ' ' << name << ' ';
   STLMetaClass *p = (STLMetaClass*) type;
   ListIterator* it = (ListIterator*) p->get_iterator(what);
   output << '[' << it->length() << "] ";
   if (it->current()) {
          //output << "{\n";
          output << endl;
          //while (it->is_valid()) {
	  for (; it->is_valid(); it->next())
	    {
	      ObjectWrapper obj(it->current(), it->current_meta_class());
	      //               const MetaClass* currentMetaClass = it->current_meta_class();
	      //               const Address curAddr = it->current();
               if (!obj.is_null()) {
                  indent(output, _indent+istep);
                  print2(output, obj, it->current_name(),
                         _indent+istep, field_deref);
                }
	       //it->next();
	    }
          //}
	  //          } while (it->is_valid());
          indent(output, _indent);
          //output << "}\n";
   }
   else
          output <<" {0x0}\n";
   delete it;
   return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool
SuifPrinterModule::print_catchall(ostream& output, 
				  const ObjectWrapper &obj,
				  const LString &name, 
				  int _indent,int deref = 0)
{
  const Address what = obj.get_address();
  const MetaClass* type = obj.get_meta_class(); 
   int field_deref = deref?deref-1:0;
   output << type->get_instance_name()
              << '(' << type->get_meta_class_id() << ") ";
   if (name)
         output << ' ' << name << ' ';
   output << " (un-implemented) ";
   Iterator* it = type->get_iterator(what);
   if (it) {
     if (it->current()) {
       //output << "{\n";
       output << endl;
       //while (it->is_valid()) {
       for (; it->is_valid(); it->next())
	 {
	   ObjectWrapper obj(it->current(), it->current_meta_class());
	   //		const MetaClass* currentMetaClass = it->current_meta_class();
	   //                 const Address curAddr = it->current();
	   if(!obj.is_null()) {
	     //                 if (curAddr) {
	     indent(output, _indent+istep);
	     print2(output, obj, it->current_name(), 
		    _indent+istep, field_deref);
	   }
	   //		it->next();
	   //}
	 }// while (it->is_valid());
       indent(output, _indent);
       //output << "}\n";
     }
         else
            output <<" {0x0}\n";
   }
   if (it) delete it;
   return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/*
bool
SuifPrinterModule::print2(ostream& output, const ObjectWrapper &obj,
			  const LString &name, int _indent, int deref)
{
  return(print2(output, obj.get_address(), obj.get_meta_class(),
		name, _indent, deref));
}
*/
bool
//SuifPrinterModule::print2(ostream& output, const Address what, const MetaClass* type,
//       //const LString &name = emptyLString, int _indent = 2, int deref = 0)
//       const LString &name, int _indent, int deref)
SuifPrinterModule::print2(ostream& output, const ObjectWrapper &obj,
			  const LString &name, int _indent, int deref)
{
  if (!start_of_object(output,obj,deref)) {
      return false;
      }
  //  MetaClassId id = type->get_meta_class_id();
  String str = get_print_string(obj.get_meta_class()->get_instance_name());

  // This is NOT always a suifobject.  It is only
  // a suifobject if the metaclass is a child of the SuifObject metaclass.
  SuifObject *o = NULL;
  if (is_kind_of_suif_object_meta_class(obj.get_meta_class())) {
    o = (SuifObject *) obj.get_address(); 
    }

  // length is at least 1 (for the \0 at the end)
  //output << "p2:deref = " << deref << endl;
  if (use_print_string() && str != emptyString) {
     bool b = parse_and_print(output, obj, name, str, _indent, deref);
     end_of_object(output,obj);
     return b;
     }

  // No print string registered.
  const MetaClass *type = obj.get_meta_class();
  bool b = false;
  if (type->is_elementary()) {
     b =  print_elementary(output, obj, name, _indent, deref);
     }
  else if (type->isKindOf(AggregateMetaClass::get_class_name())) {
     b = print_aggregate(output, 
			      AggregateWrapper(obj),
			      name, _indent, deref);
     }  
  else if (type->isKindOf(PointerMetaClass::get_class_name())) {
     b = print_pointer(output, 
			    PointerWrapper(obj),
			    name, _indent, deref);
     }  
  else if (type->isKindOf(ListMetaClass::get_class_name())) {
     b = print_list(output, obj, name, _indent, deref);
     }  
  else if (type->isKindOf(STLMetaClass::get_class_name())) {
     b = print_stl(output, obj, name, _indent, deref);
     }  
  else {
     b = print_catchall(output, obj, name, _indent, deref);
     }
  end_of_object(output,obj);
  return b;
  }
//----------------------------------------------------------------------------
// This function here for reference only. print2 will become print.
//----------------------------------------------------------------------------

void
SuifPrinterModule::print(ostream& output, const Address what,
			 const MetaClass* type, int _indent )
{
  print2(output, ObjectWrapper(what, type), emptyLString, _indent, 0);
  output <<endl;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/*
void
SuifPrinterModule::print(ostream& output, const Address what)
{
   const SuifObject *rootNode = (SuifObject*)what;
   print(output, rootNode);
   const MetaClass *type = rootNode->get_meta_class();
   int _indent = 2;

   print2(output, Address(rootNode), type, emptyLString, _indent);
   output <<endl;
}
*/
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void SuifPrinterModule::print(const SuifObject*rootNode)
{ print(get_default_stream(), rootNode); }
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void SuifPrinterModule::print(ostream& output, const SuifObject*rootNode)
{
  suif_assert(rootNode != NULL);
  //const MetaClass *type = rootNode->get_meta_class();
   int _indent = 2;

   print2(output, ObjectWrapper(rootNode), emptyLString, _indent);
   output <<endl;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void SuifPrinterModule::print(const Address what, const MetaClass* type)
{
  print2(get_default_stream(), ObjectWrapper(what, type));
  get_default_stream() <<endl;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void
SuifPrinterModule::print(ostream& output)
{
  const FileSetBlock *f = _suif_env->get_file_set_block();
  if (f) {
    print2(output, ObjectWrapper(f), emptyLString);
    output<<endl;
  }
  else
    output << "Suifprinter: Unable to get RootNode FileSetBlock - NULL\n";
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void SuifPrinterModule::print() { print(get_default_stream()); }
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool SuifPrinterModule::start_of_object(
		ostream& output, 
		const ObjectWrapper &obj,
		int deref) {
    return (deref <= 20);
    }

void SuifPrinterModule::end_of_object(ostream& output, const ObjectWrapper &obj) {
   }

