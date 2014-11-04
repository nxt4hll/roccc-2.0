#include "common/system_specific.h"
/* file "main.cc" */

/*  Copyright (c) 1994 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */


/*
 * SUIF to C converter
 */

#include <time.h>
#include <string.h>
#include "common/MString.h"
#include "common/text_util.h"
#include "iokernel/meta_class.h"
#include "iokernel/aggregate_meta_class.h"
#include "iokernel/union_meta_class.h"
#include "suifkernel/real_object_factory.h"
#include "iokernel/stl_meta_class.h"
#include "iokernel/virtual_iterator.h"
#include "suifkernel/suif_env.h"
#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "smgn/macroBase.h"
#include "macro_suifobj_adapter/MacroObjAdapterBase.h"
#include "smgn/macro.h"
#include <iostream.h>

FILE *c_file = stdout;

static bool debug_it = false;
static simple_stack<String *> defines;
static simple_stack<String *> includes;

static void parse_arguments(int argc, char *argv[], char **macro_filespec,
                            char **input_filespec, char **output_filespec);
static void usage(void);
static void print_header(void);

FILE*
open_c_file(char *name)
{
    FILE *f = fopen(name, "w");
    if (f == NULL) {
        fprintf(stderr, "Could not open %s for writing.\n", name);
        exit(1);
    }
    return f;
}

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

extern "C" void init_cfenodes(SuifEnv *);

extern int
main(int argc, char **argv)
{

    char* input_filespec;
    char* output_filespec = NULL;
    char* macro_filespec;
    SuifEnv* suif = new SuifEnv();

    suif->init();
    init_basicnodes(suif);
    init_suifnodes(suif);
    init_cfenodes(suif);

    parse_arguments(argc, argv, &macro_filespec, &input_filespec, &output_filespec);

    if (output_filespec != NULL)
        c_file = open_c_file(output_filespec);
    else
	c_file = stdout;

    suif->read(input_filespec);

    //FormattedText x;
    //suif->get_file_set_block()->print(x);
    //cout << x.get_value() << endl;

    FileSetBlock *fsb = suif->get_file_set_block();
    if (fsb == NULL) 
      {
        fprintf(stderr,"No loaded SUIF file\n");
        return -1;
      }
      

    MacroObjectBuilder builder;
    MacroObject *root = builder.build_macro_object((Address)fsb,fsb->get_meta_class());

    char *text = get_file_text(macro_filespec);
    if (text == NULL)
    {
        fprintf(stderr,"could not open macro file %s\n",macro_filespec);
        return -1;
    }

    for (int i = 0;i < defines.len(); i++)
        AddDefine(root,defines[i]);

    int len = strlen(text);
    MacroExpansion expand(len,len);

    for (int i = 0;i < includes.len(); i++)
    {
        String value = includes[i]->Right(-2);
        expand.add_include(value);
    }

    // root->Print(0);

    expand.set_debug(debug_it);
    expand.Expand(macro_filespec,text,root);
    for (int i=1;i<expand.file_count();i++)
        write_text(expand.get_file_name(i),expand.get_file_text(i));

    const String &def = expand.get_file_text(0);
    
    if (def.length() > 0)
        fprintf(c_file,"%s",(const char *)def);
    delete [] text;

    exit(0);
}


static void parse_arguments(int argc, char *argv[], char **macro_filespec,
                            char **input_filespec, char **output_filespec)
{
    bool consumed = true;
    while (argc > 1)
    {
        const char *x = argv[1];
        if ((x[0] == '-') && (x[1] == 'D'))
            defines.push(new String(x));
        else if (strcmp(x,"-d") == 0)
            debug_it = true;
        else if ((x[0] == '-') && (x[1] == 'I'))
            includes.push(new String(x));
        else
            break;
        argc --;
        argv ++;
    }

    if (argc < 3)
    {
        fprintf(stderr, "No input SUIF file specified.\n");
        usage();
        exit(1);
    }

    *macro_filespec = argv[1];
    *input_filespec = argv[2];
    if (argc == 4)
        *output_filespec = argv[3];
    else
        *output_filespec = NULL;
}


static void usage(void)
{
    char *usage_message[] =
    {
        "usage: s2c <options> <macro-file> <SUIF-file> [<c-file>]\n",
        "  options:\n",
        "    -d debug macro expansion\n"
        "    -D<name>=<value> define a name for macro expansion\n"
    };

    for (size_t line_num = 0;
         line_num < sizeof(usage_message) / sizeof(char *); ++line_num)
      {
        fprintf(stderr, "%s", usage_message[line_num]);
      }
}
