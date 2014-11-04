// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef REGION_PASSES_H
#define REGION_PASSES_H

#include <suifkernel/group_walker.h>
#include <suifkernel/token_stream.h>
#include <transforms/statement_dismantlers.h>
#include <utils/symbol_utils.h>
#include <suifnodes/suif_factory.h>
#include <suifkernel/utilities.h>
#include <basicnodes/basic_constants.h>
#include <utils/expression_utils.h>
#include <cfenodes/cfe.h>

/**
    Dismantles if statements and do & while loops. 
    
    CFors are dismantled and for loops are preserved.
*/
class RemoveIfAndLoopPass : public PipelinablePass {
public:
    RemoveIfAndLoopPass::RemoveIfAndLoopPass(SuifEnv *the_env,
					     const LString &name =
					     "dismantle_ifs_and_loops") :
      PipelinablePass(the_env, name){};

    Module *clone() const { return (Module *)this;}

    void initialize();

    void do_procedure_definition(ProcedureDefinition *proc_def);

protected:
    OptionLiteral *_preserve_ifs;
};

/**
    FlattenStatementListsPass - this pass gets rid of 
    statement lists inside of other statement lists and 
    flattens them.
*/
class FlattenStatementListsPass : public PipelinablePass {
public:
    FlattenStatementListsPass::FlattenStatementListsPass(SuifEnv *the_env,
							 const LString &name =
							 "flatten_statement_lists") :
      PipelinablePass(the_env, name){};

    Module *clone() const { return (Module *)this; }
    void initialize();
    void do_procedure_definition(ProcedureDefinition *proc_def);
};

/**
    A pass to add return labels at the end of each procedure.
*/
class AddProcedureEndLabelsPass : public PipelinablePass {
public:
    AddProcedureEndLabelsPass::AddProcedureEndLabelsPass(SuifEnv *the_env,
							 const LString &name =
							 "insert_procedure_end_labels") :
      PipelinablePass(the_env, name){};

    Module *clone() const { return (Module *)this;}
    void initialize();
    void do_procedure_definition(ProcedureDefinition *proc_def);
};

/**
    A pass to convert C for's to SUIF for's.
*/
class CFor2ForPass : public PipelinablePass {
public:
    CFor2ForPass::CFor2ForPass(SuifEnv *the_env) :
        PipelinablePass(the_env, "dismantle_cfors_to_fors"){};

    void initialize();

    Module *clone() const { return (Module *)this;}

    void do_procedure_definition(ProcedureDefinition *proc_def);

    static bool convert_cfor2for(CForStatement* cfor,
				 bool verbose);

    virtual void finalize();

protected:
    static unsigned int conversion_count;
    OptionLiteral *_verbose;
};

/**
    A pass to dismantle structural statements that have jumps 
    going inside of them.
*/
class DismantleStmtsWithJumpsInside : public PipelinablePass {
public:
    DismantleStmtsWithJumpsInside(SuifEnv *the_env) :
        PipelinablePass(the_env, "dismantle_stmts_with_jumps_inside"){};

    /*template <class T>
    void eliminate_jumps_into(ExecutionObject* obj);*/

    static void dismantle_structural_statement(Statement* stmt);

    Module *clone() const { return (Module *)this;}
    void initialize();
    void do_procedure_definition(ProcedureDefinition *proc_def);
};

/**
    A pass to dismantle structural statements that have jumps 
    going inside of them.
*/
class DismantleMultiEntryScopeStatements : public PipelinablePass {
public:
    DismantleMultiEntryScopeStatements(SuifEnv *the_env) :
        PipelinablePass(the_env, "dismantle_multi_entry_scope_statements"){};

    Module *clone() const { return (Module *)this;}
    void initialize();
    void do_procedure_definition(ProcedureDefinition *proc_def);
};

/**
    Pass to put StatementLists inside of procedures that just 
    have bare statements as their body
*/
class AddStatementListsToProcs : public PipelinablePass {
public:
    AddStatementListsToProcs(SuifEnv *the_env) :
        PipelinablePass(the_env, "add_statement_lists_to_procs"){};

    Module *clone() const { return (Module *)this;}
    void initialize();
    void do_procedure_definition(ProcedureDefinition *proc_def);
};

/* 
    A pass to add continues and break labels as
    explicit LabelLocationStatement if they are 
    not there yet.
*/
class AddExplicitLoopLabels: public PipelinablePass {
public:
    AddExplicitLoopLabels(SuifEnv *the_env) :
        PipelinablePass(the_env, "add_explicit_loop_labels"){};

    Module *clone() const { return (Module *)this;}
    void initialize();
    void do_procedure_definition(ProcedureDefinition *proc_def);
};

/**
   A pass to fix up all the explicit labels in the different 
   kinds of loops.
*/
class FixupExplicitLoopLabels: public PipelinablePass {
public:
    FixupExplicitLoopLabels(SuifEnv *the_env) :
        PipelinablePass(the_env, "fixup_explicit_loop_labels"){};

    Module *clone() const { return (Module *)this;}
    void initialize();
    void do_procedure_definition(ProcedureDefinition *proc_def);
};

/**
    A pass to remove continues and break labels as
    explicit LabelLocationStatement that have the same
    labels as continues/breaks of loops.
*/
class RemoveExplicitLoopLabels: public PipelinablePass {
public:
    RemoveExplicitLoopLabels(SuifEnv *the_env) :
        PipelinablePass(the_env, "remove_explicit_loop_labels"){};

    Module *clone() const { return (Module *)this;}
    void initialize();
    void do_procedure_definition(ProcedureDefinition *proc_def);
};

/**
    A pass to convert if conditions that are not 
	BinaryExpressions by explicitly comparing them with 0.
*/
class IfConditionsToBinaryExprs : public PipelinablePass {
public:
    IfConditionsToBinaryExprs(SuifEnv *the_env) :
        PipelinablePass(the_env, "if_conditions_to_binary_exprs"){};

    Module *clone() const { return (Module *)this;}

	void initialize();

    void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif /* REGION_PASSES_H */
