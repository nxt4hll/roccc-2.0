/* file "ion.h" */


/*
       Copyright (c) 1995, 1996, 1997 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

//#include <suif_copyright.h>


#ifndef STY_ION_H
#define STY_ION_H


/*
      This is the definition of classes for I/O for sty, the
      first-level main library of the SUIF system.
*/

#include <stdio.h>
#include <stdarg.h>
//#include <ostream.h>

/*
 *  The name ``ion'' means ``I/O Noter''.  The ion class is the
 *  virtual class giving the interface and string_ion,
 *  fixed_string_ion, and file_ion are instantiatable.
 *
 *  These classes are for writing text.  No null characters may be
 *  written with them.  They are intended for human-readable output,
 *  not bit-level or random-access I/O.
 *
 *  Note that while the only required virtual method for an
 *  instantiatable sub-class of ion is the put_c() method, in many
 *  cases it's better to override other methods, such as put_s(), for
 *  the sake of efficiency.
 */

class ion //: public ostream
  {
    //private:
    //    virtual void virtual_function_table_hack(void);

protected:
    ion(void)  { }

public:
    virtual ~ion(void)  { }

    virtual void put_c(int c) = 0;
    virtual void put_s(const char *s);
    virtual void printf(const char *format, ...);
    virtual void vprintf(const char *format, va_list arg);
  };


/*
 *  The string_ion class is an implementation of the ion interface
 *  that saves whatever is written to it to a dynamically allocated
 *  buffer that it maintains internally.  The show() method accesses
 *  the string.  Note that show() points to an internal buffer and is
 *  only guaranteed to remain valid until something else is written to
 *  the string_ion or the string_ion itself is deallocated.  The
 *  location show() returns should never be written by a user of the
 *  string_ion class.
 */

class string_ion : public ion
  {
private:
    char *_buffer;
    size_t _space;
    size_t _position;

    //    virtual void virtual_function_table_hack(void);

    void make_space(size_t additional_space);
    string_ion &operator=(const string_ion &other);

public:
    string_ion(void);
    string_ion(size_t init_space);
    string_ion(const char *init_string);
    string_ion(const string_ion &other);
    ~string_ion(void)  { delete[] _buffer; }

    char *show(void) const  { return _buffer; }

    void clear(void)  { _position = 0; }

    void put_c(int c);
    void put_s(const char *s);
  };


/*
 *  The fixed_string_ion class is an implementation of the ion
 *  interface that writes to a user-specified character buffer.  Note
 *  that the buffer it is given is fixed and the fixed_string_ion does
 *  no checking for overflow.  It should only be used when the user is
 *  sure that overflow of the buffer is not possible.
 */

class fixed_string_ion : public ion
  {
private:
    char *_position;

    //    virtual void virtual_function_table_hack(void);
    fixed_string_ion(const fixed_string_ion &);
    fixed_string_ion &operator=(const fixed_string_ion &);
public:
    fixed_string_ion(char *init_position) : _position(init_position) {}
    ~fixed_string_ion(void)  { }

    void put_c(int c);
    void put_s(const char *s);
    void printf(const char *format, ...);
    void vprintf(const char *format, va_list arg);
  };


/*
 *  The file_ion class is an implementation of the ion interface that
 *  writes to a file through the standard C ``FILE *'' method.
 */

class file_ion : public ion
  {
private:
    FILE *_fp;

    //    virtual void virtual_function_table_hack(void);
    file_ion(const file_ion&);
    file_ion &operator=(const file_ion&);

public:
    file_ion(FILE *init_fp) : _fp(init_fp) {}
    ~file_ion(void)  { }

    FILE *fp(void)  { return _fp; }

    void put_c(int c);
    void put_s(const char *s);
    void printf(const char *format, ...);
    void vprintf(const char *format, va_list arg);
  };

extern ion *stdout_ion;
extern ion *stderr_ion;

class SuifEnv;

extern "C" void init_ion(SuifEnv *);

#endif /* STY_ION_H */
