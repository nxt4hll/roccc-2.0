/* file "machine/note.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "machine/note.h"
#endif

#include <machine/substrate.h>
#include <machine/machine_ir_factory.h>
#include <machine/problems.h>
#include <machine/init.h>
#include <machine/nat_set.h>
#include <machine/util.h>
#include <machine/note.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

/*
 *  Public Note methods
 */

// Constructor for a ``null'' note
Note::Note()
{
    annote = NULL;
    ref_count = NULL;
}

Note::Note(const Note &other)
{
    annote = other.annote;
    if ((ref_count = other.ref_count))
	++*ref_count;
}

Note::~Note()
{
    clear();
}

const Note&
Note::operator=(const Note &other)
{
    if (this != &other) {
	clear();
	annote = other.annote;
	if ((ref_count = other.ref_count))
	    ++*ref_count;
    }
    return *this;
}

bool
Note::operator==(const Note &other) const
{
    if (_values_size() != other._values_size())
	return false;

    for (int i = _values_size() - 1; i >= 0; --i) {
        SuifBrick *b1, *b2;
	b1 = to<BrickAnnote>(annote)->get_brick(i),
	b2 = to<BrickAnnote>(other.annote)->get_brick(i);

	// continue only if bricks are of the same type
	if (b1->get_class_name() != b2->get_class_name())
	    return false;

	// compare the values of the two bricks
	if (is_a<IntegerBrick>(b1)) {
	    if (to<IntegerBrick>(b1)->get_value() !=
		to<IntegerBrick>(b2)->get_value())
		return false;
	} else if (is_a<StringBrick>(b1)) {
	    if (to<StringBrick>(b1)->get_value() !=
		to<StringBrick>(b2)->get_value())
		return false;
	} else if (is_a<SuifObjectBrick>(b1)) {
	    if (to<SuifObjectBrick>(b1)->get_object() != 
		to<SuifObjectBrick>(b2)->get_object())
		return false;
	}
    }
    return true;    
}


/*
 *  Protected Note methods
 */

// Constructor for a ``non-null'' notes used by creator functions

enum { ORPHAN = false, OWNED = true };

Note::Note(Annote *annote, bool owned)
{
    if (annote) {
	claim(is_a<BrickAnnote>(annote) ||
	      is_a<GeneralAnnote>(annote));
	this->annote = annote;
	if (owned)
	    this->ref_count = NULL;
	else
	    *(this->ref_count = new int) = 1;
    } else {
	this->annote = NULL;
	this->ref_count = NULL;
    }
}

void
Note::clear()
{
    if (ref_count && (--*ref_count == 0)) {
	delete annote;
	delete ref_count;
	annote = NULL;
	ref_count = NULL;
    }
}


int
Note::_values_size() const
{
    if (annote && is_a<BrickAnnote>(annote))
	return to<BrickAnnote>(annote)->get_brick_count();
    return 0;	// default for null and flag notes
}


void
Note::_append(long value)
{
    _append(Integer(value));
}

void
Note::_append(Integer value)
{
    claim(annote);
    claim(is_a<BrickAnnote>(annote));
    IntegerBrick *brick = create_integer_brick(the_suif_env, value);
    to<BrickAnnote>(annote)->append_brick(brick);
}

void
Note::_append(IdString value)
{
    claim(annote);
    claim(is_a<BrickAnnote>(annote));
    StringBrick *brick = create_string_brick(the_suif_env, value);
    to<BrickAnnote>(annote)->append_brick(brick);
}
void
Note::_append(IrObject *value)
{
    claim(annote);
    claim(is_a<BrickAnnote>(annote));
    SuifObjectBrick *brick = create_suif_object_brick(the_suif_env, value);
    to<BrickAnnote>(annote)->append_brick(brick);
}


void
Note::_insert_before(int pos, long value)
{
    _insert_before(pos, Integer(value));
}

void
Note::_insert_before(int pos, Integer value)
{
    claim(annote);
    claim(is_a<BrickAnnote>(annote));
    IntegerBrick *brick = create_integer_brick(the_suif_env, value);
    to<BrickAnnote>(annote)->insert_brick(pos, brick);
}

void
Note::_insert_before(int pos, IdString value)
{
    claim(annote);
    claim(is_a<BrickAnnote>(annote));
    StringBrick *brick = create_string_brick(the_suif_env, value);
    to<BrickAnnote>(annote)->insert_brick(pos, brick);
}

void
Note::_insert_before(int pos, IrObject *value)
{
    claim(annote);
    claim(is_a<BrickAnnote>(annote));
    SuifObjectBrick *brick = create_suif_object_brick(the_suif_env, value);
    to<BrickAnnote>(annote)->insert_brick(pos, brick);
}


static void
replace(Annote *an, int pos, SuifBrick *brick)
{
    claim(an);
    BrickAnnote *annote = to<BrickAnnote>(an);
    int delta = pos - annote->get_brick_count();

    if (delta >= 0) {
	for (; delta > 0; --delta)
	    annote->append_brick(NULL);
	annote->append_brick(brick);
    }
    else {
	annote->insert_brick(pos, brick);
	delete annote->remove_brick(pos + 1);
    }
}
    
void
Note::_replace(int pos, long value)
{
    _replace(pos, Integer(value));
}

void
Note::_replace(int pos, Integer value)
{
    replace(annote, pos, create_integer_brick(the_suif_env, value));
}

void
Note::_replace(int pos, IdString value)
{
    replace(annote, pos, create_string_brick(the_suif_env, value));
}

void
Note::_replace(int pos, IrObject *value)
{
    replace(annote, pos, create_suif_object_brick(the_suif_env, value));
}


void
Note::_remove(int pos)
{
    claim(annote);
    BrickAnnote *an = to<BrickAnnote>(annote);
    if (pos < an->get_brick_count())
	delete an->remove_brick(pos);
}


long
Note::_get_c_long(int pos) const
{
    claim(annote);
    BrickAnnote *an = to<BrickAnnote>(annote);
    claim(pos < an->get_brick_count(), "Note has no item #%d", pos);
    return _get_integer(pos).c_long();
}

Integer
Note::_get_integer(int pos) const
{
    claim(annote);
    BrickAnnote *an = to<BrickAnnote>(annote);
    claim(pos < an->get_brick_count(), "Note has no item #%d", pos);
    return to<IntegerBrick>(an->get_brick(pos))->get_value();
}

IdString
Note::_get_string(int pos) const
{
    claim(annote);
    BrickAnnote *an = to<BrickAnnote>(annote);
    claim(pos < an->get_brick_count(), "Note has no item #%d", pos);
    return IdString(to<StringBrick>(an->get_brick(pos))->get_value());
}

IrObject*
Note::_get_ir_object(int pos) const
{
    claim(annote);
    BrickAnnote *an = to<BrickAnnote>(annote);
    claim(pos < an->get_brick_count(), "Note has no item #%d", pos);
    return to<IrObject>(to<SuifObjectBrick>(an->get_brick(pos))->get_object());
}


/*
 *  MbrNote methods
 */
int
MbrNote::get_case_count() const
{
    claim(annote);

    int nbricks = to<BrickAnnote>(annote)->get_brick_count();
    claim(nbricks % 2 == 0);

    return (nbricks - 2) >> 1;
}


/*
 *  Creator functions
 */

Note
note_null()
{
    return Note();
}

Note
note_flag()
{
    GeneralAnnote *annote =
	create_general_annote(the_suif_env, empty_id_string);
    return Note(annote, ORPHAN);
}

Note
note_list_any()
{
    BrickAnnote *annote = create_brick_annote(the_suif_env, empty_id_string);
    return Note(annote, ORPHAN);
}


/*
 * OPI functions involving Note
 */

Note
clone(const Note& note)
{
    return Note(deep_clone(note.annote), ORPHAN);
}

bool
is_null(const Note& note)
{
    return !(bool)note;			// invoke Note::operator bool()
}

bool
has_notes(IrObject* obj)
{
    return obj->get_annote_count() > 0;
}

bool
has_note(IrObject* obj, NoteKey key)
{
    return obj->peek_annote(key) != NULL;
}

Note
get_note(IrObject* obj, NoteKey key)
{
    return Note(obj->peek_annote(key), OWNED);
}

Note
take_note(IrObject* obj, NoteKey key)
{
    Annote *annote = obj->take_annote(key);
    if (annote == NULL)
	return Note();
    return Note(annote, ORPHAN);
}

void
set_note(IrObject* object, NoteKey key, const Note &note)
{
    claim(note.annote, "Can't attach a null annotation");
    note.annote->set_name(key);
    object->append_annote(note.annote);
    delete note.ref_count;
    note.ref_count = NULL;
}

/*
 * The NatSetDense value is represented as an Integer.  Put a zero Integer
 * in place to stand for the empty set.
 */
NatSetNote::NatSetNote(bool dense) : Note(note_list_any())
{
    if (dense) {
	_append(k_dense);
	_append(Integer(0));
    } else {
	_append(k_sparse);
    }
}

/*
 * The NatSetDense value is represented as an Integer.  Put a zero Integer
 * in place to stand for the empty set.
 */
NatSetNote::NatSetNote(const NatSet *nat_set, bool dense) : Note(note_list_any())
{
    if (dense) {
	_append(k_dense);
	_append(Integer(0));
    } else {
	_append(k_sparse);
    }
    set_set(nat_set);
}

/*
 * The following helpers work around a bug in Integer::from_i_integer.
 * Printing a long-sized Integer in a base other than 10 is broken.
 */
static void
write_nat(long i, char *&buffer_ptr, int base)
{
    if (i != 0) {
	long digit = i % base;
	write_nat(i / base, buffer_ptr, base);
	*buffer_ptr++ = ((digit <= 9) ? '0' : ('a' - 10)) + digit;
    }
    *buffer_ptr = '\0';
}

static BitVector
integer_to_bit_vector(Integer i)
{
    if (i < 0)
	return integer_to_bit_vector(~i).invert();

    BitVector bv;

    if (i != 0) {
	size_t length = i.written_length(16).c_size_t();
	char *buffer = new char[length + 3];
	char *p = buffer;

	*p++ = '0';
	*p++ = 'x';
	if (i.is_c_long())
	    write_nat(i.c_long(), p, 16);
	else
	    i.write(p, 16);	// FIXME: this is also broken

	bv.read(buffer);
	delete[] buffer;
    }
    return bv;
}

void
NatSetNote::get_set(NatSet *nat_set) const
{
    nat_set->remove_all();

    int value_count = _values_size();
    claim(value_count > 0, "Bad NatSetNote");

    IdString tag = _get_string(0);

    if (tag == k_dense || tag == k_dense_inverse) {
	claim(value_count == 2, "Bad dense-form NatSetNote: wrong brick count");

	// FIXME: this is a workaround for a bug in Integer::from_i_integer().
	BitVector us = integer_to_bit_vector(_get_integer(1));
	claim(!us.get_infinity_bit(), "Bad dense-form NatSetNote: wrong sign");

	for (BitVectorIter bit = BitVectorIter(&us); bit.is_valid(); bit.next())
	    nat_set->insert(bit.current());
    } else {
	claim(tag == k_sparse || tag == k_sparse_inverse, "Bad NatSetNote tag");
	for (int i = 1; i < value_count; ++i)
	    nat_set->insert(_get_integer(i).c_unsigned_long());
    }
    if (tag == k_dense_inverse || tag == k_sparse_inverse)
	nat_set->complement();
}

void
NatSetNote::set_set(const NatSet *nat_set)
{
    bool finite = nat_set->is_finite();
    int value_count = _values_size();
    claim(value_count > 0, "Bad NatSetNote: wrong brick count");

    IdString tag = _get_string(0);

    if (tag == k_dense || tag == k_dense_inverse) {
	claim(value_count == 2, "Bad dense-form NatSetNote: wrong brick count");

	BitVector us;
	_replace(0, finite ? k_dense : k_dense_inverse);
	for (NatSetIter it = nat_set->iter(!finite); it.is_valid(); it.next())
	    us.set_bit(it.current(), true);
	_replace(1, us.to_i_integer());
    } else {
	claim(tag == k_sparse || tag == k_sparse_inverse, "Bad NatSetNote tag");
	while (value_count > 1)
	    _remove(--value_count);
	_replace(0, finite ? k_sparse : k_sparse_inverse);
	for (NatSetIter it = nat_set->iter(!finite); it.is_valid(); it.next())
	    _append(it.current());
    }
}
