#ifndef _UTILS__EXPRESSION_UTILS_H
#define _UTILS__EXPRESSION_UTILS_H
#include "suifkernel/suif_env.h"
#include "basicnodes/basic_forwarders.h"
#include "suifnodes/suif_forwarders.h"
#include "cfenodes/cfe_forwarders.h"
#include "common/i_integer.h"
#include "common/common_forwarders.h"
#include "common/suif_indexed_list.h"
//class SourceOp;

/**	@file 
 *	Various utilities for expression manipulation
 *	(and, sadly) a number which are not
 */

/**	Create an integer constant. The type will be the type for
 *	word_type taken fromt the target information block */
IntConstant* create_int_constant( SuifEnv* suif_env, IInteger i );

/**	Get an integer constant from an expression.
 *	Returns indeterminate if the expression is not an
 *	integer constant */
IInteger get_expression_constant(const Expression *exp);

/**
 * Checks whether the variable \a var is modified inside \a obj. 
 
 * Notice that the results are likely to be overly consirvsative 
 * at this point, as no real mod analysis is performed.
*/
bool is_variable_modified(const VariableSymbol* var, ExecutionObject* obj);

/**
 * Checks whether the value of expression \a expr is modified inside 
 * \a obj. 
 
 * Notice that the results are likely to be overly consirvsative 
 * at this point, as no real mod analysis is performed.
*/
bool is_expr_modified(const Expression* expr, ExecutionObject* obj);

/**	Get the procedure to be called by a call statement */
ProcedureSymbol *get_procedure_target_from_call(const CallStatement *call);

/*	Get the procedure to be called given the SymbolAddressExpression for the call. */
ProcedureSymbol *get_procedure_from_address_op(const Expression *procedure_address);

/*	Get the procedure containing a given executable object */
ProcedureDefinition *get_procedure_definition(const ExecutionObject *eo);

/**	Remove an object from its owner, and clear its parent pointer.
 *	This should be moved to somewhere else 
 */
SuifObject *remove_suif_object(SuifObject *obj);

/** remove the statement
 * If it's parent is a statement list, we will 
 * do the appropriate removal.
 * If not, we will replace it with a new empty statement list.
 */
Statement *remove_statement(Statement *the_statement);

/**	replace an expression with a new expression
 *	A thin and somewhat redundent wrapper around
 *	expr->get_parent()->replace(expr,replacement);
 */
Expression *replace_expression(Expression *expr, 
			       Expression *replacement);

/**	Replace a statement with a new statement */
Statement *replace_statement(Statement *stmt, 
			       Statement *replacement);


/**	Turn the expression into a store of a variable. followed
 *	by a reference.
 *      This can cause major code changes if the expression is
 *      in a do-while or while-do loop test.
 *
 *      If new statements are required, they will be returned
 *      It is possible to create Multiple statements from a
 *      single expression.
 */
list<StoreVariableStatement*> *force_dest_not_expr(Expression *expr);
void force_dest_not_expr(Expression *expr,
			 list<StoreVariableStatement*> &dismantled);


/**	Return true if the target_label could a branch target
 *      for the Statement
 *      For code labels with their address taken, the result
 *      is always true.
 */
bool is_target_label(CodeLabelSymbol *target_label,
		     Statement *the_statement);


/**	Insert a statement at the end of a loop body.
 *      If the loop continue label is used
 *      a new continue label will be created
 *      and an explicit label will be placed before the
 *      inserted statement.
 */
void insert_statement_after_loop_body(ForStatement *the_loop,
				      Statement *the_statement);

/**	Insert a statement at the end of a loop body.
 *      If the loop continue label is used
 *      a new continue label will be created
 *      and an explicit label will be placed before the
 *      inserted statement.
 */
void insert_statement_after_loop_body(DoWhileStatement *the_loop,
				      Statement *the_statement);

/**	Insert a statement at the end of a loop body.
 *      If the loop continue label is used
 *      a new continue label will be created
 *      and an explicit label will be placed before the
 *      inserted statement.
 */
void insert_statement_after_loop_body(WhileStatement *the_loop,
				      Statement *the_statement);


/**	Smart statement insert.
 *	Puts the existing statement into a statement list and inserts in that
 *	statement list, if neccessary. If loc is already in a statement list,
 *	or is a statement list itself, the existing statement is used.
 *
 *	Returns the list into which the insertion was made
 */
Statement *insert_statement_before(Statement *loc, Statement *the_statement);

/**	Insert statement after the given statement
 *	Similar optimizations to insert_statement_before apply 
 */
Statement *insert_statement_after(Statement *loc, Statement *the_statement);

/**	Return true if there is no destination for an expression.
 *	Currently, this means that it is an Eval Statement.
 */
bool is_void_dest(Expression *expr);

 /**	Create an assignment statement */
Statement *build_assign(VariableSymbol *dst, Expression *expr);
/**	Create an assignment statement */
Statement *build_assign(VariableSymbol *dest, VariableSymbol *src);
/**	Create a cast */
Expression *build_cast(Expression *expr, DataType *t);

/** 	find the statement at the base of this expression tree.
 */
Statement *get_expression_owner(Expression *expr);

/**	 If target_proc is 0, find the direct target from the call.
 * otherwise, the target proc should be consistent with the
 * call's address operand.
 *	(Original comment preserved - no idea what this does)
 */
#ifdef CALL_EXP
void inline_call(CallExpression *the_call,
		 ProcedureSymbol *target_proc);
void dismantle_a_call_expression(CallExpression *cal);
#else
void inline_call(CallStatement *the_call,
		 ProcedureSymbol *target_proc);
#endif

//	Expression generation routines which fold constants if possible
//	NB: If folding occurs, the original expressions will be deleted if
//	they lack a parent. If they have a parent and are used, they will
//	be cloned.
//	The returned expression will always be directly usable. For example,
//	if the right hand side of an add is zero so that the left hand side
//	can be returned, the lhs will be cloned if neccessary

/**	Build a dyadic expression
 *	Constant folding is done if needed. (very minimal at present)
 *	If constant folding occurs, and the parameter expressions have no parent, they are deleted. 
 *	If no folding occurs and the parameter expressions have parents, they are cloned
 */
Expression * build_dyadic_expression(LString op,Expression *left,Expression *right);

/**	Build a monadic expression. The monadic equivalent of the above */
Expression * build_monadic_expression(LString op,Expression *subexp);

/**	Clone an expression if it has a parent.
 *	\warning. Think before doing this. Cloning an expression will result in two evaluations
 *	of the expression. Perhaps you want to assign to a variable and use that instead.
 *	Most often used with constants
 */
Expression *clone_if_needed(Expression *subexp);


/**	Restrict an iinteger to the bit_size, signed-ness
 * use 2s complement on overflow
 */

IInteger restrict_int_to_data_type(DataType *t, const IInteger &ii);


/**
 * Return a constant folded version of the source expression.
 * Any portion of the expression determined to be constant
 * will be replaced with a constant expression.
 * Any replaced expressions (including the source expression)
 * will be either re-used or tossed in the trash
 *
 * These routines do not yet fold Floating Point constants.
 * 
 * WARNING.  This may update the IR node that the expression
 * is attached to. Be careful when using this while walking.
 *
 * This constant folder is extensible. To add a new IR Expression node
 * folding routine or a new Binary or Unary opcode routine:
 * <code>
 * #include "utils/fold_table.h"
 *
 * FoldTable *fold_table = FoldTable::get_fold_table(suif_env);
 * fold_table->add_binary_fold_fn(opcode, fn);
 * fold_table->add_unary_fold_fn(opcode, fn);
 * fold_table->add_expression_fold_fn(IR_meta_class_name, fn);
 * </code>
 */

Expression *fold_and_replace_constants(Expression *expr);

/**
 * Return a constant folded version of the statement.
 * Branches or If Statements will be simplified,
 * Empty StatementLists will be removed.
 * The expressions used by the statementes will be simplified.
 * WARNING.  This may update the IR node that the statement
 * is attached to. Be careful when using this while walking.
 * 
 * This constant folder is extensible. To add a new IR node
 * folding routine:
 * <code>
 * #include "utils/fold_table.h"
 *
 * FoldTable *fold_table = FoldTable::get_fold_table(suif_env);
 * fold_table->add_statement_fold_fn(IR_meta_class_name, fn);
 * </code>
 */
Statement *fold_and_replace_constants(Statement *statement);

/**
    Add jump targets that are possibly contained in \a stmt to the
    list of targets \a targets. If the statement isn't a jump of
    some sort, nothing is added to the list.

    \warning This currently doesn't support indirect jumps.
*/
void add_targets(Statement* stmt, list<CodeLabelSymbol*>& targets);

/**
	Appends all jumps from outside of the \a scope to \a targets.
*/
void get_jumps_from_outside(ScopedObject* scope, 
	searchable_list<CodeLabelSymbol*>& targets);


/**
    Returns true of the scoped object has a jump going into it from the
    outside.
*/
bool has_jumps_going_inside(ScopedObject* scope);

/**
   Clone an expression.  All references to non-owned objects
   will remain
*/
Expression *clone_expression(Expression *expr);

#endif
