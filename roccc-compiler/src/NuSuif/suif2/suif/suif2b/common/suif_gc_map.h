#ifndef _GC_MAP_H_
#define _GC_MAP_H_

#include <assert.h>
#include "suif_gc_obj.h"

/**	@file
 *	a hash table lokkup template
 */

/**	\class suif_gc_map suif_gc_map.h common/suif_gc_map.h
 *
 *	A map function based on a list and sequential search.
 *	\see suif_hash_map
 */
class suif_gc_map_inner : public SuifGcObj
    {

    private:
	void dup_list(const suif_gc_map_inner &x);
    public:
	class pair_inner : public SuifGcObj {
          public:
	    pair_inner *next;
	    };

  	class helper_inner : public SuifGcObj {
	  public:
	    virtual pair_inner* clone(const pair_inner &) const = 0;
            virtual void set_range(const pair_inner *val,pair_inner *x) = 0;
            };


	class key_inner : public SuifGcObj
	    {
	    public:
	 	virtual bool operator == (pair_inner *p) const =0;
	    };

	class iterator_inner : public SuifGcObj {
	    const suif_gc_map_inner *suif_gc_map;
	    pair_inner *current;
	  public:
	    bool operator ==(const iterator_inner &x) const {return current == x.current;}
	    bool operator !=(const iterator_inner &x) const {return current != x.current;}
	    pair_inner *get() const {return current;}
	    iterator_inner & operator ++();
	    iterator_inner operator ++(int dummy);
	    iterator_inner &operator --();
	    iterator_inner operator --(int dummy);
	    iterator_inner(const suif_gc_map_inner *x,pair_inner *t) : suif_gc_map(x),current(t) {}
	    iterator_inner() : suif_gc_map(0),current(0) {}
	    pair_inner *get_current() {return current;}
	  public:
	    iterator_inner(const iterator_inner &other) :
	      suif_gc_map(other.suif_gc_map), current(other.current) { }
	  iterator_inner &operator=(const iterator_inner &other) {
            suif_gc_map = other.suif_gc_map;
	    current = other.current;
	    return(*this);
	  }
	  };

	suif_gc_map_inner(helper_inner &x);

	iterator_inner insert(iterator_inner &x,const pair_inner &p);
	iterator_inner find(const key_inner &x) const;
	void erase(iterator_inner &x);
	pair_inner* enter_value(const key_inner &x,const pair_inner &y);
	pair_inner* enter_value_no_change(const key_inner &x,const pair_inner &y);

	iterator_inner begin() const {return iterator_inner(this,table);}
	iterator_inner end() const {
	  return iterator_inner(this,(pair_inner *)0);}
	iterator_inner insert(iterator_inner &x,pair_inner *y);

        virtual ~suif_gc_map_inner();

	suif_gc_map_inner(const suif_gc_map_inner &x);
	suif_gc_map_inner &operator =(const suif_gc_map_inner &x);


	pair_inner *table;
	helper_inner *help;
	int m_nSize;
	void clear();
    };


template <class domain, class range>
#ifndef MSVC
class suif_gc_map : private suif_gc_map_inner {
#else
class suif_gc_map : public suif_gc_map_inner {
#endif
	class key : public suif_gc_map_inner::key_inner {
                const domain &value;
            public:
                key(const domain &v) : value(v) {}
                virtual bool operator == (suif_gc_map_inner::pair_inner *p)const  
			{return ((pair *)p)->first == value;}
            };
    class helper : public suif_gc_map_inner::helper_inner {
          public:
            virtual pair_inner* clone(const pair_inner &x) const
		{
		pair *y = (pair *)&x;
		return new pair(y->first,y->second);
		}
            virtual void set_range(const pair_inner *val,pair_inner *x)
		{
		pair *yval = (pair *)val;
		pair *ref = (pair *)x;
		ref->second = yval->second;
		}
            };

	helper the_helper;


    public:
        class pair : public suif_gc_map_inner::pair_inner {
	    public:
	        domain first;
		range  second;
		pair & operator =(range &x) {second = x;return *this;}
		pair(domain x,range y) : pair_inner() , first(x),second(y) {}
		pair(domain x) : pair_inner() , first(x) {}
	    private:
	        pair(const pair &other) : first(other.first), second(other.second) {}
	        pair &operator=(const pair &other) 
			{ first = other.first; second= other.second; return (*this); }
	    };
	typedef pair value_type;
	typedef domain key_type;
	typedef range data_type;

    	class iterator : public suif_gc_map_inner::iterator_inner {
	    public:
		iterator(iterator_inner x) : iterator_inner(x) {}
		iterator() : iterator_inner() {}
		pair & operator *() const {return *(pair *)get();}
	    };

	suif_gc_map() : suif_gc_map_inner(the_helper), the_helper() {
	  SuifGcObj::register_object(this);
	}

        virtual ~suif_gc_map() {
	  SuifGcObj::unregister_object(this);
	}

	/**	Enter a value into the table with key x and value y */
	pair & enter_value(domain x,range y) {
          key k( x );
          pair p( x,y );
          return *(pair *)suif_gc_map_inner::enter_value( k,p );
          }
	/**	Find an entry by key. Returns end() if not found */
	iterator find(const domain &x) const {
	    return suif_gc_map_inner :: find(key(x));
	    }
	/**     Similar to find() except returning the associated value.
          *     assert if no value associated with the key in ths map.
	  */
	range   lookup(const domain &x) {
	    iterator iter = find(x);
	    assert(iter != end());
	    return (*iter).second;
	}

	/**	Get an iterator to iterate over the values.
	 */
        iterator begin() const {return iterator(suif_gc_map_inner :: begin());}
        iterator end() const {return iterator(suif_gc_map_inner :: end());}

	/**	Remove an entry. */
	void erase(iterator &iter) {suif_gc_map_inner::erase(iter);}
	/**	Return count of entries in table */
	unsigned size() const { return suif_gc_map_inner::m_nSize; }
	/**	Insert into a table at the given position.
	 */
	iterator insert(iterator &x,const pair &p) {return suif_gc_map_inner::insert(x,p);}

	/**     Clear all entries. */
	void clear(void) { suif_gc_map_inner::clear(); };

    };


#endif
