// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "array_info.h"

ArrayInfo::ArrayInfo(){

  array_symbol_name = "";
  dimension = 0;
  index_expr_var_names = new suif_vector<String>;
  index_expr_coeff_a = new suif_vector<int>;
  index_expr_const_c = new suif_vector<int>;

}

ArrayInfo::~ArrayInfo(){

  delete index_expr_var_names;
  delete index_expr_coeff_a;
  delete index_expr_const_c;

}

void ArrayInfo::set_array_symbol_name(String a_name){

  array_symbol_name = a_name;

}

void ArrayInfo::set_dimension(int d){

  dimension = d;

}

void ArrayInfo::push_front_index_var_name(String iv_name){

  index_expr_var_names->insert(0, iv_name);

}

void ArrayInfo::push_front_a(int a){

  index_expr_coeff_a->insert(0, a);

}

void ArrayInfo::push_front_c(int c){

  index_expr_const_c->insert(0, c);

}

String ArrayInfo::get_array_symbol_name(){

  return array_symbol_name;

}

int ArrayInfo::get_dimension(){

  return dimension;

}

String ArrayInfo::get_index_var_name(int i){

  return index_expr_var_names->at(i);

}

int ArrayInfo::get_a(int i){
  
  return index_expr_coeff_a->at(i);

}

int ArrayInfo::get_c(int i){

  return index_expr_const_c->at(i);

}
