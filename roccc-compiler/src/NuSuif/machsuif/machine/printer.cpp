/* file "machine/printer.cpp" */

/*
   Copyright (c) 2000 The President and Fellows of Harvard College

   All rights reserved.

   This software is provided under the terms described in
   the "machine/copyright.h" include file.
   */

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "machine/printer.h"
#endif

#ifndef CFE_NUMERIC_SYMS_FIXED
#include <ctype.h>
#endif

#include <machine/substrate.h>
#include <machine/machine_ir.h>
#include <machine/init.h>
#include <machine/problems.h>
#include <machine/contexts.h>
#include <machine/util.h>
#include <machine/printer.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

Printer::Printer()
{
  out = NULL;
  Gnum = 512;
  omit_unwanted_notes = true;
}

/* -------- useful default routines -------- */

/* Useful for printing annotations as comments.  Expects that
 * the annotation is a BrickAnnote. */
  void
Printer::print_annote(Annote *annote)
{
  start_comment();

  IdString name = annote->get_name();

  if (name != k_comment)
    fprintf(out, "[%s", name.chars());

  if (is_kind_of<BrickAnnote>(annote)) {
    BrickAnnote *an = (BrickAnnote *)(annote);
    char *separator = ": ";

    for (Iter<SuifBrick*> iter = an->get_brick_iterator();
        iter.is_valid(); iter.next())
    {
      fputs(separator, out);
      separator = ", ";

      SuifBrick *brick = iter.current();
      if (is_a<IntegerBrick>(brick)) {
        Integer i = ((IntegerBrick*)iter.current())->get_value();
        if (i.is_c_string_int())
          fputs(i.chars(), out);
        else
          fprintf(out, "%ld", i.c_long());
      }
      else if (is_a<StringBrick>(brick)) {
        putc('"', out);
        for (const char *p =
            ((StringBrick*)iter.current())->get_value().c_str();
            *p != '\0'; ++p)
        {
          if (*p == '"' || *p == '\\')
            putc('\\', out);
          putc(*p, out);
        }
        putc('"', out);
      }
      else {
        claim(is_a<SuifObjectBrick>(brick));
        SuifObject *so = ((SuifObjectBrick*)brick)->get_object();
        if (is_kind_of<Type>(so))
          fprint(out, (TypeId)so);
        else {
          const char *kind = so ? get_class_name(so) : "NULL";
          fprintf(out, "<<<%s object>>>", kind);
        }
      }
    }
  } else {
    claim(is_kind_of<GeneralAnnote>(annote), "Unexpected kind of Annote");
  }
  if (name != k_comment)
    fputs("]", out);
  fputs("\n", out);
}

/*
 * By default, we print two kinds of FileBlock annotations: first the
 * target-library note and then all of the history notes.
 */
  void
Printer::print_notes(FileBlock *fb)
{
  print_annote(fb->peek_annote(k_target_lib));

  Iter<Annote*> iter(fb->get_annote_iterator());
  while (iter.is_valid()) {
    Annote* an = iter.current();
    if (an->get_name() == k_history)
      print_annote(an);
    iter.next();
  }
}

  void
Printer::print_notes(Instr *instr)
{
  if (instr->get_annote_count() == 0) {
    fprintf(out, "\n");
    return;
  }

  if (Annote *an_comment = instr->peek_annote(k_comment)) {
    // print real comment first
    print_annote(an_comment);
  } else {
    // end current line containing instruction
    fprintf(out, "\n");
  }

  Iter<Annote*> iter(instr->get_annote_iterator());
  while (iter.is_valid()) {
    Annote *an = iter.current();
    if (!omit_unwanted_notes ||
        nonprinting_notes.find(an->get_name()) == nonprinting_notes.end())
      print_annote(an);
    iter.next();
  }
}

  void
Printer::print_sym(Sym *s)
{
  if (s == NULL) {
    fputs("<<null>>", out);
  } else {
    SymTable *st = to<SymTable>(s->get_parent());
    IrObject *par = to<IrObject>(st->get_parent());
    if (is_kind_of<ProcDef>(par)) {
      // We make the local symbol name unique by appending
      // the procedure name as a prefix.
      ProcDef *pd = to<ProcDef>(par);
      fprintf(out, "%s.", get_name(pd).chars());
    } else {
      // We assume that we aren't given nested symbol tables, and
      // thus st must be a global symbol table.  The symbol
      // therefore is global and doesn't need a prefix.
      claim(is_global(st));
#ifndef CFE_NUMERIC_SYMS_FIXED
      // Symbols for literals are coming out of CFE with purely numeric names.
      const char *name = get_name(s).chars();
      if (isdigit(name[0]))
        fprintf(out, "__anon.");
#endif
    }
    fprintf(out, "%s", get_name(s).chars());
  }
}

Printer*
target_printer()
{
  //return ((MachineContext*)the_context)->target_printer();
  return dynamic_cast<MachineContext*>(the_context)->target_printer();
}
