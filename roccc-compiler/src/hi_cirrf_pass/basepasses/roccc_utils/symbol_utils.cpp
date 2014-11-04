// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include <common/suif_copyright.h>
#include <common/suif_list.h>

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
#include "common/suif_map.h"
#include "transforms/procedure_walker_utilities.h"
#include <utils/expression_utils.h>
#include <utils/symbol_utils.h>
#include <utils/type_utils.h>
#include <utils/cloning_utils.h>
#include <cfenodes/cfe.h>
#include "list_utils.h"
#include "symbol_utils.h"

/**************************** Declerations ************************************/


/************************** Implementations ***********************************/

list<VariableSymbol*>* collect_variable_symbols(ExecutionObject *exec_obj){
     
    list<VariableSymbol*> *variable_symbols = new list<VariableSymbol*>;
        
    for (Iter<LoadVariableExpression> iter = object_iterator<LoadVariableExpression>(exec_obj);
         iter.is_valid(); iter.next()){
         VariableSymbol *var_sym = (&iter.current())->get_source();
         if (!is_in_list(var_sym, variable_symbols))
             variable_symbols->push_back(var_sym);
    }
           
    for (Iter<StoreVariableStatement> iter = object_iterator<StoreVariableStatement>(exec_obj);
         iter.is_valid(); iter.next()){
         VariableSymbol *var_sym = (&iter.current())->get_destination();
         if (!is_in_list(var_sym, variable_symbols))
             variable_symbols->push_back(var_sym);
    }
   
    return variable_symbols;
}  

list<VariableSymbol*>* collect_defined_symbols(ExecutionObject *exec_obj){
     
    list<VariableSymbol*> *defined_symbols = new list<VariableSymbol*>;
        
    for (Iter<StoreVariableStatement> iter = object_iterator<StoreVariableStatement>(exec_obj);
         iter.is_valid(); iter.next()){
         VariableSymbol *var_sym = (&iter.current())->get_destination();
         if (!is_in_list(var_sym, defined_symbols))
             defined_symbols->push_back(var_sym);
    }
   
    return defined_symbols;
}  

list<VariableSymbol*>* collect_used_symbols(ExecutionObject *exec_obj){
     
    list<VariableSymbol*> *used_symbols = new list<VariableSymbol*>;

    for (Iter<LoadVariableExpression> iter = object_iterator<LoadVariableExpression>(exec_obj);
         iter.is_valid(); iter.next()){
         VariableSymbol *var_sym = (&iter.current())->get_source();
         if (!is_in_list(var_sym, used_symbols))
             used_symbols->push_back(var_sym);
    }
       
    return used_symbols;
}  

list<VariableSymbol*>* collect_array_name_symbols(ExecutionObject *exec_obj){
     
    list<VariableSymbol*> *array_symbols = new list<VariableSymbol*>;
        
    for (Iter<LoadExpression> iter = object_iterator<LoadExpression>(exec_obj);
         iter.is_valid(); iter.next()){
	 Expression* source_address_expr = (&iter.current())->get_source_address();
	 if(is_a<ArrayReferenceExpression>(source_address_expr)){
	    ArrayReferenceExpression *array_ref_expr = to<ArrayReferenceExpression>(source_address_expr);
	    VariableSymbol *array_sym = get_array_name_symbol(array_ref_expr);
            if (!is_in_list(array_sym, array_symbols))
                array_symbols->push_back(array_sym);
	 }
    }
           
    for (Iter<StoreStatement> iter = object_iterator<StoreStatement>(exec_obj);
         iter.is_valid(); iter.next()){
	 Expression* destination_address_expr = (&iter.current())->get_destination_address();
	 if(is_a<ArrayReferenceExpression>(destination_address_expr)){
	    ArrayReferenceExpression *array_ref_expr = to<ArrayReferenceExpression>(destination_address_expr);
	    VariableSymbol *array_sym = get_array_name_symbol(array_ref_expr);
            if (!is_in_list(array_sym, array_symbols))
                array_symbols->push_back(array_sym);
	 }
    }
   
    return array_symbols;
}  

list<VariableSymbol*>* collect_array_name_symbols_defined_in_stores(ExecutionObject *exec_obj){
     
    list<VariableSymbol*> *defined_array_symbols = new list<VariableSymbol*>;
        
    for (Iter<StoreStatement> iter = object_iterator<StoreStatement>(exec_obj);
         iter.is_valid(); iter.next()){
	 Expression* destination_address_expr = (&iter.current())->get_destination_address();
	 if(is_a<ArrayReferenceExpression>(destination_address_expr)){
	    ArrayReferenceExpression *array_ref_expr = to<ArrayReferenceExpression>(destination_address_expr);
	    VariableSymbol *array_sym = get_array_name_symbol(array_ref_expr);
            if (!is_in_list(array_sym, defined_array_symbols))
                defined_array_symbols->push_back(array_sym);
	 }
    }
   
    return defined_array_symbols;
}  

list<VariableSymbol*>* collect_array_name_symbols_used_in_loads(ExecutionObject *exec_obj){
     
    list<VariableSymbol*> *used_array_symbols = new list<VariableSymbol*>;
        
    for (Iter<LoadExpression> iter = object_iterator<LoadExpression>(exec_obj);
         iter.is_valid(); iter.next()){
	 Expression* source_address_expr = (&iter.current())->get_source_address();
	 if(is_a<ArrayReferenceExpression>(source_address_expr)){
	    ArrayReferenceExpression *array_ref_expr = to<ArrayReferenceExpression>(source_address_expr);
	    VariableSymbol *array_sym = get_array_name_symbol(array_ref_expr);
            if (!is_in_list(array_sym, used_array_symbols))
                used_array_symbols->push_back(array_sym);
	 }
    }
   
    return used_array_symbols;
}  


VariableSymbol* get_array_name_symbol(ArrayReferenceExpression *array_ref_expr){

    Expression *base_array_address = array_ref_expr;

    do{
       base_array_address = (to<ArrayReferenceExpression>(base_array_address))->get_base_array_address();
    }while(is_a<ArrayReferenceExpression>(base_array_address));

    SymbolAddressExpression *array_sym_expr = to<SymbolAddressExpression>(base_array_address);
    return to<VariableSymbol>(array_sym_expr->get_addressed_symbol());

}


SymbolTable* get_file_block_symbol_table(ProcedureDefinition *proc_def){

   SymbolTable *st = proc_def->get_symbol_table();
   while(!is_a<FileBlock>(st->get_parent()))
       st = find_super_scope(st);

   return st;

}

SymbolTable* get_external_symbol_table(ProcedureDefinition *proc_def){

   SymbolTable *st = proc_def->get_symbol_table();
   while(!is_a<FileSetBlock>(st->get_parent()))
       st = find_super_scope(st);

   return (to<FileSetBlock>(st->get_parent()))->get_external_symbol_table();

}

void name_variable(VariableSymbol *var_sym, String base_name){

    static suif_map<String, int>* symbol_names = new suif_map<String, int>;

    if(var_sym->get_name() != emptyLString)
       return;

    if(base_name == emptyString)
       base_name = "suifTmp";

    int i = 0;
    if(symbol_names->find(base_name) == symbol_names->end())
       symbol_names->enter_value(base_name, 0);
    else i = symbol_names->lookup(base_name);

    for( ; ; i++){
        String next_name = base_name + String(i);
        if(!LString::exists(next_name)){
           SymbolTable *st = var_sym->get_symbol_table();
           suif_assert_message(st != NULL, ("attempt to name unattached symbol"));
           st->change_name(var_sym, next_name);
           (*(symbol_names->find(base_name))).second = i;
           return;
        }
    }

    suif_assert_message(0, ("Could not form a unique name"));
}

