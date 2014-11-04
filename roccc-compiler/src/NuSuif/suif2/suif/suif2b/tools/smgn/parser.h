#ifndef __PARSER__
#define __PARSER__

#include "grammar.h"

#include "macro.h"

MacroObjectPtr Parse(const char *filename,
		   const char *text,Grammar &g);

#endif
