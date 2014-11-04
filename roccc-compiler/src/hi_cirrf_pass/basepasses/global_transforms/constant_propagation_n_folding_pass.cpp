// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include <common/suif_copyright.h>

#include <iostream>
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
#include "roccc_utils/annote_utils.h"
#include "roccc_utils/print_utils.h"
#include "roccc_utils/warning_utils.h"
#include "constant_propagation_n_folding_pass.h"

using namespace std;

/**************************** Declarations ************************************/

bool change;

class ExpressionTerm{
public:
  ExpressionTerm(Expression *t, String s) {term = t; sign = s;}
  ~ExpressionTerm() { }
  Expression* get_term() {return term;}
  String get_sign() {return sign;}
private:
  String sign;
  Expression *term; 
};

class cpfp_load_variable_expression_walker: public SelectiveWalker {
public:
  cpfp_load_variable_expression_walker(SuifEnv *env)
    :SelectiveWalker(env, LoadVariableExpression::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x);
};

class cpfp_binary_expression_walker: public SelectiveWalker {
public:
  cpfp_binary_expression_walker(SuifEnv *env)
    :SelectiveWalker(env, BinaryExpression::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

class cpfp_binary_expression_walker2: public SelectiveWalker {
public:
  cpfp_binary_expression_walker2(SuifEnv *env, String ops)
    :SelectiveWalker(env, BinaryExpression::get_class_name()) {opcode_set = ops;}
  Walker::ApplyStatus operator () (SuifObject *x); 

private:
  friend void combine_constants(list<ExpressionTerm*>*);
  friend list<ExpressionTerm*>* collect_same_precedence_exprs(Expression*, String);
  String opcode_set;
};

class cpfp_binary_expression_walker3: public SelectiveWalker {
public:
  cpfp_binary_expression_walker3(SuifEnv *env)
    :SelectiveWalker(env, BinaryExpression::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

class cpfp_binary_expression_walker4: public SelectiveWalker {
public:
  cpfp_binary_expression_walker4(SuifEnv *env, String ops)
    :SelectiveWalker(env, BinaryExpression::get_class_name()) {opcode_set = ops;}
  Walker::ApplyStatus operator () (SuifObject *x); 

private:
  friend list<ExpressionTerm*>* combine_shift_exprs(Expression*, String);
  String opcode_set;
};

class cpfp_unary_expression_walker: public SelectiveWalker {
public:
  cpfp_unary_expression_walker(SuifEnv *env)
    :SelectiveWalker(env, UnaryExpression::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};
/**************************** Implementations ************************************/
ConstantPropagationAndFoldingPass::ConstantPropagationAndFoldingPass(SuifEnv *pEnv) : 
					PipelinablePass(pEnv, "ConstantPropagationAndFoldingPass") {}

void ConstantPropagationAndFoldingPass::do_procedure_definition(ProcedureDefinition* proc_def){

  OutputInformation("Constant propagation and folding pass begins") ;
  if (proc_def)
  {
    change = 1;
    while(change)
    {
      change = 0;

      cpfp_load_variable_expression_walker walker(get_suif_env());
      proc_def->walk(walker);

      cpfp_binary_expression_walker3 binary_walker10(get_suif_env());
      proc_def->walk(binary_walker10);

      cpfp_unary_expression_walker unary_walker(get_suif_env());
      proc_def->walk(unary_walker);

      cpfp_binary_expression_walker binary_walker(get_suif_env());
      proc_def->walk(binary_walker);

      cpfp_binary_expression_walker2 binary_walker2(get_suif_env(), "add/subtract");
      proc_def->walk(binary_walker2);
      cpfp_binary_expression_walker2 binary_walker3(get_suif_env(), "bitwise_and");
      proc_def->walk(binary_walker3);
      cpfp_binary_expression_walker2 binary_walker4(get_suif_env(), "bitwise_xor");
      proc_def->walk(binary_walker4);
      cpfp_binary_expression_walker2 binary_walker5(get_suif_env(), "bitwise_or");
      proc_def->walk(binary_walker5);
      cpfp_binary_expression_walker2 binary_walker6(get_suif_env(), "multiply");
      proc_def->walk(binary_walker6);
      
      //	   cpfp_binary_expression_walker2 binary_walker7(get_suif_env(), "divide");
      //	   proc_def->walk(binary_walker7);
      
      //	   cpfp_binary_expression_walker4 binary_walker8(get_suif_env(), "left_shift/right_shift");
      //	   proc_def->walk(binary_walker8);
      
    }
  }
  OutputInformation("Constant propagation and folding pass ends") ;
}
   
Walker::ApplyStatus cpfp_load_variable_expression_walker::operator () (SuifObject *x) {
        LoadVariableExpression *load_var_expr = to<LoadVariableExpression>(x);

        BrickAnnote *reaching_defs = to<BrickAnnote>(load_var_expr->lookup_annote_by_name("reaching_defs"));
        Iter<SuifBrick*> iter = reaching_defs->get_brick_iterator();

        if(!iter.is_valid())
           return Walker::Continue;

        SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());

        if(!is_a<StoreVariableStatement>(sob->get_object()))
           return Walker::Continue;

        StoreVariableStatement *store_var_stmt = to<StoreVariableStatement>(sob->get_object());
        Expression *store_var_expr = store_var_stmt->get_value();

        if(!is_a<IntConstant>(store_var_expr))
           return Walker::Continue;

        IntConstant *store_var_constant = to<IntConstant>(store_var_expr);
        IInteger constant_value = store_var_constant->get_value();

        for(iter.next(); iter.is_valid(); iter.next()){
            sob = to<SuifObjectBrick>(iter.current());

            if(!is_a<StoreVariableStatement>(sob->get_object()))
               return Walker::Continue;

            store_var_stmt = to<StoreVariableStatement>(sob->get_object());
            store_var_expr = store_var_stmt->get_value();

            if(!is_a<IntConstant>(store_var_expr))
               return Walker::Continue;

            store_var_constant = to<IntConstant>(store_var_expr);

            if(constant_value != store_var_constant->get_value())
               return Walker::Continue;
        }

        for(Iter<SuifBrick*> iter = reaching_defs->get_brick_iterator();
            iter.is_valid(); iter.next()){
            SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());

            StoreVariableStatement *store_var_stmt = to<StoreVariableStatement>(sob->get_object());
            BrickAnnote *reached_uses = to<BrickAnnote>(store_var_stmt->lookup_annote_by_name("reached_uses"));

            remove_brick(reached_uses, load_var_expr);
        }
	
	// Added by Jason because we no longer do dead code elimination
	//  to remove the statements that were propagated away
	store_var_stmt->append_annote(create_brick_annote(get_env(),
							  "NonPrintable")) ;

        load_var_expr->get_parent()->replace(load_var_expr, clone_expression(store_var_constant));
	change = 1;

        return Walker::Continue;
}

Walker::ApplyStatus cpfp_binary_expression_walker3::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	BinaryExpression *binary_expr = to<BinaryExpression>(x);

	Expression *src1 = binary_expr->get_source1();
	Expression *src2 = binary_expr->get_source2();
	String opcode = String(binary_expr->get_opcode());

	if(is_a<IntConstant>(src1) || is_a<IntConstant>(src2))
    	   return Walker::Continue;

	Expression *replacement = NULL;
   
	if(is_a<LoadVariableExpression>(src1) && is_a<LoadVariableExpression>(src2)){
	   VariableSymbol *src1_sym = (to<LoadVariableExpression>(src1))->get_source();
	   VariableSymbol *src2_sym = (to<LoadVariableExpression>(src2))->get_source();
	   if(src1_sym == src2_sym){
	      if(opcode == String("subtract"))
	 	 replacement = create_int_constant(env, binary_expr->get_result_type(), IInteger(0));
	      else if(opcode == String("divide"))
	 	 replacement = create_int_constant(env, binary_expr->get_result_type(), IInteger(1));
	      else if(opcode == String("bitwise_and") || opcode == String("bitwise_or") ||
	              opcode == String("logical_and") || opcode == String("logical_or"))
	 	 replacement = create_load_variable_expression(env, binary_expr->get_result_type(), src1_sym);
	   }
	}

	if(is_a<UnaryExpression>(src1) && is_a<LoadVariableExpression>(src2)){
	   UnaryExpression *unary_expr = to<UnaryExpression>(src1);
 	   if(!is_a<LoadVariableExpression>(unary_expr->get_source()))
	      return Walker::Continue;

 	   String unary_expr_opcode = String(unary_expr->get_opcode());
	   VariableSymbol *src1_sym = (to<LoadVariableExpression>(unary_expr->get_source()))->get_source();
	   VariableSymbol *src2_sym = (to<LoadVariableExpression>(src2))->get_source();
	
	   if(src1_sym == src2_sym){
  	      if(opcode == String("add") && unary_expr_opcode == String("negate"))
	 	 replacement = create_int_constant(env, binary_expr->get_result_type(), IInteger(0));
  	      else if(opcode == String("divide") && unary_expr_opcode == String("invert"))
	 	 replacement = create_int_constant(env, binary_expr->get_result_type(), IInteger(1));
	      else if(opcode == String("bitwise_and") && unary_expr_opcode == String("bitwise_not"))
	 	 replacement = create_int_constant(env, binary_expr->get_result_type(), IInteger(0));
	      else if(opcode == String("bitwise_or") && unary_expr_opcode == String("bitwise_not"))
	 	 replacement = create_int_constant(env, binary_expr->get_result_type(), IInteger(1));
	      else if(opcode == String("logical_and") && unary_expr_opcode == String("logical_not")) 
	 	 replacement = create_int_constant(env, binary_expr->get_result_type(), IInteger(0));
	      else if(opcode == String("logical_or") && unary_expr_opcode == String("logical_not"))
	 	 replacement = create_int_constant(env, binary_expr->get_result_type(), IInteger(1));
	   }
	}
	   
	if(is_a<LoadVariableExpression>(src1) && is_a<UnaryExpression>(src2)){
	   VariableSymbol *src1_sym = (to<LoadVariableExpression>(src1))->get_source();

	   UnaryExpression *unary_expr = to<UnaryExpression>(src2);
 	   if(!is_a<LoadVariableExpression>(unary_expr->get_source()))
	      return Walker::Continue;

 	   String unary_expr_opcode = String(unary_expr->get_opcode());
	   VariableSymbol *src2_sym = (to<LoadVariableExpression>(unary_expr->get_source()))->get_source();
	
	   if(src1_sym == src2_sym){
  	      if(opcode == String("add") && unary_expr_opcode == String("negate"))
	 	 replacement = create_int_constant(env, binary_expr->get_result_type(), IInteger(0));
  	      else if(opcode == String("divide") && unary_expr_opcode == String("invert"))
	 	 replacement = create_int_constant(env, binary_expr->get_result_type(), IInteger(1));
	      else if(opcode == String("bitwise_and") && unary_expr_opcode == String("bitwise_not"))
	 	 replacement = create_int_constant(env, binary_expr->get_result_type(), IInteger(0));
	      else if(opcode == String("bitwise_or") && unary_expr_opcode == String("bitwise_not"))
	 	 replacement = create_int_constant(env, binary_expr->get_result_type(), IInteger(1));
	      else if(opcode == String("logical_and") && unary_expr_opcode == String("logical_not")) 
	 	 replacement = create_int_constant(env, binary_expr->get_result_type(), IInteger(0));
	      else if(opcode == String("logical_or") && unary_expr_opcode == String("logical_not"))
	 	 replacement = create_int_constant(env, binary_expr->get_result_type(), IInteger(1));
	   }
	}
	   
	if(replacement){
 	   change = 1;
           binary_expr->get_parent()->replace(binary_expr, replacement);

	   set_address(get_expression_owner(replacement));
    	   return Walker::Replaced;
	}

 	return Walker::Continue;
}

Walker::ApplyStatus cpfp_binary_expression_walker::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	BinaryExpression *binary_expr = to<BinaryExpression>(x);

	Expression *src1 = binary_expr->get_source1();
	Expression *src2 = binary_expr->get_source2();
	String opcode = String(binary_expr->get_opcode());

	if(!is_a<IntConstant>(src1) && !is_a<IntConstant>(src2))
    	   return Walker::Continue;

	Expression *replacement = NULL;

	if(!is_a<IntConstant>(src1)){
	   IInteger src2_value = (to<IntConstant>(src2))->get_value();
	   if(src2_value.c_int() == 0){ 
 	      if(opcode == String("add"))
	         replacement = clone_expression(src1);
 	      else if(opcode == String("subtract"))
	         replacement = clone_expression(src1);
 	      else if(opcode == String("multiply"))
	         replacement = clone_expression(src2);
   	      else if(opcode == String("bitwise_and"))
	         replacement = clone_expression(src2);
 	      else if(opcode == String("bitwise_or"))
	         replacement = clone_expression(src1);
 	      else if(opcode == String("left_shift"))
	         replacement = clone_expression(src1);
 	      else if(opcode == String("right_shift"))
	         replacement = clone_expression(src1);
	   }else if(src2_value.c_int() == 1){ 
 	      if(opcode == String("multiply"))
	         replacement = clone_expression(src1);
 	      else if(opcode == String("divide"))
	         replacement = clone_expression(src1);
	   }
	   if(replacement){
	      change = 1;
              binary_expr->get_parent()->replace(binary_expr, replacement);
       	      set_address(get_expression_owner(replacement));
    	      return Walker::Replaced;
	   }else return Walker::Continue;	
	}

	if(!is_a<IntConstant>(src2)){
	   IInteger src1_value = (to<IntConstant>(src1))->get_value();
	   if(src1_value.c_int() == 0){ 
 	      if(opcode == String("add"))
	         replacement = clone_expression(src2);
 	      else if(opcode == String("subtract"))
	         replacement = create_unary_expression(env, binary_expr->get_result_type(), k_negate, clone_expression(src2));
 	      else if(opcode == String("multiply"))
	         replacement = clone_expression(src1);
 	      else if(opcode == String("divide"))
	         replacement = clone_expression(src1);
 	      else if(opcode == String("remainder"))
	         replacement = clone_expression(src1);
   	      else if(opcode == String("bitwise_and"))
	         replacement = clone_expression(src1);
 	      else if(opcode == String("bitwise_or"))
	         replacement = clone_expression(src2);
 	      else if(opcode == String("left_shift"))
	         replacement = clone_expression(src1);
 	      else if(opcode == String("right_shift"))
	         replacement = clone_expression(src1);
	   }else if(src1_value.c_int() == 1){ 
 	      if(opcode == String("multiply"))
	         replacement = clone_expression(src2);
	   }
	   if(replacement){
	      change = 1;
              binary_expr->get_parent()->replace(binary_expr, replacement);
       	      set_address(get_expression_owner(replacement));
    	      return Walker::Replaced;
	   }else return Walker::Continue;	
	}

	IInteger src1_value = (to<IntConstant>(src1))->get_value();
	IInteger src2_value = (to<IntConstant>(src2))->get_value();

	IInteger result_value;

 	if(opcode == String("add"))
	   result_value = src1_value + src2_value;
 	else if(opcode == String("subtract"))
	   result_value = src1_value - src2_value;
 	else if(opcode == String("multiply"))
	   result_value = src1_value * src2_value;
 	else if(opcode == String("divide"))
	   result_value = src1_value / src2_value;
 	else if(opcode == String("remainder"))
	   result_value = src1_value % src2_value;
 	else if(opcode == String("bitwise_and"))
	   result_value = src1_value & src2_value;
 	else if(opcode == String("bitwise_or"))
	   result_value = src1_value | src2_value;
 	else if(opcode == String("bitwise_xor"))
	   result_value = src1_value ^ src2_value;
 	else if(opcode == String("left_shift"))
	   result_value = src1_value << src2_value;
 	else if(opcode == String("right_shift"))
	   result_value = src1_value >> src2_value;
 	else if(opcode == String("is_equal_to"))
	   result_value = IInteger((int)(src1_value == src2_value));
 	else if(opcode == String("is_not_equal_to"))
	   result_value = IInteger((int)(src1_value != src2_value));
 	else if(opcode == String("is_less_than"))
	   result_value = IInteger((int)(src1_value < src2_value));
 	else if(opcode == String("is_less_than_or_equal_to"))
	   result_value = IInteger((int)(src1_value <= src2_value));
 	else if(opcode == String("is_greater_than"))
	   result_value = IInteger((int)(src1_value > src2_value));
 	else if(opcode == String("is_greater_than_or_equal_to"))
	   result_value = IInteger((int)(src1_value >= src2_value));
 	else if(opcode == String("logical_and"))
	   result_value = IInteger((int)(src1_value && src2_value));
 	else if(opcode == String("logical_or"))
	   result_value = IInteger((int)(src1_value || src2_value));

        replacement = create_int_constant(env, binary_expr->get_result_type(), result_value);

	change = 1;
        binary_expr->get_parent()->replace(binary_expr, replacement);

	set_address(replacement);
    	return Walker::Replaced;
}

int combine_constants(list<ExpressionTerm*>* same_precedence_exprs, String opcode){

	bool initialization = 1;
	int int_constant_term_count = 0; 
	int result = 0;
	
	list<ExpressionTerm*>::iterator iter = same_precedence_exprs->begin();
	while(iter != same_precedence_exprs->end()){
	    ExpressionTerm *expr_term = *iter;
	    Expression *term = expr_term->get_term();
	    if(is_a<IntConstant>(term)){

	       int value = (to<IntConstant>(term))->get_value().c_int();
	       value = expr_term->get_sign() == String("+") ? value : 0 - value;   	

	       if(opcode == String("add") || opcode == String("subtract"))
	          result = initialization ? value : result + value;
	       else if(opcode == String("bitwise_and"))
	          result = initialization ? value : result & value;
	       else if(opcode == String("bitwise_xor"))
	          result = initialization ? value : result ^ value;
	       else if(opcode == String("bitwise_or"))
	          result = initialization ? value : result | value;
	       else if(opcode == String("multiply"))
	          result = initialization ? value : result * value;

	       iter = same_precedence_exprs->erase(iter); 
	       delete expr_term;
	       int_constant_term_count++;
	       initialization = 0;

   	    }else iter++;
	}
	change = change || (int_constant_term_count > 1);
	return result;
}

void collect_same_precedence_exprs(list<ExpressionTerm*>* same_precedence_exprs, Expression *expr_node, String opcode_set){
        
	static list<String> *opcode_stack = new list<String>;
	
        if(is_a<BinaryExpression>(expr_node)){
           BinaryExpression *binary_expr = to<BinaryExpression>(expr_node);

           if(opcode_set.find(binary_expr->get_opcode()) == -1){

 	      if(!opcode_stack->empty()){
                 if(opcode_stack->back() == String("subtract"))
                    same_precedence_exprs->push_front(new ExpressionTerm(binary_expr, "-"));
                 else same_precedence_exprs->push_front(new ExpressionTerm(binary_expr, "+"));
	         opcode_stack->pop_back();
	      }else same_precedence_exprs->push_front(new ExpressionTerm(expr_node, "+"));

	      return;
 	   }
 
	   if(!opcode_stack->empty()){
              if(opcode_stack->back() == String("subtract")){
	         if(binary_expr->get_opcode() == String("subtract") ) 
	            opcode_stack->push_back(String("add"));
	         else opcode_stack->push_back(String("subtract"));
	      }else opcode_stack->push_back(binary_expr->get_opcode());
	   }else opcode_stack->push_back(binary_expr->get_opcode());

	   collect_same_precedence_exprs(same_precedence_exprs, binary_expr->get_source2(), opcode_set);
	   collect_same_precedence_exprs(same_precedence_exprs, binary_expr->get_source1(), opcode_set);

        }else if(!opcode_stack->empty()){

	   if(opcode_stack->back() == String("subtract"))
              same_precedence_exprs->push_front(new ExpressionTerm(expr_node, "-"));
           else same_precedence_exprs->push_front(new ExpressionTerm(expr_node, "+"));
           opcode_stack->pop_back();

	}else same_precedence_exprs->push_front(new ExpressionTerm(expr_node, "+"));

}

Walker::ApplyStatus cpfp_binary_expression_walker2::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	BinaryExpression *binary_expr = to<BinaryExpression>(x);

	Expression *src1 = binary_expr->get_source1();
	Expression *src2 = binary_expr->get_source2();
	String opcode = String(binary_expr->get_opcode());

	if(!is_a<IntConstant>(src1) && !is_a<IntConstant>(src2))
	   return Walker::Continue;

	if(opcode_set.find(opcode) == -1)
	   return Walker::Continue;

	list<ExpressionTerm*>* same_precedence_exprs = new list<ExpressionTerm*>;
	collect_same_precedence_exprs(same_precedence_exprs, binary_expr, opcode_set);

	int result = combine_constants(same_precedence_exprs, opcode);
	same_precedence_exprs->push_front(new ExpressionTerm(create_int_constant(env, binary_expr->get_result_type(), IInteger(result)), "+"));

	list<ExpressionTerm*>::iterator iter = same_precedence_exprs->begin();
	ExpressionTerm *expr_term = *iter;
	Expression *replacement = to<Expression>(expr_term->get_term());

	for(++iter; iter != same_precedence_exprs->end(); iter++){
	    expr_term = *iter;
	    Expression *expr = to<Expression>(deep_suif_clone(expr_term->get_term()));
	    if(expr_term->get_sign() == String("+")){
	       if(opcode == String("subtract"))
	          replacement = create_binary_expression(env, expr->get_result_type(), String("add"), 
							replacement, expr);
	       else replacement = create_binary_expression(env, expr->get_result_type(), opcode, 
							replacement, expr);
	    }else replacement = create_binary_expression(env, expr->get_result_type(), opcode, replacement, 
							create_unary_expression(env, expr->get_result_type(), 
 							String("negate"), expr));
	}

	binary_expr->get_parent()->replace(binary_expr, replacement);

	for(list<ExpressionTerm*>::iterator iter = same_precedence_exprs->begin();
	    iter != same_precedence_exprs->end(); iter++)
	    delete *iter;
	delete same_precedence_exprs;

	set_address(replacement);
	return Walker::Replaced;
}

/*

int collect_shift_exprs(list<Expression*>* shift_exprs, list<Expression*>* non_shift_exprs, Expression *expr_node){
       

        if(is_a<BinaryExpression>(expr_node)){

           BinaryExpression *binary_expr = to<BinaryExpression>(expr_node);
	   String opcode = binary_expr->get_opcode();

           if(opcode == String("left_shift") || opcode == String("right_shift")){
	      
  	      Expression *src2 = binary_expr->get_source2();
     	      if(!is_a<IntConstant>(src2))
	         return 0;

	      shift_exprs->push_back(binary_expr);
              return 0;

 	   }else if(opcode == String("add") || opcode == String("subtract")){
	
	      return 1 + max(collect_shift_exprs(shift_exprs, binary_expr->get_source2()),
                             collect_shift_exprs(shift_exprs, binary_expr->get_source1()));

 	   }else non_shift_exprs->push_back(expr_node);
 
        }else if(is_a<UnaryExpression>(expr_node)){

           UnaryExpression *unary_expr = to<UnaryExpression>(expr_node);
	   String opcode = unary_expr->get_opcode();

           if(opcode == String("negate"))
	      return collect_shift_exprs(shift_exprs, unary_expr->get_source());

	}

	return 1;
}

Walker::ApplyStatus cpfp_binary_expression_walker4::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	BinaryExpression *binary_expr = to<BinaryExpression>(x);

	Expression *src2 = binary_expr->get_source2();
	String opcode = String(binary_expr->get_opcode());

	if(!is_a<IntConstant>(src2))
	   return Walker::Continue;

	if(opcode_set.find(opcode) == -1)
	   return Walker::Continue;

	list<Expression*>* shift_exprs = new list<Expression*>;
	int tree_depth = collect_shift_exprs(shift_exprs, binary_expr->get_source1());

	if(shift_exprs->size() < tree_depth/2){
	   delete shift_exprs;	
	   return Walker::Continue;
 	}

	for(list<BinaryExpression*>::iterator iter = shift_exprs->begin(); 
	    iter != shift_exprs->end(); iter++){

	    BinaryExpression *binary_expr = *iter;

	    if(binary_expr == String("left_shift")){


	    }else{


	    }

	}

	binary_expr->set_source1(0);
	binary_expr->get_parent()->replace(binary_expr, src1);

	set_address(replacement);
	return Walker::Replaced;
}

*/

Walker::ApplyStatus cpfp_unary_expression_walker::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	UnaryExpression *unary_expr = to<UnaryExpression>(x);

	if(!is_a<IntConstant>(unary_expr->get_source()))
	   return Walker::Continue;

	String opcode = String(unary_expr->get_opcode());
	int unary_expr_source = (to<IntConstant>(unary_expr->get_source()))->get_value().c_int();
	int unary_expr_result;
	
	if(opcode == String("negate"))
	   unary_expr_result = 0 - unary_expr_source;
	else if(opcode == String("invert") && unary_expr_source != 0)
	   unary_expr_result = 1 / unary_expr_source;
	else if(opcode == String("bitwise_not"))
	   unary_expr_result = ~unary_expr_source;
	else if(opcode == String("logical_not"))
	   unary_expr_result = !unary_expr_source;
	else {
	   change = 0;
	   return Walker::Continue;
	}

	Expression *replacement = create_int_constant(env, unary_expr->get_result_type(), IInteger(unary_expr_result));
	
	change = 1;         
	unary_expr->get_parent()->replace(unary_expr, replacement);

	set_address(replacement);
	return Walker::Replaced;
}
