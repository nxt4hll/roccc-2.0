#include "common/system_specific.h"
#include "common/suif_list.h"
#include "module.h"
#include "command_line_parsing.h"
//#include "error_macros.h"
#include "suif_env.h"
#include "error_subsystem.h"
#include "token_stream.h"
#include "common/suif_hash_map.h"

#include "iokernel/object_stream.h" // Need this for the hash(Lstring)

//#include <iostream.h> //@@@
#include <iostream>
using namespace std;

Module::Module( SuifEnv* suif_env, const LString& module_name ) :
   _suif_env( suif_env ),
   _module_name( module_name ),
   _command_name( 0 ),
   _command_line( 0 ),
   _interfaces( new suif_hash_map<LString,Address> )
{
}


Module::~Module() {
  delete _command_line;
  delete _interfaces;
}


bool Module::parse_command_line( TokenStream* command_line_stream ) {
  suif_assert_message( _command_line, ( "Module::parse_command_line called without prior call\n" \
                            "to Module::initialize()." ) );
  _command_line->delete_values();
  Token t;
  String command_name = "";
  if ( command_line_stream -> peek_token( t ) ) {
     command_name = t._token;
  }
  _command_name -> set_argument( command_name );
  bool changed;
  if (!_command_line->parse(command_line_stream, changed)) {
    _command_line->print_to_stream( cerr );
    suif_error( _suif_env, ("Parsing of command line has failed.\n") );
    return false;
  }
  if (!command_line_stream -> is_at_end_of_command()) {
    cerr << "Illegal arguments [";
    while (command_line_stream->get_token(t))
      cerr << t._prefix << t._token << t._separator;
    cerr << "]" << endl;
    _command_line->print_to_stream( cerr );
    return false;
  }
  return true;
}



SuifEnv *Module::get_suif_env() const {
  return(_suif_env);
}

LString Module::get_module_name() const {
  suif_assert_message( _module_name.length() , ( "A module has an empty name" ) );
  return _module_name;
}


bool Module::is_initialized() const {
  return _command_line!=NULL;
}


void Module::execute() {
}


void Module::initialize() {
  _command_line = new OptionList();
  _command_name = new OptionLiteral( get_module_name() );
  _command_line -> add( _command_name );
  // All modules have the --help flag to print their description
  _help_flag = new OptionLiteral( "--help", "Print out command usage" );
  _flags = new OptionSelection( _help_flag , true);
  _command_line->add( new OptionLoop(_flags) );

  bool verbose = false;
  if (verbose) {
    _command_line->print_to_stream(cerr);
  }
}


//Module* Module::clone() {
//  return this;
//}


bool Module::delete_me() const {
  return false;
}

bool Module::supports_interface( const LString &interface_name ) const {
  if (!_interfaces) { return false; }
  suif_hash_map<LString,Address>::iterator iter =
    _interfaces->find(interface_name);
  return(iter != _interfaces->end());
}

// Returns an untyped pointer!!
// Be very careful with it.
Address Module::get_interface( const LString &interface_name ) const {
  assert(_interfaces);
  suif_hash_map<LString,Address>::iterator iter =
    _interfaces->find(interface_name);
  suif_assert(iter != _interfaces->end());
  return((*iter).second);
}

void Module::get_supported_interface_list(list<LString> &llist) const {
  for (suif_hash_map<LString,Address>::iterator iter =
	 _interfaces->begin();
       iter != _interfaces->end(); iter++) {
    LString interface = (*iter).first;
    llist.push_back(interface);
  }
}

void Module::set_interface( const LString &interface_name,
			    Address interface) {
  assert(_interfaces);
  _interfaces->enter_value(interface_name, interface);
}

void Module::interface_registered( const LString &module_name,
				   const LString &interface_name) {
  // Do nothing
}
void Module::interface_object_created( Module *module,
				       const LString &interface_name) {
  // Do nothing
}

void Module::interface_object_destructed( Module *module,
					  const LString &interface_name) {
  // Do nothing
}


void Module::import_module(const char* module_name)
{
  LString sname(module_name);
  _suif_env->import_module(sname);
}

String Module::get_description() const {
  if (!_command_line) return(emptyString);
  return(_command_line->get_description());
}

String Module::get_usage() const {
  if (!_command_line) {
    return(String(get_module_name()) + "\n" );
  }
  return(_command_line->to_string());
}
