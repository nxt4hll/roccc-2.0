#ifndef _UTILS__CLONING_UTILS_H
#define _UTILS__CLONING_UTILS_H

#include "iokernel/cast.h"
#include "suifkernel/suif_env.h"
#include "basicnodes/basic_forwarders.h"
#include "basicnodes/basic.h"
#include "suifnodes/suif_forwarders.h"

/**	@file
 *	Some support for cloning.
 *	A template that makes cloning easier
 *	A statement cloner
 */



/**	\class deep_suif_clone cloning_utils.h utils/cloning_utils.h
 *      template function to clone an object
 *
 *	\warning. Using this has the potential to cause
 *	code bloat. You get one instantiation per type and 
 *	instantiations in different functions may not get merged 
 */
template<class T> 
  T *deep_suif_clone(T *obj, SuifEnv *s = 0) { 
  	if (obj == 0) return(0);
  	return(to<T>(obj->deep_clone(s))); 
    	}

/**	Clone a statement or statement list
 *	Sets up all the parameters to handle labels correctly
 *	@param	suif environment to clone in
 *	@param	symbol table to put created labels into
 *	@param	statement to clone
 */
Statement *clone_statement(SuifEnv *env,SymbolTable *table,const Statement *stat);

#endif
