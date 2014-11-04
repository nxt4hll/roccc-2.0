/* file "machine/nat_set.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>


#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "machine/nat_set.h"
#endif

#include <machine/substrate.h>
#include <machine/problems.h>
#include <machine/nat_set.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

#define COMPLEMENT true

enum Action { INSERT, REMOVE, NEITHER };

struct Actions {
    bool complement;			// complement lhs before other actions
    Action lhs;				// least element is on lhs
    Action both;			// same element on both sides
    Action rhs;				// least element is on rhs
};

struct ActionTable {
    Actions finite_finite;		// when both lhs and rhs are finite sets
    Actions finite_infinite;		// when only lhs is finite
    Actions infinite_finite;		// when only rhs is finite
    Actions infinite_infinite;		// when neither lhs nor rhs is finite
};

#define INSERT_IF_LHS		{ !COMPLEMENT, INSERT,  NEITHER, NEITHER  }
#define INSERT_IF_RHS		{ !COMPLEMENT, NEITHER, NEITHER, INSERT   }
#define REMOVE_IF_LHS		{ !COMPLEMENT, REMOVE,  NEITHER, NEITHER  }
#define REMOVE_IF_RHS		{ !COMPLEMENT, NEITHER, NEITHER, REMOVE   }
#define INSERT_IF_BOTH		{ !COMPLEMENT, NEITHER, INSERT,  NEITHER  }
#define REMOVE_IF_BOTH		{ !COMPLEMENT, NEITHER, REMOVE,  NEITHER  }
#define COMPLEMENT__INSERT_IF_LHS_OR_BOTH__REMOVE_IF_RHS \
				{  COMPLEMENT, INSERT,  INSERT,  REMOVE   }
#define COMPLEMENT__REMOVE_IF_LHS_OR_BOTH__INSERT_IF_RHS \
				{  COMPLEMENT, REMOVE,  REMOVE,  INSERT   }

ActionTable union_action_table =
{ INSERT_IF_RHS,					// finite-finite
  COMPLEMENT__INSERT_IF_LHS_OR_BOTH__REMOVE_IF_RHS,	// finite-infinite
  INSERT_IF_BOTH,					// infinite-finite
  INSERT_IF_LHS,					// infinite-infinite
};

ActionTable intersection_action_table =
{ REMOVE_IF_LHS,					// finite-finite
  REMOVE_IF_BOTH,					// finite-infinite
  COMPLEMENT__REMOVE_IF_LHS_OR_BOTH__INSERT_IF_RHS,	// infinite-finite
  REMOVE_IF_RHS,					// infinite-infinite
};

ActionTable subtraction_action_table =
{ REMOVE_IF_BOTH,					// finite-finite
  REMOVE_IF_LHS,					// finite-infinite
  REMOVE_IF_RHS,					// infinite-finite
  COMPLEMENT__REMOVE_IF_LHS_OR_BOTH__INSERT_IF_RHS,	// infinite-infinite
};


/*
 * Implement the "combine RHS into LHS" operators (+=, *=, and -=).
 * The action-table argument gives four ways of performing the combination,
 * corresponding the four possible combinations of finite and infinite
 * (i.e., complemented) LHS and RHS sets.
 *
 * The algorithm iterates through the effective merger of the finite
 * sequences underlying LHS, i.e., it scans the complementary sequences if
 * either or both is infinite.  It edits the LHS sequence in place.
 * Depending on whether the element under scan comes from the LHS only, the
 * RHS only, or both at once, it may insert the element into the LHS,
 * remove it from the LHS, or do neither.  In cases where transformation of
 * the LHS must flip it from finite to infinite or vice versa, the
 * algorithm complements the LHS before editing begins.
 */

static void combine_iters(NatSetIter, NatSetIter, Actions&); // helper

void
combine(NatSet &lhs, const NatSet &rhs, ActionTable &action_table)
{
    if (lhs.is_finite()) {
	if (rhs.is_finite()) {
	    claim(!action_table.finite_finite.complement);
	    combine_iters(lhs.iter(), rhs.iter(),
			  action_table.finite_finite);
	}
	else {
	    if (action_table.finite_infinite.complement)
		lhs.complement();
	    combine_iters(lhs.iter(), rhs.iter(COMPLEMENT),
			  action_table.finite_infinite);
	}
    }
    else if (rhs.is_finite()) {
	if (action_table.infinite_finite.complement)
	    lhs.complement();
	combine_iters(lhs.iter(COMPLEMENT), rhs.iter(),
		      action_table.infinite_finite);
    }
    else {
	if (action_table.infinite_infinite.complement)
	    lhs.complement();
	combine_iters(lhs.iter(COMPLEMENT), rhs.iter(COMPLEMENT),
		      action_table.infinite_infinite);
    }
}


/*
 * Iterate through the merger of two sorted sequences and invoke one of
 * three actions, according to whether the current element of the merged
 * sequence comes from the LHS, from both LHS and RHS, or just from the
 * RHS.
 *
 * The possible actions are:
 *
 *   INSERT  the current element into the LHS sequence,
 *   REMOVE  the current element from the LHS sequence, or
 *   NEITHER of the above.
 *
 * After the merger has been processed, complement the LHS set if the input
 * action descriptor calls for it.
 */
void
combine_iters(NatSetIter lhs_it, NatSetIter rhs_it, Actions &actions)
{
    bool no_lhs = !lhs_it.is_valid();
    bool no_rhs = !rhs_it.is_valid();

    unsigned lhs_element, rhs_element;

    if (!no_lhs)
	lhs_element = lhs_it.current();
    if (!no_rhs)
	rhs_element = rhs_it.current();

    while (!no_lhs || !no_rhs) {
	if (no_rhs || (!no_lhs && (lhs_element < rhs_element))) {
	    switch (actions.lhs) {
	      case INSERT:
		lhs_it.insert(lhs_element);
		break;
	      case REMOVE:
		lhs_it.remove(lhs_element);
	      case NEITHER:
		break;
	    }
	    lhs_it.next();
	    if (lhs_it.is_valid())
		lhs_element = lhs_it.current();
	    else
		no_lhs = true;
	}
	else if (no_lhs || (!no_rhs && (rhs_element < lhs_element))) {
	    switch (actions.rhs) {
	      case INSERT:
		lhs_it.insert(rhs_element);
		break;
	      case REMOVE:
		lhs_it.remove(rhs_element);
	      case NEITHER:
		break;
	    }
	    rhs_it.next();
	    if (rhs_it.is_valid())
		rhs_element = rhs_it.current();
	    else
		no_rhs = true;
	}
	else {
	    claim(!no_lhs && !no_rhs && (lhs_element == rhs_element));
	    switch (actions.both) {
	      case INSERT:
		lhs_it.insert(lhs_element);
		break;
	      case REMOVE:
		lhs_it.remove(lhs_element);
	      case NEITHER:
		break;
	    }
	    lhs_it.next();
	    if (lhs_it.is_valid())
		lhs_element = lhs_it.current();
	    else
		no_lhs = true;
	    rhs_it.next();
	    if (rhs_it.is_valid())
		rhs_element = rhs_it.current();
	    else
		no_rhs = true;
	}
    }
}


/*
 *  Generic NatSet assignment.  Concrete classes can override this to use
 *  more efficient copy methods for special cases.
 */
NatSet&
NatSet::operator=(const NatSet &rhs)
{
    bool infinite = !rhs.is_finite();

    for (NatSetIter it = rhs.iter(infinite); it.is_valid(); it.next())
	insert(it.current());

    if (infinite)
	complement();

    return *this;
}

/*
 * Containment on NatSets lhs and rhs:
 *
 * o  If lhs is finite, but rhs is not, return false.
 *
 * o  If lhs is infinite, return true iff no element of ~lhs is
 *    contained in rhs.
 *
 * o  If both lhs and rhs are finite, return true iff every element of
 *    rhs is contained in lhs.
 */
bool
NatSet::contains(const NatSet &rhs) const
{
    if (is_finite())		// lhs finite
    {
	if (!rhs.is_finite())
	    return false;

	NatSetIter rhs_it = rhs.iter();
	for ( ; rhs_it.is_valid(); rhs_it.next())
	    if (!contains(rhs_it.current()))
		return false;
    }
    else			// lhs infinite
    {
	NatSetIter not_lhs_it = this->iter(true);
	for ( ; not_lhs_it.is_valid(); not_lhs_it.next())
	    if (rhs.contains(not_lhs_it.current()))
		return false;
    }
    return true;
}

/*
 * NatSets lhs and rhs overlap iff their intersection is not empty.
 */
bool
NatSet::overlaps(const NatSet &rhs) const
{
    NatSetCopy copy(*this);

    copy *= rhs;
    return !copy.is_empty();
}

/*
 * Equality on NatSets currently exploits the fact that representations
 * are in sorted order.
 */
bool
NatSet::operator==(const NatSet &rhs) const
{
    if ((is_finite() != rhs.is_finite()) || us_size() != rhs.us_size())
	return false;

    bool complement = !is_finite();

    NatSetIter lhs_it = this->iter(complement);
    NatSetIter rhs_it = rhs.  iter(complement);

    while (lhs_it.is_valid()) {
	if (lhs_it.current() != rhs_it.current())
	    return false;
	lhs_it.next();
	rhs_it.next();
    }
    return true;
}


void
NatSet::operator+=(const NatSet &rhs)
{
    combine(*this, rhs, union_action_table);
}

void
NatSet::operator*=(const NatSet &rhs)
{
    combine(*this, rhs, intersection_action_table);
}

void
NatSet::operator-=(const NatSet &rhs)
{
    combine(*this, rhs, subtraction_action_table);
}



/* ---------------  Iterator implementations  --------------- */

class NatSetIterRep : public NatSetIterPure {
  public:
    virtual NatSetIterRep* clone() const = 0;
};

NatSetIter::NatSetIter(NatSet &nat_set)
{
    rep = nat_set.iter().rep;
}

NatSetIter::NatSetIter(const NatSetIter &other)
{
    rep = NULL;
    operator=(other);
}

NatSetIter::~NatSetIter()
{
    delete rep;
}

NatSetIter&
NatSetIter::operator=(const NatSetIter &other)
{
    if (this != &other) {
	delete rep;
	rep = other.rep->clone();
    }
    return *this;
}

unsigned
NatSetIter::current() const
{
    return rep->current();
}

bool
NatSetIter::is_valid() const
{
    return rep->is_valid();
}

void
NatSetIter::next()
{
    rep->next();
}

void
NatSetIter::insert(unsigned element)
{
    rep->insert(element);
}

void
NatSetIter::remove(unsigned element)
{
    rep->remove(element);
}


/*
 *  Generate the elements in the complement of a finite set.  Construct
 *  this from an iterator over the finite set.
 */
class NatSetIterInfinite : public NatSetIterRep
{
  public:
    NatSetIterInfinite(NatSetIterRep *us_it) : us_it(us_it), element(0)
	{ skip(); }
    ~NatSetIterInfinite()		{ delete us_it; }
    unsigned current() const		{ return element; }
    bool is_valid() const		{ return true; }
    void next()				{ element++; skip(); }
    void insert(unsigned e)		{ us_it->remove(e); }
    void remove(unsigned e)		{ us_it->insert(e); }
    NatSetIterRep* clone() const;

  protected:
    NatSetIterInfinite(NatSetIterRep *us_it, unsigned element)
	: us_it(us_it), element(element)
      { }

  private:
    NatSetIterRep *us_it;		// underlying finite-set iterator
    unsigned element;

    void skip() {			// skip elements of the underlying set
	while (us_it->is_valid() && element == us_it->current())
	    { element++; us_it->next(); }
    }
};

NatSetIterRep*
NatSetIterInfinite::clone() const
{
    return new NatSetIterInfinite(us_it->clone(), element);
}


class NatSetIterDense : public NatSetIterRep
{
  public:
    NatSetIterDense(BitVector &us) : us(us), us_it(&us) { }
    NatSetIterDense(BitVector &us, const BitVectorIter &us_it)
	: us(us), us_it(us_it)
      { }
    NatSetIterRep* clone() const    { return new NatSetIterDense(us, us_it); }
				    
    bool is_valid() const
	{ return const_cast<BitVectorIter&>(us_it).is_valid(); }
    unsigned current() const	    { return us_it.current(); }
    void next()			    { us_it.next(); }
    void insert(unsigned element)   { us.set_bit(element, true); }
    void remove(unsigned element)   { us.set_bit(element, false); }

  protected:
    BitVector &us;
    BitVectorIter us_it;
};


class NatSetIterSparse : public NatSetIterRep
{
  public:
    NatSetIterSparse(Set<unsigned> &us) : us(us)
	{ new_it = us.begin(); next(); }
    NatSetIterSparse(Set<unsigned> &us, Set<unsigned>::iterator now_it,
		     Set<unsigned>::iterator new_it)
	: us(us), now_it(now_it), new_it(new_it)
      { }
    NatSetIterRep* clone() const
	{ return new NatSetIterSparse(us, now_it, new_it); }
				    
    bool is_valid() const	    { return now_it != us.end(); }
    unsigned current() const	    { claim(is_valid()); return *now_it; }
    void next()
	{ now_it = new_it; if (is_valid()) new_it++; }
    void insert(unsigned element)   { us.insert(now_it, element); }
    void remove(unsigned element);

  protected:
    Set<unsigned> &us;
    Set<unsigned>::iterator now_it, new_it; // handles on current, next elements
};

void
NatSetIterSparse::remove(unsigned element)
{
    if (is_valid() && (*now_it == element)) {
	claim(now_it != new_it);
	us.erase(now_it);
	now_it = new_it;		// try to detect repeated removal
    } else {
	claim(!is_valid() || element < *now_it);
	us.erase(element);
    }
}
    
/* Print in set notation, but compress runs of consecutive elements.
 * E.g., print {4-7,9-10} instead of {4,5,6,7,9,10}.
 */
void
NatSet::print(FILE *file, unsigned bound) const
{
    NatSetIter it(iter());
    unsigned e1 = bound, e2;
    char separator = '\0';

    fputc('{', file);

    for ( ; it.is_valid() && ((e2 = it.current()) < bound); it.next()) {
	if ((separator != '-' && e1 < bound)
	 || (separator == '-' && e1 != e2 - 1)) {
	    fprintf(file, "%.1s%d", &separator, e1);
	    separator = (e1 == e2 - 1) ? '-' : ',';
	}
	e1 = e2;
    }
    if (e1 < bound)
	fprintf(file, "%.1s%d", &separator, e1);

    fputc('}', file);
}


/* ---------------  NatSetDense implementation  --------------- */

NatSetDense::NatSetDense(bool complement, int size_hint)
{
    if (complement)
	us.set_to_ones();
    if (size_hint != 0)
	accommodate(size_hint - 1);
}

NatSetDense::NatSetDense(const NatSet &rhs)
{
    operator=(rhs);
}

NatSet&
NatSetDense::operator=(const NatSet &rhs)
{
    if (const NatSetDense *rhs_rep = dynamic_cast<const NatSetDense*>(&rhs))
	us = rhs_rep->us;
    else    
	NatSet::operator=(rhs);
    return *this;
}

bool
NatSetDense::is_finite() const
{
    return !us.get_infinity_bit();
}

bool
NatSetDense::is_empty() const
{
    return us.count() == 0;
}

int
NatSetDense::size() const
{
    return us.count();
}

bool
NatSetDense::contains(unsigned element) const
{
    return us.get_bit(element);
}

bool
NatSetDense::contains(const NatSet &rhs) const
{
    if (const NatSetDense *rhs_rep = dynamic_cast<const NatSetDense*>(&rhs))
	return !(~us & rhs_rep->us);
    else    
	return NatSet::contains(rhs);
}

bool
NatSetDense::overlaps(const NatSet &rhs) const
{
    if (const NatSetDense *rhs_rep = dynamic_cast<const NatSetDense*>(&rhs))
	return !!(us & rhs_rep->us);
    else    
	return NatSet::overlaps(rhs);
}

void
NatSetDense::insert(unsigned element)
{
    us.set_bit(element, true);
}

void
NatSetDense::remove(unsigned element)
{
    us.set_bit(element, false);
}

void
NatSetDense::accommodate(unsigned element)
{
    if (us.num_significant_bits() <= element)
	us.set_bit(element, false);
}

void
NatSetDense::insert_all()
{
    us.set_to_ones();
}

void
NatSetDense::remove_all()
{
    us.set_to_zero();
}

void
NatSetDense::complement()
{
    us = us.invert();
}

bool
NatSetDense::operator==(const NatSet &rhs) const
{
    if (const NatSetDense *rhs_rep = dynamic_cast<const NatSetDense*>(&rhs))
	return is_finite() == rhs_rep->is_finite()
	    && us	   == rhs_rep->us;
    else    
	return NatSet::operator==(rhs);
}

void
NatSetDense::operator+=(const NatSet &rhs)
{
    if (const NatSetDense *rhs_rep = dynamic_cast<const NatSetDense*>(&rhs))
	us |= rhs_rep->us;
    else
	NatSet::operator+=(rhs);
}

void
NatSetDense::operator*=(const NatSet &rhs)
{
    if (const NatSetDense *rhs_rep = dynamic_cast<const NatSetDense*>(&rhs))
	us &= rhs_rep->us;
    else
	NatSet::operator*=(rhs);
}

void
NatSetDense::operator-=(const NatSet &rhs)
{
    if (const NatSetDense *rhs_rep = dynamic_cast<const NatSetDense*>(&rhs))
	us &= ~rhs_rep->us;
    else
	NatSet::operator-=(rhs);
}

NatSetIter
NatSetDense::iter(bool complement) const
{
    return const_cast<NatSetDense*>(this)->iter(complement);
}

NatSetIter
NatSetDense::iter(bool complement)
{
    if (complement ^ us.get_infinity_bit())
	return NatSetIter(new NatSetIterInfinite(new NatSetIterDense(us)));
    return NatSetIter(new NatSetIterDense(us));
}

int
NatSetDense::us_size() const
{
    return us.count();
}

NatSet*
NatSetDense::clone() const
{
    NatSetDense *copy = new NatSetDense();

    for (BitVectorIter us_it(&us); us_it.is_valid(); us_it.next())
	copy->insert(us_it.current());

    if (us.get_infinity_bit())
	copy->complement();

    return copy;
}


/* ---------------  NatSetSparse implementation  --------------- */

NatSetSparse::NatSetSparse(const NatSet &rhs)
{
    operator=(rhs);
}

NatSet&
NatSetSparse::operator=(const NatSet &rhs)
{
    if (const NatSetSparse *rhs_rep = dynamic_cast<const NatSetSparse*>(&rhs)) {
	us		= rhs_rep->us;
	is_complemented = rhs_rep->is_complemented;
    } else {
	is_complemented = false;
	NatSet::operator=(rhs);
    }
    return *this;
}

bool
NatSetSparse::is_finite() const
{
    return !is_complemented;
}

bool
NatSetSparse::is_empty() const
{
    return !is_complemented && us.empty();
}

int
NatSetSparse::size() const
{
    claim(!is_complemented);
    return us.size();
}

bool
NatSetSparse::contains(unsigned element) const
{
    if (is_complemented)
	return us.find(element) == us.end();
    return us.find(element) != us.end();
}

void
NatSetSparse::insert(unsigned element)
{
    if (is_complemented)
	us.erase(element);
    us.insert(element);    
}

void
NatSetSparse::remove(unsigned element)
{
    if (is_complemented)
	us.insert(element);
    us.erase(element);    
}

void
NatSetSparse::insert_all()
{
    is_complemented = true;
    us.clear();
}

void
NatSetSparse::remove_all()
{
    is_complemented = false;
    us.clear();
}

void
NatSetSparse::complement()
{
    is_complemented = !is_complemented;
}

bool
NatSetSparse::operator==(const NatSet &rhs) const
{
    if (const NatSetSparse *rhs_rep = dynamic_cast<const NatSetSparse*>(&rhs))
	return this->is_complemented == rhs_rep->is_complemented
	    && this->us		     == rhs_rep->us;
    else    
	return NatSet::operator==(rhs);
}

NatSetIter
NatSetSparse::iter(bool complement) const
{
    return const_cast<NatSetSparse*>(this)->iter(complement);
}

NatSetIter
NatSetSparse::iter(bool complement)
{
    if (complement ^ is_complemented)
	return NatSetIter(new NatSetIterInfinite(new NatSetIterSparse(us)));
    return NatSetIter(new NatSetIterSparse(us));
}

int
NatSetSparse::us_size() const
{
    return us.size();
}

NatSet*
NatSetSparse::clone() const
{
    NatSetSparse *copy = new NatSetSparse();

    for (Set<unsigned>::iterator us_it = us.begin(); us_it != us.end(); ++us_it)
	copy->insert(*us_it);

    if (is_complemented)
	copy->complement();

    return copy;
}


/* ---------------  NatSetCopy implementation  --------------- */


NatSetCopy::NatSetCopy(const NatSet &rhs)
{
    operator=(rhs);
}

NatSet&
NatSetCopy::operator=(const NatSet &rhs)
{
    if (this != &rhs)
	own = rhs.clone();
    return *this;
}

bool
NatSetCopy::is_finite() const
{
    return own->is_finite();
}

bool
NatSetCopy::is_empty() const
{
    return own->is_empty();
}


int
NatSetCopy::size() const
{
    return own->size();
}


bool
NatSetCopy::contains(unsigned element) const
{
    return own->contains(element);
}

bool
NatSetCopy::contains(const NatSet &rhs) const
{
    return own->contains(rhs);
}

bool
NatSetCopy::overlaps(const NatSet &rhs) const
{
    return own->overlaps(rhs);
}

void
NatSetCopy::insert(unsigned element)
{
    return own->insert(element);
}

void
NatSetCopy::remove(unsigned element)
{
    return own->remove(element);
}

void
NatSetCopy::accommodate(unsigned element)
{
    return own->accommodate(element);
}

void
NatSetCopy::insert_all()
{
    return own->insert_all();
}

void
NatSetCopy::remove_all()
{
    return own->remove_all();
}

void
NatSetCopy::complement()
{
    return own->complement();
}

bool NatSetCopy::operator==(const NatSet &rhs) const
{
    return *own == rhs;
}

void
NatSetCopy::operator+=(const NatSet &rhs)
{
    return *own += rhs;
}

void
NatSetCopy::operator*=(const NatSet &rhs)
{
    return *own *= rhs;
}

void
NatSetCopy::operator-=(const NatSet &rhs)
{
    return *own -= rhs;
}

NatSetIter
NatSetCopy::iter(bool complement) const
{
    return own->iter(complement);
}

NatSetIter
NatSetCopy::iter(bool complement)
{
    return own->iter(complement);
}

void
NatSetCopy::print(FILE *file, unsigned bound) const
{
    return own->print(file, bound);
}

int
NatSetCopy::us_size() const
{
    return own->us_size();
}

NatSet*
NatSetCopy::clone() const
{
    return own->clone();
}
