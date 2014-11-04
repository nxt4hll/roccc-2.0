#ifndef _PGEN_NODE_BUILDER_H_
#define _PGEN_NODE_BUILDER_H_
/**
  * The NodeBuilder class is a utility class for building IR nodes.
  *
  * The constructor takes in a SymbolTable.  This symbol table represents the
  * environment of the new IR nodes.
  * New SymbolTableObjects will be automatically owned by this symbol table.
  * 
  * All lookup_ method will return a null if no matching object is found,
  * unless otherwise stated.
  */

#include "common/lstring.h"
#include "basicnodes/basic.h"
#include "suifnodes/suif.h"
#include "cfenodes/cfe.h"



class NodeBuilder {
 private:
  SymbolTable* _symtab;
  SuifEnv*     _suif_env;
  TypeBuilder* _type_builder;  

 protected:
  Expression* get_field_offset_exp(FieldSymbol* f);
  bool        is_same_type(Type*, Type*);
  void        trash(SuifObject* );
  MultiValueBlock* new_cstr_value_block(String s);
  VariableDefinition* add_variable_definition(ProcedureDefinition* p,
					      VariableSymbol* v,
					      ValueBlock* b);

 public:

  NodeBuilder(SymbolTable*);
  NodeBuilder(ScopedObject*);

  SuifEnv *get_suif_env() const;
  ParameterSymbol* new_param(LString name, QualifiedType* type);

  VariableSymbol* new_int_var(LString name, bool addr_taken = false);
  VariableSymbol* new_var(LString name, DataType* type,
			  bool addr_taken = false);
  VariableSymbol* new_var(LString name, QualifiedType* type,
			  bool addr_taken = false);
  VariableSymbol* lookup_var(LString name, bool local_only = false);

  Type*           lookup_type(LString, bool local_only = false);
  StructType*     new_struct_type(LString);
  void            add_struct_field(StructType*,
			       LString fname,
			       QualifiedType* ftype);
  IntegerType*       get_int_type(void);
  BooleanType*       get_bool_type(void);
  FloatingPointType* get_float_type(void);
  IntegerType*       get_char_type(void);
  VoidType*          get_void_type(void);

  QualifiedType*     get_qualified_type(DataType*);

  /** Strip away all QualifiedType layer.
    * If \a t is not a QualifiedType, return t;
    */
  Type*              get_unqualified_type(Type* t);

  /** if \a t is a pointer type, return its reference_type.
    * Otherwise return 0.
    */
  Type*                    get_pointed_to_type(Type* t);

  PointerType*             get_pointer_type(Type* t);

  StoreVariableStatement*  store_var(VariableSymbol*, Expression*);
  StoreStatement*          store(Expression* target_addr, Expression*);
  StoreStatement*          store_field(Expression* group_addr,
				       LString field_name,
				       Expression* value);
  EvalStatement*           eval(Expression*);
  StatementList*           new_statement_list(void);
  IfStatement*             new_if(Expression*, Statement*, Statement*);

  LoadExpression*          load(Expression* addr);    // LoadExpression
  LoadVariableExpression*  load_var(VariableSymbol*); // LoadVarExpression
  LoadVariableExpression*  load_var(LString); // LoadVarExpression

  SymbolAddressExpression* sym_addr(Symbol*); // SymbolAddressExpression
  Expression*              load_field(Expression* group_addr,
				      LString field_name);

  FieldAccessExpression*   field_addr(Expression* grp_addr,
				      LString field_name);

  IntConstant*     int_const(IInteger);
  IntConstant*     int_const(int);
  IntConstant*     bool_const(bool);
  FloatConstant*   float_const(float);

  /** The expression returned has type array(char).
    */
  Expression*      cstr_const(const char*, ProcedureDefinition*);
  IntConstant*     char_const(char);

  BinaryExpression* binary_exp(LString op, DataType* result_type,
			       Expression* arg1, Expression* arg2);
  BinaryExpression* and_exp(Expression*, Expression*);
  BinaryExpression* multiply_exp(Expression*, Expression*);
  BinaryExpression* add_exp(Expression*, Expression*);
  BinaryExpression* subtract_exp(Expression*, Expression*);
  BinaryExpression* is_less_than_or_equal_to_exp(Expression*, Expression*);
  BinaryExpression* divfloor_exp(Expression*, Expression*);
  BinaryExpression* max_exp(Expression*, Expression*);
  BinaryExpression* min_exp(Expression*, Expression*);

  UnaryExpression* convert_exp(Expression*, DataType*);

  ProcedureSymbol*     lookup_proc(LString name, bool local_only = false);
  ProcedureDefinition* new_proc_defn1(LString name,
				      DataType* return_type,
				      ParameterSymbol* arg);

  /** Lookup or create if necessary a variable definition containint
    *  a C string.
    */
  VariableSymbol*      get_cstr_const_var(String s, ProcedureDefinition* p);

  void                 replace(Statement* old_stmt, Statement* new_stmt);
  void                 replace(Expression* old_exp, Expression* new_exp);


};




/** Retrieve and cast the reference type of a pointer type.
  * If \a t is not a point to \a T, return 0;
  *
  * Originally implemented as a method of NodeBuilder.
  * Now as a standalone function because the Visual C++ compiler
  *  cannot handle it.
  */
template<class T> T* to_ref_type(NodeBuilder* nb, Type* t) {
  Type* ut = nb->get_unqualified_type(t);
  if (!is_kind_of<PointerType>(ut)) return 0;
  Type* reftype =
    nb->get_unqualified_type(to<PointerType>(t)->get_reference_type());
  if (!is_kind_of<T>(reftype)) return 0;
  return to<T>(reftype);
};

#endif // _PGEN_NODE_BUILDER_H_
