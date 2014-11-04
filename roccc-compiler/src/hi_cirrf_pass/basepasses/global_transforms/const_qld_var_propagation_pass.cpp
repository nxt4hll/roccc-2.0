// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include <common/suif_copyright.h>

#include <iostream>
#include <iokernel/cast.h>
#include <iokernel/clone_stream.h>
#include <common/i_integer.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>
#include <basicnodes/basic.h>
#include <basicnodes/basic_constants.h>
#include <suifkernel/suifkernel_messages.h>
#include <suifkernel/utilities.h> 
#include <suifkernel/group_walker.h>
#include <utils/cloning_utils.h>
#include <cfenodes/cfe.h>
#include "const_qld_var_propagation_pass.h"
#include "roccc_utils/warning_utils.h"

// THIS CONSTANT PROPAGATION PASS PROPAGATES ONLY const QUALIFIED INTEGER CONSTANTS

/**************************** Declarations ************************************/

suif_map<VariableSymbol*, ValueBlock*> const_qualified_scalars;

class cqvp_load_variable_expression_walker: public SelectiveWalker {
public:
  cqvp_load_variable_expression_walker(SuifEnv *env)
    :SelectiveWalker(env, LoadVariableExpression::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

/**************************** Implementations ************************************/
ConstQualedVarPropagationPass::ConstQualedVarPropagationPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "ConstQualedVarPropagationPass") {}

void ConstQualedVarPropagationPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  OutputInformation("Constant qualified variable propagation pass begins");

  suif_map<VariableSymbol*, ValueBlock*> temp_const_defs;
    if (proc_def){
        DefinitionBlock *proc_def_block = proc_def->get_definition_block();
        for(Iter<VariableDefinition*> iter = proc_def_block->get_variable_definition_iterator();
            iter.is_valid(); iter.next()){
            VariableDefinition *var_def = iter.current();
            VariableSymbol *var_sym = var_def->get_variable_symbol();
            QualifiedType *qualed_var_type = var_sym->get_type();
            if(is_a<IntegerType>(qualed_var_type->get_base_type())){
               bool found = 0;
               for(Iter<LString> iter2 = qualed_var_type->get_qualification_iterator();
                   iter2.is_valid(); iter2.next())
                   if(iter2.current() == LString("const"))
                      found = 1;
               if(found){
                  temp_const_defs.enter_value(var_sym, var_def->get_initialization());
               }
            }
        }
        for(Iter<StoreVariableStatement> iter = object_iterator<StoreVariableStatement>(proc_def->get_body());
            iter.is_valid(); iter.next()){
            StoreVariableStatement *store_var_stmt = &iter.current();
            Expression *store_var_value = store_var_stmt->get_value();
            if(!is_a<IntConstant>(store_var_value))
               continue;
            VariableSymbol *store_var_destination = store_var_stmt->get_destination();
            if(!is_a<IntegerType>(store_var_destination->get_type()->get_base_type()))  
               continue;
            suif_map<VariableSymbol*,ValueBlock*>::iterator iter2 =
                        temp_const_defs.find(to<LoadVariableExpression>(store_var_value)->get_source());
            if(iter2 != temp_const_defs.end())
               const_qualified_scalars.enter_value(store_var_destination, (*iter2).second);
        }
        cqvp_load_variable_expression_walker walker(get_suif_env());
        proc_def->walk(walker);
    }
  OutputInformation("Constant qualified variable propagation pass ends");
}
   
Walker::ApplyStatus cqvp_load_variable_expression_walker::operator () (SuifObject *x) {
	LoadVariableExpression *load_var_expr = to<LoadVariableExpression>(x);
	VariableSymbol *source_sym = load_var_expr->get_source();

        suif_map<VariableSymbol*,ValueBlock*>::iterator iter = const_qualified_scalars.find(source_sym);
        if(iter != const_qualified_scalars.end()){
           ValueBlock *scalar_value_block = to<ValueBlock>((*iter).second);
           if(!is_a<ExpressionValueBlock>(scalar_value_block))
              return Walker::Continue;
           Expression *element_value_expr = to<ExpressionValueBlock>(scalar_value_block)->get_expression();
           if(!is_a<IntConstant>(element_value_expr))
              return Walker::Continue;
           Expression *replacement = to<Expression>(deep_suif_clone(element_value_expr));
           load_var_expr->get_parent()->replace(load_var_expr, replacement);
        }
	
    	return Walker::Continue;
}

