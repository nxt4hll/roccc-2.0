// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include <common/suif_copyright.h>

#include <iostream>
#include <iokernel/cast.h>
#include <iokernel/clone_stream.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>
#include <basicnodes/basic.h>
#include <basicnodes/basic_constants.h>
#include <suifkernel/suifkernel_messages.h>
#include <suifkernel/utilities.h>
#include <suifkernel/group_walker.h>
#include "transforms/procedure_walker_utilities.h"
#include <utils/expression_utils.h>
#include <utils/symbol_utils.h>
#include <utils/type_utils.h>
#include <utils/cloning_utils.h>
#include <cfenodes/cfe.h>
#include "roccc_utils/IR_utils.h"
#include "roccc_utils/list_utils.h"
#include "roccc_utils/warning_utils.h"
#include "common/suif_list.h"
#include "flatten_statement_lists_pass.h"

using namespace std;

/**************************** Declarations ************************************/

/**************************** Implementations ************************************/

FlattenStatementListsPass::FlattenStatementListsPass( SuifEnv* suif_env) : 
				PipelinablePass( suif_env, "FlattenStatementListsPass" ) {}


void FlattenStatementListsPass::do_procedure_definition(ProcedureDefinition *proc_def){

  OutputInformation("Flatten statement lists pass begins") ;
  if(proc_def)
  {
    list<StatementList*>* lists = 
      collect_objects<StatementList>(proc_def->get_body());
    while(lists->size() > 0)
    {
      StatementList *the_list = *(lists->get_nth(0));
      if(is_a<StatementList>(the_list->get_parent()))
      {
	StatementList* parent_list = 
	  to<StatementList>(the_list->get_parent()) ;
	StatementList* replacement_list = 
	  create_statement_list(get_suif_env());

	while(parent_list->get_statement_count() > 0)
	{
	  if(is_a<StatementList>(parent_list->get_statement(0)))
	  {
	    StatementList *current_list = 
	      to<StatementList>(parent_list->remove_statement(0));
	    while(current_list->get_statement_count() > 0)
	    {
	      replacement_list->
		append_statement(current_list->remove_statement(0));
	    }
	    remove_from_list(current_list, lists);
	  }
	  else
	  { 
	    replacement_list->
	      append_statement(parent_list->remove_statement(0));
	  }
	}
	parent_list->get_parent()->replace(parent_list, replacement_list);
	remove_from_list(parent_list, lists);
	lists->push_back(replacement_list);	
      }
      else if(the_list->get_statement_count() == 1)
      {
	the_list->get_parent()->
	  replace(the_list, the_list->remove_statement(0));
	lists->erase(0);	
      }
      else
      {
	lists->erase(0);
      }
      
    }
    delete lists;
  }
  OutputInformation("Flatten statement lists pass ends") ;
}

