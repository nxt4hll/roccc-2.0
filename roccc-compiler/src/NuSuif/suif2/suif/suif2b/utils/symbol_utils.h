#ifndef _UTILS__SYMBOL_UTILS_H
#define _UTILS__SYMBOL_UTILS_H

#include "suifkernel/suifkernel_forwarders.h"
#include "basicnodes/basic_forwarders.h"
#include "suifnodes/suif_forwarders.h"

/**	@file
 *	Routines for handling symbols
 */

/**
 *  This function returns TRUE if the Symbol Table is
 *  the external_symbol_table 
 */
bool is_external_symbol_table(SymbolTable *the_table);

/*
 *  This function returns TRUE if the function can tell that
 *  the_symbol is statically allocated.
 */
bool is_statically_allocated(Symbol *the_symbol);

/**
 *  These two functions each return a new unique variable Symbol that
 *  is installed in the nearest enclosing Symbol table of
 *  scope_finder.  If scope_finder is not in a scope, NULL is
 *  returned.  The first version makes up a name for the variable by
 *  itself and the second uses the base_name provided plus some
 *  decimal digits.
 *
 * \warning. This is slow and naming is done in the wrong place. Avoid using
 * these in new code. Variables without good names should be 
 * created with no identifier. A pass will then give them unique
 * names later which are guaranteed not to clash.
 * \see new_anonymous_variable
 * \bug The returned pointer will be NULL if the scope_finder is not valid -
 * This case should probably assert
 */
VariableSymbol *new_unique_variable(SuifEnv *s, 
					   SuifObject *scope_finder, 
					   QualifiedType *the_type);
VariableSymbol *new_unique_variable(SuifEnv *s, 
					   SuifObject *scope_finder, 
					   QualifiedType *the_type,
                                           String base_name);

/**	Create a new anonymous variable
 *
 *	Use this for temporary variables. Naming will be done
 *	later to avoid symbol name clashes
 */
VariableSymbol *new_anonymous_variable(SuifEnv *s,
                                           SuifObject *scope_finder,
                                           QualifiedType *the_type);

/**     Create a new anonymous variable
 *
 *      Use this for temporary variables. Naming will be done
 *      later to avoid symbol name clashes 
 */
VariableSymbol *new_anonymous_variable(SuifEnv *s,
                                           SymbolTable *scope,
                                           QualifiedType *the_type);

/**
 *  These two functions each return a new unique code label Symbol
 *  that is installed in the nearest enclosing Symbol table of
 *  scope_finder.  If scope_finder is not in a scope, NULL is
 *  returned.  The first version makes up a name for the label by
 *  itself and the second uses the base_name provided plus some
 *  decimal digits.
 *  See comments for new_unique_variable. Use new_anonymous_label
 *  instead in new code.
 *  \see new_anonymous_label
 */
CodeLabelSymbol *new_unique_label(SuifEnv *s, SuifObject *scope_finder);
CodeLabelSymbol *new_unique_label(SuifEnv *s, SuifObject *scope_finder,
                                           String base_name);

/**	Create a new anonymous label in the given scope */
CodeLabelSymbol *new_anonymous_label(SuifObject *scope_finder);

CodeLabelSymbol *new_anonymous_label(SymbolTable *scope);

/**
 *  The following four functions are used to lookup variable symbols
 *  by name.  The first two are designed for convenience when only one
 *  match is expected.  if there is more than one match, only the
 *  first match will be returned.  The lookup_var() function looks in
 *  the specified scope and then if the name is not found continues
 *  with enclosing scopes until there are no more enclosing scopes.
 *  If no match is found, NULL is returned.  The lookup_var_locally()
 *  function is the same except that it only looks in the specified
 *  scope, not in any enclosing scopes.  The other two functions, the
 *  ones beginning with ``multi_'' return a (possibly empty) list of
 *  all matches found, but are otherwise the same as the first two
 *  functions.
 */
VariableSymbol *lookup_var(SymbolTable *scope, String name);
VariableSymbol *lookup_var_locally(SymbolTable *scope, String name);
list<VariableSymbol *> *multi_lookup_var(SymbolTable *scope,
                                                String name);
list<VariableSymbol *> *multi_lookup_var_locally(SymbolTable *scope,
                                                        String name);

/**
 *  The following four functions are completely analagous to the last
 *  four except that they apply to procedure symbols instead of
 *  variable symbols.
 */
ProcedureSymbol *lookup_proc(SymbolTable *scope, String name);
ProcedureSymbol *lookup_proc_locally(SymbolTable *scope, String name);
list<ProcedureSymbol *> *multi_lookup_proc(SymbolTable *scope,
                                                  String name);
list<ProcedureSymbol *> *multi_lookup_proc_locally(SymbolTable *scope,
                                                          String name);

/*
 *  The following four functions are completely analagous to the last
 *  four except that they apply to code label symbols instead of
 *  procedure symbols.
 */
CodeLabelSymbol *lookup_code_label(SymbolTable *scope, String name);
CodeLabelSymbol *lookup_code_label_locally(SymbolTable *scope,
                                                    String name);
list<CodeLabelSymbol *> *multi_lookup_code_label(SymbolTable *scope,
                                                         String name);
list<CodeLabelSymbol *> *multi_lookup_code_label_locally(
        SymbolTable *scope, String name);

/**
 *  Return the nearest enclosing Symbol table of
 *  a suif object, or NULL if there is no such Symbol table.
 */
SymbolTable *find_scope(const SuifObject *the_object);

/**
 *  Return the nearest enclosing Symbol table of
 *  a Scoped Object or NULL if there is no such Symbol table.
 */
SymbolTable *find_scoped_object_scope(const ScopedObject *the_obj);

/**
 *  Returns the scope containing the 
 *  symbol table or NULL if there is none.
 *  This is a replacement for the symbol_table::super_scope() method
 */
SymbolTable *find_super_scope(const SymbolTable *scope);


/**
  * @return true if the variable is in the scope of the object.
  */
bool is_in_scope(const VariableSymbol* var, const SuifObject*);


/**
 * create a LoadVariableExpression with the
 * right type in the same suifenv
 */
LoadVariableExpression *create_var_use(VariableSymbol *var);

/**	Change the name of a symbol 
 *      If the symbol is renamed to an emptyLString
 *      the symbol will be removed from the lookup table
 *      and an annotation "orig_name" will be placed on the
 *      symbol so that the symmbol naming pass can make
 *      up a readable derivative name
 */
void rename_symbol(SymbolTableObject *s, const LString &name);

/**
 * Return true if sym is a VariableSymbol or ProcedureSymbol and has
 * a definition.
 */
bool   is_definition_symbol(Symbol* sym);

/**
 *	Find the definition block corresponding to a given symbol table
 */
DefinitionBlock *get_corresponding_definition_block(SymbolTable *st);


#endif
