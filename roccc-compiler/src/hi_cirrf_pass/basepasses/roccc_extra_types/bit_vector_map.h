// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/*

*/

#ifndef BIT_VECTOR_MAP_H
#define BIT_VECTOR_MAP_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "common/suif_map.h"

class BitVectorMap : public SuifObject 
{
public:
  BitVectorMap();
  ~BitVectorMap();

  void add(SuifObject *so);
  void remove_from_kill_set_map(SuifObject *so);
  int lookup(SuifObject *so);
  SuifObject* reverse_lookup(int i);
  list<int>* get_kill_set(SuifObject* so);
  void empty();
  int size(); 

protected:
  int counter;
  suif_map<SuifObject*, int>* bit_vector_map;
  suif_map<int, SuifObject*>* reverse_map;
  suif_map<VariableSymbol*, list<int>*>* kill_set_map;
};

#endif

