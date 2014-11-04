// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include "array_dismantlers.h"
#include "suifkernel/command_line_parsing.h"
#include "suifkernel/utilities.h"
#include "typebuilder/type_builder.h"
#include "utils/expression_utils.h"
#include "basicnodes/basic_constants.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif_factory.h"
#include "common/suif_map.h"
#include "utils/cloning_utils.h"
#include "utils/array_utils.h"

ArrayReferenceDismantlerPass::
ArrayReferenceDismantlerPass(SuifEnv *the_env, const LString &name) :
  PipelinablePass(the_env, name),_env(the_env) {}

Module *ArrayReferenceDismantlerPass::clone() const {
  return((Module*)this);
}

void ArrayReferenceDismantlerPass::initialize() {
  PipelinablePass::initialize();
  _command_line->set_description("Dismantle ArrayReferenceExpressions into pointer arithmetic");
}

static void dismantle_array_reference(SuifEnv *env, 
    ArrayReferenceExpression *cal, TypeBuilder *tb)
{
    Expression *array_address = cal->get_base_array_address();
    array_address->set_parent(0);
    DataType *type = array_address->get_result_type();
    ArrayType* array_type =
            to<ArrayType>(tb->unqualify_type(to<PointerType>(type)->
                    get_reference_type()));
    Expression *index = cal->get_index();
 	cal->set_index(0);
	cal->set_base_array_address(0);
    index->set_parent(0);
    Expression *low = array_type->get_lower_bound();
    IInteger scale = to<DataType>(array_type->get_element_type()->get_base_type())
		->get_bit_size()/BITSPERBYTE;
    Expression *scaled_index = build_dyadic_expression(k_multiply,
		build_dyadic_expression(k_subtract,index,low),
		create_int_constant(env,scale));
    Expression *ptr_calc = build_dyadic_expression(k_add,array_address,scaled_index);
    cal->get_parent()->replace(cal,ptr_calc);
    delete cal;
}

void ArrayReferenceDismantlerPass:: do_procedure_definition(ProcedureDefinition *pd) {
    list<ArrayReferenceExpression *> *l = collect_objects<ArrayReferenceExpression>(pd);
    TypeBuilder *tb = (TypeBuilder *)
         _env->get_object_factory(TypeBuilder::get_class_name());
    for (list<ArrayReferenceExpression*>::iterator iter = l->begin();
       iter != l->end(); iter++) 
    {
        ArrayReferenceExpression *cal = *iter;
        dismantle_array_reference(_env, cal, tb);
	}
    delete l;
}

MultiDimArrayDismantlerPass::
MultiDimArrayDismantlerPass(SuifEnv *the_env, const LString &name) :
  Pass(the_env, name) {}

void MultiDimArrayDismantlerPass::initialize() {
  Pass::initialize();
  _command_line->set_description("Dismantle all MultiDimArrayExpressions into ArrayExpressions");
}

Module *MultiDimArrayDismantlerPass::clone() const {
  return((Module*)this);
}

static Type *disassemble_multi_array_type(
                        SuifEnv *env,
                        TypeBuilder *type_builder,
                        MultiDimArrayType *typ) {
    QualifiedType * element_type = typ->get_element_type();
    int dims = typ->get_lower_bound_count();
    for (int i = 0;i < dims; i++) {
	Expression *low = typ->get_lower_bound(i);
	Expression *high = typ->get_upper_bound(i);

        ArrayType *array_type = type_builder->get_array_type(
                element_type,
                clone_if_needed(low),
		clone_if_needed(high));
	    element_type = type_builder->get_qualified_type(array_type);
    }
    return element_type->get_base_type();
}

static void dismantle_multi_dim_array_expression(
		SuifEnv *env,
		MultiDimArrayExpression *exp,
		TypeBuilder *type_builder,
		suif_hash_map<MultiDimArrayType *,Type *> &type_map) 
{
    Expression *ref_exp = exp->get_array_address();

    Type *typ = ref_exp->get_result_type();
    if (is_kind_of<PointerType>(typ))
	typ = to<PointerType>(typ)->get_reference_type();
    if (is_kind_of<ReferenceType>(typ))
        typ = to<ReferenceType>(typ)->get_reference_type();

    if (is_kind_of<QualifiedType>(typ))
        typ = to<QualifiedType>(typ)->get_base_type();

    simple_stack<Expression *> lows;
    int dims;
    Type *rep_type;

    if (is_kind_of<MultiDimArrayType>(typ)) {
    	MultiDimArrayType *mdatyp= to<MultiDimArrayType>(typ);
	suif_hash_map<MultiDimArrayType *,Type *>::iterator iter =
		type_map.find(mdatyp);
	kernel_assert_message(iter != type_map.end(),
		("Error - type not converted"));
	rep_type = (*iter).second;
    	dims = exp->get_index_count();
    	for (int i = dims - 1;i >=0 ; i--) {
	    lows.push(mdatyp->get_lower_bound(i));
	    }
	}
    else {
	// this arm should never be taken, so assert
	kernel_assert_message(false,("This code should not have been accessed"));
	rep_type = typ;
        dims = 0;
	while (is_kind_of<ArrayType>(typ)) {
	    ArrayType *atype = to<ArrayType>(typ);
	    dims ++;
	    lows.push(atype->get_lower_bound());
	    typ = to<QualifiedType>(atype->get_element_type())->get_base_type();
	    }
	}

    exp->replace(ref_exp,0);
    ref_exp->set_parent(0);
    int index_count = exp->get_index_count();
    for (int i = 0;i < index_count;i ++) {
	Type *ref_type = rep_type;
	for (int j = 0;j <= i;j ++) {
	    ref_type = type_builder->unqualify_type(ref_type);
	    ref_type = to<ArrayType>(ref_type)->get_element_type();
	    }
	ref_type = type_builder->unqualify_type(ref_type);
	ref_type = type_builder->get_pointer_type(ref_type);
	Expression *index = exp->get_index(index_count - i - 1);



	// process nested multi dim arra expressions
        for (Iter<MultiDimArrayExpression> iter =
                object_iterator<MultiDimArrayExpression>(index);
         iter.is_valid();
         iter.next()) {
        MultiDimArrayExpression *mexpr = &iter.current();
        dismantle_multi_dim_array_expression(env,mexpr,type_builder,type_map);
        }
	
	exp->replace(index,0);
	index->set_parent(0);
        ref_exp = create_array_reference_expression(
		env,to<DataType>(ref_type),ref_exp,
		index);
	}
    exp->get_parent()->replace(exp,ref_exp);
}

void MultiDimArrayDismantlerPass::do_file_set_block( FileSetBlock* file_set_block ) {
    SuifEnv *env = get_suif_env();
    TypeBuilder *type_builder = (TypeBuilder *)
          env->get_object_factory(TypeBuilder::get_class_name());
    suif_hash_map<MultiDimArrayType *,Type *> type_map;
    ReplacingWalker walker(env);

    list<MultiDimArrayType *> type_list;

    for (Iter<MultiDimArrayType> titer =
                object_iterator<MultiDimArrayType>(file_set_block);
         titer.is_valid(); titer.next()) 
    {
        MultiDimArrayType *type = &titer.current();
	    type_list.push_back(type);
	}

    for (list<MultiDimArrayType *>::iterator tliter = type_list.begin();tliter != type_list.end();tliter ++) {
	    MultiDimArrayType *type = *tliter;
	    Type *rep_type = disassemble_multi_array_type(env,type_builder,type);
	    type_map.enter_value(type,rep_type);
	    to<BasicSymbolTable>(type->get_parent())->remove_symbol_table_object(type);
	    walker.add_replacement(type,rep_type);
    }

    for (Iter<MultiDimArrayExpression> iter = 
		object_iterator<MultiDimArrayExpression>(file_set_block);
        iter.is_valid(); iter.next()) 
    {
    	MultiDimArrayExpression *expr = &iter.current();
    	dismantle_multi_dim_array_expression(env,expr,type_builder,type_map);
	}
    file_set_block->walk(walker);
};

NonConstBoundDismantlerPass::
NonConstBoundDismantlerPass(SuifEnv *the_env, const LString &name) :
  Pass(the_env, name) {}

Module *NonConstBoundDismantlerPass::clone() const {
  return((Module*)this);
}
void NonConstBoundDismantlerPass::initialize() {
  Pass::initialize();
  _command_line->set_description("Dismantle all ArrayExpressions with non constant array bounds");
}

typedef suif_hash_map<SuifObject *,SuifObject *> ReverseMap;
class NonConstDimExpressionWalker : public SelectiveWalker {
    ReplacingWalker &_map;
    TypeBuilder *_type_builder;
    ReverseMap &_rev_map;
  public:
    NonConstDimExpressionWalker(SuifEnv *env,
				ReplacingWalker &map,
				ReverseMap &rev_map,
				const LString &selector);
    Walker::ApplyStatus operator () (SuifObject *x);
    };

NonConstDimExpressionWalker::NonConstDimExpressionWalker(
	SuifEnv *env,ReplacingWalker &map,ReverseMap &rev_map,
			const LString &selector)
        : SelectiveWalker(env,selector), _map(map),_rev_map(rev_map) {
    _type_builder = (TypeBuilder *)
          env->get_object_factory(TypeBuilder::get_class_name());
    }

Walker::ApplyStatus NonConstDimExpressionWalker::operator () (SuifObject *x) {
    if (_rev_map.find(x) != _rev_map.end())
	return Walker::Continue;
    ArrayExpressionProxy exp(to<Expression>(x));        
    Expression *ref_exp = exp.get_array_address();

    ArrayTypeProxy type(exp.get_array_type());
    if (!type.has_non_const_bounds())
        return Walker::Continue;

    DataType *element_type = type.get_element_type();
    IInteger bit_size = element_type->get_bit_size();

    int dims = exp.get_dimension_count();
    int type_dims = type.get_dimension_count();

    Expression *index = 0;
    Expression *offset = 0;
    int missing_dims = type_dims - dims; // the more nested dims are the missing ones
    for (int i = type_dims-1;i >=0;i--) {
	// This code uses an iterative approach which is not good in general
	// as it serialises everything in the generated code
	Expression *low = type.get_lower_bound(i);
	Expression *next_index;
	if (i < missing_dims) {
	    next_index = low;
	    }
	else {
	    next_index = exp.get_index(i-missing_dims);
	    }
	if (!index) {
	    index = next_index;
	    offset = low;
	    }
	else {
            Expression *high = type.get_upper_bound(i);

            Expression *elements = build_dyadic_expression(k_add,
                                build_dyadic_expression(k_subtract,
                                                        high,
                                                        low),
                                create_int_constant(get_env(),
                                                    low->get_result_type(),
                                                    1));
	    index = build_dyadic_expression(k_add,
					    next_index,
					    build_dyadic_expression(k_multiply,
								    elements,
								    index));
            offset = build_dyadic_expression(k_add,
                                            low,
                                            build_dyadic_expression(k_multiply,
                                                                    elements,
                                                                    offset));
	    }
	}

    index = build_dyadic_expression(k_subtract,index,offset);
    ref_exp = create_array_reference_expression(	
					get_env(),
					type.get_element_type(),
					to<Expression>(ref_exp->deep_clone()),
					index);

    // next line is done automatically
    // x->get_parent()->replace(x,ref_exp); 
    _rev_map.enter_value(ref_exp,x);
    set_address(ref_exp);
    return Walker::Replaced;
}

class NonConstBoundArrayTypeWalker : public SelectiveWalker {
    TypeBuilder *_type_builder;
    ReverseMap &_rev_map;
  public:
    NonConstBoundArrayTypeWalker(SuifEnv *env,ReverseMap &rev_map,const LString &selector);
    Walker::ApplyStatus operator () (SuifObject *x);
};

Walker::ApplyStatus NonConstBoundArrayTypeWalker::operator () (SuifObject *x) {
    ArrayTypeProxy type(to<DataType>(x));
    if (!type.has_non_const_bounds())
	return Walker::Continue;    
    Type *rep_type = _type_builder->get_array_type(_type_builder->get_qualified_type(type.get_element_type()),0,1);
    set_address(rep_type);
    to<BasicSymbolTable>(x->get_parent())->remove_symbol_table_object(
 		to<SymbolTableObject>(x));
    return Walker::Replaced;
}

NonConstBoundArrayTypeWalker::NonConstBoundArrayTypeWalker(
		SuifEnv *env,ReverseMap &rev_map,const LString &selector) 
        : SelectiveWalker(env,selector),_rev_map(rev_map) {
    _type_builder = (TypeBuilder *) env->get_object_factory(
			TypeBuilder::get_class_name());
}

void NonConstBoundDismantlerPass::do_file_set_block( FileSetBlock* file_set_block ) {
    SuifEnv *env = get_suif_env();
    ReplacingWalker walker(env);
    ReverseMap rev_map;
    NonConstDimExpressionWalker ew1(env,walker,rev_map,MultiDimArrayExpression::get_class_name());
    NonConstDimExpressionWalker ew2(env,walker,rev_map,ArrayReferenceExpression::get_class_name());

    NonConstBoundArrayTypeWalker  aw1(env,rev_map,MultiDimArrayType::get_class_name());
    NonConstBoundArrayTypeWalker  aw2(env,rev_map,ArrayType::get_class_name());
    walker.append_walker(ew1);
    walker.append_walker(ew2);
    file_set_block->walk(walker);
    ReplacingWalker type_walker(env);
    type_walker.append_walker(aw1);
    type_walker.append_walker(aw2);

    file_set_block->walk(type_walker);
}

OneDimArrayConverter::OneDimArrayConverter(SuifEnv* the_suif_env):\
suif_env(the_suif_env)
{
    tb = (TypeBuilder*)suif_env->
        get_object_factory(TypeBuilder::get_class_name());
    type_map = new suif_map<ArrayType*, MultiDimArrayType*>;
    all_array_types = new suif_vector<ArrayType*>;
};

OneDimArrayConverter::~OneDimArrayConverter(){
    delete type_map;
    delete all_array_types;
};

MultiDimArrayType* OneDimArrayConverter::array_type2multi_array_type(ArrayType* at){
    suif_vector<ArrayType*> array_types;

    suif_map<ArrayType*, MultiDimArrayType*>::iterator type_iter =
            type_map->find(to<ArrayType>(at));

    if (type_iter == type_map->end()) {
        suif_vector<Expression*> lower_bounds;
        suif_vector<Expression*> upper_bounds;
        suif_vector<ArrayType*> array_types;
        Type *type = at->get_element_type()->get_base_type();

        array_types.push_back(at);                      // sub-types for this array type
        all_array_types->push_back(at);                 // all array types

        while (is_kind_of<ArrayType>(type)) {           // unwrap array access
            ArrayType *atyp = to<ArrayType>(type);
            array_types.push_back(atyp);
            type = atyp->get_element_type()->get_base_type();
        }

        // save lower and upper bounds
        for (int i = array_types.size()-1;i >=0;i--) {
            ArrayType *atyp = array_types[i];
            lower_bounds.push_back(deep_suif_clone(atyp->get_lower_bound()));
            upper_bounds.push_back(deep_suif_clone(atyp->get_upper_bound()));
        }

        IInteger bit_size = to<DataType>(type)->get_bit_size();
        IInteger bit_alignment = to<DataType>(type)->get_bit_alignment();

        MultiDimArrayType* multi_type = tb->get_multi_dim_array_type(
                bit_size, bit_alignment.c_int(), 
                tb->get_qualified_type(type),
                lower_bounds, upper_bounds);

        // save the translation in the map
        type_map->enter_value(at, multi_type);
        return multi_type;
    }else
        return (*type_iter).second;
}

static QualifiedType* unwrap_ptr_ref_type(DataType* type){
    if (is_kind_of<PointerType>(type)){
        return (QualifiedType*)to<PointerType>(type)->get_reference_type();
    }else if (is_kind_of<ReferenceType>(type)){
        return (QualifiedType*)to<ReferenceType>(type)->get_reference_type();
    }else
        suif_assert_message(false, ("Expecting either a pointer or a reference type"));
    return NULL;
};

/**
    Convert ArrayReferenceExpression \a top_array to a 
    MultiDimArrayExpression.
*/
void OneDimArrayConverter::
convert_array_expr2multi_array_expr(ArrayReferenceExpression* top_array){
    Expression* expr = top_array;
    unsigned int i = 0;

    suif_vector<Expression*> lower_bounds;
    suif_vector<Expression*> upper_bounds;
    list<Expression*> indices;
    IInteger bit_size, bit_alignment;
    suif_vector<ArrayReferenceExpression *> exprs;

    do{
        ArrayReferenceExpression* are = to<ArrayReferenceExpression>(expr);
        expr = are->get_base_array_address();
        exprs.push_back(are);
	} while(is_kind_of<ArrayReferenceExpression>(expr));

    // collect bounds and indeces
    for (unsigned int exp_ind = 0; exp_ind < exprs.size();exp_ind++)
    {
        ArrayReferenceExpression* are = exprs[exp_ind];
        DataType* type = are->get_base_array_address()->get_result_type();
        ArrayType* array_type;

		if(is_kind_of<ArrayType>(type)){
			array_type = to<ArrayType>(type);
		}else{
			array_type = to<ArrayType>(
				unwrap_ptr_ref_type(type)->get_base_type());
		}
        
        bit_size = array_type->get_element_type()->
                get_base_type()->get_bit_size();
        bit_alignment = array_type->get_element_type()->
                get_base_type()->get_bit_alignment();

        // What happens to ArrayType?? Does it become garbage?
        suif_assert(array_type->get_lower_bound());
        suif_assert(array_type->get_upper_bound());

        // clone bounds and add to the lists
        lower_bounds.push_back(array_type->get_lower_bound());
        upper_bounds.push_back(array_type->get_upper_bound());
        // save the index
        Expression* index = are->get_index();
        remove_suif_object(index); index->set_parent(NULL);
        indices.push_back(index);

        suif_assert(upper_bounds[i]);
        suif_assert(lower_bounds[i]);
        suif_assert(indices[i]);

        i++;
	}

    // Build the offset for the expression. We have to traverse upwards to do this
    Expression *top_expr_base = exprs[exprs.size()-1]->get_base_array_address();
    
	DataType* top_expr_base_type = top_expr_base->get_result_type();
	ArrayType* top_array_type;
	if(is_kind_of<ArrayType>(top_expr_base_type)){
		top_array_type = to<ArrayType>(top_expr_base_type);
	}else{
		top_array_type = to<ArrayType>(tb->unqualify_data_type(unwrap_ptr_ref_type(
			top_expr_base_type)));
	}
    
	Expression* inc    = create_int_constant(suif_env, 1);
    Expression* offset = create_int_constant(suif_env, 0);
    suif_vector<Expression *> elements;

    for (unsigned int ind = 0;ind < lower_bounds.size(); ind ++)
    {
        Expression *lower = lower_bounds[ind];
        Expression *upper = upper_bounds[ind];

        offset = 
            build_dyadic_expression(k_add, 
                offset,
                build_dyadic_expression(
                    k_multiply, 
                    deep_suif_clone(inc),
                    deep_suif_clone(lower)));
        
        Expression *element =
                    build_dyadic_expression(k_add, 
                        build_dyadic_expression(k_subtract,
                            deep_suif_clone(upper), 
                            deep_suif_clone(lower)),
                        create_int_constant(suif_env,1));

        inc = build_dyadic_expression(k_multiply, 
            inc, 
            deep_suif_clone(element));

	    elements.push_back(element);
    }
    // Now, inc and offset are ready

    // retrieve the multi-type
    MultiDimArrayType* multi_type;
	suif_map<ArrayType*, MultiDimArrayType*>::iterator type_iter =
	    type_map->find(top_array_type);
#ifdef CONVERT_TYPES
    suif_assert_message((type_iter != type_map->end()),
        ("Array type never translated"));
#else
    if(type_iter == type_map->end()){
        multi_type = array_type2multi_array_type(top_array_type);
    }else
#endif //CONVERT_TYPES
    multi_type = (*type_iter).second;

    remove_suif_object(top_expr_base);

    // add a convert to the necessary type
    top_expr_base = create_unary_expression(suif_env, 
            tb->get_pointer_type(multi_type), 
            k_convert, top_expr_base);
    //top_expr_base->print_to_default();

    // construct the expression to be returned 
    MultiDimArrayExpression* mae =
        create_multi_dim_array_expression(suif_env,
            top_array->get_result_type(), 
            top_expr_base, 
            offset);

    // now when we have the expression, set the indices
    for (list<Expression*>::iterator ind_iter = indices.begin();
        ind_iter!=indices.end(); ind_iter++)
    {
        Expression* index = *ind_iter;
        //printf("%p \t", index);index->print_to_default();
        mae->append_index(index);
    }

    for (suif_vector<Expression *>::iterator eiter = elements.begin();
        eiter != elements.end(); eiter ++)
    {
        Expression* element = *eiter;
        mae->append_element(element);
    }

    replace_expression(top_array, mae);
}

/**
    Remove all the array types collected by all the calls
    to array_type2multi_array_type.
    \see array_type2multi_array_type
*/
void OneDimArrayConverter::remove_all_one_dim_array_types(){
    for (int j=0;j < (int)all_array_types->size();j++)
    {
        remove_suif_object((*all_array_types)[j]);
    }
}

void One2MultiArrayExpressionPass::initialize() {
    PipelinablePass::initialize();
    _command_line -> set_description(
                    "Convert one-dimentional array "
                    "access expressions and array types to"
                   " multidimantional ones\n"
                   " If -preserve1dim is specified, then single"
                   " ArrayReferenceExpressions are preserved and not"
                   " converted to corresponding multidimentional ones."
                   );
    _preserve_one_dim = new OptionLiteral("-preserve1dim");
    OptionSelection *opt = new OptionSelection(true);
    opt->add(_preserve_one_dim);
    _command_line->add(opt);
}

void One2MultiArrayExpressionPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
    bool kill_all = !(_preserve_one_dim->is_set());
    // access all array type declarations and create corresponding multi array types
    SuifEnv* suif_env = proc_def->get_suif_env();
    TypeBuilder* tb = (TypeBuilder*)suif_env->
        get_object_factory(TypeBuilder::get_class_name());
    (void) tb; // avoid warning
#ifdef CONVERT_TYPES
    for (Iter<ArrayType> at_iter = object_iterator<ArrayType>(proc_def);
        at_iter.is_valid();at_iter.next())
    {
        MultiDimArrayType* multi_type = 
            converter->array_type2multi_array_type(&at_iter.current());
	}
#endif //CONVERT_TYPES

    // collect tops of array access chains into this list
    list<ArrayReferenceExpression*> ref_exprs;
    for (Iter<ArrayReferenceExpression> are_iter =
		object_iterator<ArrayReferenceExpression>(proc_def);
         are_iter.is_valid(); are_iter.next())
    {
        // itself an array and parent is *not* an array
        ArrayReferenceExpression* are = &are_iter.current();
        if((kill_all || is_kind_of<ArrayReferenceExpression>(are->get_base_array_address())) &&
           !is_kind_of<ArrayReferenceExpression>(are->get_parent()))
        {
            //printf("%p \t", are);are->print_to_default();
            ref_exprs.push_back(are);
	    }
    }

    // for top all expressions, convert them to multi-exprs
    for(list<ArrayReferenceExpression*>::iterator ref_iter = ref_exprs.begin();
        ref_iter != ref_exprs.end(); ref_iter++)
    {
        ArrayReferenceExpression* top_array = *ref_iter;
        converter->convert_array_expr2multi_array_expr(top_array);
    }
#ifdef CONVERT_TYPES    
    // replace the types of all array variables
    for (Iter<VariableSymbol> iter = object_iterator<VariableSymbol>(proc_def);
            iter.is_valid();iter.next())
    {
        VariableSymbol* vd = &iter.current();
        DataType *vtype = tb->unqualify_data_type(vd->get_type());
        if (is_kind_of<ArrayType>(vtype)) {
            MultiDimArrayType* multi_type =
                    converter->array_type2multi_array_type(to<ArrayType>(vtype));
            vd->replace(vd->get_type(), tb->get_qualified_type(multi_type));
        }
    }

    // remove the remaining one-dim array types
    converter->remove_all_one_dim_array_types();
#endif //CONVERT_TYPES
    // make sure no traces of single-dim arrays are left
    if(kill_all){
        {for(Iter<ArrayReferenceExpression> iter =
            object_iterator<ArrayReferenceExpression>(proc_def);
            iter.is_valid(); iter.next())
            {
                // ArrayReferenceExpression* are = &iter.current();
                //are->print_to_default(); printf("at %p \t", are);
                suif_assert_message(false, ("ARE not eliminated"));
            }
        }
#ifdef CONVERT_TYPES
        {for(Iter<ArrayType> iter =
            object_iterator<ArrayType>(proc_def);
            iter.is_valid(); iter.next())
        {suif_assert_message(false, ("ArrayType not eliminated"));}}
#endif
    }
}

#if 0
void EliminateArrayConvertsPass::do_procedure_definition(ProcedureDefinition* proc_def){
    suif_hash_map<ParameterSymbol*, Type*> params;
    TypeBuilder *tb = (TypeBuilder*)
         get_suif_env()->get_object_factory(TypeBuilder::get_class_name());

    // collect all procedure parameters of pointer type into params list
    for(Iter<ParameterSymbol*> iter = proc_def->get_formal_parameter_iterator();
        iter.is_valid(); iter.next())
    {
        ParameterSymbol* par_sym = iter.current();
        Type* par_type = tb->unqualify_type(par_sym->get_type());

        if(is_kind_of<PointerType>(par_type)){
            // put NULLs into the map at first,
            // they will later be overwritten
            params[par_sym] = NULL;
        }
    }
    if(params.size()==0) return;    // nothing to do
    
    // walk thru all AREs and look for arrays that are in the param list
    {for(Iter<ArrayReferenceExpression> iter =
        object_iterator<ArrayReferenceExpression>(proc_def);
            iter.is_valid(); iter.next())
        {
            ArrayReferenceExpression* are = &iter.current();
            if(is_kind_of<UnaryExpression>(are->get_base_array_address())){
                UnaryExpression* ue = to<UnaryExpression>(are->get_base_array_address());
                if(ue->get_opcode() == k_convert){
                    if(is_kind_of<LoadVariableExpression>(ue->get_source())){
                        LoadVariableExpression* lve = 
                            to<LoadVariableExpression>(ue->get_source());
                        VariableSymbol* array = lve->get_source();
            
                        for(suif_hash_map<ParameterSymbol*, Type*>::iterator iter = params.begin();
                            iter!=params.end();iter++)
                        {
                            ParameterSymbol* par_sym = (*iter).first;
                            if(par_sym == array){
                                // match!
                                Type* array_type;
                                suif_hash_map<ParameterSymbol*, Type*>::iterator iter =
                                    params.find(par_sym);
                                
                                if(iter==params.end() || (*iter).second==NULL){
                                    //array_type = to<PointerType>(ue->get_result_type())->get_reference_type();
                                    array_type = tb->get_qualified_type(ue->get_result_type());
                                    params[par_sym] = array_type;
                                    //printf("%s has type ",par_sym->get_name().c_str());
                                    //array_type->print_to_default();
                                }else{
                                    array_type = params[par_sym].second;
                                    suif_assert(is_kind_of<QualifiedType>(array_type));
                                }

                                array->replace(array->get_type(), array_type);
                                remove_suif_object(ue);
                                remove_suif_object(lve);
                                lve->replace(lve->get_result_type(), tb->unqualify_type(array_type));
                                // put the LoadVar directly under ARE
                                are->set_base_array_address(lve);
                                //are->print_to_default();
                            }
                        }
                    } else {
                        suif_warning(ue->get_source(),
                            ("Expecting a LoadVariableExpression here"));
                    }
                } else {
                    suif_warning(ue, ("Disallow converts in AREs for "
                            "things other than procedure parameters"));
                }
            }
        }
    }
}
#endif
