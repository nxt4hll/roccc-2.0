/* file "halt/note.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "halt/note.h"
#endif

#include <machine/machine.h>

#include <halt/note.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

// This file exists to match the #pragma interface for note.h,
// which must be included above so that G++ knows to generate
// object code for normally inlined methods.
