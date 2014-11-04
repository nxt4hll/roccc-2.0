/* file "string_enum.cc" */


/*
       Copyright (c) 1997 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>


/*
      This is the implementation of a mechanism for creating
      extensible string-to-enumerated-type mappings for sty, the
      first-level main library of the SUIF system.
*/


#include "common/machine_dependent.h"
#include "common/suif_list.h"
#include "string_enum.h"


#ifdef DEAL_WITH_SOLARIS_BRAIN_DAMAGE
/*
 *  The Sun ProCompiler for Solaris doesn't seem to be able to figure
 *  out how to instantiate the zero() template for pointers from
 *  sty/zero.h when elem_t, the type referenced by the pointer, is a
 *  function type instead of an object type.  So we have an explicit
 *  zero() functions of the appropriate type here as a work-around for
 *  this bug in that compiler.
 */

inline void (*zero(void (**)(void)))(void)  { return NULL; }
#endif /* DEAL_WITH_SOLARIS_BRAIN_DAMAGE */

//static dlist_tos<void (*)(void)> initializer_list;
static list<void (*)(void)> initializer_list;
bool ready_to_initialize = false;


int *base_enum_string_map_data::data = NULL;
size_t base_enum_string_map_data::max = 0;
size_t base_enum_string_map_data::enum_count = 1;


extern void register_enum_string_initializer(void (*initializer)(void))
  {
    if (ready_to_initialize)
        (*initializer)();
    else
        initializer_list.push_back(initializer);
  }

extern int *make_map_table(int *base_table, char **addition_table,
			   size_t base_max, size_t base_enum_count, 
			   size_t *new_max,
			   size_t *new_enum_count)
  {
    size_t result_max = base_max;
    int result_enum_count = base_enum_count;
      {
        for (size_t table_index = 0; addition_table[table_index] != NULL;
             ++table_index)
          {
            size_t this_id =
                    LString(addition_table[table_index]).get_ordinal();
            if (this_id > result_max)
                result_max = this_id;
            ++result_enum_count;
          }
      }
    int *result = new int[result_max + 1];
    size_t id;
    if (base_table != NULL)
      {
        for (id = 0; id <= base_max; ++id)
            result[id] = base_table[id];
      }
    for (; id < result_max; ++id)
        result[id] = 0;

      {
        for (size_t table_index = 0; addition_table[table_index] != NULL;
             ++table_index)
          {
            size_t this_id =
                    LString(addition_table[table_index]).get_ordinal();
            result[this_id] = table_index + base_enum_count;
          }
      }
    *new_max = result_max;
    *new_enum_count = result_enum_count;
    return result;
  }

class SuifEnv;

extern "C" void init_string_enum(SuifEnv *) {
  static bool done = false;
  if (done) return;
  done = true;

  while (!initializer_list.empty())
    {
      void (*this_initializer)(void) = initializer_list.front();
      initializer_list.pop_front();
      (*this_initializer)();
    }
  ready_to_initialize = true;
}
