/* Generic List Implementation */

/*  Copyright (c) 1994 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */

#define _MODULE_ "libsuif.a"

#pragma implementation "gclist.h"

#define RCS_BASE_FILE gclist_cc

#include "gclist.h"

/*
 *  Delete a gclist.
 */

gclist::~gclist ()
{
    erase();
}


/*
 *  Add a gclist_e element at the start of a gclist.
 */

gclist_e *
gclist::push (gclist_e *e)
{
    e->next_e = head_e;
    head_e = e;
    if (!tail_e)
	tail_e = head_e;
    return e;
}


/*
 *  Remove and return the element at the start of a gclist.
 */

gclist_e *
gclist::pop ()
{
    assert(head_e);

    gclist_e *e = head_e;
    head_e = e->next_e;
    if (!head_e)
	tail_e = NULL;
    return e;
}


/*
 *  Add a gclist_e element at the end of a gclist.
 */

gclist_e *
gclist::append (gclist_e *e)
{
    if (tail_e) {
	tail_e->next_e = e;
	tail_e = e;
    } else {
	head_e = tail_e = e;
    }
    e->next_e = NULL;
    return e;
}


/*
 *  Insert the list element e before the element pos.  pos==NULL means
 *  insert at the end of the list.
 */

gclist_e *
gclist::insert_before (gclist_e *e, gclist_e *pos)
{
    if (!pos) {
	append(e);
    } else if (pos == head_e) {
	push(e);
    } else {
	gclist_e *el = head_e, *prev_e = NULL;
	while (el) {
	    if (el->next_e == pos) {
		e->next_e = el->next_e;
		el->next_e = e;
		return e;
	    }
	    prev_e = el;
	    el = el->next_e;
	}
	assert(0);
    }
    return e;
}


/*
 *  Insert the list element e after the element pos.  pos==NULL means
 *  insert at the beginning of the list.
 */

gclist_e *
gclist::insert_after (gclist_e *e, gclist_e *pos)
{
    if (!pos) {
	push(e);
    } else {
	e->next_e = pos->next_e;
	pos->next_e = e;
	if (!e->next_e)
	    tail_e = e;
    }
    return e;
}


/*
 *  Remove the specified gclist_e element from the list.
 */

gclist_e *
gclist::remove (gclist_e *e)
{
    gclist_e *el = head_e, *prev_e = NULL;

    while (el) {
	if (el == e) {
	    if (prev_e)
		prev_e->next_e = el->next_e;
	    if (head_e == el)
		head_e = el->next_e;
	    if (tail_e == el)
		tail_e = prev_e;
	    return e;
	}
	prev_e = el;
	el = el->next_e;
    }

    return e;
}


/*
 *  Delete all the entries in the list.  NOTE this will not call the
 *  destructor of any data contained in the elements!
 */

void
gclist::erase ()
{
    gclist_iter gli(this);
    while (!gli.is_empty())
	delete gli.step();
    head_e = NULL;
    tail_e = NULL;
}


/*
 *  Functions to combine two lists.  All of the elements of the parameter list
 *  are removed and combined with the current list.
 */

void
gclist::push (gclist *l)
{
    if (!l->tail_e) return;
    l->tail_e->next_e = head_e;
    head_e = l->head_e;
    if (!tail_e)
	tail_e = l->tail_e;
    l->clear();
}


void
gclist::append (gclist *l)
{
    if (!tail_e) {
	grab_from(l);
	return;
    }
    tail_e->next_e = l->head_e;
    if (l->tail_e)
	tail_e = l->tail_e;
    l->clear();
}


/*
 *  Insert the list l before the element pos.  pos==NULL means
 *  insert at the end of the list.
 */

void
gclist::insert_before (gclist *l, gclist_e *pos)
{
    if (!l->tail_e) return;

    if (!pos) {
	append(l);
    } else if (pos == head_e) {
	push(l);
    } else {
	gclist_e *el = head_e, *prev_e = NULL;
	while (el) {
	    if (el->next_e == pos) {
		l->tail_e->next_e = el->next_e;
		el->next_e = l->head_e;
		l->clear();
		return;
	    }
	    prev_e = el;
	    el = el->next_e;
	}
	assert(0);
    }
}


/*
 *  Insert the list l after the element pos.  pos==NULL means
 *  insert at the beginning of the list.
 */

void
gclist::insert_after (gclist *l, gclist_e *pos)
{
    if (!l->tail_e) return;

    if (!pos) {
	push(l);
    } else {
	l->tail_e->next_e = pos->next_e;
	pos->next_e = l->head_e;
	if (!l->tail_e->next_e)
	    tail_e = l->tail_e;
    }
}


/*
 *  Steal all of the elements from another gclist and clear that list.
 *  The current list must be empty when this function is called.
 */

void
gclist::grab_from (gclist *l)
{
    assert(is_empty());
    head_e = l->head_e;
    tail_e = l->tail_e;
    l->clear();
}


/*
 *  Count the number of elements in the list.
 */

int
gclist::count () const
{
    int cnt;
    gclist_iter gi(this);

    for (cnt = 0; !gi.is_empty(); gi.step()) {
	cnt++;
    }

    return cnt;
}


/*
 *  Determine if the specified element is in the list.
 */

int
gclist::contains (const gclist_e *e) const
{
    gclist_e *el = head_e;

    while (el) {
	if (el == e)
	    return 1;
	el = el->next_e;
    }
    return 0;
}


/*
 *  Access an element a particular distance from the front of the list.
 */

gclist_e *
gclist::operator[] (int ndx) const
{
    // int origndx = ndx;
    
    gclist_e *el = head_e;
    while (el) {
	if (!ndx)
	    return el;
	ndx--;
	el = el->next();
    }
    assert(0);
    return NULL;
}


/*****************************************************************************/


void
gclist_iter::reset (const gclist *gl)
{
    cur = NULL;
    nxt = gl ? gl->head() : NULL;
}


void
gclist_iter::set (gclist_e *e)
{
    nxt = e;
}


gclist_e *
gclist_iter::step ()
{
    cur = nxt;
    if (nxt) nxt = nxt->next();
    return cur;
}


