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
#include "roccc_utils/loop_utils.h"
#include "roccc_utils/IR_utils.h"
#include "roccc_extra_types/array_info.h"
#include "roccc_utils/warning_utils.h"
#include "preprocessing_pass.h"

using namespace std;

// THIS PASS ANNOTES ALL THE ARRAY REFERENCE EXPRESSIONS W/ THE COEFFICIENTS 
// OF ITS INDEX EXPRESSION.

// FOR THE REMAINING DATA DEPENDENCE ANALYSIS PASSES TO WORK CORRECTLY, 
// THIS PASS SHOULD GET EXECUTED FIRST.

// JUMP INDIRECT IS NOT SUPPORTED
// MULTI WAY BRANCH (SWITCH/CASE) IS NOT SUPPORTED

// TO BE COMPLETED: CForStatement preprocessing

/**************************** Declarations ************************************/

class pp_c_for_statement_walker_PreprocessPass: public SelectiveWalker {
public:
  pp_c_for_statement_walker_PreprocessPass(SuifEnv *env)
    :SelectiveWalker(env, CForStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

class pp_array_reference_expression_walker: public SelectiveWalker {
public:
  pp_array_reference_expression_walker(SuifEnv *env)
    :SelectiveWalker(env, ArrayReferenceExpression::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

String get_var(Expression *index_expr);
int get_a(Expression *index_expr);
int get_c(Expression *index_expr);

/**************************** Implementations ************************************/
PreprocessingPass::PreprocessingPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "PreprocessingPass") {}

void PreprocessingPass::do_procedure_definition(ProcedureDefinition* proc_def){
  OutputInformation("Original preprocessing pass begins") ;
  if (proc_def)
  {
    pp_c_for_statement_walker_PreprocessPass walker(get_suif_env());
    proc_def->walk(walker);
    pp_array_reference_expression_walker walker2(get_suif_env());
    proc_def->walk(walker2);
  }
  OutputInformation("Original preprocessing pass ends") ;
}

Walker::ApplyStatus pp_c_for_statement_walker_PreprocessPass::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	CForStatement *c_for_stmt = to<CForStatement>(x);

        Iter<CForStatement> iter = object_iterator<CForStatement>(c_for_stmt->get_body());
        if (iter.is_valid())
            return Walker::Continue;

	BrickAnnote *c_for_info = to<BrickAnnote>(c_for_stmt->lookup_annote_by_name("c_for_info"));
	int nest_level = ((to<IntegerBrick>(c_for_info->get_brick(0)))->get_value()).c_int(); 
	
	list<BrickAnnote*>* nest_annotes = new list<BrickAnnote*>;
	nest_annotes->push_back(c_for_info);

	CForStatement *current_c_for_stmt = c_for_stmt;

	for(int i = 0; i<nest_level; i++){
	    VariableSymbol *induction_var_sym = get_c_for_basic_induction_variable(current_c_for_stmt);
	    int lower_bound = get_c_for_lower_bound(current_c_for_stmt);
	    int upper_bound = get_c_for_upper_bound(current_c_for_stmt);
	    int step = get_c_for_step(current_c_for_stmt);

	    for(list<BrickAnnote*>::iterator iter = nest_annotes->begin();
		iter != nest_annotes->end(); iter++){
		BrickAnnote *current_annote = *iter;
		current_annote->append_brick(create_string_brick(env, induction_var_sym->get_name())); 
		current_annote->append_brick(create_integer_brick(env, lower_bound)); 
		current_annote->append_brick(create_integer_brick(env, upper_bound)); 
		current_annote->append_brick(create_integer_brick(env, step)); 
	    }
	
	    current_c_for_stmt = get_enclosing_c_for_stmt(current_c_for_stmt);
	    if(current_c_for_stmt)
	       nest_annotes->push_back(to<BrickAnnote>(current_c_for_stmt->lookup_annote_by_name("c_for_info")));
	}

	delete nest_annotes;
    	return Walker::Truncate;
}

String get_var(Expression *index_expr){

	if(is_a<IntConstant>(index_expr))
	   return "";
	else if(is_a<LoadVariableExpression>(index_expr))
	   return (to<LoadVariableExpression>(index_expr))->get_source()->get_name();
	else if(is_a<UnaryExpression>(index_expr)){
	   UnaryExpression *u_expr = to<UnaryExpression>(index_expr);
	   if(u_expr->get_opcode() == String("negate"))
	      return get_var(u_expr->get_source());
	}else if(is_a<BinaryExpression>(index_expr)){
	   BinaryExpression *b_expr = to<BinaryExpression>(index_expr);
	   Expression *src1 = b_expr->get_source1();
	   Expression *src2 = b_expr->get_source2();
	   if(b_expr->get_opcode() == String("multiply")){
	      if(get_a(src1) == 0)
		 return get_var(src2);
	      if(get_a(src2) == 0)
		 return get_var(src1);
	   }
	   if(get_a(src1) == 0)
	      return get_var(src2);
	   else return get_var(src1); 
	}
	return "";
}

int get_a(Expression *index_expr){

	if(is_a<IntConstant>(index_expr))
	   return 0;
	else if(is_a<LoadVariableExpression>(index_expr))
	   return 1;
	else if(is_a<UnaryExpression>(index_expr)){
	   UnaryExpression *u_expr = to<UnaryExpression>(index_expr);
	   if(u_expr->get_opcode() == String("negate"))
	      return -1 * get_a(u_expr->get_source());
	}else if(is_a<BinaryExpression>(index_expr)){
	   BinaryExpression *b_expr = to<BinaryExpression>(index_expr);
	   Expression *src1 = b_expr->get_source1();
	   Expression *src2 = b_expr->get_source2();
	   if(b_expr->get_opcode() == String("multiply")){
	      if(get_a(src1) == 0)
		 return get_c(src1) * get_a(src2);
	      if(get_a(src2) == 0)
		 return get_c(src2) * get_a(src1);
	   }
	   int sign = b_expr->get_opcode() == String("subtract") ? -1 : 1;
	   if(get_a(src1) == 0)
	      return sign * get_a(src2);
	   else return get_a(src1); 
	}
	return 0;
}

int get_c(Expression *index_expr){

	if(is_a<IntConstant>(index_expr))
	   return (to<IntConstant>(index_expr))->get_value().c_int();
	else if(is_a<LoadVariableExpression>(index_expr))
	   return 0;
	else if(is_a<UnaryExpression>(index_expr)){
	   UnaryExpression *u_expr = to<UnaryExpression>(index_expr);
	   if(u_expr->get_opcode() == String("negate"))
	      return -1 * get_c(u_expr->get_source());
	}else if(is_a<BinaryExpression>(index_expr)){
	   BinaryExpression *b_expr = to<BinaryExpression>(index_expr);
	   Expression *src1 = b_expr->get_source1();
	   Expression *src2 = b_expr->get_source2();
	   int sign = b_expr->get_opcode() == String("subtract") ? -1 : 1;
	   if(get_c(src1) == 0)
	      return sign * get_c(src2);
	   else return get_c(src1); 
	}
	return 0;
}

Walker::ApplyStatus pp_array_reference_expression_walker::operator () (SuifObject *x) {

	SuifEnv *env = get_env();
	ArrayReferenceExpression *array_ref_expr = to<ArrayReferenceExpression>(x);

	if(array_ref_expr->lookup_annote_by_name("array_ref_info"))
	   delete (array_ref_expr->remove_annote_by_name("array_ref_info"));
	   
	BrickAnnote *array_ref_info_annote = create_brick_annote(env, "array_ref_info"); 
	// Removed by Jason to make my life a little easier
	// Reinstated to get systolic array working again...
	array_ref_expr->append_annote(array_ref_info_annote);

	// Also, create an annote to help me later...
	BrickAnnote* array_ref_info_annote2 = create_brick_annote(env,
								  "array_ref_info") ;

	// Added by Jason to make my life a little easier
	//dynamic_cast<SymbolAddressExpression*>(array_ref_expr->get_base_array_address())->get_addressed_symbol()->append_annote(array_ref_info_annote) ;

	// Modified by Jason on 3/24/2009
	//  The previous version did not work with multidimensional arrays
	ArrayReferenceExpression* currentCast = array_ref_expr ;
	while(dynamic_cast<SymbolAddressExpression*>(currentCast->get_base_array_address()) == NULL)
	  {
	    currentCast = dynamic_cast<ArrayReferenceExpression*>(currentCast->get_base_array_address()) ;
	    assert(currentCast != NULL) ;
	  }
	dynamic_cast<SymbolAddressExpression*>(currentCast->get_base_array_address())->get_addressed_symbol()->append_annote(array_ref_info_annote2) ;	

	// Unfortunately, this causes problems in the strip annotes phase
	//  so I need to change that as well.

	ArrayInfo *array_ref_info = new ArrayInfo();

	Expression *base_array_address = array_ref_expr; 
	Expression *index = NULL;

	int dimension = 0;

	do{

	   index = (to<ArrayReferenceExpression>(base_array_address))->get_index();
	   String var = get_var(index);
	   int a = get_a(index);
	   int c = get_c(index);

	   array_ref_info->push_front_index_var_name(var);
	   array_ref_info->push_front_a(a);
	   array_ref_info->push_front_c(c);
	   dimension++;

	   base_array_address = (to<ArrayReferenceExpression>(base_array_address))->get_base_array_address();

	}while(is_a<ArrayReferenceExpression>(base_array_address));
	
        SymbolAddressExpression *array_sym_expr = to<SymbolAddressExpression>(base_array_address);
        VariableSymbol *array_sym = to<VariableSymbol>(array_sym_expr->get_addressed_symbol());

	array_ref_info->set_array_symbol_name(array_sym->get_name());
	array_ref_info->set_dimension(dimension);

	array_ref_info_annote->append_brick(create_suif_object_brick(env, array_ref_info));

	// Added for my benefit
	array_ref_info_annote2->append_brick(create_suif_object_brick(env, array_ref_info)) ;

    	return Walker::Truncate;
}

