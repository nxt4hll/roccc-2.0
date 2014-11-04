// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef TRANSFORMS__PROCEDURE_WALKER_UTILITIES
#define TRANSFORMS__PROCEDURE_WALKER_UTILITIES

//	This class defines a Walker which contains a number of utilities for procedure
//	based walkers. There are a number of routines which build various kinds of
//	objects. These would probably be better placed somewhere else, but that
//	somewhere else does not currently exist

#include "suifkernel/group_walker.h"
#include "basicnodes/basic_forwarders.h"
#include "suifnodes/suif_forwarders.h"

class ProcedureWalker : public SelectiveWalker {
	ProcedureDefinition *proc_def;
    public:
        ProcedureWalker(SuifEnv *the_env,ProcedureDefinition *def,const LString &type);

	CodeLabelSymbol *create_new_label();

    // static version of the above
    static CodeLabelSymbol *create_new_label(ProcedureDefinition* proc_def);

	ProcedureDefinition *get_proc_def();

	DefinitionBlock *get_definition_block();

	SymbolTable *get_symbol_table();
	};

#endif
