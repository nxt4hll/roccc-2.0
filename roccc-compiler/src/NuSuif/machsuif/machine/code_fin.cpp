/* file "machine/code_fin.cc */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "machine/code_fin.h"
#endif

#include <machine/substrate.h>
#include <machine/init.h>
#include <machine/problems.h>
#include <machine/contexts.h>
#include <machine/code_fin.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

/*
 * init -- Set up per-procedure variables.
 */
void
CodeFin::init(OptUnit *unit)
{
    cur_unit = unit;

    StackFrameInfoNote sfi_note = get_note(unit, k_stack_frame_info);
    claim(!is_null(sfi_note), "No stack_frame_info note on procedure %s",
	  get_name(unit).chars());

    is_leaf = sfi_note.get_is_leaf();
    is_varargs = sfi_note.get_is_varargs();
    max_arg_area = sfi_note.get_max_arg_area();

    frame_map.clear();
}



/* -------------------------   StackFrameInfoNote  -------------------------- */

enum { IS_LEAF = 0, IS_VARARGS, FRAME_SIZE,
       FRAME_OFFSET, MAX_ARG_AREA, SAVED_REG };

bool
StackFrameInfoNote::get_is_leaf() const
{
    return _get_c_long(IS_LEAF);
}

void
StackFrameInfoNote::set_is_leaf(bool is_leaf)
{
    _replace(IS_LEAF, is_leaf);
}

bool
StackFrameInfoNote::get_is_varargs() const
{
    return _get_c_long(IS_VARARGS);
}

void
StackFrameInfoNote::set_is_varargs(bool is_varargs)
{
    _replace(IS_VARARGS, is_varargs);
}

int
StackFrameInfoNote::get_frame_size() const
{
    return _get_c_long(FRAME_SIZE);
}

void
StackFrameInfoNote::set_frame_size(int frame_size)
{
    _replace(FRAME_SIZE, frame_size);
}

int
StackFrameInfoNote::get_frame_offset() const
{
    return _get_c_long(FRAME_OFFSET);
}

void
StackFrameInfoNote::set_frame_offset(int frame_offset)
{
    _replace(FRAME_OFFSET, frame_offset);
}

int
StackFrameInfoNote::get_max_arg_area() const
{
    return _get_c_long(MAX_ARG_AREA);
}

void
StackFrameInfoNote::set_max_arg_area(int max_arg_area)
{
    _replace(MAX_ARG_AREA, max_arg_area);
}


/* --------------------------   target_code_fin  ---------------------------- */

CodeFin*
target_code_fin()
{
    return dynamic_cast<MachineContext*>(the_context)->target_code_fin();
}
