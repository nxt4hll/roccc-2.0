// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef IR_UTILS_H
#define IR_UTILS_H

/**************************** Declarations ************************************/

bool is_textually_preceeding(Statement *s1, Statement *s2, Statement *s=0);

bool is_at_the_same_nesting_level(Statement *s1, Statement *s2);

int get_statement_pos(Statement *stmt);

Statement* get_next_statement(Statement *stmt);

bool is_equal(SuifObject *a, SuifObject *b);

bool is_enclosing(ExecutionObject *p, ExecutionObject *s);

Statement* get_enclosing_loop(ExecutionObject *s);
CForStatement* get_enclosing_c_for_stmt(ExecutionObject *s);

void remove_untargeted_labels(ProcedureDefinition *proc_def);
void remove_statements(list<Statement*>* to_be_removed);

bool is_stmt_within_begin_end_hw_marks(Statement *stmt);

int get_signed_bit_size(long N);
int get_unsigned_bit_size(long N);

#endif

