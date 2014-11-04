/* file "halt/note.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef HALT_NOTE_H
#define HALT_NOTE_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "halt/note.h"
#endif

#include <machine/machine.h>

class HaltLabelNote : public Note {
 public:
    HaltLabelNote() : Note(note_list_any()) { }
    HaltLabelNote(long kind, long id) : Note(note_list_any())
	{ _replace(0, kind); _replace(1, id); }
    HaltLabelNote(const HaltLabelNote &other) : Note(other) { }
    HaltLabelNote(const Note &note) : Note(note) { }

    long get_kind() const		     { return _get_c_long(0); }
    void set_kind(long kind)		     { _replace(0, kind); }
    long get_unique_id() const		     { return _get_c_long(1); }
    void set_unique_id(long id)	     { _replace(1, id); }

    int  get_size_static_args(void)	     { return _values_size() - 2; }
    long get_static_arg(int pos)	     { return _get_c_long(pos+2); }
    void set_static_arg(int pos, long value) { _replace(pos + 2, value); }
};

#endif /* HALT_NOTE_H */
