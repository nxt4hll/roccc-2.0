#ifndef __SIMPLE_STACK__
#define __SIMPLE_STACK__
#include "system_specific.h"
#include <assert.h>

/**	@file
 *	A simple stack class - try to avoid
 */

/**	A very simple stack class.
 *
 *	\bug The origins of this class predate STL, so it is not
 *	STL compatible.
 */
template <class x> class simple_stack
    {
        x *buff;
        int buf_len;
	int pos;
	int expansion;
	void resize(int new_size)
	    {
	    if (new_size < pos)
	 	return;
            x *temp = new x[new_size];
            for (int i=0;i<pos;i++)temp[i]=buff[i];

	    delete [] buff;
            buff = temp;
            buf_len = new_size;
	    assert(buf_len >= pos);
	    }
    public:
	simple_stack(unsigned int start_size = 10, unsigned int expansion_size = 10) 
        : buf_len(start_size>3?start_size:3),pos(0),expansion(expansion_size)
		{
		buff = new x[buf_len];
		if (expansion < 1)
		    expansion = 1;
		}

	simple_stack(const simple_stack &y) : buf_len(y.buf_len),
				pos(y.pos),expansion(y.expansion)
	    {
	    buff = new x[y.buf_len];
            for (int i=0;i<pos;i++)
                buff[i] = y.buff[i];
	    assert(buf_len >= pos);
	    }

	virtual ~simple_stack()
	    {
	    if (buff != 0)
	        delete [] buff;
	    }

	simple_stack &operator =(const simple_stack &y)
	    {
	    if (buf_len < y.pos)
		{
	    	if (buff != 0)
	  	    delete [] buff;
	    	buf_len = y.buf_len;
	        buff = new x[buf_len];
		}
	    expansion = y.expansion;
  	    pos = y.pos;
	    for (int i=0;i<pos;i++)
		buff[i] = y.buff[i];
	    assert(buf_len >= pos);
	    return *this;
	    }

	// trim the buffer back to size actually in use
	void freeze()
	    {
	    resize(pos);
	    assert(buf_len >= pos);
	    }

	x *get() const
	    {
	    return buff;
	    }

	x *get_copy() const
	    {
	    x *res=new x[pos];
	    for (int i=0;i<pos;i++)
	    	res[i]=buff[i];

	    return res;
	    }

	void reset() {pos=0;}
	void push(x ch)
	    {
	    if (pos >= buf_len)
                {
		expand(expansion);
                }
	    buff[pos++] = ch;
	    assert(buf_len >= pos);
	    }

	x pop()
	    {
	    return buff[--pos];
	    }

	x& top()
	    {
	    return buff[pos - 1];
	    }
		

   	int len() const { return pos;}

	x &operator[](int i) const {return buff[i];}

	void set_len(int newlen)
	    {
	    if ((newlen >= 0) && (newlen <= pos))
	 	pos = newlen;
	    assert(buf_len >= pos);
	    }

	void cut(int len)
	    {
	    if (len < pos)
		pos = len + 1;
	    assert(buf_len >= pos);
	    }
	void remove(int p)
	    {
	    pos --;
	    for (int i = p;i < pos;i++)
		buff[i] = buff[i + 1];
	    }
 	void expand(int by)
	    {
	    resize(buf_len + by);
	    }

	void set_expansion(int i)
	    {
	    if (i > 0)
		expansion = i;
  	    }

	int get_buf_len() {return buf_len;}
	};

inline void push_string(simple_stack<char> &x,const char *s)
    {
    while (*s != 0)
	{
	x.push(*s);
	s++;
	}
    }

#endif
