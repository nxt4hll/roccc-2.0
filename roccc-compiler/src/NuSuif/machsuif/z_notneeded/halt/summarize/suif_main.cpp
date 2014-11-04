/* file "summarize/suif_main.cpp" */

/*
    Copyright (c) 2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file. 
*/

#include <machine/copyright.h>

#include <machine/machine.h>

#include <summarize/suif_pass.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

int
main(int argc, char *argv[])
{
    // initialize the environment
    SuifEnv *suif_env = new SuifEnv;
    suif_env->init();

    init_summarize(suif_env);

    // transform the input arguments into a stream of input tokens
    TokenStream token_stream(argc, argv);

    // execute the Module "summarize"
    ModuleSubSystem *mSubSystem = suif_env->get_module_subsystem();
    mSubSystem->execute("summarize", &token_stream);

    delete suif_env;

    return 0;
}


  
