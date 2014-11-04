// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef TRANSFORMS__SYMBOL_TRANSFORMS
#define TRANSFORMS__SYMBOL_TRANSFORMS


#include "suifkernel/module_subsystem.h"
#include "suifkernel/suif_env.h"
#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "procedure_walker_utilities.h"


/**
    Sets the addr_taken bits on symbols correctly. addr_taken for 
    a symbol is true iff the symbol is part of an 
    SymbolAddressExpression.

    This is needed to alleviate a problem with the front end producing 
    bogus information.
*/
class SetAddrTakenPass : public Pass {
public:
  SetAddrTakenPass(SuifEnv *the_env,  const LString &name = 
		   "set_address_taken");
  Module *clone() const;
  void initialize();
  void do_file_set_block(FileSetBlock *fsb);
};


#endif /* TRANSFORMS__DISMANTLE_IF_H */
