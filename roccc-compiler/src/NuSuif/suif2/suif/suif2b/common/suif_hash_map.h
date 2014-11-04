
#ifndef _HASH_MAP_H_
#define _HASH_MAP_H_

#include <stddef.h>
#include <assert.h>

/**	@file
 *	A hash map template
 */

/**     \class suif_hash_map suif_hash_map.h common/suif_hash_map.h
 *
 *      a map function based on a hash table
 *      \see suif_map
 */


class suif_hash_map_table;
// avoid nested class decs -some compilers cannot handle

class suif_hash_map_inner {
  void dup_table(const suif_hash_map_inner &x);
public:
  class pair_inner {
  public:
    pair_inner *next;
  }; // end class suif_hash_map_inner::pair_inner
  
  class helper_inner {
  public:
    virtual pair_inner* clone(const pair_inner *) const = 0;
    virtual void set_range(const pair_inner *val,pair_inner *x) = 0;
    virtual ~helper_inner(){}
  };// end class suif_hash_map_inner::helper_inner
  
  
  class key_inner {
  public:
    virtual bool operator == (pair_inner *p) const =0;
    virtual int key_hash() const =0;
    virtual ~key_inner(){}
  };// end class suif_hash_map_inner::key_inner
  
  class iterator_inner {
    const suif_hash_map_table *hash;
    pair_inner *current;
    int index;
    void advance();
    void retreat();
  public:
    bool operator ==(const iterator_inner &x) const {return current == x.current;}
    bool operator !=(const iterator_inner &x) const {return current != x.current;}
    pair_inner *get() const {return current;}
    iterator_inner & operator ++();
    iterator_inner operator ++(int dummy);
    iterator_inner & operator --();
    iterator_inner operator --(int dummy);
    
    iterator_inner(const suif_hash_map_table *x,pair_inner *t,int inx) : hash(x),current(t),index(inx) {}
    int get_index() {return index;}
    const suif_hash_map_table* get_table() {return hash;}
  public:
    iterator_inner(const iterator_inner &other) :
      hash(other.hash),current(other.current),index(other.index) {}
    
    iterator_inner() : hash(0), current(0), index(0) {}
    
    iterator_inner &operator=(const iterator_inner &other) {
      hash = other.hash;
      current = other.current;
      index = other.index;
      return(*this);
    }
  };// end class suif_hash_map_inner::iterator_inner
  
  suif_hash_map_inner(helper_inner &x,int table_size = 32);
  iterator_inner find(const key_inner &x) const;
  void erase(iterator_inner &x);
  pair_inner* enter_value(const key_inner &x,const pair_inner &y);
  pair_inner* enter_value_no_change(const key_inner &x,const pair_inner &y);
  
  iterator_inner begin() const;
  iterator_inner end() const {return iterator_inner(0,(pair_inner *)0,0);}
  
  virtual ~suif_hash_map_inner();
  
  void clear();
  
  suif_hash_map_inner &operator =(const suif_hash_map_inner &x);
  suif_hash_map_inner(const suif_hash_map_inner &x);
  unsigned size() const;
  
  suif_hash_map_table *top_table;
  helper_inner *help;
};//end class suif_hash_map_inner

template <class domain,class range>
// #ifndef MSVC
// class suif_hash_map :  private suif_hash_map_inner {
// #else
class suif_hash_map :  public suif_hash_map_inner {
  //#endif
  class key : public suif_hash_map_inner::key_inner {
    const domain &value;
  
  public:
    key(const domain &v) : value(v) {}

    bool operator == (suif_hash_map_inner::pair_inner *p) const {
      return ((pair *)p)->first == value;
    }
    
    int key_hash() const {
      /*return hash(value)*/
      return hash(value);/*jul modif*/
    }
  };//end class suif_hash_map::key
  
  class helper : public suif_hash_map_inner::helper_inner {
  public:
    virtual pair_inner* clone(const pair_inner *x) const {
      pair *y = (pair *)x;
      return new pair(y->first,y->second);
    }
    virtual void set_range(const pair_inner *val,pair_inner *x) {
      pair *yval = (pair *)val;
      pair *ref = (pair *)x;
      ref->second = yval->second;
    }
  };//end class suif_hash_map::helper
  
  helper the_helper;
  
  
public:
  
  class pair : public suif_hash_map_inner::pair_inner {
  public:
    domain first;
    range  second;
    pair & operator =(const range &x) {second = x;return *this;}
    pair(domain x,range y) : pair_inner() , first(x),second(y) {}
    pair(domain x) : pair_inner() , first(x) {}
  private:
    pair(const pair &other) :
      first(other.first), second(other.second) {}
    pair &operator=(const pair &other) {
      first = other.first; second = other.second;
      return(*this);
    }
    
  };//end class suif_hash_map::pair
  
  class literator : public suif_hash_map_inner::iterator_inner {
  public:
    literator(iterator_inner x) : iterator_inner(x) {}
    literator() {}
    pair & operator *() const{return *(pair *)get();}
  public:
    literator(const literator &other) :
      iterator_inner((const literator &)other)
    {}
  };//end class suif_hash_map::literator

  typedef literator iterator;
  typedef const literator const_iterator;

  /**     Enter a value into the table with key x and value y */
  pair& enter_value(domain x,range y) {
    pair *pPair;
    pair P(x,y);
    pPair = &P;
    pPair = (pair*)(suif_hash_map_inner :: enter_value(key(x),P));
    return *pPair;
  }
  
  /**     Find an entry by key. Returns end() if not found */
  iterator find(const domain &x) const {
    return suif_hash_map_inner :: find(key(x));
  }
  
  /**     Similar to find() except returning the associated value.
   *     assert if no value associated with the key in ths map.
   */
  range   lookup(const domain &x) {
    iterator iter = find(x);
    assert(iter != end());
    return (*iter).second;
  }
  
  /**     Get an iterator to iterate over the values.
   *	\warning {The order is not specified. In particular it
   *	cannot be expected that it will be the order in which
   *	the values were entered}
   */
  iterator begin() {return iterator(suif_hash_map_inner :: begin());}
  const_iterator begin() const {return iterator(suif_hash_map_inner :: begin());}
  iterator end() {return iterator(suif_hash_map_inner :: end());}
  const_iterator end() const {return iterator(suif_hash_map_inner :: end());}
  
  /**	Remove item from table */
  void erase(iterator &iter) {suif_hash_map_inner::erase(iter);}
  
  /**	Create hash table
   *	@param size the size of the hash table to use. 
   *	This will be rounded up to a power of 2
   */
  suif_hash_map(int size = 32) :
    suif_hash_map_inner(the_helper,size), the_helper() {}
  
  ~suif_hash_map() {}
  
  typedef pair value_type;
  typedef domain key_type;
  typedef range data_type;
  
  /**	Insert a value. The iterator value is ignored. */
  iterator insert(iterator &x,const pair &p) {
    enter_value(p.first, p.second);
    return(x);
  }
  /**	Return number of entries in table */
  unsigned size() const { return suif_hash_map_inner::size(); }
  
  /**     Clear the table. */
  void clear() { suif_hash_map_inner::clear(); }
  
};
  
  /* size_t hash( const void * a ); */
  //size_t _hash( void * a ); /* jul modif */
size_t hash( const void *a );
size_t hash( const unsigned int i );
  //size_t hash( unsigned int i );
  //  size_t hashString( String x) ;
  
#endif
