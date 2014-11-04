#include "system_specific.h"
#include "lstring.h"
#include <string.h>
#include <stdlib.h>
#include "MString.h"

class Lexicon {
private:
        int m_nStrCount;
        LStringInner *bases[64];
protected:

public:
        Lexicon() : m_nStrCount(0)
	    {
	    for (int i =0;i<64;i++)
		bases[i] = 0;
	    }

	LStringInner *find_str(const char *str,bool enter = true)
	    {
	    int index = *str & 0x3f;
	    LStringInner *key = bases[index];
	    LStringInner *last = 0;
	    int compare = 0;
	    while (key != 0)
		{
		compare = strcmp(key->pStr,str);
		last = key;
		if (compare < 0)
		    key = key->lower;
		else if (compare > 0)
		    key = key->higher;
		else
		    {
		    return key;
		    }
		}
	    if (!enter)
	  	return 0;
	    m_nStrCount ++; // 0 is used for null string
	    key = new LStringInner(str,m_nStrCount);
	    if (last == 0)
		{
		bases[index] = key;
		}
	    else if (compare < 0)
		last->lower = key;
	    else
		last->higher = key;
	    return key;
	    }
};


LStringInner::LStringInner(const char *str,int ord)
	: len(strlen(str)),ordinal(ord),pStr(0),lower(0),higher(0) {
    pStr = new char[len + 1];
    memcpy(pStr,str,len + 1);
    }

LStringInner::~LStringInner()
    {
    delete pStr;
    }

static Lexicon *g_Broker = 0;

static inline Lexicon *get_broker()
    {
    if (g_Broker == 0) {
	g_Broker = new Lexicon;
	}
    return g_Broker;
    }


LString::LString(void) :
  m_p(get_broker()->find_str("")) {
}

LString::LString(const char *pStr) :
  m_p(get_broker()->find_str(pStr)) {
}


LString::LString(const String &x) :
  m_p(x.length() != 0 ? get_broker()->find_str(x.c_str()) :
      get_broker()->find_str(""))
{
}

String LString::operator+(const LString& lstring2) const
{
	return String(*this) + String(lstring2);
}

const LString emptyLString;

bool LString::exists(const char *str) {
  return (get_broker()->find_str(str,false) != 0);
}

size_t hash( const unsigned int );

size_t hash( const LString s ) {
  const char* ptr = s.c_str();
  unsigned int hash_value = 0;
  while ( *ptr ) { hash_value+=*ptr;ptr++; }
  return hash( hash_value );
}


// const LString LString::emptyString;

bool LString::operator <(const LString &lstring) const {
    if (!m_p)
	return (lstring.m_p!=0);
    const char *l = m_p->pStr;
    const char *r = lstring.m_p->pStr;
    while (*l && (*l == *r)) {
	l++;
	r++;
	}
    if (*l == 0)
	return false;
    return (*l < *r);
    }
