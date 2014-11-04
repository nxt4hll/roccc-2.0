#include "common/system_specific.h"
#include "common/suif_indexed_list.h"
#include "expression_utils.h"

#include "suifkernel/utilities.h"
#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"
#include "basicnodes/basic_constants.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "cfenodes/cfe.h"
#include "typebuilder/type_builder.h"
#include "symbol_utils.h"
#include "semantic_helper.h"

#include "type_utils.h"
#include "cloning_utils.h"
#include "suifkernel/module_subsystem.h"
#include "fold_table.h"

IInteger restrict_int_to_data_type(DataType *t, const IInteger &ii) {
  bool is_signed = false;
  if (is_kind_of<IntegerType>(t)) {
    is_signed = to<IntegerType>(t)->get_is_signed();
  }
  IInteger size = t->get_bit_size();
  return(IInteger::ii_finite_size(ii, size, is_signed));
}


IInteger get_expression_constant(const Expression *exp) {
  if (is_kind_of<IntConstant>(exp)) {
    IInteger val = to<IntConstant>(exp)->get_value();
    // Fold it for the datatype.
    return(restrict_int_to_data_type(exp->get_result_type(), val));
  }
  IInteger ii;
  return ii;
}

bool is_variable_modified(const VariableSymbol* var, ExecutionObject* obj){
    // make sure the index is not a global
    if(is_external_symbol_table(var->get_symbol_table())) return true;

    // check that variable is not a static --
    // has a def. and is marked as static
    if(var->get_definition()!=NULL &&
        var->get_definition()->get_is_static()) return true;

    if(var->get_is_address_taken()) return true;

    // make sure body doesn't modify the index
    {for (Iter<StoreVariableStatement> iter = object_iterator<StoreVariableStatement>(obj);
            iter.is_valid(); iter.next())
    {
        if(iter.current().get_destination()==var) return true;
    }}
    
    return false;
}

bool is_expr_modified(const Expression* expr, ExecutionObject* obj){
    VariableSymbol* var = NULL;
    {for (SemanticHelper::SrcVarIter iter(expr);
            iter.is_valid(); iter.next())
    {
        var = iter.current();
        if(is_variable_modified(var, obj)) return true;
    }}
    
    return false;
}

Expression * create_const_op(SuifEnv *env,int value) {
    TargetInformationBlock *tib = find_target_information_block(env);
    suif_assert(tib);
    DataType *type = tib->get_word_type();

    Constant *exp = create_int_constant(env, type, value);
//    Expression *exp = create_load_constant_expression(env,type,iconst);
    return exp;
}

IntConstant* create_int_constant( SuifEnv* suif_env, IInteger i ) {
    TargetInformationBlock *tib = find_target_information_block(suif_env);
    suif_assert(tib);
    DataType *type = tib->get_word_type();

    return create_int_constant( suif_env, type, i );
}

 ProcedureSymbol *get_procedure_from_address_op(const Expression *procedure_address) {
  //  SourceOp procedure_address = st->get_callee_address();
  //  if (!procedure_address.is_expression()) { return NULL; }
  const Expression *addr_expr = procedure_address;//.get_expression();

  // skip copys
  while(1) {
    if (is_kind_of<UnaryExpression>(addr_expr)) {
      UnaryExpression *uexpr =
	to<UnaryExpression>(addr_expr);
      if (uexpr->get_opcode() == k_convert) {
	addr_expr = uexpr->get_source();
      }
    }
    break;
  }

  if (!is_kind_of<SymbolAddressExpression>(addr_expr)) { return NULL; }
  SymbolAddressExpression *ldc = to<SymbolAddressExpression>(addr_expr);
  Symbol *sym = ldc->get_addressed_symbol();
  if (!is_kind_of<ProcedureSymbol>(sym)) return NULL;
  ProcedureSymbol *ps_target = to<ProcedureSymbol>(sym);
  return(ps_target);
}

ProcedureSymbol *get_procedure_target_from_call(const CallStatement *st) {
  Expression *procedure_address = st->get_callee_address();
  return(get_procedure_from_address_op(procedure_address));
}

ProcedureSymbol *get_procedure_target_from_call(const ExecutionObject *call) {
  if (is_kind_of<CallStatement>(call)) {
    return(get_procedure_target_from_call(to<CallStatement>(call)));
  }
  return(NULL);
}

ProcedureDefinition *get_procedure_definition(const ExecutionObject *eo) {
  const SuifObject *so = eo;
  while (!is_kind_of<ProcedureDefinition>(so)) {
    so = so->get_parent();
    if (so == NULL) return NULL;
  }
  return(to<ProcedureDefinition>(so));
}

SuifObject *remove_suif_object(SuifObject *obj) {
  if(obj==NULL)return NULL;
  SuifObject *parent = obj->get_parent();
  if (parent == NULL) return obj;

  // A number of nodes can NOT be replaced like this.
  // We need a scheme to tell us which ones in the
  // hoof.

  if (is_kind_of<SymbolTableObject>(obj) &&
      is_kind_of<SymbolTableObject>(parent)) {
    
    SymbolTable *table = to<SymbolTable>(parent);
    SymbolTableObject *sto = to<SymbolTableObject>(obj);
    table->remove_symbol(sto);

  } else {

    obj->set_parent(NULL);
    parent->replace( obj, 0 );

  }
  return(obj);
}

Expression *replace_expression(Expression *expr,
			       Expression *replacement) {
  SuifObject *parent = expr->get_parent();
  
  suif_assert_message(replacement->get_parent() == NULL,
		      ("Attempt to replace an expression with an owned Expression"));
  replacement->set_parent(parent);
  expr->set_parent(0);
  if (parent != NULL)
    parent->replace( expr, replacement );
  return(expr);
}

Statement *replace_statement(Statement *stmt,
			     Statement *replacement) {
  if (replacement == NULL) {
    return(remove_statement(stmt));
  }
  SuifObject *parent = stmt->get_parent();
  suif_assert_message(parent != NULL, 
		      ("Attempt to replace a statement %s that is not atached",
		       stmt->getClassName().c_str()));
  //  if (parent == NULL) return obj;
  suif_assert(replacement->get_parent() == NULL);
  replacement->set_parent(parent);
  stmt->set_parent(0);
  parent->replace( stmt, replacement );
  return(stmt);
}


/*
void remove_source_op(const SourceOp &op) {
  if (op.is_expression()) {
    Expression *e = op.get_expression();
    remove_suif_object(e);
  }
}
*/


static Statement *insert_statement(Statement *loc,
				   Statement *the_statement,
				   bool is_after) {
  suif_assert(loc && the_statement);
  SuifObject *par = loc->get_parent();
  //  Statement *retval = NULL;

  // When inserting statement lists with NO annotations,
  // it's OK to dismantle the statement list.
  // Cases to handle
  // 1) loc is a StatementList,
  //      insert the_statement in loc.
  // 2) loc is owned by a StatementList
  //      insert the_statement into the list
  // 3) loc is not a Statement list
  //      create a new Statement list, put the loc and the_statement in.

  if (is_a<StatementList>(loc)) {
    StatementList *list = to<StatementList>(loc);
    if (is_after)
      list->append_statement(the_statement);
    else
      list->insert_statement(0, the_statement);
    return(list);
  }

  if (is_a<StatementList>(loc->get_parent())) {
    StatementList *list = to<StatementList>(loc->get_parent());

    for (int i = 0; i < list->get_statement_count(); i++) {
      if (list->get_statement(i) == loc) {
	if (is_after) {
	  list->insert_statement(i+1, the_statement);
	} else {
	  list->insert_statement(i, the_statement);
	}
	return(list);
      }
    }
    suif_assert_message(0, 
			("Could not find the statement in its owning list"));
  }

  StatementList *list = create_statement_list(loc->get_suif_env());
  if (par != NULL) {
    replace_statement(loc, list);
  }
  if (is_after) {
    list->append_statement(loc);
    list->append_statement(the_statement);
  } else {
    list->append_statement(the_statement);
    list->append_statement(loc);
  }
//  return(loc); commented out by Betul
  return(list);	// added by Betul
}

Statement *remove_statement(Statement *loc) {
  if (loc == NULL) return(NULL);
  SuifObject *par = loc->get_parent();
  if (is_kind_of<StatementList>(par)) {
    StatementList *l = to<StatementList>(par);
    for (int pos = 0; pos < l->get_statement_count(); pos++) {
      if (l->get_statement(pos) == loc) {
	l->remove_statement(pos);
	return(loc);
      }
    }
    suif_assert_message(0, ("statement not contained in the parent list"));
  }
  // we don't like null statements.  Put in a statement list.
  StatementList *l = create_statement_list(loc->get_suif_env());
  return(replace_statement(loc, l));
}


Statement *insert_statement_before(Statement *loc,
				   Statement *the_statement) {
  return(insert_statement(loc, the_statement, false));
}

Statement *insert_statement_after(Statement *loc,
			    Statement *the_statement) {
  return(insert_statement(loc, the_statement, true));
}


Statement *get_expression_owner(Expression *expr) {
  if (expr == NULL) return(NULL);
  SuifObject *par;
  for (par = expr->get_parent(); 
       par && is_kind_of<Expression>(par); par = par->get_parent());
  if (par == NULL) return(NULL);
  if (is_kind_of<Statement>(par)) return(to<Statement>(par));
  return(NULL);
}

bool is_target_label(CodeLabelSymbol *target_label,
		     Statement *st) {
  // If the address was taken, return the conservative result: TRUE
  if (target_label->get_is_address_taken()) 
    return(true);

  // iterate over all the owned objects
  for (Iter<Statement> iter = object_iterator<Statement>(st);
       iter.is_valid(); iter.next()) {
    Statement *sub_st = &iter.current();
    // check for target labels
    
#if 0
    // we can not use this code until we add the virtual target_label
    // field.
    for (Iter<CodeLabelSymbol *> code_iter = 
	   sub_st->get_target_label_iterator();
	 code_iter.is_valid(); code_iter.next()) {
      CodeLabelSymbol *lab = code_iter.current();
      if (lab == target_label)
	return(true);
    }
#endif
    if (is_kind_of<JumpStatement>(sub_st)) {
      JumpStatement *br = to<JumpStatement>(sub_st);
      if (br->get_target() == target_label)
	return(true);
    }
    if (is_kind_of<BranchStatement>(sub_st)) {
      BranchStatement *br = to<BranchStatement>(sub_st);
      if (br->get_target() == target_label)
	return(true);
    }
    // special check for multi-way branches until it
    // participates in the target_label.
    if (is_kind_of<MultiWayBranchStatement>(sub_st)) {
      MultiWayBranchStatement *mwb = to<MultiWayBranchStatement>(sub_st);
      if (mwb->get_default_target() == target_label)
	return(true);
      for (Iter<MultiWayBranchStatement::case_pair> case_iter =
	     mwb->get_case_iterator();
	   case_iter.is_valid();
	   case_iter.next()) {
	MultiWayBranchStatement::case_pair &pair = case_iter.current();
	if (pair.second == target_label) return(true);
      }
    }
  }
  return(false);
}

// These should become a visitor pattern.
void insert_statement_after_loop_body(WhileStatement *the_loop,
				      Statement *the_statement)
{
  if (the_loop->get_body() == NULL)
    {
      // because there was no body, it is safe to just add
      // the store as the body without creating a new label.
      the_loop->set_body(the_statement);
      return;
    }
  
  CodeLabelSymbol *old_cont_lab = the_loop->get_continue_label();
  // Check to see if there are any potential
  // branches to the label from inside the while loop.
  if (is_target_label(old_cont_lab, the_loop)) {
    // create a new CodeLabel and LabelLocation
    // place the label in the same scope as the old label
    CodeLabelSymbol *new_cont_lab = 
      create_code_label_symbol(old_cont_lab->get_suif_env(),
			       (LabelType*)old_cont_lab->get_type());
    LabelLocationStatement *label_statement =
      create_label_location_statement(old_cont_lab->get_suif_env(),
				      old_cont_lab);
    SymbolTable *symtab = old_cont_lab->get_symbol_table();
    suif_assert_message(symtab != NULL, ("invalid label parent"));
    symtab->add_symbol(new_cont_lab);

    // Add the continue label and the cloned store
    insert_statement_after(the_loop->get_body(), label_statement);
    // replace the loop continue label with the new one
    the_loop->set_continue_label(new_cont_lab);
  }
  // always stick the cloned store at the end of the body
  insert_statement_after(the_loop->get_body(), the_statement);
}

// These should become a visitor pattern.
void insert_statement_after_loop_body(DoWhileStatement *the_loop,
				   Statement *the_statement)
{
  if (the_loop->get_body() == NULL)
    {
      // because there was no body, it is safe to just add
      // the store as the body without creating a new label.
      the_loop->set_body(the_statement);
      return;
    }
  
  CodeLabelSymbol *old_cont_lab = the_loop->get_continue_label();
  // Check to see if there are any potential
  // branches to the label from inside the while loop.
  if (is_target_label(old_cont_lab, the_loop)) {
    // create a new CodeLabel and LabelLocation
    // place the label in the same scope as the old label
    CodeLabelSymbol *new_cont_lab = 
      create_code_label_symbol(old_cont_lab->get_suif_env(),
			       (LabelType*)old_cont_lab->get_type());
    LabelLocationStatement *label_statement =
      create_label_location_statement(old_cont_lab->get_suif_env(),
				      old_cont_lab);
    SymbolTable *symtab = old_cont_lab->get_symbol_table();
    suif_assert_message(symtab != NULL, ("invalid label parent"));
    symtab->add_symbol(new_cont_lab);

    // Add the continue label and the cloned store
    insert_statement_after(the_loop->get_body(), label_statement);
    // replace the loop continue label with the new one
    the_loop->set_continue_label(new_cont_lab);
  }
  // always stick the cloned store at the end of the body
  insert_statement_after(the_loop->get_body(), the_statement);
}

// These should become a visitor pattern.
void insert_statement_after_loop_body(ForStatement *the_loop,
				      Statement *the_statement)
{
  if (the_loop->get_body() == NULL)
    {
      // because there was no body, it is safe to just add
      // the store as the body without creating a new label.
      the_loop->set_body(the_statement);
      return;
    }
  
  CodeLabelSymbol *old_cont_lab = the_loop->get_continue_label();
  // Check to see if there are any potential
  // branches to the label from inside the while loop.
  if (is_target_label(old_cont_lab, the_loop)) {
    // create a new CodeLabel and LabelLocation
    // place the label in the same scope as the old label
    CodeLabelSymbol *new_cont_lab = 
      create_code_label_symbol(old_cont_lab->get_suif_env(),
			       (LabelType*)old_cont_lab->get_type());
    LabelLocationStatement *label_statement =
      create_label_location_statement(old_cont_lab->get_suif_env(),
				      old_cont_lab);
    SymbolTable *symtab = old_cont_lab->get_symbol_table();
    suif_assert_message(symtab != NULL, ("invalid label parent"));
    symtab->add_symbol(new_cont_lab);

    // Add the continue label and the cloned store
    insert_statement_after(the_loop->get_body(), label_statement);
    // replace the loop continue label with the new one
    the_loop->set_continue_label(new_cont_lab);
  }
  // always stick the cloned store at the end of the body
  insert_statement_after(the_loop->get_body(), the_statement);
}

void force_dest_not_expr(Expression *expr,
			 list<StoreVariableStatement*> &dismantled) {
  list<StoreVariableStatement*> *l = force_dest_not_expr(expr);
  if (!l) return;
  for (list<StoreVariableStatement*>::iterator iter = l->begin(); 
       iter != l->end(); iter++) {
    dismantled.push_back(*iter);
  }
  delete l;
}

list<StoreVariableStatement*> *force_dest_not_expr(Expression *expr) {

  ExecutionObject *par = 
    to<ExecutionObject>(expr->get_parent()); // the dest
  if (par == NULL) return(NULL);
  Statement *loc = get_expression_owner(expr);
  if (loc == NULL) {
    suif_warning(expr, "force_dest_not_expr: Attempt to disconnect "
		 "an unowned expression");
    return(NULL);
  }
  
  if (is_kind_of<StoreVariableStatement>(par)) {
    
    list<StoreVariableStatement*> *store_list =
      new list<StoreVariableStatement*>;
    store_list->push_back(to<StoreVariableStatement>(par));
    return(store_list);
  }
  if (is_kind_of<EvalStatement>(par)) return(NULL);

  list<StoreVariableStatement*> *store_list =
    new list<StoreVariableStatement*>;

  // any other execution object and we can just do a replace.
  if (is_kind_of<ExecutionObject>(par)) {
    //ExecutionObject *dst = to<ExecutionObject>(par);
    
    VariableSymbol *var =
      new_unique_variable(par->get_suif_env(), par,
			  retrieve_qualified_type(expr->get_result_type()));

    // Now we need to create a new LoadVariableExpression
    // and a StoreVariableStatement
    LoadVariableExpression *load = create_var_use(var);
    replace_expression(expr, load);

    StoreVariableStatement *store = 
      create_store_variable_statement(par->get_suif_env(),
				      var, expr);
    store_list->push_back(store);

    // There are 3 cases we have to deal with here.
    // Obviously, we should set up a visitor pattern to
    // handle others.
    //
    // 1) Most statements just place the Store before the statement
    // 2) The DoWhile places the store just after its body
    //     and handles the case where the continue label for the
    //     loop is used.
    // 3) The WhileStatement must clone the store
    //    place 1 before the while statement and the second goes
    //    at the end of the do_while body.
    //

    // place the store before the loc
    // If the statement is a do_while, place the statement
    // at the end of the do_while.
    if (is_kind_of<DoWhileStatement>(loc)) {
      // do {
      //    continue;  // i.e. goto cont_lab;
      // } while(foo())
      //
      // converts to
      //
      // do {         // New continue label
      //    goto cont_lab;
      // cont_lab:
      //    tmp = foo();
      // } while (tmp)
      DoWhileStatement *the_while = to<DoWhileStatement>(loc);
      insert_statement_after_loop_body(the_while, store);
    } else {
      insert_statement_before(loc, store);
    }
    if (is_kind_of<WhileStatement>(loc)) {
      // while(foo()) {
      //    continue;  // i.e. goto cont_lab;
      // }
      //
      // converts to
      //
      // tmp = foo();
      // while(tmp) {  // New continue label
      //    goto cont_lab;
      // cont_lab:
      //    tmp = foo();
      // }
      StoreVariableStatement *store_clone = 
	deep_suif_clone<StoreVariableStatement>(store);

      store_list->push_back(store_clone);
      
      WhileStatement *the_while = to<WhileStatement>(loc);
      insert_statement_after_loop_body(the_while, store_clone);
    }
    return(store_list);
  } else {
    suif_assert_message(0, ("Unknown parent of expression"));
  }
  delete store_list;
  return(NULL);
}
    
bool is_void_dest(Expression *expr) {
  return(is_kind_of<EvalStatement>(to<ExecutionObject>(expr->get_parent())));
}

Statement *build_assign(VariableSymbol *dst, Expression *expr) {
  suif_assert(dst->get_suif_env() == expr->get_suif_env());
  DataType *dst_type = unqualify_data_type(dst->get_type());
  expr = build_cast(expr, dst_type);
  StoreVariableStatement *store = 
    create_store_variable_statement(dst->get_suif_env(), dst, expr);
  return(store);
}

Statement *build_assign(VariableSymbol *dst, VariableSymbol *src) {
  Expression *expr = create_var_use(src);
  return(build_assign(dst, expr));
}


Expression *build_cast(Expression *expr, DataType *t) {
  if (expr->get_result_type() == t) { return(expr); }
  UnaryExpression *cvt = 
    create_unary_expression(expr->get_suif_env(), t, k_convert, expr);
  return(cvt);
}


// If target is NULL, read the target from the direct call re
extern void inline_call(CallStatement *the_call, 
			ProcedureSymbol *target_proc)
  {
    suif_assert(the_call != NULL);
    SuifEnv *s = the_call->get_suif_env();
    VariableSymbol *result_var = NULL;
    CodeLabelSymbol *return_label = NULL;

    ProcedureSymbol *the_proc = get_procedure_target_from_call(the_call);
    // if it's not a direct call, assume that the user got it correct.
    // if it is a direct call, the use should have specified
    if (target_proc == NULL) { target_proc = the_proc; }
    if (the_proc != NULL) {
      suif_assert(the_proc == target_proc);
    }

    suif_assert(target_proc->get_definition());

    result_var = the_call->get_destination();

    // clone the body.
    ProcedureDefinition *proc_def_copy = 
      deep_suif_clone(target_proc->get_definition());
    
    SymbolTable *old_symtab = proc_def_copy->get_symbol_table();
    remove_suif_object(old_symtab);
    
    // @@ OOPS. The body is an ExecutionObject.
    // What do we do here for inlining a Machsuif thing?
    Statement *new_body = to<Statement>(proc_def_copy->get_body());
    remove_suif_object(new_body);
    //    proc_def_copy->set_body(NULL);
    DefinitionBlock *defs = proc_def_copy->get_definition_block();
    remove_suif_object(defs);

    ScopeStatement *new_block = create_scope_statement(s, 
						       new_body, 
						       old_symtab,
						       defs);

    // Build the list of parameter assignments
    unsigned param_num = 0;
    for (Iter<ParameterSymbol*> param_iter = 
	   proc_def_copy->get_formal_parameter_iterator();
	 param_iter.is_valid(); param_iter.next(), param_num++)
      {
        ParameterSymbol *this_param = param_iter.current();
        if (param_num < the_call->get_argument_count())
          {
	    // NO CALL BY REF PARAMETERS allowed (i.e fortran).
	    // Perhaps we could assert that here...
	    // ALSO, all of these parameter symbols
	    // need to be converted to variable symbols..
            Expression *this_actual = the_call->get_argument(param_num);
            remove_suif_object(this_actual);
	    insert_statement_before(new_body, 
				    build_assign(this_param, this_actual));
          }
      }


    // @@ Need to replace the parameter symbols with
    // "normal" variable symbols??
    delete proc_def_copy;

    //    insert_statement_before(new_block, the_call->owner());
    if (result_var != NULL)
      {
	if (is_kind_of<StoreVariableStatement>(the_call->get_parent())) {
	  VariableSymbol *var = 
	    to<StoreVariableStatement>(the_call->get_parent())->
	    get_destination();
	  insert_statement_after(new_block, 
				 build_assign(var, result_var));
	}
      }

    return_label = new_unique_label(s, new_body, "return");
    insert_statement_after(new_body,
			   create_label_location_statement(s, return_label));

    // Walk through this code (in new_body)
    // If we find a return, then we need to dismantle all of the
    // nested statements that are inside of it.
    // Let's do this the easy suif2 way:
    // build a list of the ReturnStatements.
    // Iterate over them and dismantle any SCOPE/FOR/DO/WHILE
    // that contain them..
    list<ReturnStatement *> *return_list = 
      collect_objects<ReturnStatement>(new_body);
	{for (list<ReturnStatement*>::iterator riter = return_list->begin();
	 riter != return_list->end();  riter++) {
      //      ReturnStatement *ret = *riter;
      //
      // @@@ don't do this for now.  We'll use simple examples.
      //      dismantle_outers(ret);
    }}

    for (list<ReturnStatement*>::iterator riter = return_list->begin();
	 riter != return_list->end();  riter++) {
      ReturnStatement *ret = *riter;
      JumpStatement *jump = create_jump_statement(s, return_label);
      Expression *return_expr = ret->get_return_value();
      if (return_expr == NULL) {
	replace_statement(ret, jump);
      } else {
	remove_suif_object(return_expr);

	StatementList *new_list = create_statement_list(s);
	Statement *ret_assign = build_assign(result_var,
					     return_expr);
	new_list->append_statement(ret_assign);
	new_list->append_statement(jump);
	replace_statement(ret, jump);
      }
      delete ret;   // again. really just need to trash it.
    }
    delete return_list;

    // Now we need to change all of the returns to 
    // an assignment of the result variable and a GOTO
    // the label.
#ifdef CALL_EXP
    Statement *old_call_statement = get_expression_owner(the_call);
    replace_statement(old_call_statement, 
		      new_block);
    delete old_call_statement;  // or trash it.
#else
    replace_statement(the_call, new_block);
    delete the_call;
#endif
    
}

static bool is_constant_exp(Expression *exp) {
    return is_kind_of<IntConstant>(exp);
}

Expression *clone_if_needed(Expression *e) {
    if (!e->get_parent())
	return e;
    return to<Expression>(e->deep_clone());
}

static void delete_if_unused(Expression *e) {
    if (!e->get_parent())
        delete e;
}

Expression * build_dyadic_expression(LString op,Expression *left,Expression *right) {
    SuifEnv *env = left->get_suif_env();
    DataType *result_type = left->get_result_type();
    suif_assert(result_type);
    if (is_constant_exp( left) && is_constant_exp(right)) {
	IntConstant * cleft = to<IntConstant>(left);
	IntConstant * cright = to<IntConstant>(right);
	if (op == k_add) {
	    Expression *exp = create_int_constant(
					env,
					result_type,
					cleft->get_value() + cright->get_value());
	    delete_if_unused(left);
	    delete_if_unused(right);
	    return exp;
	    }
        if (op == k_subtract) {
            Expression *exp = create_int_constant(
                                        env,
                                        result_type,
                                        cleft->get_value() - cright->get_value());
            delete_if_unused(left);
            delete_if_unused(right);
            return exp;
            }
        if (op == k_multiply) {
            Expression *exp = create_int_constant(
                                        env,
                                        result_type,
                                        cleft->get_value() * cright->get_value());
            delete_if_unused(left);
            delete_if_unused(right);
            return exp;
            }
        if (op == k_divide && (cright->get_value() != 0)) {
            Expression *exp = create_int_constant(
                                        env,
                                        result_type,
                                        cleft->get_value() / cright->get_value());
            delete_if_unused(left);
            delete_if_unused(right);
            return exp;
            }


	}
    return create_binary_expression(	env,
					result_type,
					op,
					clone_if_needed(left),
					clone_if_needed(right));
}

Expression * build_monadic_expression(LString op,Expression *subexp) {
    SuifEnv *env = subexp->get_suif_env();
    if (is_constant_exp(subexp)) {
	if (op == k_negate) {
	    IntConstant * csubexp = to<IntConstant>(subexp);
	    Expression * exp = create_int_constant(
				env,
				subexp->get_result_type(),
				-csubexp->get_value());
	    delete_if_unused(subexp);
	    return exp;
	    }
	}
    return create_unary_expression(	env,
					subexp->get_result_type(),
					op,
					clone_if_needed(subexp));
}

Expression *fold_and_replace_constants(Expression *expr) {
  SuifEnv *s = expr->get_suif_env();
  FoldTable *fold_table = FoldTable::get_fold_table(s);
  return(fold_table->fold_and_replace_expression(expr));
}

Statement *fold_and_replace_constants(Statement *stmt) {
  SuifEnv *s = stmt->get_suif_env();
  FoldTable *fold_table = FoldTable::get_fold_table(s);
  return(fold_table->fold_and_replace_statement(stmt));
}

void add_targets(Statement* the_stmt, list<CodeLabelSymbol*>& targets){
    /* Jumps -- recurse on targets */
    if(is_kind_of<JumpStatement>(the_stmt)){
        JumpStatement* the_jump = to<JumpStatement>(the_stmt);
        targets.push_back(the_jump->get_target());
    }else
    if(is_kind_of<JumpIndirectStatement>(the_stmt)){
        //JumpIndirectStatement* the_ji_stmt = to<JumpIndirectStatement>(the_ji_stmt);
        suif_assert_message(false, 
            ("JumpIndirectStatement is not supported yet, need to get this info from ptr analysis..."));
        //targets.push_back(the_ji_stmt->get_target());
    }else
    if(is_kind_of<BranchStatement>(the_stmt)){
        BranchStatement* the_branch = to<BranchStatement>(the_stmt);
        targets.push_back(the_branch->get_target());
    }else
    if(is_kind_of<MultiWayBranchStatement>(the_stmt)){
        typedef indexed_list<class IInteger,class CodeLabelSymbol*>::pair case_pair;

        MultiWayBranchStatement* the_mw_branch = to<MultiWayBranchStatement>(the_stmt);

        for(Iter<case_pair> it = the_mw_branch->get_case_iterator();it.is_valid();it.next()){
            CodeLabelSymbol* label = the_mw_branch->lookup_case(it.current().first);
            targets.push_back(label);
        }
    }
}

void get_jumps_from_outside(ScopedObject* scope, searchable_list<CodeLabelSymbol*>& targets){
	// this might not be the most efficient way of doing things, btw.
    SuifObject* obj = scope;
    while(!is_kind_of<ProcedureDefinition>(obj)) obj = obj->get_parent();
    suif_assert_message(obj, ("Scope not inside of a procedure"));
    ProcedureDefinition* proc_def = to<ProcedureDefinition>(obj);

    // Collect all possible jumps from outside of this scope into targets
    {for(Iter<Statement> iter = object_iterator<Statement>(proc_def); 
        iter.is_valid(); iter.next())
    {
        Statement* stmt = &iter.current();
        obj = stmt;
        while(!is_kind_of<ProcedureDefinition>(obj) && obj!=scope){
			obj = obj->get_parent();
		}
        
        if(obj!=scope){
            add_targets(stmt, targets);
        }
    }}
};

bool has_jumps_going_inside(ScopedObject* scope){
    searchable_list<CodeLabelSymbol*> targets;
    
    get_jumps_from_outside(scope, targets);

    // Walk through the local labels and see if there's a match
    {for(Iter<LabelLocationStatement> iter = object_iterator<LabelLocationStatement>(scope); 
        iter.is_valid(); iter.next())
    {
        LabelLocationStatement* current = &iter.current();

		if(targets.is_member(current->get_defined_label())){
			// jump inside detected
			return true;
		}
    }}
    return false;
};

Expression *clone_expression(Expression *expr) {
  return(deep_suif_clone<Expression>(expr));
}
