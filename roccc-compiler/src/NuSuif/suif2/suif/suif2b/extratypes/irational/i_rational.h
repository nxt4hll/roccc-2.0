/* file "i_rational.h" */

/*  Copyright (c) 1995 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */

#include <common/suif_copyright.h>

/*
 *  This is the header file for the i_integer class for
 *  extended-precision integer arithmetic.
 */

#ifndef IRATIONAL_H
#define IRATIONAL_H

#include <common/i_integer.h>

class IRational
  {
private:
    IInteger the_numerator;
    IInteger the_denominator;

    void reduce(void);

public:
    IRational(void) : the_denominator(1) { }
    IRational(signed char initial_value)  { set_integer(initial_value); }
    IRational(unsigned char initial_value)  { set_integer(initial_value); }
    IRational(short initial_value)  { set_integer(initial_value); }
    IRational(unsigned short initial_value)  { set_integer(initial_value); }
    IRational(int initial_value)  { set_integer(initial_value); }
    IRational(unsigned int initial_value)  { set_integer(initial_value); }
    IRational(long initial_value)  { set_integer(initial_value); }
    IRational(unsigned long initial_value)  { set_integer(initial_value); }
    IRational(const IInteger &initial_value)  { set_integer(initial_value); }
    IRational(const IInteger &initial_numerator,
               const IInteger &initial_denominator) :
            the_numerator(initial_numerator),
            the_denominator(initial_denominator)
      { reduce(); }
    IRational(const IRational &initial_value)
      { set_rational(initial_value); }
    IRational(const char *initial_string, int base = 10)
      { read(initial_string, base); }

    IRational numerator(void) const  { return the_numerator; }
    IRational denominator(void) const  { return the_denominator; }

    bool is_undetermined(void) const
      { return the_numerator.is_undetermined(); }
    bool is_signless_infinity(void) const
      { return the_numerator.is_signless_infinity(); }
    bool is_finite(void) const  { return the_numerator.is_finite(); }
    bool is_negative(void) const  { return the_numerator.is_negative(); }

    bool is_c_char(void) const
      { return (is_integer() && the_numerator.is_c_char()); }
    bool is_c_unsigned_char(void) const
      { return (is_integer() && the_numerator.is_c_unsigned_char()); }
    bool is_c_signed_char(void) const
      { return (is_integer() && the_numerator.is_c_signed_char()); }
    bool is_c_short(void) const
      { return (is_integer() && the_numerator.is_c_short()); }
    bool is_c_unsigned_short(void) const
      { return (is_integer() && the_numerator.is_c_unsigned_short()); }
    bool is_c_int(void) const
      { return (is_integer() && the_numerator.is_c_int()); }
    bool is_c_unsigned_int(void) const
      { return (is_integer() && the_numerator.is_c_unsigned_int()); }
    bool is_c_long(void) const
      { return (is_integer() && the_numerator.is_c_long()); }
    bool is_c_unsigned_long(void) const
      { return (is_integer() && the_numerator.is_c_unsigned_long()); }

    char c_char(void) const
      { assert(is_c_char()); return the_numerator.c_char(); }
    unsigned char c_unsigned_char(void) const
      { assert(is_c_unsigned_char()); return the_numerator.c_unsigned_char(); }
    signed char c_signed_char(void) const
      { assert(is_c_signed_char()); return the_numerator.c_signed_char(); }
    short c_short(void) const
      { assert(is_c_short()); return the_numerator.c_short(); }
    unsigned short c_unsigned_short(void) const
      {
        assert(is_c_unsigned_short());
        return the_numerator.c_unsigned_short();
      }
    int c_int(void) const
      { assert(is_c_int()); return the_numerator.c_int(); }
    unsigned int c_unsigned_int(void) const
      { assert(is_c_unsigned_int()); return the_numerator.c_unsigned_int(); }
    long c_long(void) const
      { assert(is_c_long()); return the_numerator.c_long(); }
    unsigned long c_unsigned_long(void) const
      { assert(is_c_unsigned_long()); return the_numerator.c_unsigned_long(); }

    void set_c_char(char new_value)  { set_integer(new_value); }
    void set_c_unsigned_char(unsigned char new_value)
      { set_integer(new_value); }
    void set_c_signed_char(signed char new_value)  { set_integer(new_value); }
    void set_c_short(short new_value)  { set_integer(new_value); }
    void set_c_unsigned_short(unsigned short new_value)
      { set_integer(new_value); }
    void set_c_int(int new_value)  { set_integer(new_value); }
    void set_c_unsigned_int(unsigned int new_value)
      { set_integer(new_value); }
    void set_c_long(long new_value)  { set_integer(new_value); }
    void set_c_unsigned_long(unsigned long new_value)
      { set_integer(new_value); }
    void set_integer(const IInteger &new_value)
      { the_numerator = new_value; the_denominator = 1; }
    void set_rational(const IRational &new_value)
      {
        the_numerator = new_value.the_numerator;
        the_denominator = new_value.the_denominator;
      }

    IInteger written_length(int base = 10) const;
    void write(char *location, int base = 10) const;
    //    void write(ostream *out, int base = 10) const;

    void read(const char *location, int base = 10);
    void read(istream *in, int base = 10);

    String to_String(int base = 10) const;

    void print(ostream *fp, int base = 10) const;

    bool is_integer(void) const  { return (the_denominator == 1); }
    IInteger floor(void) const;
    IInteger ceiling(void) const;
    IInteger round(void) const;

    IRational &operator=(const IRational &other)
      { set_rational(other); return *this; }

    bool operator==(const IRational &other) const
      {
        return ((the_numerator == other.the_numerator) &&
                (the_denominator == other.the_denominator));
      }
    bool operator!=(const IRational &other) const
      { return !(*this == other); }
    bool operator<(const IRational &other) const
      {
        return ((the_numerator * other.the_denominator) <
                (other.the_numerator * the_denominator));
      }
    bool operator>(const IRational &other) const
      { return (other < *this); }
    bool operator<=(const IRational &other) const
      { return !(*this > other); }
    bool operator>=(const IRational &other) const
      { return !(*this < other); }

    IRational operator+(const IRational &other) const
      {
        return IRational((the_numerator * other.the_denominator) +
                          (other.the_numerator * the_denominator),
                          the_denominator * other.the_denominator);
      }
    IRational operator-(const IRational &other) const
      { return *this + (-other); }
    IRational operator*(const IRational &other) const
      {
        return IRational(the_numerator * other.the_numerator,
                          the_denominator * other.the_denominator);
      }
    IRational operator/(const IRational &other) const
      {
        return IRational(the_numerator * other.the_denominator,
                          the_denominator * other.the_numerator);
      }
    IRational operator%(const IRational &) const  { return 0; }

    bool operator!(void) const  { return (*this == 0); }
    bool operator&&(const IRational &other) const
      { return ((*this != 0) && (other != 0)); }
    bool operator||(const IRational &other) const
      { return ((*this != 0) || (other != 0)); }

    /* unary */
    IRational operator+(void) const  { return *this; }
    IRational operator-(void) const
      { return IRational(-the_numerator, the_denominator); }

    IRational &operator+=(const IRational &other)
      { return (*this = *this + other); }
    IRational &operator-=(const IRational &other)
      { return (*this = *this - other); }
    IRational &operator*=(const IRational &other)
      { return (*this = *this * other); }
    IRational &operator/=(const IRational &other)
      { return (*this = *this / other); }
    IRational &operator%=(const IRational &other)
      { return (*this = *this % other); }

    /* prefix */
    IRational &operator++(void)  { *this += 1; return *this; }
    IRational &operator--(void)  { *this -= 1; return *this; }

    /* postfix */
    IRational operator++(int)
      { IRational result = *this; *this += 1; return result; }
    IRational operator--(int)
      { IRational result = *this; *this -= 1; return result; }
  };

#endif /* IRATIONAL_H */
