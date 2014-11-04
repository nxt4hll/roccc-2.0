// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
/* FILE "symbol_transforms.cpp" */


/*
       Copyright (c) 1998 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include "common/suif_copyright.h"


#include "iokernel/cast.h"
#include "suifkernel/utilities.h"
#include "common/i_integer.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "cfenodes/cfe.h"
#include "basicnodes/basic_constants.h"
#include "suifkernel/suifkernel_messages.h"
#include "procedure_walker_utilities.h"
#include "utils/expression_utils.h"
#include "utils/type_utils.h"
#include "cfeutils/cfe_dismantle.h"
#include "cfeutils/cexpr_utils.h"
#include "typebuilder/type_builder.h"

#include "expression_dismantlers.h"
#include "utils/trash_utils.h"
#include "common/suif_hash_map.h"
#include "utils/type_utils.h"
#include "common/suif_list.h"
#include "suifkernel/command_line_parsing.h"
#include "symbol_transforms.h"

SetAddrTakenPass::SetAddrTakenPass(SuifEnv *the_env,  const LString &name) :
  Pass(the_env, name)
{}
Module *SetAddrTakenPass::clone() const {
  return((Module*)this);
}

void SetAddrTakenPass::initialize() {
  Pass::initialize();
  _command_line->set_description("Set addr_taken to true iff the symbol is part of an SymbolAddressExpression"); 
}

void SetAddrTakenPass::do_file_set_block(FileSetBlock *fsb) {
  // set all add_taken bits to false first
  {for (Iter<Symbol> iter = object_iterator<Symbol>(fsb);
       iter.is_valid(); iter.next())
  {
    Symbol *sym = &iter.current();
    sym->set_is_address_taken(false);
  }}

  // now set those that are part of SymbolAddressExpression
  // to true
  {for (Iter<SymbolAddressExpression> iter =
	 object_iterator<SymbolAddressExpression>(fsb);
       iter.is_valid(); iter.next())
  {
    SymbolAddressExpression *expr = &iter.current();
    Symbol *sym = expr->get_addressed_symbol();
    sym->set_is_address_taken(true);
  }}
}
