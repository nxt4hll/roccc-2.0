/**	@file
 *	An infinite length integer class
 */


/*
       Copyright (c) 1995, 1996, 1997 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include "MString.h"
#include <iostream> /* jul modif */

#ifndef COMMON_I_INTEGER_H
#define COMMON_I_INTEGER_H

/**	\class IInteger i_integer.h common/i_integer.h
 *	provides an infinite size integer class
 *	as well as undetermined and signless and signed infinity values
*/

#include <stdio.h>
#include <limits.h>




class StringInteger;
class IInteger
  {
private:

    enum IIntegerTag
    {
      IIT_C_INT, IIT_STRING_INT, IIT_POS_INFINITY, IIT_NEG_INFINITY,
      IIT_SIGNLESS_INFINITY, IIT_UNDETERMINED
    };

    IIntegerTag _tag;
    union
      {
        long long_val;
        StringInteger *si_val; // eventually, this will be a vector<char>
      } _value;

    IInteger(StringInteger *initial_value);

public:
    /** 	create an IInteger with indeterminate value*/
    IInteger(void);
    IInteger(signed char initial_value);
    IInteger(unsigned char initial_value);
    IInteger(short initial_value);
    IInteger(unsigned short initial_value);
    IInteger(int initial_value);
    IInteger(unsigned int initial_value);
    IInteger(long initial_value);
    /**		Create an IInteger from various types */
    IInteger(unsigned long initial_value);
#ifdef LONGLONG
    IInteger(LONGLONG initial_value);
    /**		If LONGLONG is defined, create an IInteger from long longs */
    IInteger(unsigned LONGLONG initial_value);
#endif
    /**	Copy constructor */
    IInteger(const IInteger &initial_value);
    /**	Convert an ascii string to a value
     *  @param initial_string string to convert
     *  @param base base to use. Must be in range 1 to 36
     *  \par NOTE:
     *	   No prefix (such as 0x for hex) should be present.
     *	   \par As well as numeric values, the values ?? (indeterminate
     *     +Inf -Inf and Inf are recognized
     */
    IInteger(const char *initial_string, int base = 10);
    ~IInteger();

    /**	Is the iinteger indeterminate?*/
    bool is_undetermined(void) const;
    /**	Is the iinteger unsigned infinity?
     *	NOTE:	There are three infinities, Inf (signless)
     *	+Inf and -Inf (signed)
     */
    bool is_signless_infinity(void) const;
    /**	Is the iinteger finite?*/
    bool is_finite(void) const;
    /**	Is the iinteger negative?*/
    bool is_negative(void) const;

    /**	Is the integer a string int */
    bool is_c_string_int(void) const;
    /** Will the value fit in a char */
    bool is_c_char(void) const;
    /** Will the value fit in an unsigned char */
    bool is_c_unsigned_char(void) const;
    /** Will the value fit in a signed char */
    bool is_c_signed_char(void) const;
    /** Will the value fit in a short */
    bool is_c_short(void) const;
    /** Will the value fit in an unsigned short */
    bool is_c_unsigned_short(void) const;
    /** Will the value fit in an int */
    bool is_c_int(void) const;
    /** Will the value fit in an unsigned int */
    bool is_c_unsigned_int(void) const;
    /** Will the value fit in a long */
    bool is_c_long(void) const;
    /** Will the value fit in an unsigned long*/
    bool is_c_unsigned_long(void) const;
#ifdef LONGLONG
    /** Will the value fit in a long long (if LONGLONG defined)*/
    bool is_c_long_long(void) const;
    /** Will the value fit in an unsigned long long (if LONGLONG defined)*/
    bool is_c_unsigned_long_long(void) const;
#endif
    /** Will the value fit in a size_t */
    bool is_c_size_t(void) const;


    /**	Retrieve value as a string - requires is_c_string
     *  \see {to_String} */
    char* c_string_int(void) const;
    /** Retrieve the value as a char - requires is_c_char */
    char c_char(void) const;
    /** Retrieve the value as an unsigned char - requires is_unsigned_c_char*/
    unsigned char c_unsigned_char(void) const;
    /** Retrieve the value as a signed char - requires is_signed_c_char*/
    signed char c_signed_char(void) const;
    /** Retrieve the value as a short - requires is_c_short*/
    short c_short(void) const;
    /** Retrieve the value as an unsigned short - requires is_unsigned_c_short*/
    unsigned short c_unsigned_short(void) const;
    /** Retrieve the value as an int - requires is_c_int*/
    int c_int(void) const;
    /** Retrieve the value as an unsigned int - requires is_c_unsigned_int*/
    unsigned int c_unsigned_int(void) const;
    /** Retrieve the value as a long - requires is_c_long */
    long c_long(void) const;
    /** Retrieve the value as an unsigned long - requires is_c_unsigned_long*/
    unsigned long c_unsigned_long(void) const;
#ifdef LONGLONG
    /** Retrieve the value as a long long - requires is_c_long_long */
    long c_long_long(void) const;
    /** Retrieve the value as an unsigned long - requires is_c_unsigned_long_long*/
    unsigned long c_unsigned_long_long(void) const;
#endif
    /** Retrieve the value as a size_t - requires is_c_size_t */
    size_t c_size_t(void) const;
    /** Retrieve the value as a double. Any non-indeterminate non-infinite
     *  value can be returned as a double (unless it is enormously large)
     *  but precision will be lost if it larger than about 10^16*/
    double c_double(void) const;

    void set_c_char(char new_value);
    void set_c_unsigned_char(unsigned char new_value);
    void set_c_signed_char(signed char new_value);
    void set_c_short(short new_value);
    void set_c_unsigned_short(unsigned short new_value);
    
    void set_c_int(int new_value);
    void set_c_unsigned_int(unsigned int new_value);
    void set_c_long(long new_value);
    void set_c_unsigned_long(unsigned long new_value);
#ifdef LONGLONG
    void set_c_long_long(LONGLONG new_value);
    void set_c_unsigned_long_long(unsigned LONGLONG new_value);
#endif
    void set_c_size_t(size_t new_value);

    void set_integer(const IInteger &new_value);

    /**	Set the value to undetermined */
    void clear(void);



    /**	Write the value to an ostring */
    void write(std::ostream *out, int base = 10) const;

    void write(String &str, int base) const;
    String to_String(int base = 10) const;

    // Reading from an istream
    void read(const char *location, int base = 10);
    void read(std::istream *in, int base = 10);

    /** The old way to write directly to a previously
     * allocated char *.
     * \par Prefer to_string for new code */
    IInteger written_length(int base = 10) const;
    void write(char *location, int base = 10) const;

    /**	Output to a char string. This is allocated from the heap -
     * the caller is responsible for its deletion.
     * \par use to_String for new code */
    char *to_string(int base = 10) const; // create a new char *

    /**	Appears to duplicate write, but this is not clear - avoid */
    void print(std::ostream *in, int base = 10) const;

    bool is_divisible_by(const IInteger &other) const;

    IInteger add(const IInteger &other) const;
    IInteger subtract(const IInteger &other) const;
    IInteger multiply(const IInteger &other) const;
    IInteger div(const IInteger &other) const;
    IInteger mod(const IInteger &other) const;

    IInteger negate(void) const;

    IInteger &operator=(const IInteger &other);

    bool operator==(const IInteger &other) const;
    bool operator!=(const IInteger &other) const;
    bool operator<(const IInteger &other) const;
    bool operator>(const IInteger &other) const;
    bool operator<=(const IInteger &other) const;
    bool operator>=(const IInteger &other) const;


    /*
     *  We now only compare with longs
     *  or unsigned longs
     */
    bool operator==(int other) const;
    bool operator!=(int other) const;
    bool operator<(int other) const;
    bool operator>(int other) const;
    bool operator<=(int other) const;
    bool operator>=(int other) const;

    bool operator==(unsigned int other) const;
    bool operator!=(unsigned int other) const;
    bool operator<(unsigned int other) const;
    bool operator>(unsigned int other) const;
    bool operator<=(unsigned int other) const;
    bool operator>=(unsigned int other) const;

    bool operator==(long other) const;
    bool operator!=(long other) const;
    bool operator<(long other) const;
    bool operator>(long other) const;
    bool operator<=(long other) const;
    bool operator>=(long other) const;

    bool operator==(unsigned long other) const;
    bool operator!=(unsigned long other) const;
    bool operator<(unsigned long other) const;
    bool operator>(unsigned long other) const;
    bool operator<=(unsigned long other) const;
    bool operator>=(unsigned long other) const;

    /** add */
    IInteger operator+(const IInteger &other) const;
    /** subtract */
    IInteger operator-(const IInteger &other) const;
    /** multiply */
    IInteger operator*(const IInteger &other) const;
    /** divide */
    IInteger operator/(const IInteger &other) const;
    /** remainder */
    IInteger operator%(const IInteger &other) const;

    /** Bitwise exclusive or */
    IInteger operator^(const IInteger &other) const;
    /** Bitwise and */
    IInteger operator&(const IInteger &other) const;
    /** Bitwise or */
    IInteger operator|(const IInteger &other) const;
    /** Bitwise negate */
    IInteger operator~(void) const;
    /** left shift */
    IInteger operator<<(const IInteger &other) const;
    /** right shift */
    IInteger operator>>(const IInteger &other) const;

    /** logical not */
    bool operator!(void) const;
    /** logical and */
    bool operator&&(const IInteger &other) const;
    /** logical or */
    bool operator||(const IInteger &other) const;

    /* unary */
    IInteger operator+(void) const;
    IInteger operator-(void) const;

    IInteger &operator+=(const IInteger &other);
    IInteger &operator-=(const IInteger &other);
    IInteger &operator*=(const IInteger &other);
    IInteger &operator/=(const IInteger &other);
    IInteger &operator%=(const IInteger &other);
    IInteger &operator^=(const IInteger &other);
    IInteger &operator&=(const IInteger &other);
    IInteger &operator|=(const IInteger &other);
    IInteger &operator>>=(const IInteger &other);
    IInteger &operator<<=(const IInteger &other);

    /* prefix */
    IInteger &operator++(void);
    IInteger &operator--(void);

    /* postfix */
    IInteger operator++(int);
    IInteger operator--(int);

    /**
     *  The following two static functions return the GCD (Greatest Common
     *  Denominator) of op1 and op2.  In addition, the second form sets
     *  *coeff1 and *coeff2 to values such that
     *
     *      (op1 * (*coeff1)) + (op2 * (*coeff2)) = GCD.
     *
     *  These coefficients can be extracted by the same algorithm that
     *  finds the GCD.
     */
    static IInteger ii_gcd(const IInteger &op1, const IInteger &op2);
    static IInteger ii_gcd(const IInteger &op1, const IInteger &op2,
			   IInteger *coeff1, IInteger *coeff2);
    /**
     * restrict the iinteger to a finite bit size
     * use 2's complement to resolve overflows
     * infinities and undetermined will return themselves
     */
    static IInteger ii_finite_size(const IInteger &ii,
				   const IInteger &bit_size,
				   bool is_signed);

    /**	Return a positive infinity */
    static IInteger i_positive_inf(void);
    /** Return a negative infinity */
    static IInteger i_negative_inf(void);
    /** Return a signless infinity (not the same as positive infinity) */
    static IInteger i_signless_inf(void);

  };

inline IInteger zero(IInteger *)  { return 0; }

/**
 * These just dispatch to IInteger::i_positive_inf ...
 * and should disappear sometime
 */

extern IInteger i_positive_inf(void);
extern IInteger i_negative_inf(void);
extern IInteger i_signless_inf(void);

#endif /* COMMON_I_INTEGER_H */
