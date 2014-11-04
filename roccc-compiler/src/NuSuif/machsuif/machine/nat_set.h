/* file "machine/nat_set.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef MACHINE_NAT_SET_H
#define MACHINE_NAT_SET_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "machine/nat_set.h"
#endif

#include <machine/substrate.h>

class NatSet;
class NatSetIterRep;

class NatSetIterPure {
  public:
    virtual ~NatSetIterPure() { }

    virtual unsigned current() const = 0;
    virtual bool is_valid() const = 0;
    virtual void next() = 0;

    virtual void insert(unsigned) = 0;
    virtual void remove(unsigned) = 0;
};

class NatSetIter : public NatSetIterPure {
  public:
    NatSetIter(NatSet&);
    NatSetIter(const NatSetIter&);
    NatSetIter(NatSetIterRep *rep) : rep(rep) { }
    virtual ~NatSetIter();

    virtual NatSetIter& operator=(const NatSetIter&);

    virtual unsigned current() const;
    virtual bool is_valid() const;
    virtual void next();

    virtual void insert(unsigned);
    virtual void remove(unsigned);

  protected:
    NatSetIterRep *rep;    
};

class NatSet {
  public:
    virtual ~NatSet() { }
    virtual NatSet& operator=(const NatSet&);

    virtual bool is_finite() const = 0;
    virtual bool is_empty() const = 0;

    virtual int size() const = 0;

    virtual bool contains(unsigned element) const = 0;
    virtual bool contains(const NatSet&) const;
    virtual bool overlaps(const NatSet&) const;

    virtual void insert(unsigned element) = 0;
    virtual void remove(unsigned element) = 0;
    virtual void accommodate(unsigned element) = 0;

    virtual void insert_all() = 0;
    virtual void remove_all() = 0;

    virtual void complement() = 0;
    
    virtual bool operator==(const NatSet&) const;
    virtual bool operator!=(const NatSet &that) const
	{ return !(*this == that); }

    virtual void operator+=(const NatSet&);
    virtual void operator*=(const NatSet&);
    virtual void operator-=(const NatSet&);

    virtual NatSetIter iter(bool complement = false) = 0;
    virtual NatSetIter iter(bool complement = false) const = 0;

    virtual void print(FILE* = stdout, unsigned bound = UINT_MAX) const;

  protected:
    friend class NatSetCopy;

    virtual int us_size() const = 0;
    virtual NatSet* clone() const = 0;
};

class NatSetDense : public NatSet {
  public:
    NatSetDense(bool complement = false, int size_hint = 0);
    NatSetDense(const NatSet&);
    NatSet& operator=(const NatSet& that);

    bool is_finite() const;
    bool is_empty() const;

    int size() const;

    bool contains(unsigned element) const;
    bool contains(const NatSet&) const;
    bool overlaps(const NatSet&) const;

    void insert(unsigned element);
    void remove(unsigned element);
    void accommodate(unsigned element);

    void insert_all();
    void remove_all();

    void complement();
    
    bool operator==(const NatSet&) const;

    void operator+=(const NatSet&);
    void operator*=(const NatSet&);
    void operator-=(const NatSet&);

    NatSetIter iter(bool complement = false);
    NatSetIter iter(bool complement = false) const;

  protected:
    BitVector us;		// underlying set

    int us_size() const;
    NatSet* clone() const;
};

class NatSetSparse : public NatSet {
  public:
    NatSetSparse(bool complement = false, int size_hint = 0)
	: is_complemented(complement) { }
    NatSetSparse(const NatSet&);

    NatSet& operator=(const NatSet &rhs);

    bool is_finite() const;
    bool is_empty() const;

    int size() const;

    bool contains(unsigned element) const;

    void insert(unsigned element);
    void remove(unsigned element);
    void accommodate(unsigned element) { }

    void insert_all();
    void remove_all();

    void complement();
    
    bool operator==(const NatSet&) const;

    NatSetIter iter(bool complement = false);
    NatSetIter iter(bool complement = false) const;

  protected:
    Set<unsigned> us;
    bool is_complemented;

    int us_size() const;
    NatSet* clone() const;
};

class NatSetCopy : public NatSet {
  public:
    NatSetCopy(const NatSet&);
    NatSet& operator=(const NatSet&);

    bool is_finite() const;
    bool is_empty() const;

    int size() const;

    bool contains(unsigned element) const;
    bool contains(const NatSet&) const;
    bool overlaps(const NatSet&) const;

    void insert(unsigned element);
    void remove(unsigned element);
    void accommodate(unsigned element);

    void insert_all();
    void remove_all();

    void complement();
    
    bool operator==(const NatSet&) const;

    void operator+=(const NatSet&);
    void operator*=(const NatSet&);
    void operator-=(const NatSet&);

    NatSetIter iter(bool complement = false);
    NatSetIter iter(bool complement = false) const;

    void print(FILE* = stdout, unsigned bound = UINT_MAX) const;

  protected:
    int us_size() const;
    NatSet* clone() const;

  private:
    NatSet *own;
};

#endif /* MACHINE_NAT_SET_H */
