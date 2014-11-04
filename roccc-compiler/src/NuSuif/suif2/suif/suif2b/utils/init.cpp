#include "common/system_specific.h"
#include "suifkernel/suif_env.h"

#include "suifkernel/module_subsystem.h"
#include "fold_table.h"

extern "C" void init_utils(SuifEnv *s) {
  s->require_module("basicnodes");
  s->require_module("suifnodes");
  s->require_module("typebuilder");
  s->require_module("suifcloning");
  ModuleSubSystem *ms = s->get_module_subsystem();
  if (!ms->is_available("constant_folding_table")) {
    ms->register_module(new FoldTable(s));
  }
}
