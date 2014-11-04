
#include "common/system_specific.h"
#include "type_utils.h"

#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "iokernel/cast.h"
#include "typebuilder/type_builder.h"

#include "common/suif_map.h"
#include "print_utils.h"
//#include <iostream.h>
#include <iostream>//jul modif

DataType *unqualify_data_type(Type *t) {
  while (is_kind_of<QualifiedType>(t)) {
    t = to<QualifiedType>(t)->get_base_type();
  }
  if (is_kind_of<DataType>(t)) return(to<DataType>(t));
  suif_assert( false );
  return 0;
}

ProcedureType *unqualify_procedure_type(Type *t) {
  if (is_kind_of<QualifiedType>(t)) {
    return unqualify_procedure_type(to<QualifiedType>(t)->get_base_type());
  }
  if (is_kind_of<ProcedureType>(t)) return(to<ProcedureType>(t));
  return(NULL);
}


Type* unqualify_type( Type* t ) {
  while ( is_kind_of<QualifiedType>(t) ) {
     t = ( to<QualifiedType>(t)) ->get_base_type();
  }
  return t;
}

TypeBuilder *get_type_builder(SuifEnv *env)
{
  TypeBuilder *tb = (TypeBuilder *)
    env->get_object_factory(TypeBuilder::get_class_name());
  
  suif_assert_message(tb, 
		      ("initialization error in typebuilder call init_typebuilder\n"));
  return(tb);
}

IInteger get_data_type_size( DataType* t ) {
  suif_assert( t );
  IInteger size = t->get_bit_size();
  return size;
}

QualifiedType *retrieve_qualified_type(DataType *t) {
  TypeBuilder *tb = get_type_builder(t->get_suif_env());
  suif_assert_message(tb, ("initialization error in typebuilder call init_typebuilder\n"));
  return(tb->get_qualified_type(t));
}

PointerType *retrieve_pointer_type(Type *t) {
  TypeBuilder *tb = (TypeBuilder *)
    t->get_suif_env()->get_object_factory(TypeBuilder::get_class_name());
  suif_assert_message(tb, ("initialization error in typebuilder call init_typebuilder\n"));
  return(tb->get_pointer_type(t));
}

LabelType *retrieve_label_type(SuifEnv *s) {
  TypeBuilder *tb = (TypeBuilder *)
    s->get_object_factory(TypeBuilder::get_class_name());
  suif_assert_message(tb, ("initialization error in typebuilder call init_typebuilder\n"));
  return(tb->get_label_type());
}

DataType *get_data_type(VariableSymbol *sym) {
#ifdef NO_COVARIANCE
    return to<QualifiedType>(sym->get_type())->get_base_type();
#else
    return sym->get_type()->get_base_type();
#endif
    }
















list<Type*> TypeHelper::get_arguments(CProcedureType *proc)
{
  list<Type*> l;
  for (Iter<QualifiedType*> it = proc->get_argument_iterator();
       it.is_valid();
       it.next()) {
    l.push_back(it.current());
  }
  return l;
}




bool TypeHelper::is_unknown_bound(ArrayType *arr)
{
  return (is_unknown_int(arr->get_lower_bound()) &&
	  is_unknown_int(arr->get_upper_bound()));
}


/** Check if one array has unknown bound and the other has known bound nad
  * they are compatible.
  *
  * @return true if \a a1 has unknown bound but compatible with that of
  *              \a a2.
  *
  * This method is not symmetric.  Interchange value of \a a1 and \a a2
  * may produce different result.
  */
bool TypeHelper::is_unknown_but_compatible_bound(ArrayType *a1, ArrayType *a2)
{
  Expression *lb1 = a1->get_lower_bound();
  Expression *ub1 = a1->get_upper_bound();
  Expression *lb2 = a2->get_lower_bound();
  Expression *ub2 = a2->get_upper_bound();
  return ((is_unknown_int(ub1) && is_isomorphic_int(lb1, lb2)) ||
	  (is_unknown_int(lb1) && is_isomorphic_int(ub1, ub2)));
}


// Return true if both qt1 and qt2 have the same qualification.
//
bool TypeHelper::is_same_qualified(QualifiedType *qt1, QualifiedType *qt2)
{
  if (qt1->get_qualification_count() != qt2->get_qualification_count())
    return false;
  Iter<LString> it = qt1->get_qualification_iterator();
  for (; it.is_valid(); it.next()) {
    if (!qt2->has_qualification_member(it.current()))
      return false;
  }
  return true;
}









/* ***************************************************************
 * is_isomorphic_bound
 ***************************************************************** */
bool TypeHelper::is_isomorphic_bound(ArrayType *t1, ArrayType *t2)
{
  return (is_isomorphic_int(t1->get_lower_bound(), t2->get_lower_bound()) &&
	  is_isomorphic_int(t1->get_upper_bound(), t2->get_upper_bound()));
}




bool TypeHelper::is_isomorphic_int(Expression* e1, Expression *e2)
{
  if (!is_kind_of<IntConstant>(e1))
      return false;
  if (!is_kind_of<IntConstant>(e2))
      return false;
  IntConstant *int1 = to<IntConstant>(e1);
  IntConstant *int2 = to<IntConstant>(e2);
  return (int1->get_value() == int2->get_value() ||
	  (int1->get_value().is_undetermined() &&
	   int2->get_value().is_undetermined()));
}



bool TypeHelper::is_unknown_int(Expression* b1)
{
  return (is_kind_of<IntConstant>(b1) &&
	  to<IntConstant>(b1)->get_value().is_undetermined());
}





/* **************************************************************
 * is_isomorphic_type
 *
 *****************************************************************/

/**
  * Precondition: t1 != t2,  (t1,t2) not in isomap,
  *               t1->get_name() == t2->get_name()
  */
static bool is_isomorphic_void(VoidType* t1, VoidType* t2,
			       TypePairStack *isomap)
{
  return true;
}

static bool is_isomorphic_boolean(BooleanType* t1, BooleanType* t2,
				TypePairStack *isomap)
{
  return true;
}

static bool is_isomorphic_enumerated(EnumeratedType *t1, EnumeratedType *t2,
				     TypePairStack *isomap)
{
  if (t1->get_case_count() != t2->get_case_count())
    return false;
  for (Iter<EnumeratedType::case_pair> it = t1->get_case_iterator();
       it.is_valid();
       it.next()) {
    LString name = it.current().first;
    IInteger value = it.current().second;
    if (!t2->has_case_member(name)) return false;
    if (value != t2->lookup_case(name)) return false;
  }
  return true;
}
  

static bool is_isomorphic_integer(IntegerType *t1, IntegerType *t2, 
				  TypePairStack *isomap)
{
  return (t1->get_is_signed() == t2->get_is_signed() &&
	  t1->get_bit_size() == t2->get_bit_size() &&
	  t1->get_bit_alignment() == t2->get_bit_alignment());
}

static bool is_isomorphic_float(FloatingPointType *t1, FloatingPointType *t2,
				TypePairStack *isomap)
{
  return (t1->get_bit_size() == t2->get_bit_size() &&
	  t1->get_bit_alignment() == t2->get_bit_alignment());
}

static bool is_isomorphic_pointer(PointerType *t1, PointerType *t2,
				  TypePairStack *isomap)
{
  return TypeHelper::is_isomorphic_type(t1->get_reference_type(),
					t2->get_reference_type(),
					isomap);
}

static bool is_isomorphic_array(ArrayType *t1, ArrayType *t2,
				TypePairStack *isomap)
{
  return (TypeHelper::is_isomorphic_type(t1->get_element_type(),
					 t2->get_element_type(),
					 isomap) &&
	  TypeHelper::is_isomorphic_bound(t1, t2));
}


static bool is_isomorphic_field(FieldSymbol *f1, FieldSymbol *f2,
				TypePairStack *isomap)
{
  return (TypeHelper::is_isomorphic_int(f1->get_bit_offset(),
					f2->get_bit_offset()) &&
	  TypeHelper::is_isomorphic_type(f1->get_type(), f2->get_type(),
					 isomap));
}


FieldSymbol *TypeHelper::lookup_field(const GroupSymbolTable *gtab,
				      const String &name)
{
  for (Iter<SymbolTableObject*> it = gtab->get_symbol_table_object_iterator();
       it.is_valid();
       it.next()) {
    if (it.current()->get_name() == name)
      return to<FieldSymbol>(it.current());
  }
  return NULL;
}


static bool is_isomorphic_group(GroupType *t1, GroupType *t2,
				TypePairStack *isomap)
{
  if (t1->get_meta_class() != t2->get_meta_class())
    return false;
  if (t1->get_is_complete() != t2->get_is_complete())
    return false;
  if (!t1->get_is_complete())
    return (t1->get_name() == t2->get_name());
  //
  // both are complete
  //
  GroupSymbolTable *gtab1 = t1->get_group_symbol_table();
  GroupSymbolTable *gtab2 = t2->get_group_symbol_table();
  if (gtab1->get_symbol_table_object_count() !=
      gtab2->get_symbol_table_object_count())
    return false;
  TypePairStack mymap(t1, t2, isomap);
  for (Iter<SymbolTableObject*> it = gtab1->get_symbol_table_object_iterator();
       it.is_valid();
       it.next()) {
    FieldSymbol *field1 = to<FieldSymbol>(it.current());
    FieldSymbol *field2 = TypeHelper::lookup_field(gtab2, field1->get_name());
    if (field2 == NULL) return false;
    if (!is_isomorphic_field(field1, field2, &mymap))
      return false;
  }
  return true;
}

   
static bool is_isomorphic_cprocedure(CProcedureType *t1, CProcedureType *t2,
				     TypePairStack *isomap)
{
  if (t1->get_arguments_known() != t2->get_arguments_known() ||
      t1->get_has_varargs() != t2->get_has_varargs() ||
      // t1->get_bit_alignment() != t2->get_bit_alignment() ||
      !TypeHelper::is_isomorphic_type(t1->get_result_type(),
				      t2->get_result_type(),
				      isomap) ||
      t1->get_argument_count() != t2->get_argument_count())
    return false;
  for (size_t i=0; i<t1->get_argument_count(); i++) {
    if (!TypeHelper::is_isomorphic_type(t1->get_argument(i),
					t2->get_argument(i),
					isomap))
      return false;
  }
  return true;
}


static bool is_isomorphic_qualified(QualifiedType *t1, QualifiedType *t2,
				    TypePairStack *isomap)
{
  return (TypeHelper::is_same_qualified(t1, t2) &&
	  TypeHelper::is_isomorphic_type(t1->get_base_type(),
					 t2->get_base_type(),
					 isomap));
}


// PreCondition 1. t1 and t2 are instances of the same C++ class.
//              2. (t1,t2) not in isomap
//              3. t1->get_name() == t2->get_name()
//
static bool is_isomorphic_subtype(Type* t1, Type *t2, TypePairStack *isomap)
{
  if (is_kind_of<VoidType>(t1))
    return is_isomorphic_void(to<VoidType>(t1), to<VoidType>(t2), isomap);
  if (is_kind_of<BooleanType>(t1))
    return is_isomorphic_boolean(to<BooleanType>(t1), to<BooleanType>(t2), isomap);
  if (is_kind_of<EnumeratedType>(t1))
    return is_isomorphic_enumerated(to<EnumeratedType>(t1),
				    to<EnumeratedType>(t2),
				    isomap);
  if (is_kind_of<IntegerType>(t1))
    return is_isomorphic_integer(to<IntegerType>(t1), to<IntegerType>(t2),
				 isomap);
  if (is_kind_of<FloatingPointType>(t1))
    return is_isomorphic_float(to<FloatingPointType>(t1),
			       to<FloatingPointType>(t2),
			       isomap);
  if (is_kind_of<PointerType>(t1))
    return is_isomorphic_pointer(to<PointerType>(t1), to<PointerType>(t2),
				 isomap);
  if (is_kind_of<ArrayType>(t1))
    return is_isomorphic_array(to<ArrayType>(t1), to<ArrayType>(t2), isomap);
  if (is_kind_of<GroupType>(t1))
    return is_isomorphic_group(to<GroupType>(t1), to<GroupType>(t2), isomap);
  if (is_kind_of<CProcedureType>(t1))
    return is_isomorphic_cprocedure(to<CProcedureType>(t1),
				    to<CProcedureType>(t2),
				    isomap);
  if (is_kind_of<QualifiedType>(t1))
    return is_isomorphic_qualified(to<QualifiedType>(t1),
				   to<QualifiedType>(t2),
				   isomap);
  if (is_kind_of<LabelType>(t1))
    return true;
  // SUIF_THROW(SuifDevException(__FILE__, __LINE__, 
  //			      String("Unknown type ") + to_id_string(t1)));
  return false;
}





// Return true if t1 and t2 are isomorphic structurally.
//
bool TypeHelper::is_isomorphic_type(Type *t1, Type *t2, TypePairStack *isomap)
{
  if (t1 == t2) return true;
  if (t1->getClassName() != t2->getClassName())
    return false;
  if (t1->get_name() != t2->get_name())
    return false;
  if (isomap != NULL && isomap->is_in(t1, t2))
    return true;
  return is_isomorphic_subtype(t1, t2, isomap);
}
    



/* ******************************************************************
 * find_isomorphic_type
 ******************************************************************** */


// Find in symtab a Type* isomorphic to oldtype.
// Return a Type instance or NULL
//
Type* TypeHelper::find_isomorphic_type(Type* oldtype, SymbolTable *symtab)
{
  if (symtab == NULL) return NULL;
  Iter<SymbolTableObject*> it = symtab->get_symbol_table_object_iterator();
  for (; it.is_valid(); it.next()) {
    if (!is_kind_of<Type>(it.current()))
      continue;
    Type *entry = to<Type>(it.current());
    if (entry->get_name() != oldtype->get_name())
      continue;
    if (is_isomorphic_type(entry, oldtype))
      return entry;
  }
  return find_isomorphic_type(oldtype, symtab->get_explicit_super_scope());
}
