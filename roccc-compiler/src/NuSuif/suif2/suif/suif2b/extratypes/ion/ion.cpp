/* file "ion.cc" */


/*
       Copyright (c) 1995, 1996, 1997 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

//#include <suif_copyright.h>


/*
      This is the implementation of classes for I/O for sty, the
      first-level main library of the SUIF system.
*/


#include "ion.h"
#include <common/i_integer.h>
#include <suifkernel/suifkernel_messages.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>

extern "C" void init_ion(SuifEnv *) {
  
}

/*
 *  This file defines three base ``ion'' methods, put_s(), printf(),
 *  and vprintf(), in terms of the put_c() method.  This is to allow
 *  ion sub-classes to be implemented as easily as possible, requiring
 *  the user to only provide a single method.  The methods defined
 *  here are virtual and can be replaced by more efficient methods
 *  specific to the sub-class.  These methods use ANSI C library
 *  functions.
 */


/*
 *  We define some macros here for sizes of buffers that are
 *  guaranteed big enough for certain ASCII representations of values
 *  of different sizes.  Taking the sizeof() applied to the type and
 *  multiplying by CHAR_BIT gives the number of bits.  We know zero is
 *  representable by each integral type, and for each representable
 *  value ``x'', all the values from zero to ``x'' are representable.
 *  Hence the standard guarantees that the magnitude of every
 *  representable value is less than two to the power of the number of
 *  bits.  Further, the sizes are all positive numbers of bits.  So
 *  the binary representation of the number in ASCII fits in
 *  ``sizeof(<type>) * CHAR_BIT'' characters.  Any other radix will
 *  have no more than that many characters.  Add a character for sign
 *  and one for the trailing NULL character adds 2; we throw in one
 *  more just to have some slop space.
 */
#define ASCII_TYPE_SPACE(x) ((sizeof(x) * CHAR_BIT) + 3)

/*
 *  This macro should be the size of a buffer big enough to hold any
 *  result of the ``%p'' printf() directive.
 *
 *  NOTE: This is *not* guaranteed to work on all platforms, though it
 *  will likely work on most.  The ANSI C standard gives no way to get
 *  a bound on how long the result of the ``%p'' printf(), since the
 *  form of the result is left entirely implementation defined.
 */
#define ASCII_POINTER_SPACE  200

/*
 *  This macro contains the default amount of space for a string_ion
 *  to allocate in its initial buffer.
 */
#define DEFAULT_INIT_STRING_ION_SPACE 80


ion *stdout_ion = new file_ion(stdout);
ion *stderr_ion = new file_ion(stderr);


//void ion::virtual_function_table_hack(void)
//  {
    /* This function is a hack to help compilers that generate virtual
     * function tables and bodies for inline methods only for
     * translation units that define the first non-pure virtual method
     * for a given class.  It is important that this function come
     * before any other virtual functions in the declaration of this
     * class! */
//  }

void ion::put_s(const char *s)
  {
    const char *follow = s;
    while (*follow != 0)
      {
        this->put_c(*follow);
        ++follow;
      }
  }

void ion::printf(const char *format, ...)
  {
    va_list args;

    va_start(args, format);
    this->vprintf(format, args);
    va_end(args);
  }

void ion::vprintf(const char *format, va_list arg)
  {
    IInteger count = 0;
    const char *follow = format;
    while (*follow != 0)
      {
        if (*follow != '%')
          {
            this->put_c(*follow);
            ++count;
            ++follow;
            continue;
          }
        ++follow;
        if (*follow == '%')
          {
            this->put_c('%');
            ++count;
            ++follow;
            continue;
          }
        bool left_justify = false;
        bool always_sign = false;
        bool space_sign = false;
        bool zero_pad = false;
        bool alternate_output_form = false;
        while (true)
          {
            switch (*follow)
              {
                case '-':
                    left_justify = true;
                    ++follow;
                    continue;
                case '+':
                    always_sign = true;
                    ++follow;
                    continue;
                case ' ':
                    space_sign = true;
                    ++follow;
                    continue;
                case '0':
                    zero_pad = true;
                    ++follow;
                    continue;
                case '#':
                    alternate_output_form = true;
                    ++follow;
                    continue;
                default:
                    break;
              }
            break;
          }
        if (zero_pad && left_justify)
            zero_pad = false;
        IInteger field_width = 0;
        bool width_specified = false;
        if (*follow == '*')
          {
            width_specified = true;
            ++follow;
            field_width = va_arg(arg, int);
          }
        else
          {
            while (isdigit(*follow))
              {
                field_width = (field_width * 10) + (*follow - '0');
                width_specified = true;
                ++follow;
              }
          }
        IInteger precision = 0;
        bool precision_specified = false;
        if (*follow == '.')
          {
            precision_specified = true;
            ++follow;
            if (*follow == '*')
              {
                ++follow;
                precision = va_arg(arg, int);
              }
            else
              {
                while (isdigit(*follow))
                  {
                    precision = (precision * 10) + (*follow - '0');
                    ++follow;
                  }
              }
          }
        const char *modifier = NULL;
        switch (*follow)
          {
            case 'h':
            case 'l':
            case 'L':
                modifier = follow;
                ++follow;
                break;
            default:
                break;
          }
        IInteger item_space;
        char *item = NULL;
        switch (*follow)
          {
            case 'd':
            case 'i':
            case 'o':
            case 'u':
            case 'x':
            case 'X':
            case 'f':
            case 'e':
            case 'E':
            case 'g':
            case 'G':
            case 'p':
              {
                /* We just use the native sprintf() for most
                 * specifiers, though we have to be careful that
                 * buffers are the right sizes. */

                char *format_buf =
                        new char[(field_width.written_length() +
                                 precision.written_length() +
                                 9).c_unsigned_long()];
                sprintf(format_buf, "%%%s%s%s%s%s", (left_justify ? "-" : ""),
                        (always_sign ? "+" : ""), (space_sign ? " " : ""),
                        (zero_pad ? "0" : ""),
                        (alternate_output_form ? "#" : ""));
                char *next_position = &(format_buf[strlen(format_buf)]);
                if (width_specified)
                  {
                    field_width.write(next_position);
                    next_position = &(next_position[strlen(next_position)]);
                  }
                if (precision_specified)
                  {
                    *next_position = '.';
                    ++next_position;
                    precision.write(next_position);
                    next_position = &(next_position[strlen(next_position)]);
                  }
                if (modifier != NULL)
                  {
                    *next_position = *modifier;
                    ++next_position;
                  }
                *next_position = *follow;
                ++next_position;
                *next_position = 0;
                IInteger max_length = precision + field_width + 50;
                suif_assert_message(max_length.is_c_unsigned_long(),
			    ("out of memory address space"));
                char *buffer = new char[max_length.c_unsigned_long()];
                switch (*follow)
                  {
                    case 'd':
                    case 'i':
                        if ((modifier != NULL) && (*modifier == 'h'))
			  //sprintf(buffer, format_buf, va_arg(arg, short));
			  sprintf(buffer, format_buf, va_arg(arg, int));//jul modif!
                        else if ((modifier != NULL) && (*modifier == 'l'))
                            sprintf(buffer, format_buf, va_arg(arg, long));
                        else
                            sprintf(buffer, format_buf, va_arg(arg, int));
                        break;
                    case 'o':
                    case 'u':
                    case 'x':
                    case 'X':
                        if ((modifier != NULL) && (*modifier == 'h'))
                          {
                            //sprintf(buffer, format_buf,
                            //        va_arg(arg, unsigned short));
			    sprintf(buffer, format_buf,
				    va_arg(arg, unsigned int));//jul modif!
			  }
                        else if ((modifier != NULL) && (*modifier == 'l'))
                          {
                            sprintf(buffer, format_buf,
                                    va_arg(arg, unsigned long));
                          }
                        else
                          {
                            sprintf(buffer, format_buf,
                                    va_arg(arg, unsigned int));
                          }
                        break;
                    case 'f':
                    case 'e':
                    case 'E':
                    case 'g':
                    case 'G':
                        if ((modifier != NULL) && (*modifier == 'L'))
                          {
                            sprintf(buffer, format_buf,
                                    va_arg(arg, long double));
                          }
                        else
                          {
                            sprintf(buffer, format_buf, va_arg(arg, double));
                          }
                        break;
                    case 'p':
                        sprintf(buffer, format_buf, va_arg(arg, void *));
                        break;
                    default:
                        suif_assert(false);
                  }
                ++follow;
                this->put_s(buffer);
                count += strlen(buffer);
                delete[] format_buf;
                delete[] buffer;
                continue;
              }
            case 'c':
              {
                static char char_buf[2] = { 0, 0};

                /* The always_sign, space_sign, zero_pad, and
                 * alternate_output_form flags are ignored.  The
                 * precision is also ignored. */
                char_buf[0] = (unsigned char)va_arg(arg, int);
                item = char_buf;
                item_space = 1;
                suif_assert_message(modifier == NULL,
				    ("modifier `%c' used with `%c' conversion "
                             "specifier",
                             ((modifier == NULL) ? (char)0 : *modifier),
                             *follow));
                break;
              }
            case 's':
              {
                /* The always_sign, space_sign, zero_pad, and
                 * alternate_output_form flags are ignored. */
                item = va_arg(arg, char *);
                item_space = strlen(item);
                suif_assert_message(modifier == NULL,
				    ("modifier `%c' used with `%c' conversion "
                             "specifier", ((modifier == NULL) ? 0 : *modifier),
                             *follow));
                if (precision_specified && (precision < item_space))
                  {
                    IInteger padding = 0;
                    if (width_specified)
                      {
                        padding = field_width - precision;
                        if (padding < 0)
                            padding = 0;
                      }
                    count += precision + padding;
                    if (!left_justify)
                      {
                        while (padding > 0)
                          {
                            this->put_c(' ');
                            --padding;
                          }
                      }
                    while (precision > 0)
                      {
                        this->put_c(item[0]);
                        ++item;
                        --precision;
                      }
                    while (padding > 0)
                      {
                        this->put_c(' ');
                        --padding;
                      }
                    ++follow;
                    continue;
                  }
                break;
              }
            case 'n':
              {
                int *ptr = va_arg(arg, int *);
                if (count > INT_MAX)
                  {
		    fprintf(stderr, "overflow in count for `n' conversion specfier");
		    //                    warn("overflow in count for `n' conversion specfier");
                    *ptr = INT_MAX;
                  }
                else
                  {
                    *ptr = count.c_int();
                  }
                ++follow;
                continue;
              }
            default:
	      suif_assert_message(false, ("unknown conversion specifier 0x%02x\n",
					  *follow));
				  
	      /*
                err_buf->put_s("unknown conversion specifier ");
                if (isprint(*follow))
                    err_buf->printf("`%c'", *follow);
                else
                    err_buf->printf("0x%03d", *follow);
                wrap_error();
                suif_assert(false);
	      */
          }
        ++follow;

        if (width_specified)
          {
            IInteger padding = field_width - item_space;
            if (padding > 0)
              {
                count += item_space + padding;
                if (left_justify)
                  {
                    this->put_s(item);
                    while (padding > 0)
                      {
                        this->put_c(' ');
                        --padding;
                      }
                  }
                else
                  {
                    while (padding > 0)
                      {
                        this->put_c(' ');
                        --padding;
                      }
                    this->put_s(item);
                  }
                continue;
              }
          }
        count += item_space;
        this->put_s(item);
      }
  }


//void string_ion::virtual_function_table_hack(void)
//  {
    /* This function is a hack to help compilers that generate virtual
     * function tables and bodies for inline methods only for
     * translation units that define the first non-pure virtual method
     * for a given class.  It is important that this function come
     * before any other virtual functions in the declaration of this
     * class! */
//  }

string_ion::string_ion(void) :
  _buffer(new char[_space]),
  _space(DEFAULT_INIT_STRING_ION_SPACE),
  _position(0)
  {
    //    _space = DEFAULT_INIT_STRING_ION_SPACE;
    //    _position = 0;
    //    _buffer = new char[_space];
    _buffer[0] = 0;
  }

string_ion::string_ion(size_t init_space) :
  _buffer(new char[init_space]),
  _space(init_space),
  _position(0)
  {
    //    _space = init_space;
    //    _position = 0;
    //    _buffer = new char[_space];
    _buffer[0] = 0;
  }

string_ion::string_ion(const char *init_string) :
  _buffer(0),
  _space(0),
  _position(0)
  {
    _position = strlen(init_string);
    _space = _position + 5;
    _buffer = new char[_space];
    memcpy(_buffer, init_string, _position + 1);
  }

string_ion::string_ion(const string_ion &other) :
  _buffer(0),
  _space(0),
  _position(0)

  {
    const char *init_string = other.show();
    _position = strlen(init_string);
    _space = _position + 5;
    _buffer = new char[_space];
    memcpy(_buffer, init_string, _position + 1);
  }

void string_ion::put_c(int c)
  {
    if (_position == _space - 1)
        make_space(_space);
    _buffer[_position] = c;
    ++_position;
    _buffer[_position] = 0;
  }

void string_ion::put_s(const char *s)
  {
    size_t addition_length = strlen(s);
    if (_position + addition_length >= _space)
        make_space(_space + addition_length);
    memcpy(&_buffer[_position], s, addition_length + 1);
    _position += addition_length;
  }

void string_ion::make_space(size_t additional_space)
  {
    char *new_buffer = new char[_space + additional_space];
    memcpy(new_buffer, _buffer, _position + 1);
    delete[] _buffer;
    _buffer = new_buffer;
    _space += additional_space;
  }


//void fixed_string_ion::virtual_function_table_hack(void)
//  {
    /* This function is a hack to help compilers that generate virtual
     * function tables and bodies for inline methods only for
     * translation units that define the first non-pure virtual method
     * for a given class.  It is important that this function come
     * before any other virtual functions in the declaration of this
     * class! */
//  }

void fixed_string_ion::put_c(int c)
  {
    *_position = c;
    ++_position;
    *_position = 0;
  }

void fixed_string_ion::put_s(const char *s)
  {
    size_t addition_length = strlen(s);
    memcpy(_position, s, addition_length + 1);
    _position += addition_length;
  }

void fixed_string_ion::printf(const char *format, ...)
  {
    va_list args;

    va_start(args, format);
    int result = vsprintf(_position, format, args);
    va_end(args);
    suif_assert_message(result >= 0, ("vsprintf() error"));
    _position += result;
  }

void fixed_string_ion::vprintf(const char *format, va_list arg)
  {
    int result = vsprintf(_position, format, arg);
    suif_assert_message(result >= 0, ("vsprintf() error"));
    _position += result;
  }


//void file_ion::virtual_function_table_hack(void)
//  {
    /* This function is a hack to help compilers that generate virtual
     * function tables and bodies for inline methods only for
     * translation units that define the first non-pure virtual method
     * for a given class.  It is important that this function come
     * before any other virtual functions in the declaration of this
     * class! */
//  }

void file_ion::put_c(int c)
  {
    fputc(c, _fp);
  }

void file_ion::put_s(const char *s)
  {
    fputs(s, _fp);
  }

void file_ion::printf(const char *format, ...)
  {
    va_list args;

    va_start(args, format);
    vfprintf(_fp, format, args);
    va_end(args);
  }

void file_ion::vprintf(const char *format, va_list arg)
  {
    vfprintf(_fp, format, arg);
  }
