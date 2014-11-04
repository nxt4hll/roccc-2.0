#ifndef STATEMENT_UTILS_H
#define STATEMENT_UTILS_H

#include <suifnodes/suif_forwarders.h>
#include <basicnodes/basic_forwarders.h>
#include <common/i_integer.h>
/** check for a guarded for statement
 * A guarded for statement will guarantee that
 * the body of the for statement is executed at
 * least one time.
 *
 * Guarding a for statement usually involves
 * surrounding it with an If Statement that
 * is the same as the initial condition on the
 * for entry.
 *
 * Because this adds a commonly used semantic, some
 * transformations of FOR statements should remove
 * this guard after a transformation
 */
bool is_for_statement_guarded(ForStatement *the_for);

/**
  * after doing analysis that proves that a for statement
  * is guarded, you may call this
  */
void set_for_statement_guarded(ForStatement *the_for);

/**
  * Remove the guard from the for statement.
  * It is always valid to call this.
  */
void set_for_statement_unguarded(ForStatement *the_for);

/**
  * Add a conditional to guard the for statement
  * Then mark it guarded
  *  If it is already guarded, this is a NOOP
  *  If the bounds are constant, it will just be marked
  * the return value with be the for statement
  *  or a statement that has replaced the For statement.
  */
Statement *guard_for_statement(ForStatement *the_for);

Expression *build_for_statement_entry_test(ForStatement *the_for);

/**
  * try to statically evaluate the for statement entry test.
  *  result is 
  *   ii_undetermined()  - Can not tell
  *
  *   1 for TRUE, the for statement will always
  *      be entered
  *
  *   0 for FALSE, the for statement will never be entered
  */
  
IInteger evaluate_for_statement_entry_test(ForStatement *the_for);




#endif /* STATEMENT_UTILS_H */
