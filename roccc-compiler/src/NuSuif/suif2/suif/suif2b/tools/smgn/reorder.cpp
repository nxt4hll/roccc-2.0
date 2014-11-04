#include <stdio.h>
#include "text_util.h"
#include "simple_stack.h"
class text_list
    {
	 static text_list *files;
    public:
	String name;
	text_list *next;
	simple_stack<String> text;
	text_list(String the_name) : name(the_name),text(10,10) 
	    {
	    text_list *p = files;
	    text_list *q = NULL;
	    while ((p != NULL) && (p->name < the_name))
	    	{
		q = p;
		p = p->next;
		}
	    next = p;
	    if (q == NULL)
		files = this;
 	    else
		q->next = this;
	    }
	static void put_text(const char * filename);
	};

text_list *text_list::files = NULL;

int main(int argc,char *argv[])
    {
    const char *text = get_file_text(argv[1]);
    text_list * current = NULL;
    if (text == NULL)
        {
        fprintf(stderr,"could not open file %s\n",argv[1]);
        return -1;
        }

    while (*text != 0)
	{
	if (prefix_match(text,"//==== file: "))
	    {
	    const char *ftext = skip_space(text + 12);
	    String filename;
	    while ((*ftext != 0) && (*ftext != ' '))
		{
		filename.push(*ftext);
		ftext ++;
		}
	    current = new text_list(filename);
	    }
	String line;
        while ((*text != 0) && (*text != '\n'))
            {
	    if (*text != 0xd)
                line.push(*text);
            text ++;
            }
	if (*text != 0xd)
	    line.push(*text);
	if (*text != 0)
	    text ++;
	if (current != NULL)
	    current->text.push(line);
	}
    text_list::put_text(argv[2]);
    return 0;
    }

void text_list::put_text(const char *filename)
    {
    text_list *current = text_list::files;
    String result;
    while (current != NULL)
	{
	for (int i = 0;i < current->text.len();i++)
	    result += current->text[i];
	current = current->next;
	}
    write_text(filename,(const char *)result);
    }
