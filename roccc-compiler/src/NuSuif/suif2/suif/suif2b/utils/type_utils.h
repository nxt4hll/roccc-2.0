#ifndef UTILS__TYPE_UTILS_H
#define UTILS__TYPE_UTILS_H

/** @file
  * This file is a repository for useful utility routines involving types.
  */

#include <suifkernel/suif_env.h>
#include <basicnodes/basic_forwarders.h>
#include <suifnodes/suif_forwarders.h>

#include <common/i_integer.h>
#include <common/suif_vector.h>

#include <common/suif_list.h>
#include <suifnodes/suif.h>
#include <suifkernel/suif_exception.h>
#include "type_pair_stack.h"
#include <typebuilder/type_builder_forwarders.h>

//class IIntOrSourceOp;
TypeBuilder *get_type_builder(SuifEnv *env);


DataType *unqualify_data_type(Type *t);
ProcedureType *unqualify_procedure_type(Type *t);

Type* unqualify_type( Type* t );

IInteger get_data_type_size( DataType* t );
QualifiedType *retrieve_qualified_type( DataType* t );
PointerType *retrieve_pointer_type( Type* t );
LabelType *retrieve_label_type( SuifEnv *s );

DataType *get_data_type(VariableSymbol *sym);




/** @class TypeHelper type_utils.h utils/type_utils.h
 * This class contains methods to manipulate type objects.
 */
class TypeHelper {
 public:

  /**   Get the list of arguments for the procedure type proc.
   *   @return a list of Type*.  If \a proc has no prototype, returns an
   *   empty list.
   */
  static list<Type*> get_arguments(CProcedureType *proc);

  /** @return true iff the array type \a arr has unknown lower and
    * upper bound.
    */
  static bool        is_unknown_bound(ArrayType *arr);


  /**
   * @return true iff \a a1 has unknown bounds but still compatible with \a a2
   */
  static bool is_unknown_but_compatible_bound(ArrayType *a1, ArrayType *a2);

  /**
   * @return true if \a exp describes an undetermined integer.
   */
  static bool        is_unknown_int(Expression *exp);

  /**
   * @return true iff \a qt1 and \a qt2 have the same set of qualifications.
   */
  static bool        is_same_qualified(QualifiedType *qt1, QualifiedType *qt2);

  /**
   *  @return true iff \a t1 and \a t2 are isomorphic structurally.
   */
  static bool        is_isomorphic_type(Type* t1, Type *t2,
					TypePairStack *st = NULL);

  /**
   * @return true if the upper- and lower-bounds of \a a1 and \a a2 are
   * isomorphic.
   */
  static bool        is_isomorphic_bound(ArrayType* a1, ArrayType* a2);


  /**
   *  @return true if \a e1 and \a e2 describes the same integer constant.
   */
  static bool        is_isomorphic_int(Expression* e1, Expression *e2);


  /**
   *  @return a Type* from symtab that is isomorphic with \a t1.  Return NULL
   *     if \a symtab contains no such type object.
   */
  static Type*       find_isomorphic_type(Type* oldtype, SymbolTable *symtab);


  /**
   *   @return the FieldSymbol from \a gtab with name \a fieldname.
   *          return NULL if not found.
   */
  static FieldSymbol *lookup_field(const GroupSymbolTable *gatb,
				   const String& fieldname);


};



#endif
