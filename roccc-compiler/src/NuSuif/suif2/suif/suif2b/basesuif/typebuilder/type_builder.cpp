#include "common/system_specific.h"
/*
 *  SUIF type factory
 *
 * Tries to ensure there is only one type of a given kind in a symbol
 * table tree.
 *
 * Inserts stuff in the right symbol table
 */


#include "type_builder.h"


#include "suifkernel/suif_object.h"
#include "suifkernel/iter.h"
#include "suifkernel/module.h"
#include "suifkernel/module_subsystem.h"
#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif_forwarders.h"
#include "suifkernel/suif_env.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"

//#include "suifkernel/error_macros.h"
#include "iokernel/cast.h"

#include "common/suif_list.h"


class TypeBuilderModule : public Module {
public:
  TypeBuilderModule(SuifEnv *suif) : Module(suif,"TypeBuilder") {}
  virtual void initialize() {
    _suif_env->add_object_factory( new TypeBuilder() );
  }
  virtual Module* clone() const { return (Module *)this; }
};

static const LString type_builder_class_name("TypeBuilder");


extern "C" void EXPORT init_typebuilder( SuifEnv *suif )
{
  suif->require_module("suifnodes");

  ModuleSubSystem *mSubSystem = suif->get_module_subsystem();
  if (!mSubSystem->retrieve_module(type_builder_class_name)) {
    mSubSystem -> register_module(new TypeBuilderModule( suif ));
    }
}


const LString &TypeBuilder::get_class_name() {
  return (type_builder_class_name);
}

const LString &TypeBuilder::getName() {

  return (type_builder_class_name);
}

void TypeBuilder::init( SuifEnv *env ) {
  _suif_env = env;
}

DataType *TypeBuilder::unqualify_data_type(Type *t) {
    while (is_kind_of<QualifiedType>(t)) {
    	t = (to<QualifiedType>(t))->get_base_type();
  	}
    if (is_kind_of<DataType>(t)) return(to<DataType>(t));
    return(NULL);
    }

Type* TypeBuilder::unqualify_type( Type* t ) {
    while ( is_kind_of<QualifiedType>(t) ) {
	t = ( to<QualifiedType>(t)) ->get_base_type();
	}
    return t;
    }

VoidType * TypeBuilder::get_void_type() {
    // Look for existing VoidType in symbol table
    FileSetBlock *block = _suif_env->get_file_set_block();
    suif_assert_message(block, ("fileset block not attached to suifenv"));
    SymbolTable *symbolTable = block->get_external_symbol_table();
    suif_assert_message(symbolTable, ("external_symbol table  not attached"));
    Iter<SymbolTableObject *> iter = symbolTable->get_symbol_table_object_iterator();
    while (iter.is_valid()) {
	SymbolTableObject *obj = iter.current();
        if (is_a<VoidType>(obj))
            return to<VoidType>(obj);
        iter.next();
        }
    VoidType * element = create_void_type(_suif_env,0,0);
    symbolTable->append_symbol_table_object(element);
    return (element);
    }

BooleanType* TypeBuilder::get_boolean_type( IInteger size_in_bits, int alignment_in_bits ) {
    FileSetBlock *block = _suif_env->get_file_set_block();
    suif_assert_message(block, ("fileset block not attached to suifenv"));
    SymbolTable *symbolTable = block->get_external_symbol_table();
    suif_assert_message(symbolTable, ("external_symbol table  not attached"));
    Iter<SymbolTableObject *> iter = symbolTable->get_symbol_table_object_iterator();
    while (iter.is_valid()) {
        SymbolTableObject *obj = iter.current();
        if (is_a<BooleanType>(obj)) {
	    BooleanType *element = to<BooleanType>(obj);
	    if (( element->get_bit_size() == size_in_bits
        	&& element->get_bit_alignment() == alignment_in_bits))
		return element;
	    }
        iter.next();
        }
    BooleanType * element = create_boolean_type(_suif_env, size_in_bits, alignment_in_bits);
    symbolTable->append_symbol_table_object(element);
    return (element);
    }


IntegerType* TypeBuilder::get_integer_type( IInteger size_in_bits, int alignment_in_bits, bool is_signed) {
    FileSetBlock *block = _suif_env->get_file_set_block();
    suif_assert_message(block, ("fileset block not attached to suifenv"));
    SymbolTable *symbolTable = block->get_external_symbol_table();
    suif_assert_message(symbolTable, ("external_symbol table  not attached"));
    Iter<SymbolTableObject *> iter = symbolTable->get_symbol_table_object_iterator();
    while (iter.is_valid()) {
        SymbolTableObject *obj = iter.current();
        if (is_a<IntegerType>(obj)) {
            IntegerType *element = to<IntegerType>(obj);
            if (( element->get_bit_size() == size_in_bits)
		&& (element->get_is_signed() == is_signed)
                && (element->get_bit_alignment() == alignment_in_bits))
                return element;
            }
        iter.next();
        }
    IntegerType * element = create_integer_type(_suif_env,size_in_bits, alignment_in_bits,is_signed);
    symbolTable->append_symbol_table_object(element);
    return (element);
    }

IntegerType* TypeBuilder::get_smallest_integer_type() {
    FileSetBlock *block = _suif_env->get_file_set_block();
    suif_assert_message(block, ("fileset block not attached to suifenv"));
    SymbolTable *symbolTable = block->get_external_symbol_table();
    suif_assert_message(symbolTable, ("external_symbol table  not attached"));
    Iter<SymbolTableObject *> iter = symbolTable->get_symbol_table_object_iterator();
    IntegerType *smallest_yet = NULL;
    int smallest_size_yet = 0;
    int smallest_alignment_yet = 0;

    while (iter.is_valid()) {
        SymbolTableObject *obj = iter.current();
        if (is_a<IntegerType>(obj)) {
            IntegerType *element = to<IntegerType>(obj);
            if ((smallest_yet == NULL) ||
		(element->get_bit_size() < smallest_size_yet) ||
		((element->get_bit_size() < smallest_size_yet) &&
		   (element->get_bit_alignment() < smallest_alignment_yet)))
	      {
		smallest_yet = element;
		smallest_size_yet = element->get_bit_size().c_int();
		smallest_alignment_yet = element->get_bit_alignment();
	      }
            }
        iter.next();
        }
    return (smallest_yet);
    }




FloatingPointType* TypeBuilder::get_floating_point_type( IInteger size_in_bits, int alignment_in_bits ) {
    FileSetBlock *block = _suif_env->get_file_set_block();
    suif_assert_message(block, ("fileset block not attached to suifenv"));
    SymbolTable *symbolTable = block->get_external_symbol_table();
    suif_assert_message(symbolTable, ("external_symbol table  not attached"));
    Iter<SymbolTableObject *> iter = symbolTable->get_symbol_table_object_iterator();
    while (iter.is_valid()) {
        SymbolTableObject *obj = iter.current();
        if (is_a<FloatingPointType>(obj)) {
            FloatingPointType *element = to<FloatingPointType>(obj);
            if (( element->get_bit_size() == size_in_bits)
                && (element->get_bit_alignment() == alignment_in_bits))
                return element;
            }
        iter.next();
        }
    FloatingPointType * element = create_floating_point_type(_suif_env, size_in_bits, alignment_in_bits);
    symbolTable->append_symbol_table_object(element);
    return (element);
    }

PointerType* TypeBuilder::get_pointer_type( IInteger size_in_bits, int alignment_in_bits, Type* reference_type) {
  if (is_kind_of<DataType>(reference_type)) {
    // Qualify it before getting the pointer.
    reference_type = get_qualified_type(to<DataType>(reference_type));
  }
  //FileSetBlock *block = _suif_env->get_file_set_block();
    PointerType *element;
    SymbolTable *symbolTable = reference_type->get_symbol_table();
    suif_assert_message(symbolTable, ("reference type not attached"));
    Iter<SymbolTableObject *> iter = symbolTable->get_symbol_table_object_iterator();
    while (iter.is_valid()) {
    	SymbolTableObject *obj = iter.current();

    	if (is_a<PointerType>(obj)) {
	    element = to<PointerType>(obj);

    	    if (element->get_reference_type() == reference_type
             && element->get_bit_size() == size_in_bits
             && element->get_bit_alignment() == alignment_in_bits)
      		return (element);
  	    }
	iter.next();
	}
    element = create_pointer_type(_suif_env, size_in_bits, alignment_in_bits,
                                             reference_type);

    symbolTable->append_symbol_table_object(element);

    return (element);
    }


ReferenceType* TypeBuilder::get_reference_type( IInteger size_in_bits, int alignment_in_bits, Type* reference_type) {
    if (is_kind_of<DataType>(reference_type)) {
        // Qualify it before getting the reference.
        reference_type = get_qualified_type(to<DataType>(reference_type));
        }
    ReferenceType *element;
    SymbolTable *symbolTable = reference_type->get_symbol_table();
    suif_assert_message(symbolTable, ("reference type not attached"));
    Iter<SymbolTableObject *> iter = symbolTable->get_symbol_table_object_iterator();
    while (iter.is_valid()) {
    	SymbolTableObject *obj = iter.current();

    	if (is_a<ReferenceType>(obj)) {
	    element = to<ReferenceType>(obj);

    	    if (element->get_reference_type() == reference_type
             && element->get_bit_size() == size_in_bits
             && element->get_bit_alignment() == alignment_in_bits)
      		return (element);
  	    }
	iter.next();
	}
    element = create_reference_type(_suif_env, size_in_bits, alignment_in_bits,
                                             reference_type);

    symbolTable->append_symbol_table_object(element);

    return (element);
    }

static bool is_equal_expression(Expression *a, Expression *b) {
  if (is_kind_of<IntConstant>(a)) {
    if (!is_kind_of<IntConstant>(b)) { return(false); };
    return(to<IntConstant>(a)->get_value() ==
	   to<IntConstant>(b)->get_value());
  }
  if (is_kind_of<LoadVariableExpression>(a)) {
    if (!is_kind_of<LoadVariableExpression>(b)) { return(false); };
    return(to<LoadVariableExpression>(a)->get_source() ==
	   to<LoadVariableExpression>(b)->get_source());
  }
  return(a == b);
}

ArrayType* TypeBuilder::get_array_type(
                          QualifiedType* element_type,
                          const IInteger &lower_bound,
                          const IInteger &upper_bound) {
  Expression *lower = create_int_constant( _suif_env, get_integer_type(true) , lower_bound );
  Expression *upper = create_int_constant( _suif_env, get_integer_type(true), upper_bound );
  return get_array_type( element_type, lower, upper );
}


static IInteger get_int_constant(Expression *expr) {
  if (is_kind_of<IntConstant>(expr)) {
    return(to<IntConstant>(expr)->get_value());
  }
  IInteger ii;
  return(ii);
}

ArrayType* TypeBuilder::get_array_type(
                         QualifiedType* element_type,
			 Expression *lower_bound,
			 Expression *upper_bound) {
    DataType *etype = unqualify_data_type(element_type);
    IInteger ilb = get_int_constant(lower_bound);
    IInteger iub = get_int_constant(upper_bound);
    IInteger size_in_bits = (iub - ilb + 1) * etype->get_bit_size();
    return get_array_type(	size_in_bits,
				etype->get_bit_alignment(),
				element_type,
				lower_bound,
				upper_bound);
    }

ArrayType* TypeBuilder::get_array_type( IInteger size_in_bits,
                         int alignment_in_bits,
					QualifiedType* element_type,
                          Expression *lower_bound,
                          Expression *upper_bound) {
    ArrayType *element;
    SymbolTable *symbolTable = element_type->get_symbol_table();
    suif_assert_message(symbolTable, ("reference type not attached"));
    Iter<SymbolTableObject *> iter = symbolTable->get_symbol_table_object_iterator();

    while (iter.is_valid()) {
      	SymbolTableObject *obj = iter.current();

      	if (is_a<ArrayType>(obj)) {

            element = to<ArrayType>(obj);

            if ( element->get_bit_size() == size_in_bits
                && element->get_bit_alignment() == alignment_in_bits
                && element->get_element_type() == element_type
                && is_equal_expression(element->get_lower_bound(), lower_bound)
                && is_equal_expression(element->get_upper_bound(), upper_bound)) {
		delete lower_bound;
		delete upper_bound;
                return (element);
		}
	    }
	iter.next();
	}

    element = create_array_type(_suif_env, size_in_bits,
                                             alignment_in_bits,
                                             element_type,
                                             lower_bound,
                                             upper_bound);

    symbolTable->append_symbol_table_object(element);
    return (element);
    }

MultiDimArrayType* TypeBuilder::get_multi_dim_array_type(
                          IInteger size_in_bits,
                          int alignment_in_bits,
                          QualifiedType* element_type,
                          suif_vector<Expression *> &lower_bounds,
                          suif_vector<Expression *> &upper_bounds) {
    MultiDimArrayType *element;
    SymbolTable *symbolTable = element_type->get_symbol_table();
    suif_assert_message(symbolTable, ("multi dimension array element type not attached"));
    suif_assert_message(lower_bounds.size() == upper_bounds.size(),("inconsistent numbers of bounds in multi dimensioned array"));
    int bounds =  upper_bounds.size();
    Iter<SymbolTableObject *> iter = symbolTable->get_symbol_table_object_iterator();

    while (iter.is_valid()) {
        SymbolTableObject *obj = iter.current();

        if (is_a<MultiDimArrayType>(obj)) {

            element = to<MultiDimArrayType>(obj);

            if ( element->get_bit_size() == size_in_bits
                && element->get_bit_alignment() == alignment_in_bits
                && element->get_element_type() == element_type
		&& lower_bounds.size() == (unsigned)element->get_lower_bound_count()
		&& upper_bounds.size() == (unsigned)element->get_upper_bound_count()) {
		int i = 0;
		while ((i < bounds)
			 && is_equal_expression(element->get_lower_bound(i), lower_bounds[i])
                	 && is_equal_expression(element->get_upper_bound(i), upper_bounds[i]))
		    i++;
		if (i >= bounds) {
		    for (i=0;i < bounds;i++) {
			delete lower_bounds[i];
			delete upper_bounds[i];
			}
                    return (element);
		    }
		}
            }
        iter.next();
        }

    element = create_multi_dim_array_type(_suif_env, size_in_bits,
                                             alignment_in_bits,
                                             element_type);
    for (int i = 0;i < bounds;i ++) {
	element->append_lower_bound(lower_bounds[i]);
	element->append_upper_bound(upper_bounds[i]);
	}

    symbolTable->append_symbol_table_object(element);
    return (element);
    }

MultiDimArrayType* TypeBuilder::get_multi_dim_array_type(
                          QualifiedType* element_type,
                          suif_vector<Expression *> &lower_bounds,
                          suif_vector<Expression *> &upper_bounds) {
    DataType *etype = unqualify_data_type(element_type);
    IInteger size_in_bits = etype->get_bit_size();
    for (unsigned i =0;i < lower_bounds.size();i++) {
    	IInteger ilb = get_int_constant(lower_bounds[i]);
    	IInteger iub = get_int_constant(upper_bounds[i]);
    	size_in_bits = (iub - ilb + 1) * size_in_bits;
	}

    return get_multi_dim_array_type(size_in_bits,etype->get_bit_alignment(),element_type,lower_bounds,upper_bounds);
    }

static bool same_qualifiers(QualifiedType *element,list<LString> &qualifiers)
    {
    if ((unsigned)element->get_qualification_count() != qualifiers.length())
	return false;
    list<LString>::iterator iter = qualifiers.begin();
    while (iter != qualifiers.end()) {
	if (!element->has_qualification_member(*iter))
	    return false;
	iter ++;
	}
    return true;
    }

QualifiedType* TypeBuilder::get_qualified_type( DataType *base_type,
				   list<LString> qualifiers) {

    QualifiedType *element;

    // If we are qualifiying an already qualified type, we need to copy the
    // qualifiers of the existing type. Note that the new qualifiers are added
    // to the front of the list

    while (is_a<QualifiedType>(base_type)) {
	QualifiedType *qt = to<QualifiedType>(base_type);
	Iter<LString> qualIter = qt->get_qualification_iterator();
 	while (qualIter.is_valid()) {
            qualifiers.push_back(qualIter.current());
	    qualIter.next();
	    }
	base_type = qt->get_base_type();
        }

    SymbolTable *symbolTable = base_type->get_symbol_table();
    suif_assert_message(symbolTable != 0, 
			("Base type not attached to a symbol table"));
    Iter<SymbolTableObject *> iter = symbolTable->get_symbol_table_object_iterator();


    while (iter.is_valid()) {

      	SymbolTableObject *obj = iter.current();

      	if (is_a<QualifiedType>(obj)) {
	    element = to<QualifiedType>(obj);
	    if ((element->get_base_type() == base_type)
	      && same_qualifiers(element,qualifiers))
		return element;
	    }
	iter.next();
	}

    element = create_qualified_type(_suif_env,base_type);

    list<LString>::iterator it = qualifiers.begin();

    while (it != qualifiers.end()) {
        element->append_qualification(*it);
	it++;
        }

    symbolTable->append_symbol_table_object(element);
    return (element);
    }

QualifiedType* TypeBuilder::get_qualified_type( DataType *base_type,
                                   const LString &qualifier) {
    list<LString> qualifiers;
    qualifiers.push_back(qualifier);
    return get_qualified_type(base_type,qualifiers);
    }

QualifiedType* TypeBuilder::get_qualified_type( QualifiedType *base_type,
						const LString &qualifier) {
    list<LString> qualifiers;
    qualifiers.push_back(qualifier);
    return(get_qualified_type(base_type, qualifiers));
}

QualifiedType* TypeBuilder::get_qualified_type( QualifiedType *qt,
						list<LString> qualifiers) {
    list<LString> all_qualifiers;
    
    for (Iter<LString> qualIter = qt->get_qualification_iterator();
	 qualIter.is_valid(); qualIter.next()) {
      all_qualifiers.push_back(qualIter.current());
    }
    for (list<LString>::iterator iter = qualifiers.begin();
	 iter != qualifiers.end(); iter++) {
      LString qual = *iter;
      if (!qt->has_qualification_member(qual)) {
	all_qualifiers.push_back(qual);
      }
    }
    return(get_qualified_type(qt->get_base_type(), all_qualifiers));
}


QualifiedType* TypeBuilder::get_qualified_type( Type *base_type) {
    if (is_kind_of<QualifiedType>(base_type)) {
	return to<QualifiedType>(base_type);
	}
    list<LString> qualifiers;
    return get_qualified_type(to<DataType>(base_type),qualifiers);
    }

QualifiedType* TypeBuilder::get_qualified_type( DataType *base_type) {
    list<LString> qualifiers;
    return get_qualified_type(base_type,qualifiers);
    }


LabelType* TypeBuilder::get_label_type() {
    FileSetBlock *block = _suif_env->get_file_set_block();
    suif_assert_message(block, ("file set block not attached to suifenv"));
    SymbolTable *symbolTable = block->get_external_symbol_table();
    suif_assert_message(symbolTable, ("external_symbol table not attached"));
    for (Iter<SymbolTableObject *> iter =
	   symbolTable->get_symbol_table_object_iterator();
	 iter.is_valid(); iter.next()) {
        SymbolTableObject *obj = iter.current();
        if (is_a<LabelType>(obj)) {
	  LabelType *element = to<LabelType>(obj);
	  return element;
	}
    }
    LabelType * element = create_label_type(_suif_env);
    symbolTable->append_symbol_table_object(element);
    return (element);
  
    }

bool TypeBuilder::is_ancestor_of(SuifObject *parent, SuifObject *child) {
    while ((child != NULL) && (child != parent)) {
	child = child->get_parent();
	}
    return (child == parent);
    }


//	Given two symbol tables, return the one that is least nested.
//	Return NULL if they are not related and no common parent exists
SymbolTable * TypeBuilder::most_nested_common_scope(SymbolTable *newtable,
						    SymbolTable *table) {
    if (is_ancestor_of(table,newtable))
      return table;
    if (is_ancestor_of(newtable,table))
      return newtable;

    SymbolTable *ptable = newtable;
    while (ptable != NULL) {
	   ptable = ptable->get_explicit_super_scope();
      if (is_ancestor_of(ptable,table))
	    return newtable;
	}
    ptable = table;
    while (ptable != NULL) {
        ptable = ptable->get_explicit_super_scope();
        if (is_ancestor_of(newtable,ptable))
            return ptable;
        }
    return NULL;
    }

SymbolTable * 
TypeBuilder::most_nested_common_scope(list<QualifiedType *> &tlist,
				      SymbolTable *table) {
    list<QualifiedType *>::iterator iter = tlist.begin();
    if (iter == tlist.end())
	return table;
    if (table == NULL)
	{
	table = to<SymbolTable>((*iter)->get_parent());
	iter ++;
	}
    while (iter != tlist.end()) {
	table = most_nested_common_scope(
			to<SymbolTable>( (*iter)->get_parent() ),
			table);
	if (table == NULL)
	    return NULL;
     iter++;
	}
    return table;
    }

bool TypeBuilder::is_argument_types_match(list<QualifiedType *>&argument_list,
					  CProcedureType *typ)
    {
    list<QualifiedType *>::iterator it = argument_list.begin();
    Iter<QualifiedType *> it2 = typ->get_argument_iterator();

    while ((it != argument_list.end()) && it2.is_valid()) {
        if (it2.current() != (*it))
            return false;
        it2.next();
        it ++;
        }
    return ((it == argument_list.end()) && !it2.is_valid());
    }

CProcedureType* TypeBuilder::get_c_procedure_type(
						  DataType * result_type,
					      list<QualifiedType *> argument_list,
					      bool has_varargs,
					      bool arguments_known,
					      int bit_alignment) {
    if (bit_alignment == 0) {
      // Only works for THIS machine.
      // We need to get this from the c_target_info block
      bit_alignment = sizeof(int)*8;
    }
    FileSetBlock *block = _suif_env->get_file_set_block();
    SymbolTable *table = block->get_external_symbol_table();
    table = most_nested_common_scope(
		to<SymbolTable>(result_type->get_parent()),
		table);
//    suif_assert(table);
    table = most_nested_common_scope(argument_list,table);
//    suif_assert(table);

    // Couldn't find a symbol table that contains the intersection
    // of all the arguments, so use the global symbol table
    if (!table) {
       table = _suif_env->get_file_set_block()->get_external_symbol_table();
    }

    // Search the table
    Iter<SymbolTableObject *> iter = table->get_symbol_table_object_iterator();
    CProcedureType *type;

    while (iter.is_valid()) {
      	SymbolTableObject *obj = iter.current();
      	if (is_a<CProcedureType>(obj)) {

      	    type = to<CProcedureType>(obj);

      	    if (((unsigned)type->get_argument_count() == 
		 argument_list.length())
  	      &&  (type->get_has_varargs() == has_varargs)
  	      &&  (type->get_arguments_known() == arguments_known)
  	      &&  (type->get_bit_alignment() == bit_alignment)
	      &&  (type->get_result_type() == result_type)
	      &&  is_argument_types_match(argument_list,type))
		return type;
	    }
  	   iter.next();
    }

    type = create_c_procedure_type(_suif_env,result_type,has_varargs, arguments_known, bit_alignment);
    table->append_symbol_table_object( type );
    list<QualifiedType *>::iterator it = argument_list.begin();

    while (it != argument_list.end()) {
	type->append_argument(*it);
	it ++;
	}

    return type;
    }


TypeBuilder* get_type_builder(SuifEnv *_suif)
{
  return ((TypeBuilder *)
	  (_suif->get_object_factory(TypeBuilder::get_class_name())));
}

VoidType *get_void_type(
  SuifEnv *_suif)

{
  return (get_type_builder(_suif)->get_void_type());
}

BooleanType *get_boolean_type(
  SuifEnv *_suif,
  IInteger size_in_bits,
  int alignment_in_bits)

{
  return (get_type_builder(_suif)->get_boolean_type(size_in_bits,
					   alignment_in_bits));
}


IntegerType *get_integer_type(
  SuifEnv *_suif,
  IInteger size_in_bits,
  int alignment_in_bits,
  bool is_signed )

{
  return (get_type_builder(_suif)->get_integer_type(size_in_bits,
					   alignment_in_bits,
					   is_signed));
}

IntegerType* get_integer_type( SuifEnv * _suif,
			       bool is_signed)
{
  return (get_type_builder(_suif)->get_integer_type(is_signed));
}


FloatingPointType *get_floating_point_type(
  SuifEnv *_suif,
  IInteger size_in_bits,
  int alignment_in_bits)

{
  return (get_type_builder(_suif)->get_floating_point_type(size_in_bits,
						  alignment_in_bits));
}

FloatingPointType* get_floating_point_type( SuifEnv * _suif)
{
  return (get_type_builder(_suif)->get_floating_point_type());
}


FloatingPointType* get_double_floating_point_type( SuifEnv * _suif)
{
  return (get_type_builder(_suif)->get_double_floating_point_type());
}



PointerType *get_pointer_type(
  SuifEnv *_suif,
  IInteger size_in_bits,
  int alignment_in_bits,
  Type *reference_type)

{
  return (get_type_builder(_suif)->get_pointer_type(size_in_bits,
					   alignment_in_bits,
					   reference_type));
}

PointerType *get_pointer_type(SuifEnv *_suif,
			      Type *reference_type)

{
  return (get_type_builder(_suif)->get_pointer_type(
					   reference_type));
}





ArrayType *get_array_type(
			  SuifEnv *_suif,
			  IInteger size_in_bits,
			  int alignment_in_bits,
			  QualifiedType *reference_type,
			  Expression *lower_bound,
			  Expression *upper_bound) {

  return (get_type_builder(_suif)->get_array_type(size_in_bits,
					 alignment_in_bits,
					 reference_type,
					 lower_bound,
					 upper_bound));
}



ArrayType *get_array_type(
			  SuifEnv *_suif,
			  QualifiedType *reference_type,
                          Expression *lower_bound,
                          Expression *upper_bound) {
  return (get_type_builder(_suif)->get_array_type(
					 reference_type,
					 lower_bound,
					 upper_bound));
}


ArrayType *get_array_type(
			  SuifEnv *_suif_env,
			  QualifiedType *reference_type,
                          const IInteger &lower_bound,
                          const IInteger &upper_bound) {
  Expression *lower = create_int_constant( _suif_env, get_integer_type(_suif_env,true), lower_bound );
  Expression *upper = create_int_constant( _suif_env, get_integer_type(_suif_env,true), upper_bound );
  return (get_type_builder(_suif_env)->get_array_type(
					 reference_type,
					 lower,
					 upper ));
}


QualifiedType* get_qualified_type(
  SuifEnv *_suif,
  DataType *base_type,
  LString qualifier) {
  list<LString> qualifiers;
  qualifiers.push_back( qualifier );
  return (get_type_builder(_suif)->get_qualified_type(base_type, qualifiers));
}

QualifiedType *get_qualified_type(
  SuifEnv *_suif,
  DataType *base_type,
  list<LString> qualifiers)

{

  return (get_type_builder(_suif)->get_qualified_type(base_type, qualifiers));
}

CProcedureType *get_c_procedure_type(
  SuifEnv *_suif,
  DataType * result_type,
  list<QualifiedType *>& argument_list,
  bool has_varargs,
  bool arguments_known,
  int bit_alignment)

{
  return (get_type_builder(_suif)->get_c_procedure_type(result_type,
						   argument_list,
						   has_varargs,
						   arguments_known,
						   bit_alignment));
}


MultiDimArrayType* get_multi_dim_array_type( SuifEnv* _suif,
                          QualifiedType* element_type,
			  suif_vector<Expression *> &lower_bounds,
			  suif_vector<Expression *> &upper_bounds)
{
  return (get_type_builder(_suif)->get_multi_dim_array_type(element_type,
							    lower_bounds,
							    upper_bounds));
}


GlobalInformationBlock* get_global_information_block( SuifEnv* suif_env, const LString& name ) {
  GlobalInformationBlock* block = 0;
  FileSetBlock *file_set_block = suif_env->get_file_set_block();
  if ( file_set_block ) {
    for ( Iter<GlobalInformationBlock*> iter = file_set_block->get_information_block_iterator();
            iter.is_valid();
            iter.next() ) {
        GlobalInformationBlock* current_block = iter.current();

        if ( current_block->getClassName() == name ) {
           block = current_block;
           break;
        }
      }
    }
    return block;
}

TargetInformationBlock *find_target_information_block(SuifEnv *env) {
    return to<TargetInformationBlock>(get_global_information_block( env, TargetInformationBlock::get_class_name() ) );
 }


BooleanType * TypeBuilder::get_boolean_type() {
    TargetInformationBlock *tib = find_target_information_block(_suif_env);
    BooleanType *type = NULL;
    if (tib != NULL) {
	type = tib->get_default_boolean_type();
	}

    if (type == NULL) {
	type = get_boolean_type(sizeof(bool)*8,sizeof(bool)*8);
	}
    return type;
    }

IntegerType *TypeBuilder::get_integer_type(bool is_signed) {
    TargetInformationBlock *tib = find_target_information_block(_suif_env);
    IntegerType *type = NULL;
    if (tib != NULL) {
        type = tib->get_word_type();
        }

    if (type == NULL) {
        type = get_integer_type(sizeof(int)*8,sizeof(int)*8);
        }
    return type;
    }

FloatingPointType *TypeBuilder::get_floating_point_type() {
    return get_floating_point_type(sizeof(float)*8,sizeof(float)*8);
    }

FloatingPointType *TypeBuilder::get_double_floating_point_type() {

    return get_floating_point_type(sizeof(double)*8,sizeof(double)*8);
    }

PointerType* TypeBuilder::get_pointer_type(Type* reference_type)
    {
    return get_pointer_type(sizeof(void *)*8,sizeof(void *)*8,reference_type);
    }

ReferenceType* TypeBuilder::get_reference_type(Type* reference_type)
    {
    return get_reference_type(sizeof(void *)*8,sizeof(void *)*8,reference_type);
    }


FieldSymbol * TypeBuilder::add_symbol_to_group(
                        GroupType *group,
                        const LString &symbol_name,
                        const QualifiedType *q_symbol_type) {
    IInteger size = group->get_bit_size();
    int align = group->get_bit_alignment();
    DataType *symbol_type = q_symbol_type->get_base_type();
    IInteger sym_size = symbol_type->get_bit_size();
    int sym_align = symbol_type->get_bit_alignment();
    if (align < sym_align)
	group->set_bit_alignment(sym_align);
    if (sym_align > 0) {
	IInteger inc = size % sym_align;
    	int i_inc = inc.c_int();
        if (i_inc > 0)
	    size = size + (sym_align - i_inc);
	}
    Expression *offset =  create_int_constant(_suif_env,get_integer_type(false),size);
    FieldSymbol *fsym =
      create_field_symbol(_suif_env, const_cast<QualifiedType*>(q_symbol_type),
			  offset, symbol_name);

    group->get_group_symbol_table()->add_symbol(fsym);
    size = size + sym_size;
    group->set_bit_size(size);
    return fsym;
    }

FieldSymbol * TypeBuilder::add_union_symbol_to_group(
                        GroupType *group,
                        const LString &symbol_name,
                        const QualifiedType *symbol_type,
			int pos) {
    int align = group->get_bit_alignment();
    IInteger sym_size = symbol_type->get_base_type()->get_bit_size();
    int sym_align = symbol_type->get_base_type()->get_bit_alignment();
    if (align < sym_align)
        group->set_bit_alignment(sym_align);
    Expression *offset =  create_int_constant(_suif_env,get_integer_type(false),0);
    FieldSymbol *fsym =
      create_field_symbol(_suif_env,
			  const_cast<QualifiedType*>(symbol_type),
			  offset,
			  symbol_name);

    if (pos >= 0)
	group->get_group_symbol_table()->insert_symbol_table_object(pos,fsym);
    else
      	group->get_group_symbol_table()->add_symbol(fsym);
    IInteger size = group->get_bit_size();
    if (size < sym_size)
	group->set_bit_size(sym_size);
    return fsym;
    }

static IInteger get_bound(Expression *exp) {
    if (is_kind_of<IntConstant>(exp))
	return to<IntConstant>(exp)->get_value();
    return IInteger(); // undetermined
    }

void TypeBuilder::get_array_bounds(Type *type,size_t bound,IInteger &low,IInteger &high) {
    if (is_kind_of<QualifiedType>(type))
	type = to<QualifiedType>(type)->get_base_type();
    if (is_kind_of<ArrayType>(type)) {
	ArrayType *atype = to<ArrayType>(type);
	suif_assert_message(bound == 0, ("bound in get_array_bounds not zero for ArrayType"));
	
	low = get_bound(atype->get_lower_bound());
	high = get_bound(atype->get_upper_bound());
	return;
	}
    if (is_kind_of<MultiDimArrayType>(type)) {
	MultiDimArrayType *atype = to<MultiDimArrayType>(type);
        suif_assert_message(((bound >= 0) && (bound < atype->get_lower_bound_count())), ("bound in get_array_bounds out of range for MultiDimArrayType"));

        low = get_bound(atype->get_lower_bound(bound));
        high = get_bound(atype->get_upper_bound(bound));
        return;
        }
    suif_assert_message(0,("get_array_bounds called for non_array type"));
    }

inline Type *get_data_type(VariableSymbol *sym) {
#ifdef NO_COVARIANCE
    return to<QualifiedType>(sym->get_type())->get_base_type();
#else
    return sym->get_type()->get_base_type();
#endif
    }

  // find the field containing the offset
  // In the case of unions, the first union containing the
  // field is returned
FieldSymbol * TypeBuilder::find_symbol_containing_offset(
                        GroupType *group,
                        IInteger offset) {
    GroupSymbolTable *gst = group->get_group_symbol_table();
    int field_no = 0;
    int field_count = gst->get_symbol_table_object_count();
    while (field_no < field_count) {
        FieldSymbol *fsym = to<FieldSymbol>(
            gst->get_symbol_table_object(field_no));
	
        DataType *ftype = to<DataType>(get_data_type(fsym));
	IInteger position = 
		to<IntConstant>(fsym->get_bit_offset())->get_value();
	IInteger size = ftype->get_bit_size();
	if ((position <= offset) && ((position + size ) > offset))
	    return fsym;
	field_no ++;
        }
    return 0;
    }

  // as above, but groups are looked through until a suitable
  // non-group field is found.
FieldSymbol * TypeBuilder::find_non_group_symbol_containing_offset(
                        GroupType *group,
                        IInteger offset) {
    FieldSymbol *field = find_symbol_containing_offset(group,offset);
    if (!field)
	return 0;
    while (is_kind_of<GroupType>(get_data_type(field))) {
	offset -= to<IntConstant>(field->get_bit_offset())->get_value();
	field = find_symbol_containing_offset(
		to<GroupType>(get_data_type(field)),offset);
	if (!field)
	    return 0;
	}
    return field;
    }

  // returns the type of the non_group_symbol, but arrays are
  // also looked through to find a non-group, non-array type
DataType * TypeBuilder::find_type_of_field_containing_offset(
                        GroupType *group,
                        IInteger offset) {
    FieldSymbol *field = find_non_group_symbol_containing_offset(group,offset);
    Type *ftype = get_data_type(field);
    while (is_kind_of<ArrayType>(ftype))
	ftype = to<ArrayType>(ftype)->get_element_type()->get_base_type();
    return to<DataType>(ftype);
    }

FieldSymbol * TypeBuilder::find_field_of_type(
                        GroupType *group,
                        const Type *field_type) {
    UnionType *utype = to<UnionType>(group);
    GroupSymbolTable * gst = utype->get_group_symbol_table();
    int field_no = 0;
    int field_count = gst->get_symbol_table_object_count();
    while (field_no < field_count) {
        FieldSymbol *field = to<FieldSymbol>(gst->get_symbol_table_object(field_no));
#ifdef NO_COVARIANCE
	QualifiedType *ftype = to<QualifiedType>(field->get_type());
#else
        QualifiedType *ftype = field->get_type();
#endif
        if ((field_type == ftype) || (field_type == ftype->get_base_type()))
             return field;

        field_no ++;
        }
    return 0;
    }

static bool matching_group(	GroupType *type,
				list<LString>&names,
        			list<QualifiedType *> &type_list) {
    GroupSymbolTable *gst = type->get_group_symbol_table();
    size_t field_no = 0;
    size_t field_count = gst->get_symbol_table_object_count();

    // compare length to length of type list.
    // NB - not names which are optional
    if (field_count != type_list.length())
	return false;
    while (field_no < field_count) {
	FieldSymbol *field = to<FieldSymbol>(gst->get_symbol_table_object(field_no));
	if (field->get_type() != type_list[field_no])
	    return false;
	if (names.length() > field_no) {
	    LString fname = field->get_name();
	    if ((names[field_no] != emptyLString) && (names[field_no] != fname))
		return false;
	    }
	field_no ++;
	}
    return true;
    }
	

static bool is_outer_scope(SymbolTable *outer,SymbolTable *inner) {
    while ((outer != inner) && inner) {
	inner = inner->get_explicit_super_scope();
	}
    return (inner != 0);
    }

GroupType *TypeBuilder::get_group_type(
	list<SymbolTable*>symbol_tables,
	list<LString>&names,
	list<QualifiedType *> &type_list) {
    size_t i;
    for (i = 0;i < symbol_tables.length();i++) {
        SymbolTable *symbol_table = symbol_tables[i];
    	Iter<SymbolTableObject *> iter = symbol_table->get_symbol_table_object_iterator();
        while (iter.is_valid()) {
            SymbolTableObject *obj = iter.current();
            if (is_a<GroupType>(obj)) {
                GroupType *group = to<GroupType>(obj);
		if (group->get_is_complete() && matching_group(group,names,type_list))
		return group;
                }
            iter.next();
            }
	}
    // no existing type, so build one. First find appropriate symbol table
    i = 0;
    while (i < symbol_tables.length() - 1) {
	size_t j = 0;
	while (j < type_list.length()) {
	    if (!is_outer_scope(symbol_tables[i],to<SymbolTable>(type_list[j]->get_parent())))
	        break;
	    j ++;
	    }
	if (j < type_list.length())
	    break;
	i++;
	}
    SymbolTable *target_table = symbol_tables[i];
    GroupType *gtype = create_group_type(_suif_env,0,0,emptyLString,true);
    for (i = 0;i < type_list.length(); i++) {
	LString name = emptyLString;
	if (names.length() > i)
	    name = names[i];
	add_symbol_to_group(gtype,name,type_list[i]);
	}
    target_table->append_symbol_table_object(gtype);
    return gtype;
    }

UnionType *TypeBuilder::get_union_type(
        list<SymbolTable*>symbol_tables,
        list<LString>&names,
        list<QualifiedType *> &type_list) {
    size_t i;
    for (i = 0;i < symbol_tables.length();i++) {
        SymbolTable *symbol_table = symbol_tables[i];
        Iter<SymbolTableObject *> iter = symbol_table->get_symbol_table_object_iterator();
        while (iter.is_valid()) {
            SymbolTableObject *obj = iter.current();
            if (is_a<UnionType>(obj)) {
                UnionType *group = to<UnionType>(obj);
                if (group->get_is_complete() && matching_group(group,names,type_list))
                return group;
                }
            iter.next();
            }
        }
    // no existing type, so build one. First find appropriate symbol table
    i = 0;
    while (i < symbol_tables.length() - 1) {
        size_t j = 0;
        while (j < type_list.length()) {
            if (!is_outer_scope(symbol_tables[i],to<SymbolTable>(type_list[j]->get_parent())))
                break;
            j ++;
            }
        if (j < type_list.length())
            break;
        i++;
        }
    SymbolTable *target_table = symbol_tables[i];
    UnionType *gtype = create_union_type(_suif_env,0,0,emptyLString,true);
    for (i = 0;i < type_list.length(); i++) {
        LString name = emptyLString;
        if (names.length() > i)
            name = names[i];
        add_union_symbol_to_group(gtype,name,type_list[i]);
        }
    target_table->append_symbol_table_object(gtype);
    return gtype;
    }


