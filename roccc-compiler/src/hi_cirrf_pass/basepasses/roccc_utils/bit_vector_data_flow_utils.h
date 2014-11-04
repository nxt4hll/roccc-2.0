// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef BIT_VECTOR_DATA_FLOW_UTILS_H
#define BIT_VECTOR_DATA_FLOW_UTILS_H

#include "roccc_extra_types/bit_vector.h"

// kills the definitions in A that has the same destination with s
void kill_annotes(BrickAnnote *A, StoreVariableStatement *s);

// kills the definitions in A that has the same destination with s
void kill_annotes(BrickAnnote *A, CallStatement *s);

// kills the definitions in A that has the same destination with s
void kill_annotes(BrickAnnote *A, StoreStatement *s);

// kills the expressions in A that uses the variable defined by s
void kill_ae_annotes(BrickAnnote *A, StoreVariableStatement *s);

// kills the expressions in A that uses the variable defined by s
void kill_ae_annotes(BrickAnnote *A, CallStatement *s);

void remove_from_in_n_out_stmts(Statement* to_be_removed, ProcedureDefinition *proc_def);
void remove_from_in_n_out_stmts(list<Statement*>* to_be_removed, ProcedureDefinition *proc_def);

void remove_from_kill_set_map(Statement* to_be_removed, ProcedureDefinition *proc_def);
void remove_from_kill_set_map(Statement* to_be_removed, BitVectorMap *bv_map);
void remove_from_kill_set_map(list<Statement*>* to_be_removed, BitVectorMap *bv_map);
void remove_from_kill_set_map(list<Statement*>* to_be_removed, ProcedureDefinition *proc_def);

bool annotes_equal_for_sym(BrickAnnote *A, BrickAnnote *B, SuifObject *s, VariableSymbol *var_sym);

bool is_all_reaching_definitions_in_expr_same(Expression *a, Expression *b);

bool is_all_reaching_definitions_in_this_loop(CForStatement *parent_stmt, LoadVariableExpression *load_var_expr);

bool is_all_reached_uses_in_this_loop(CForStatement *parent_stmt, StoreVariableStatement *store_var_stmt);
bool is_all_reached_uses_in_this_loop(CForStatement *parent_stmt, CallStatement *call_stmt);

#endif

