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
#include "string_utils.h"

using namespace std;

/**************************** Declerations ************************************/

#define ROCCC_MACRO_COUNT 24
String roccc_macro_names[ROCCC_MACRO_COUNT] = {"ROCCC_min2", "ROCCC_min3", "ROCCC_max2",
                                "ROCCC_max3", "ROCCC_bitcmb", "ROCCC_boollut", "ROCCC_boolsel",
                                "ROCCC_sin", "ROCCC_cos", "ROCCC_allzero", "ROCCC_abs",
                                "ROCCC_bit_array_lookup", "ROCCC_lut_lookup", "ROCCC_lut_write",
                                "ROCCC_gipcore", "ROCCC_sat_add", "ROCCC_sat_clamp",
                                "ROCCC_create_state_table", "ROCCC_lookup_in_state_table",
                                "ROCCC_create_lookup_table", "ROCCC_lookup_in_table",
				"ROCCC_cam", "ROCCC_mux", "ROCCC_convert"};

/************************** Implementations ***********************************/

int find(String s, String ss, int pos){

	String temp = s.Right(s.size() - pos);

	return temp.find(ss);

}

bool is_a_roccc_macro_name(String proc_name){
   
   for(int i = 0; i<ROCCC_MACRO_COUNT; i++)
       if(roccc_macro_names[i] == proc_name)
          return 1;
  
   if(proc_name.starts_with("ROCCC_mux") || proc_name.starts_with("ROCCC_cam"))
      return 1;

   return 0;
}
