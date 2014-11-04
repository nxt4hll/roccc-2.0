#include "grammar.h"
#include "macro.h"
#include "common/text_util.h"
#include "parser.h"
#include "rem_comments.h"
#include <stdio.h>
#include "common/MString.h"
#include <stdlib.h>

static void AddDefine(MacroObject *mac,String *s)
    {
    int pos = s->find("=");
    if (pos < 0)
	return;
    String name = s->Mid(2,pos - 2);
    String value = s->Right(-pos-1);
    NamedList *p = to_NamedList(mac);
    if (p == NULL)
 	return;
    // printf("adding name %s value %s\n",(const char *)name,(const char *)value);
    p->AddObject(name,new StringMacroObject(value));
    // p->Print(0);
    }

extern simple_stack<String *> includes;

int main(int argc,char *argv[])
    {
    bool print_result = false;
    bool debug_it = false;
    simple_stack<String *> defines;
    //    bool consumed = true;
    while (argc > 1)
	{
	const char *x = argv[1];
	if (strcmp(x,"-p") == 0)
	    print_result = true;
	else if ((x[0] == '-') && (x[1] == 'D'))
	    defines.push(new String(x));
        else if (strcmp(x,"-d") == 0)
            debug_it = true;
	else if ((x[0] == '-') && (x[1] == 'I'))
	    includes.push(new String(x + 2));
	else 
	    break;
	argc --;
        argv ++;
	}
    if (argc < 2)
	{
	fprintf(stderr,"usage is smgn <options> <grammar> <source> <macro file 1> <macro file 2> ...\n");
	fprintf(stderr,"options are: -p print result of parsing source\n");
	fprintf(stderr,"             -d debug macro expansion\n");
	fprintf(stderr,"             -D<name>=<value> define a name for macro expansion\n");
	fprintf(stderr,"             -I<directory> add directory for includes\n");
	return -1;
	}
    Grammar g;
    if (!g.read_grammar(argv[1]))
	{
	fprintf(stderr,"errors in grammar\n");
	return -1;
	}

    if (!g.Check())
        {
        fprintf(stderr,"errors in grammar\n");
        return -1;
        }

    if (argc < 3)
	{
	printf("grammar check complete\n");
	return 0;
	}

    // g.Print();
    const char * textfile = argv[2];
    char * text = get_file_text(argv[2]);

    if (text == NULL)
	{
	fprintf(stderr,"could not read file %s\n",argv[2]);
	return -1;
	}

    // remove comment lines and include include files

    String *result_text = rem_comments(text);

    MacroObjectPtr mac = Parse(textfile, result_text->c_str(),g);
    int i;
    for (i = 0;i < defines.len(); i++)
	AddDefine(mac,defines[i]);

    if (mac.is_null())
	{
	fprintf(stderr,"parser error");
	return -1;
	}

    if (print_result)
        mac->Print();

    for (int pos =3;pos < argc;pos ++)
	{
	char *text = get_file_text(argv[pos]);
	if (text == NULL)
	    {
	    fprintf(stderr,"could not open file %s\n",argv[pos]);
	    return -1;
	    }
	int len = strlen(text);
	{
	MacroExpansion expand(len,len);
	for (i = 0;i < includes.len(); i++)
	    {
    	    expand.add_include(*includes[i]);
	    }

	expand.set_debug(debug_it);
	expand.Expand(argv[pos], text,mac);
	printf("expanded file count is %d\n",expand.file_count());
	for (i=1;i<expand.file_count();i++) {
	    String x = expand.get_file_text(i);
	    write_text(expand.get_file_name(i),(const char *)x);
	    }

	const String def = expand.get_file_text(0);
	if (def.length() > 0)
	    fprintf(stderr,"%s",(const char *)def);
	}
	//delete [] text;
	}

    return 0;
    }
