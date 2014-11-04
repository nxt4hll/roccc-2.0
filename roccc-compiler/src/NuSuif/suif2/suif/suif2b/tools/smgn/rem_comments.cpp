#include "rem_comments.h"
#include "common/MString.h"
#include "common/text_util.h"
#include "common/simple_stack.h"

simple_stack<String *> includes;
static String *result;

static String get_file_name(const String &x) {
    String filename = x;
    int i = x.find("<");
    if (i > 0) {
	filename.trim_to_first('<');
	filename.truncate_to_last('>');
	}
    else {
	filename.trim_to_first('"');
	filename.truncate_to_last('"');
	}
    return filename;
    }

static void rem_comments_inner(char *text)
    {
    while (*text != 0)
	{
	char ch = *text;
	switch (ch)
	    {
	case '\\':
	    {
	    result->push(ch);
	    ch = *(++text);
	    result->push(ch);
	    ch = *(++text);
	    break;
	    }
	case '"':
	case '\'':
	    {
	    char key = ch;
	    result->push(ch);
	    ch = *(++text);
	    while ((ch != 0) && (ch != key))
		{
		if (ch == '\\')
		    {
		    result->push(ch);
		    ch = *(++text);
		    }
		result->push(ch);
		ch = *(++text);
		}
	    result->push(ch);
	    ch = *(++text);
	    break;
	    }
	case '#':
	    {
	    String include;
	    while ((ch != 0) && (ch != '\n')) {
		include.push(ch);
	        ch = *(++text);
		}
	    if (include.Left(9) == String("#include ")) {
		String filename = get_file_name(include);
		char *text = get_file_text(filename,includes.len() == 0);
		int i = 0;
		while (!text && (i < includes.len())) {
		    String fname = *(includes[i]) + String("/") + filename;
                    text = ::get_file_text((const char *)fname,(i+1) == includes.len());
		    if(text)
			break;
		    i++;
		    }
		if (text)
		    rem_comments_inner(text);
		}

	    break;
	    }
	default:
	    {
	    result->push(ch);
	    ch = *(++text);
	    }
	  }
	}
    }

String * rem_comments(char *text)
    {
    result = new String(1000,1000);
    rem_comments_inner(text);
    return result;
    }
