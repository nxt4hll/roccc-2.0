// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.   

#include "common/system_specific.h"
#include <common/suif_copyright.h>
#include "common/suif_map.h"

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
#include "roccc_utils/list_utils.h"
#include "roccc_utils/IR_utils.h"
#include "roccc_utils/symbol_utils.h"
#include "roccc_utils/data_dependence_utils.h"
#include "roccc_utils/warning_utils.h"
#include "roccc_extra_types/array_info.h"
#include "renaming_pass.h"

// THIS PASS RENAMES THE ARRAYS WHERE DEPENDENCES BTW STORE AND LOAD OPS ALLOW.

/*

For 1D:

Textually preeceding loads:
 ... = A[i']
A[i] = ... 

i>i' --> true_dep/no_ren/seq_ex
i=i' --> false_dep/ren/par_ex
i<i' --> false_dep/ren/par_ex

Textually succeeding loads:
A[i] = ... 
 ... = A[i']

i>i' --> true_dep/no_ren/seq_ex
i=i' --> eliminated by array_raw_elim pass
i<i' --> false_dep/ren/par_ex

For 2D:

Textually preeceding loads:
 ... = A[i'][j']
A[i][j] = ... 

i>i' --> true_dep/no_ren/seq_ex
i=i' j>j' --> true_dep/no_ren/seq_ex
i=i' j=j' --> false_dep/ren/par_ex
i=i' j<j' --> false_dep/ren/par_ex
i<i' --> false_dep/ren/par_ex

Textually succeeding loads:
A[i][j] = ... 
 ... = A[i'][j']

i>i' --> true_dep/no_ren/seq_ex
i=i' j>j' --> true_dep/no_ren/seq_ex
i=i' j=j' --> eliminated by array_raw_elim pass
i=i' j<j' --> false_dep/ren/par_ex
i<i' --> false_dep/ren/par_ex

*/

// JUMP INDIRECT IS NOT SUPPORTED
// MULTI WAY BRANCH (SWITCH/CASE) IS NOT SUPPORTED

/**************************** Declarations ************************************/

class rp_store_statement1_walker: public SelectiveWalker {
public:
  rp_store_statement1_walker(SuifEnv *env)
    :SelectiveWalker(env, StoreStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

class rp_store_statement2_walker: public SelectiveWalker {
public:
  rp_store_statement2_walker(SuifEnv *env)
    :SelectiveWalker(env, StoreStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

list<String> *array_names;
list<String> *not_to_renames;
suif_map<String, VariableSymbol*>* rename_map; 

/**************************** Implementations ************************************/
RenamingPass::RenamingPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "RenamingPass") {}

void RenamingPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  OutputInformation("Scalar renaming pass begins") ;
  if (proc_def)
  {
    array_names = new list<String>;
    not_to_renames = new list<String>;
    rename_map = new suif_map<String, VariableSymbol*>;

    rp_store_statement1_walker walker1(get_suif_env());
    proc_def->walk(walker1);
    if(!is_items_in_list(array_names, not_to_renames))
    {
      rp_store_statement2_walker walker2(get_suif_env());
      proc_def->walk(walker2);
    }
  }
  OutputInformation("Scalar renaming pass ends") ;
}

Walker::ApplyStatus rp_store_statement1_walker::operator() (SuifObject *x) {
	StoreStatement *store_stmt = to<StoreStatement>(x);

	ArrayReferenceExpression *dest_ref_expr = to<ArrayReferenceExpression>(store_stmt->get_destination_address()); 
	BrickAnnote *store_array_ref_info_annote = to<BrickAnnote>(dest_ref_expr->lookup_annote_by_name("array_ref_info"));
        SuifObjectBrick *sob = to<SuifObjectBrick>(store_array_ref_info_annote->get_brick(0));
        ArrayInfo *store_array_ref_info = (ArrayInfo*)(sob->get_object());

	String dest_array_name = store_array_ref_info->get_array_symbol_name();

	if(is_in_list(dest_array_name, not_to_renames))
	   return Walker::Continue;

	if(!is_in_list(dest_array_name, array_names))
	   array_names->push_back(dest_array_name);

	CForStatement *enclosing_c_for_loop = get_enclosing_c_for_stmt(store_stmt);
	BrickAnnote *c_for_info = to<BrickAnnote>(enclosing_c_for_loop->lookup_annote_by_name("c_for_info"));

	BrickAnnote *reached_uses = to<BrickAnnote>(store_stmt->lookup_annote_by_name("reached_uses"));
	for(Iter<SuifBrick*> iter = reached_uses->get_brick_iterator();
	    iter.is_valid(); iter.next()){
	    
	    SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current()); 
	    LoadExpression *use_expr = to<LoadExpression>(sob->get_object());
 	    ArrayReferenceExpression *use_ref_expr = to<ArrayReferenceExpression>(use_expr->get_source_address()); 
	    BrickAnnote *use_array_ref_info_annote = to<BrickAnnote>(use_ref_expr->lookup_annote_by_name("array_ref_info"));
            sob = to<SuifObjectBrick>(use_array_ref_info_annote->get_brick(0));
            ArrayInfo *use_array_ref_info = (ArrayInfo*)(sob->get_object());

	    if(is_dependent(store_array_ref_info, use_array_ref_info, c_for_info)){
	       not_to_renames->push_back(dest_array_name);
	       return Walker::Continue;
	    }
	}

    	return Walker::Continue;
}

Walker::ApplyStatus rp_store_statement2_walker::operator() (SuifObject *x) {
	StoreStatement *store_stmt = to<StoreStatement>(x);

	ArrayReferenceExpression *dest_ref_expr = to<ArrayReferenceExpression>(store_stmt->get_destination_address()); 
	BrickAnnote *array_ref_info = to<BrickAnnote>(dest_ref_expr->lookup_annote_by_name("array_ref_info"));
	String dest_array_name = to<StringBrick>(array_ref_info->get_brick(1))->get_value();

	if(is_in_list(dest_array_name, not_to_renames))
	   return Walker::Continue;

        Expression *base_array_address = dest_ref_expr;

        while(is_a<ArrayReferenceExpression>(base_array_address))
           base_array_address = (to<ArrayReferenceExpression>(base_array_address))->get_base_array_address();

        SymbolAddressExpression *array_sym_expr = to<SymbolAddressExpression>(base_array_address);
        VariableSymbol *array_sym = to<VariableSymbol>(array_sym_expr->get_addressed_symbol());

        if(rename_map->find(array_sym->get_name()) == rename_map->end()){
 	   VariableSymbol *replacement = to<VariableSymbol>(array_sym->deep_clone());
	   rename_symbol(replacement, "");
	   SymbolTable *sym_table = get_procedure_definition(store_stmt)->get_symbol_table();
	   sym_table->add_symbol(replacement);
	   name_variable(replacement, array_sym->get_name());
	   array_sym_expr->set_addressed_symbol(replacement);
	}else array_sym_expr->set_addressed_symbol(rename_map->lookup(array_sym));

    	return Walker::Continue;
}

