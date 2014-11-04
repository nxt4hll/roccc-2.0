#ifndef _LIST_H_
#define _LIST_H_

/**	@file
 *	A list class simpler but similar to STL list class
 */

#if defined(USE_STL) || defined(USE_STL_LIST)
#include <list>
#else

/**	\class list suif_list.h common/suif_list.h
 *	This is largely compatible with the STL list class, but simpler.
 *	It should be possible to replace it with the STL version, if
 *	your compiler is capable of compiling STL code. The motivation
 *	for this class is to support compilers that cannot handle STL.
 */
template <class T>
class list {
public:
	typedef T value_type;

        class Node {
            public:
               Node *pvPrev;
               T data;
               Node *pvNext;
               Node(const T& x) : pvPrev(0),data(x),pvNext(0) {}
	    private:
	       Node(const Node&) : pvPrev(0),data(0),pvNext(0) {}
	       Node &operator=(const Node &other) {
		 pvPrev = other.pvPrev; data= other.data;
		 pvNext = other.pvNext; return(*this);
	       }
               };

        class const_literator;
		class literator;

	class literator {
		friend class list<T>;
		friend class const_literator;
	protected:
		Node *node;
	public:
		literator& operator--() {
			node = node->pvPrev;
			return *this;
		}
		literator operator--(int dummy) {
			literator iter(*this);
			node = node->pvPrev;
			return iter;
		}
		literator operator++(int dummy) {
			literator iter(*this);
			node = node->pvNext;
			return iter;
		}
		literator& operator++() {
			node = node->pvNext;
			return *this;
		}
		T& operator*() const { return node->data; }	
		bool operator==(const literator& iter) const {
			return (node == iter.node);
		}
		bool operator!=(const literator& iter) const {
			return (node != iter.node);
		}
		literator &operator =(const literator &x) {node = x.node;return *this;}
		literator(const literator &x) : node(x.node) {}
		literator() : node(0) {}
	protected:
		literator &operator =(Node *x) {node = x;return *this;}
		literator(Node *x) : node(x) {}
	};
        typedef literator iterator;

	class const_literator {
		friend class list<T>;
	protected:
		Node *node;
	public:
		const_literator& operator--() {
			node = node->pvPrev;
			return *this;
		}
		const_literator operator--(int dummy) {
			const_literator iter(*this);
			node = node->pvPrev;
			return iter;
		}
		const_literator operator++(int dummy) {
			const_literator iter(*this);
			node = node->pvNext;
			return iter;
		}
		const_literator& operator++() {
			node = node->pvNext;
			return *this;
		}
		const T& operator*() const { return node->data; }	
		bool operator==(const const_literator& iter) const {
			return (node == iter.node);
		}
		bool operator!=(const const_literator& iter) const {
			return (node != iter.node);
		}
		const_literator &operator =(const const_literator &x) {node = x.node;return *this;}
		const_literator(const const_literator &x) : node(x.node) {}
		const_literator(const literator &x) : node(x.node) {}
		const_literator() : node(0) {}
	protected:
		const_literator &operator =(Node *x) {node = x;return *this;}
		const_literator(Node *x) : node(x) {}
	};
        typedef const_literator const_iterator;

protected:
        unsigned m_nLength;

	void clone_list(const list &x) {
	    for (iterator i=x.begin();i != x.end();i++)
		push_back(*i);
	    }
	
	void init_list() {
	    m_begin = (Node *)0;
	    m_end = (Node *)0;
            m_nLength=0;
	    }
	
protected:
	iterator m_begin;
	iterator m_end;

public:
	list() :
	  m_nLength(0), m_begin(), m_end()
                {
		init_list();
		}

	~list() {
		clear_list();
		}

	/**	Empty the list */
        void clear_list() {
            while (m_nLength > 0)
                pop_front();
	    init_list();
            }

	/**	is the list empty */
	bool empty() const { return m_nLength == 0; }

	/**	Number of elements in list */
	unsigned length() const { return m_nLength; }

	/**	Number of elements in list */
	unsigned size() const { return m_nLength; }

	/**	Insert an element into the list at the iterator position */	
	iterator insert(const iterator& pos, const T& x) { // add x before pos
		Node *new_node = new Node(x);
		new_node->pvNext = pos.node;
		if (pos.node != 0)
		    {
		    new_node->pvPrev = pos.node->pvPrev;
		    pos.node->pvPrev = new_node;
		    }
		else
		    {
		    new_node->pvPrev = m_end.node;
		    m_end = new_node;
		    }
		if (new_node->pvPrev != 0)
		    new_node->pvPrev->pvNext = new_node;
		else
		    m_begin = new_node;

		m_nLength++;
		return iterator(new_node);
		}

	/**	Get the nth entry as an iterator.
	 *	Returns end() if not found
	 *	\warning 0 is first element !
  	 *	values < 0 return first element too.
	 */
	iterator get_nth(int pos) const {
	    iterator x = begin();
            while ((x != end()) && (pos > 0))
                {
                x++;
                pos--;
                }
	    return x;
	    }


	/**	Insert before a given position
	 *	(0 for front of list)
	 */
        iterator insert(int pos,const T& y) {
            iterator x = get_nth(pos);
            return insert(x,y);
            }

	iterator begin() { return m_begin; }
	iterator begin() const { return m_begin; }
	iterator end() { return iterator(0);}
	iterator end() const { return iterator(0);}

	/**	Push to front of list*/
	void push_front(const T& x) { insert(begin(), x); }

	/**	Pop from front of list */
	void pop_front() {
		erase(begin());
		}

	/**	Push to back of list */
	void push_back(const T& x) { insert(end(), x); }

	/**	Pop from back of list */
	void pop_back() {
		erase(m_end);
		}

	/**	Get reference to first element */	
	T & front() { return m_begin.node->data; }

	/**	Get reference to last element */
	T & back() { return m_end.node->data; }

	/**	Index operation (slow) */
	T & operator [](int i) const {
	    iterator x = get_nth(i);
	    return x.node->data;
	    }

	/**	Remove the item at the given iterator position
	 *	\warning - the parameter iterator is invalid 
	 *	upon return - use the returned iterator instead */
	iterator erase(const iterator& pos) {
	        iterator ret_iter = pos;
	        Node *the_node = pos.node;
		// shouldn't this be an assertion???
		if (!the_node) {
		    return(ret_iter);
		    }
		ret_iter++;
		if (the_node->pvPrev)
		    (the_node->pvPrev->pvNext) = (the_node->pvNext);
		else
		    m_begin = (the_node->pvNext);
		if (the_node->pvNext)
		    (the_node->pvNext->pvPrev) = (the_node->pvPrev);
		else
		  m_end = (the_node->pvPrev);
		delete (the_node);
		m_nLength--;
		return(ret_iter);
		}

        /**	Erase item at pos and return iterator pointing at
	 *	following item */
        iterator erase(int pos) {
            iterator x = get_nth(pos);
            return(erase(x));
            }

	list & operator =(const list &x) {
	    clear_list();
	    clone_list(x);
	    return *this;
	    }

	list(const list &x) {
	    init_list();
	    clone_list(x);
	    }
		
private:


}; // class list


#endif
#endif

