// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef TRANSFORMS__DISMANTLE_IF_H
#define TRANSFORMS__DISMANTLE_IF_H
/* file "dismantle_if.h" */

/*
       Copyright (c) 1998 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>

#include "suifpasses/suifpasses.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/suif_env.h"

#include "suifnodes/suif.h"
#include "procedure_walker_utilities.h"
#include <cfenodes/cfe_forwarders.h>

class if_statement_walker : public ProcedureWalker {
    public:
        if_statement_walker(SuifEnv *the_env,ProcedureDefinition *def);
        static Statement* dismantle_if_statement(IfStatement*);
        Walker::ApplyStatus operator () (SuifObject *x);
    };

class while_statement_walker : public ProcedureWalker{
    public:
        while_statement_walker(SuifEnv *the_env,ProcedureDefinition *def);
        static Statement* dismantle_while_statement(WhileStatement*);
        Walker::ApplyStatus operator () (SuifObject *x);
    };

class do_while_statement_walker : public ProcedureWalker{
    public:
        do_while_statement_walker(SuifEnv *the_env,ProcedureDefinition *def);
        static Statement* dismantle_do_while_statement(DoWhileStatement*);
        Walker::ApplyStatus operator () (SuifObject *x);
    };

class c_for_statement_walker : public ProcedureWalker{
    public:
        c_for_statement_walker(SuifEnv *the_env,ProcedureDefinition *def);
        static Statement* dismantle_c_for_statement(CForStatement*);
        Walker::ApplyStatus operator () (SuifObject *x);
    };

class for_statement_walker : public ProcedureWalker{
    public:
        for_statement_walker(SuifEnv *the_env,ProcedureDefinition *def);
        static Statement* dismantle_for_statement(ForStatement*);
        Walker::ApplyStatus operator () (SuifObject *x);
    };

class scope_statement_walker : public ProcedureWalker{
    public:
        scope_statement_walker(SuifEnv *the_env,ProcedureDefinition *def);
        static Statement* dismantle_scope_statement(ScopeStatement*);
        Walker::ApplyStatus operator () (SuifObject *x);
    protected:
        bool dismantle_only_when_needed;
    };

class multi_way_branch_statement_walker : public ProcedureWalker{
    public:
        multi_way_branch_statement_walker(SuifEnv *the_env,ProcedureDefinition *def);
        static Statement* dismantle_multi_way_branch_statement(MultiWayBranchStatement*);
        Walker::ApplyStatus operator () (SuifObject *x);
    };

//	Unlike the previous multi way branch statement walker, this walker
//	does not disamntle them up into ifs but rather divides them up into
//	contiguous subranges

class multi_way_branch_statement_compactor : public ProcedureWalker{
    public:
        multi_way_branch_statement_compactor(SuifEnv *the_env,ProcedureDefinition *def);
        Walker::ApplyStatus operator () (SuifObject *x);
    };


class DismantleEmptyScopeStatements : public PipelinablePass {
public:
  DismantleEmptyScopeStatements(SuifEnv *the_env,
				const LString &name = 
				"dismantle_empty_scope_statements");
  Module *clone() const { return (Module *)this; }
  void initialize();
  void do_procedure_definition(ProcedureDefinition *pd);
};

/**
   class MarkGuardedFors
   A loop is guarded if it executes at least once.
   Mark a ForStatement as guarded if lb TEST UB is true.
   This only really works when they are constants.
*/
class MarkGuardedFors : public PipelinablePass {
public:
  MarkGuardedFors(SuifEnv *the_env,
		  const LString &name = 
		  "mark_guarded_fors");
  Module *clone() const { return (Module *)this; }
  void initialize();
  void do_procedure_definition(ProcedureDefinition *pd);
};

/**
   class GuardAllFors
   Make all For statements execute at least once
   by guarding all the Fors with a test if necessary.
*/
class GuardAllFors : public PipelinablePass {
public:
  GuardAllFors(SuifEnv *the_env,
		  const LString &name = 
		  "guard_all_fors");
  Module *clone() const { return (Module *)this; }
  void initialize();
  void do_procedure_definition(ProcedureDefinition *pd);
};

class DismantleCallArguments : public PipelinablePass {
public:
  DismantleCallArguments(SuifEnv *the_env,
			 const LString &name = 
			 "dismantle_call_arguments");
  Module *clone() const { return (Module *)this; }
  void initialize();
  void do_procedure_definition(ProcedureDefinition *pd);
};


#endif /* TRANSFORMS__DISMANTLE_IF_H */
