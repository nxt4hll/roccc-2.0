#include "common/system_specific.h"
// #ifdef PGI_BUILD
// #include <iostream.h>
// #endif
#include "drivers.h"

#include "suifkernel/token_stream.h"
#include "suifkernel/command_line_parsing.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/error_subsystem.h"
//#include "suifkernel/error_macros.h"

#include "passes.h"
#include "dispatcher.h"
#include "common/suif_vector.h"

// #ifndef PGI_BUILD
// #include <iostream.h>
// #endif
// #include <fstream.h>
// #ifndef MSVC
// #include <strstream.h>
// #else
// #include <strstrea.h>
// #endif

#include <iostream>//jul modif
#include <fstream>//jul modif
#include <sstream>//jul modif
using namespace std;//jul modif

#include "suifkernel/char_source.h"
#include "suifkernel/suif_exception.h"


const LString Driver::get_class_name() {
  static LString name( "execute" );
  return name;
}


Driver::Driver( SuifEnv* suif_env ) :
  Module( suif_env ),
  _arguments( 0 ),
  _f( 0 ),
  _file( 0 ),
  _f_file( 0 ),
  _stream( 0 ),
  _istream( 0 ),
  _token_stream( 0 ),
  _isInteractive(false)
{
  // override an inherited instance variable
  _module_name = Driver::get_class_name();
}


Driver::~Driver() {
  delete _token_stream;
}

void Driver::initialize() {
  Module::initialize();
  _command_line -> set_description(
       "This module executes a series of modules which are read in either\n"
       "from a file, from the command line, or from standard input." );
  _f =  new OptionLiteral( "-f" );
  _file = new OptionString( "file-name", &_file_name );
  _e =  new OptionLiteral( "-e" );
  _istring = new OptionString( "input-string", &_input_string );
  _istring->set_description( "A semicolon separated list of modules and\n"
			     "their command line options\n");
  _stream = new OptionStream( "input-stream", &_istream  );
  _stream -> set_description( "A list of modules and their command line options\n"
                              "surrounded by curly braces.\n" );
  _f_file = (new OptionList())->add( _f )->add( _file );
  _f_file -> set_description( "A file name that contains the input." );
  _i_string = (new OptionList())->add(_e)->add(_istring);
  _i_string -> set_description( "A string of semicolon separated modules" );

  OptionLiteral *intOpt = new OptionLiteral("-i", &_isInteractive, true);
  intOpt->set_description("Interactive mode, read commands from stdin");
  OptionString *intStr = new OptionString("init-string", &_input_string);
  intStr->set_description("A list of commands surrounded by curly braces.\n");
  OptionList *intLst = (new OptionList())->add(intOpt)->add(intStr);

  _arguments = (new OptionSelection(true))->
    add( _f_file )->
    add(_i_string)->
    add( _stream )->
    add(intLst);
  _command_line -> add( _arguments );
}

bool Driver::parse_command_line( TokenStream* command_line_stream ) {
  bool has_succeeded = Module::parse_command_line( command_line_stream );
  if ( !has_succeeded ) {			// illegal command options
    return false;
  } else if (_istream != NULL) {		// {...}
    _token_stream = new TokenStream(new IstreamCharSource(_istream));
    return true;
  } else if ( !_file_name.is_empty() ) {	// -f
    ifstream *fs = new ifstream(_file_name);
    suif_assert_message(!fs->bad(),
			("Could not open file: %s\n", _file_name.c_str()));
    _token_stream = new TokenStream(new IstreamCharSource(fs));
    return true;
  } else if ( !_input_string.is_empty() ) {	// -e
    //#ifndef MSVC
    istringstream *sstr = new istringstream(_input_string.c_str());//jul modif
// #else
// 	istringstream *sstr = new istringstream((char*)_input_string.c_str());
// #endif
    suif_assert_message(!sstr->bad(),
			("Could not parse [%s]\n", _input_string.c_str()));
    _token_stream = new TokenStream(new IstreamCharSource(sstr));
    return true;
  } else if (_isInteractive) {			// -i
    return true;
  } else {					// empty options
    _token_stream = new TokenStream(new StdCharSource());
    return true;
  }
}


Module* Driver::clone() const {
  return new Driver( _suif_env );
}

bool Driver::delete_me() const {
  return true;
}

static void do_it(TokenStream *tokstr, SuifEnv* senv,
		  bool isInteractive)
{
  ModuleSubSystem* mSubSystem = senv->get_module_subsystem();
  Token module_name;
  while (tokstr->peek_token(module_name)) {
    if (module_name.is_equal("#")) {
      // line starts with #, skip till EOL
      while (tokstr->get_token(module_name) &&
	     !module_name.is_equal("\n"))
	;
      continue;
    } else if (module_name.is_equal("\n") ||
	       module_name.is_equal(";")) {
      // skip empty line
      tokstr->get_token(module_name);
      continue;
    } else {
      // module_name has a valid module name
      // _token_stream->peek_token() has the module_name
      //
      try {
	if (!mSubSystem->execute(module_name.get_name(), tokstr))
	  break;
      } catch (SuifDevException& dexp) {
	cerr << dexp.get_message() << endl;
	cerr << module_name.get_name() << " gave an unrecoverable exception."
	     << endl;
	// pull off the last token and continue
	tokstr->get_token(module_name);
	if (isInteractive) {
	  cerr << "Skip to EOL." << endl;
	  for (Token t; tokstr->get_token(t); )
	    if (t._token == String("\n")) break;
	} else
	  throw;
      } catch (SuifException& exp) {
	cerr << exp.get_message() << endl;
	cerr << module_name.get_name()
	     << " gave an exception.  Your IR may be inconsistent." << endl;
	// pull off the last token and continue
	tokstr->get_token(module_name);
	if (isInteractive) {
	  cerr << "Skip to EOL." << endl;
	  for (Token t; tokstr->get_token(t); )
	    if (t._token == String("\n")) break;
	}
      }
    }
  }
}

void Driver::execute() {
  if (_token_stream != NULL)
    do_it(_token_stream, _suif_env, false);
  if (_isInteractive) {
    StdCharSource csrc("suif> ");
    TokenStream tokstr(&csrc);
    do_it(&tokstr, _suif_env, true);
  }
}


const LString Pipeliner::get_class_name() {
  static LString name( "pipeline" );
  return name;
}


Pipeliner::Pipeliner( SuifEnv* suif_env ) :
  Driver( suif_env ) {
  _module_name = Pipeliner::get_class_name();
}


Pipeliner::~Pipeliner() {
}

Module* Pipeliner::clone() const {
  return new Pipeliner( _suif_env );
}


void Pipeliner::execute() {
  suif_vector<PipelinablePass*> pipelined_modules;
  ModuleSubSystem* mSubSystem = _suif_env->get_module_subsystem();
  Token module_name;
  bool has_succeeded = true;
  while ( has_succeeded ) {
    while ( _token_stream->is_at_end_of_command() &&
	    _token_stream->get_token(module_name))
      ;
    has_succeeded = !_token_stream->is_empty();
    if ( !has_succeeded ) break;
    has_succeeded = _token_stream->peek_token( module_name );
    if ( !has_succeeded ) break;
    Module* module =  mSubSystem->parse_command_line_and_clone( module_name._token, _token_stream );
    if ( module ) {
      pipelined_modules.push_back( (PipelinablePass*)module );
    } else {
      suif_error( _suif_env, "Couldn't find module: %s\n", module_name._token.c_str() );
    }
  }
  PipelinerDispatchPass dispatch_pass( _suif_env, &pipelined_modules );
  dispatch_pass.execute();
}








