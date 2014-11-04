#ifndef UTILS__VALUE_BLOCK_UTILS_H
#define UTILS__VALUE_BLOCK_UTILS_H
#include "suifkernel/suif_env.h"
#include "basicnodes/basic_forwarders.h"
#include "suifnodes/suif_forwarders.h"

/**	append a value block to a multivalue block. The position is
 *	calculated from the size of the multi value block
 *	BUG: Padding is not performed
 */
void append_to_multi_value_block( MultiValueBlock* block, ValueBlock* value_block );

/**	Build a string constant
 *
 *	The variable is added at global scope.
 *	@param Suif environment
 *      @param The string
 *
 *	NB: Requires that there be exactly one FileBlock in the suif environment
 */
VariableSymbol *build_string_constant_variable(SuifEnv *env,const char *string);

/**	Build a variable for a given value block.
 *
 *	@param   suif environment
 *	@param   name for variable
 *      @param   type for variable
 *      @param   value block
 *      @param   true for static (default false)
 */
VariableSymbol *build_initialized_variable(
	SuifEnv *env,const LString &name,DataType *type,ValueBlock *vb,bool make_static = false);


#endif
