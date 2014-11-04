// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef SYMBOL_UTILS_H
#define SYMBOL_UTILS_H

list<VariableSymbol*>* collect_variable_symbols(ExecutionObject *exec_obj);
list<VariableSymbol*>* collect_defined_symbols(ExecutionObject *exec_obj);
list<VariableSymbol*>* collect_used_symbols(ExecutionObject *exec_obj);

list<VariableSymbol*>* collect_array_name_symbols(ExecutionObject *exec_obj);
list<VariableSymbol*>* collect_array_name_symbols_defined_in_stores(ExecutionObject *exec_obj);
list<VariableSymbol*>* collect_array_name_symbols_used_in_loads(ExecutionObject *exec_obj);
VariableSymbol* get_array_name_symbol(ArrayReferenceExpression *array_ref_expr);

SymbolTable* get_file_block_symbol_table(ProcedureDefinition *proc_def);
SymbolTable* get_external_symbol_table(ProcedureDefinition *proc_def);

void name_variable(VariableSymbol *var_sym, String base_name = "");

#endif

