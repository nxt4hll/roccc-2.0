/* file "bvd/catalog.h" -- Maps from operands to slot sets */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef BVD_OPND_CATALOG_H
#define BVD_OPND_CATALOG_H

#include <machine/copyright.h>
   
#ifdef USE_PRAGMA_INTERFACE
#pragma interface "bvd/catalog.h"
#endif

#include <machine/machine.h>


class RegSymCatalog : public OpndCatalog {
  public:
    typedef bool (*filter_f)(Opnd);

    RegSymCatalog(bool record = false, filter_f = NULL,
		  int hash_table_size = 1024);

    virtual int size() const { return _next_slot; }

    virtual bool enroll(Opnd, int *slot = NULL);
    virtual bool lookup(Opnd, int *slot = NULL) const;

    filter_f _filter;
    int _next_slot;

    HashMap<unsigned long, int> _hash_map;
    virtual unsigned long hash_map_key(Opnd) const;

    virtual bool get_slot(Opnd, int*, bool);

    virtual ~RegSymCatalog() { }
};

#endif /* BVD_OPND_CATALOG_H */
