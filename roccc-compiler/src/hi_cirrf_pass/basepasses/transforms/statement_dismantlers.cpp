// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
/* FILE "statement_dismantlers.cpp" */


/*
       Copyright (c) 1998,2000 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>
#include <utils/trash_utils.h>
#include <utils/statement_utils.h>

#include "iokernel/cast.h"

#include "suifkernel/suif_env.h"

#include "suifkernel/utilities.h"
#include "suifkernel/suifkernel_messages.h"

#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"
#include "basicnodes/basic_constants.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "cfenodes/cfe.h"
#include "cfenodes/cfe_factory.h"

#include "procedure_walker_utilities.h"
#include "utils/expression_utils.h"
#include "utils/cloning_utils.h"
#include "utils/type_utils.h"
#include "utils/symbol_utils.h"
#include "typebuilder/type_builder.h"
#include "statement_dismantlers.h"
#include "common/suif_list.h"
#include "suifkernel/command_line_parsing.h"

#include "common/i_integer.h"
#include "utils/type_utils.h"

static void append_flattened(StatementList *slist,Statement *stat) {
    if (!stat)
	return;
    if (is_kind_of<StatementList>(stat)) {
	StatementList *sl = to<StatementList>(stat);
	int len = sl->get_statement_count();
	for (int i = 0; i < len; i++) {
	    Statement *s = sl->get_statement(i);
	    s->set_parent(0);
	    slist->append_statement(s);
	    }
	}
    else {
	stat->set_parent(0);
	slist->append_statement(stat);
	}
    }

if_statement_walker::if_statement_walker(SuifEnv *the_env,ProcedureDefinition *def)
                : ProcedureWalker(the_env,def,IfStatement::get_class_name()) {}

Walker::ApplyStatus if_statement_walker::operator () (SuifObject *x){
    SuifObject *result = dismantle_if_statement(to<IfStatement>(x));
    set_address(result);
    return Walker::Replaced;
}

Statement* if_statement_walker::dismantle_if_statement(IfStatement* the_if){
    Statement *result = 0;
    ProcedureDefinition* proc_def = get_procedure_definition(the_if);

    Expression *condition = the_if->get_condition();
    // we are going to reuse the condition so we must remove it
    // from the if statement first to avoid ownership conflicts

    remove_suif_object(condition);
    //    the_if->set_condition(SourceOp());
    //    remove_source_op(condition);

    Statement *then_part = the_if->get_then_part();
    Statement *else_part = the_if->get_else_part();

    // likewise of these parts - remove from if.

    the_if->set_then_part(0);
    the_if->set_else_part(0);
    remove_suif_object(then_part);
    remove_suif_object(else_part);

    if (then_part != 0) {
    StatementList *replacement = create_statement_list( the_if->get_suif_env() );
	result = replacement;
    UnaryExpression* negated_condition = 
    create_unary_expression( the_if->get_suif_env(),
			      condition->get_result_type(),
			      k_logical_not,
			      condition );
	if (else_part != 0) {
	    CodeLabelSymbol *else_label = new_anonymous_label(proc_def->get_symbol_table());
	    CodeLabelSymbol *done_label = new_anonymous_label(proc_def->get_symbol_table());
	    replacement->append_statement(create_branch_statement(the_if->get_suif_env(),
                                                 negated_condition, else_label));
	    replacement->append_statement(then_part);
	    replacement->append_statement(create_jump_statement(the_if->get_suif_env(),done_label));
	    replacement->append_statement(create_label_location_statement(the_if->get_suif_env(), else_label));
	    replacement->append_statement(else_part);
	    replacement->append_statement(create_label_location_statement(the_if->get_suif_env(), done_label));
	    }
	else {
            CodeLabelSymbol *done_label = create_new_label(get_procedure_definition(the_if));
            replacement-> append_statement(create_branch_statement(the_if->get_suif_env(),
                                                 negated_condition, done_label));
            replacement->append_statement(then_part);
            replacement-> append_statement(create_label_location_statement(the_if->get_suif_env(), done_label));
	    }
	}
    else {
	if (else_part != 0)
	    {
	    StatementList *replacement = create_statement_list(the_if->get_suif_env());
	    result = replacement;
       	    CodeLabelSymbol *done_label = create_new_label(proc_def);
            replacement->append_statement(create_branch_statement(the_if->get_suif_env(), condition, done_label));
            replacement->append_statement(else_part);
            replacement-> append_statement(create_label_location_statement(the_if->get_suif_env(), done_label));
	    }
	else
	    {
	    EvalStatement *replacement = create_eval_statement(the_if->get_suif_env());
	    result = replacement;
	    replacement->append_expression(condition);
	    }
	}

    the_if->get_parent()->replace(the_if, result);
    return result;
}

while_statement_walker::while_statement_walker(SuifEnv *the_env,ProcedureDefinition *def)
	: ProcedureWalker(the_env,def,WhileStatement::get_class_name()) {}

Walker::ApplyStatus while_statement_walker::operator () (SuifObject *x){
    SuifObject *result = dismantle_while_statement(to<WhileStatement>(x));
    set_address(result);
    return Walker::Replaced;
}

Statement *while_statement_walker::dismantle_while_statement(WhileStatement *the_while){
    Statement *result = 0;

    Expression *condition = the_while->get_condition();

    // we are going to reuse the condition so we must remove it
    // from the if statement first to avoid ownership conflicts

    //    the_while->set_condition(SourceOp());
    //    remove_source_op(condition);
    remove_suif_object(condition);

    Statement *body = the_while->get_body();
    CodeLabelSymbol *break_label = the_while->get_break_label();
    CodeLabelSymbol *continue_label = the_while->get_continue_label();

    // likewise of these parts - remove from if.

    the_while->set_body(0);
    the_while->set_break_label(0);
    the_while->set_continue_label(0);

    if (body != 0) {
        StatementList *replacement = create_statement_list(body->get_suif_env());
        result = replacement;
        suif_assert(continue_label != 0);
        UnaryExpression* negated_condition = 
	    create_unary_expression(body->get_suif_env(),
				   condition->get_result_type(),
				   k_logical_not,
				   condition );
        replacement-> append_statement(create_label_location_statement(body->get_suif_env(), continue_label));
        replacement-> append_statement(create_branch_statement(body->get_suif_env(),
                                                 negated_condition, break_label));
        replacement->append_statement(body);

        suif_assert(break_label != 0);
        replacement->append_statement(create_jump_statement(body->get_suif_env(),continue_label));
        replacement-> append_statement(create_label_location_statement(body->get_suif_env(), break_label));
        }

    the_while->get_parent()->replace(the_while,result);
    return result;
}

do_while_statement_walker::do_while_statement_walker(SuifEnv *the_env,ProcedureDefinition *def)
	: ProcedureWalker(the_env,def,DoWhileStatement::get_class_name()) {}

Walker::ApplyStatus do_while_statement_walker::operator () (SuifObject *x){
    SuifObject *result = dismantle_do_while_statement(to<DoWhileStatement>(x));
    set_address(result);
    return Walker::Replaced;
}

Statement *do_while_statement_walker::dismantle_do_while_statement(DoWhileStatement *the_do_while){
    Statement *result = 0;

    Expression *condition = the_do_while->get_condition();

    // we are going to reuse the condition so we must remove it
    // from the if statement first to avoid ownership conflicts

    remove_suif_object(condition);

    Statement *body = the_do_while->get_body();
    CodeLabelSymbol *break_label = the_do_while->get_break_label();
    CodeLabelSymbol *continue_label = the_do_while->get_continue_label();

    // likewise of these parts - remove from if.

    the_do_while->set_body(0);
    remove_suif_object(body);
    the_do_while->set_break_label(0);
    the_do_while->set_continue_label(0);

    if (body != 0) {
        StatementList *replacement = create_statement_list(body->get_suif_env());
        result = replacement;
        suif_assert(continue_label != 0);
        replacement-> append_statement(create_label_location_statement(body->get_suif_env(), continue_label));
	replacement->append_statement(body);
        replacement-> append_statement(create_branch_statement(body->get_suif_env(),
                                                 condition, continue_label));
        suif_assert(break_label != 0);
        replacement-> append_statement(create_label_location_statement(body->get_suif_env(), break_label));
        }

    the_do_while->get_parent()->replace(the_do_while,result);
    return result;
}

//  Rename a symbol-table object if its name is already taken by an entry
//  in the symbol table (new_st) it's about be added to.

void
rename_if_collision(SymbolTableObject *sto, SymbolTable *new_st)
{
    LString orig = sto->get_name();
    if (orig == emptyLString || !new_st->has_lookup_table_member(orig))
      return;

    for (IInteger serial = 0; ; ++serial) {
      String suffix;
      serial.write(suffix, 10);
      LString trial(orig + suffix);
      if (!new_st->has_lookup_table_member(trial)) {
          sto->set_name(trial);
          return;
      }
    }
}

scope_statement_walker::scope_statement_walker(SuifEnv *the_env,ProcedureDefinition *def)
        : ProcedureWalker(the_env,def,ScopeStatement::get_class_name()) {}

Walker::ApplyStatus scope_statement_walker::operator () (SuifObject *x) {
    SuifObject *result = dismantle_scope_statement(to<ScopeStatement>(x));
    set_address(result);
    return Walker::Replaced;
}

Statement* scope_statement_walker::dismantle_scope_statement(ScopeStatement *the_scope_stat){
    ProcedureDefinition* proc_def = get_procedure_definition(the_scope_stat);
    Statement *body = the_scope_stat->get_body();
    if(body==NULL){
        the_scope_stat->print_to_default();
    }
    body->set_parent(0);
    the_scope_stat->set_body(0);

    // This is a bug?
    //    remove_suif_object(body);

    SymbolTable * symbol_table = the_scope_stat->get_symbol_table();
    DefinitionBlock * definition_block = the_scope_stat->get_definition_block();

    SymbolTable *new_symbol_table = proc_def->get_symbol_table();
    DefinitionBlock *new_definition_block = proc_def->get_definition_block();

    if (symbol_table != 0) {

	// start by creating a name for the symbol in the new symbol table
    Iter<SymbolTable::lookup_table_pair> piter = symbol_table->get_lookup_table_iterator();
	while (piter.is_valid()) {
	    indexed_list<LString,SymbolTableObject*>::pair p = piter.current();
	    SymbolTableObject * obj = p.second;
	    const LString &name = p.first;
	    new_symbol_table->add_lookup_table(name,obj);
	    piter.next();
    }

    // now move all symbols into the symbol table for the procedure scope
	// at the same time, we delete them from the current symbol table and
	// remove all references to this symbol table from the name list attached
	// to the symbol
	// DLH 
	//    I modifed this to build a list so we aren't iterating over
	// a changing object (which didn't work. and I don't expect it to)

	list<SymbolTableObject*> l;
    {for (Iter<SymbolTableObject*> iter = 
	       symbol_table->get_symbol_table_object_iterator();
	     iter.is_valid(); iter.next()) {
	    l.push_back(iter.current());
    }}

	for (list<SymbolTableObject*>::iterator iter =
	       l.begin(); iter != l.end(); iter++) 
    {
	    SymbolTableObject *object = *iter;
	    symbol_table->remove_symbol_table_object(object);
            rename_if_collision(object, new_symbol_table);
            new_symbol_table->add_symbol(object);
	    //	    symbol_table->remove_all_from_lookup_table(object);
	    //	    object->remove_all_from_name(symbol_table);
	    }
    }

    if (definition_block != 0) {
	// move all definition block entries
	    int i = definition_block->get_variable_definition_count();
	    while (i > 0){
	        i--;
	        VariableDefinition *next = definition_block->remove_variable_definition(i);
	        new_definition_block->append_variable_definition(next);
	    }

        i = definition_block->get_procedure_definition_count();
        while (i > 0){
            i--;
            ProcedureDefinition *next = definition_block->remove_procedure_definition(i);
            new_definition_block->append_procedure_definition(next);
        }
    }
    the_scope_stat->get_parent()->replace(the_scope_stat, body);
    return body;
}

for_statement_walker::for_statement_walker(SuifEnv *the_env,ProcedureDefinition *def)
   	: ProcedureWalker(the_env,def, ForStatement::get_class_name()) {}

//	This is just a first cut and has not yet been compared with the old code

Walker::ApplyStatus for_statement_walker::operator () (SuifObject *x) {
    SuifObject *result = dismantle_for_statement(to<ForStatement>(x));
    set_address(result);
    return Walker::Replaced;
}

Statement *for_statement_walker::dismantle_for_statement(ForStatement *the_for){
    StatementList *replacement = create_statement_list(the_for->get_suif_env());
    VariableSymbol*  index = the_for->get_index();
    DataType *type = unqualify_data_type(index->get_type());
    Expression *lower = the_for->get_lower_bound();
    Expression *upper = the_for->get_upper_bound();
    Expression *step = the_for->get_step();
    LString compare_op = the_for->get_comparison_opcode();
    Statement* body =  the_for->get_body();
    Statement* pre_pad = the_for->get_pre_pad();
//    Statement* post_pad = the_for->get_post_pad();
    CodeLabelSymbol* break_lab = the_for->get_break_label();
    CodeLabelSymbol* continue_lab = the_for->get_continue_label();
    the_for->set_index(0);
    remove_suif_object(lower);
    remove_suif_object(upper);
    remove_suif_object(step);
    remove_suif_object(body);
    remove_suif_object(pre_pad);
//    the_for->set_post_pad(0);
//    remove_suif_object(post_pad);
    the_for->set_break_label(0);
    the_for->set_continue_label(0);

    // I am guessing what pre-pad and post-pad do

    if(pre_pad != 0)replacement->append_statement(pre_pad);

    // initialize the index. Is this right? should we ever initialize to upper, for -ve steps?
    // Is index guaranteed not to be changed? Should we be creating a temporary?

    replacement->append_statement(create_store_variable_statement(body->get_suif_env(),index,lower));

    replacement->append_statement(create_label_location_statement(body->get_suif_env(), continue_lab));

    if (body != 0)
	replacement->append_statement(body);

    // increment the counter

    Expression *index_expr = 
      create_load_variable_expression(body->get_suif_env(),
				      unqualify_data_type(index->get_type()),
				      index);
    Expression *increment = 
      create_binary_expression(body->get_suif_env(),type,k_add,
			       index_expr,step);

    replacement->append_statement(create_store_variable_statement(body->get_suif_env(),index,increment));

    // and loop if not out of range

    Expression *compare =  
      create_binary_expression(body->get_suif_env(),type,
			       compare_op,
			       deep_suif_clone<Expression>(index_expr),
			       deep_suif_clone<Expression>(step));
    replacement->append_statement(create_branch_statement(body->get_suif_env(),compare,continue_lab));

    // end of loop

    replacement->append_statement(create_label_location_statement(body->get_suif_env(),break_lab));
//    if(post_pad != 0)replacement->append_statement(post_pad);
    the_for->get_parent()->replace(the_for,replacement);
    return replacement;
}

multi_way_branch_statement_walker::multi_way_branch_statement_walker(SuifEnv *the_env,ProcedureDefinition *def)
        : ProcedureWalker(the_env,def,MultiWayBranchStatement::get_class_name()) {}

Walker::ApplyStatus multi_way_branch_statement_walker::operator () (SuifObject *x){
    SuifObject *result = dismantle_multi_way_branch_statement(to<MultiWayBranchStatement>(x));
    set_address(result);
    return Walker::Replaced;
}

Statement* multi_way_branch_statement_walker::dismantle_multi_way_branch_statement(MultiWayBranchStatement *the_case){
    StatementList *replacement = create_statement_list(the_case->get_suif_env());

    Expression *operand = the_case->get_decision_operand ();
    remove_suif_object(operand);

    DataType *type = operand->get_result_type();

    CodeLabelSymbol *default_lab =  the_case->get_default_target();
    the_case->set_default_target(0);
    Iter<MultiWayBranchStatement::case_pair > iter = the_case->get_case_iterator();
    while (iter.is_valid()) {
	MultiWayBranchStatement::case_pair pair = iter.current();
	IInteger value = pair.first;
	CodeLabelSymbol *lab = pair.second;
	IntConstant *exp = create_int_constant(the_case->get_suif_env(),type, value);
//	Expression *exp = create_load_constant_expression(get_env(),type,iconst);
	TypeBuilder *type_builder = (TypeBuilder *)
          the_case->get_suif_env()->get_object_factory(TypeBuilder::get_class_name());
	Expression *compare =  
	  create_binary_expression(the_case->get_suif_env(),type_builder->get_boolean_type(),
				   k_is_equal_to,
				   deep_suif_clone(operand),
				   exp);
	replacement->append_statement(create_branch_statement(the_case->get_suif_env(),compare,lab));
	iter.next();
	}
    delete operand;
    replacement->append_statement(create_jump_statement(the_case->get_suif_env(),default_lab));
    the_case->get_parent()->replace(the_case,replacement);
    return replacement;
}

//	constants to control building of Multi Way Groups
const double required_density = 0.8;

class MultiWayGroup {
    public:
	MultiWayGroup(SuifEnv *env,IInteger value ,CodeLabelSymbol *lab,
		      CodeLabelSymbol *default_lab, VariableSymbol *decision);
	MultiWayGroup(const MultiWayGroup&);
	int get_label_count();
	void generate_code(StatementList *x,IInteger bound_below,IInteger bound_above);
	const IInteger &get_low_bound() {return _low_bound;}
	const IInteger &get_high_bound() {return _high_bound;}
	void merge(const MultiWayGroup &x);
	MultiWayBranchStatement *get_statement() {return _statement;}
	bool try_add_element(IInteger value ,CodeLabelSymbol *lab);
    private:
        MultiWayBranchStatement *_statement;
        IInteger _low_bound;
        IInteger _high_bound;
        MultiWayGroup *_low;
        MultiWayGroup *_high;
        SuifEnv *_env;

        Expression *ie(IInteger val);
        CodeLabelSymbol *_default_lab;
        VariableSymbol *_decision;
    };

class MultiWayGroupList {
    public:
        MultiWayGroupList(SuifEnv *env,CodeLabelSymbol *default_lab,VariableSymbol *decision);
        ~MultiWayGroupList();

	void generate_code(StatementList *x);
	void add_element(IInteger value ,CodeLabelSymbol *lab);
    private:

	void generate_code_subgroup(StatementList *x,IInteger bound_below,IInteger bound_above,
					int low,int high);
        list<MultiWayGroup *> _list;
        SuifEnv *_env;
        CodeLabelSymbol *_default_lab;
        VariableSymbol *_decision;
    };

MultiWayGroupList::MultiWayGroupList(SuifEnv *env,CodeLabelSymbol *default_lab,VariableSymbol *decision)
	: _env(env),_default_lab(default_lab),_decision(decision) {}


Expression *MultiWayGroup::ie(IInteger val) {
    return create_int_constant(_env,val);
    }

//	If bound_below (bound_above) is not indeterminate it is a value which the condition value
//	is known top be greater than (less than or equal)
void MultiWayGroupList::generate_code(StatementList *x) {

    // start by trying to merge blocks

    bool closed = false;
    while (!closed) {
	closed = true;
	size_t i = 0;
	while (i + 1 < _list.length()) {
	    MultiWayGroup *low = _list[i];
	    MultiWayGroup *high = _list[i + 1];
	    double density = (double)(low->get_label_count() + high->get_label_count())
			/(high->get_high_bound() - low->get_low_bound() + 1).c_double();
            if (density >= required_density) {
	 	low->merge(*high);
		delete high->get_statement();
		delete high;
		_list.erase(i + 1);
		closed = false;
		i++; // skip over element we just removed
		}
	    i ++;
	    }
	}
    generate_code_subgroup(x,IInteger(),IInteger(),0,_list.length() - 1);
    }

void MultiWayGroupList::generate_code_subgroup(	StatementList *x,
						IInteger bound_below,
						IInteger bound_above,
                                             	int low,int high) {
    if (low == high) {
	_list[low]->generate_code(x,bound_below,bound_above);
	return;
	}


    DataType *bool_type = get_type_builder(_env)->get_boolean_type();
    int i = (low + high) / 2;
    MultiWayGroup *middle = _list[i];

    IInteger divide_point = middle->get_high_bound();

    Expression *check = create_binary_expression(_env,
                                                 bool_type,
                                                 k_is_less_than_or_equal_to,
                                                 create_var_use(_decision),
                                                 create_int_constant(_env,divide_point));
    StatementList *low_part = create_statement_list(_env);
    StatementList *high_part = create_statement_list(_env);
    IfStatement *ifstat = create_if_statement(_env,check,low_part,high_part);
    x->append_statement(ifstat);
    generate_code_subgroup(low_part,bound_below,divide_point,low,i),
    generate_code_subgroup(high_part,divide_point,bound_above,i + 1,high);
    }

void MultiWayGroup::generate_code(StatementList *x,
				  IInteger bound_below,
                                  IInteger bound_above) {
    Expression *low_check = NULL;
    Expression *high_check = NULL;
    DataType *bool_type = get_type_builder(_env)->get_boolean_type();

    // if there are no more than two entries, build if statements

    int lab_count = get_label_count();
    if (lab_count <= 2) {
	for (int i=0;i < lab_count; i ++) {
	    MultiWayBranchStatement::case_pair pair = _statement->get_case(i);

	    low_check = create_binary_expression(_env,
                                                 bool_type,
                                                 k_is_equal_to,
                                                 create_var_use(_decision),
                                                 ie(pair.first));
            x->append_statement(create_branch_statement(_env,low_check,pair.second));
	    }
	x->append_statement(create_jump_statement(_env,_default_lab));
	delete _statement;
	_statement = 0;
	return;
	}
  	
    if (bound_below.is_undetermined() || (_low_bound > bound_below + 1)) {
	low_check = create_binary_expression(_env,
						 bool_type,
						 k_is_less_than,
						 create_var_use(_decision),
						 ie(_low_bound));
	x->append_statement(create_branch_statement(_env,low_check,_default_lab));
	}
    if (bound_above.is_undetermined() || (_high_bound < bound_above)) {
        high_check = create_binary_expression(_env,
                                                 bool_type,
                                                 k_is_greater_than,
                                                 create_var_use(_decision),
                                                 ie(_high_bound));
	x->append_statement(create_branch_statement(_env,high_check,_default_lab));
        }

    // in fill the statement so that missing entries are added so as to 
    // complete the table

    IInteger next_value = _low_bound;
    while (next_value < _high_bound) {
	if (!_statement->lookup_case(next_value)) {
	    _statement->insert_case(next_value,_default_lab);
	    }	
	next_value ++;
	}
    
    x->append_statement(_statement);
    }

MultiWayGroupList::~MultiWayGroupList() {
    for (size_t i = 0; i < _list.length(); i ++) {
	delete _list[i];
	}
    }

MultiWayGroup::MultiWayGroup(SuifEnv *env,
			     IInteger value ,
			     CodeLabelSymbol *lab,
			     CodeLabelSymbol *default_lab,
                             VariableSymbol *decision) 
	:_low_bound(value),_high_bound(value),
	 _low(0),_high(0),_env(env),
	 _default_lab(default_lab),_decision(decision) {
    _statement = create_multi_way_branch_statement(env,create_var_use(decision),default_lab);
    _statement->insert_case(value,lab);
    }

MultiWayGroup::MultiWayGroup(const MultiWayGroup &x) :
	_statement(x._statement),
	_low_bound(x._low_bound),
	_high_bound(x._high_bound),
	_low(x._low),
	_high(x._high),
	_env(x._env),
	_default_lab(x._default_lab),
	_decision(x._decision) {}

int MultiWayGroup::get_label_count() {
    if (!_statement)
	return _low->get_label_count() + _high->get_label_count();
    return _statement->get_case_count();
    }

bool MultiWayGroup::try_add_element(IInteger value ,CodeLabelSymbol *lab) {
    if (value == (_low_bound -1)) {
	_statement->insert_case(value,lab);
	_low_bound = value;
	return true;
	}
    if (value == (_high_bound + 1)) {
	_statement->insert_case(value,lab);
        _high_bound = value;
        return true;
        }
	
    if (value < _low_bound) {
	double density = (double)_statement->get_case_count()/((_high_bound - value + 1).c_double());
	if (density < required_density)
	    return false;
	_low_bound = value;
	}
    else if (value > _high_bound) {
	double density = (double)_statement->get_case_count()/(value - _low_bound - 1).c_double();
        if (density < required_density)
            return false;
        _high_bound = value;
	}
    _statement->insert_case(value,lab);
    return true;
    }

void MultiWayGroupList::add_element(IInteger value ,CodeLabelSymbol *lab) {

    int i = _list.length() - 1;
    while ((i >= 0) && (_list[i]->get_low_bound() > value))
	i --;

    // We have now found the position at which we can insert the element if it is
    // insertable. Try to add it as a larger element in the current block or a smallest
    // element in the next larger block
    if ((i >= 0) &&_list[i]->try_add_element(value,lab))
	return;

    if ((i < ((int)_list.length())-1) 
	&& _list[i+1]->try_add_element(value,lab))
	return; 

    MultiWayGroup *new_group = new MultiWayGroup(_env,value,lab,_default_lab,_decision);
    _list.insert(i + 1,new_group);
    }
	
//	Merge two groups. The second group must follow the first
void MultiWayGroup::merge(const MultiWayGroup &x) {
    for (int i = 0;i < x._statement->get_case_count(); i++) {
        MultiWayBranchStatement::case_pair pair = x._statement->get_case(i);
        _statement -> insert_case(pair.first,pair.second);
        }
    _high_bound = x._high_bound;
    }


//	now requires compact and ordered. 
static bool is_compact(MultiWayBranchStatement *the_case) {
    Iter<MultiWayBranchStatement::case_pair > iter = the_case->get_case_iterator();
    IInteger low;
    IInteger high;
    bool first = true;
    int count = 0;
    while (iter.is_valid()) {
        MultiWayBranchStatement::case_pair pair = iter.current();
	if (first) {
	    low =pair.first;
	    high = pair.first; 
	    first = false;
	    }
	else {
#ifdef COMPACT_ONLY
	    if (low > pair.first)
		low = pair.first;
	    if (high < pair.first)
		high = pair.first;
#else
	    if (pair.first < low)
		return false;
	    if (pair.first < high)
		return false;
	    high = pair.first;
#endif
	    }
	
	count ++;
	iter.next();
	}
    if (count == 0)
	return false;
    double density = (double)count / (high - low + 1).c_double();
    return density > 0.9999999999;
    }


multi_way_branch_statement_compactor::multi_way_branch_statement_compactor(SuifEnv *the_env,ProcedureDefinition *def)
        : ProcedureWalker(the_env,def,MultiWayBranchStatement::get_class_name()) {}

Walker::ApplyStatus multi_way_branch_statement_compactor::operator () (SuifObject *x)
    {
    MultiWayBranchStatement *the_case = to<MultiWayBranchStatement>(x);

    // is the table already compact?

    if (is_compact(the_case))
       return Walker::Continue;

    SymbolTable *scope = find_scope(x);
    if (!scope)
	return Walker::Continue;

    CodeLabelSymbol *default_lab =  the_case->get_default_target();

    // very special case - the case list is empty, so just jump to the default label

    if (the_case->get_case_count() == 0) {
	Statement *replacement = create_jump_statement(get_env(),default_lab);
	the_case->get_parent()->replace(the_case,replacement);
    	set_address(replacement);
    	return Walker::Replaced;
	}

    StatementList *replacement = create_statement_list(get_env());

    Expression *operand = the_case->get_decision_operand ();

    remove_suif_object(operand);

    DataType *type = operand->get_result_type();
    VariableSymbol *decision = create_variable_symbol(get_env(),get_type_builder(get_env())->get_qualified_type(type));
    scope->add_symbol(decision);

    replacement->append_statement(create_store_variable_statement(get_env(),decision,operand));

    the_case->set_default_target(0);
    MultiWayGroupList jump_list(get_env(),default_lab,decision);;
    
    Iter<MultiWayBranchStatement::case_pair > iter = the_case->get_case_iterator();
    while (iter.is_valid()) {
	MultiWayBranchStatement::case_pair pair = iter.current();
	jump_list.add_element(pair.first,pair.second);
	iter.next();
	}
    // we have built the new structure, now need to generate code for it

    jump_list.generate_code(replacement);
    
    the_case->get_parent()->replace(the_case,replacement);
    set_address(replacement);
    return Walker::Replaced;
    }


c_for_statement_walker::c_for_statement_walker(SuifEnv *the_env,ProcedureDefinition *def)
   	: ProcedureWalker(the_env,def, CForStatement::get_class_name()) {}

//	This is just a first cut and has not yet been compared with the old code

Walker::ApplyStatus c_for_statement_walker::operator () (SuifObject *x) {
    SuifObject *result = dismantle_c_for_statement(to<CForStatement>(x));
    set_address(result);
    return Walker::Replaced;
}
    
Statement* c_for_statement_walker::dismantle_c_for_statement(CForStatement *the_for){
    StatementList *replacement = create_statement_list(the_for->get_suif_env());
    StatementList *body  = create_statement_list(the_for->get_suif_env());
    Statement* original_body = the_for->get_body();
    the_for->set_body(0);
    body->append_statement(original_body);
    the_for->set_body(body);
    Statement* pre_pad = the_for->get_pre_pad();
    CodeLabelSymbol* break_lab = the_for->get_break_label();
    CodeLabelSymbol* continue_lab = the_for->get_continue_label();
    Statement* before = the_for->get_before();
    Expression* test = the_for->get_test();
    Statement* step = the_for->get_step();;

    if(before)remove_suif_object(before);
    if(test)remove_suif_object(test);
    if(step)remove_suif_object(step);
    if(body)remove_suif_object(body);
    if(pre_pad)remove_suif_object(pre_pad);
    the_for->set_break_label(0);
    the_for->set_continue_label(0);

    // I am guessing what pre-pad and post-pad do
    if (before)
	replacement->append_statement(before);
    
    // and loop if not out of range
    replacement->append_statement(
	create_label_location_statement(the_for->get_suif_env(), continue_lab));

    if(test) {
	    SuifEnv *env = the_for->get_suif_env();
	    DataType *bool_type = get_type_builder(env)->get_boolean_type();
	    test = create_unary_expression(env,bool_type,k_logical_not,test);
	    replacement->append_statement(create_branch_statement(env,test,break_lab));
	}

    if(pre_pad){
        replacement->append_statement(pre_pad);
    }

    append_flattened(replacement,body);
    append_flattened(replacement,step);
    replacement->append_statement(create_jump_statement(body->get_suif_env(),continue_lab));
    replacement->append_statement(
        create_label_location_statement(body->get_suif_env(), break_lab));
    // end of loop

    the_for->get_parent()->replace(the_for, replacement);
    return replacement;
}

DismantleEmptyScopeStatements::
DismantleEmptyScopeStatements(SuifEnv *the_env,
			      const LString &name) :
  PipelinablePass(the_env, name) 
{}

void DismantleEmptyScopeStatements::initialize() {
  PipelinablePass::initialize();
  _command_line->set_description("Eliminate empty scope statements");
}

void DismantleEmptyScopeStatements::
do_procedure_definition(ProcedureDefinition *pd) 
{
  list<ScopeStatement*> *l = collect_objects<ScopeStatement>(pd);
  while (!l->empty()) {
    ScopeStatement *sc = l->front();
    l->pop_front();
    if ((sc->get_definition_block()->get_variable_definition_count() == 0)
	&& sc->get_symbol_table()->get_symbol_table_object_count() == 0) {
      Statement *body = sc->get_body();
      sc->set_body(NULL);
      replace_statement(sc, body);
      trash_it(sc);
    }
  }
  delete l;
}


MarkGuardedFors::
MarkGuardedFors(SuifEnv *the_env,
		const LString &name) :
  PipelinablePass(the_env, name) 
{}

void MarkGuardedFors::initialize() {
  PipelinablePass::initialize();
  _command_line->set_description("Mark a ForStatement as guarded if lb TEST UB is true.    This works only when they are constants.");
}

void MarkGuardedFors::
do_procedure_definition(ProcedureDefinition *pd) 
{
  for (Iter<ForStatement> iter = object_iterator<ForStatement>(pd);
       iter.is_valid(); iter.next()) {
    ForStatement *the_for = &iter.current();
    if (is_for_statement_guarded(the_for)) continue;

    IInteger ii = 
      evaluate_for_statement_entry_test(the_for);
    if (ii.is_undetermined()) return;
    if (ii == 0)  return;
    if (ii == 1) {
      set_for_statement_guarded(the_for);
    }
  }
}

GuardAllFors::
GuardAllFors(SuifEnv *the_env,
		const LString &name) :
  PipelinablePass(the_env, name) 
{}

void GuardAllFors::initialize() {
  PipelinablePass::initialize();
  _command_line->set_description("Make all For statements execute at least once by guarding all the Fors with a test if necessary.");
}

DismantleCallArguments::DismantleCallArguments(SuifEnv *the_env,
		       const LString &name) :
  PipelinablePass(the_env, name) 
{}

void DismantleCallArguments::initialize() {
  PipelinablePass::initialize();
  _command_line->set_description("Replace expressions in call arguments with temporary variables whose values are set to the results of the expressions");
}

void DismantleCallArguments::
do_procedure_definition(ProcedureDefinition *pd) 
{
  list<CallStatement*> *l = collect_objects<CallStatement>(pd);
  while (!l->empty()) {
    CallStatement *call = l->front();
    l->pop_front();

    for (size_t i = 0; i < call->get_argument_count(); i++) {
      Expression *arg = call->get_argument(i);
      if (!arg) continue;
      if (is_kind_of<LoadVariableExpression>(arg)) continue;
      list<StoreVariableStatement *> l;
      force_dest_not_expr(arg, l);
    }
  }
  delete l;  
}

void GuardAllFors::
do_procedure_definition(ProcedureDefinition *pd)
{
  list<ForStatement*> *l = collect_objects<ForStatement>(pd);
  for (list<ForStatement*>::iterator it = l->begin(); it != l->end(); it++) {
    ForStatement *the_for = *it;
    // This may replace the for statement.
    // with a lower bound assignment or
    // with an IF statement that checks the condition
    // The for may get trashed in the process.
    guard_for_statement(the_for);
  }
  delete l;
}

