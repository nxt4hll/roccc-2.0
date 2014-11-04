// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "bit_vector.h"
#include "math.h"
#include "iostream"

using namespace std;

BitVector::BitVector(BitVectorMap *bvm){ 

   bv_map = bvm;
   bv_value = new suif_vector<int>;
}

BitVector::~BitVector(){ 

   bv_map = NULL;
   bv_value->clear();
   delete bv_value; 
}

void BitVector::reset(){ 

  bv_value->clear();
  int bv_size = (int)ceil(bv_map->size() / 32.0); 
  for(int i = 0; i < bv_size; i++)
      bv_value->push_back(0);
}

BitVector* BitVector::clone(){

  BitVector *new_bv = new BitVector(bv_map);
  new_bv->reset();

  suif_vector<int>* new_bv_value = new_bv->get_bit_vector_value();
  for(int i=0; i< bv_value->size(); i++)
      new_bv_value->at(i) = bv_value->at(i);

  return new_bv;
}

void BitVector::mark(SuifObject *loc){

  int bit_loc = bv_map->lookup(loc);

  int vector_index = (int)floor(bit_loc / 32.0);
  int bit_index = bit_loc % 32;

  int mask = 1 << bit_index;

  bv_value->at(vector_index) = bv_value->at(vector_index) | mask;
}

void BitVector::mark(int loc){

  int vector_index = (int)floor(loc / 32.0);
  int bit_index = loc % 32;

  int mask = 1 << bit_index;

  bv_value->at(vector_index) = bv_value->at(vector_index) | mask;

}

void BitVector::unmark(SuifObject *loc){

  int bit_loc = bv_map->lookup(loc);

  int vector_index = (int)floor(bit_loc / 32.0);
  int bit_index = bit_loc % 32;

  int mask = 0xFFFFFFFF ^ (1 << bit_index);

  bv_value->at(vector_index)= bv_value->at(vector_index) & mask;
}

void BitVector::unmark(int loc){

  int vector_index = (int)floor(loc / 32.0);
  int bit_index = loc % 32;

  int mask = 0xFFFFFFFF ^ (1 << bit_index);

  bv_value->at(vector_index) = bv_value->at(vector_index) & mask;

}

bool BitVector::is_marked(SuifObject *loc){

  int bit_loc = bv_map->lookup(loc);

  int vector_index = (int)floor(bit_loc / 32.0);
  int bit_index = bit_loc % 32;

  int mask = 1 << bit_index;

  unsigned int result = bv_value->at(vector_index) & mask;

  return result > 0;
}


bool BitVector::is_marked(int loc){

  int vector_index = (int)floor(loc / 32.0);
  int bit_index = loc % 32;

  int mask = 1 << bit_index;

  unsigned int result = bv_value->at(vector_index) & mask;

  return result > 0;
}

void BitVector::intersect(BitVector *a_bv){

  suif_vector<int>* a_bv_value = a_bv->get_bit_vector_value();

  for(int i=0; i< bv_value->size(); i++)
      bv_value->at(i) = bv_value->at(i) & a_bv_value->at(i);
}

BitVector* BitVector::subtract(BitVector *a_bv){

  suif_vector<int>* a_bv_value = a_bv->get_bit_vector_value();

  BitVector *new_bv = new BitVector(bv_map);
  new_bv->reset();
  suif_vector<int>* new_bv_value = new_bv->get_bit_vector_value();

  for(int i=0; i < bv_value->size(); i++)
      new_bv_value->at(i) = bv_value->at(i) & (~(a_bv_value->at(i))); 

  return new_bv;
}

void BitVector::subtract(BitVector *a_bv, BitVector *b_bv){

  suif_vector<int>* a_bv_value = a_bv->get_bit_vector_value();
  suif_vector<int>* b_bv_value = b_bv->get_bit_vector_value();

  for(int i=0; i < a_bv_value->size(); i++)
      bv_value->at(i) = a_bv_value->at(i) & (~(b_bv_value->at(i))); 

}

void BitVector::subtract_n_overwrite(BitVector *a_bv){

  suif_vector<int>* a_bv_value = a_bv->get_bit_vector_value();

  for(int i=0; i< bv_value->size(); i++)
      bv_value->at(i) = bv_value->at(i) & (~(a_bv_value->at(i)));
       
}

void BitVector::union_(BitVector *a_bv){

  suif_vector<int>* a_bv_value = a_bv->get_bit_vector_value();

  for(int i=0; i< bv_value->size(); i++)
      bv_value->at(i) = bv_value->at(i) | a_bv_value->at(i);

  if(a_bv_value->size() > bv_value->size()){
     for(int i= bv_value->size(); i < a_bv_value->size(); i++)
         bv_value->push_back(a_bv_value->at(i));
  }

}

void BitVector::copy(BitVector *a_bv){

  suif_vector<int>* a_bv_value = a_bv->get_bit_vector_value();

  bv_map = a_bv->get_bit_vector_map();
  for(int i=0; i< bv_value->size(); i++)
      bv_value->at(i) = a_bv_value->at(i);

}

suif_vector<int>* BitVector::get_bit_vector_value(){

   return bv_value;
}

BitVectorMap* BitVector::get_bit_vector_map(){

   return bv_map;
}
  
void BitVector::set_bit_vector_value(suif_vector<int>* bv_val){

   bv_value = bv_val;
}   

void BitVector::set_bit_vector_map(BitVectorMap *bvm){

   bv_map = bvm;
}

String BitVector::to_string(){

   String output = "";

   for(int i = bv_value->size()-1; i>=0; i=i-1){
       int num = bv_value->at(i);
       for(int i=32; i>=0; i--) {
           int bit = ((num >> i) & 1);
           output += String(bit);
       }
       output += " ";
   }

   return output;
}
