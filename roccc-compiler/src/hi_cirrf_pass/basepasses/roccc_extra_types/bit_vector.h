// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef BIT_VECTOR_H
#define BIT_VECTOR_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "common/suif_map.h"
#include "bit_vector_map.h"

class BitVector : public SuifObject {
public:
  BitVector(BitVectorMap *bvm);
  ~BitVector();

  void reset();
  BitVector* clone();

  void mark(SuifObject *loc);
  void mark(int loc);

  void unmark(SuifObject *loc);
  void unmark(int loc);
  
  bool is_marked(SuifObject *loc);
  bool is_marked(int loc);

  void copy(BitVector *a_bv);
  void intersect(BitVector *a_bv);
  void union_(BitVector *a_bv);

  BitVector* subtract(BitVector *a_bv);
  void subtract(BitVector *a_bv, BitVector *b_bv);  
  void subtract_n_overwrite(BitVector *a_bv);

  suif_vector<int>* get_bit_vector_value();
  BitVectorMap* get_bit_vector_map();

  void set_bit_vector_value(suif_vector<int>* bv_val);
  void set_bit_vector_map(BitVectorMap *bvm);

  String to_string();

private:
  suif_vector<int>* bv_value;
  BitVectorMap* bv_map; 
};

#endif

