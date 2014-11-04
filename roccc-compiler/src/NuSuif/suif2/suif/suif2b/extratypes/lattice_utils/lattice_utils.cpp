#include <suifkernel/suif_env.h>
extern "C" void init_lattice_utils(SuifEnv *s) {
  s->require_DLL("bit_vector");
}
