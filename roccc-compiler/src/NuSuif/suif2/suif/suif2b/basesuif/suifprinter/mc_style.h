#ifndef MC_STYLE_H
#define MC_STYLE_H

#include "suifkernel/suifkernel_forwarders.h"
#include "basicnodes/basic_forwarders.h"
#include "suifnodes/suif_forwarders.h"
#include "suifkernel/module.h"

class MCPrintStyleModule;


/**
	Pretty much the same as the method in CPrintStyleModule, 
	except that it retrieves the module for you.
*/
String to_cstring(const SuifObject *obj);

class MCPrintStyleModule : public Module {
public:
  MCPrintStyleModule(SuifEnv *env, const LString &name = MCPrintStyleModule::get_class_name());
  void initialize();
  Module* clone() const;
  static void print_dispatch(Module *module, std::ostream &str, const ObjectWrapper &obj);
  //  String print_to_string(const SuifObject *obj);
  void print_it(std::ostream &str, const ObjectWrapper &obj, String indent);
  static LString get_class_name();
};

#endif /* MC_STYLE_H */
