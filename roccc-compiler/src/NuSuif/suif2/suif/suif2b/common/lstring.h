/**	@file
 *
 *	The intent of the LString class is to give all the
 *	functionality of a string class, along with the ability
 *	to do near-constant time comparisons between two strings.
 *	To do this, we need several things:
 *	
 *		-- A hash table that operates over strings.
 *		-- A string broker.  This will have functions like
 *		'newstring', 'deletestring', 'copystring', etc.  This
 *		component will presumably be coupled with the hash table.
 *
 *		-- The LStrings need to talk to the broker.  If two strings
 *		are concatenated, then they form a new LString which needs
 *		to be hashed up and put in the broker, then returned as a new
 *		string.
 */
#ifndef _LSTRING_H_
#define _LSTRING_H_

#include <stddef.h>
#include "system_specific.h"

class String;

class LStringInner
	{
        private:
	  LStringInner(const LStringInner&);
	  LStringInner &operator=(const LStringInner&);
	public:
	    LStringInner(const char *str,int ord);
	    ~LStringInner();

	    int len;
	    int ordinal;
	    char *pStr;
	    LStringInner *lower;
	    LStringInner *higher;
	};

/**	\class LString lstring.h common/lstring.h
 *	is a shared value string class so that 
 *	comparisons for equality only require a pointer comparison.
 *      In addition, each unique value has an unique ordinal 
 *	associated with it which makes it easy to build arrays
 *	using an LString index. 
 *	\see SparseVector
 *	\warning {ordinals may change from execution to execution - do not
 *      use them for persistence.}
 */

class LString {
	LStringInner *m_p;
protected:

public:
 	/**	build an empty LString */
	LString(void);
   	/**	build an Lstring for a char * 
  	 *	\warning  {this is a slow operation, requiring a
 	 *	hash table lookup. For LString constants, create them
	 *	during program initialization using static LStrings and
	 *	try to share commonly used constants}
	 */
	LString(const char *pStr);
  	/**	copy constructor (fast) */
	LString(const LString& lstring) : m_p(lstring.m_p) {}
	/**	Create an LString from a String. Also slow - see above 
 	 *	comments */
	LString(const String &x);

	/**	Return length of LString */
	unsigned size() const {if (m_p) return m_p->len; else return 0;}
	/**	Return length of LString */
	unsigned length() const {if (m_p) return m_p->len; else return 0;}
	/**	Return as const char * */
	inline const char *c_str() const { if (m_p)return m_p->pStr;else return 0;}
	/**	Return as a cast. Will happen implicitly */
	inline operator const char*() const { if (m_p)return m_p->pStr;else return 0;}
    	String operator+(const LString& lstring2) const;
	inline bool operator==(const LString &lstring) const { return (m_p == lstring.m_p);}
 	inline bool operator!=(const LString &lstring) const { return (m_p != lstring.m_p);}

	LString& operator=(const LString &lstring) {m_p = lstring.m_p;return *this;}
	bool operator <(const LString &lstring) const;

	/**	get ordinal associate with an LString. Each unique LString
  	 *	has a unique ordinal. This is useful for indexing into
	 *	an array (for operators, for example, which are LStrings
	 *	in SUIF)
	 */
	int get_ordinal() const {if (m_p)return m_p->ordinal;return 0;}

	/**	Does an LString exist with the given value
	 *	\warning {This merely looks in the table for an entry.
 	 *	Extries are not removed when an LString is destructed, so
	 *	there may be no actual LString with the given value}
	 */
	static bool exists(const char *str);

	// Removed to reduce clutter when debugging - use the global
	// static const LString emptyString;
};

size_t hash( const LString s );
#ifdef MSVC
#ifdef COMMON_EXPORTS
#define extern extern DLLEXPORT
#else
#define extern extern DLLIMPORT
#endif
#endif

extern const LString emptyLString;
#undef extern
#endif

