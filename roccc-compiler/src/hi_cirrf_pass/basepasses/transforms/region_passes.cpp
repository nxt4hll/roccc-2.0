// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include <typebuilder/type_builder.h>
#include <utils/trash_utils.h>
#include <utils/type_utils.h>
#include <utils/expression_utils.h>
#include <basicnodes/basic_factory.h>
#include <suifkernel/command_line_parsing.h>
#include <common/suif_indexed_list.h>
#include <utils/node_builder.h>
#include <utils/cloning_utils.h>
#include <typebuilder/type_builder.h>

#include "region_passes.h"

class RemoveIfAndLoopWalker: public GroupWalker {
public:
    RemoveIfAndLoopWalker(SuifEnv* the_env, ProcedureDefinition* proc_def, bool preserve_its);
protected:
    if_statement_walker*        the_if_statement_walker;
    while_statement_walker*     the_while_statement_walker;
    do_while_statement_walker*  the_do_while_statement_walker;
    c_for_statement_walker*     the_c_for_statement_walker;
};

class RecordStatementListsWalker: public ProcedureWalker {
public:
    RecordStatementListsWalker(SuifEnv *the_env,ProcedureDefinition *def)
        : ProcedureWalker(the_env,def, StatementList::get_class_name()) {}

    Walker::ApplyStatus RecordStatementListsWalker::operator () (SuifObject *x){
        statements.push_back(to<StatementList>(x));
        return Walker::Continue;
    };

    list<StatementList*>* get_list(){return &statements;};
protected:
    list<StatementList*> statements;
};

RemoveIfAndLoopWalker::RemoveIfAndLoopWalker(SuifEnv* the_env, ProcedureDefinition* proc_def, bool preserve_ifs)
    : GroupWalker(the_env)
{
    the_while_statement_walker = new while_statement_walker(the_env, proc_def);
    the_do_while_statement_walker = new do_while_statement_walker(the_env, proc_def);
    the_if_statement_walker = new if_statement_walker(the_env, proc_def);
    the_c_for_statement_walker = new c_for_statement_walker(the_env, proc_def);

    append_walker(*the_while_statement_walker);
    append_walker(*the_do_while_statement_walker);
    if(!preserve_ifs){
        append_walker(*the_if_statement_walker);
    }
    append_walker(*the_c_for_statement_walker);
}

void RemoveIfAndLoopPass::initialize() {
    PipelinablePass::initialize();
    _command_line -> set_description(
      "Dismantle ifs and while/do and CFor loops in the IR");
    _preserve_ifs = new OptionLiteral("-preserve_ifs");
    OptionSelection *opt = new OptionSelection(true);
    opt->add(_preserve_ifs);
    _command_line->add(opt);
};

void RemoveIfAndLoopPass::do_procedure_definition(ProcedureDefinition *proc_def){
    if(proc_def){
      RemoveIfAndLoopWalker walker(get_suif_env(), proc_def, _preserve_ifs->is_set());
      proc_def->walk(walker);
    }
};

void FlattenStatementListsPass::initialize() {
  PipelinablePass::initialize();
  _command_line->set_description("Get rid of statement lists inside of other statement lists and flatten them");
}

void FlattenStatementListsPass::do_procedure_definition(ProcedureDefinition *proc_def){
    if(proc_def){
        RecordStatementListsWalker walker(get_suif_env(), proc_def);
        walker.set_post_order();
        proc_def->walk(walker);

        list<StatementList*>* lists = walker.get_list();
        for(list<StatementList*>::iterator iter = lists->begin();iter!=lists->end();iter++){

        StatementList* the_list = *iter;

            if(is_kind_of<StatementList>(the_list->get_parent())
                //&& the_list->get_annote_count()==0
                )
            {
                StatementList* parent_list = to<StatementList>(the_list->get_parent());

                {for(int pos=0; pos<parent_list->get_statement_count(); pos++){
                    if(parent_list->get_statement(pos)==the_list){
                        parent_list->remove_statement(pos);
                        // slam them in
                        int i=0;
                        while (the_list->get_statement_count() != 0) {
                            Statement *a_statement = the_list->remove_statement(0
                                /*the_list->get_statement_count()-1*/);
                              parent_list->insert_statement(pos+i, a_statement);
                              i++;
                        }

                    }
                }}

                // Move the annotes
                list<Annote*> an_list;
                {for (Iter<Annote*> iter = the_list->get_annote_iterator();
                    iter.is_valid(); iter.next()) {
                    Annote *an = iter.current();
                    an_list.push_back(an);
                }}

                {for (list<Annote*>::iterator iter = an_list.begin();
                    iter != an_list.end(); iter++) {
                    //fprintf(stderr, "Moving annotes from %p to %p\n", the_list, parent_list);
                    Annote *an = *iter;
                    the_list->remove_annote(an);
                    parent_list->append_annote(an);
                }}

            }
        }

        // make sure everything has been flattened
        {for (Iter<StatementList> iter = object_iterator<StatementList>(proc_def);
                iter.is_valid(); iter.next())
        {
            suif_assert(!is_kind_of<StatementList>(iter.current().get_parent()));
        }}
    }
}

void AddProcedureEndLabelsPass::initialize() {
  PipelinablePass::initialize();
  _command_line->set_description("Add a new special label to the end of each procedure");
}

void AddProcedureEndLabelsPass::do_procedure_definition(ProcedureDefinition *proc_def){
    if(proc_def){
        ExecutionObject* body = proc_def->get_body();
        if(is_kind_of<StatementList>(body)){
            StatementList* list = to<StatementList>(body);
            list->append_statement(
                create_label_location_statement(get_suif_env(),
                    new_unique_label(get_suif_env(),
                            list, "return_label")));
        }
    }
}

static Statement* get_meat(Statement* stmt) {
    if(is_kind_of<StatementList>(stmt) &&
        (to<StatementList>(stmt)->get_statement_count()))
        return (to<StatementList>(stmt))->get_statement(0);
    else
        return stmt;
}

void CFor2ForPass::initialize() {
    PipelinablePass::initialize();
    _command_line -> set_description("Convert CForStatement Loops with invariant"
				   " lower bounds, upper bounds and steps\n"
				   "to SUIF ForStatement loops");
    _verbose = new OptionLiteral("-verbose");
    OptionSelection *opt = new OptionSelection(true);
    opt->add(_verbose);
    _command_line->add(opt);
}

/**
    Test if \a cfor is a legal SUIF for statement.
    At this point, only look at loops of form
        for(int i=10; i<20; i=+2)
*/
bool CFor2ForPass::convert_cfor2for(CForStatement* cfor,
				    bool verbose) {
    StatementList*      body =  to<StatementList>(cfor->get_body());
    Statement*          pre_pad = cfor->get_pre_pad();
    CodeLabelSymbol*    break_lab = cfor->get_break_label();
    CodeLabelSymbol*    continue_lab = cfor->get_continue_label();
    Statement *         before = cfor->get_before();
    Expression *        test = cfor->get_test();
    Statement *         step = cfor->get_step();;
#ifdef OUT
#error "OUT already defined"
#endif
#define OUT \
    { \
      if (verbose) \
	suif_warning("Failing to convert cfor to a for\n"); \
      return false; \
    }

    // make sure before is an index assignment
    Statement* before_meat = get_meat(before);
    if(!is_kind_of<StoreVariableStatement>(before_meat)) OUT;
    VariableSymbol* var = to<StoreVariableStatement>(before_meat)->get_destination();
    Expression* lb = to<StoreVariableStatement>(before_meat)->get_value();

    // make sure test is a comparison with the index
    if(!is_kind_of<BinaryExpression>(test)) OUT;

    BinaryExpression* comp_test = to<BinaryExpression>(test);
    if(!is_kind_of<LoadVariableExpression>(comp_test->get_source1())) OUT;
    if((to<LoadVariableExpression>(
        comp_test->get_source1()))->get_source()!=var) OUT;
    Expression* ub = comp_test->get_source2();

    const LString test_opcode = comp_test->get_opcode();

    if(!is_kind_of<StatementList>(step)) OUT;
    StatementList* list = to<StatementList>(step);
    if(list->get_statement_count()!=2) OUT;
    //list->print_to_default();
    Statement* stmt1 = list->get_statement(0);
    Statement* stmt2 = list->get_statement(1);

    Expression* incr = NULL;
    LString incr_opcode;

    /**
        Make sure step is an index increment.

        We support two cases: pre-increment such as ++i and
        post-increment such as i++. In any case, this is done
        is 2 steps.

        In the case of pre-increment, we have
            tmp = i+1
            i = tmp
    */
    if(!is_kind_of<StoreVariableStatement>(stmt1)) OUT;
    if(!is_kind_of<StoreVariableStatement>(stmt2)) OUT;

    StoreVariableStatement* svs1 = to<StoreVariableStatement>(stmt1);
    StoreVariableStatement* svs2 = to<StoreVariableStatement>(stmt2);
    BinaryExpression* bin = NULL;

    if(!is_kind_of<LoadVariableExpression>(svs2->get_value())) goto POST;

    // tmp in both
    if(to<LoadVariableExpression>(svs2->get_value())->get_source()!=
       svs1->get_destination()) OUT;
    // var in both
    if(svs2->get_destination()!=var) OUT;

    if(!is_kind_of<BinaryExpression>(svs1->get_value())) OUT;

    bin = to<BinaryExpression>(svs1->get_value());

    if(!is_kind_of<LoadVariableExpression>(bin->get_source1())) OUT;

    if(to<LoadVariableExpression>(bin->get_source1())->get_source()!=var) OUT;

    incr_opcode = to<BinaryExpression>(to<StoreVariableStatement>(stmt1)->
            get_value())->get_opcode();

    incr =
        to<BinaryExpression>(to<StoreVariableStatement>(
                stmt1)->get_value())->get_source2();
    goto NEXT;

    /*
        In the case of post-increment, we have

            tmp = i
            i = tmp + 1
    */
POST:
    if(!is_kind_of<LoadVariableExpression>(svs1->get_value())) OUT;

    // i is the source of 1
    if(to<LoadVariableExpression>(svs1->get_value())->get_source()!=var) OUT;
    // and dest of 2
    if(svs2->get_destination()!=var) OUT;

    if(!is_kind_of<BinaryExpression>(svs2->get_value())) OUT;

    bin = to<BinaryExpression>(svs2->get_value());

    if(!is_kind_of<LoadVariableExpression>(bin->get_source1())) OUT;

    if(to<LoadVariableExpression>(bin->get_source1())->get_source()!=
        svs1->get_destination()) OUT;

    incr_opcode = bin->get_opcode();

    incr = to<BinaryExpression>(to<StoreVariableStatement>(
                stmt2)->get_value())->get_source2();

NEXT:
    if(incr_opcode!=k_add && incr_opcode!=k_subtract) OUT;

    if(is_variable_modified(var, body)) OUT;

    if(is_expr_modified(ub, body)) OUT;

    // remove everything from for
    remove_suif_object(lb);
    remove_suif_object(ub);
    remove_suif_object(incr);
    remove_suif_object(body);
    remove_suif_object(pre_pad);
    cfor->set_break_label(0);
    cfor->set_continue_label(0);

    // deal with i-- or i-=4
    if(incr_opcode==k_subtract){
        // int decrement by a const is a common case, so
        // just change the constant. Otherwise, negate
        // incr.
        if(is_kind_of<IntConstant>(incr)){
            IntConstant* int_const = to<IntConstant>(incr);
            int_const->set_value(IInteger(0)-int_const->get_value());
        }else{
            incr = create_unary_expression(incr->get_suif_env(),
			      incr->get_result_type(),
			      k_negate,
			      incr );
        }
    }

    // constrct the for statement itself.
    // what do we do with the rest of the stuff?
    ForStatement* for_stmt =
        create_for_statement(cfor->get_suif_env(),
            var,
            lb,
            ub,
            incr,
            test_opcode,
            body,
            pre_pad,
            break_lab,
            continue_lab
        );

    cfor->get_parent()->replace(cfor, for_stmt);
    // TODO: do we need to trash cfor here somehow?
    return true;
#undef OUT
};

void CFor2ForPass::do_procedure_definition(ProcedureDefinition *proc_def){
    list<CForStatement*>* to_be_converted = 
        collect_objects<CForStatement>(proc_def);

    {for (list<CForStatement*>::iterator iter = to_be_converted->begin();
            iter!=to_be_converted->end(); iter++)
    {
        CForStatement* cfor = *iter;
        if(convert_cfor2for(cfor, _verbose->is_set())) conversion_count++;
    }}

    delete to_be_converted;
};

void CFor2ForPass::finalize(){
    if(conversion_count!=0){
      if (_verbose->is_set())
        printf(
            "\n%d CFor loop(s) have been successfully "
            "converted to SUIF For loops\n",
            conversion_count);
    }
};


void DismantleStmtsWithJumpsInside::initialize() {
  PipelinablePass::initialize();
  _command_line->set_description("Dismantle structural statements that have jumps going inside of them.");
}


void DismantleStmtsWithJumpsInside::dismantle_structural_statement(Statement* stmt){
    if(is_kind_of<ForStatement>(stmt)){
        for_statement_walker::dismantle_for_statement(to<ForStatement>(stmt));
    }else
    if(is_kind_of<IfStatement>(stmt)){
        if_statement_walker::dismantle_if_statement(to<IfStatement>(stmt));
    }else
    if(is_kind_of<WhileStatement>(stmt)){
        while_statement_walker::dismantle_while_statement(to<WhileStatement>(stmt));
    }else
    if(is_kind_of<DoWhileStatement>(stmt)){
        do_while_statement_walker::dismantle_do_while_statement(to<DoWhileStatement>(stmt));
    }else
    if(is_kind_of<CForStatement>(stmt)){
        c_for_statement_walker::dismantle_c_for_statement(to<CForStatement>(stmt));
    }else
    if(is_kind_of<ScopeStatement>(stmt)){
        scope_statement_walker::dismantle_scope_statement(to<ScopeStatement>(stmt));
    }else{
        suif_warning("Trying to dismantle an unknown statement type %s",
		     stmt->getClassName().c_str());
    }
};

template <class T>
static void eliminate_jumps_into(ProcedureDefinition* proc_def, T* dummy=0){
    list<T*>* stmt_list = collect_objects<T>(proc_def);
    typename list<T*>::iterator iter = stmt_list->begin();
    for ( ; iter != stmt_list->end(); iter++)
    {
        T* stmt = *iter;
        //printf("Looking at %s\n", stmt->getClassName().c_str());
        if(has_jumps_going_inside(stmt)){
            suif_warning("Dismantling %s\n", stmt->getClassName().c_str());
            DismantleStmtsWithJumpsInside::dismantle_structural_statement(stmt);
        };
    }

    delete stmt_list;
};

void DismantleStmtsWithJumpsInside::do_procedure_definition(ProcedureDefinition *proc_def){
    eliminate_jumps_into<IfStatement>       (proc_def);
    eliminate_jumps_into<ForStatement>      (proc_def);
    eliminate_jumps_into<WhileStatement>    (proc_def);
    eliminate_jumps_into<DoWhileStatement>  (proc_def);
    eliminate_jumps_into<CForStatement>     (proc_def);
    eliminate_jumps_into<ScopeStatement>    (proc_def);
};

void DismantleMultiEntryScopeStatements::initialize() {
  PipelinablePass::initialize();
  _command_line->set_description("Dismantle scoped statements that have jumps going inside of them.");
}

void DismantleMultiEntryScopeStatements::
do_procedure_definition(ProcedureDefinition *proc_def){
    eliminate_jumps_into<ScopeStatement>    (proc_def);
};

unsigned int CFor2ForPass::conversion_count = 0;

void AddStatementListsToProcs::initialize() {
  PipelinablePass::initialize();
  _command_line->set_description("Put StatementLists inside of procedures that just have bare statements as their body");
}

void AddStatementListsToProcs::do_procedure_definition(ProcedureDefinition *proc_def){
    ExecutionObject* body = proc_def->get_body();
    suif_assert_message(is_kind_of<Statement>(body),
        ("Expecting the body to be a statement"));
    Statement* stmt = to<Statement>(body);
    if(is_kind_of<StatementList>(stmt)) return;
    suif_warning("AddStatementListsToProcs is replacing "
        "a statement by a statement list");
    remove_suif_object(stmt);
    StatementList* list = create_statement_list(get_suif_env());
    list->append_statement(stmt);
    proc_def->set_body(list);
};

CodeLabelSymbol* get_label_after(const Statement* stmt){
    SuifObject* parent = stmt->get_parent();
    if(!is_kind_of<StatementList>(parent)) return NULL;
    StatementList* list = to<StatementList>(parent);
    
    for(int i = 0; i<list->get_statement_count(); i++)
    {
        Statement* curr_stmt = list->get_statement(i);
        if(curr_stmt == stmt){
            // last statement
            if(i == list->get_statement_count()-1) return NULL;

            Statement* next_stmt = list->get_statement(i+1);
            if(!is_kind_of<LabelLocationStatement>(next_stmt)) return NULL;

            LabelLocationStatement* next_lab = to<LabelLocationStatement>(next_stmt);
            return next_lab->get_defined_label();
        }
    }
    suif_assert_message(false, ("Must never be here"));
    return NULL;
};

CodeLabelSymbol* get_last_label(const Statement* stmt){
    if(!is_kind_of<StatementList>(stmt)) return NULL;
    StatementList* list = to<StatementList>(stmt);
    
	Statement* last_stmt = list->get_statement(
		list->get_statement_count()-1);

    if(!is_kind_of<LabelLocationStatement>(last_stmt)) return NULL;

    LabelLocationStatement* last_lab = to<LabelLocationStatement>(last_stmt);
    return last_lab->get_defined_label();
};

template <class T> void add_loop_labels(T* loop){
    CodeLabelSymbol* break_label = loop->get_break_label();
    CodeLabelSymbol* continue_label = loop->get_continue_label();

    if(break_label!=NULL){
		// if the loop is supposed to have a break label
		// get the real label after it
		CodeLabelSymbol* real_break_label = get_label_after(loop);
		suif_assert_message(real_break_label!=break_label,
            ("The loop doesn't seems to already have the right label after it.\n "));
		
		LabelLocationStatement* label_statement =
            create_label_location_statement(break_label->get_suif_env(), break_label);
        // Add the continue label and the cloned store
        insert_statement_after(loop->get_body(), label_statement);
		
		fprintf(stderr, "Adding a break label to a %s\n", loop->getClassName().c_str());
    }

    if(continue_label!=NULL){
		CodeLabelSymbol* real_continue_label = get_last_label(loop);
		suif_assert_message(real_continue_label!=continue_label,
            ("The loop doesn't seems to already have the right label after it.\n "));

        LabelLocationStatement* label_statement =
            create_label_location_statement(continue_label->get_suif_env(), continue_label);
        // Add the continue label and the cloned store
        insert_statement_before(loop->get_body(), label_statement);
		fprintf(stderr, "Adding a continue label to a %s\n", loop->getClassName().c_str());
    }
};

template <class T> void apply_to_loops(ProcedureDefinition *proc_def, 
	void fp(T* loop))
{
	for (Iter<T> iter = object_iterator<T>(proc_def);
		iter.is_valid(); iter.next())
    {
        T* loop = &iter.current();
		(*fp)(loop);
	};
};

void AddExplicitLoopLabels::initialize() {
  PipelinablePass::initialize();
  _command_line->set_description("Add continues and break labels as explicit LabelLocationStatement if they are not there yet");
};

void AddExplicitLoopLabels::do_procedure_definition(ProcedureDefinition *proc_def){
    apply_to_loops<ForStatement>(proc_def, &add_loop_labels<ForStatement>);
	apply_to_loops<WhileStatement>(proc_def, &add_loop_labels<WhileStatement>);
	apply_to_loops<DoWhileStatement>(proc_def, &add_loop_labels<DoWhileStatement>);
};

template <class T> void change_loop_labels(T* loop){
    CodeLabelSymbol* break_label = loop->get_break_label();
    CodeLabelSymbol* continue_label = loop->get_continue_label();

	/* 
		The idea is to find a label location statement right 
		after this loop and if 
			1) its labels is not the same as break_label AND
			2) there are no jumps to it from outside the loop

		Proceed to change the label of the for loop.
	*/
    if(break_label!=NULL){
		// if the loop is supposed to have a break label
		// get the real label after it
		CodeLabelSymbol* real_break_label = get_label_after(loop);
		suif_assert_message(real_break_label,
            ("The loop doesn't have a LabelLocationStatement with after it.\n "));
		
		searchable_list<CodeLabelSymbol*> targets;
		// find jumps to this label from outside the body
		get_jumps_from_outside(loop, targets);

		// find if real_break_label is one of them
		if(targets.is_member(real_break_label)){
			suif_assert_message(false,("There's a jump to the loop break label from the outside!"));
		}else{
			loop->set_break_label(real_break_label);
		}
    }

    if(continue_label!=NULL){
        CodeLabelSymbol* real_continue_label = get_last_label(loop);
		suif_assert_message(real_continue_label,
            ("The loop doesn't have a LabelLocationStatement as the last statement of the body.\n "));
		searchable_list<CodeLabelSymbol*> targets;
		// find jumps to this label from outside the body
		get_jumps_from_outside(loop, targets);

		// find if real_break_label is one of them
		if(targets.is_member(real_continue_label)){
			suif_assert_message(false,("There's a jump to the loop continue label from the outside!"));
		}else{
			loop->set_break_label(real_continue_label);
		}
    }
};

void FixupExplicitLoopLabels::initialize() {
  PipelinablePass::initialize();
  _command_line->set_description("Fix up all the explicit labels in the different kinds of loops");
};

void FixupExplicitLoopLabels::do_procedure_definition(ProcedureDefinition *proc_def){
    apply_to_loops<ForStatement>(proc_def, &change_loop_labels<ForStatement>);
	apply_to_loops<WhileStatement>(proc_def, &change_loop_labels<WhileStatement>);
	apply_to_loops<DoWhileStatement>(proc_def, &change_loop_labels<DoWhileStatement>);
};

template <class T> void remove_loop_labels(T* loop){
	// TODO
};

void RemoveExplicitLoopLabels::initialize() {
  PipelinablePass::initialize();
  _command_line->set_description("Remove continues and break labels as explicit LabelLocationStatement that have the same labels as continues/breaks of loops.");
};

void RemoveExplicitLoopLabels::do_procedure_definition(ProcedureDefinition *proc_def){
	apply_to_loops<ForStatement>(proc_def, &remove_loop_labels<ForStatement>);
	apply_to_loops<WhileStatement>(proc_def, &remove_loop_labels<WhileStatement>);
	apply_to_loops<DoWhileStatement>(proc_def, &remove_loop_labels<DoWhileStatement>);
};

void IfConditionsToBinaryExprs::initialize() {
  PipelinablePass::initialize();
  _command_line->set_description("Convert if conditions that are not BinaryExpressions by explicitly comparing them with 0");
};

void IfConditionsToBinaryExprs::do_procedure_definition(ProcedureDefinition *proc_def){
	SuifEnv* suif_env  = proc_def->get_suif_env();
	list<IfStatement*>* to_be_converted = 
        collect_objects<IfStatement>(proc_def);

	{for (	list<IfStatement*>::iterator iter = to_be_converted->begin();
            iter!=to_be_converted->end(); iter++)
    {
		IfStatement* the_if = *iter;
		Expression* cond = the_if->get_condition();

		if(!is_kind_of<BinaryExpression>(cond)){
			remove_suif_object(cond);
			the_if->set_condition(NULL);
			BinaryExpression* bin_expr =
				create_binary_expression(suif_env, 
					cond->get_result_type(),
					k_is_not_equal_to, 
					cond, 
					create_int_constant(suif_env, 0)
				);
			the_if->set_condition(bin_expr);
		}
	}}

	delete to_be_converted;
};
