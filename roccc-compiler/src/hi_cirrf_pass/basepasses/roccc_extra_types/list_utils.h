// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef LIST_UTILS_H
#define LIST_UTILS_H

/**************************** Declarations ************************************/

/************************** Implementations ***********************************/

template<class T>
void push_back_items_into_list(list<T> *items, list<T> *l){
  typename list<T>::iterator iter = items->begin();
  for (; iter != items->end(); iter++)
    l->push_back(*iter);
}

template<class T>
list<T>* clone_list(list<T> *l){

   list<T> *l_new = new list<T>;
   for(typename list<T>::iterator iter = l->begin();
       iter != l->end(); iter++)
       l_new->push_back(*iter);
   
   return l_new;
}

template<class T>
void remove_from_list(T item, list<T> *l){
  typename list<T>::iterator iter = l->begin();
  for( ; iter != l->end(); )
      if((*iter) == item)
         iter = l->erase(iter);
      else iter++;
}


template<class T>
void remove_items_from_list(list<T> *items, list<T> *l){
  typename list<T>::iterator iter = items->begin();
  for (; iter != items->end(); iter++)
    remove_from_list((*iter), l);
}


template<class T>
bool is_in_list(T item, list<T> *l){
  typename list<T>::iterator iter = l->begin();
  for (; iter != l->end(); iter++)
    if ((*iter) == item)
      return true;

  return false;
}


template<class T>
bool is_items_in_list(list<T> *items, list<T> *l){
  typename list<T>::iterator iter = items->begin();
  for (; iter != items->end(); iter++)
    if (!is_in_list(*iter, l))
      return false;
  
  return true;
}


template<class T>
bool is_lists_the_same(list<T> *a, list<T> *b){
  
   return is_items_in_list(a,b) && is_items_in_list(b,a);
}

template<class T>
T* get_element_at_index_in_list(int index, list<T> *l){

   if ((unsigned)index < 0 || (unsigned)index >= l->size())
       return NULL;

   typename list<T>::iterator iter = l->get_nth(index);

   if (iter != l->end())
       return &(*iter);

   return NULL;
}

template<class T>
int get_index_of_element_in_list(T item, list<T> *l){

   int index = 0;
   typename list<T>::iterator iter = l->begin();
   for (; iter != l->end(); iter++, index++)
     if ((*iter) == item)
       return index;
   
   return -1;
}

// computes a union b, then assigns the result to A
template<class T>
void union_lists(list<T> *A, list<T> *B){
  typename list<T>::iterator iter = B->begin();
  for (; iter != B->end(); iter++)
    if(!is_in_list(*iter, A))
      A->push_back(*iter);
  
}

// computes a intersect b, then assigns the result to A
template<class T>
void intersect_lists(list<T> *A, list<T> *B){
  typename list<T>::iterator iter = A->begin();
  for (; iter != A->end(); )
    if(!is_in_list(*iter, B))
      iter = A->erase(iter);
    else iter++;

}

// computes a-b, then assigns the result to a new BrickAnnote
template<class T>
list<T>* subtract_lists(list<T> *A, list<T> *B){

   list<T> *C = new list<T>;
   typename list<T>::iterator iter = A->begin();
   for (; iter != A->end(); iter++)
     if(!is_in_list(*iter, B))
       C->push_back(*iter);
   
   return C;
}

#endif

