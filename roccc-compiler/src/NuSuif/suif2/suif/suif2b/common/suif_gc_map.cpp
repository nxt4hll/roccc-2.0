/*
 * file : suif_gc_map.h
 */

#include "suif_gc_map.h"
#include "machine_dependent.h"


suif_gc_map_inner::iterator_inner & suif_gc_map_inner::iterator_inner::operator ++()
    {
    if (current == NULL)
	return *this;
    current = current->next;
    return *this;
    }

suif_gc_map_inner::iterator_inner suif_gc_map_inner::iterator_inner::operator ++(int dummy)
    {
    if (current == NULL)
        return *this;
    iterator_inner x = *this;

    current = current->next;
    return x;
    }

suif_gc_map_inner::iterator_inner &suif_gc_map_inner::iterator_inner::operator --()
    {
    if (current == NULL)
        return *this;
    pair_inner *x = suif_gc_map->table;
    pair_inner *y = NULL;
    while (x != current)
	{
	y = x;
	x = x->next;
	}
    current = y;
    return *this;
    }

suif_gc_map_inner::iterator_inner suif_gc_map_inner::iterator_inner::operator --(int dummy)
    {
    if (current == NULL)
        return *this;
    iterator_inner p = *this;
    pair_inner *x = suif_gc_map->table;
    pair_inner *y = NULL;
    while (x != current)
        {
        y = x;
        x = x->next;
        }
    current = y;
    return p;
    }



suif_gc_map_inner::suif_gc_map_inner(helper_inner &x) :
	table(0), help(&x), m_nSize(0)
    {
    }

suif_gc_map_inner::~suif_gc_map_inner()
    {
    clear();
    }

suif_gc_map_inner::iterator_inner suif_gc_map_inner::find(const key_inner &x) const
    {
    pair_inner *y = table;
    while ((y != NULL) && (!(x == y)))
	y = y->next;
    return iterator_inner(this,y);
    }

suif_gc_map_inner::pair_inner *suif_gc_map_inner::enter_value(const key_inner &x,const pair_inner &val)
    {
    pair_inner *y = table;
    while ((y != NULL) && (!(x == y)))
        y = y->next;
    if (y != NULL)
	{
	help->set_range(&val,y);
	return y;
	}
    pair_inner *ny = help->clone(val);
    ny->next = table;
    table = ny;
    m_nSize++;
    return ny;
    }

suif_gc_map_inner::pair_inner *suif_gc_map_inner::enter_value_no_change(const key_inner &x,const pair_inner &val)
    {
    pair_inner *y = table;
    while ((y != NULL) && (!(x == y)))
        y = y->next;
    if (y != NULL)
        {
        return y;
        }
    pair_inner *ny = help->clone(val);
    ny->next = table;
    table = ny;
    m_nSize++;
    return ny;
    }


void suif_gc_map_inner::erase(iterator_inner &x)
    {
    pair_inner *y = table;
    pair_inner *last = (pair_inner *)NULL;
    while ((y != NULL) && (x.get_current() != y))
	{
	last = y;
        y = y->next;
	}
    if (y == NULL)
	return;
    if (last == NULL)
	table = y->next;
    else
	last->next = y->next;
    delete y;
    m_nSize--;
    }

suif_gc_map_inner::iterator_inner suif_gc_map_inner::insert(iterator_inner &x,const pair_inner &p)
    {

    pair_inner *y = table;
    pair_inner *last = (pair_inner *)NULL;
    while ((y != NULL) && (x.get_current() != y))
        {
        last = y;
        y = y->next;
        }
    if ( y == 0 ) {
       m_nSize++;
    } else {
      delete y;
    }
    y = help->clone(p);

    if (last == NULL)
	{
	y->next = table;
	table = y;
	}
    else
	{
	y->next = last->next;
	last->next = y;
	}
	
    return iterator_inner(this,y);
    }

void suif_gc_map_inner::dup_list(const suif_gc_map_inner &x)
    {
    pair_inner *y = table;
    pair_inner *last = (pair_inner *)NULL;
    while (y != NULL)
        {
	pair_inner *yn = help->clone(*y);
	if (last == NULL)
	    table = yn;
	else
	    last->next = yn;
        last = y;
        y = y->next;
        }
    }

void suif_gc_map_inner::clear()
    {
    pair_inner *x = table;
    pair_inner *y;
    while (x != NULL)
        {
        y = x;
        x = x->next;
        delete y;
        }
    m_nSize = 0;
    // NULLify the table once we are done clearing
    table = NULL;
    }

suif_gc_map_inner::suif_gc_map_inner(const suif_gc_map_inner &x) :
  table(0), help(x.help), m_nSize(0)
    {
    dup_list(x);
    }

suif_gc_map_inner &suif_gc_map_inner::operator =(const suif_gc_map_inner &x)
    {
    clear();
    dup_list(x);
    return *this;
    }
