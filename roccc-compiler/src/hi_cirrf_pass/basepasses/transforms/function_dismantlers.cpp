// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include "suifkernel/utilities.h"
#include "function_dismantlers.h"
#include "common/suif_map.h"
#include "basicnodes/basic.h"
#include "suifnodes/suif.h"
#include <suifkernel/command_line_parsing.h>
#include "cfenodes/cfe.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif_factory.h"
#include "cfenodes/cfe_factory.h"
#include "utils/expression_utils.h"
#include "utils/symbol_utils.h"
#include "typebuilder/type_builder.h"
#include "basicnodes/basic_constants.h"
#include "utils/trash_utils.h"

DismantleStructuredReturns::DismantleStructuredReturns(SuifEnv *env, const LString &name)
        : Pass(env, name) {}

void DismantleStructuredReturns::initialize() {
  Pass::initialize();
  _command_line->set_description("Change procedures that return structures to pass the result back as an extra argument");
}

void DismantleStructuredReturns::do_file_set_block( FileSetBlock* file_set_block ) {
    suif_map<CProcedureType *,QualifiedType *> type_map;
    list<ArrayReferenceExpression*> ref_exprs;
    SuifEnv *env = 0;
    TypeBuilder *tb = 0;
    VoidType *vt = 0;
    for (Iter<ProcedureSymbol> iter =
                object_iterator<ProcedureSymbol>(file_set_block);
	iter.is_valid();
	iter.next()) {
	ProcedureSymbol *sym = &iter.current();
	Type *type = sym->get_type();
	if (!is_kind_of<CProcedureType>(type))
	    continue;
	CProcedureType *cp_type = to<CProcedureType>(type);
	type = cp_type->get_result_type();
	if (!env) {
	    env = type->get_suif_env();
	    tb = (TypeBuilder*)
                env->get_object_factory(TypeBuilder::get_class_name());
	    vt = tb->get_void_type();
	    }
	suif_map<CProcedureType *,QualifiedType *>::iterator t_iter = type_map.find(cp_type);

	QualifiedType *qtype;
	
 	if (t_iter == type_map.end()) {
	    if (!is_kind_of<GroupType>(type) && !is_kind_of<ArrayType>(type))
                continue;
	    qtype = tb->get_qualified_type(
                        tb->get_pointer_type(to<DataType>(type)));

	    cp_type->set_result_type(vt);
	    
	    cp_type->insert_argument(0,qtype);
	    type_map.enter_value(cp_type,qtype);
	    }
	else {
	    qtype = (*t_iter).second;
	    }
	ProcedureDefinition *def = sym->get_definition();
	if (!def) 
	    continue;
	ParameterSymbol *par = create_parameter_symbol(env,qtype);
	def->get_symbol_table()->append_symbol_table_object(par);
        def->insert_formal_parameter(0,par);
	//	Convert all returns into assigned and returns
	for (Iter<ReturnStatement> ret_iter = object_iterator<ReturnStatement>(def->get_body());
        	ret_iter.is_valid();
        	ret_iter.next()) {
	    ReturnStatement *ret = &ret_iter.current();
	    Expression *retval = ret->get_return_value();
	    ret->set_return_value(0);
	    retval->set_parent(0);
	    insert_statement_before(ret,
			create_store_statement(env,retval,create_var_use(par)));
	    }
	}
    //	Change all calls to the new form
    for (Iter<CallStatement> cs_iter =
                object_iterator<CallStatement>(file_set_block);
        cs_iter.is_valid();
        cs_iter.next()) {
        CallStatement *call = &cs_iter.current();
	Type *type = call->get_callee_address()->get_result_type();
	Type *p_type = tb->unqualify_type(to<PointerType>(type)->get_reference_type());
        if (!is_kind_of<PointerType>(p_type))
            continue;
        p_type = tb->unqualify_type(to<PointerType>(p_type)->get_reference_type());

	if (!is_kind_of<CProcedureType>(p_type))
	    continue;
	CProcedureType *cp_type = to<CProcedureType>(p_type);
	
	suif_map<CProcedureType *,QualifiedType *>::iterator t_iter = type_map.find(cp_type);
	if (t_iter == type_map.end())
	    continue;
	QualifiedType *qtype = (*t_iter).second;
	DataType *var_type = to<DataType>(tb->unqualify_type(to<PointerType>(qtype->get_base_type())
		->get_reference_type()));
	VariableSymbol *var =
      	    new_anonymous_variable(env,call,tb->get_qualified_type(var_type));
	Expression *exp = create_symbol_address_expression(
		env,
		tb->get_pointer_type(var_type),
		var);
        call->insert_argument(0,exp);
        call->set_destination(0);
	}

    for (Iter<CallExpression> ce_iter =
                object_iterator<CallExpression>(file_set_block);
        ce_iter.is_valid();
        ce_iter.next()) {
        CallExpression *call = &ce_iter.current();
        Type *type = call->get_callee_address()->get_result_type();
        Type *p_type = tb->unqualify_type(to<PointerType>(type)->get_reference_type());
	if (!is_kind_of<PointerType>(p_type))
            continue;
	p_type = tb->unqualify_type(to<PointerType>(p_type)->get_reference_type());
        if (!is_kind_of<CProcedureType>(p_type))
            continue;
        CProcedureType *cp_type = to<CProcedureType>(p_type);
;
        suif_map<CProcedureType *,QualifiedType *>::iterator t_iter = type_map.find(cp_type);
        if (t_iter == type_map.end())
            continue;
        QualifiedType *qtype = (*t_iter).second;
        DataType *var_type = to<DataType>(tb->unqualify_type(to<PointerType>(qtype->get_base_type())
                ->get_reference_type()));
        VariableSymbol *var =
            new_anonymous_variable(env,call,tb->get_qualified_type(var_type));
        Expression *exp = create_symbol_address_expression(
                env,
                tb->get_pointer_type(var_type),
                var);
        call->insert_argument(0,exp);

	Statement *loc = get_expression_owner(call);
	call->get_parent()->replace(call,create_var_use(var));
	call->set_parent(0);
        suif_assert(vt != 0);
        call->set_result_type(vt);

	EvalStatement *es = create_eval_statement(env);
	insert_statement_before(loc,es);
	// Would be better to turn this into a call statement
	es->append_expression(call);
        }
    }


// This will find the last non-StatementList statement
//
static Statement *find_last_statement(Statement *st) {
  if (st == NULL) return NULL;
  if (is_kind_of<StatementList>(st)) {
    StatementList *the_list = to<StatementList>(st);
    size_t len = the_list->get_statement_count();
    // An empty statement list
    if (len == 0) return(st);
    return(find_last_statement(the_list->get_statement(len -1)));
  }
  return(st);
}

static Expression *build_empty_expression(DataType *t) {
  SuifEnv *s = t->get_suif_env();

  suif_assert_message(!is_kind_of<GroupType>(t),
		      ("build_empty_expression:: can not handle GroupType"));
  if (is_kind_of<VoidType>(t))
    return(NULL);

  if (is_kind_of<IntegerType>(t)) {
    IntConstant *c = create_int_constant(s, t, IInteger(0));
    return c;
  }
  // This works for Int, Float, Enum, pointers
  TypeBuilder *tb = get_type_builder(s);
  
  IntegerType *it = tb->get_integer_type();
  IntConstant *c = create_int_constant(s, it, IInteger(0));
  Expression *cvt = create_unary_expression(s, t,  k_convert, c);
  return(cvt);
}

NormalizeProcedureReturns::
NormalizeProcedureReturns(SuifEnv *env, const LString &name)
  : PipelinablePass(env, name) {}

void NormalizeProcedureReturns::initialize() {
  PipelinablePass::initialize();
  _command_line->set_description("Ensure all returned results match the type of the procedure");
}


void NormalizeProcedureReturns::
do_procedure_definition( ProcedureDefinition *pd ) {
  ProcedureSymbol *ps = pd->get_procedure_symbol();
  if (!ps) return;
  ProcedureType *t = to<ProcedureType>(ps->get_type());
  if (!is_kind_of<CProcedureType>(t))
    return;
  CProcedureType *ct = to<CProcedureType>(t);
  DataType *result = ct->get_result_type();
  bool is_void = is_kind_of<VoidType>(result);

  for (Iter<ReturnStatement> iter = object_iterator<ReturnStatement>(pd);
       iter.is_valid(); iter.next()) {
    ReturnStatement *ret = &iter.current();
    if (ret == NULL) continue;
    
    Expression *ret_expr = ret->get_return_value();

    if (is_void) {
      if (ret_expr != NULL) {
	// Should not be here
	trash_it(remove_suif_object(ret_expr));
      }
    } else {
      if (ret_expr == NULL) {
	// build a NULL expression to match the DataType.
	Expression *x = build_empty_expression(result);
	ret->set_return_value(x);
      }
    }
  }
}

RequireProcedureReturns::
RequireProcedureReturns(SuifEnv *env, const LString &name)
  : PipelinablePass(env, name) {}


void RequireProcedureReturns::initialize() {
  PipelinablePass::initialize();
  _command_line->set_description("Add return statements to procedures if necessary");
}


void RequireProcedureReturns::
do_procedure_definition( ProcedureDefinition *pd) {
  if (is_kind_of<Statement>(pd->get_body())) {
    Statement *st = to<Statement>(pd->get_body());
    Statement *last_st = find_last_statement(st);
    suif_assert_message(last_st != NULL,
			("Only found a NULL last statement for procedure body"));
    
    if (is_kind_of<ReturnStatement>(last_st))
      return;
    // If it is a Statement List, add a new return into the
    // statement.
    ReturnStatement *ret = 
      create_return_statement(get_suif_env(), NULL);
    // We leave the "expression" blank here. 
    // Use the normalize_procedure_returns pass to add the value.
    insert_statement_after(last_st, ret);
  }
}
