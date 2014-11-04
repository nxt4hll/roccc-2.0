/* file "suifvm/c_printer_defs.h */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

/* This module make several definitions used by m2c to simplify
 * the process of constructing a C program from suifvm instructions. */

typedef int _BOOL;

#define _ABS(s) (s < 0) ? -s : s

#define _MIN(s0, s1) (s0 < s1) ? s0 : s1

#define _MAX(s0, s1) (s0 > s1) ? s0 : s1

#define _LSR(s0, s1) ((unsigned long)s0) >> s1

