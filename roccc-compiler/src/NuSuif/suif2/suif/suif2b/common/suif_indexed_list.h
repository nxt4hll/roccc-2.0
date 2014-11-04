#ifndef __SUIF_INDEXED_LIST_H__
#define __SUIF_INDEXED_LIST_H__

#include "suif_list.h"

/**	@file
 *	A searchable list (not hashed) template
 *
 *	This file contains two lists:
 *	An indexed list contains pairs - a key and a value
 *	A searchable list just contains entries with a comparison defined
 */

/**	\class indexed_list suif_indexed_list.h common/suif_indexed_list.h
 *
 *	A list class which contains pairs of keys and values. The
 *	values are ordered as a list unlike suif_hash_map.
 *	\warning lookup is a serial search
 *	\see suif_hash_map
 */
template <class Domain,class Range> class indexed_list
    {
    public:
	class pair
	    {
	  	friend class indexed_list<Domain,Range>;
	    public:
		Domain first;
		Range second;
		pair(const Domain &key,const Range &value) : first(key),second(value) {}
        	static void* constructorFunction(void* address) {
                    return new (address) pair;
                    }

	    protected:
		pair() {}
	    };

	typedef list<pair> pair_list;
	typedef typename pair_list::iterator iterator;
	typedef typename pair_list::const_iterator const_iterator;
        //typedef const iterator const_iterator;
	typedef pair value_type;

	/**	push to end of list */
	void push_back(const Domain &key,const Range &value)
	    {
	    the_list.push_back(pair(key,value));
	    }

	/**	return a reference to the last element
  	 *	\warning behavior is undefined on an empty list
	 */
	pair &back() {return the_list.back();}

	/**	Insert after the iterator position */
	iterator insert(const iterator& pos, const pair& x)
	    {
	    return the_list.insert(pos,x);
	    }

	/**	Make the list empty */
	void clear_list() {
	    the_list.clear_list();
	    }

	/**	Find the entry with the given key. Returns end()
	 *	if none */
	iterator find(const Domain &key) {
	    iterator iter = the_list.begin();
	    while (iter != the_list.end())
		{
		if ((*iter).first == key)
		    break;
		iter ++;
		}
	    return iter;;
	    }
	const_iterator find(const Domain &key) const {
	    const_iterator iter = the_list.begin();
	    while (iter != the_list.end())
		{
		if ((*iter).first == key)
		    break;
		iter ++;
		}
	    return iter;;
	    }

	/**	Find the number of entries with the given key */
	int num_with_key(const Domain &key) {
	    int count = 0;
            iterator iter = the_list.begin();
            while (iter != the_list.end())
                {
                if ((*iter).first == key)
                    count ++;
                iter ++;
                }
            return count;
            }

	/**	Find the nth entry with a given key. 
	 *	\warning The first entry is 1, not zero
	 */
        iterator find(const Domain &key,int no) {
            iterator iter = the_list.begin();
            while (iter != the_list.end())
                {
                if ((*iter).first == key)
		    {
		    no --;
		    if (no <= 0)
                        break;
		    }
                iter ++;
                }
            return iter;;
            }
        const_iterator find(const Domain &key,int no) const {
            const_iterator iter = the_list.begin();
            while (iter != the_list.end())
                {
                if ((*iter).first == key)
		    {
		    no --;
		    if (no <= 0)
                        break;
		    }
                iter ++;
                }
            return iter;;
            }

	/**	Return true if there is an entry with the given key */
	bool is_member(const Domain &key) const {
	    return find(key) != the_list.end();
	    }

	/**	Remove the value identified by the iterator */
	Range remove(iterator iter) {
	    Range x = (*iter).second;
	    the_list.erase(iter);
	    return x;
	    }

	/**	Remove the first entry which matches the key. Return
	 *	true if one was found */
	bool remove(Domain &key) {
	    iterator iter = find(key);
	    if (iter == the_list.end())
		return false;
	    the_list.erase(iter);
	    return true;
	    }

	iterator begin() { return the_list.begin();}
        const_iterator begin() const { return the_list.begin();}
        iterator end() { return the_list.end();}
        const_iterator end() const { return the_list.end();}

	/**	Return number of entries */
        unsigned length() const { return the_list.length(); }
	/**     Return number of entries */
        unsigned size() const { return the_list.size(); }

	/**	Index operator
	 *	\warning - no bound check done
	 */
        pair & operator [](int i) const {
                return the_list[i];
                }



    private:
	pair_list the_list;
    };

/**	\class searchable_list
 *	A list of elements which have equality defined
 */

template <class Domain> class searchable_list : public list<Domain>
    {
    public:
        typedef typename list<Domain>::iterator iterator;
	typedef typename list<Domain>::const_iterator const_iterator;
	/*	Find an entry in the list */
        iterator find(const Domain &key) {
            iterator iter = this->begin();
            while (iter != this->end())
                {
                if ((*iter) == key)
                    break;
                iter ++;
                }
            return iter;;
            }




        const_iterator find(const Domain &key) const {
	  const_iterator iter = this->begin();
	  while (iter != this->end())
                {
                if ((*iter) == key)
                    break;
                iter ++;
                }
            return iter;;
            }

	/**	Is there an entry with the given value */
        bool is_member(const Domain &key) const {
            return find(key) != this->end();
            }

	/**	Remove first entry with given value */
        bool remove(const Domain &key) {
            iterator iter = find(key);
            if (iter == this->end())
                return false;
            erase(iter);
            return true;
            }

    };
#endif
