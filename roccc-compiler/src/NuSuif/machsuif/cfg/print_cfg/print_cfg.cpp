/* file "print_cfg/print_cfg.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "print_cfg/print_cfg.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>

#include <print_cfg/print_cfg.h>


#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

void
PrintCfg::do_opt_unit(OptUnit *unit)
{
  //const char *cur_proc_name = get_name(unit).chars();

    // get the body of the OptUnit
    AnyBody *orig_body = get_body(unit);

    claim(is_kind_of<Cfg>(orig_body),
	  "expected OptUnit body in CFG form");

    Cfg *cfg = (Cfg *)orig_body;
   
    /*
    char *fname = new char[strlen(cur_proc_name) + strlen(".cfg") + 10];
    sprintf(fname, "%s.cfg", cur_proc_name);
    FILE *f = fopen(fname, "w");
    delete fname; fname = NULL;
    fprint(f, cfg, show_layout, show_code);
    fclose(f);
    */

    //fprint(stdout, cfg, true, true);
    fprint(stdout, cfg, show_layout, show_code);
}
