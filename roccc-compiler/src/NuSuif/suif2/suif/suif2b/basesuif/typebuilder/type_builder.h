#ifndef TYPE_BUILDER__TYPE_BUILDER_H
#define TYPE_BUILDER__TYPE_BUILDER_H

#include "suifkernel/real_object_factory.h"
#include "basicnodes/basic_forwarders.h"
#include "basicnodes/basic.h"
#include "suifnodes/suif_forwarders.h"
#include "suifkernel/suifkernel_forwarders.h"
#include "common/i_integer.h"
#include "common/suif_vector.h"


/**
 * \class TypeBuilder type_builder.h
 *
 * 
 * Utility functions for building, retrieving, and manipulating types
 *
 * For each class of type, the get_type functions look for that type
 * in the appropriate symbol tables. If none is found, then the
 * type is created and inserted into the highest valid symbol table.
 *
 * For each class of type, there is generally a get_type
 * routine which takes all parameters and one which defaults
 * the size and alignment parameters. The defaults for these
 * are looked up in the TargetInformationBlock or, if no
 * target information block has been found, they are derived
 * from the types on the HOST machine. This will be bad if the
 * host machine and the target machine are not the same.
 *
 * The default size will be a size best suited to a variable in
 * memory.
 */

GlobalInformationBlock* get_global_information_block( SuifEnv* suif_env, const LString& name );
TargetInformationBlock *find_target_information_block(SuifEnv *env);

extern "C" void EXPORT init_typebuilder( SuifEnv *suif );


class TypeBuilder : public RealObjectFactory {
public:
  virtual void init(SuifEnv* suif);

  static const LString &get_class_name();
  virtual const LString &getName();

  DataType *unqualify_data_type(Type *t);
  Type* unqualify_type( Type* t );

  // ---------- unnamed types -----------
  //

  virtual VoidType* get_void_type();
  virtual BooleanType* get_boolean_type(
                          IInteger size_in_bits,
                          int alignment_in_bits );

  virtual BooleanType * get_boolean_type();

  virtual IntegerType* get_integer_type(
                          IInteger size_in_bits,
                          int alignment_in_bits,
                          bool is_signed = true );


  /**
   * Find a small integer type. This should be type char. This can be taken
   * as the smallest type creatable on the machine (ie, one addressing unit)
   * However, if a front end fails to generate this, you will get the smallest
   * type generated.
   */
  virtual IntegerType* get_smallest_integer_type();

  virtual IntegerType *get_integer_type(bool is_signed = true);

  virtual FloatingPointType* get_floating_point_type(
                          IInteger size_in_bits,
                          int alignment_in_bits );

  virtual FloatingPointType *get_floating_point_type();
  virtual FloatingPointType *get_double_floating_point_type();

  virtual PointerType* get_pointer_type(
                          IInteger size_in_bits,
                          int alignment_in_bits,
                          Type* reference_type);

   virtual PointerType* get_pointer_type(Type* reference_type);

  virtual ReferenceType* get_reference_type(
                          IInteger size_in_bits,
                          int alignment_in_bits,
                          Type* reference_type);

   virtual ReferenceType* get_reference_type(Type* reference_type);



  virtual ArrayType* get_array_type(
                          IInteger size_in_bits,
                          int alignment_in_bits,
                          QualifiedType* element_type,
			  Expression *lower_bound,
			  Expression *upper_bound);

  /**
   * array type with calculated size and alignment
   */
  virtual ArrayType* get_array_type(
                          QualifiedType* element_type,
			  const IInteger &lower_bound,
			  const IInteger &upper_bound);

  virtual ArrayType* get_array_type(
			  QualifiedType* element_type,
			  Expression *lower_bound,
			  Expression *upper_bound);

  virtual MultiDimArrayType* get_multi_dim_array_type(
                          IInteger size_in_bits,
                          int alignment_in_bits,
                          QualifiedType* element_type,
			  suif_vector<Expression *> &lower_bounds,
			  suif_vector<Expression *> &upper_bounds);

  virtual MultiDimArrayType* get_multi_dim_array_type(
                          QualifiedType* element_type,
			  suif_vector<Expression *> &lower_bounds,
			  suif_vector<Expression *> &upper_bounds);




  virtual QualifiedType* get_qualified_type(DataType *base_type,
					  list<LString> qualifiers
					  );

  virtual QualifiedType* get_qualified_type(DataType *base_type,
                                          const LString &qualifier
                                          );

  virtual QualifiedType* get_qualified_type(QualifiedType *base_type,
					    list<LString> qualifiers
					    );

  virtual QualifiedType* get_qualified_type(QualifiedType *base_type,
					    const LString &qualifier
					    );


  virtual QualifiedType* get_qualified_type(Type *base_type);

  virtual QualifiedType* get_qualified_type(DataType *base_type);

  virtual LabelType* get_label_type();

  virtual CProcedureType* get_c_procedure_type(
					       DataType * result_type,
                          list<QualifiedType *> argument_list,
                          bool has_varags = false,
                          bool arguments_known = true,
                          int bit_alignment = 0);


  /** 
   * get a group type, returning an existing group if it exists
   * the name list is may be empty or partial - missing names are empty
   * if a new group is created it is created in the first symbol table
   * enclosing all the types, or the last symbol table if none
   */
  virtual GroupType *get_group_type(
		list<SymbolTable*>symbol_tables,
		list<LString>&names,
		list<QualifiedType *> &type_list);

  /** 
   * get a union type, returning an existing union if it exists
   * the name list is may be empty or partial - missing names are empty
   * if a new union is created it is created in the first symbol table
   * enclosing all the types, or the last symbol table if none
   */
  virtual UnionType *get_union_type(
                list<SymbolTable*>symbol_tables,
                list<LString>&names,
                list<QualifiedType *> &type_list);


  virtual FieldSymbol * add_symbol_to_group(
			GroupType *group,
			const LString &symbol_name,
		 	const QualifiedType *symbol_type);

  /** 
   * Add a union to a union type
   * the last optional field positions the union. Useful when
   * the new union is the default (for initialization)
   */
  virtual FieldSymbol * add_union_symbol_to_group(
                        GroupType *group,
                        const LString &symbol_name,
                        const QualifiedType *symbol_type,
		 	int pos = -1);

  /**
   * Find a field of a given type. Useful when building unions to do
   * type breaking activities. The field_type can be either of a 
   * QualifiedType or a simple type. If it is a simple type, any
   * qualifications on the field symbol's type are ignored in matching
   */
  virtual FieldSymbol * find_field_of_type(
			GroupType *group,
			const Type *field_type);

  /** 
   * find the field containing the offset
   * In the case of unions, the first union containing the
   * field is returned
   */
  virtual FieldSymbol * find_symbol_containing_offset(
			GroupType *group,
			IInteger offset);

  /**
   * as above, but groups are looked through until a suitable
   * non-group field is found.
   */
  virtual FieldSymbol * find_non_group_symbol_containing_offset(
                        GroupType *group,
                        IInteger offset);

  /**
   * returns the type of the non_group_symbol, but arrays are
   * also looked through to find a non-group, non-array type
   */
  virtual DataType * find_type_of_field_containing_offset(
			GroupType *group,
			IInteger offset);


  /**
   *   Return true if the argument types in argument_list matches with that
   *          in typ.  Two types match if they are the same Type object.
   */
  static bool is_argument_types_match(list<QualifiedType *>&argument_list,
				      CProcedureType *typ);
  

  /**
   *  Given two symbol tables, return the one that is least nested.
   *  Return NULL if they are not related and no common parent exists
   */
  static SymbolTable * most_nested_common_scope(SymbolTable *newtable,
						SymbolTable *table);
  static SymbolTable * most_nested_common_scope(list<QualifiedType *> &tlist,
						SymbolTable *table);

  
  /*
   *      Return true iff ancestor is (the parent of)* child.
   */
  static bool is_ancestor_of(SuifObject *ancestor, SuifObject *child);

  /**
   * Get the bounds of an array of a multidimensional array
   * If the given bound is not an integer constant, or if the bound
   * is not determined, undetermined is returned
   *	bound is the number of the bound to get, 0 based from left to right
   */
  void get_array_bounds(Type *type,size_t bound,IInteger &low,IInteger &high);

};


TypeBuilder* get_type_builder( SuifEnv* suif_env );


//#ifdef COMPATIBILITY
//	Don't call these!
/**
 * !!! DON'T USE THIS !!
 */
VoidType * get_void_type(SuifEnv *);
/**
 * !!! DON'T USE THIS !!
 */
BooleanType* get_boolean_type( SuifEnv *,
                         IInteger size_in_bits,
                         int alignment_in_bits );
/**
 * !!! DON'T USE THIS !!
 */
IntegerType* get_integer_type( SuifEnv *,
                         IInteger size_in_bits,
                         int alignment_in_bits,
                         bool is_signed = true );

/**
 * !!! DON'T USE THIS !!
 */
IntegerType* get_integer_type( SuifEnv *,
                         bool is_signed = true);

/**
 * !!! DON'T USE THIS !!
 */
FloatingPointType* get_floating_point_type( SuifEnv *,
                         IInteger size_in_bits,
                         int alignment_in_bits );
/**
 * !!! DON'T USE THIS !!
 */
FloatingPointType* get_floating_point_type( SuifEnv * );

/**
 * !!! DON'T USE THIS !!
 */
FloatingPointType* get_double_floating_point_type( SuifEnv * );

/**
 * !!! DON'T USE THIS !!
 */
PointerType* get_pointer_type( SuifEnv *,
                         IInteger size_in_bits,
                         int alignment_in_bits,
                         Type* reference_type);

/**
 * !!! DON'T USE THIS !!
 */
PointerType* get_pointer_type( SuifEnv *,
                         Type* reference_type);

/**
 * !!! DON'T USE THIS !!
 */
ArrayType* get_array_type( SuifEnv *,
                         IInteger size_in_bits,
                         int alignment_in_bits,
			   QualifiedType* element_type,
			  Expression *lower_bound,
			  Expression *upper_bound);

/**
 * !!! DON'T USE THIS !!
 */
ArrayType* get_array_type( SuifEnv*,
			   QualifiedType* element_type,
                          const IInteger &lower_bound,
                          const IInteger &upper_bound );


/**
 * !!! DON'T USE THIS !!
 */
ArrayType *get_array_type(
			  SuifEnv *_suif,
			  QualifiedType *reference_type,
                          Expression *lower_bound,
                          Expression *upper_bound);


/**
 * !!! DON'T USE THIS !!
 */
QualifiedType* get_qualified_type( SuifEnv *,
				   DataType *base_type,
				   list<LString> qualifiers);

/**
 * !!! DON'T USE THIS !!
 */
QualifiedType* get_qualified_type(
  SuifEnv *_suif,
  DataType *base_type,
  LString qualifier);


/**
 * !!! DON'T USE THIS !!
 */
CProcedureType* get_c_procedure_type( SuifEnv *,
				      DataType* result,
				      list<QualifiedType *>& argument_list,
				      bool has_varags = false,
				      bool arguments_known = true ,
				      int bit_alignment = 0);


/**
 * !!! DON'T USE THIS !!
 */
MultiDimArrayType* get_multi_dim_array_type( SuifEnv*,
                          QualifiedType* element_type,
			  suif_vector<Expression *> &lower_bounds,
			  suif_vector<Expression *> &upper_bounds);

    
//#endif

//extern bool equalExpression(Expression *a, Expression *b);



#endif
