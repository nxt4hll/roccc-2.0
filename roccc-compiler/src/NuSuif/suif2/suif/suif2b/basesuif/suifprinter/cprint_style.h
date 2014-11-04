#ifndef EXPRESSION_STRING_H
#define EXPRESSION_STRING_H

#include "suifkernel/suifkernel_forwarders.h"
#include "basicnodes/basic_forwarders.h"
#include "suifnodes/suif_forwarders.h"
#include "suifkernel/module.h"

class CPrintStyleModule;

typedef String (*string_gen_fn)(CPrintStyleModule *map, const SuifObject *obj);

/**
	Pretty much the same as the method in CPrintStyleModule, 
	except that it retrieve the module for you.
*/
String to_cstring(const SuifObject *obj);

class CPrintStyleModule : public Module {
  CascadingMap<string_gen_fn> *_print_map;
public:
  CPrintStyleModule(SuifEnv *env, const LString &name = "cprint");
  ~CPrintStyleModule();
  void initialize();
  Module* clone() const;
  static void print_dispatch(Module *module, std::ostream &str, const ObjectWrapper &obj);
  String print_to_string(const SuifObject *obj);

  void set_print_function(const LString &class_name,
			  string_gen_fn fn);
  
  static LString get_class_name();
};


#if 0
class BuildCStringState {
  // VisitorMap *_expression_map;
  //  VisitorMap *_statement_map;
  CascadingMap<string_gen_fn> *_print_map;

  String _str;  // this is the return value from a handle.
public:
  
  BuildCStringState(SuifEnv *s);

  String build_expression_string(Expression *ex);

  String build_statement_string(Statement *st);

  //  String build_source_op_string(const SourceOp &op);
  //  String build_source_op_string(Expression *op);

  String build_variable_symbol_string(VariableSymbol *var);

  String build_procedure_symbol_string(ProcedureSymbol *ps);
  

  // These should probably be protected or private, butthen
  // I'd need to have a bunch of friend classes.
  void set_string(const String &str);
  String get_string() const;

};
#endif

#endif
