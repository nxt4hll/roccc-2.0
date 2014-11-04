// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include <common/suif_copyright.h>

#include <iostream>
#include <iokernel/cast.h>
#include <iokernel/clone_stream.h>
#include <common/i_integer.h>
#include "common/suif_map.h"
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>
#include <basicnodes/basic.h>
#include <basicnodes/basic_constants.h>
#include <suifkernel/suifkernel_messages.h>
#include <suifkernel/utilities.h> 
#include <suifkernel/group_walker.h> 
#include <typebuilder/type_builder.h>
#include <cfenodes/cfe.h>
#include "utils/expression_utils.h"
#include "utils/symbol_utils.h"
#include "utils/type_utils.h"
#include "roccc_utils/IR_utils.h"
#include "roccc_utils/symbol_utils.h"
#include "roccc_utils/type_utils.h"
#include "dfa_state_table_expansion_pass.h"

using namespace std;

/**************************** Declarations ************************************/

class dste_call_statement_walker: public SelectiveWalker {
public:
  dste_call_statement_walker(SuifEnv *env)
    :SelectiveWalker(env, CallStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

class dste_call_statement_walker2: public SelectiveWalker {
public:
  dste_call_statement_walker2(SuifEnv *env)
    :SelectiveWalker(env, CallStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

class ST : public SuifObject{
public:
  ST(VariableSymbol *st, VariableSymbol *cs, int l, int sc, int ac, int ss, int es, int fs){
     state_table_sym = st; 
     char_set_sym = cs; 
     length = l; 
     state_count = sc; 
     alphabet_count = ac;
     start_state = ss;
     error_state = es;
     final_state = fs;
  }
  ~ST() {}
  VariableSymbol* get_state_table_sym() {return state_table_sym;}
  VariableSymbol* get_char_set_sym() {return char_set_sym;}
  int get_length() {return length;}
  int get_state_count() {return state_count;}
  int get_alphabet_count() {return alphabet_count;}
  int get_start_state() {return start_state;}
  int get_error_state() {return error_state;}
  int get_final_state() {return final_state;}
private:
  VariableSymbol *state_table_sym;
  VariableSymbol *char_set_sym;
  int length;
  int state_count;
  int alphabet_count;
  int start_state, error_state, final_state;
};

suif_map<int, ST*>* state_table_defs;
SuifObject *proc_def_body;

/**************************** Implementations ************************************/
DFA_StateTableExpansionPass::DFA_StateTableExpansionPass(SuifEnv *pEnv) : 
				PipelinablePass(pEnv, "DFA_StateTableExpansionPass") {}

void DFA_StateTableExpansionPass::do_procedure_definition(ProcedureDefinition* proc_def){

    cout << "dfa state table expansion pass" << endl;
    if (proc_def){

	state_table_defs = new suif_map<int, ST*>;
        proc_def_body = proc_def->get_body();

  	dste_call_statement_walker walker(get_suif_env());
	proc_def->walk(walker);

	if(state_table_defs->size() > 0){
           dste_call_statement_walker2 walker2(get_suif_env());
	   proc_def->walk(walker2);
	}
    }
    cout << "dfa state table expansion pass" << endl;
}

Walker::ApplyStatus dste_call_statement_walker::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	CallStatement *call_stmt = to<CallStatement>(x);

	SymbolAddressExpression *callee_address = to<SymbolAddressExpression>(call_stmt->get_callee_address());
	ProcedureSymbol *proc_symbol = to<ProcedureSymbol>(callee_address->get_addressed_symbol());
	String called_proc_name = proc_symbol->get_name();
	
	if(called_proc_name == String("ROCCC_create_state_table")){

  	   Expression *argument_expr = call_stmt->get_argument(0);
	   if(is_a<UnaryExpression>(argument_expr))
	      argument_expr = (to<UnaryExpression>(argument_expr))->get_source();
	   int st_id = (to<IntConstant>(argument_expr))->get_value().c_int();

  	   argument_expr = call_stmt->get_argument(1);
	   if(is_a<UnaryExpression>(argument_expr))
	      argument_expr = (to<UnaryExpression>(argument_expr))->get_source();
	   SymbolAddressExpression *array_symbol_expr = to<SymbolAddressExpression>(argument_expr);
	   VariableSymbol *state_table_sym = to<VariableSymbol>(array_symbol_expr->get_addressed_symbol());

  	   argument_expr = call_stmt->get_argument(2);
	   if(is_a<UnaryExpression>(argument_expr))
	      argument_expr = (to<UnaryExpression>(argument_expr))->get_source();
	   array_symbol_expr = to<SymbolAddressExpression>(argument_expr);
	   VariableSymbol *char_array_sym = to<VariableSymbol>(array_symbol_expr->get_addressed_symbol());

  	   argument_expr = call_stmt->get_argument(3);
	   if(is_a<UnaryExpression>(argument_expr))
	      argument_expr = (to<UnaryExpression>(argument_expr))->get_source();
	   int length = (to<IntConstant>(argument_expr))->get_value().c_int();

  	   argument_expr = call_stmt->get_argument(4);
	   if(is_a<UnaryExpression>(argument_expr))
	      argument_expr = (to<UnaryExpression>(argument_expr))->get_source();
	   int state_count = (to<IntConstant>(argument_expr))->get_value().c_int();

  	   argument_expr = call_stmt->get_argument(5);
	   if(is_a<UnaryExpression>(argument_expr))
	      argument_expr = (to<UnaryExpression>(argument_expr))->get_source();
	   int alphabet_count = (to<IntConstant>(argument_expr))->get_value().c_int();

  	   argument_expr = call_stmt->get_argument(6);
	   if(is_a<UnaryExpression>(argument_expr))
	      argument_expr = (to<UnaryExpression>(argument_expr))->get_source();
	   int start_state = (to<IntConstant>(argument_expr))->get_value().c_int();

  	   argument_expr = call_stmt->get_argument(7);
	   if(is_a<UnaryExpression>(argument_expr))
	      argument_expr = (to<UnaryExpression>(argument_expr))->get_source();
	   int error_state = (to<IntConstant>(argument_expr))->get_value().c_int();

  	   argument_expr = call_stmt->get_argument(8);
	   if(is_a<UnaryExpression>(argument_expr))
	      argument_expr = (to<UnaryExpression>(argument_expr))->get_source();
	   int final_state = (to<IntConstant>(argument_expr))->get_value().c_int();

	   state_table_defs->enter_value(st_id, new ST(state_table_sym, char_array_sym, length, state_count, alphabet_count,
										start_state, error_state, final_state));
		
  	   call_stmt->get_parent()->replace(call_stmt, create_statement_list(env));
	}
	
	return Walker::Continue;
}

Walker::ApplyStatus dste_call_statement_walker2::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	CallStatement *call_stmt = to<CallStatement>(x);

	SymbolAddressExpression *callee_address = to<SymbolAddressExpression>(call_stmt->get_callee_address());
	ProcedureSymbol *proc_symbol = to<ProcedureSymbol>(callee_address->get_addressed_symbol());
	String called_proc_name = proc_symbol->get_name();
	
	if(called_proc_name == String("ROCCC_lookup_in_state_table")){

 	   Expression *argument_expr = call_stmt->get_argument(0);
	   if(is_a<UnaryExpression>(argument_expr))
	      argument_expr = (to<UnaryExpression>(argument_expr))->get_source();
	   int st_id = (to<IntConstant>(argument_expr))->get_value().c_int();

	   suif_map<int, ST*>::iterator iter = state_table_defs->find(st_id);
	   if(iter == state_table_defs->end())
	      return Walker::Continue;
	
	   ST* state_table_def = (*iter).second;
	   int pattern_length = state_table_def->get_length();
	   int state_count = state_table_def->get_state_count();
	   int alphabet_count = state_table_def->get_alphabet_count();
	   int start_state = state_table_def->get_start_state();
	   int error_state = state_table_def->get_error_state();
	   int final_state = state_table_def->get_final_state();

	   VariableSymbol *char_set_array_sym = state_table_def->get_char_set_sym();
	   DataType *stream_data_type = get_array_element_type(char_set_array_sym);
	   QualifiedType *qualed_stream_data_type = retrieve_qualified_type(stream_data_type);

	   VariableSymbol *state_table_sym = state_table_def->get_state_table_sym();
	   DataType *dfa_state_data_type = get_array_element_type(state_table_sym);
	   QualifiedType *qualed_dfa_state_type = retrieve_qualified_type(dfa_state_data_type);

	   StatementList *replacement = create_statement_list(env);

 	   argument_expr = call_stmt->get_argument(1);
	   argument_expr->set_parent(0);
	   if(is_a<UnaryExpression>(argument_expr))
	      argument_expr = (to<UnaryExpression>(argument_expr))->get_source();

	   VariableSymbol* input_char_sym = new_anonymous_variable(env, proc_def_body, qualed_stream_data_type);
	   name_variable(input_char_sym, "ic");

	   StoreVariableStatement *store_var_stmt = create_store_variable_statement(env, input_char_sym, argument_expr);
	   replacement->append_statement(store_var_stmt);

           TypeBuilder *tb = get_type_builder(env);
           SymbolTable *external_symbol_table = get_external_symbol_table(get_procedure_definition(call_stmt));

           list<QualifiedType*> cam_proc_argument_types;
	   for(int j = 0; j < alphabet_count+1; j++)
	       cam_proc_argument_types.push_back(qualed_stream_data_type);
           DataType *cam_return_type;
           int cam_return_bit_size = get_unsigned_bit_size(alphabet_count);
	   cam_return_type = tb->get_integer_type(cam_return_bit_size, cam_return_bit_size, 0);
           cam_return_type->set_name(String("ROCCC_int") + String(cam_return_bit_size));
           CProcedureType *cam_proc_type = tb->get_c_procedure_type(cam_return_type, cam_proc_argument_types);
	   ProcedureSymbol *cam_proc_sym = create_procedure_symbol(env, cam_proc_type, String("ROCCC_cam") + String(alphabet_count));
           external_symbol_table->add_symbol(cam_proc_sym);

	   VariableSymbol* alphabet_index_sym = new_anonymous_variable(env, proc_def_body, retrieve_qualified_type(cam_return_type));
	   name_variable(alphabet_index_sym, "ais");

           SymbolAddressExpression *cam_proc_sym_expr = create_symbol_address_expression(env, tb->get_pointer_type(cam_proc_type), cam_proc_sym);
           CallStatement *cam_macro_stmt = create_call_statement(env, alphabet_index_sym, cam_proc_sym_expr);
	   for(int j = 0; j < alphabet_count; j++){

	       Expression *cam_argument_expr = create_symbol_address_expression(env, char_set_array_sym->get_type()->get_base_type(), char_set_array_sym);
	       cam_argument_expr = create_array_reference_expression(env, stream_data_type, cam_argument_expr, 
								create_int_constant(env, tb->get_integer_type(), IInteger(j)));
	       cam_argument_expr = create_load_expression(env, stream_data_type, cam_argument_expr);

               cam_macro_stmt->append_argument(cam_argument_expr);
	   }

           cam_macro_stmt->append_argument(create_load_variable_expression(env, stream_data_type, input_char_sym));
 	   replacement->append_statement(cam_macro_stmt);

	   suif_vector<VariableSymbol*>* state_column_vars = new suif_vector<VariableSymbol*>;
	   for(int i = 0; i < state_count; i++){

 	       VariableSymbol* state_column_sym = new_anonymous_variable(env, proc_def_body, qualed_dfa_state_type);
	       name_variable(state_column_sym, "st");
	       state_column_vars->push_back(state_column_sym);
	   }

           list<QualifiedType*> mux_proc_argument_types;
	   for(int j = 0; j < alphabet_count; j++)
	       mux_proc_argument_types.push_back(qualed_dfa_state_type);
	   mux_proc_argument_types.push_back(retrieve_qualified_type(cam_return_type));
           CProcedureType *mux_proc_type = tb->get_c_procedure_type(dfa_state_data_type, mux_proc_argument_types);
	   ProcedureSymbol *mux_proc_sym = create_procedure_symbol(env, mux_proc_type, String("ROCCC_mux") + String(alphabet_count));
           external_symbol_table->add_symbol(mux_proc_sym);

           ArrayType *state_table_array_type = to<ArrayType>(state_table_sym->get_type()->get_base_type());
	   for(int i = 0; i < state_count; i++){

	       if(i == error_state){
	          StoreVariableStatement *state_column_var_init_stmt = create_store_variable_statement(env, state_column_vars->at(i), 
									create_int_constant(env, tb->get_integer_type(), IInteger(i)));
	          replacement->append_statement(state_column_var_init_stmt);
		  continue;
	       }

               SymbolAddressExpression *mux_proc_sym_expr = create_symbol_address_expression(env, tb->get_pointer_type(mux_proc_type), mux_proc_sym);
               CallStatement *mux_macro_stmt = create_call_statement(env, state_column_vars->at(i), mux_proc_sym_expr);

	       for(int j = 0; j < alphabet_count; j++){
	           Expression *mux_argument_expr = create_symbol_address_expression(env, state_table_array_type, state_table_sym);
	           mux_argument_expr = create_array_reference_expression(env, state_table_array_type->get_element_type()->get_base_type(),
		  					mux_argument_expr, create_int_constant(env, tb->get_integer_type(), IInteger(i)));
	           mux_argument_expr = create_array_reference_expression(env, dfa_state_data_type, mux_argument_expr, 
							create_int_constant(env, tb->get_integer_type(), IInteger(j)));
	           mux_argument_expr = create_load_expression(env, dfa_state_data_type, mux_argument_expr);

                   mux_macro_stmt->append_argument(mux_argument_expr);
	       }
	   
               mux_macro_stmt->append_argument(create_load_variable_expression(env, cam_return_type, alphabet_index_sym));
	       replacement->append_statement(mux_macro_stmt);
	   }

	   suif_vector<VariableSymbol*>* current_state_vars = new suif_vector<VariableSymbol*>;
	   suif_vector<VariableSymbol*>* next_state_vars = new suif_vector<VariableSymbol*>;

	   StatementList *var_init_stmt_list = create_statement_list(env);
	   for(int i = 0; i < pattern_length; i++){

 	       VariableSymbol* current_state_sym = new_anonymous_variable(env, proc_def_body, qualed_dfa_state_type);
	       name_variable(current_state_sym, "cs");
               BrickAnnote *feedback_level_annote = create_brick_annote(env, "feedback_level");
               current_state_sym->append_annote(feedback_level_annote);
               feedback_level_annote->insert_brick(0, create_integer_brick(env, IInteger(0)));
	       current_state_vars->push_back(current_state_sym);

	       StoreVariableStatement *init_var_stmt = create_store_variable_statement(env, current_state_sym, 
							create_int_constant(env, dfa_state_data_type, IInteger(start_state)));
	       var_init_stmt_list->append_statement(init_var_stmt);

 	       VariableSymbol* next_state_sym = new_anonymous_variable(env, proc_def_body, qualed_dfa_state_type);
	       name_variable(next_state_sym, "ns");
	       next_state_vars->push_back(next_state_sym);
	   }
	   insert_statement_before(get_enclosing_c_for_stmt(call_stmt), var_init_stmt_list);

           mux_proc_argument_types.empty();
	   for(int j = 0; j < state_count+1; j++)
	       mux_proc_argument_types.push_back(qualed_dfa_state_type);
           mux_proc_type = tb->get_c_procedure_type(dfa_state_data_type, mux_proc_argument_types);
           mux_proc_sym = create_procedure_symbol(env, mux_proc_type, String("ROCCC_mux") + String(state_count));
           external_symbol_table->add_symbol(mux_proc_sym);

	   for(int i = 0; i < pattern_length; i++){

               SymbolAddressExpression *mux_proc_sym_expr = create_symbol_address_expression(env, tb->get_pointer_type(mux_proc_type), mux_proc_sym);
               CallStatement *mux_macro_stmt = create_call_statement(env, next_state_vars->at(i), mux_proc_sym_expr);

	       for(int k = 0; k < state_count; k++)
	           mux_macro_stmt->append_argument(create_load_variable_expression(env, dfa_state_data_type, state_column_vars->at(k)));

	       mux_macro_stmt->append_argument(create_load_variable_expression(env, dfa_state_data_type, current_state_vars->at(i)));
	       replacement->append_statement(mux_macro_stmt);
	   }

           Expression *equality_check_expr = create_binary_expression(env, tb->get_boolean_type(), String("is_equal_to"),
                                                create_load_variable_expression(env, dfa_state_data_type, next_state_vars->at(0)),
                                                create_int_constant(env, dfa_state_data_type, IInteger(final_state)));
           VariableSymbol *destination_sym = call_stmt->get_destination();
	   StoreVariableStatement *result_stmt = create_store_variable_statement(env, destination_sym, equality_check_expr); 
	   replacement->append_statement(result_stmt);
           for(int i = 1; i < pattern_length; i++){
               equality_check_expr = create_binary_expression(env, tb->get_boolean_type(), String("is_equal_to"),
                                                create_load_variable_expression(env, dfa_state_data_type, next_state_vars->at(i)),
                                                create_int_constant(env, dfa_state_data_type, IInteger(final_state)));
               Expression *result_expr = create_binary_expression(env, tb->get_boolean_type(), String("logical_or"),
                                                create_load_variable_expression(env, tb->get_boolean_type(), destination_sym),
                                                equality_check_expr);
	       result_stmt = create_store_variable_statement(env, destination_sym, result_expr); 
	       replacement->append_statement(result_stmt);
           }           
                    
           for(int i = 0; i < pattern_length-1; i++){
               
               StoreVariableStatement *store_var_stmt = create_store_variable_statement(env, current_state_vars->at(i+1),
                                        create_load_variable_expression(env, dfa_state_data_type, next_state_vars->at(i)));
               replacement->append_statement(store_var_stmt);
           }   

  	   call_stmt->get_parent()->replace(call_stmt, replacement);
	   return Walker::Truncate;
  	}

        return Walker::Continue;
}

