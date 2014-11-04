/* file "machine/problems.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef MACHINE_PROBLEMS_H
#define MACHINE_PROBLEMS_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "machine/problems.h"
#endif

#include <list>
#include <machine/substrate.h>
#include <machine/instr.h>

extern int debuglvl;		/* user defined diagnostic print level */

#ifndef NDEBUG
extern void debug(const int, const char * ...);
#else
#define debug if (false)
#endif

#ifndef NDEBUG
extern void warn(const char * ...);
#else
#define warn if (false)
#endif

#ifndef NDEBUG
#define if_debug(lvl) if (debuglvl >= lvl)
#else
#define if_debug(lvl) if (false)
#endif


extern char *__assertion_file_name;
extern int __assertion_line_num;
extern char *__assertion_module_name;

extern void _internal_assertion_failure(void);
extern void _internal_assertion_failure(const char *format, va_list ap);
extern void _internal_assertion_failure_extra(Instr* instr, const char *filename, const int linenum, const char *module, const char *format, va_list ap);
extern void _internal_assertion_failure_extra(Instr* instr, const char *filename, const int linenum, const char *module);

extern void _pop_file_call_stack();
extern const char* _get_top_filename_from_stack();
extern const int _get_top_linenum_from_stack() ;
extern void _push_onto_file_call_stack(const char *s, const int linenum);

/*
struct __file_with_linenum_struct;
extern std::list< __file_with_linenum_struct > __file_call_assertion_stack;
*/

inline void __do_assertion(bool assertion)
  {  
      if (!(assertion))  
          //  _internal_assertion_failure(); 
          _internal_assertion_failure_extra(NULL, _get_top_filename_from_stack(), _get_top_linenum_from_stack(), __assertion_module_name); 
      _pop_file_call_stack();
  }


inline void __do_assertion(bool assertion, const char *format, ...)
  {
    if (!(assertion))
      {
        va_list ap;
        va_start(ap, format);
        //_internal_assertion_failure(format, ap);
        _internal_assertion_failure_extra(NULL, _get_top_filename_from_stack(), _get_top_linenum_from_stack(), __assertion_module_name, 
                format, ap);
        va_end(ap);
      }
    _pop_file_call_stack();
  }


inline void __do_assertion_with_instr(bool assertion, Instr* instr)
{  
    if (!(assertion))  
    { 
        _internal_assertion_failure_extra(instr, _get_top_filename_from_stack(), _get_top_linenum_from_stack(), __assertion_module_name); 
    } 
    _pop_file_call_stack();
}

inline void __do_assertion_with_instr(bool assertion, Instr *instr, const char *format, ...)
{
    if (!(assertion))
    {
        va_list ap;
        va_start(ap, format);
        _internal_assertion_failure_extra(instr, _get_top_filename_from_stack(), _get_top_linenum_from_stack(), __assertion_module_name, 
                format, ap);
        va_end(ap);
    }
    _pop_file_call_stack();
}




#ifndef NDEBUG

#ifndef _MODULE_
#define _MODULE_ NULL
#endif

/*
#define claim  __assertion_file_name = __FILE__, \
               __assertion_line_num = __LINE__, \
               __assertion_module_name = _MODULE_, \
               __do_assertion

#define instr_claim  __assertion_file_name = __FILE__, \
               __assertion_line_num = __LINE__, \
               __assertion_module_name = _MODULE_, \
               __do_assertion_with_instr
               */

#define claim  _push_onto_file_call_stack(__FILE__, __LINE__), \
               __assertion_file_name = __FILE__, \
               __assertion_line_num = __LINE__, \
               __assertion_module_name = _MODULE_, \
               __do_assertion


#define instr_claim  _push_onto_file_call_stack(__FILE__, __LINE__), \
                __assertion_file_name = __FILE__, \
               __assertion_line_num = __LINE__, \
               __assertion_module_name = _MODULE_, \
               __do_assertion_with_instr
 
#else

#define claim if (false)
#define instr_claim if (false)

#endif

#endif /* MACHINE_PROBLEMS_H */
