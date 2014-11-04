/*  Generic List Definitions */

/*  Copyright (c) 1994 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */

#ifndef GCLIST_H
#define GCLIST_H
#include <assert.h>
//#include <gc_cpp.h>
#include <stdio.h>
#pragma interface

/*
 *  Class gclist_e: List elements for generic lists.  A gclist_e list
 *  element only contains a pointer to the next element in the list.
 *  Other fields are added in derived classes.
 */

class gclist_e /*: public gc*/ {
    friend class gclist;
private:
    /* We make explicit copy constructor and assignment operator and
     * make them private to foil C++'s automatic default versions. */
    gclist_e(const gclist_e &)  { assert(0); }
    void operator=(const gclist_e &)  { assert(0); }

protected:
    gclist_e *next_e;
    
public:
    gclist_e()				{ next_e = NULL; }
    gclist_e *next() const		{ return next_e; }
};


/*
 *  Class gclist: Generic lists are used as base classes throughout the
 *  SUIF system.  The gclist class contains pointers to the gclist_e list
 *  elements at the head and tail of a list.  An empty list is
 *  indicated by a NULL head pointer.
 *
 *  The push and pop functions add and remove elements from the front
 *  of the list.  The append function adds a list element on the end of
 *  the list.  The remove function removes the given element from the list.
 *  The grab_from function takes all of the elements from another gclist and
 *  then clears that list.  The count function returns the number of
 *  elements in a list.
 * 
 *  There is no storage management in gclist.  Since deallocation
 *  is intimately related to the contents and usage patterns of the
 *  derived classes, such behavior should be specified by them.
 */

class gclist /*: public gc*/ {
private:
    /* We make explicit copy constructor and assignment operator and
     * make them private to foil C++'s automatic default versions. */
    gclist(const gclist &)  { assert(0); }
    void operator=(const gclist &)  { assert(0); }

protected:
    gclist_e *head_e, *tail_e;

public:
    gclist()				{ head_e = tail_e = NULL; }
    virtual ~gclist();

    int is_empty() const		{ return head_e == NULL; }
    gclist_e *head() const		{ return head_e; }
    gclist_e *tail() const		{ return tail_e; }
    gclist_e *push(gclist_e *e);
    gclist_e *pop();
    gclist_e *append(gclist_e *e);
    gclist_e *insert_before(gclist_e *e, gclist_e *pos);
    gclist_e *insert_after(gclist_e *e, gclist_e *pos);
    gclist_e *remove(gclist_e *e);
    void clear()			{ head_e = tail_e = NULL; }
    void erase();
    void grab_from(gclist *l);
    void push(gclist *l);
    void append(gclist *l);
    void insert_before(gclist *l, gclist_e *pos);
    void insert_after(gclist *l, gclist_e *pos);
    int count() const;
    int contains(const gclist_e *e) const;
    gclist_e *operator[](int ndx) const;
};


/*
 *  Class gclist_iter:  Iterator for generic lists.  This provides an easy
 *  way to traverse the elements of a gclist.  The reset function initializes
 *  the iterator to point to the beginning of a list.  The is_empty function
 *  can then be used as the exit condition for a loop.  Within the loop,
 *  a call to the step function will return the current list element and
 *  advance the iterator.  The peek function returns the current list
 *  element without advancing to the next element.
 */

class gclist_iter /*: public gc*/ {
private:
    /* We make explicit copy constructor and assignment operator and
     * make them private to foil C++'s automatic default versions. */
    gclist_iter(const gclist_iter &)  { assert(0); }
    void operator=(const gclist_iter &)  { assert(0); }

protected:
    gclist_e *cur, *nxt;

public:
    gclist_iter()			{ cur = nxt = NULL; }
    gclist_iter(const gclist *gl)		{ reset(gl); }

    void reset(const gclist *gl);
    void set(gclist_e *e);		/* set the next element */

    int is_empty() const		{ return nxt == NULL; }
    gclist_e *step();
    gclist_e *peek() const		{ return nxt; }
    gclist_e *cur_elem() const		{ return cur; }
};


/*
 *  The following macro definitions are used to automatically generate
 *  subclasses of gclist that contain various data types.  (This is an
 *  alternative to using C++ templates, which seem to be plagued with bugs
 *  in the GNU compiler.)  The set_elem() virtual function can be overridden
 *  in a derived class to automatically update a list element when it is
 *  added to a list.  For example, set_elem() may set back pointers in the
 *  TYPE object to the CLASS list and/or to the CLASS_E list element.
 */

#define DECLARE_GCLIST_CLASS(CLASS, TYPE)					      \
    DECLARE_GCLIST_CLASSES(CLASS, CLASS##_e, CLASS##_iter, TYPE)

#define DECLARE_GCLIST_CLASSES(CLASS, CLASS_E, CLASS_ITER, TYPE)		      \
DECLARE_GCLIST_CLASS_E(CLASS_E, TYPE, gclist_e, ;);			      \
DECLARE_GCLIST_CLASS_LIST(CLASS, CLASS_E, TYPE, gclist, ;);		      \
DECLARE_GCLIST_CLASS_ITER(CLASS, CLASS_E, CLASS_ITER, TYPE, gclist_iter)


#define DECLARE_GCLIST_CLASS_E(CLASS_E, TYPE, BASE_E, EXTRA)		      \
class CLASS_E: public BASE_E {						      \
public:									      \
    TYPE contents;							      \
    CLASS_E(TYPE t) { contents = t; }					      \
    CLASS_E *next() const { return (CLASS_E*)BASE_E::next(); }		      \
    EXTRA								      \
}


#define DECLARE_GCLIST_CLASS_LIST(CLASS, CLASS_E, TYPE, BASE_CLASS, EXTRA)      \
class CLASS: public BASE_CLASS {					      \
protected:								      \
    virtual void set_elem(CLASS_E *) { }				      \
    void set_elems(CLASS *l)						      \
	{ CLASS_E *i = l->head(); while (i) { set_elem(i); i = i->next(); } } \
public:									      \
    CLASS() { }								      \
    CLASS(TYPE t) { append(t); }					      \
    CLASS(TYPE t1, TYPE t2) { append(t1); append(t2); }			      \
    CLASS(TYPE t1, TYPE t2, TYPE t3) { append(t1); append(t2); append(t3); }  \
    CLASS_E *head() const { return (CLASS_E*)BASE_CLASS::head(); }	      \
    CLASS_E *tail() const { return (CLASS_E*)BASE_CLASS::tail(); }	      \
    TYPE push(TYPE t) { return push(new CLASS_E(t))->contents; }	      \
    CLASS_E *push(CLASS_E *e)						      \
	{ set_elem(e); return (CLASS_E*)BASE_CLASS::push(e); }		      \
    TYPE pop() { CLASS_E *e = (CLASS_E *)BASE_CLASS::pop();		      \
		 TYPE t = e->contents; delete e; return t; }		      \
    TYPE append(TYPE t) { return append(new CLASS_E(t))->contents; }	      \
    CLASS_E *append(CLASS_E *e)						      \
	{ set_elem(e); return (CLASS_E*)BASE_CLASS::append(e); }	      \
    TYPE insert_before(TYPE t, CLASS_E *pos)				      \
	{ return insert_before(new CLASS_E(t), pos)->contents; }	      \
    CLASS_E *insert_before(CLASS_E *e, CLASS_E *pos)			      \
	{ set_elem(e); return (CLASS_E*)BASE_CLASS::insert_before(e, pos); }  \
    TYPE insert_after(TYPE t, CLASS_E *pos)				      \
	{ return insert_after(new CLASS_E(t), pos)->contents; }		      \
    CLASS_E *insert_after(CLASS_E *e, CLASS_E *pos)			      \
	{ set_elem(e); return (CLASS_E*)BASE_CLASS::insert_after(e, pos); }   \
    CLASS_E *remove(CLASS_E *e) { return (CLASS_E*)BASE_CLASS::remove(e); }   \
    void copy(CLASS const * l) {						      \
	CLASS_E *i = l->head();						      \
	while (i) { append(i->contents); i = i->next(); } }		      \
    void grab_from(CLASS *l) { set_elems(l); BASE_CLASS::grab_from(l); }      \
    void push(CLASS *l) { set_elems(l); BASE_CLASS::push(l); }		      \
    void append(CLASS *l) { set_elems(l); BASE_CLASS::append(l); }	      \
    void insert_before(CLASS *l, CLASS_E *pos)				      \
	{ set_elems(l); BASE_CLASS::insert_before(l, pos); }		      \
    void insert_after(CLASS *l, CLASS_E *pos)				      \
	{ set_elems(l); BASE_CLASS::insert_after(l, pos); }		      \
    TYPE operator[](int ndx) const					      \
	{ return ((CLASS_E*)BASE_CLASS::operator[](ndx))->contents; }	      \
    CLASS_E *lookup(TYPE const t) const {				      \
	CLASS_E *i = head();						      \
	while (i) { if (i->contents == t) break; i = i->next(); }	      \
	return i; }							      \
    EXTRA								      \
}


#define DECLARE_GCLIST_CLASS_ITER(CLASS, CLASS_E, CLASS_ITER, TYPE, BASE_ITER)  \
class CLASS_ITER: public BASE_ITER {					      \
public:									      \
    CLASS_ITER() : BASE_ITER() { }					      \
    CLASS_ITER(CLASS const *l) : BASE_ITER(l) { }			      \
    TYPE step() { return ((CLASS_E*)BASE_ITER::step())->contents; }	      \
    TYPE peek() const { assert(!is_empty());				      \
			return ((CLASS_E*)BASE_ITER::peek())->contents; }     \
    CLASS_E *cur_elem() const { return (CLASS_E*)BASE_ITER::cur_elem(); }     \
}									      \

#endif /* GCLIST_H */


