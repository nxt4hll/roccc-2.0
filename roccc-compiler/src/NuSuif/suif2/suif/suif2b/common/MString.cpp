#include "MString.h"
#include "lstring.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


class StringInner {
    public:
	int _ref_count;
	StringInner * _next;
	int _size;
	int _used;
	char _text[8];
    };

static const char text_size = 8; // bigger than needed to simplify debugging
static const int pre_alloc_entries = 100;
static const int quantum_size = 8;
static const int quantum_shift = 3; 
static const int max_size_to_keep = pre_alloc_entries*quantum_size;

static StringInner * pre_alloced[pre_alloc_entries] = {
		0,0,0,0,0, 0,0,0,0,0,
                0,0,0,0,0, 0,0,0,0,0,
                0,0,0,0,0, 0,0,0,0,0,
                0,0,0,0,0, 0,0,0,0,0,
                0,0,0,0,0, 0,0,0,0,0,

                0,0,0,0,0, 0,0,0,0,0,
                0,0,0,0,0, 0,0,0,0,0,
                0,0,0,0,0, 0,0,0,0,0,
                0,0,0,0,0, 0,0,0,0,0,
                0,0,0,0,0, 0,0,0,0,0};

int used(StringInner *x) {
    int len = 0;
    while (x) {
	len += x->_used;
	x = x->_next;
	}
    return len;
    }

static StringInner *allocate(int size = 16) {
    StringInner *t;
    int i = (size + quantum_size - 1) >> quantum_shift;
    int required_size = (i << quantum_shift) + sizeof(StringInner) - text_size;
    if (i < pre_alloc_entries) {
	t = pre_alloced[i];
	if (t) {
	    pre_alloced[i] = t->_next;
	    }
	else {
	    t = (StringInner *)malloc(required_size + sizeof(int));
	    t->_size = required_size - sizeof(StringInner) + text_size;
	    }	
	}
    else {
	t = (StringInner *)malloc(required_size + sizeof(int));
	t->_size = required_size - sizeof(StringInner) + text_size;
	}
    t->_used = 0;
    t->_next = 0;

    // always start with a ref count of 1. This assumes we will
    // always but the allocated area in a string somewhere
    t->_ref_count = 1;
    return t;
    }

static void deallocate(StringInner *t) {
    if (t->_size < max_size_to_keep) {
	int i = (t->_size + quantum_size - 1) >> quantum_shift;
	t->_next = pre_alloced[i];
	pre_alloced[i] = t;
	}
    else {
	free(t);
	}
    }

// is this String shared. is_shared is conservative - it may be true 
// even though the string is not shread but it is never false if it is 
// shared
bool String::is_shared() {
    if (!_is_shared)
        return false;
    StringInner *t = _head;
    while (t && (t->_ref_count == 1))
	t = t->_next;
    _is_shared = (t != 0);
    return _is_shared;
    }

void String::free_buffers(StringInner *t) {
    while (t) {
        StringInner *s = t;
        t = t->_next;
	    s->_ref_count --;
        if (s->_ref_count > 0)
            return;
        deallocate(s);
        }
    }

void String::free_buffers() {
    free_buffers(_head);
    _head = 0;
    _tail = 0;
    _len = 0;
    }

void String::make_contiguous(int extra_space) {
    StringInner *new__head;
    if ((_head->_size >= _len + extra_space) && (_head == _tail) && (_head->_ref_count == 1)) {
	return;
	}
    new__head = allocate(_len + extra_space); // space for zero on end
    StringInner *from = _head;
    char *pos = &(new__head->_text[0]);
    while (from) {
	memcpy(pos,from->_text,from->_used);
	pos += from->_used;
 	from = from->_next;
	}
    new__head->_used = _len;
    *pos = 0;
    free_buffers();
    _len = new__head->_used;
    _head = new__head;
    _tail = _head;
    _tail->_next = 0;
    _is_shared = false;
    }

String::String() :
  _head(allocate()),
  _tail(0),
  _len(0),
  _is_shared(false)
{
  _tail=_head; 
}

String::String(int _size,int expand) :
  _head(allocate(_size)),
  _tail(0),
  _len(0),
  _is_shared(false)
{
  _tail = _head;
}

String & String::operator =(const String &x) {
    // assert(x._len == used(x._head));
    // assert(_len == used(_head));
    x._head->_ref_count ++;
    free_buffers();
    _head = x._head;
    _tail = x._tail;
    _len = x._len;
    _is_shared = true;
    const_cast<String *>(&x)->_is_shared = true;
    return *this;
    }

String & String::operator =(const char *x) {
    // assert(_len == used(_head));
    int size = 0;
    int alloc_size = 1;
    if(x) {
	size = strlen(x);
	alloc_size = size;
	}
    if (!_head) {
	_head = allocate(alloc_size);
	_tail = _head;
	}
    else if ((_head->_ref_count > 1) || (_head->_size < size)) {
	free_buffers();
	_head = allocate(alloc_size);
	_tail = _head;
	}
    else {
	free_buffers(_head->_next);
	_tail = _head;
	_tail->_next = 0;
	}
    _len = size;
    if(x)
	memcpy(_head->_text,x,size);
    _head->_used = size;
    _is_shared = false;
    return *this;
    }

String & String::operator +=(const char *x) {
    // assert(_len == used(_head));
    if(!x)
	return *this;
    int size = strlen(x);
    if (is_shared()) {
	make_contiguous(size + 1); // +1 is optimization for possible future get
	}
    _len += size;  // change the length after make_contiguous
                   // since make_contiguous needs it.
    int avail = _tail->_size - _tail->_used;
    if (avail) {
	if (avail > size)
	    avail = size;
	char *pos = _tail->_text + _tail->_used;
	memcpy(pos,x,avail);
	x += avail;
	_tail->_used += avail;
	// assert(_tail->_used <= _tail->_size);
	size -= avail;
	}
    if (size) {
	_tail->_next = allocate(size);
	_tail = _tail->_next;
	memcpy(_tail->_text,x,size);
	_tail->_used = size;
	// assert(_tail->_used <= _tail->_size);
	}
    return *this;
    }

String & String::operator +=(char x) {
    // assert(_len == used(_head));
    push(x);
    return *this;
    }

void String::push(char x) {
    // assert(_len == used(_head));

    if (is_shared())
	make_contiguous(2); // 1 extra to allow easy future get
    if (_tail->_used < _tail->_size) {
	_tail->_text[_tail->_used] = x;
	_tail->_used ++;
	}
    else {
	_tail->_next = allocate(1);
        _tail = _tail->_next;
	_tail->_text[0] = x;
	_tail->_used = 1;
        }
    _len ++;
    }

String String::operator +(const String &x) const {
    // assert(x._len == used(x._head));
    // assert(_len == used(_head));

    return String(*this,x);
    }

String String::operator +(const LString &x) const {
    // assert(x._len == used(x._head));
    // assert(_len == used(_head));

    return String(*this,String(x));
    }


String String::operator +(const char *x) const {
    // assert(x._len == used(x._head));
    // assert(_len == used(_head));
    return String(*this,x);
    }


String String::operator +(char x) const {
    // assert(_len == used(_head));

    return String(*this,x);
    }

void String::append(const String &x) {
    // assert(x._len == used(x._head));
    // assert(_len == used(_head));

    *this += x;
    }

void String::append(const char *x) {
    // assert(_len == used(_head));

    *this += x;
    }

String & String::operator +=(const String &x) {
    // assert(x._len == used(x._head));
    // assert(_len == used(_head));

    if(is_shared())
	make_contiguous(1);
    _tail->_next = x._head;
    _tail = x._tail;
    x._head->_ref_count ++;
    _len += x._len;
    _is_shared = true;
    const_cast<String *>(&x)->_is_shared = true;
    return *this;
    }

String & String::operator +=(const LString &x) {
    return operator +=(String(x));
    }

int String::compare(const String &x) const {
    // assert(x._len == used(x._head));
    // assert(_len == used(_head));

    const StringInner * t_head = _head;
    const StringInner * x_head = x._head;
    while (t_head && (t_head->_used == 0))
	t_head = t_head->_next;
    while (x_head && (x_head->_used == 0))
        x_head = x_head->_next;

    const char *tpos = 0;
    const char *xpos = 0;
    int t_len = 0;
    int x_len = 0;
    if (t_head) {
	tpos = t_head->_text;
	t_len = t_head->_used;
	}
    if (x_head) {
	xpos = x_head->_text;
	x_len = x_head->_used;
	}
    
    while (tpos && xpos) {
	int _len = x_len;
	if (_len > t_len)
	    _len = t_len;
	int i = 0;
	while ( i < _len) {
	    if (xpos[i] != tpos[i])
		return tpos[i] - xpos[i];
	    i++;
	    }
	tpos += _len;
	xpos += _len;
	t_len -= _len;
	x_len -= _len;
	if (t_len == 0) {
	    t_head= t_head->_next;
	    if (t_head) {
		tpos = t_head->_text;
		t_len = t_head->_used;
		}
	    else {
		tpos = 0;
		}
	    }
        if (x_len == 0) {
            x_head= x_head->_next;
            if (x_head) {
                xpos = x_head->_text;
                x_len = x_head->_used;
                }
            else {
                xpos = 0;
                }
            }
	}
    if (tpos) 
	return *tpos;
    if (xpos)
	return -(*xpos);
    return 0;
    }

bool String::operator == (const String  &x) const
    {
    if (_len != x._len)
	return false;
    return (compare(x) == 0);
    }

bool String::operator != (const String &x) const
    {
    if (_len != x._len)
        return true;
    return (compare(x) != 0);
    }

bool String::operator < (const String  &x) const
    {
    return (compare(x) < 0);
    }

bool String::operator <= (const String  &x) const
    {
    return (compare(x) <= 0);
    }

bool String::operator > (const String  &x) const
    {
    return (compare(x) > 0);
    }

bool String::operator >= (const String  &x) const
    {
    return (compare(x) >= 0);
    }

const char * String::get() const {
    // assert(_len == used(_head));

    if ((_head->_size > _len) && (_head->_used == _len)) {
        // just plonk a zero. Always safe under above conditions
        _head->_text[_len] = 0;
        }
    else {
        const_cast<String *>(this)->make_contiguous(1);
    	_head->_text[_len] = 0;
	}
    return _head->_text;
    }

String::operator const char*() const {
  return get();
}


const char * String::c_str() const {
  return get();
}

bool String::truncate_to_last(char marker)
    {
    // assert(_len == used(_head));

    if (is_shared())
	make_contiguous(1);
    const char *x=get();
    const char *y = x;
    while (*x != 0)
	{
	if (*x == marker)
	    y = x;
	x++;
	}
    if (*y == marker)
	{
	_len = (y-_head->_text);
	_head->_used = _len;
	return true;
	}
    return false;
    }

bool String::trim_to_first(char marker)
    {
    // assert(_len == used(_head));

    if (is_shared())
        make_contiguous(1);

    const char *x=get();
    while ((*x != marker) && (*x != 0))
        x++;
    if (*x == marker)
        {
	x++;
	char *y = (char *)get();
	while (*x != 0)
	    {
	    *y = *x;
	    y++;
	    x++;
	    }
        _len = (y-_head->_text);
	_head->_used = _len;
        return true;
        }
    return false;
    }


bool String::truncate_at_pos(int pos)
    {
    // assert(_len == used(_head));

    if (is_shared())
        make_contiguous(1);
    if (pos >= _len)
	return false;
    _len = pos;
    StringInner *t = _head;
    while (t && pos > t->_used) {
	pos -= t->_used;
	}
    if (t) {
	t->_used = pos;
	// assert(t->_used <= t->_size);
	free_buffers(t->_next);
	_tail = t;
	_tail->_next = 0;
	}
    return true;
    }

void String::make_empty() {
    if (_head->_ref_count > 1) {
	free_buffers();
	_head = allocate(1);
	_tail = _head;
	}
    else {
        free_buffers(_head->_next);
        _head->_used = 0;
	}
    _tail = _head;
    _tail->_next = 0;
    _len = 0;
    }

String::String(int i) :
  _head (0), _tail(0), _len(0), _is_shared(false)
{
    char temp[80];
    sprintf(temp,"%d",i);
    *this = temp;
}

String::String(size_t i) :
  _head (0), _tail(0), _len(0), _is_shared(false)
{
    char temp[80];
    sprintf(temp,"%u",i);
    *this = temp;
}

String::String(bool b) :_head (0), _tail(0), _len(0), _is_shared(false) {
    if (b)
	*this = "true";
    else
	*this = "false";
    }

String::String(long l) :_head (0), _tail(0), _len(0), _is_shared(false) {
    char temp[80];
    sprintf(temp,"%ld",l);
    *this = temp;
    }

String::String(double d) :_head (0), _tail(0), _len(0), _is_shared(false) {
    char temp[80];
    sprintf(temp,"%22.16e",d);
    *this = temp;
    }

String::String(const char *text) :
  _head(0), _tail(0), _len(0), _is_shared(false)
{
    int size = 0;
    int alloc_size = 1;
    if(text) {
	size = strlen(text);
	alloc_size = size;
	}
    _len = size;
    _head = allocate(alloc_size);
    _tail = _head;
    if(text)
	memcpy(_head->_text,text,size);
    _head->_used = size;
    // assert(_head->_used <= _head->_size);
    }

String::String(const String &x,const String &y) :
  _head(x._head),
  _tail(x._tail),
  _len(x._len),
  _is_shared(true)
{
    x._head->_ref_count ++;
    // x._is_shared = true;   We will always copy this, so no need to set flag
    *this += y;
    }

String::String(const String &x) :
  _head(x._head),
  _tail(x._tail),
  _len(x._len),
  _is_shared(true)
{
    x._head->_ref_count ++;
    const_cast<String *>(&x)->_is_shared = true;
    }

String::String(const String &x,const char y) :
  _head(0), _tail(0), _len(0), _is_shared(false)
{
    int _size = x._len + 1;
    _len = _size;
    _head = allocate(_size);
    _tail = _head;
    _head->_used = _len;
    // assert(_head->_used <= _head->_size);
    const StringInner *t = x._head;
    char *pos = _head->_text;
    while (t) {
        memcpy(pos,t->_text,t->_used);
        pos += t->_used;
        t = t->_next;
        }
    _head->_text[_len] = y;
    }

String String::Left(int t_len) const {
    if (t_len < 0)
        t_len = _len + t_len;

    if (t_len >= _len)
	return *this;

    String t(*this);
    t.truncate_at_pos(t_len);
    // assert(_len == used(_head));

    return t;
    }

String String::Right(int t_len) const {
    if (t_len < 0)
	t_len = _len + t_len;
    if (t_len >= _len)
        return *this;
    int i = _len - t_len;
    const char *x = get();
    return String(x + i);
    }

String String::Repeat(int times){
    const char* str = get();
    String result("");
    for(int i=0;i<times;i++) result.append(str);
    return result; 
    }

String String::Mid(int left_pos,int t_len) const {
    if (left_pos >= _len)
	return String();
    const char *x = get();
    String t(x + left_pos);
    t.truncate_at_pos(t_len);
    return t;
    }

String String::substr(int left_pos,int t_len) const {
    return Mid(left_pos,t_len);
    }

bool String::ends_in(const String &s) const {
    int s_len = s._len;
    if (s_len > _len)
	return false;
    int i = _len - s_len;
    const char *x = get();
    const char *y = s.get();
    x += i;
    while ((i < _len) && (*x == *y)) {
	i++;
	x++;
	y++;
	}
    return (i >= _len);
    }

bool String::starts_with(const String &s) const {
    int s_len = s._len;
    if (s_len > _len)
        return false;
    int i = 0;
    const char *x = get();
    const char *y = s.get();
    while ((i < s_len) && (*x == *y)) {
        i++;
        x++;
        y++;
        }
    return (i >= s_len);
    }

int String::find(const String &s) const {
    int x_len = s._len;
    int s_len = _len - x_len;
    if (s_len < 0)
	return -1;
    const char *xs = s.get();
    const char *x = get();

    for (int i=0;i<=s_len;i++) {
	const char *m = x + i;
	int j = 0;
	while ((j < x_len) && (xs[j] == m[j]))
	    j++;
	if (j >= x_len)
	    return i;
	}
    return -1;
    }

String::~String() {
    free_buffers();
    }

String::String(const char *text,int size) :
  _head(allocate(size)),
  _tail(0),
  _len(size),
  _is_shared(false)
{
    _tail = _head;
    _head->_used = _len;
    // assert(_head->_used <= _head->_size);
    memcpy(_head->_text,text,size);
    }

simple_stack<String> *split(const char *x,char separator)
    {
    simple_stack<String> * sp= new simple_stack<String>(10,10);
    String s;
    while (*x != 0)
	{
	if (*x == separator)
	    {
	    sp->push(s);
	    s.make_empty();
	    }
	else
	    {
	    s.push(*x);
	    }
	x++;
	}
    sp->push(s);
    return sp;
    }

const String emptyString;

String::String(const LString &x) :
  _head(0), _tail(0), _len(0), _is_shared(false)
{
    int _size = x.length();
    _len = _size;
    if (_size <= 0) 
	_size = 1;
    _head = allocate(_size);
    _tail = _head;
    if (x.c_str()) {
        memcpy(_head->_text,x.c_str(),_size);
	}
    _head->_used = _len;
    // assert(_head->_used <= _head->_size);
    }

char &String::operator[](int i) {
    // assert((i>=0) && (i<= _len));
    StringInner *t = _head;
    while (t && (i >= t->_used)) {
	i -= t->_used;
	t = t->_next;
	}
    // assert(t);
    return t->_text[i];
    }

void String::set_value(const char *text,int size) {
    int alloc_size = size + 1;
    if (!_head) {
        _head = allocate(alloc_size);
        }
    else if ((_head->_ref_count > 1) || (_head->_size < size)) {
        free_buffers();
        _head = allocate(alloc_size);
        _tail = _head;
        }
    else {
        free_buffers(_head->_next);
        _tail = _head;
        _tail->_next = 0;
        }
    _len = size;
    if(text)
        memcpy(_head->_text,text,size);
    _head->_used = size;
    _is_shared = false;
    }
	

