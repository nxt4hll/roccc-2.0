#ifndef __STRING__
#define __STRING__
#include "lstring.h"
#include "simple_stack.h"

/**
 * @file
 * A String implementation containing most of the methods
 * that you would expect in a String.
 *
 * Strings are represented as possibly shared segment lists
 * Copying and assignment are fast. (const char *) operator
 * and c_str which access the value as a char pointer can be slow.
 */

/** @class String Mstring.h common/Mstring.h
 * Strings are represented as possibly shared segment lists.
 */
class StringInner;
class String  
    {
	StringInner *_head;
	StringInner *_tail;
	int _len;
	bool _is_shared;
        const char *get() const;
 	void free_buffers(StringInner *t);
	void free_buffers();
   	void make_contiguous(int extra_size);
	bool is_shared();
    public:
	/**
	 *	Constructors.
         */

	/** Construct an empty string */
	String();
	/** Construct a string from text*/
	String(const char *text);
	/** Construct a string from the first len characters of text */
	String(const char *text,int len);
	/** Construct an empty string with a buffer of given size
	* @param - size in bytes (rounded up to modulo 8)
	* @param - ignored. Must be present or what you will get is a 
	*	   string containing size as text
	*/
	String(int size,int expand);
	/** Construct a concatenation of the two strings */
	String(const String &x,const String &y);
	/** Construct a concatenation of the string and a character*/
	String(const String &x, char ch);
	/** Copy constructor (very fast - no text copying required) */
	String(const String &x);
 	/** Construct a string as base 10 representation of an int */
	String(int i);
 	/** Construct a string as base 10 representation of an int */
	String(size_t i);
	/** Construct a string from a bool (true or false) */
	String(bool b);
	/** Construct a string as base 10 representation of a long */
	String(long l);
	/** Construct a string as base 10 representation of double */
	String(double d);
	/**	Construct a String from an LString */
	String(const LString &x);

	/**	Assignment and concatenate and assign operators */
	String & operator =(const String &x);
	String & operator =(const char *x);

	String & operator +=(const String &x);
        String & operator +=(const char *x);
	String & operator +=(char x);
	String & operator +=(const LString &x);

	/**	Index into string 
	 *	\warning does no bounds checking
	 */
	char &operator[](int i);

	/**	append a String or char *. Equivalent to += */
	void append(const String &x);
	void append(const char *x); 

	/**	Push a character onto a string. Present for hystorical reasons.
	*	Use += operator by preference
	*/
	void push(char x);

	/**	Add a String or a single char */
	String operator +(const String &x) const;
	String operator +(const char *x) const;
	String operator +(const LString &x) const;
	String operator +(char x) const;

	/**	set the value of a string to the left most len
	 *	characters of a char *
	 *	@param - text to set
	 *	@param - no of characters to take
	 */
	void set_value(const char *text,int len);


	~String();

	/**	compare a string. 
	 *	@param	- string to compare
	 *	@return - -ve if "this" < param string, 0 if equal, +ve if greater
	 *	Very strcmp like.
	 */
  	int compare(const String &x) const;

	/**	The usual comparison operators */
	bool operator == (const String  &x) const;
        bool operator != (const String &x) const;

	bool operator < (const String  &x) const;
	bool operator <= (const String  &x) const;
	bool operator > (const String  &x) const;
	bool operator >= (const String  &x) const;

	/**	Access the string as a const char *.
	 *	Two forms, as a cast and as a function
	 */
	operator const char *() const;
	const char * c_str() const;

	/**	Truncate the string the the last instance of the character
	 *	the matching character is removed.
	 *	Hence, fullpathname.truncate_to_last('/') will
	 *	give you the directory name.
	 */
	bool truncate_to_last(char marker);

	/**	Trim the prefix finishing with the first instance of the characeter
	*	The character is trimmed.
	*/
	bool trim_to_first(char marker);

	/**	Truncate to a given position
	 *	The new length will be equal to the parameter value
	 */
	bool truncate_at_pos(int pos);

	bool is_empty() const {return _len == 0;}

	int size() const {return _len;};
	int length() const {return _len;}

	/**	Basic like string functions
	 *	Left(len) - get String of left "len" chars
	 *	Right(len) - get String of right "len" characters
	 *	Mid(left_pos,len) - extract middle part of string
     *  Repeat(int times) - return this string repeated \a times times
	 *	NOTE: for Left and Right, the length
	 *	can be negative, in which case, the length of
	 *	the string is added to the parameter - in other
	 *	words, it is the number of characters to chop off
	 */
	String Left(int len) const;
	String Right(int len) const;
	String Mid(int left_pos,int len) const;
	String substr(int left_pos,int len) const;
    String Repeat(int times);

	/**	Does the string end in or start with the comparison string? */
	bool ends_in(const String &s) const;
	bool starts_with(const String &s) const;

	/**	Find a substring. Returns index of first character of
	 *	substring, or -1 if not found
	 */
	int find(const String &x) const;

	/**	Clear a string */
  	void make_empty();

	// Removed - use the global. Having this here causes 
	// too much clutter when debugging
	// static const String emptyString;
    };

/**	Split a string into a stack of elements, separated by a separator
 *	@param x - the string to split up
 *	@param separator - the separator
 *	NOTE: repeated separators will produce an empty string.
 *	e.g. split("a  b",' ') produces "a","","b" as output.
 */
simple_stack<String> *split(const char *x,char separator);

#ifdef MSVC
#ifdef COMMON_EXPORTS
#define extern extern DLLEXPORT
#else
#define extern extern DLLIMPORT
#endif
#endif

extern const String emptyString;

#undef extern

#endif


