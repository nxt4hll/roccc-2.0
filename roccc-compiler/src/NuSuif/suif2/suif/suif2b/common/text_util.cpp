#include "system_specific.h"
#include <string.h>
#include "text_util.h"
#include <stdio.h>

static bool valid_name_characters[] = {
	false,false,false,false,false,false,false,false,  false,false,false,false,false,false,false,false,	// 00 - 0f
        false,false,false,false,false,false,false,false,  false,false,false,false,false,false,false,false,	// 10 - 1f
        false,false,false,false,false,false,false,false,  false,false,false,false,false,false,false,false,	// 20 - 2f
	// 0     1     2     3     4     5     6     7       8     9 
        true ,true ,true ,true ,true ,true ,true ,true,   true ,true ,false,false,false,false,false,false,	// 30 - 3f
	//       A     B     C     D     E     F     G       H     I     J     K     L     M     N     O
	false,true ,true ,true ,true ,true ,true ,true,   true ,true ,true ,true ,true ,true ,true ,true,	// 40 - 4f
	// P     Q     R     S     T     U     V     W       X     Y     Z                             _
        true ,true ,true ,true ,true ,true ,true ,true,   true ,true ,true ,false,false,false,false,true ,      // 50 - 5f
        //       a     b     c     d     e     f     g       h     i     j     k     l     m     n     o
        false,true ,true ,true ,true ,true ,true ,true,   true ,true ,true ,true ,true ,true ,true ,true,       // 60 - 6f
        // p     q     r     s     t     u     v     w       x     y     z
        true ,true ,true ,true ,true ,true ,true ,true,   true ,true ,true ,false,false,false,false,false,      // 70 - 7f
        false,false,false,false,false,false,false,false,  false,false,false,false,false,false,false,false,      // 80 - 8f
        false,false,false,false,false,false,false,false,  false,false,false,false,false,false,false,false,      // 90 - 9f
        false,false,false,false,false,false,false,false,  false,false,false,false,false,false,false,false,      // a0 - af
        false,false,false,false,false,false,false,false,  false,false,false,false,false,false,false,false,      // b0 - bf
        false,false,false,false,false,false,false,false,  false,false,false,false,false,false,false,false,      // c0 - cf
        false,false,false,false,false,false,false,false,  false,false,false,false,false,false,false,false,      // d0 - df
        false,false,false,false,false,false,false,false,  false,false,false,false,false,false,false,false,      // e0 - ef
        false,false,false,false,false,false,false,false,  false,false,false,false,false,false,false,false};     // f0 - ff



int get_name(const char *text,String &buff)
    {
    int pos = 0;
    while (valid_name_characters[(int)text[pos]])
	pos ++;
    buff.set_value(text,pos);
    return pos;
    }

int get_value(const char *text,int &val)
    {
    int pos = 0;
    val = 0;
    while (isnumeric(text[pos]))
        {
	val = 10*val + text[pos] - '0';
        pos++;
        }
    return pos;
    }



int get_mark(const char *text,String &buff)
    {
    int pos = 0;
    buff.make_empty();
    while (!isalpha(text[pos]) && !iswhitespace(text[pos]))
        {
        buff.push(text[pos]);
        pos++;
        }
    return pos;
    }

int get_string(const char *text,String &buff)
    {
    int delim = *text;
    text ++;
    int pos = 0;
    buff.make_empty();
    while ((text[pos] != 0) && (text[pos] != delim))
        {
	if (text[pos] == '\\')
	    {
	    pos ++;
	    if (text[pos] == 0)
		break;
	    if (text[pos] == 'n')
		buff.push('\n');
	    else 
		buff.push(text[pos]);
	    pos ++;
	    }
	else
	    {
            buff.push(text[pos]); 
            pos++;
	    }
        }

    return pos + 2;
    }


const char *skip_space(const char *text)
    {
    while (iswhitespace(*text))
        text ++;
    return text;
    }

const char *skip_past(const char *text,char ch)
    {
    while ((*text != 0) && (*text != ch))
         text++;
    if (*text == ch)
        text++;
    return text;
    }

char *get_file_text(const char *filename,bool diagnose)
    {
    FILE * file = fopen(filename,"r");
    if (file == NULL)
        {
        if(diagnose)fprintf(stderr,"could not open file %s\n",filename);
        return NULL;
        }

    int status = fseek(file,0,SEEK_END);

    if (status != 0)
        {
        fprintf(stderr,"could not open file %s\n",filename);
        return NULL;
        }

    long len = ftell(file);
    if (len == 0)
        {
        fprintf(stderr,"could not open file %s\n",filename);
        return NULL;
        }

    status = fseek(file,0,SEEK_SET);
    if (status != 0)
        {
        fprintf(stderr,"could not open file %s\n",filename);
        return NULL;
        }
    char *ftext = new char[len + 10]; // 10 for luck - should only need 1
    if (ftext == NULL)
        {
        fprintf(stderr,"out of memory trying to read in %s\n",filename);
        return NULL;
        }

    status = fread(ftext,1,len,file);
#ifndef MSVC
    if (status != len)
        {
        fprintf(stderr,"warning could not read all of file %s\n",filename);
#endif
        if (status <= 0)
	    return NULL;
#ifndef MSVC
        }
#endif
    ftext[status] = 0;
    return ftext;
    }

bool prefix_match(const char *t1,const char *t2)
    {
    while ((*t1 == *t2) && (*t1 != 0))
	{
	t1 ++;
	t2 ++;
	}
    return (*t2 == 0);
    }

int line_count(const char *start,const char *current)
    {
    int count = 0;
    while (start < current)
        {
        if (*start == '\n')
            count ++;
        start ++;
        }
    return count + 1;
    }

void send_error(const char *start,const char *text,const char *message,bool use_stdout) {
  send_error(NULL, start, text, message, use_stdout);
}

void send_error(const char *filename, const char *start,const char *text,const char *message,bool use_stdout)
    {
    FILE *out = stderr;
    if (use_stdout)
	out = stdout;
    int line_no = line_count(start,text);
    if (filename == NULL) {
      fprintf(out,"line #%d: %s\n",line_no,message);
    } else {
      fprintf(out,"%s:%d: %s\n",filename, line_no,message);
    }
    char extract[80];
    char tab[80];
    int i = 70;
    const char *t = text;
    while ((i > 0) && (t > start) && (*t != '\n'))
        {
        i--;
        t--;
        }
    i = 0;

    if (*t == '\n')
        t++;

    while ((i < 79) && (*t != '\n'))
        {
        extract[i]= *t;
        if (extract[i] == 9)
            tab[i] = 9;
        else
            tab[i] = ' ';
        if (t == text)
            tab[i] = '^';
        t++;
        i++;
        }
    extract[i] = 0;
    tab[i] = 0;
    fprintf(out,"%s\n%s\n",extract,tab);
    }

bool string_match_case(const char *text,const char *match,bool as_word)
    {
    while ((*match != 0) && (UPCASE(*text) == UPCASE(*match)))
        {
        text ++;
        match ++;
        }

    if (as_word && (isalpha(*text) || (*text == '_')))
	return false;
    return (*match == 0);
    }

bool write_text(const char *filename,const char *text)
    {
    FILE * outfile = fopen(filename,"wb+");
    if (outfile == NULL)
        {
        fprintf(stderr,"could not open output file  %s\n",filename);
        return false;
        }
    fwrite(text,sizeof(char),strlen(text),outfile);
    fclose(outfile);
    return true;
    }

const char *skip_space_and_comments(const char *text,bool nl)
    {
    while (iswhitespace(*text) || (nl && (*text == '#')))
        {
        if (*text == '\n')
            {
            nl = true;
            }
        else if ((*text == '#') && nl)
            {
            while ((*text != 0) && (*text != '\n'))
               text ++;
            }
        text ++;
        }
    return text;
    }


