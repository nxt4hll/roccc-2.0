// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef ARRAY_INFO_H
#define ARRAY_INFO_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

class ArrayInfo : public SuifObject {
public:
  ArrayInfo();
  ~ArrayInfo();

  void set_array_symbol_name(String a_name);
  void set_dimension(int d);
  void push_front_index_var_name(String iv_name);
  void push_front_a(int a);
  void push_front_c(int c);

  String get_array_symbol_name();
  int get_dimension();
  String get_index_var_name(int i);
  int get_a(int i);
  int get_c(int i);

private:
  String array_symbol_name;
  int dimension;
  suif_vector<String>* index_expr_var_names;
  suif_vector<int>* index_expr_coeff_a;
  suif_vector<int>* index_expr_const_c;

};

#endif

