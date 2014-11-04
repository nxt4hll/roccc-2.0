#include "node_builder.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif_factory.h"
#include "cfenodes/cfe_factory.h"
#include "typebuilder/type_builder.h"
#include "utils/symbol_utils.h"
#include "utils/type_utils.h"
#include "utils/expression_utils.h"
#include "utils/print_utils.h"
#include "utils/trash_utils.h"


SuifEnv *NodeBuilder::get_suif_env() const {
  return(_suif_env);
}

/** Create an expression for byte offset of a group field.
  *
  */
Expression* NodeBuilder::get_field_offset_exp(FieldSymbol* fsym)
{
  Expression *boffset = fsym->get_bit_offset();
  if (!is_kind_of<IntConstant>(boffset)) {
    trash(fsym);
    SUIF_THROW(SuifException(String("Field offset not in IntConstant ") +
			     to_id_string(fsym)));
  }
  IInteger v = to<IntConstant>(boffset)->get_value();
  return int_const(v.div(IInteger(BITSPERBYTE)));
}


/** Decide if the two types are the same modular qualification.
  */
bool NodeBuilder::is_same_type(Type* t1, Type* t2)
{
  return (TypeHelper::is_isomorphic_type(get_unqualified_type(t1),
					 get_unqualified_type(t2)));
}

/**
  * put into trash can only if garbage has no parent.
  */
void NodeBuilder::trash(SuifObject* garbage) {
  if (garbage->get_parent() == 0)
    ::trash_it(_suif_env, garbage);
}



MultiValueBlock* NodeBuilder::new_cstr_value_block(String s)
{
  ArrayType* type =
    _type_builder->get_array_type(get_qualified_type(get_char_type()),
				  IInteger(0),
				  IInteger(s.length()+1));
  MultiValueBlock* mb = create_multi_value_block(_suif_env, type);
  IInteger charlen = get_char_type()->get_bit_size();
  IInteger offset(0);
  for (int i =0; i<s.length(); i++) {
    mb->add_sub_block(offset,
		      create_expression_value_block(_suif_env,
						    char_const(s[i])));
    offset += charlen;
  }
  mb->add_sub_block(offset,
		    create_expression_value_block(_suif_env,
						  char_const('\0')));
  return mb;
}

 

VariableDefinition*
NodeBuilder::add_variable_definition(ProcedureDefinition* p,
				     VariableSymbol* v,
				     ValueBlock* b)
{
  VariableDefinition* vdef = create_variable_definition(_suif_env, v, 0, b);
  p->get_definition_block()->append_variable_definition(vdef);
  return vdef;
}











NodeBuilder::NodeBuilder(SymbolTable* symtab) :
  _symtab(symtab),
  _suif_env(symtab->get_suif_env()),
  _type_builder(get_type_builder(_suif_env))
{}

NodeBuilder::NodeBuilder(ScopedObject* sobj) :
  _symtab(find_scope(sobj)),
  _suif_env(sobj->get_suif_env()),
  _type_builder(get_type_builder(_suif_env))
{}




ParameterSymbol* NodeBuilder::new_param(LString name, QualifiedType* type)
{
  ParameterSymbol* par = create_parameter_symbol(_suif_env,
						 type,
						 name);
  _symtab->add_symbol(par);
  return par;
}


VariableSymbol* NodeBuilder::new_int_var(LString name, bool addr_taken)
{
  return new_var(name, get_int_type());
}


VariableSymbol* NodeBuilder::new_var(LString name,
				     DataType* dtype,
				     bool addr_taken)
{
  VariableSymbol* sym = create_variable_symbol(_suif_env,
					       get_qualified_type(dtype),
					       name,
					       addr_taken);
  _symtab->add_symbol(sym);
  return sym;
}


VariableSymbol* NodeBuilder::new_var(LString name,
				     QualifiedType* qtype,
				     bool addr_taken)
{
  VariableSymbol* sym = create_variable_symbol(_suif_env,
					       qtype,
					       name,
					       addr_taken);
  _symtab->add_symbol(sym);
  return sym;
}


VariableSymbol* NodeBuilder::lookup_var(LString name, bool local_only)
{
  if (local_only)
    return ::lookup_var_locally(_symtab, name);
  else
    return ::lookup_var(_symtab, name);
}


// Lookup a named type locally on the symbol table.
// return 0 if not found.
//
static Type* lookup_type_locally(SymbolTable* stab, LString name)
{
  for (Iter<SymbolTableObject*> iter =
	 stab->get_symbol_table_object_iterator();
       iter.is_valid();
       iter.next()) {
    if (!is_kind_of<Type>(iter.current())) continue;
    Type* type = to<Type>(iter.current());
    if (type->get_name() == name) return type;
  }
  return 0;
}

Type* NodeBuilder::lookup_type(LString name, bool local_only)
{
  for (SymbolTable* stab = _symtab; stab!=0; stab=find_super_scope(stab)) {
    Type* t = lookup_type_locally(stab, name);
    if (t != 0) return t;
    if (local_only) return 0;
  }
  return 0;
}


StructType* NodeBuilder::new_struct_type(LString name)
{
  GroupSymbolTable* gtab = create_group_symbol_table(_suif_env);
  StructType* stype = create_struct_type(_suif_env, 0, sizeof(int), name, true,
					 gtab);
  _symtab->add_symbol(stype);
  return stype;
}

void NodeBuilder::add_struct_field(StructType* stype,
				   LString fname,
				   QualifiedType* ftype)
{
  _type_builder->add_symbol_to_group(stype, fname, ftype);
}


IntegerType* NodeBuilder::get_int_type(void)
{
  return _type_builder->get_integer_type(true);
}

BooleanType* NodeBuilder::get_bool_type(void)
{
  return _type_builder->get_boolean_type();
}

FloatingPointType* NodeBuilder::get_float_type(void)
{
  return _type_builder->get_floating_point_type();
}

IntegerType* NodeBuilder::get_char_type(void)
{
  return _type_builder->get_smallest_integer_type();
}

VoidType* NodeBuilder::get_void_type(void)
{
  return _type_builder->get_void_type();
}


QualifiedType* NodeBuilder::get_qualified_type(DataType* basetype)
{
  return _type_builder->get_qualified_type(basetype);
}


Type* NodeBuilder::get_unqualified_type(Type* qt)
{
  return _type_builder->unqualify_type(qt);
}


/** If t is a pointer type (after removing qualification), return the type
  * it points to.
  * Otherwise return 0.
  */
Type* NodeBuilder::get_pointed_to_type(Type* t)
{
  Type* ptype = get_unqualified_type(t);
  if (!is_kind_of<PointerType>(ptype)) return 0;
  return to<PointerType>(ptype)->get_reference_type();
}


PointerType* NodeBuilder::get_pointer_type(Type* t)
{
  return _type_builder->get_pointer_type(t);
}




StoreVariableStatement* NodeBuilder::store_var(VariableSymbol* dst,
					       Expression* src)
{
  if (!is_same_type(dst->get_type(), src->get_result_type())) {
    trash(dst);
    trash(src);
    SUIF_THROW(SuifException(String("Type mismatch in StoreVariableStatement for ")
			     + to_id_string(dst) + " from expression " +
			     to_id_string(src)));
  }
  return create_store_variable_statement(_suif_env, dst, src);
}

StoreStatement* NodeBuilder::store(Expression* dst_addr, Expression* src_addr)
{
  if (!is_same_type(get_pointed_to_type(dst_addr->get_result_type()),
		    src_addr->get_result_type())) {
    trash(dst_addr);
    trash(src_addr);
    SUIF_THROW(SuifException(String("Type error in StoreStatement from ") +
			     to_id_string(src_addr) + " to " +
			     to_id_string(dst_addr)));
  }
  return create_store_statement(_suif_env, src_addr, dst_addr);
}

StoreStatement* NodeBuilder::store_field(Expression* group_addr,
					 LString field_name,
					 Expression* value)
{
  FieldAccessExpression* fexp = field_addr(group_addr, field_name);
  return store(fexp, value);
}



EvalStatement* NodeBuilder::eval(Expression* exp)
{
  EvalStatement* es = create_eval_statement(_suif_env);
  es->append_expression(exp);
  return es;
}

StatementList* NodeBuilder::new_statement_list(void)
{
  return create_statement_list(_suif_env);
}

IfStatement* NodeBuilder::new_if(Expression* pred,
				 Statement* then_part,
				 Statement* else_part)
{
  if (!(is_kind_of<IntegerType>(pred->get_result_type()) ||
	is_kind_of<BooleanType>(pred->get_result_type()))) {
    trash(then_part);
    trash(else_part);
    SUIF_THROW(SuifException(String("Expection a boolean or integer expression ")
			     + to_id_string(pred)));
  }
  return create_if_statement(_suif_env, pred, then_part, else_part);
}








LoadExpression* NodeBuilder::load(Expression* addr)
{
  Type* ptype = get_unqualified_type(addr->get_result_type());
  if (!is_kind_of<PointerType>(ptype)) {
    trash(addr);
    SUIF_THROW(SuifException(String("Cannot make a LoadExpression out of ") +
			     to_id_string(addr) + " whose type " +
			     to_id_string(ptype) + " is not a pointer type."));
  }
  Type* btype =
    get_unqualified_type(to<PointerType>(ptype)->get_reference_type());
  if (!is_kind_of<DataType>(btype)) {
    trash(addr);
    SUIF_THROW(SuifException(String("Cannot make a LoadExpression out of ") +
			     to_id_string(addr) + " whose type " +
			     to_id_string(ptype) + " is not a pointer to a DataType."));
  }
  DataType *dtype = to<DataType>(btype);
  return create_load_expression(_suif_env, dtype, addr);
}


LoadVariableExpression* NodeBuilder::load_var(VariableSymbol* var)
{
  Type* rtype = get_unqualified_type(var->get_type());
  if (!is_kind_of<DataType>(rtype)) {
    trash(var);
    SUIF_THROW(SuifException(String("Cannot make a LoadVariableExpression out of ") +
			     to_id_string(var) + " whose type " +
			     to_id_string(rtype) + " is not a data type."));
  }
  return create_load_variable_expression(_suif_env, to<DataType>(rtype), var);
}



LoadVariableExpression* NodeBuilder::load_var(LString vname)
{
  VariableSymbol* var = lookup_var(vname);
  if (var == 0)
    SUIF_THROW(SuifDevException(__FILE__, __LINE__,
				String("Cannot find variable <") + vname +
				       ">"));
  return load_var(var);
}


SymbolAddressExpression* NodeBuilder::sym_addr(Symbol* sym)
{
  PointerType* ptype = get_pointer_type(sym->get_type());
  return create_symbol_address_expression(_suif_env, ptype, sym);
}



/** find the field with the name.
  */
static FieldSymbol* find_field(GroupType* gtype, LString field_name)
{
  for (Iter<SymbolTableObject*> iter =
	 gtype->get_group_symbol_table()->get_symbol_table_object_iterator();
       iter.is_valid();
       iter.next()) {
    if (iter.current()->get_name() != field_name) continue;
    return to<FieldSymbol>(iter.current());
  }
  return 0;
}



Expression* NodeBuilder::load_field(Expression* grp_addr,
				    LString field_name)
{
  Expression* faddr = field_addr(grp_addr, field_name);
  return load(faddr);
}

 


FieldAccessExpression* NodeBuilder::field_addr(Expression* grp_addr,
					       LString field_name)
{
  GroupType* gtype = to_ref_type<GroupType>(this, grp_addr->get_result_type());
  if (gtype == 0) {
    trash(grp_addr);
    SUIF_THROW(SuifException(String("Type error: expecting an addr to a group ")
				    + to_id_string(grp_addr)));
  }
  FieldSymbol *f = find_field(to<GroupType>(gtype), field_name);
  if (f == 0) {
    trash(grp_addr);
    SUIF_THROW(SuifException(String("Cannot find field <") + field_name +
				    "> in group type " +
				    to_id_string(gtype)));
  }
  FieldAccessExpression *res =
    create_field_access_expression(_suif_env,
				   // to<DataType>(fieldtype),
				   get_pointer_type(f->get_type()),
				   grp_addr,
				   f);
  return res;
}


  
IntConstant* NodeBuilder::int_const(IInteger c)
{
  return create_int_constant(_suif_env, get_int_type(), c);
}

IntConstant* NodeBuilder::int_const(int i)
{
  return int_const(IInteger(i));
}


IntConstant* NodeBuilder::bool_const(bool v)
{
  return int_const(IInteger(v ? 1 : 0));
}

FloatConstant* NodeBuilder::float_const(float v)
{
  char buf[64];
  sprintf(buf, "%f", v);
  return create_float_constant(_suif_env, get_float_type(), String(buf));
}

Expression* NodeBuilder::cstr_const(const char* value,
				    ProcedureDefinition* proc)
{
  VariableSymbol* var = get_cstr_const_var(String(value), proc);
  return load_var(var);
}


IntConstant* NodeBuilder::char_const(char c)
{
  return create_int_constant(_suif_env, get_char_type(), c);
}

      




BinaryExpression* NodeBuilder::binary_exp(LString op,
					  DataType* result_type,
					  Expression* arg1,
					  Expression* arg2)
{
  return create_binary_expression(_suif_env, result_type, op, arg1, arg2);
}


static void check_bool_types(DataType* t1, DataType* t2)
{
  if (!is_kind_of<NumericType>(t1) && !is_kind_of<BooleanType>(t1))
    SUIF_THROW(SuifDevException(__FILE__, __LINE__,
				String("Expection either numeric or boolean "
				       "type - ") + to_id_string(t1)));
  if (!is_kind_of<NumericType>(t2) && !is_kind_of<BooleanType>(t2))
    SUIF_THROW(SuifDevException(__FILE__, __LINE__,
				String("Expection either numeric or boolean "
				       "type - ") + to_id_string(t2)));
}

 
static void check_numptr_types(DataType* t1, DataType* t2)
{
  if (!is_kind_of<NumericType>(t1) && !is_kind_of<PointerType>(t1))
    SUIF_THROW(SuifDevException(__FILE__, __LINE__,
				String("Expection either numeric or pointer "
				       "type - ") + to_id_string(t1)));
  if (!is_kind_of<NumericType>(t2) && !is_kind_of<PointerType>(t2))
    SUIF_THROW(SuifDevException(__FILE__, __LINE__,
				String("Expection either numeric or pointer "
				       "type - ") + to_id_string(t2)));
}
				       

BinaryExpression* NodeBuilder::and_exp(Expression* arg1, Expression* arg2)
{
  check_bool_types(arg1->get_result_type(), arg2->get_result_type());
  return binary_exp("logical_and", arg1->get_result_type(), arg1, arg2);
}

BinaryExpression* NodeBuilder::multiply_exp(Expression* arg1, Expression* arg2)
{
  check_numptr_types(arg1->get_result_type(), arg2->get_result_type());
  return binary_exp("multiply", arg1->get_result_type(), arg1, arg2);
}

BinaryExpression* NodeBuilder::add_exp(Expression* arg1, Expression* arg2)
{
  check_numptr_types(arg1->get_result_type(), arg2->get_result_type());
  return binary_exp("add", arg1->get_result_type(), arg1, arg2);
}

BinaryExpression* NodeBuilder::subtract_exp(Expression* arg1, Expression* arg2)
{
  check_numptr_types(arg1->get_result_type(), arg2->get_result_type());
  return binary_exp("subtract", arg1->get_result_type(), arg1, arg2);
}

BinaryExpression* NodeBuilder::is_less_than_or_equal_to_exp(Expression* arg1,
							    Expression* arg2)
{
  check_numptr_types(arg1->get_result_type(), arg2->get_result_type());
  return binary_exp("is_less_than_or_equal_to", get_bool_type(), arg1, arg2);
}

BinaryExpression* NodeBuilder::divfloor_exp(Expression* arg1, Expression* arg2)
{
  check_numptr_types(arg1->get_result_type(), arg2->get_result_type());
  return binary_exp("divfloor", arg1->get_result_type(), arg1, arg2);
}

BinaryExpression* NodeBuilder::max_exp(Expression* arg1 , Expression* arg2)
{
  check_numptr_types(arg1->get_result_type(), arg2->get_result_type());
  return binary_exp("maximum", arg1->get_result_type(), arg1, arg2);
}


BinaryExpression* NodeBuilder::min_exp(Expression* arg1, Expression* arg2)
{
  check_numptr_types(arg1->get_result_type(), arg2->get_result_type());
  return binary_exp("minimum", arg1->get_result_type(), arg1, arg2);
}


UnaryExpression* NodeBuilder::convert_exp(Expression* arg, DataType* t)
{
  return create_unary_expression(_suif_env, t, "convert", arg);
} 


ProcedureSymbol* NodeBuilder::lookup_proc(LString name, bool local_only)
{
  if (local_only)
    return ::lookup_proc_locally(_symtab, name);
  else
    return ::lookup_proc(_symtab, name);
}


ProcedureDefinition* NodeBuilder::new_proc_defn1(LString name,
						 DataType* return_type,
						 ParameterSymbol* arg)
{
  list<QualifiedType*> argtypes;
  argtypes.push_back(to<QualifiedType>(arg->get_type()));
  CProcedureType *cproctype =
    _type_builder->get_c_procedure_type(return_type, argtypes);
  ProcedureSymbol* psym = create_procedure_symbol(_suif_env, cproctype, name);
  _symtab->add_symbol(psym);
  ProcedureDefinition* pdfn =
    create_procedure_definition(_suif_env, psym, 0,
				create_basic_symbol_table(_suif_env, _symtab),
				0);
  pdfn->append_formal_parameter(arg);
  psym->set_definition(pdfn);
  return pdfn;
}


VariableSymbol* NodeBuilder::get_cstr_const_var(String s, ProcedureDefinition* p)
{
  NodeBuilder nb(p->get_symbol_table());
  MultiValueBlock* vb = nb.new_cstr_value_block(s);
  VariableSymbol* var = new_var("", vb->get_type());
  nb.add_variable_definition(p, var, vb);
  return var;
}

  

void NodeBuilder::replace(Statement* oldstmt, Statement* newstmt)
{
  ::replace_statement(oldstmt, newstmt);
}

void NodeBuilder::replace(Expression* old_exp, Expression* new_exp)
{
  ::replace_expression(old_exp, new_exp);
}
