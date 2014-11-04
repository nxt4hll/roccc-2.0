#ifndef __REF_COUNTER_H__
#define __REF_COUNTER_H__

#include "simple_stack.h"

/**	@file
 *	A reference counter pointer type and associated templates 
 */

/**	A simple ref_counter class.
 *	Use this to create a pointer type.
 *
 *	The type we are reference counting is not hidden, as it should be
 *	because that makes derivation harder and we want to avoid nested
 *	classes since that is not well supported by some C++ compilers
 */
template <class T> class RCPointer {
    	T *_x;
    public:
	RCPointer() : _x(NULL) {}
	RCPointer(T * x) : _x(x) {
		if (_x) {
		    _x->inc_refs();
		    }
		}
	RCPointer(const RCPointer &a) : _x(a._x) {
                if (_x) {
                    _x->inc_refs();
                    }
                }

 	~RCPointer() {
		make_null();
		}
	RCPointer &operator =(const RCPointer &a) {
		if (a._x) {
                    a._x->inc_refs();
                    }
		make_null();
		_x = a._x;
	 	return *this;
		}
	bool operator ==(const RCPointer &x) const {
	    return (_x == x._x);
	    }

    /*operator bool() const {
        return !is_null();
    }*/
	void make_null() {
            if (_x) {
                if (_x->dec_refs() == 0) {
                    delete _x;
		    }
                }
            _x = 0;
	    }

	bool is_null() const {return (!_x);}
        T* get_ptr() const {
		return _x;
		}

	T *operator ->() const {return get_ptr();}
	operator T *() const {return get_ptr();}
	};

/**	Use this to derive a reference counted pointer type for
 *	a derived type. You need to do this because C++ has
 *	templates rather than parameterized types.
 *
 *	for example:
 *	BaseClass			RCPointer<BaseClass>
 *	DerivedClass:public BaseClass	DerivedRCPointer<DerivedClass,RCPointer<BaseClass>>
 */
template <class T,class P> class DerivedRCPointer : public P {
    public:
        DerivedRCPointer() : P() {}
        DerivedRCPointer(T * x) : P(x) {}
        DerivedRCPointer(const DerivedRCPointer &a) : P(a) {}

	T* get_ptr() const {
	    return (T *)P::get_ptr();
            }

        T *operator ->() const {return get_ptr();}
        operator T *() const {return get_ptr();}
    };


/**	The RCPointer and DerivedRCPointer templates require a suitable class to chew on.
 *	This is such a class. 
 *	\bug You do not have to use this class
 *	as the base of your refernece counted classes
 */
class RefCountedClass {
	unsigned long _count;
    public:
	// note that we use zero as a "latch" to detect overflow
	// _count is actually one greater than the reference count
	RefCountedClass() : _count(1) {}
	unsigned long inc_refs() {
		if(_count)_count ++;
		return _count-1;}
	unsigned long dec_refs() {if (_count > 1)--_count;return _count-1;}
	unsigned long ref_count() {return _count -1;}
    };

/**	\template ref_stack.
 *
 *	A stack for use with reference counted objects
 *	The template parameter is the RCPointer class, not the
 *	class being reference counted.
 *
 *	\warning {
 *	This class is needed to actually remove the references from
 *	the stack when they are not logically present. If you used a
 *	simple stack with such pointers, objects would hang around
 *	because there would still be references on the stack, even though
 *	they had been popped.
 */
template <class x> class ref_stack : public simple_stack<x>
    {
    typedef simple_stack<x> SS;
    public:
	ref_stack(unsigned int start_size = 10, 
		  unsigned int expansion_size = 10) 
		: SS(start_size,expansion_size) {}

	void reset() {
	  int l = SS::len();
		while (l > 0) {
		    l --;
		    (*this)[l].make_null();
		    }
		SS::reset();
		}
	void set_len(int newlen) {
	  int l = SS::len();
            while (l > newlen) {
                l --;
                (*this)[l].make_null();
                }
	    SS::set_len(newlen);
	    }

	void cut(int len) {
	  set_len(len + 1);
	    }

	void remove(int p) {
	  if (p >= SS::len())
		return;
	    x &top = SS::top();
	    SS::remove(p);
	    top.make_null();
	    }
	x pop() {
	    x top = SS::top();
	    set_len(SS::len()-1);
	    return top;
	    }
	};

#endif


