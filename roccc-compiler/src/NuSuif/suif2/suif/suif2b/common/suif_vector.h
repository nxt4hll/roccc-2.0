#ifndef _VECTOR_H_
#define _VECTOR_H_

/**	@file
 *	A vector template similar to but simpler than the STL vector class
 */



#if defined(USE_STL) || defined(USE_STL_VECTOR)
#include <vector>
#define suif_vector vector
#else

#include <assert.h>

/**	\class suif_vector suif_vector.h common/suif_vector.h
 *
 *	A vector class along the lines of the STL vector class but
 *	simpler. The intent is to support compilers which are unable
 *	to compile STL
 *	A vector is a list with a fast index operation but potentially
 *	slower insert and slower insert/erase at anywhere but end
 */
template <class T>
class suif_vector {
public:
	typedef T VectorType;
	typedef T value_type;
	typedef T* iterator;
	typedef const T* const_iterator;

protected:
	T *buff;

protected:

	iterator m_start;
	iterator m_finish;
	iterator m_end_of_storage;
protected:
	void allocate_vector(unsigned n) {
	        if (n == 0) n++;
		buff = new T[n];
		m_end_of_storage = buff + n;
		}	
	void vector_fill(iterator& start, unsigned n, const T& val) {
		iterator tmp = start;
		while (tmp < m_finish) {
		    *(tmp) = val;
		    tmp++;		
		    }				
		}	
	// copy from start up to, but not including, end...to dest
	iterator copy(const_iterator start, const_iterator end, iterator dest) {
		for (const_iterator tmp = start;
		     tmp != end; tmp++, dest++) {
		  *dest = *tmp;
		  }
		return dest;
		}
	// What is the return value here?
	// It seems like it should always end up just before start.
	// Shouldn't it return dest2 instead?
	const_iterator reverse_copy(const_iterator start, const_iterator end, 
				    iterator dest) {
		iterator dest2 = dest + (end - start) -1;
		const_iterator tmp = end -1;
		while (tmp >= start) {
		    *dest2-- = *tmp--;
		    }
		return tmp;
		}

	void insert_aux(iterator pos, const T& x) {
		if (m_finish != m_end_of_storage) {
		    reverse_copy(pos, end(), pos + 1);
		    *pos = x;
		    m_finish = end() + 1;
		    }
		else {
		    int oldsize = size();
		    int newsize = 2 * oldsize + 1;
		    T *pOld = buff;
		    allocate_vector(newsize);
		    T *pPos = buff;
		    pPos = copy(pOld, pos, pPos);
		    m_finish = copy(pos, end(), pPos+1); 	
		    *pPos = x;	
		    delete [] pOld;
		    m_start = buff;
		    }
		}
public:
	suif_vector() : buff(0),m_start(0),m_finish(0),m_end_of_storage(0) {
		allocate_vector(1);
		m_start = buff;
		m_finish = buff;
		}

	/**	Create a vector
	 *	@param	Size to create
	 *	@param  Default value for elements 
	 */
	suif_vector(unsigned n, const T& val = T()) :
	  buff(0),m_start(0),m_finish(0),m_end_of_storage(0) {
		iterator tmp;
		allocate_vector(n);
		m_start = buff;
		tmp = m_start;
		m_finish = tmp + n;	
		vector_fill(m_start, n, val);
		}

	suif_vector(const suif_vector<T>& vec) :
	  buff(0),m_start(0),m_finish(0),m_end_of_storage(0) {
		const_iterator it = vec.begin(), end = vec.end();
		unsigned n = vec.size();
		delete [] buff;
		allocate_vector(n);
		copy(it, end, buff);
		m_start = buff;
		m_finish = buff + n;
		}

        ~suif_vector() {
           delete [] buff;
           }
	suif_vector<T>& operator=(const suif_vector<T>& vec) {
		const_iterator it = vec.begin(), end = vec.end();
		unsigned n = vec.size();
		delete [] buff;
		allocate_vector(n);
		copy(it, end, buff);
		m_start = buff;
		m_finish = buff + n;
		return *this;
		}

	iterator begin() { return m_start; }
	iterator end() { return m_finish; }

	const_iterator begin() const { return m_start; }
	const_iterator end() const { return m_finish; }
	/**	return number of elements */
	inline unsigned length() const { return m_finish - m_start; }
	/**     return number of elements */
	inline unsigned size() const { return m_finish - m_start; }

	/**	Is the vector empty */
	inline bool empty() const { return size() == 0; }	

	/**	Return available entries */
	unsigned capacity() const {
		return ((unsigned)(m_end_of_storage - m_start));
		}

	/**	Insert a value at the given position*/
	void insert(iterator pos, const T& x) {
		if (m_finish != m_end_of_storage && pos == end()) {
		    *m_finish = x;
		    m_finish++;
		    }
		else {
		    insert_aux(pos, x);
		    }	
		}

	/**	Insert at the given position */
	void insert(int pos,const T& y) {
		if (pos < 0)
		     insert(begin(),y);
		else if (pos >= (int)size())
		    push_back(y);
		else
		    insert(begin() + pos,y);
		}
        /**	Remove an element and return position of following (or end() ) */
	iterator erase(iterator pos) {
		iterator tmp;
		tmp = pos + 1;
		if (tmp != end()) {
		    copy(tmp, end(), pos);
		    --m_finish;
		    }
		else {
		    --m_finish;
		    }
		return(tmp);
		}

	/**	Remove entry in given position */
	void erase(unsigned pos) {
		if (pos < 0)
		    pos = 0;
		else if (pos >= size())
		    pos = size() - 1;
		if (pos >= 0)
		    erase(begin() + pos);
		}

	void push_back(const T& x) {
		if (m_finish != m_end_of_storage) {
		    *m_finish = x;
		    m_finish++;
		    }
		else {
		    insert_aux(end(), x);
		    }
		}

	void pop_back() { m_finish--;  }

	/**	Index operator */
	T& operator[](unsigned n) {
	        assert ( n < size());
		return buff[n];
		}

	const T& operator[](unsigned n) const {
	        assert ( n < size());
		return buff[n];
		}

	T& at(unsigned n) const {
		return buff[n];
		}

	T& front() {
		return *begin();
		}

	T& back() {
		return *(end()-1);
		}

	const T& front() const {
		return *begin();
		}

	const T& back() const {
		return *(end()-1);
		}

	/**	Find an item in the list
	 *	This forces items to have equality defined.
	 *	This should probably be removed */
	bool is_member(const T& x) const {
	  for (const_iterator it = begin(); it != end(); it++) {
	    if ((*it) == x) return true;
	  }
	  return false;
	}

	/**	Empty the list */
        void clear(void) {
          m_finish = buff;
        }
private:


}; // class vector

#endif /* USE_STL_VECTOR */

#endif

