#ifndef SUIFPASSES__DRIVERS_H
#define SUIFPASSES__DRIVERS_H

#include "suifkernel/module.h"

// executes a series of Modules whose names
// are passed to it in the command_line_stream
class Driver : public Module {
public:
  Driver( SuifEnv* suif_env );
  virtual ~Driver();

  virtual void initialize();

  virtual Module* clone() const;

  virtual bool parse_command_line( TokenStream* command_line_stream );

  virtual void execute();

  virtual bool delete_me() const;

  static const LString get_class_name();

protected:
  OptionSelection* _arguments;
  OptionLiteral* _f;
  OptionLiteral* _e;
  OptionString* _file;
  OptionList* _f_file;

  OptionString* _istring;
  OptionList* _i_string;

  OptionStream* _stream;
  std::istream* _istream;
  sf_owned TokenStream* _token_stream;
  String _file_name;
  String _input_string;

  bool _isInteractive;
};


// executes a series of PipelinablePasses whose
// names are passed to it in the command_line_stream
// The PipelinablePasses are executed in a pipelined
// fashion.
class Pipeliner : public Driver {
public:
  Pipeliner( SuifEnv* suif_env );
  virtual ~Pipeliner();

  virtual Module* clone() const;

  virtual void execute();

  static const LString get_class_name();
};


#endif

