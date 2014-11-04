/* file "i_rational.cc" */

/*  Copyright (c) 1995,1999 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */

#include <common/suif_copyright.h>
#include <ostream.h>
#include <ctype.h>
/*
 *  This file implements extended-precision rational arithmetic
 *  routines for the i_irational class.
 */

#include "i_rational.h"


/*----------------------------------------------------------------------*
    Begin Private Function Declarations
 *----------------------------------------------------------------------*/

static IRational read_digits_with_decimal_point(const char **location,
                                                 int base);

/*----------------------------------------------------------------------*
    End Private Function Declarations
 *----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*
    Begin Public Method Implementations
 *----------------------------------------------------------------------*/

void IRational::reduce(void)
  {
    if (the_numerator.is_finite() && the_denominator.is_finite())
      {
        if (the_denominator == 0)
          {
            the_numerator = IInteger::i_signless_inf();
            the_denominator = 1;
          }
        else
          {
            bool result_negative =
                    (the_numerator.is_negative() !=
                     the_numerator.is_negative());
            if (the_numerator.is_negative())
                the_numerator = -the_numerator;
            if (the_denominator.is_negative())
                the_denominator = -the_denominator;
            IInteger the_gcd = IInteger::ii_gcd(the_numerator, the_denominator);
            the_numerator /= the_gcd;
            the_denominator /= the_gcd;
            if (result_negative)
                the_numerator = -the_numerator;
          }
      }
    else
      {
        if (the_numerator.is_undetermined() ||
            the_denominator.is_undetermined())
          {
            the_denominator = 1;
          }
        else if (the_numerator.is_finite())
          {
            the_numerator = 0;
            the_denominator = 1;
          }
        else if (the_denominator.is_finite())
          {
            if (the_denominator == 0)
              {
                the_numerator = IInteger();
                the_denominator = 1;
              }
            else if (the_denominator.is_negative())
              {
                the_numerator = -the_numerator;
                the_denominator = 1;
              }
            else
              {
                the_denominator = 1;
              }
          }
        else
          {
            the_numerator = IInteger();
            the_denominator = 1;
          }
      }
  }

IInteger IRational::written_length(int base) const
  {
    if (the_denominator == 1)
      {
        return the_numerator.written_length(base);
      }
    else
      {
        return the_numerator.written_length(base) +
               the_denominator.written_length(base) + 1;
      }
  }

void IRational::write(char *location, int base) const
  {
    the_numerator.write(location, base);
    if (the_denominator != 1)
      {
        unsigned long denominator_length = strlen(location);
        location[denominator_length] = '/';
        the_denominator.write(&(location[denominator_length + 1]), base);
      }
  }

void IRational::read(const char *location, int base)
  {
    const char *follow_string = location;

    IRational new_numerator =
            read_digits_with_decimal_point(&follow_string, base);

    while (isspace(*follow_string))
        ++follow_string;

    if (*follow_string == '/')
      {
        ++follow_string;
        IRational new_denominator =
                read_digits_with_decimal_point(&follow_string, base);
        operator=(new_numerator / new_denominator);
      }
    else
      {
        operator=(new_numerator);
      }
  }

void IRational::print(ostream *fp, int base) const
  {
    the_numerator.print(fp, base);
    if (the_denominator != 1)
      {
        fp->put('/');//, fp);
        the_denominator.print(fp, base);
      }
  }

IInteger IRational::floor(void) const
  {
    IInteger div = the_numerator / the_denominator;
    IInteger mod = the_numerator % the_denominator;
    if (mod < 0)
        return div - 1;
    else
        return div;
  }

IInteger IRational::ceiling(void) const
  {
    return -((-(*this)).floor());
  }

IInteger IRational::round(void) const
  {
    return (*this + IRational(1, 2)).floor();
  }

/*----------------------------------------------------------------------*
    End Public Method Implementations
 *----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*
    Begin Public Function Implementations
 *----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*
    End Public Function Implementations
 *----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*
    Begin Private Function Implementations
 *----------------------------------------------------------------------*/


static IRational read_digits_with_decimal_point(const char **location,
                                                 int base)
  {
    IInteger pre_decimal_point(*location, base);

    const char *follow_string = *location;

    while (isspace(*follow_string))
        ++follow_string;

    if (*follow_string == '-')
        ++follow_string;

    while (isspace(*follow_string))
        ++follow_string;

    while (*follow_string != 0)
      {
        int this_digit = *follow_string;
        if ((this_digit >= '0') && (this_digit <= '9'))
            this_digit -= '0';
        else if ((this_digit >= 'a') && (this_digit <= 'z'))
            this_digit -= ('a' - 10);
        else if ((this_digit >= 'A') && (this_digit <= 'Z'))
            this_digit -= ('A' - 10);
        else
            break;

        if (this_digit >= base)
            break;

        ++follow_string;
      }

    if (*follow_string != '.')
      {
        *location = follow_string;
        return pre_decimal_point;
      }

    const char *decimal_place = follow_string;

    ++follow_string;

    while (*follow_string != 0)
      {
        int this_digit = *follow_string;
        if ((this_digit >= '0') && (this_digit <= '9'))
            this_digit -= '0';
        else if ((this_digit >= 'a') && (this_digit <= 'z'))
            this_digit -= ('a' - 10);
        else if ((this_digit >= 'A') && (this_digit <= 'Z'))
            this_digit -= ('A' - 10);
        else
            break;

        if (this_digit >= base)
            break;

        ++follow_string;
      }

    *location = follow_string;

    if (follow_string == decimal_place + 1)
        return pre_decimal_point;

    IInteger post_decimal_point(decimal_place + 1, base);

    IInteger denominator = 1;
    while (follow_string > decimal_place + 1)
      {
        denominator *= base;
        --follow_string;
      }

    return IRational(pre_decimal_point) +
           IRational(post_decimal_point, denominator);
  }

/*----------------------------------------------------------------------*
    End Private Function Implementations
 *----------------------------------------------------------------------*/
