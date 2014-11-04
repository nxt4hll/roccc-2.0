// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include <common/suif_copyright.h>
#include <common/suif_list.h>

#include <iostream>
#include <math.h>
#include <iokernel/cast.h>
#include <iokernel/clone_stream.h>
#include <common/i_integer.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>
#include <basicnodes/basic.h>
#include <basicnodes/basic_constants.h>
#include <suifkernel/suifkernel_messages.h>
#include <suifkernel/utilities.h> 
#include <suifkernel/group_walker.h> 
#include "transforms/procedure_walker_utilities.h"
#include <utils/expression_utils.h>
#include <utils/symbol_utils.h>
#include <utils/type_utils.h>
#include <utils/cloning_utils.h>
#include <cfenodes/cfe.h>
#include "list_utils.h"
#include "print_utils.h"
#include "IR_utils.h"

using namespace std;

/**************************** Declerations ************************************/

bool is_textually_preceeding2(Statement *s1, Statement *s2, Statement *s, int reset = 0);

/************************** Implementations ***********************************/

bool is_textually_preceeding2(Statement *s1, Statement *s2, Statement *s, int reset){
    static bool is_s1_reached = 0;

    if(reset)
       is_s1_reached = 0;

    if(s == s2){
       if(is_s1_reached)
          return 1;
       else return 0;
    }

    if(s == s1)
       is_s1_reached = 1;

    if (is_a<IfStatement>(s)){
	bool saved_is_s1_reached_value = is_s1_reached;

	bool then_part_result = is_textually_preceeding2(s1, s2, (to<IfStatement>(s))->get_then_part());
	bool then_part_is_s1_reached_value = is_s1_reached;

	is_s1_reached = saved_is_s1_reached_value;
        bool else_part_result = 0;
	if((to<IfStatement>(s))->get_else_part()) 
           else_part_result = is_textually_preceeding2(s1, s2, (to<IfStatement>(s))->get_else_part());

	is_s1_reached = is_s1_reached || then_part_is_s1_reached_value;
	return then_part_result || else_part_result;

    }else if (is_a<WhileStatement>(s))
        return is_textually_preceeding2(s1, s2, (to<WhileStatement>(s))->get_body());
    else if (is_a<CForStatement>(s))
        return is_textually_preceeding2(s1, s2, (to<CForStatement>(s))->get_body());
    else if (is_a<ScopeStatement>(s))
        return is_textually_preceeding2(s1, s2, (to<ScopeStatement>(s))->get_body());
    else if (is_a<StatementList>(s)){
	bool result = 0;
        for (Iter<Statement*> iter = (to<StatementList>(s))->get_child_statement_iterator();
             iter.is_valid(); iter.next())
             result = result || is_textually_preceeding2(s1, s2, iter.current());
	return result;
    }

    return 0;
}

bool is_textually_preceeding(Statement *s1, Statement *s2, Statement *s){

    if(s1 == s2)
       return 0; 

    if(!s){
       ProcedureDefinition *proc_def = get_procedure_definition(s1);

       if(proc_def != get_procedure_definition(s2))
          cout << "ERROR! Statements do not belong to the same procedure." << endl;

       list<SuifObject*>* s1_parents = new list<SuifObject*>;

       SuifObject* s1_parent = s1;
       while(is_kind_of<Statement>(s1_parent)){
	  s1_parents->push_back(s1_parent);
	  s1_parent = s1_parent->get_parent();
       }

       SuifObject* s2_parent = s2;
       while(is_kind_of<Statement>(s2_parent) && (!is_in_list(s2_parent, s1_parents)))
	  s2_parent = s2_parent->get_parent();

       delete s1_parents;

       if(is_kind_of<Statement>(s2_parent))
          return is_textually_preceeding2(s1, s2, to<Statement>(s2_parent), 1);

       s = to<Statement>(proc_def->get_body());
    }

    return is_textually_preceeding2(s1, s2, s, 1);

}


bool is_at_the_same_nesting_level(Statement *stmt1, Statement *stmt2){  // INCORRECT, FIX NEEDED

    if(stmt1->get_parent() == stmt2->get_parent())
       return 1;

    return 0;
}

int get_statement_pos(Statement *stmt){

    if(!is_a<StatementList>(stmt->get_parent()))
       return -1;
    
    StatementList *parent = to<StatementList>(stmt->get_parent());  

    int stmt_pos = 0;
    for(Iter<Statement*> iter = parent->get_child_statement_iterator(); 
        iter.is_valid(); iter.next()) {
        if(stmt == iter.current())
	   return stmt_pos;
	stmt_pos++; 
    }
    
    return -1;
}

Statement* get_next_statement(Statement *stmt){

    if(!is_a<StatementList>(stmt->get_parent()))
       return NULL;
    
    StatementList *parent = to<StatementList>(stmt->get_parent());  

    for(Iter<Statement*> iter = parent->get_statement_iterator(); 
        iter.is_valid(); iter.next()) {
        if(stmt == iter.current()){
	   iter.next();
	   if(iter.is_valid())
	      return iter.current();
	   return NULL;
	}
    }
    
    return NULL;
}

bool is_equal(SuifObject *a, SuifObject *b){

	if(is_a<ArrayReferenceExpression>(a) & is_a<ArrayReferenceExpression>(b)){

	   ArrayReferenceExpression *exp_a = to<ArrayReferenceExpression>(a); 
	   ArrayReferenceExpression *exp_b = to<ArrayReferenceExpression>(b); 
	   return is_equal(exp_a->get_base_array_address(), exp_b->get_base_array_address()) && 
		  is_equal(exp_a->get_index(), exp_b->get_index());

	}else if(is_a<MultiDimArrayExpression>(a) & is_a<MultiDimArrayExpression>(b)){

	   MultiDimArrayExpression *exp_a = to<MultiDimArrayExpression>(a); 
	   MultiDimArrayExpression *exp_b = to<MultiDimArrayExpression>(b); 
	   bool equal = is_equal(exp_a->get_array_address(), exp_b->get_array_address());
	   Iter<Expression*> iter_a = exp_a->get_index_iterator(); 
	   Iter<Expression*> iter_b = exp_b->get_index_iterator();
	   for( ; iter_a.is_valid(), iter_b.is_valid(); iter_a.next(), iter_b.next()) {
	       equal = equal && is_equal(iter_a.current(), iter_b.current());
	       if(!equal) return 0;
           }
	   return !iter_a.is_valid() && !iter_b.is_valid() && equal; 

	}else if(is_a<SymbolAddressExpression>(a) & is_a<SymbolAddressExpression>(b)){

	   SymbolAddressExpression *exp_a = to<SymbolAddressExpression>(a); 
	   SymbolAddressExpression *exp_b = to<SymbolAddressExpression>(b); 
	   return exp_a->get_addressed_symbol() == exp_b->get_addressed_symbol();

	}else if(is_a<BinaryExpression>(a) & is_a<BinaryExpression>(b)){

	   BinaryExpression *exp_a = to<BinaryExpression>(a); 
	   BinaryExpression *exp_b = to<BinaryExpression>(b); 
	   return is_equal(exp_a->get_source1(), exp_b->get_source1()) && 
		  is_equal(exp_a->get_source2(), exp_b->get_source2()) &&
		  (String)exp_a->get_opcode() == (String)exp_b->get_opcode();

	}else if(is_a<UnaryExpression>(a) & is_a<UnaryExpression>(b)){

	   UnaryExpression *exp_a = to<UnaryExpression>(a); 
	   UnaryExpression *exp_b = to<UnaryExpression>(b); 
	   return is_equal(exp_a->get_source(), exp_b->get_source()) && 
		  (String)exp_a->get_opcode() == (String)exp_b->get_opcode();

	}else if(is_a<LoadVariableExpression>(a) & is_a<LoadVariableExpression>(b)){

	   LoadVariableExpression *exp_a = to<LoadVariableExpression>(a); 
	   LoadVariableExpression *exp_b = to<LoadVariableExpression>(b); 
	   return exp_a->get_source() == exp_b->get_source();

	}else if(is_a<LoadExpression>(a) & is_a<LoadExpression>(b)){

	   LoadExpression *exp_a = to<LoadExpression>(a); 
	   LoadExpression *exp_b = to<LoadExpression>(b); 
	   return is_equal(exp_a->get_source_address(), exp_b->get_source_address());

	}else if(is_a<IntConstant>(a) & is_a<IntConstant>(b)){

	   IntConstant *exp_a = to<IntConstant>(a); 
	   IntConstant *exp_b = to<IntConstant>(b); 
	   return exp_a->get_value().c_int() == exp_b->get_value().c_int();

	}else if(is_a<EvalStatement>(a) & is_a<EvalStatement>(b)){

	   EvalStatement *stmt_a = to<EvalStatement>(a); 
	   EvalStatement *stmt_b = to<EvalStatement>(b); 

	   Iter<Expression*> iter_a = stmt_a->get_expression_iterator(); 
	   Iter<Expression*> iter_b = stmt_b->get_expression_iterator();

	   bool equal = 1;
	   for( ; iter_a.is_valid(), iter_b.is_valid(); iter_a.next(), iter_b.next()) {
	       equal = equal && is_equal(iter_a.current(), iter_b.current());
	       if(!equal) return 0;
           }
	   return !iter_a.is_valid() && !iter_b.is_valid() && equal; 

	}else if(is_a<IfStatement>(a) & is_a<IfStatement>(b)){

	   IfStatement *stmt_a = to<IfStatement>(a); 
	   IfStatement *stmt_b = to<IfStatement>(b); 
	   return is_equal(stmt_a->get_condition(), stmt_b->get_condition()) &&
	          is_equal(stmt_a->get_then_part(), stmt_b->get_then_part()) &&
	          is_equal(stmt_a->get_else_part(), stmt_b->get_else_part());

	}else if(is_a<CForStatement>(a) & is_a<CForStatement>(b)){

	   CForStatement *stmt_a = to<CForStatement>(a); 
	   CForStatement *stmt_b = to<CForStatement>(b); 
	   return is_equal(stmt_a->get_before(), stmt_b->get_before()) &&
	          is_equal(stmt_a->get_test(), stmt_b->get_test()) &&
	          is_equal(stmt_a->get_step(), stmt_b->get_step()) &&
	          is_equal(stmt_a->get_body(), stmt_b->get_body());

	}else if(is_a<WhileStatement>(a) & is_a<WhileStatement>(b)){

	   WhileStatement *stmt_a = to<WhileStatement>(a); 
	   WhileStatement *stmt_b = to<WhileStatement>(b); 
	   return is_equal(stmt_a->get_condition(), stmt_b->get_condition()) &&
	          is_equal(stmt_a->get_body(), stmt_b->get_body());

	}else if(is_a<StoreStatement>(a) & is_a<StoreStatement>(b)){

	   StoreStatement *stmt_a = to<StoreStatement>(a); 
	   StoreStatement *stmt_b = to<StoreStatement>(b); 
	   return is_equal(stmt_a->get_destination_address(), stmt_b->get_destination_address()) &&
	          is_equal(stmt_a->get_value(), stmt_b->get_value());

	}else if(is_a<StoreVariableStatement>(a) & is_a<StoreVariableStatement>(b)){

	   StoreVariableStatement *stmt_a = to<StoreVariableStatement>(a); 
	   StoreVariableStatement *stmt_b = to<StoreVariableStatement>(b); 
	   return stmt_a->get_destination() == stmt_b->get_destination() &&
	          is_equal(stmt_a->get_value(), stmt_b->get_value());

	}else if(is_a<CallStatement>(a) & is_a<CallStatement>(b)){

	   CallStatement *stmt_a = to<CallStatement>(a); 
	   CallStatement *stmt_b = to<CallStatement>(b);

	   Iter<Expression*> iter_a = stmt_a->get_argument_iterator(); 
	   Iter<Expression*> iter_b = stmt_b->get_argument_iterator();

	   bool equal = 1;
	   for( ; iter_a.is_valid(), iter_b.is_valid(); iter_a.next(), iter_b.next()) {
	       equal = equal && is_equal(iter_a.current(), iter_b.current());
	       if(!equal) return 0;
           }
 
	   return stmt_a->get_destination() == stmt_b->get_destination() &&
	          is_equal(stmt_a->get_callee_address(), stmt_b->get_callee_address()) &&
	          !iter_a.is_valid() && !iter_b.is_valid() && equal; 

	}else if(is_a<JumpStatement>(a) & is_a<JumpStatement>(b)){

	   JumpStatement *stmt_a = to<JumpStatement>(a); 
	   JumpStatement *stmt_b = to<JumpStatement>(b); 
	   return stmt_a->get_target() == stmt_b->get_target();

	}else if(is_a<BranchStatement>(a) & is_a<BranchStatement>(b)){

	   BranchStatement *stmt_a = to<BranchStatement>(a); 
	   BranchStatement *stmt_b = to<BranchStatement>(b); 
	   return stmt_a->get_target() == stmt_b->get_target() &&
	          is_equal(stmt_a->get_decision_operand(), stmt_b->get_decision_operand());

	}else if(is_a<ReturnStatement>(a) & is_a<ReturnStatement>(b)){

	   ReturnStatement *stmt_a = to<ReturnStatement>(a); 
	   ReturnStatement *stmt_b = to<ReturnStatement>(b); 
	   return is_equal(stmt_a->get_return_value(), stmt_b->get_return_value());

	}else if(is_a<MarkStatement>(a) & is_a<MarkStatement>(b)){

	   return 1;

	}else if(is_a<ScopeStatement>(a) & is_a<ScopeStatement>(b)){

	   return 0;
	
	}else if(is_a<StatementList>(a) & is_a<StatementList>(b)){

	   StatementList *stmt_a = to<StatementList>(a); 
	   StatementList *stmt_b = to<StatementList>(b); 

	   Iter<Statement*> iter_a = stmt_a->get_statement_iterator(); 
	   Iter<Statement*> iter_b = stmt_b->get_statement_iterator();

	   bool equal = 1;
	   for( ; iter_a.is_valid(), iter_b.is_valid(); iter_a.next(), iter_b.next()) {
	       equal = equal && is_equal(iter_a.current(), iter_b.current());
	       if(!equal) return 0;
           }
	   return !iter_a.is_valid() && !iter_b.is_valid() && equal; 

	}else if(is_a<LabelLocationStatement>(a) & is_a<LabelLocationStatement>(b)){

	   LabelLocationStatement *stmt_a = to<LabelLocationStatement>(a); 
	   LabelLocationStatement *stmt_b = to<LabelLocationStatement>(b); 
	   return stmt_a->get_defined_label() == stmt_b->get_defined_label();

	}
	
	return 0;
}

bool is_enclosing(ExecutionObject *p, ExecutionObject *s){  // is p the parent of s

    ProcedureDefinition *parent_proc_def = get_procedure_definition(s);
    ExecutionObject *root = parent_proc_def->get_body();

    if(s == p)
       return 0;

    ExecutionObject *current_exec_obj = s;
    while(current_exec_obj!=root && current_exec_obj!=p )
        current_exec_obj = to<ExecutionObject>(current_exec_obj->get_parent());

    if(current_exec_obj == p)
       return 1;

    return 0;
}

Statement* get_enclosing_loop(ExecutionObject *s){

    ProcedureDefinition *parent_proc_def = get_procedure_definition(s);
    ExecutionObject *root = parent_proc_def->get_body();

    if(s == root)
       return NULL;

    ExecutionObject *current_exec_obj = to<ExecutionObject>(s->get_parent());
    while(current_exec_obj!=root){
        if(is_a<CForStatement>(current_exec_obj) || is_a<WhileStatement>(current_exec_obj))
           return to<Statement>(current_exec_obj);
        current_exec_obj = to<ExecutionObject>(current_exec_obj->get_parent());
    }

    if(is_a<CForStatement>(current_exec_obj) || is_a<WhileStatement>(current_exec_obj))
       return to<Statement>(current_exec_obj);

    return NULL;
}


CForStatement* get_enclosing_c_for_stmt(ExecutionObject *s){

    ProcedureDefinition *parent_proc_def = get_procedure_definition(s);
    ExecutionObject *root = parent_proc_def->get_body();

    if(s == root)
       return NULL;

    ExecutionObject *current_exec_obj = to<ExecutionObject>(s->get_parent());
    while(current_exec_obj!=root){
        if(is_a<CForStatement>(current_exec_obj))
           return to<CForStatement>(current_exec_obj);
        current_exec_obj = to<ExecutionObject>(current_exec_obj->get_parent());
    }

    if(is_a<CForStatement>(current_exec_obj))
       return to<CForStatement>(current_exec_obj);
    
    return NULL;
}

list<CodeLabelSymbol*>* collect_code_labels_defined_by_label_location_stmts(ProcedureDefinition *proc_def){

    list<CodeLabelSymbol*> *code_label_list = new list<CodeLabelSymbol*>;

    for(Iter<LabelLocationStatement> iter = object_iterator<LabelLocationStatement>(proc_def);
	iter.is_valid(); iter.next()){

	code_label_list->push_back((&iter.current())->get_defined_label());

    }

    return code_label_list;

}

list<CodeLabelSymbol*>* collect_code_labels_targeted_by_jump_stmts(ProcedureDefinition *proc_def){

    list<CodeLabelSymbol*> *code_label_list = new list<CodeLabelSymbol*>;

    for(Iter<JumpStatement> iter = object_iterator<JumpStatement>(proc_def);
	iter.is_valid(); iter.next()){

	code_label_list->push_back((&iter.current())->get_target());

    }

    return code_label_list;

}

void remove_untargeted_labels(ProcedureDefinition *proc_def){

    list<CodeLabelSymbol*> *code_labels_defined_by_label_location_stmts = 
				collect_code_labels_defined_by_label_location_stmts(proc_def);

    list<CodeLabelSymbol*> *code_labels_targeted_by_jump_stmts = 
				collect_code_labels_targeted_by_jump_stmts(proc_def);

    list<CodeLabelSymbol*> *untargeted_label_syms = 
				subtract_lists(code_labels_defined_by_label_location_stmts, 
					       code_labels_targeted_by_jump_stmts);
   
    if(untargeted_label_syms->size() > 0){

       list<LabelLocationStatement*>* label_locations = collect_objects<LabelLocationStatement>(proc_def);
       for(list<LabelLocationStatement*>::iterator iter = label_locations->begin();
  	   iter != label_locations->end(); ){

	   if(is_in_list((*iter)->get_defined_label(), untargeted_label_syms)){
	      remove_statement(*iter);
	      iter = label_locations->erase(iter); 
	   }else iter++;
       }

    }

    delete code_labels_defined_by_label_location_stmts;
    delete code_labels_targeted_by_jump_stmts;
    delete untargeted_label_syms;

}

void remove_statements(list<Statement*>* to_be_removed){

    while(to_be_removed->size() > 0){

        Statement *stmt = *(to_be_removed->get_nth(0));	   
        
        if(stmt->get_parent() != NULL){
        if(is_a<StatementList>(stmt->get_parent())){
        
           StatementList *parent_list = to<StatementList>(stmt->get_parent());
           int i = 0;
           while(i < parent_list->get_statement_count())
              if(is_in_list(parent_list->get_statement(i), to_be_removed))
                 parent_list->remove_statement(i);
              else i++;
     
        }else remove_statement(stmt);
        }

        int i = 0;
        while(i < to_be_removed->size())
           if((*(to_be_removed->get_nth(i)))->get_parent() == NULL)
              to_be_removed->erase(i);
           else i++;
        }
}

bool is_stmt_within_begin_end_hw_marks(Statement *stmt){

    ProcedureDefinition *proc_def = get_procedure_definition(stmt);
           
    BrickAnnote *begin_hw_mark_annote = to<BrickAnnote>(proc_def->lookup_annote_by_name("begin_hw"));
    BrickAnnote *end_hw_mark_annote = to<BrickAnnote>(proc_def->lookup_annote_by_name("end_hw"));
            
    SuifObjectBrick *sob = to<SuifObjectBrick>(begin_hw_mark_annote->get_brick(0));   
    MarkStatement *begin_hw_mark_stmt = to<MarkStatement>(sob->get_object());
        
    sob = to<SuifObjectBrick>(end_hw_mark_annote->get_brick(0));
    MarkStatement *end_hw_mark_stmt = to<MarkStatement>(sob->get_object());
            
    if(is_textually_preceeding(begin_hw_mark_stmt, stmt, NULL))
       if(is_textually_preceeding(stmt, end_hw_mark_stmt, NULL))
 	  return 1;

   return 0;
}

int get_signed_bit_size(long N){

        int power = 1;

        while(power <= 32){
             if((long)pow(2,power) > N)
                return power+1;
             power++;
        }

        return power+1;
}

int get_unsigned_bit_size(long N){

        int power = 1;

        while(power <= 32){
             if((long)pow(2,power) > N)
                return power;
             power++;
        }

        return power;
}


