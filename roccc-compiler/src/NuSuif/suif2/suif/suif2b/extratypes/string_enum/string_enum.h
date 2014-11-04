/* file "string_enum.h" */


/*
       Copyright (c) 1997 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>


#ifndef STY_STRING_ENUM_H
#define STY_STRING_ENUM_H

#include <common/lstring.h>

#include <common/machine_dependent.h>


/*
      This is the interface to a mechanism for creating extensible
      string-to-enumered-type mappings for sty, the first-level main
      library of the SUIF system.
*/


extern void register_enum_string_initializer(void (*initializer)(void));
extern int *make_map_table(int *base_table, char **addition_table,
			   size_t base_max, size_t base_enum_count, 
			   size_t *new_max,
			   size_t *new_enum_count);

class base_enum_string_map_data
  {
public:
    static int *data;
    static size_t max;
    static size_t enum_count;

    static int **get_data_pointer(void)
      {
        return &data;
      }
    static size_t *get_max_pointer(void)
      {
        return &max;
      }
    static size_t *get_enum_count_pointer(void)
      {
        return &enum_count;
      }
    static void initialize_data(void)  { }
  };

template <class base_map_t, class addition_map_t> class enum_string_map_data
  {
public:
    enum_string_map_data(void)
      { register_enum_string_initializer(&initialize_data); }
    ~enum_string_map_data(void)  { delete[] *(get_data_pointer()); }


    static int **get_data_pointer(void)
      {
        static int *data;

        return &data;
      }
    static size_t *get_max_pointer(void)
      {
        static size_t max;

        return &max;
      }
    static size_t *get_enum_count_pointer(void)
      {
        static size_t enum_count;

        return &enum_count;
      }
    static void initialize_data(void)
      {
        base_map_t::initialize_data();
        *(get_data_pointer()) = make_map_table(
                *(base_map_t::get_data_pointer()), addition_map_t::table,
                *(base_map_t::get_max_pointer()),
                *(base_map_t::get_enum_count_pointer()), get_max_pointer(),
                get_enum_count_pointer());
      }
  };

template <class map_data_t> class enum_string_map
  {
public:
    enum_string_map(void)  { }
    ~enum_string_map(void)  { }

    static int map(LString the_string)
      {
        static map_data_t data;
	size_t ord = the_string.get_ordinal();
	if (ord > *(data.get_max_pointer())) return(0);
	return((*(data.get_data_pointer()))[ord]);
      }
  };


#endif /* STY_STRING_ENUM_H */
