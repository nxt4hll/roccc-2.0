
#include "simple_module.h"
#include "suifkernel/token_stream.h"
#include "suifkernel/suif_exception.h"
#include <iostream>

using namespace std;

SimpleModule::SimpleModule(SuifEnv* senv, LString name) :
  Module(senv, name),
  _arguments()
{}

SimpleModule::~SimpleModule(void)
{}

Module* SimpleModule::clone(void) const
{
  return const_cast<SimpleModule*>(this);
}

bool SimpleModule::delete_me(void) const
{
  return false;
}

void SimpleModule::execute(void)
{
  try {
    if ((_arguments.size() == 1) &&
	( _arguments[0] == LString("-help") ||
	  _arguments[0] == LString("-h") ||
	  _arguments[0] == LString("-?")))
      cerr << get_help_string() << endl;
    else
      execute(&_arguments);
  } catch (...) {
    _arguments.clear();
    throw;
  }
  _arguments.clear();
}


String SimpleModule::get_description(void) const
{
  return get_help_string();
}

String SimpleModule::get_help_string(void) const
{
  return "Description not implemented.";
}

bool SimpleModule::parse_command_line( TokenStream* command_line_stream )
{
  _arguments.clear();
  Token tok;
  command_line_stream->get_token(tok);  // remove command name
  while (!command_line_stream->is_at_end_of_command()) {
    if (command_line_stream->get_token(tok))
      _arguments.push_back(tok.get_name());
    else
      break;
  }
  return true;
}

FileSetBlock* SimpleModule::get_file_set_block(void)
{
  return get_suif_env()->get_file_set_block();
}

void SimpleModule::check_file_set_block(void)
{
  if (get_file_set_block() == NULL) {
    SUIF_THROW(SuifException(get_module_name() + " expects a file set block."));
  }
}

void SimpleModule::check_arg_count(suif_vector<LString>* args,
				   unsigned cnt)
{
  if (args->size() != cnt) {
    SUIF_THROW(SuifException(get_module_name() + " expects exactly " +
			     String((long)cnt) + " arguments"));
  }
}
