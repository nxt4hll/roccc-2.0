// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef DATA_DEPENDENCE_UTILS_H
#define DATA_DEPENDENCE_UTILS_H

#include "roccc_extra_types/array_info.h"

bool is_dependent(ArrayInfo* w_array_info, ArrayInfo* r_array_info, BrickAnnote* enclosing_loop_info);

bool is_pair_dependant(ArrayInfo* store_ref_info, ArrayInfo* load_ref_info, BrickAnnote* loop_nest_info);
bool is_a_feedback_pair(ArrayInfo* store_ref_info, ArrayInfo* load_ref_info, String loop_counter_name, int loop_step_size);

#endif

