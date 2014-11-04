/* file "machine/code_fin.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef MACHINE_CODE_FIN_H
#define MACHINE_CODE_FIN_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "machine/code_fin.h"
#endif

#include <machine/substrate.h>
#include <machine/machine_ir.h>
#include <machine/note.h>

class StackFrameInfoNote : public Note {
 public:
    StackFrameInfoNote() : Note(note_list_any()) { }
    StackFrameInfoNote(const StackFrameInfoNote &other)
	: Note(other) { }
    StackFrameInfoNote(const Note& note) : Note(note) { }

    bool get_is_leaf() const;
    void set_is_leaf(bool is_leaf);

    bool get_is_varargs() const;
    void set_is_varargs(bool is_varargs);

    int get_frame_size() const;
    void set_frame_size(int frame_size);

    int get_frame_offset() const;
    void set_frame_offset(int frame_offset);

    int get_max_arg_area() const;
    void set_max_arg_area(int max_arg_area);
};

class CodeFin {
  public:
    virtual ~CodeFin() { }

    virtual void init(OptUnit *unit);
    virtual void analyze_opnd(Opnd) = 0;
    virtual void layout_frame() = 0;
    virtual Opnd replace_opnd(Opnd) = 0;
    virtual void make_header_trailer() = 0;
    virtual void finalize() { }

    List<Instr*>& header()  { return _header; }
    List<Instr*>& trailer() { return _trailer; }

  protected:
    // Properties of current procedure, initialized by init()
    //
    OptUnit *cur_unit;

    bool is_leaf;
    bool is_varargs;
    int max_arg_area;

    Map<Sym*,int> frame_map;			// variables -> frame offsets

  private:
    List<Instr*> _header;			// header instructions, reversed
    List<Instr*> _trailer;			// trailer instructions
};

CodeFin *target_code_fin();

#endif /* MACHINE_CODE_FIN_H */
