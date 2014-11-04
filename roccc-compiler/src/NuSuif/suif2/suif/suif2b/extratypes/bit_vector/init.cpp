class SuifEnv;
#include "bit_vector.h"

extern "C" void init_bit_vector(SuifEnv *suif_env) {
  BitVector::do_initialization();
}
