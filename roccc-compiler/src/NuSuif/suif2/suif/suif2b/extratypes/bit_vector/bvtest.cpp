#include "bit_vector.h"

class SuifEnv;
extern "C" void init_bit_vector(SuifEnv *);

int main() {
  init_bit_vector(0);
  {for (unsigned i = 0; i < 1600; i++) {
    BitVector bv;
    bv.set_bit(i, true);
    BitVectorIter iter(&bv);
    if (!iter.is_valid()) {
      fprintf(stderr, "ERROR: %d\n", i);
      continue;
    }
    size_t val = iter.current();
    if (val != i) {
      fprintf(stderr, "ERROR: %d\n", i);
      continue;
    }
    if (bv.count() != 1) {
      fprintf(stderr, "ERROR: %d\n", i);
      continue;
    }
    iter.next();
    if (iter.is_valid()) {
      fprintf(stderr, "ERROR: %d\n", i);
      continue;
    }
  }}
  
  {for (unsigned i = 24; i < 160; i++) {
    BitVector bv;
    bv.set_bit(i, true);
    for (unsigned j = 32; j < 160; j++) {
      if (j > 0 && j != i+1) bv.set_bit(j-1, false);
      if (i == j) continue;
      bv.set_bit(j, true);
      BitVectorIter iter(&bv);
      if (!iter.is_valid()) {
	fprintf(stderr, "ERROR: %d %d\n", i, j);
	continue;
      }

      size_t val = iter.current();
      if (bv.count() != 2) {
	fprintf(stderr, "ERROR: %d %d\n", i, j);
	continue;
      }
      iter.next();
      size_t val2 = iter.current();
      if (!iter.is_valid()) {
	fprintf(stderr, "ERROR: %d %d\n", i, j);
	continue;
      }
      if (!((val == i && val2 == j) ||
	    (val == j && val2 == i))) {
	fprintf(stderr, "ERROR: %d %d\n", i, j);
	continue;
      }
      iter.next();
      if (iter.is_valid()) {
	fprintf(stderr, "ERROR: %d %d\n", i, j);
	continue;
      }
    }
  }}


  return(0);
}
    
