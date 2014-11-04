#ifndef __BASIC_CONSTANTS__H_
#define __BASIC_CONSTANTS__H_
#include "common/system_specific.h"
#include "common/lstring.h"

#ifdef MSVC
#ifdef BASICNODES_EXPORTS
#define extern extern DLLEXPORT
#else
#define extern extern DLLIMPORT
#endif
#endif  // MSVC

extern LString k_add;
extern LString k_subtract;
extern LString k_multiply;
extern LString k_divide;
extern LString k_remainder;
extern LString k_bitwise_and;
extern LString k_bitwise_or;
extern LString k_bitwise_nand;
extern LString k_bitwise_nor;
extern LString k_bitwise_xor;
extern LString k_left_shift;
extern LString k_right_shift;
extern LString k_rotate;
extern LString k_is_equal_to;
extern LString k_is_not_equal_to;
extern LString k_is_less_than;
extern LString k_is_less_than_or_equal_to;
extern LString k_is_greater_than;
extern LString k_is_greater_than_or_equal_to;
extern LString k_logical_and;
extern LString k_logical_or;
extern LString k_maximum;
extern LString k_minimum;
extern LString k_negate;
extern LString k_invert;
extern LString k_absolute_value;
extern LString k_bitwise_not;
extern LString k_logical_not;
extern LString k_convert;
extern LString k_treat_as;
extern LString k_copy;
extern LString k_select;
extern LString k_array_reference;
extern LString k_field_access;
extern LString k_extract_fields;
extern LString k_set_fields;
extern LString k_extract_elements;
extern LString k_set_elements;
extern LString k_bit_size_of;
extern LString k_byte_size_of;
extern LString k_bit_alignment_of;
extern LString k_byte_alignment_of;
extern LString k_bit_offset_of;
extern LString k_byte_offset_of;
extern LString k_va_start;
extern LString k_va_start_old;
extern LString k_va_arg;
extern LString k_va_end;
extern LString k_sc_and;
extern LString k_sc_or;
extern LString k_sc_select;
extern LString k_load;
extern LString k_constant;
extern LString k_volatile;
// Used for Branches
extern LString k_branch_if_false;
extern LString k_branch_if_true;
#undef extern
#endif
