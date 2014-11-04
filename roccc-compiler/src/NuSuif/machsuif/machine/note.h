/* file "machine/note.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef MACHINE_NOTE_H
#define MACHINE_NOTE_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "machine/note.h"
#endif

#include <machine/substrate.h>
#include <machine/nat_set.h>

typedef IdString NoteKey;

class Note {
  public:
    Note();				// null-note constructor
    Note(const Note&);			// copy constructor
    ~Note();

    const Note& operator=(const Note&);
    bool operator==(const Note&) const;
    operator bool() const { return annote != NULL; }

  protected:

    Annote *annote;			// underlying SUIF annotation
    mutable int *ref_count;		// nullified by set_note

    friend Note clone(const Note&);
    friend Note get_note (IrObject*, NoteKey);
    friend Note take_note(IrObject*, NoteKey);
    friend void set_note (IrObject*, NoteKey, const Note&);
    friend Note note_flag();
    friend Note note_list_any();

    // Protected constructor, only invoked by friends.
    Note(Annote*, bool owned);

    void clear();

    // Methods that access the value sequence of a BrickAnnote
    int _values_size() const;

    void _insert_before(int pos, long value);
    void _insert_before(int pos, Integer value);
    void _insert_before(int pos, IdString value);
    void _insert_before(int pos, IrObject *value);

    void _append(long value);
    void _append(Integer value);
    void _append(IdString value);
    void _append(IrObject *value);

    void _replace(int pos, long value);
    void _replace(int pos, Integer value);
    void _replace(int pos, IdString value);
    void _replace(int pos, IrObject *value);

    void _remove(int pos);

    long _get_value(int pos, long const&) const
        { return _get_c_long(pos); }
    Integer _get_value(int pos, Integer const&) const
        { return _get_integer(pos); }
    IdString _get_value(int pos, IdString const&) const
        { return _get_string(pos); }
    IrObject* _get_value(int pos, IrObject* const&) const
        { return _get_ir_object(pos); }

    long      _get_c_long(int pos) const;
    Integer   _get_integer(int pos) const;
    IdString  _get_string(int pos) const;
    IrObject* _get_ir_object(int pos) const;
};

Note note_null();
bool is_null(const Note&);

Note note_flag();

Note note_list_any();	// used only by Note classes

bool has_notes(IrObject*);
bool has_note (IrObject*, NoteKey);
Note get_note (IrObject*, NoteKey);
Note take_note(IrObject*, NoteKey);
void set_note (IrObject*, NoteKey, const Note&);

template <class ValueType>
class OneNote : public Note {
 public:
    OneNote() : Note(note_list_any()) { _replace(0, ValueType()); }
    OneNote(ValueType value) : Note(note_list_any()) { set_value(value); }
    OneNote(const OneNote<ValueType> &other) : Note(other) { }
    OneNote(const Note &note) : Note(note) { }

    ValueType get_value() const
        { return _get_value(0, ValueType()); }
    void set_value(ValueType value)
        { _replace(0, value); }
};

template <class ValueType>
class ListNote : public Note {
  public:
    ListNote() : Note(note_list_any()) { }
    ListNote(const ListNote<ValueType> &other) : Note(other) { }
    ListNote(const Note &note) : Note(note) { }

    int values_size() const
        { return _values_size(); }
    ValueType get_value(int pos) const
        { return _get_value(pos, ValueType()); }
    void set_value(int pos, ValueType value)
        { _replace(pos, value); }
    void append(ValueType value)
        { _append(value); }
    void insert_before(int pos, ValueType value)
        { _insert_before(pos, value); }
    void remove(int pos)
        { _remove(pos); }
};

class LineNote : public Note {
 public:
    LineNote() : Note(note_list_any()) { }
    LineNote(const LineNote &other) : Note(other) { }
    LineNote(const Note &note) : Note(note) { }

    int get_line() const	{ return _get_c_long(0); }
    void set_line(int line)	{ _replace(0, line); }
    IdString get_file() const	{ return _get_string(1); }
    void set_file(IdString file){ _replace(1, file); }
};

class MbrNote : public Note {
 public:
    MbrNote() : Note(note_list_any()) { }
    MbrNote(const MbrNote &other) : Note(other) { }
    MbrNote(const Note &note) : Note(note) { }

    int get_case_count() const;
    VarSym* get_table_sym() const
        { return to<VarSym>(_get_ir_object(0)); }
    void set_table_sym(VarSym* var)
        { _replace(0, var); }

    LabelSym* get_default_label() const
        { return to<LabelSym>(_get_ir_object(1)); }
    void set_default_label(LabelSym* label)
        { _replace(1, label); }

    int get_case_constant(int pos) const
        { return _get_c_long((pos << 1) + 2); }
    void set_case_constant(int constant, int pos)
        { _replace((pos << 1) + 2, constant); }

    LabelSym* get_case_label(int pos) const
        { return to<LabelSym>(_get_ir_object((pos << 1) + 3)); }
    void set_case_label(LabelSym* label, int pos)
        { _replace((pos << 1) + 3, label); }
};

class NatSetNote : public Note {
  public:
    NatSetNote(bool dense = false);
    NatSetNote(const NatSet*, bool dense = false);
    NatSetNote(const NatSetNote &other) : Note(other) { }
    NatSetNote(const Note &note) : Note(note) { }

    void get_set(NatSet*) const;
    void set_set(const NatSet*);
};

#endif /* MACHINE_NOTE_H */
