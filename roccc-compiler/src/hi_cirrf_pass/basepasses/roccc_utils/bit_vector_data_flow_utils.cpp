// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include <common/suif_copyright.h>
#include <common/suif_list.h>

#include <iostream>
#include <iokernel/cast.h>
#include <iokernel/clone_stream.h>
#include <common/i_integer.h>
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
#include "list_utils.h"
#include "print_utils.h"
#include "IR_utils.h"
#include "control_flow_utils.h"
#include "roccc_extra_types/bit_vector.h"
#include "bit_vector_annote_utils.h"
#include "bit_vector_data_flow_utils.h"

using namespace std;

/**************************** Declerations ************************************/

/************************** Implementations ***********************************/

// kills the definitions in A that has the same destination with s
void kill_annotes(BrickAnnote *A, StoreVariableStatement *s)
{
  SuifObjectBrick *sob = to<SuifObjectBrick>(A->get_brick(0));
  BitVector *bv = (BitVector*)(sob->get_object());
  BitVectorMap *bv_map = (BitVectorMap*)(bv->get_bit_vector_map());
  list<int>* kill_set_list = bv_map->get_kill_set(s); 

  for(list<int>::iterator iter = kill_set_list->begin();
      iter != kill_set_list->end(); iter++)
  {
    bv->unmark(*iter);
  }
}

// ROCCC 2.0 change.
//  There are multiple definitions for every call statement.  We must
//  kill the definitions in A that have the same destintion as any of
//  the destinations from the call statement s.
// kills the definitions in A that has the same destination with s
void kill_annotes(BrickAnnote *A, CallStatement *s)
{
  SuifObjectBrick *sob = to<SuifObjectBrick>(A->get_brick(0));
  BitVector *bv = (BitVector*)(sob->get_object());	
  BitVectorMap *bv_map = (BitVectorMap*)(bv->get_bit_vector_map());
  list<int>* kill_set_list = bv_map->get_kill_set(s); 
  
  for(list<int>::iterator iter = kill_set_list->begin();
      iter != kill_set_list->end(); iter++)
  {
    bv->unmark(*iter);
  }

}

// kills the definitions in A that has the same destination with s
void kill_annotes(BrickAnnote *A, StoreStatement *s){
        ArrayReferenceExpression *destination_address = to<ArrayReferenceExpression>(s->get_destination_address());

	int i = 0;
	while(i < A->get_brick_count()){
	    SuifObjectBrick *sob = to<SuifObjectBrick>(A->get_brick(i));
	    if(is_a<StoreStatement>(sob->get_object())){
	       StoreStatement *store_stmt = to<StoreStatement>(sob->get_object());
	       ArrayReferenceExpression *array_ref_expr = to<ArrayReferenceExpression>(store_stmt->get_destination_address());
	       if(is_equal(destination_address, array_ref_expr) 
//		&&  is_all_reaching_defs_in_expr_same(destination_address, array_ref_expr)
		)
	          delete (A->remove_brick(i));
	       else i++; 
	    }
	}
}

// kills the expressions in A that uses the variable defined by s
void kill_ae_annotes(BrickAnnote *A, StoreVariableStatement *s)
{
  VariableSymbol *dest_symbol = s->get_destination();

  int i = 0;
  bool found = false;
  while(i < A->get_brick_count())
  {
    SuifObjectBrick *sob = to<SuifObjectBrick>(A->get_brick(i));
    BinaryExpression *binary_expr = to<BinaryExpression>(sob->get_object());
    
    list<LoadVariableExpression*> *load_var_expr_list = collect_objects<LoadVariableExpression>(binary_expr);
    for(list<LoadVariableExpression*>::iterator iter = load_var_expr_list->begin();
	iter != load_var_expr_list->end(); iter++){
      
      LoadVariableExpression *load_var_expr = to<LoadVariableExpression>(*iter);
      VariableSymbol *source_symbol = load_var_expr->get_source();
      if(dest_symbol == source_symbol){
	found = true;
	break;
      }
    }
    if(found)
      A->remove_brick(i);
    else i++;
    found = false;
  }
}

// kills the expressions in A that uses the variable defined by s
void kill_ae_annotes(BrickAnnote *A, CallStatement *s)
{
  VariableSymbol *dest_symbol = s->get_destination();
  
  int i = 0;
  bool found = false;
  while(i < A->get_brick_count())
  {
    SuifObjectBrick *sob = to<SuifObjectBrick>(A->get_brick(i));
    BinaryExpression *binary_expr = to<BinaryExpression>(sob->get_object());
    
    list<LoadVariableExpression*> *load_var_expr_list = collect_objects<LoadVariableExpression>(binary_expr);
    for(list<LoadVariableExpression*>::iterator iter = load_var_expr_list->begin();
	iter != load_var_expr_list->end(); iter++){
      
      LoadVariableExpression *load_var_expr = to<LoadVariableExpression>(*iter);
      VariableSymbol *source_symbol = load_var_expr->get_source();
      if(dest_symbol == source_symbol){
	found = true;
	break;
      }
    }
    if(found)
      A->remove_brick(i);
    else i++;
    found = false;
  }
}

void remove_from_in_n_out_stmts(Statement *to_be_removed, ProcedureDefinition *proc_def)
{
  
  if(to_be_removed == NULL)
    return;

  list<StoreVariableStatement*>* store_var_stmts_to_be_removed = collect_objects<StoreVariableStatement>(to_be_removed);
  list<CallStatement*>* call_stmts_to_be_removed = collect_objects<CallStatement>(to_be_removed);

  list<Statement*>* stmts = collect_objects<Statement>(proc_def);
  for(list<Statement*>::iterator iter = stmts->begin();
      iter != stmts->end(); iter++){
    
    if(is_a<StatementList>(*iter))
      continue;
    
    Statement *stmt = *iter;
    
    BrickAnnote *in_stmts = to<BrickAnnote>(stmt->lookup_annote_by_name("in_stmts"));
    SuifObjectBrick *sob = to<SuifObjectBrick>(in_stmts->get_brick(0));
    BitVector *bv = (BitVector*)(sob->get_object());
    
    for(list<StoreVariableStatement*>::iterator iter2 = store_var_stmts_to_be_removed->begin();
	iter2 != store_var_stmts_to_be_removed->end(); iter2++)
      bv->unmark(*iter2);
    
    for(list<CallStatement*>::iterator iter2 = call_stmts_to_be_removed->begin();
	iter2 != call_stmts_to_be_removed->end(); iter2++)
      bv->unmark(*iter2);
  }
  
  delete stmts;
  delete call_stmts_to_be_removed;
  delete store_var_stmts_to_be_removed;
}

void remove_from_in_n_out_stmts(list<Statement*>* to_be_removed, ProcedureDefinition *proc_def)
{
  if(to_be_removed->empty())
    return;

  list<Statement*>::iterator iter = to_be_removed->begin();
  list<StoreVariableStatement*>* store_var_stmts_to_be_removed = collect_objects<StoreVariableStatement>(*iter);
  list<CallStatement*>* call_stmts_to_be_removed = collect_objects<CallStatement>(*iter);
  
  for(iter++ ; iter != to_be_removed->end(); iter++)
  {    
    list<StoreVariableStatement*>* current_store_vars_to_be_removed = collect_objects<StoreVariableStatement>(*iter);

    for(list<StoreVariableStatement*>::iterator iter2 = current_store_vars_to_be_removed->begin();
	iter2 != current_store_vars_to_be_removed->end(); iter2++)
      store_var_stmts_to_be_removed->push_back(*iter2);
    
    list<CallStatement*>* current_calls_to_be_removed = collect_objects<CallStatement>(*iter);
    
    for(list<CallStatement*>::iterator iter2 = current_calls_to_be_removed->begin();
	iter2 != current_calls_to_be_removed->end(); iter2++)
      call_stmts_to_be_removed->push_back(*iter2);
    
    delete current_store_vars_to_be_removed;
    delete current_calls_to_be_removed;
  }
  
  list<Statement*>* stmts = collect_objects<Statement>(proc_def);
  for(list<Statement*>::iterator iter = stmts->begin();
      iter != stmts->end(); iter++){
    
    if(is_a<StatementList>(*iter))
      continue;
    
    Statement *stmt = *iter;
    
    BrickAnnote *in_stmts = to<BrickAnnote>(stmt->lookup_annote_by_name("in_stmts"));
    SuifObjectBrick *sob = to<SuifObjectBrick>(in_stmts->get_brick(0));
    BitVector *bv = (BitVector*)(sob->get_object());
    
    for(list<StoreVariableStatement*>::iterator iter2 = store_var_stmts_to_be_removed->begin();
	iter2 != store_var_stmts_to_be_removed->end(); iter2++)
      bv->unmark(*iter2);
    
    for(list<CallStatement*>::iterator iter2 = call_stmts_to_be_removed->begin();
	iter2 != call_stmts_to_be_removed->end(); iter2++)
      bv->unmark(*iter2);
  }
  
  delete stmts;
  delete call_stmts_to_be_removed;
  delete store_var_stmts_to_be_removed;
}

void remove_from_kill_set_map(Statement *to_be_removed, 
			      ProcedureDefinition *proc_def)
{
  if(to_be_removed == NULL)
    return;

  BrickAnnote *bit_vector_map_annote = to<BrickAnnote>(proc_def->lookup_annote_by_name("bit_vector_map"));
  SuifObjectBrick *sob = to<SuifObjectBrick>(bit_vector_map_annote->get_brick(0));
  BitVectorMap *bv_map = (BitVectorMap*)(sob->get_object());
  
  remove_from_kill_set_map(to_be_removed, bv_map);
}

void remove_from_kill_set_map(Statement *to_be_removed, BitVectorMap *bv_map)
{
  if(to_be_removed == NULL)
    return;

  if(is_a<StoreVariableStatement>(to_be_removed) || is_a<CallStatement>(to_be_removed))
  {
    
    bv_map->remove_from_kill_set_map(to_be_removed);
    
  }
  else
  {
    list<StoreVariableStatement*>* store_var_stmts_to_be_removed = collect_objects<StoreVariableStatement>(to_be_removed);
    list<CallStatement*>* call_stmts_to_be_removed = collect_objects<CallStatement>(to_be_removed);
    
    for(list<StoreVariableStatement*>::iterator iter = store_var_stmts_to_be_removed->begin();
	iter != store_var_stmts_to_be_removed->end(); iter++)
      bv_map->remove_from_kill_set_map(*iter);
    
    for(list<CallStatement*>::iterator iter = call_stmts_to_be_removed->begin();
	iter != call_stmts_to_be_removed->end(); iter++)
      bv_map->remove_from_kill_set_map(*iter);
    
    delete store_var_stmts_to_be_removed;
    delete call_stmts_to_be_removed;
  }
}

void remove_from_kill_set_map(list<Statement*> *to_be_removed, 
			      BitVectorMap *bv_map)
{
  if(to_be_removed->empty())
    return;

  list<Statement*>::iterator iter = to_be_removed->begin();
  list<StoreVariableStatement*>* store_var_stmts_to_be_removed = collect_objects<StoreVariableStatement>(*iter);
  list<CallStatement*>* call_stmts_to_be_removed = collect_objects<CallStatement>(*iter);

  for(iter++ ; iter != to_be_removed->end(); iter++)
  {
    list<StoreVariableStatement*>* current_store_vars_to_be_removed = collect_objects<StoreVariableStatement>(*iter);

    for(list<StoreVariableStatement*>::iterator iter2 = current_store_vars_to_be_removed->begin();
	iter2 != current_store_vars_to_be_removed->end(); iter2++)
      store_var_stmts_to_be_removed->push_back(*iter2);
    
    list<CallStatement*>* current_calls_to_be_removed = collect_objects<CallStatement>(*iter);

    for(list<CallStatement*>::iterator iter2 = current_calls_to_be_removed->begin();
	iter2 != current_calls_to_be_removed->end(); iter2++)
      call_stmts_to_be_removed->push_back(*iter2);
    
    delete current_store_vars_to_be_removed;
    delete current_calls_to_be_removed;
  }

  for(list<StoreVariableStatement*>::iterator iter = store_var_stmts_to_be_removed->begin();
      iter != store_var_stmts_to_be_removed->end(); iter++)
    bv_map->remove_from_kill_set_map(*iter);
  
  for(list<CallStatement*>::iterator iter = call_stmts_to_be_removed->begin();
      iter != call_stmts_to_be_removed->end(); iter++)
    bv_map->remove_from_kill_set_map(*iter);
  
  delete store_var_stmts_to_be_removed;
  delete call_stmts_to_be_removed;
}

void remove_from_kill_set_map(list<Statement*>* to_be_removed, ProcedureDefinition *proc_def){

        BrickAnnote *bit_vector_map_annote = to<BrickAnnote>(proc_def->lookup_annote_by_name("bit_vector_map"));
        SuifObjectBrick *sob = to<SuifObjectBrick>(bit_vector_map_annote->get_brick(0));
        BitVectorMap *bv_map = (BitVectorMap*)(sob->get_object());

	remove_from_kill_set_map(to_be_removed, bv_map);
}

bool annotes_equal_for_sym(BrickAnnote *A, BrickAnnote *B, SuifObject *s, VariableSymbol *var_sym){

        SuifObjectBrick *sob = to<SuifObjectBrick>(B->get_brick(0));
        BitVector *bv_B = (BitVector*)(sob->get_object());

        sob = to<SuifObjectBrick>(A->get_brick(0));
        BitVector *bv = (BitVector*)(sob->get_object());

        BitVectorMap *bv_map = bv->get_bit_vector_map();
        list<int>* kill_set_list = bv_map->get_kill_set(var_sym);

        for(list<int>::iterator iter = kill_set_list->begin();
            iter != kill_set_list->end(); iter++){

            if(bv->is_marked(*iter))
               if(bv_map->reverse_lookup(*iter) != s)
                  if(!(bv_B->is_marked(*iter)))
                     return 0;   

            if(bv_B->is_marked(*iter))
               if(bv_map->reverse_lookup(*iter) != s)
                  if(!(bv->is_marked(*iter)))
                     return 0;
        }

        return 1;       // equal
}

bool is_all_reaching_definitions_in_expr_same(Expression *a, Expression *b){

    list<SuifObject*>* reaching_defs_a = new list<SuifObject*>;

    for(Iter<LoadVariableExpression> iter = object_iterator<LoadVariableExpression>(a);
        iter.is_valid(); iter.next()){

        LoadVariableExpression *load_var_expr = &iter.current();
        BrickAnnote *reaching_defs = to<BrickAnnote>(load_var_expr->lookup_annote_by_name("reaching_defs"));

        for(Iter<SuifBrick*> iter = reaching_defs->get_brick_iterator();
            iter.is_valid(); iter.next()){
            SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
            SuifObject *def_obj = sob->get_object();

            if(!is_in_list(def_obj, reaching_defs_a))
               reaching_defs_a->push_back(def_obj);
        }
    }

    for(Iter<LoadVariableExpression> iter = object_iterator<LoadVariableExpression>(b);
        iter.is_valid(); iter.next()){

        LoadVariableExpression *load_var_expr = &iter.current();
        BrickAnnote *reaching_defs = to<BrickAnnote>(load_var_expr->lookup_annote_by_name("reaching_defs"));

        for(Iter<SuifBrick*> iter = reaching_defs->get_brick_iterator();
            iter.is_valid(); iter.next()){
            SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
            SuifObject *def_obj = sob->get_object();

	    if(!is_in_list(def_obj, reaching_defs_a)){
               delete reaching_defs_a;
	       return 0;
	    }
        }
    }

    delete reaching_defs_a;
    return 1;
}


bool is_all_reaching_definitions_in_this_loop(CForStatement *parent_stmt, LoadVariableExpression *load_var_expr){

    BrickAnnote *reaching_defs = to<BrickAnnote>(load_var_expr->lookup_annote_by_name("reaching_defs"));

    for(Iter<SuifBrick*> iter = reaching_defs->get_brick_iterator();
        iter.is_valid(); iter.next()){

        SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());

	if(!is_kind_of<Statement>(sob->get_object())){

	   SuifObject *so = (to<VariableSymbol>(sob->get_object()))->get_symbol_table(); 
           while(is_a<SymbolTable>(so = so->get_parent()));

     	   if(is_a<Statement>(so)){
	      if(!is_enclosing(parent_stmt, to<Statement>(so)))
    	         return 0;
	   }else if(is_a<ProcedureDefinition>(so))
 	      return 0;

	   continue;
	}

        Statement *def_stmt = to<Statement>(sob->get_object());
        if(!is_enclosing(parent_stmt, def_stmt))
           return 0;
    }

    return 1;
}


bool is_all_reached_uses_in_this_loop(CForStatement *parent_stmt, StoreVariableStatement *store_var_stmt){

    BrickAnnote *reached_uses = to<BrickAnnote>(store_var_stmt->lookup_annote_by_name("reached_uses"));

    for(Iter<SuifBrick*> iter = reached_uses->get_brick_iterator();
        iter.is_valid(); iter.next()){
        SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
        LoadVariableExpression *use_expr = to<LoadVariableExpression>(sob->get_object());

        if(!is_enclosing(parent_stmt, get_expression_owner(use_expr)))
           return 0;
    }

    return 1;
}

bool is_all_reached_uses_in_this_loop(CForStatement *parent_stmt, CallStatement *call_stmt){

    BrickAnnote *reached_uses = to<BrickAnnote>(call_stmt->lookup_annote_by_name("reached_uses"));

    for(Iter<SuifBrick*> iter = reached_uses->get_brick_iterator();
        iter.is_valid(); iter.next()){
        SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
        LoadVariableExpression *use_expr = to<LoadVariableExpression>(sob->get_object());

        if(!is_enclosing(parent_stmt, get_expression_owner(use_expr)))
           return 0;
    }

    return 1;
}



