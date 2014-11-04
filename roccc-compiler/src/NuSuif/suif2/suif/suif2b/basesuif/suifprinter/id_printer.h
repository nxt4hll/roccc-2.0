#ifndef _ID_PRINTER_H_
#define _ID_PRINTER_H_

/*
 * This module allows one to call
 *    suif_env->to_string("print_id", obj)
 * which will return a short printable string unique to the object.
 *
 */

#include "suifkernel/suifkernel_forwarders.h"
#include "basicnodes/basic_forwarders.h"
#include "suifnodes/suif_forwarders.h"
#include "suifkernel/module.h"


class IdPrinter : public Module {
public:
  static void print_id(Module *module, std::ostream &str, const ObjectWrapper &obj);

  IdPrinter(SuifEnv *env, const LString &name = "print_id");
  Module* clone() const;
  void initialize();
};


#endif // _ID_PRINTER_H_
