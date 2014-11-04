#include "system_specific.h"
#include "suif_hash_map.h"
#include "assert.h"

#ifndef NULL
#define NULL 0
#endif

static int bitsize(unsigned long size)
    {
    unsigned long i = 1;
    int bits = 0;
    while (size != 0)
        {
        size = size & (~i);
        i <<= 1;
        bits++;
        }
    return bits;
    }

class suif_hash_map_table {
    public:
	int table_size;
        suif_hash_map_inner::pair_inner **table;
        long mask;
	suif_hash_map_table * parent;
	suif_hash_map_table * child;
	int entry_count;
	suif_hash_map_table(int size, suif_hash_map_table *pred,int mask);
	~suif_hash_map_table();
	void clear();
    };

suif_hash_map_table::suif_hash_map_table(int size, suif_hash_map_table *pred,int the_mask)
	: table_size(size),table(new suif_hash_map_inner::pair_inner *[size]),
		mask(the_mask),parent(0),child(pred),entry_count(0) {
    assert(table != NULL);
    for (int i =0;i < table_size;i++)
        table[i] = (suif_hash_map_inner::pair_inner *)NULL;
    if (pred != NULL)
	pred->parent = this;
    }

suif_hash_map_table::~suif_hash_map_table() {
    if (child != NULL)
	delete child;
    clear();
    delete [] table;
    }

void suif_hash_map_table::clear() {
    for (int i =0;i < table_size;i++) {
	suif_hash_map_inner::pair_inner *t = table[i];
	table[i] = NULL;
	while (t != NULL) {
	    suif_hash_map_inner::pair_inner *s = t;
	    t = t->next;
	    delete s;
	    }
	}
    }

void suif_hash_map_inner::iterator_inner::advance() {
   if (current == NULL)
        return;
    current = current->next;
    while ((current == NULL) && (hash != NULL)) {
        while ((current == NULL) && (index <  hash->table_size - 1))
            {
            index ++;
            current = hash->table[index];
            }
        if (current == NULL) {
            index = -1;
            hash = hash->child;
            }
        }
    }

void suif_hash_map_inner::iterator_inner::retreat() {
   if (current == NULL) {
	return;
	}

    pair_inner *list = hash->table[index];
    if (list != current) {
	while (list->next != current)
	    list = list->next;
	current = list;
	return;
	}

    current = NULL;
    while (current == NULL) {
	index --;
	while ((index >= 0) && (hash->table[index] == NULL))
	    index --;
	if (index >= 0) {
	    current = hash->table[index];
	    while (current->next != NULL)
		current = current->next;
	    return;
	    }
	hash = hash->parent;
	if (hash == NULL)
	    return;
	index = hash->table_size;
        }
    }

suif_hash_map_inner::iterator_inner & suif_hash_map_inner::iterator_inner::operator ++()
    {
    advance();
    return *this;
    }

suif_hash_map_inner::iterator_inner suif_hash_map_inner::iterator_inner::operator ++(int dummy)
    {
    if (current == NULL)
        return *this;
    iterator_inner x = *this;
    advance();
    return x;
    }

suif_hash_map_inner::iterator_inner & suif_hash_map_inner::iterator_inner::operator --()
    {
    retreat();
    return *this;
    }

suif_hash_map_inner::iterator_inner suif_hash_map_inner::iterator_inner::operator --(int dummy)
    {
    if (current == NULL)
        return *this;
    iterator_inner x = *this;
    retreat();
    return x;
    }



suif_hash_map_inner::suif_hash_map_inner(helper_inner &the_help,int the_table_size) : help(&the_help)
    {
    int size2 = bitsize(the_table_size);
    int table_size = (1 << (size2 - 1));

    int mask = table_size - 1;
    top_table = new suif_hash_map_table(table_size,NULL,mask);
    }

suif_hash_map_inner::~suif_hash_map_inner()
    {
    delete top_table;
    }

suif_hash_map_inner::iterator_inner suif_hash_map_inner::find(const key_inner &x) const
    {
    int hash_value = x.key_hash();
    suif_hash_map_table *next_table = top_table;
    while (next_table != NULL) {
	int index = hash_value & next_table->mask;
    	pair_inner *y = next_table->table[index];
    	while ((y != NULL) && (!(x == y)))
	    y = y->next;
	if (y != NULL)
            return iterator_inner(next_table,y,index);
	next_table = next_table -> child;
	}
    static int count = 0;
    count ++;
    // printf(" not found %d no %d\n",hash_value,count);
    return iterator_inner(NULL,NULL,0);
    }

suif_hash_map_inner::pair_inner*
suif_hash_map_inner::enter_value(const key_inner &x,const pair_inner &val)
    {
    int hash_value = x.key_hash();
    // printf("entering %d\n",hash_value);
    suif_hash_map_table *next_table = top_table;
    while (next_table != NULL) {
        int index = hash_value & next_table->mask;
        pair_inner *y = next_table->table[index];
        while ((y != NULL) && (!(x == y)))
            y = y->next;
        if (y != NULL) {
	    help->set_range(&val,y);
            return y;
            }
	next_table = next_table -> child;
        }

    // Here we decide if we are going to create a new table. There are
    // all sorts of strategies that you could adopt here.

    // What we do here is if the table has more than twice the entries it can hold we
    // add a new table 8 times the size

    // Note that we do not reenter the values in the bigger table. Instead we keep the
    // smaller table and search sequentially

    if (top_table->entry_count > (top_table->table_size << 1))
	{
	top_table = new suif_hash_map_table(top_table->table_size << 3,top_table,(top_table->mask << 3) | 0x7);
	}
    top_table->entry_count ++;
    pair_inner *ny = help->clone(&val);
    int index = hash_value & top_table->mask;
    ny->next = top_table->table[index];
    top_table->table[index] = ny;
    return ny;
    }

suif_hash_map_inner::pair_inner*
suif_hash_map_inner::enter_value_no_change(const key_inner &x,const pair_inner &val)
    {
    int hash_value = x.key_hash();
    // printf("entering no change %d\n",hash_value);
    suif_hash_map_table *next_table = top_table;
    while (next_table != NULL) {
        int index = hash_value & next_table->mask;
        pair_inner *y = next_table->table[index];
        while ((y != NULL) && (!(x == y)))
            y = y->next;
        if (y != NULL) {
            return y;
            }
        next_table = next_table -> child;
        }

    // Here we decide if we are going to create a new table. There are
    // all sorts of strategies that you could adopt here.

    // What we do here is if the table has more than twice the entries it can hold we
    // add a new table 8 times the size

    // Note that we do not reenter the values in the bigger table. Instead we keep the
    // smaller table and search sequentially

    if (top_table->entry_count > (top_table->table_size << 1))
        {
        top_table = new suif_hash_map_table(top_table->table_size << 3,top_table,(top_table->mask << 3) | 0x7);
        }
    top_table->entry_count++;
    pair_inner *ny = help->clone(&val);
    int index = hash_value & top_table->mask;
    ny->next = top_table->table[index];
    top_table->table[index] = ny;
    return ny;
    }

void suif_hash_map_inner::erase(iterator_inner &x)
    {
    int index = x.get_index();
    const suif_hash_map_table* table = x.get_table();
    pair_inner *y = table->table[index];
    pair_inner *last = (pair_inner *)NULL;
    while ((y != NULL) && (x.get() != y))
        {
        last = y;
        y = y->next;
        }
    if (y == NULL)
        return;
    if (last == NULL)
        table->table[index] = y->next;
    else
        last->next = y->next;
    delete y;
    }

void suif_hash_map_inner::clear()
    {
    suif_hash_map_table* table = top_table;
    while (table != NULL) {
	table->clear();
	table = table->child;
	}
    }

suif_hash_map_inner &suif_hash_map_inner::operator =(const suif_hash_map_inner &x)
    {
    delete top_table;
    dup_table(x);
    return *this;
    }

suif_hash_map_inner::suif_hash_map_inner(const suif_hash_map_inner &x) :
  help(x.help)
    {
    dup_table(x);
    }

void suif_hash_map_inner::dup_table(const suif_hash_map_inner &x)
    {
    top_table = NULL;

    suif_hash_map_table *table = x.top_table;

    while (table != NULL) {
	top_table = new suif_hash_map_table(table->table_size,top_table,table->mask);
    	for (int i =0;i < table->table_size;i++)
            {
            pair_inner *el = table->table[i];
            pair_inner *last = (pair_inner *)NULL;
	    while (el != NULL)
                {
                pair_inner *y = help->clone(el);
                if (last == NULL)
                    top_table->table[i] = y;
                else
                    last->next = y;
                el = el->next;
                last = y;
		top_table->entry_count ++;
                }
	    }
       table = table->child;
       }
    // need to re-order the tables
    table = NULL;

    while (top_table != NULL) {
	suif_hash_map_table *t = top_table->child;
    	top_table->parent = top_table->child;
	top_table->child = table;
	table = top_table;
	top_table = t;
	}
    top_table = table;
    }

suif_hash_map_inner::iterator_inner suif_hash_map_inner::begin() const
    {
    int i = 0;
    suif_hash_map_table *table = top_table;

    while (table != NULL) {
        while ((i < table->table_size) && (table->table[i] == 0))
	    i++;
    	if (i < table->table_size)
            return iterator_inner(table,table->table[i],i);
	table = table->child;
	}
    return end();
    }

unsigned suif_hash_map_inner::size() const {
  unsigned count = 0;

  for (suif_hash_map_table *table = top_table;
       table != NULL;
       table = table->child) {
    count = count + table->entry_count;
  }
  return(count);
}


size_t hash( const void* a ) {
  size_t i = (long)a;
  return (i >> 2) + (i >> 10);
}

size_t hash( const unsigned int i) {
  return (i >> 2) + (i >> 10);
}
