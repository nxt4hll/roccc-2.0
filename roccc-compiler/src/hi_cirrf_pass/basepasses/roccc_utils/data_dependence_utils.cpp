// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include <common/suif_copyright.h>
#include <common/suif_list.h>

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
#include "list_utils.h"
#include "data_dependence_utils.h"

/**************************** Declerations ************************************/


/************************** Implementations ***********************************/

// 
int get_step(BrickAnnote* loop_info, String var_name){

    for(int i = 1; i < loop_info->get_brick_count(); i = i+4){
	
        String loop_index_var_name = (to<StringBrick>(loop_info->get_brick(i)))->get_value();
	
	if(loop_index_var_name == var_name)
	   return (to<IntegerBrick>(loop_info->get_brick(i+3)))->get_value().c_int();
    }

    return -1;
}

// computes if a write to an array is dependent on the read on the same array enclosed in a common nest of loops
bool is_dependent(ArrayInfo* w_array_info, ArrayInfo* r_array_info, BrickAnnote* enclosing_loop_info){

    for(int i = 0; i < w_array_info->get_dimension(); i++){
	
        String array_index_var_name = w_array_info->get_index_var_name(i);
        int w_index_constant = w_array_info->get_c(i);
        int r_index_constant = r_array_info->get_c(i);
	
	if(w_index_constant-r_index_constant/get_step(enclosing_loop_info, array_index_var_name) > 0)
	   return 1;
    }

    return 0;
}

// computes the dependence between a write to an array and the read on the same array enclosed in a common nest of loops
void compute_dependence(BrickAnnote* w_array_info, BrickAnnote* r_array_info, BrickAnnote* enclosing_loop_info){



}

bool is_a_feedback_pair(ArrayInfo* store_ref_info, ArrayInfo* load_ref_info, String loop_counter_name, int loop_step_size){
                                                                          
        int bc = 0;
        bool found = 0;
        while(bc < store_ref_info->get_dimension()){
  
           if(store_ref_info->get_index_var_name(bc) != load_ref_info->get_index_var_name(bc))
              return 0;

           String index_var_name = store_ref_info->get_index_var_name(bc);
  
           if(store_ref_info->get_a(bc) != load_ref_info->get_a(bc))
              return 0;
  
           if(index_var_name == loop_counter_name){
              found = 1;
              if(store_ref_info->get_c(bc) - load_ref_info->get_c(bc) !=  loop_step_size)
                 return 0;
           }else if(store_ref_info->get_c(bc) != load_ref_info->get_c(bc))
              return 0;
  
           bc++;
        }

        return found;
}

bool is_pair_dependant(ArrayInfo* store_ref_info, ArrayInfo* load_ref_info, BrickAnnote* loop_nest_info) {
              
        String loop_counter_name;
        int U, L, S;
           
        int bc_loop_nest = loop_nest_info->get_brick_count() - 1;
        while(bc_loop_nest > 0){
        
           loop_counter_name = (to<StringBrick>(loop_nest_info->get_brick(bc_loop_nest-3)))->get_value();
           L = (to<IntegerBrick>(loop_nest_info->get_brick(bc_loop_nest-2)))->get_value().c_int();
           U = (to<IntegerBrick>(loop_nest_info->get_brick(bc_loop_nest-1)))->get_value().c_int();
           S = (to<IntegerBrick>(loop_nest_info->get_brick(bc_loop_nest)))->get_value().c_int();
        
           int bc = 0;
           bool index_found = 0;
           while(!index_found && bc < store_ref_info->get_dimension()){
              if(store_ref_info->get_index_var_name(bc) == loop_counter_name)
                 index_found = 1;
              else bc ++;
           }
         
           if(store_ref_info->get_index_var_name(bc) != load_ref_info->get_index_var_name(bc))
              return 1;
 
           if(store_ref_info->get_a(bc) != load_ref_info->get_a(bc))
              return 1;

           int Cw_minus_Cr = store_ref_info->get_c(bc) - load_ref_info->get_c(bc);
 
           if(Cw_minus_Cr <= U-L && Cw_minus_Cr > 0 && Cw_minus_Cr % S == 0)
              return 1;
           else return 0;
    
           bc_loop_nest -= 4;
        }
       
        return 0;
}
           

