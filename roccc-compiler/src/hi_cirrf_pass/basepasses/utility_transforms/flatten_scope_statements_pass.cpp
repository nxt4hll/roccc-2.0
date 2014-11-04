// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include <common/suif_copyright.h>

#include <iostream>
#include <iokernel/cast.h>
#include <iokernel/clone_stream.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>
#include <basicnodes/basic.h>
#include <basicnodes/basic_constants.h>
#include <suifkernel/suifkernel_messages.h>
#include <suifkernel/utilities.h>
#include <suifkernel/group_walker.h>
#include "transforms/procedure_walker_utilities.h"
#include <utils/expression_utils.h>
#include <utils/symbol_utils.h>
#include <utils/type_utils.h>
#include <utils/cloning_utils.h>

#include "roccc_utils/IR_utils.h"
#include "roccc_utils/control_flow_utils.h"

#include "common/suif_list.h"
#include "flatten_scope_statements_pass.h"

/**************************** Declarations ************************************/

class fssp_scope_statement_walker : public SelectiveWalker {
public:
    fssp_scope_statement_walker(SuifEnv *the_env)
        : SelectiveWalker(the_env, ScopeStatement::get_class_name()) {}

    Walker::ApplyStatus operator () (SuifObject *x);
};

/**************************** Implementations ************************************/

FlattenScopeStatementsPass::FlattenScopeStatementsPass( SuifEnv* suif_env) : 
				PipelinablePass( suif_env, "FlattenScopeStatementsPass" ) {}

void FlattenScopeStatementsPass::do_procedure_definition(ProcedureDefinition *proc_def){
    if(proc_def){
        fssp_scope_statement_walker walker(get_suif_env());
        proc_def->walk(walker);
    }
}

Walker::ApplyStatus fssp_scope_statement_walker::operator () (SuifObject *x){
    ScopeStatement *scope_stmt = to<ScopeStatement>(x); 
    
    Statement *scope_stmt_body = scope_stmt->get_body();
    SymbolTable *scope_stmt_sym_table = scope_stmt->get_symbol_table();
    DefinitionBlock *scope_stmt_def_block = scope_stmt->get_definition_block();

    ProcedureDefinition *enclosing_proc_def = get_procedure_definition(scope_stmt);
    SymbolTable *enclosing_proc_def_sym_table = enclosing_proc_def->get_symbol_table();
    DefinitionBlock *enclosing_proc_def_def_block = enclosing_proc_def->get_definition_block();

    while(scope_stmt_sym_table->get_symbol_table_object_count() > 0){
        SymbolTableObject *sym_tab_obj = scope_stmt_sym_table->remove_symbol_table_object(0);
	rename_symbol(sym_tab_obj, "" ); //  String("renamed_") + String(sym_tab_obj->get_name()));
 	enclosing_proc_def_sym_table->append_symbol_table_object(sym_tab_obj);
    }

    while(scope_stmt_def_block->get_variable_definition_count() > 0){
        VariableDefinition *var_def = scope_stmt_def_block->remove_variable_definition(0);
 	enclosing_proc_def_def_block->append_variable_definition(var_def);
    }

    scope_stmt->set_body(0); 
    scope_stmt->get_parent()->replace(scope_stmt, scope_stmt_body);
 
    set_address(scope_stmt_body);
    return Walker::Replaced;
};

