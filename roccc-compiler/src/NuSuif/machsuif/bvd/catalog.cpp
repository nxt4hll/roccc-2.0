/* file "bvd/catalog.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "bvd/catalog.h"
#endif

#include <machine/machine.h>

#include <bvd/catalog.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

RegSymCatalog::RegSymCatalog
	(bool record, filter_f filter, int hash_map_size)
    : OpndCatalog(record, hash_map_size / 2), _filter(filter),
      _next_slot(0), _hash_map(hash_map_size)
{
    claim((hash_map_size > 0)
       && (hash_map_size == (hash_map_size & -hash_map_size)),
	  "Register/symbol catalog map size must be a non-zero power of two");
}

bool
RegSymCatalog::enroll(Opnd opnd, int *slot)
{
    return get_slot(opnd, slot, true);
}

bool
RegSymCatalog::lookup(Opnd opnd, int *slot) const
{
    return const_cast<RegSymCatalog*>(this)->
	get_slot(opnd, slot, false);
}

/*
 * get_slot -- Look up or allocate a slot for an operand, returning true
 * iff "successful".
 *
 * Specifically, if `enroll' is true, then return true iff the operand is
 * not filtered out and it produces a new entry when enrolled in _hash_map.
 * If `enroll' is false, then if the operand is filtered out, or if it's
 * neither a register nor a symbol, or if no entry for the operand exists
 * in _hash_map, return false.
 *
 * As a side effect when a slot is found for the operand, store it
 * indirectly through `slot', provided the latter is not null.
 */
bool
RegSymCatalog::get_slot(Opnd opnd, int *slot, bool enroll)
{
    if ((!is_reg(opnd) && !is_var(opnd)) || (_filter && !_filter(opnd)))
	return false;				// not a candidate operand

    unsigned long key = hash_map_key(opnd);

    if (enroll && _hash_map.find(key) == _hash_map.end()) {
	_hash_map.enter_value(key, _next_slot);	// entry in _hash_map is new
	if (slot != NULL)
	    *slot = _next_slot;
	enroll_inverse(_next_slot, opnd);
	_next_slot++;
	return true;
    }

    if (enroll && slot == NULL)
	return false;
    
    HashMap<unsigned long, int>::iterator handle = _hash_map.find(key);
    if (handle == _hash_map.end())
	return false;
    if (slot != NULL)
	*slot = (*handle).second;
    return !enroll;
}

/* hash_map_key -- Helper for get_slot.  Return zero if the given operand
 * is not a VR or var_sym.  Otherwise, return a unique non-zero hash table
 * key for it.  For a register, we use an even number that is based on the
 * register number.  A hard register gets a positive even key; a negative
 * number gets a negative even key.  For a symbol, the key is the odd
 * number that is one beyond the symbol's address.
 */
unsigned long
RegSymCatalog::hash_map_key(Opnd opnd) const
{
    if  (is_reg(opnd)) {
	int reg = get_reg(opnd);
	if (is_virtual_reg(opnd))
	    reg = -(reg + 1);
	return reg << 1;
    } else if (is_var(opnd)) {
	return (unsigned long)get_var(opnd) | 1;
    }
    return 0;
}
