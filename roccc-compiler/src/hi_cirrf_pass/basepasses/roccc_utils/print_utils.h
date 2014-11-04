// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef PRINT_UTILS_H
#define PRINT_UTILS_H

/**************************** Declarations ************************************/

String print_statement(Statement *s);

String print_expression(Expression *e);

String print_execution_object(ExecutionObject *e);

String print_symbol(Symbol *s);

String print_type(Type *t);

#endif

