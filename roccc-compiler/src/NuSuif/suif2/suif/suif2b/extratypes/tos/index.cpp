/* file "index.cc" */


/*
       Copyright (c) 1997 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>


/*
      This is the implementation of the index template and related
      templates, which are templates for classes implementing mappings
      from arbitrary keys to arbitrary types, for sty, the first-level
      main library of the SUIF system.
*/


#define _MODULE_ "libsty"

//#pragma implementation "index.h"


#include <common/machine_dependent.h>
#include "const_cast.h"
#include "referenced_item.h"
#include "ion/ion.h"
#include <common/i_integer.h>
#include <common/MString.h>
#include "tos.h"
#include "slist_tos.h"
#include "dlist_tos.h"
#include "cdlist_tos.h"
#include "adlist_tos.h"
#include "special_ro_tos.h"
#include "array_tos.h"
#include "index.h"


/*
 *  This file is currently only used for the ``#pragma
 *  implementation'' of ``index.h''.
 */
