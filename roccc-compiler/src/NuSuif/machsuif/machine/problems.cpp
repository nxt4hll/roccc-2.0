/* file "machine/problems.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College
    Copyright (c) 1995, 1996, 1997 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "machine/problems.h"
#endif

#include <stdio.h>
#include <stdarg.h>
#include <machine/problems.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

/***** Debugging Help *****/

int debuglvl = 0;

/* debug() -- print out a diagnostic message */
#ifndef NDEBUG
void debug(const int lvl, const char *msg ...)
{
    if(lvl > debuglvl) return;
    va_list ap;
    va_start(ap,msg);
    fprintf(stderr, "DEBUG (%d): ", lvl);
    vfprintf(stderr, msg, ap);
    va_end(ap);
    putc('\n', stderr);
    fflush(stderr);
}
#endif

/* warn() -- print out a warning message */
#ifndef NDEBUG
void warn(const char *msg ...)
{
    va_list ap;
    va_start(ap,msg);
    fprintf(stderr, "WARNING: ");
    vfprintf(stderr, msg, ap);
    va_end(ap);
    putc('\n', stderr);
    fflush(stderr);
}
#endif

/***** Assertion Related Routines *****/

char *__assertion_file_name = NULL;
int __assertion_line_num = 0;
char *__assertion_module_name = NULL;

void
_internal_assertion_failure(void)
{
    _internal_assertion_failure_extra(NULL, __assertion_file_name, __assertion_line_num, __assertion_module_name);
     
/*
    fprintf(stderr, "\n");
    fprintf(stderr, "        **** ASSERTION FAILURE ****\n");
    fprintf(stderr, "    file `%s', line %d", __assertion_file_name,
            __assertion_line_num);
    if (__assertion_module_name != NULL)
        fprintf(stderr, " (module %s)", __assertion_module_name);
    fprintf(stderr, "\n");
    abort();
    */
}

void
_internal_assertion_failure(const char *format, va_list ap)
{
    
    _internal_assertion_failure_extra(NULL, __assertion_file_name, __assertion_line_num, __assertion_module_name, 
            format, ap); 

    /*
    fprintf(stderr, "\n");
    fprintf(stderr, "        **** ASSERTION FAILURE ****\n");
    fprintf(stderr, "    file `%s', line %d", __assertion_file_name,
            __assertion_line_num);
    if (__assertion_module_name != NULL)
        fprintf(stderr, " (module %s)", __assertion_module_name);
    fprintf(stderr, "\n");
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
    abort();
    */
}

inline void
_print_bar(void)
{
    fprintf(stderr, "-----------------------------------------------------\n"); 

}

void
_print_assertion_header(Instr* instr, const char *filename, const int linenum, const char* module)
{
    fprintf(stderr, "\n");
    fprintf(stderr, "        **** ASSERTION FAILURE ****\n");
    if( instr != NULL ) {
        _print_bar();
        fprintf(stderr, "INSTR: ");
        fprint(stderr, instr);
    }
    _print_bar();
    fprintf(stderr, "INFO : File(%s) at line(%d)", filename, linenum);
    //fprintf(stderr, "    file `%s', line %d", filename, linenum);
    if (module != NULL)
        fprintf(stderr, " (module %s)", module);
    fprintf(stderr, "\n");
    _print_bar();
    fflush(stderr);
 
}

void
_internal_assertion_failure_extra(Instr* instr, const char *filename, const int linenum, const char *module)
{
    _print_assertion_header(instr, filename, linenum, module); 
    abort();
}



void
_internal_assertion_failure_extra(Instr *instr, const char *filename, const int linenum, const char *module, 
        const char *format, va_list ap)
{
    _print_assertion_header(instr, filename, linenum, module); 
    fprintf(stderr, "MSG  : ");
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
    _print_bar();
    fflush(stderr);
    abort();
}

// ##############################
#include <list>
struct __file_with_linenum_struct {
    const std::string filename;
    const int linenum;
    __file_with_linenum_struct(const std::string &_filename, const int _linenum)
        : filename(_filename), linenum(_linenum) { }
};

std::list<__file_with_linenum_struct> __file_call_assertion_stack;

void 
_pop_file_call_stack()
{
    assert( __file_call_assertion_stack.size() > 0 );
    __file_call_assertion_stack.pop_front();

}

void 
_push_onto_file_call_stack(const char *s, const int linenum)
{
    __file_call_assertion_stack.push_front( __file_with_linenum_struct(s, linenum) );

}

const char* 
_get_top_filename_from_stack()
{
    assert( __file_call_assertion_stack.size() > 0 );
    return __file_call_assertion_stack.front().filename.c_str();
}

const int 
_get_top_linenum_from_stack() 
{
    assert( __file_call_assertion_stack.size() > 0 );
    return __file_call_assertion_stack.front().linenum;
}
// ##############################
