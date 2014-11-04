/* file "bvd/flow_fun.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "bvd/flow_fun.h"
#endif

#include <bvd/flow_fun.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

FlowFun::FlowFun(int num_slots_hint)
    : _id(false, num_slots_hint), _cs(false, num_slots_hint)
{
    // set up identity function on bit vectors
    _id.insert_all();
    _cs.remove_all();
}

FlowFun::FlowFun(const FlowFun &other)
{ 
    _id = other._id;
    _cs = other._cs;
}

void
FlowFun::operator=(const FlowFun &other)
{
    _id = other._id;
    _cs = other._cs;
}

void
FlowFun::print(int bound, FILE *fp) {

    putc('{', fp);

    for (int i = 0; i < bound; i++) {
	if (!_id.contains(i))
	    fprintf(fp, "%s", (_cs.contains(i) ? "1" : "0"));
	else
	    fprintf(fp, "%s", (_cs.contains(i) ? "x" : "i"));
    }

    putc('}', fp);
}
