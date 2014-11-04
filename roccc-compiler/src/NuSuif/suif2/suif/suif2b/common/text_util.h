#ifndef __TEXT_UTIL__
#define __TEXT_UTIL__

#include <ctype.h>
#include "MString.h"

/**	@file
	Text manipulation utilities
*/
/*inline int isalpha(int ch)
    {
    ch |= 'a' - 'A';
    return ((ch >= 'a') && (ch <= 'z'));
    }*/

inline int isnumeric(int ch)
    {
    return ((ch >= '0') && (ch <= '9'));
    }

inline int iswhitespace(int ch)
    {
    return ((ch == '\n') || (ch == ' ') || (ch == 9));
    }

inline char UPCASE(const char x)
    {
    if ((x >= 'a') && (x <= 'z'))
        return x-'a'+'A';
    else
        return x;
    }

inline char LOWCASE(const char x)
    {
    if ((x >= 'A') && (x <= 'Z'))
        return x-'A'+'a';
    else
        return x;
    }




int get_name(const char *text,String &buff);

int get_value(const char *text,int &val);

int get_mark(const char *text,String &buff);

int get_string(const char *text,String &buff);

const char *skip_space(const char *text);

const char *skip_past(const char *text,char ch);

char *get_file_text(const char *filename,bool diagnose = true);

bool prefix_match(const char *t1,const char *t2);

void send_error(const char *filename, const char *start,const char *text,const char * message,bool use_stdout = false);
void send_error(const char *start,const char *text,const char * message,bool use_stdout = false);

bool string_match_case(const char *text,const char *match,bool as_word = false);

bool write_text(const char *filename,const char *text);

const char *skip_space_and_comments(const char *text,bool nl = false);

#endif
