// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include "suifkernel/suif_env.h"
#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "utils/symbol_utils.h"

#include "procedure_walker_utilities.h"

ProcedureWalker::ProcedureWalker(SuifEnv *the_env,ProcedureDefinition *def,const LString &type)
	: SelectiveWalker(the_env,type),proc_def(def) {}

CodeLabelSymbol *ProcedureWalker::create_new_label(ProcedureDefinition* proc_def) {
    CodeLabelSymbol *label = new_anonymous_label(proc_def->get_symbol_table());
      return label;
    }


CodeLabelSymbol *ProcedureWalker::create_new_label() {
    CodeLabelSymbol *label = new_anonymous_label(get_symbol_table());
      return label;
    }

ProcedureDefinition *ProcedureWalker::get_proc_def() {
    return proc_def;
    }

DefinitionBlock *ProcedureWalker::get_definition_block() {
    return proc_def->get_definition_block();
    }

SymbolTable *ProcedureWalker::get_symbol_table() {
    return proc_def->get_symbol_table();
    }


