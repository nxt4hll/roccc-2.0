#include "common/system_specific.h"
#include "standard_modules.h"

#include "suifkernel/suif_env.h"
#include "suifkernel/dll_subsystem.h"
#include "suifkernel/suif_object.h"
#include "suifkernel/command_line_parsing.h"
#include "suifkernel/print_subsystem.h"
#include "suifkernel/module_subsystem.h"

#include "common/formatted.h"
#include "common/suif_vector.h"
#include "common/suif_list.h"
#include "iokernel/object_wrapper.h"


// #include <iostream.h>
// #include <fstream.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

const LString PrintModule::get_class_name() {
  static LString name( "print" );
  return name;
}



PrintModule::PrintModule( SuifEnv * suif_env ) : Module( suif_env ) {
  _module_name = PrintModule::get_class_name();
}


void PrintModule::initialize() {
  Module::initialize();
  _is_raw = new OptionLiteral("-raw");
  _do_list = new OptionLiteral("-list");
  _command_line->set_description("Use the default PrintSubSystem to print the default file_set_block");

  Option *libstyle = 
    build_prefixed_string( "-style", "stylename", 
			   &_style, 
			   "Choose a Module to implement the print. e.g. \"print_suif\"" );
  Option *filename = 
    build_prefixed_string( "-o", "filename", 
			   &_filename, 
			   "Output the result to the filename" );
  OptionSelection *sel = new OptionSelection();
  sel->add(_is_raw)->add(_do_list)->add(libstyle)->add(filename);
  OptionLoop *opt = new OptionLoop(sel, true);
  _command_line->add(opt);
  //recursive.. ooops.
  set_interface("print", (Address)PrintModule::print_dispatch);
}


void PrintModule::print_dispatch(Module *mod, ostream &output, 
				 const ObjectWrapper &obj) {
  PrintModule *me = (PrintModule *)mod;
  SuifEnv *s = mod->get_suif_env();
  PrintSubSystem *psub = s->get_print_subsystem();
  if (psub)
    {
      LString style;
      if (!me->_is_raw->is_set())
	{
	  if (me->_style == emptyString)
	    style = psub->get_default_print_style();
	  else
	    style = me->_style;
	}
      if (style != me->get_module_name())
	{
	  psub->print(style, output, obj);
	  return;
	}
    }

  // Bummer, have to print raw.
  // because there is no print subsystem
  // or the default print style is ME
  //
  if (!is_kind_of_suif_object_meta_class(obj.get_meta_class()))
    {
      suif_warning("No raw print defined on a non-suifobject. Aborting\n");
      return;
    }
	
  FormattedText text;
  ( (SuifObject*)s->get_file_set_block()) -> print(text);
  output << text.get_value() << endl;
}

 
void PrintModule::execute() {

  if (_do_list->is_set()) {
    cout << "List of print styles available: " << endl;
    _suif_env->get_module_subsystem()->print_modules(cout, ", ",
						     "print");
    return;
  }
  
  SuifObject *fsb = (SuifObject*)_suif_env->get_file_set_block();
  if (fsb == NULL) return;

  if (_filename != emptyString)
    {
      ofstream outputFile( _filename.c_str(), ios::out );
      if (outputFile.bad()) 
	{
	  suif_warning("Could not open %s. Aborting\n",
		       _filename.c_str());
	  // delete outputFile;
	  return;
	}
      print_dispatch(this, outputFile, ObjectWrapper(fsb));
      return;
    }
  print_dispatch(this, cout, ObjectWrapper(fsb));
}

Module *PrintModule::clone() const {
  return(Module *)this;
}

const LString SaveModule::get_class_name() {
  static LString name( "save" );
  return name;
}


SaveModule::SaveModule( SuifEnv* suif ) :
       Module( suif ),
       _file_name_argument( 0 )  {
  _module_name = SaveModule::get_class_name();
}


SaveModule::~SaveModule() {
//  delete _file_name_argument;
}

void SaveModule::initialize() {
  Module::initialize();
  _file_name_argument = new OptionString( "file-name" );
  _command_line -> add( _file_name_argument );
  _command_line-> set_description(
    "Saves a SUIF tree to a file." );
}


void SaveModule::execute() {
  String file_name = _file_name_argument->get_string()->get_string();
  _suif_env->write( file_name );
}

Module *SaveModule::clone() const {
  return(Module *)this;
}


const LString ImportModule::get_class_name() {
  static LString name( "import" );
  return name;
}


ImportModule::ImportModule( SuifEnv* suif ) :
      Module( suif ),
      _module_name_argument( 0 ) {
  _module_name = ImportModule::get_class_name();
}

ImportModule::~ImportModule() {
//  delete _module_name_argument;
}


void ImportModule::initialize() {
  Module::initialize();
  _module_name_argument = new OptionString( "module name" );
  _repetition  = new OptionLoop( _module_name_argument );
  _command_line -> add( _repetition );
  _command_line-> set_description(
    "Imports a number of modules." );
}


void ImportModule::execute() {
  int module_count = _module_name_argument->get_number_of_values();
  for ( int i = 0; i < module_count ; i++ ) {
    String module_name = _module_name_argument->get_string( i )->get_string();
    _suif_env->get_dll_subsystem()->load_and_initialize_DLL( module_name );
  }
}

Module *ImportModule::clone() const {
  return(Module *)this;
}

const LString RequireModule::get_class_name() {
  static LString name( "require" );
  return name;
}


RequireModule::RequireModule( SuifEnv* suif ) :
      Module( suif ),
      _module_name_argument( 0 ) {
  _module_name = RequireModule::get_class_name();
}

RequireModule::~RequireModule() {
//  delete _module_name_argument;
}


void RequireModule::initialize() {
  Module::initialize();
  _module_name_argument = new OptionString( "module name" );
  _repetition  = new OptionLoop( _module_name_argument );
  _command_line -> add( _repetition );
  _command_line-> set_description(
    "Requires a number of modules." );
}


void RequireModule::execute() {
  int module_count = _module_name_argument->get_number_of_values();
  for ( int i = 0; i < module_count ; i++ ) {
    String module_name = _module_name_argument->get_string( i )->get_string();
    _suif_env->get_dll_subsystem()->require_DLL( module_name );
  }
}

Module *RequireModule::clone() const {
  return(Module *)this;
}


const LString XLoadModule::get_class_name() {
  static LString name( "load" );
  return name;

}


XLoadModule::XLoadModule( SuifEnv* suif ) :
    Module( suif ),
    _file_name_argument( 0 ) {
  _module_name = XLoadModule::get_class_name();
}

XLoadModule::~XLoadModule() {
//  delete _file_name_argument;
}


void XLoadModule::initialize() {
  Module::initialize();
  _file_name_argument = new OptionString( "file-name" );
  _command_line-> add( _file_name_argument );
  _command_line-> set_description(
    "Loads a SUIF tree from a file." );
}


void XLoadModule::execute() {
  String file_name = _file_name_argument -> get_string() -> get_string();
  _suif_env->read( file_name );
}

Module *XLoadModule::clone() const {
  return(Module *)this;
}


ListModulesModule::ListModulesModule( SuifEnv* suif, const LString &name ) :
    Module( suif, name )
{
}

ListModulesModule::~ListModulesModule() {
}


void ListModulesModule::initialize() {
  Module::initialize();
  _command_line-> set_description("List all modules in the suif environment");
  _flags->add(build_prefixed_string( "-interface", "interface", 
				     &_interface, 
				     "Only list modules that conform to the specified interface. e.g. \"Pass\" \"FrontendPass\", \"print\"" ));

  /*
  _file_name_argument = new OptionString( "file-name" );
  _command_line-> add( _file_name_argument );
  _command_line-> set_description(
    "Loads a SUIF tree from a file." );
  */
}


void ListModulesModule::execute() {
  ModuleSubSystem *ms = _suif_env->get_module_subsystem();
  LString interface;
  list<LString> mlist;
  ms->get_module_list(interface, mlist);

  for (list<LString>::iterator iter = mlist.begin();
       iter != mlist.end(); iter++) {
    LString module_name = *iter;
    Module *m = ms->retrieve_module(module_name);
    if (_interface == emptyString 
	|| m->supports_interface(_interface)) {
      cout << "Module: ";
      cout << m->get_module_name().c_str() << ": ";
      // description
      // 
      if (m->get_description() != emptyString) {
	cout << "\n  Description: ";
	cout << m->get_description().c_str() << "\n";
      }
      
      // supported interfaces
      bool is_first = true;
      list<LString> llist;
      m->get_supported_interface_list(llist);
      for (list<LString>::iterator liter = llist.begin();
	   liter != llist.end(); liter++) {
	LString interface = *liter;
	if (is_first) {
	  is_first = false;
	  cout  << "\n  Supports Interfaces: ";
	} else {
	  cout << ", ";
	}
	cout << interface.c_str();
      }
      if (!is_first) {
	cout  << ";\n";
      }
      cout  << "\n";
    }
  }
}

Module *ListModulesModule::clone() const {
  return(Module *)this;
}







ListInterfacesModule::ListInterfacesModule( SuifEnv* suif, const LString &name ) :
    Module( suif, name )
{
}

ListInterfacesModule::~ListInterfacesModule() {
}


void ListInterfacesModule::initialize() {
  Module::initialize();
  _command_line-> set_description("List all modules in the suif environment");
  /*
  _file_name_argument = new OptionString( "file-name" );
  _command_line-> add( _file_name_argument );
  _command_line-> set_description(
    "Loads a SUIF tree from a file." );
  */
}


void ListInterfacesModule::execute() {
  ModuleSubSystem *ms = _suif_env->get_module_subsystem();
  //  LString interface;
  list<LString> ilist;
  ms->get_interface_list(ilist);

  for (list<LString>::iterator iter = ilist.begin();
       iter != ilist.end(); iter++) {
    LString interface_name = *iter;
    cout << "Interface: ";
    cout << interface_name.c_str() << ": ";
    // description
    // 
    String description = ms->get_interface_description(interface_name);
    if (description != emptyString) {
      cout << "\n  Description: ";
      cout << description.c_str() << "\n";
    }
    cout  << "\n";
  }
}

Module *ListInterfacesModule::clone() const {
  return(Module *)this;
}







