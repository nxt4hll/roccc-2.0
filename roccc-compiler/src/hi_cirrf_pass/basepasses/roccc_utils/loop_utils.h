// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef LOOP_UTILS_H
#define LOOP_UTILS_H

#include <suifnodes/suif.h>
#include <cfenodes/cfe.h>

/**************************** Declarations ************************************/

VariableSymbol* get_c_for_basic_induction_variable(CForStatement *the_c_for);
list<VariableSymbol*>* get_c_for_induction_variables(CForStatement *the_c_for);

list<VariableSymbol*>* get_loop_invariants(Statement *loop_body, list<VariableSymbol*> *induction_variables);
list<VariableSymbol*>* get_loop_invariants(Statement *loop_body, VariableSymbol* induction_variable);

int get_c_for_lower_bound(CForStatement *the_c_for);
void set_c_for_lower_bound(CForStatement *the_c_for, int lb);

Expression* get_c_for_lower_bound_expr(CForStatement *the_c_for);
void set_c_for_lower_bound_expr(CForStatement *the_c_for, Expression *lb_expr);

int get_c_for_upper_bound(CForStatement *the_c_for);
void set_c_for_upper_bound(CForStatement *the_c_for, int ub);

Expression* get_c_for_upper_bound_expr(CForStatement *the_c_for);
void set_c_for_upper_bound_expr(CForStatement *the_c_for, Expression *ub_expr);

String get_c_for_test_opcode(CForStatement *the_c_for);
void set_c_for_test_opcode(CForStatement *the_c_for, String new_test_opcode);

int get_c_for_step(CForStatement *the_c_for);
void set_c_for_step(CForStatement *the_c_for, int step);

int get_c_for_iteration_count(CForStatement *the_c_for);

int is_error_code_set();
void reset_error_code();

#endif

