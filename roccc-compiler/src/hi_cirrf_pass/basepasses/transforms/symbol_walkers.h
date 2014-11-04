// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef TRANSFORMS__SYMBOL_WALKERS_H
#define TRANSFORMS__SYMBOL_WALKERS_H

#include <common/suif_copyright.h>

//	Various walkers on symbols 
//
//		1/ Name all symbols
//		2/ Avoid extern collisions
//		3/ Combined pass

#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"
#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

//	The passes in this file are mainly used to massage
//	Suif trees before outputting code as C

//	Name anonymous symbols
class NameAllSymbolsPass : public Pass {
    public:
  	Module* clone() const {return (Module*)this;}

  	void do_file_set_block( FileSetBlock* file_set_block );

	NameAllSymbolsPass(SuifEnv *env, const LString &name =
			   "name_all_symbols");
    };

//	Make sure that local names do not clash with global names so
//	that the globals will not be hidden by C scoping
class AvoidExternCollisions : public Pass {
    public:
        Module* clone() const {return (Module*)this;}

        void do_file_set_block( FileSetBlock* file_set_block );

	AvoidExternCollisions(SuifEnv *env, const LString &name =
			      "avoid_external_collisions");
    };

class AvoidFileScopeCollisions : public Pass {
    public:
        Module* clone() const {return (Module*)this;}

        void do_file_set_block( FileSetBlock* file_set_block );

	AvoidFileScopeCollisions(SuifEnv *env, const LString &name =
				 "avoid_file_scope_collisions");
    };

class AvoidLabelCollisions : public PipelinablePass {
    public:
        Module* clone() const {return (Module*)this;}

        void do_procedure_definition( ProcedureDefinition* proc_def );

	AvoidLabelCollisions(SuifEnv *env, const LString &name =
			     "avoid_label_collisions");
    };

class CombinedPass : public Pass {
    public:
        Module* clone() const {return (Module*)this;}

        void do_file_set_block( FileSetBlock* file_set_block );

	CombinedPass(SuifEnv *env, const LString &name =
		     "rename_colliding_symbols");
    };

class CollisionAvoider : public SelectiveWalker {
        BasicSymbolTable *external_symbol_table;
        list<SymbolTable*> *file_scope_symbol_tables;
        LString source_file_name;
        bool name_all_symbols;
    public:

        CollisionAvoider(SuifEnv *env,
                         BasicSymbolTable *the_external_symbol_table,
                         list<SymbolTable*> *the_file_scope_symbol_tables,
			 LString  source_file_name,
                         bool name_all)
                : SelectiveWalker(env,Symbol::get_class_name()),
                  external_symbol_table(the_external_symbol_table),
                  file_scope_symbol_tables(the_file_scope_symbol_tables),
                  source_file_name(source_file_name),
                  name_all_symbols(name_all) {}

        ApplyStatus operator () (SuifObject *x);
        };

#endif

