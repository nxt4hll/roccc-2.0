// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include <cassert>

#include "bit_vector_map.h"
#include "list_utils.h"

BitVectorMap::BitVectorMap()
{ 
  bit_vector_map = new suif_map<SuifObject*, int>; 
  reverse_map = new suif_map<int, SuifObject*>; 
  kill_set_map = new suif_map<VariableSymbol*, list<int>*>;
  counter = 0;
}

BitVectorMap::~BitVectorMap()
{
  if (bit_vector_map != NULL)
  {
    bit_vector_map->clear();
    delete bit_vector_map; 
  }

  if (reverse_map != NULL)
  {
    reverse_map->clear(); 
    delete reverse_map;
  }

  if (kill_set_map != NULL)
  {
    for(suif_map<VariableSymbol*, list<int>*>::iterator iter = kill_set_map->begin();
	iter != kill_set_map->end(); iter++)
    {
      (*iter).second->empty();
      delete (*iter).second;
    }
    kill_set_map->clear();
    delete kill_set_map;
  }
}

void BitVectorMap::add(SuifObject *so)
{
  bit_vector_map->enter_value(so, counter); 
  reverse_map->enter_value(counter, so);  

  VariableSymbol* stmt_destination = NULL;

  if(is_a<StoreVariableStatement>(so))
    stmt_destination = (to<StoreVariableStatement>(so))->get_destination();
  else if(is_a<CallStatement>(so))
    stmt_destination = (to<CallStatement>(so))->get_destination();
  else if(is_a<VariableSymbol>(so))
    stmt_destination = to<VariableSymbol>(so);
  
  if(kill_set_map->find(stmt_destination) != kill_set_map->end()){
    list<int>* kill_set_list = kill_set_map->lookup(stmt_destination);
    kill_set_list->push_back(counter);
  }else{
    list<int>* kill_set_list = new list<int>;
    kill_set_list->push_back(counter);
    kill_set_map->enter_value(stmt_destination, kill_set_list);
  }
  
  counter++;

}

void BitVectorMap::remove_from_kill_set_map(SuifObject *so){ 
   
   VariableSymbol* stmt_destination = NULL;

   if(is_a<StoreVariableStatement>(so))
      stmt_destination = (to<StoreVariableStatement>(so))->get_destination();
   else if(is_a<CallStatement>(so))
      stmt_destination = (to<CallStatement>(so))->get_destination();
   else if(is_a<VariableSymbol>(so))
      stmt_destination = to<VariableSymbol>(so);

   if(kill_set_map->find(stmt_destination) != kill_set_map->end()){
      list<int>* kill_set_list = kill_set_map->lookup(stmt_destination);
      remove_from_list(bit_vector_map->lookup(so), kill_set_list);
   }

}

int BitVectorMap::lookup(SuifObject *so){ 
   
   return bit_vector_map->lookup(so); 

}

SuifObject* BitVectorMap::reverse_lookup(int i){ 
   
   return reverse_map->lookup(i); 

}

list<int>* BitVectorMap::get_kill_set(SuifObject *so){

   VariableSymbol* stmt_destination = NULL;

   if(is_a<StoreVariableStatement>(so))
      stmt_destination = (to<StoreVariableStatement>(so))->get_destination();
   else if(is_a<CallStatement>(so))
      stmt_destination = (to<CallStatement>(so))->get_destination();
   else if(is_a<VariableSymbol>(so))
      stmt_destination = to<VariableSymbol>(so);

   if(kill_set_map->find(stmt_destination) != kill_set_map->end())
      return kill_set_map->lookup(stmt_destination);

   return NULL;
}

void BitVectorMap::empty(){ 

   bit_vector_map->clear();
   reverse_map->clear(); 

   for(suif_map<VariableSymbol*, list<int>*>::iterator iter = kill_set_map->begin();
       iter != kill_set_map->end(); iter++){
       (*iter).second->empty();
       delete (*iter).second;
   }

   kill_set_map->clear();
   counter = 0;

}

int BitVectorMap::size(){ 

   return counter; 

}


