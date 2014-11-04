/* file "machine/opnd.cpp" */

/*
   Copyright (c) 2000 The President and Fellows of Harvard College

   All rights reserved.

   This software is provided under the terms described in
   the "machine/copyright.h" include file.
 */

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "machine/opnd.h"
#endif

#include <machine/substrate.h>
#include <machine/machine_ir.h>
#include <machine/machine_ir_factory.h>
#include <machine/init.h>
#include <machine/problems.h>
#include <machine/note.h>
#include <machine/printer.h>
#include <machine/contexts.h>
#include <machine/opnd.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif


FileBlock *the_file_block;
ScopeTable *the_file_scope;

OptUnit *the_local_unit;
ScopeTable *the_local_scope;

int the_vr_count;

Opnd::Opnd()
{
  o = NULL;
}

Opnd::operator bool()
{
  return !is_null(*this);
}

bool
Opnd::operator==(const Opnd &other) const
{
  if (o == other.o)
    return true;
  if (o == NULL )
    return false;
  if (o->get_kind() != other.o->get_kind())
    return false;
  return *o == *other.o;
}

void print_opnd_type(Opnd o) {
  if(is_reg(o)) {
    if(is_hard_reg(o)) {
      std::cout << "(is hard reg)";
    }
    else if(is_virtual_reg(o)) {
      std::cout << "(is virtual reg)[$vr"<<get_reg(o)<<"]";
    }
    else {
      std::cout << "(is undeterminate reg)";
    }
  }
  else if(is_immed(o)) {
    if(is_immed_integer(o)) {
      std::cout << "(is immed integer)";
    }
    else if(is_immed_string(o)) {
      std::cout << "(is immed string)";
    }
    else {
      std::cout << "(is undeterminate immed)";
    }
  }
  else if(is_addr(o)) {
    if(is_addr_sym(o)) {
      FormattedText ft;
      Sym *symbol = get_sym(o);
      symbol->print(ft);
      char* addr_name;
      
      addr_name =  (char*)(symbol->get_name()).c_str();
  
      std::cout << "(is addr sym)["<<addr_name<<"]";
    }
    else if(is_addr_exp(o)) {
      std::cout << "(is addr exp)";
    }
    else {
      std::cout << "(is undeterminate addr)";
    }
  }
  else if(is_var(o)) {
    FormattedText ft;
    VarSym *vsym = get_var(o);
    vsym->print(ft);
    char* var_name;
    
    var_name =  (char*)(vsym->get_name()).c_str();

    std::cout << "(is var)["<<var_name<<"]";
  }
  else if(is_null(o)) {
    std::cout << "(is null)";
  }
  else {
    std::cout << "(I don't know) !!!)";
  }

  return;
}

  TypeId
get_type(Opnd opnd)
{
  if (is_null(opnd))
    return type_v0;
  return ((IrOpnd*)opnd)->get_type();
}

  int
get_kind(Opnd opnd)
{
  if (is_null(opnd))
    return opnd::NONE;
  return ((IrOpnd*)opnd)->get_kind();
}


  Opnd
clone(Opnd opnd)
{
  if (!is_addr_exp(opnd))
    return opnd;
  OpndAddrExp *clone = (to<OpndAddrExp>(opnd))->clone();
  the_local_scope->append_symbol_table_object(clone);
  return clone;
}

  Opnd
opnd_null()
{
  return Opnd();
}

  bool
is_null(Opnd opnd)
{
  return static_cast<IrOpnd*>(opnd) == NULL;
}

  Opnd
opnd_reg(int reg, TypeId type, bool is_virtual)
{
  int min = is_virtual;	// smallest VR number is 1
  claim(reg >= min, "Register number must be greater than %d", min);

  OpndReg *obj = create_opnd_reg(the_suif_env, type, is_virtual ? -reg : reg);
  the_local_scope->append_symbol_table_object(obj);
  return obj;
}


  Opnd
opnd_reg(TypeId type)
{
  int number = next_vr_number();
  OpndReg *obj = create_opnd_reg(the_suif_env, type, number);
  the_local_scope->append_symbol_table_object(obj);
  return obj;
}

  bool
is_reg(Opnd opnd)
{
  return is_kind_of<OpndReg>((IrOpnd*)opnd);
}

  bool
is_hard_reg(Opnd opnd)
{
  return get_kind(opnd) == opnd::REG_HARD;
}

  bool
is_virtual_reg(Opnd opnd)
{
  return get_kind(opnd) == opnd::REG_VIRTUAL;
}

  int
get_reg(Opnd opnd)
{
  IrOpnd *o = opnd;
  claim(is_kind_of<OpndReg>(o), "get_reg: not a register operand");
  int reg = static_cast<OpndReg*>(o)->get_reg();
  return reg < 0 ? -(reg + 1) : reg;
}

  Opnd
opnd_var(VarSym* var)
{
  OpndVar *obj = create_opnd_var(the_suif_env, var);
  the_local_scope->append_symbol_table_object(obj);
  return obj;
}

  bool
is_var(Opnd opnd)
{
  return is_kind_of<OpndVar>((IrOpnd*)opnd);
}

  VarSym*
get_var(Opnd opnd)
{
  IrOpnd *o = opnd;
  claim(is_kind_of<OpndVar>(o), "get_var: not a variable operand");
  return static_cast<OpndVar*>(o)->get_var();
}

char* get_var_name(Opnd var) {
  
  claim(is_var(var), "opnd.cpp::get_var_name:the type must be var!");
  
  
  FormattedText ft;
  VarSym *vsym = get_var(var);
  vsym->print(ft);
  char* var_name;
    
  var_name =  (char*)(vsym->get_name()).c_str();

  return   var_name;
  
}

  Opnd
opnd_immed(int value, TypeId type)
{
  OpndImmedInteger *obj =
    create_opnd_immed_integer(the_suif_env, type, Integer(value));
  the_local_scope->append_symbol_table_object(obj);
  return obj;
}

  Opnd
opnd_immed(Integer value, TypeId type)
{
  OpndImmedInteger *obj =
    create_opnd_immed_integer(the_suif_env, type, value);
  the_local_scope->append_symbol_table_object(obj);
  return obj;
}

  Opnd
opnd_immed(IdString value, TypeId type)
{
  OpndImmedString *obj = create_opnd_immed_string(the_suif_env, type, value);
  the_local_scope->append_symbol_table_object(obj);
  return obj;
}

  Opnd
opnd_immed(IdString value)
{
  OpndImmedString *obj = create_opnd_immed_string(the_suif_env, NULL, value);
  the_local_scope->append_symbol_table_object(obj);
  return obj;
}

  bool
is_immed(Opnd opnd)
{
  return is_immed_integer(opnd) || is_immed_string(opnd);
}

  bool
is_immed_integer(Opnd opnd)
{
  return is_kind_of<OpndImmedInteger>((IrOpnd*)opnd);
}

  bool
is_immed_string(Opnd opnd)
{
  return is_kind_of<OpndImmedString>((IrOpnd*)opnd);
}

  int
get_immed_int(Opnd opnd)
{
  return get_immed_integer(opnd).c_int();
}

  Integer
get_immed_integer(Opnd opnd)
{
  IrOpnd *o = opnd;
  claim(is_kind_of<OpndImmedInteger>(o),
      "get_immed_int: not an integer immediate operand");
  return static_cast<OpndImmedInteger*>(o)->get_immed();
}

  IdString
get_immed_string(Opnd opnd)
{
  IrOpnd *o = opnd;
  claim(is_kind_of<OpndImmedString>(o),
      "get_immed_string: not a string immediate operand");
  return static_cast<OpndImmedString*>(o)->get_immed();
}

  bool
is_addr(Opnd opnd)
{
  return is_addr_sym(opnd) || is_addr_exp(opnd);
}

  TypeId
get_deref_type(Opnd opnd)
{
  if (is_addr_sym(opnd)) {
    Sym *sym = get_sym(opnd);
    if (is_kind_of<LabelSym>(sym))
      return NULL;
    else {
      claim(is_kind_of<VarSym>(sym) || is_kind_of<ProcSym>(sym));
      return sym->get_type();
    }
  }
  claim(is_addr_exp(opnd));

  IrOpnd *o = opnd;
  return static_cast<OpndAddrExp*>(o)->get_deref_type();
}

  void
set_deref_type(Opnd opnd, TypeId type)
{
  claim(is_addr_exp(opnd));
  IrOpnd *o = opnd;
  static_cast<OpndAddrExp*>(o)->set_deref_type(type);
}

  Opnd
opnd_addr_sym(Sym *sym)
{
  OpndAddrSym *obj = create_opnd_addr_sym(the_suif_env, sym, type_addr());
  the_local_scope->append_symbol_table_object(obj);
  return obj;
}
// FIXME
Opnd
opnd_addr_sym(Sym *sym, Type* the_type)
{
  OpndAddrSym *obj = create_opnd_addr_sym(the_suif_env, sym, the_type);
  the_local_scope->append_symbol_table_object(obj);
  return obj;
}


  bool
is_addr_sym(Opnd opnd)
{
  return is_kind_of<OpndAddrSym>((IrOpnd*)opnd);
}

  Sym*
get_sym(Opnd opnd)
{
  IrOpnd *o = opnd;
  claim(is_kind_of<OpndAddrSym>(o), "get_sym: not an address-symbol operand");
  return static_cast<OpndAddrSym*>(o)->get_sym();
}

char* get_addr_name(Opnd addr_sym) {
  
  claim(is_addr_sym(addr_sym), "opnd.cpp::get_addr_name:the type must be addr_sym!");
  
  
  FormattedText ft;
  Sym *symbol = get_sym(addr_sym);
  symbol->print(ft);
  char* addr_name;
  
  addr_name =  (char*)(symbol->get_name()).c_str();
  
  return   addr_name;
  
}

  bool
is_addr_exp(Opnd opnd)
{
  return is_kind_of<OpndAddrExp>((IrOpnd*)opnd);
}

  int
srcs_size(Opnd addr_exp)
{
  claim(is_addr_exp(addr_exp), "srcs_size: not an address expression");
  IrOpnd *o = addr_exp;
  return static_cast<OpndAddrExp*>(o)->get_src_count();
}


AddrExpOpnd::AddrExpOpnd(const Opnd &opnd) :
  Opnd(is_addr_exp(opnd) ? opnd : opnd_null())
{ }

AddrExpOpnd::AddrExpOpnd(Opnd &opnd) :
  Opnd(is_addr_exp(opnd) ? opnd : opnd_null())
{ }

int
AddrExpOpnd::srcs_size() const
{
  return static_cast<OpndAddrExp*>(o)->get_src_count();
}

  OpndHandle
srcs_start(Opnd addr_exp)
{
  claim(is_addr_exp(addr_exp), "srcs_start: not an address expression");
  IrOpnd *o = addr_exp;
  return static_cast<OpndAddrExp*>(o)->srcs().begin();
}

OpndHandle
AddrExpOpnd::srcs_start() const
{
  return static_cast<OpndAddrExp*>(o)->srcs().begin();
}

  OpndHandle
srcs_end(Opnd addr_exp)
{
  claim(is_addr_exp(addr_exp), "srcs_end: not an address expression");
  IrOpnd *o = addr_exp;
  return static_cast<OpndAddrExp*>(o)->srcs().end();
}

OpndHandle
AddrExpOpnd::srcs_end() const
{
  claim(o != NULL, "srcs_end: not an address expression");
  return static_cast<OpndAddrExp*>(o)->srcs().end();
}

TypeId
AddrExpOpnd::get_deref_type() const
{
  claim(o != NULL, "get_deref_type: not an address expression");
  return static_cast<OpndAddrExp*>(o)->get_deref_type();
}

TypeId
AddrExpOpnd::get_type() const
{
  claim(o != NULL, "get_type: not an address expression");
  return static_cast<OpndAddrExp*>(o)->get_type();
}



Opnd
AddrExpOpnd::get_src(int pos) const
{
  claim(o != NULL, "get_src: not an address expression");
  OpndAddrExp *oae = static_cast<OpndAddrExp*>(o);

  for (int d = pos - oae->get_src_count(); d >= 0; --d)
    oae->append_src(opnd_null());
  return oae->get_src(pos);
}

Opnd
AddrExpOpnd::get_src(OpndHandle handle) const
{
  claim(o != NULL, "get_src: not an address expression");
  OpndAddrExp *oae = static_cast<OpndAddrExp*>(o);

  if (handle == oae->srcs().end())
    return opnd_null();
  return *handle;
}

  void
AddrExpOpnd::set_src(int pos, Opnd src)
{
  claim(o != NULL, "set_src: not an address expression");
  OpndAddrExp *oae = static_cast<OpndAddrExp*>(o);

  int d = pos - oae->get_src_count();

  if (d >= 0) {
    for ( ; d > 0; d--)
      oae->append_src(opnd_null());
    oae->append_src(src);
  } else {
    oae->replace_src(pos, src);
  }
}

  void
AddrExpOpnd::set_src(OpndHandle handle, Opnd src)
{
  claim(o != NULL, "set_src: not an address expression");
  OpndAddrExp *oae = static_cast<OpndAddrExp*>(o);


  if (handle == oae->srcs().end())
    oae->append_src(src);
  else
    *handle = src;
}

  Opnd
opnd_addr_exp(int kind)
{
  return opnd_addr_exp(kind, NULL);
}

  Opnd
opnd_addr_exp(int kind, TypeId deref_type)
{
  OpndAddrExp *ae =
    create_opnd_addr_exp(the_suif_env, kind, deref_type);
  the_local_scope->append_symbol_table_object(ae);
  return ae;
}

/* ----------------------------  SymDispOpnd  ---------------------------- */

  bool
is_sym_disp(Opnd opnd)
{
  return get_kind(opnd) == opnd::SYM_DISP;
}

  SymDispOpnd::SymDispOpnd(Opnd addr_sym, Opnd disp, TypeId deref_type)
: AddrExpOpnd(opnd::SYM_DISP, deref_type)
{
  set_src(0, addr_sym);
  set_src(1, disp);
}

SymDispOpnd::SymDispOpnd(const Opnd &opnd) :
  AddrExpOpnd(is_sym_disp(opnd) ? opnd : opnd_null())
{ }

SymDispOpnd::SymDispOpnd(Opnd &opnd) :
  AddrExpOpnd(is_sym_disp(opnd) ? opnd : opnd_null())
{ }

Opnd
SymDispOpnd::get_addr_sym() const
{
  return get_src(0);
}

  void
SymDispOpnd::set_addr_sym(Opnd opnd)
{
  set_src(0, opnd);
}

Opnd
SymDispOpnd::get_disp() const
{
  return get_src(1);
}

  void
SymDispOpnd::set_disp(Opnd opnd)
{
  set_src(1, opnd);
}

/* --------------------------  IndexSymDispOpnd  ------------------------ */

  bool
is_index_sym_disp(Opnd opnd)
{
  return get_kind(opnd) == opnd::INDEX_SYM_DISP;
}

IndexSymDispOpnd::IndexSymDispOpnd(Opnd index, Opnd addr_sym,
    Opnd disp, TypeId deref_type)
: AddrExpOpnd(opnd::INDEX_SYM_DISP, deref_type)
{
  set_src(0, index);
  set_src(1, addr_sym);
  set_src(2, disp);
}

IndexSymDispOpnd::IndexSymDispOpnd(const Opnd &opnd) :
  AddrExpOpnd(is_index_sym_disp(opnd) ? opnd : opnd_null())
{ }

IndexSymDispOpnd::IndexSymDispOpnd(Opnd &opnd) :
  AddrExpOpnd(is_index_sym_disp(opnd) ? opnd : opnd_null())
{ }

Opnd
IndexSymDispOpnd::get_index() const
{
  return get_src(0);
}

  void
IndexSymDispOpnd::set_index(Opnd opnd)
{
  set_src(0, opnd);
}

Opnd
IndexSymDispOpnd::get_addr_sym() const
{
  return get_src(1);
}

  void
IndexSymDispOpnd::set_addr_sym(Opnd opnd)
{
  set_src(1, opnd);
}

Opnd
IndexSymDispOpnd::get_disp() const
{
  return get_src(2);
}

  void
IndexSymDispOpnd::set_disp(Opnd opnd)
{
  set_src(2, opnd);
}


/* ----------------------------  BaseDispOpnd  ---------------------------- */

  bool
is_base_disp(Opnd opnd)
{
  return get_kind(opnd) == opnd::BASE_DISP;
}

  BaseDispOpnd::BaseDispOpnd(Opnd base, Opnd disp, TypeId deref_type)
: AddrExpOpnd(opnd::BASE_DISP, deref_type)
{
  set_src(0, base);
  set_src(1, disp);
}

BaseDispOpnd::BaseDispOpnd(const Opnd &opnd) :
  AddrExpOpnd(is_base_disp(opnd) ? opnd : opnd_null())
{ }

BaseDispOpnd::BaseDispOpnd(Opnd &opnd) :
  AddrExpOpnd(is_base_disp(opnd) ? opnd : opnd_null())
{ }

Opnd
BaseDispOpnd::get_base() const
{
  return get_src(0);
}

  void
BaseDispOpnd::set_base(Opnd opnd)
{
  set_src(0, opnd);
}

Opnd
BaseDispOpnd::get_disp() const
{
  return get_src(1);
}

  void
BaseDispOpnd::set_disp(Opnd opnd)
{
  set_src(1, opnd);
}


/* ----------------------------  BaseIndexOpnd  ---------------------------- */

  bool
is_base_index(Opnd opnd)
{
  return get_kind(opnd) == opnd::BASE_INDEX;
}

  BaseIndexOpnd::BaseIndexOpnd(Opnd base, Opnd index, TypeId deref_type)
: AddrExpOpnd(opnd::BASE_INDEX, deref_type)
{
  set_src(0, base);
  set_src(1, index);
}

BaseIndexOpnd::BaseIndexOpnd(const Opnd &opnd) :
  AddrExpOpnd(is_base_index(opnd) ? opnd : opnd_null())
{ }

BaseIndexOpnd::BaseIndexOpnd(Opnd &opnd) :
  AddrExpOpnd(is_base_index(opnd) ? opnd : opnd_null())
{ }

Opnd
BaseIndexOpnd::get_base() const
{
  return get_src(0);
}

  void
BaseIndexOpnd::set_base(Opnd opnd)
{
  set_src(0, opnd);
}

Opnd
BaseIndexOpnd::get_index() const
{
  return get_src(1);
}

  void
BaseIndexOpnd::set_index(Opnd opnd)
{
  set_src(1, opnd);
}

/* --------------------------  BaseIndexDispOpnd  -------------------------- */

  bool
is_base_index_disp(Opnd opnd)
{
  return get_kind(opnd) == opnd::BASE_INDEX_DISP;
}

BaseIndexDispOpnd::BaseIndexDispOpnd(Opnd base, Opnd index, Opnd disp,
    TypeId deref_type )
: AddrExpOpnd(opnd::BASE_INDEX_DISP, deref_type)
{
  set_src(0, base);
  set_src(1, index);
  set_src(2, disp);
}

BaseIndexDispOpnd::BaseIndexDispOpnd(const Opnd &opnd) :
  AddrExpOpnd(is_base_index_disp(opnd) ? opnd : opnd_null())
{ }

BaseIndexDispOpnd::BaseIndexDispOpnd(Opnd &opnd) :
  AddrExpOpnd(is_base_index_disp(opnd) ? opnd : opnd_null())
{ }

Opnd
BaseIndexDispOpnd::get_base() const
{
  return get_src(0);
}

  void
BaseIndexDispOpnd::set_base(Opnd opnd)
{
  set_src(0, opnd);
}

Opnd
BaseIndexDispOpnd::get_index() const
{
  return get_src(1);
}

  void
BaseIndexDispOpnd::set_index(Opnd opnd)
{
  set_src(1, opnd);
}

Opnd
BaseIndexDispOpnd::get_disp() const
{
  return get_src(2);
}

  void
BaseIndexDispOpnd::set_disp(Opnd opnd)
{
  set_src(2, opnd);
}

/* --------------------------  IndexScaleDispOpnd  ------------------------- */

  bool
is_index_scale_disp(Opnd opnd)
{
  return get_kind(opnd) == opnd::INDEX_SCALE_DISP;
}

IndexScaleDispOpnd::IndexScaleDispOpnd(Opnd index, Opnd scale, Opnd disp,
    TypeId deref_type )
: AddrExpOpnd(opnd::INDEX_SCALE_DISP, deref_type)
{
  set_src(0, index);
  set_src(1, scale);
  set_src(2, disp);
}

IndexScaleDispOpnd::IndexScaleDispOpnd(const Opnd &opnd) :
  AddrExpOpnd(is_index_scale_disp(opnd) ? opnd : opnd_null())
{ }

IndexScaleDispOpnd::IndexScaleDispOpnd(Opnd &opnd) :
  AddrExpOpnd(is_index_scale_disp(opnd) ? opnd : opnd_null())
{ }

Opnd
IndexScaleDispOpnd::get_index() const
{
  return get_src(0);
}

  void
IndexScaleDispOpnd::set_index(Opnd opnd)
{
  set_src(0, opnd);
}

Opnd
IndexScaleDispOpnd::get_scale() const
{
  return get_src(1);
}

  void
IndexScaleDispOpnd::set_scale(Opnd opnd)
{
  set_src(1, opnd);
}

Opnd
IndexScaleDispOpnd::get_disp() const
{
  return get_src(2);
}

  void
IndexScaleDispOpnd::set_disp(Opnd opnd)
{
  set_src(2, opnd);
}

/* ------------------------  BaseIndexScaleDispOpnd  ----------------------- */

  bool
is_base_index_scale_disp(Opnd opnd)
{
  return get_kind(opnd) == opnd::BASE_INDEX_SCALE_DISP;
}

BaseIndexScaleDispOpnd::BaseIndexScaleDispOpnd(Opnd base, Opnd index,
    Opnd scale, Opnd disp,
    TypeId deref_type )
: AddrExpOpnd(opnd::BASE_INDEX_SCALE_DISP, deref_type)
{
  set_src(0, base);
  set_src(1, index);
  set_src(2, scale);
  set_src(3, disp);
}

BaseIndexScaleDispOpnd::BaseIndexScaleDispOpnd(const Opnd &opnd) :
  AddrExpOpnd(is_base_index_scale_disp(opnd) ? opnd : opnd_null())
{ }

BaseIndexScaleDispOpnd::BaseIndexScaleDispOpnd(Opnd &opnd) :
  AddrExpOpnd(is_base_index_scale_disp(opnd) ? opnd : opnd_null())
{ }

Opnd
BaseIndexScaleDispOpnd::get_base() const
{
  return get_src(0);
}

  void
BaseIndexScaleDispOpnd::set_base(Opnd opnd)
{
  set_src(0, opnd);
}

Opnd
BaseIndexScaleDispOpnd::get_index() const
{
  return get_src(1);
}

  void
BaseIndexScaleDispOpnd::set_index(Opnd opnd)
{
  set_src(1, opnd);
}

Opnd
BaseIndexScaleDispOpnd::get_scale() const
{
  return get_src(2);
}

  void
BaseIndexScaleDispOpnd::set_scale(Opnd opnd)
{
  set_src(2, opnd);
}

Opnd
BaseIndexScaleDispOpnd::get_disp() const
{
  return get_src(3);
}

  void
BaseIndexScaleDispOpnd::set_disp(Opnd opnd)
{
  set_src(3, opnd);
}

/* ------------------------------------------------------------------------- */


  static size_t
string_hash(const char *s)
{
  size_t h = 0;
  for (const char *p = s; *p; ++p)
    h += *p;
  return h;
}

  size_t
hash(const Opnd opnd)
{
  if (is_reg(opnd)) {
    int reg = get_reg(opnd);
    if (is_virtual_reg(opnd))
      reg = -(reg + 1);
    return reg << 1;
  } else if (is_var(opnd)) {
    return (size_t)get_var(opnd) | 1;
  } else if (is_immed_integer(opnd)) {
    Integer i = get_immed_integer(opnd);
    if (i.is_c_string_int())
      return string_hash(i.chars());
    else
      return i.c_long();
  } else if (is_immed_string(opnd)) {
    return string_hash(get_immed_string(opnd).chars());
  }
  claim(false, "Operand kind %d isn't yet hashable", get_kind(opnd));
  return 0;
}

  void
fprint(FILE *out, Opnd opnd)
{
  Printer *printer = target_printer();
  printer->set_file_ptr(out);
  printer->print_opnd(opnd);
}

/*
 * Focus on a new FileBlock.  Set the_local_scope in case an operand needs
 * creating before focus has narrowed to a procedure definition.
 */
  void
focus(FileBlock *file_block)
{
  the_file_block = file_block;
  the_file_scope = file_block->get_symbol_table();
  the_local_scope = the_file_scope;
  the_context = target_context(file_block);
}

  void
focus(OptUnit *local_unit)
{
  the_local_unit = local_unit;
  the_local_scope = local_unit->get_symbol_table();

  OneNote<long> note = get_note(local_unit, k_vr_count);
  the_vr_count = is_null(note) ? 0 : note.get_value();
}

  void
defocus(OptUnit *local_unit)
{
  claim(the_local_unit == local_unit);

  OneNote<long> note = get_note(local_unit, k_vr_count);
  if (is_null(note)) {
    note = OneNote<long>(the_vr_count);
    set_note(local_unit, k_vr_count, note);
  } else {
    note.set_value(the_vr_count);
  }
  the_vr_count = 0;
  the_local_scope = NULL;
  the_local_unit = NULL;
}

  int
next_vr_number()
{
  return -(++the_vr_count);
}

  void
audit_opnds(FILE *fp, const char *heading)
{
  int size_reg = 0;
  int size_var = 0;
  int size_immed_integer = 0;
  int size_immed_string = 0;
  int size_addr_sym = 0;
  int size_addr_exp = 0;

  for (Iter<SymbolTableObject*> i =
      the_local_scope->get_symbol_table_object_iterator();
      i.is_valid(); i.next())
  {
    SymbolTableObject *sto = i.current();
    if (is_kind_of<IrOpnd>(sto))
      switch (static_cast<IrOpnd*>(sto)->get_kind())
      {
        case opnd::NONE:
          claim(false, "Unexpected operand kind: opnd::NONE");
        case opnd::REG_HARD:
        case opnd::REG_VIRTUAL:
          size_reg += sizeof(OpndReg);
          break;
        case opnd::VAR:
          size_var += sizeof(OpndVar);
          break;
        case opnd::IMMED_INTEGER:
          size_immed_integer += sizeof(OpndImmedInteger);
          break;
        case opnd::IMMED_STRING:
          size_immed_string += sizeof(OpndImmedString);
          break;
        case opnd::ADDR_SYM:
          size_addr_sym += sizeof(OpndAddrSym);
          break;
        case opnd::SYM_DISP:
        case opnd::INDEX_SYM_DISP:
        case opnd::BASE_DISP:
        case opnd::BASE_INDEX:
        case opnd::BASE_INDEX_DISP:
        case opnd::INDEX_SCALE_DISP:
        case opnd::BASE_INDEX_SCALE_DISP:
          size_addr_sym +=
            sizeof(OpndAddrExp) +
            sizeof(IrOpnd*) *
            static_cast<OpndAddrExp*>(sto)->get_src_count();
          break;
        default:
          claim(false, "Unknown operand kind");
      }
  }
  fprintf(fp, "%s operand storage (bytes):\n", heading);
  fprintf(fp, "  Register:          %8d\n", size_reg);
  fprintf(fp, "  Variable symbol:   %8d\n", size_var);
  fprintf(fp, "  String immediate:  %8d\n", size_immed_string);
  fprintf(fp, "  Integer immediate: %8d\n", size_immed_integer);
  fprintf(fp, "  Address symbol:    %8d\n", size_addr_sym);
  fprintf(fp, "  Address expression:%8d\n", size_addr_exp);
  fprintf(fp, "                     %8d (total)\n",
      size_reg + size_var + size_immed_string + size_immed_integer +
      size_addr_sym + size_addr_exp);
}



/* ----------------------------   OpndCatalog   ---------------------------- */

OpndCatalog::OpndCatalog(bool record, unsigned roll_reserve)
{
  if (record) {
    roll = new Vector<Opnd>;
    roll->reserve(roll_reserve);
  } else {
    roll = NULL;
  }
}

OpndCatalog::~OpndCatalog()
{
  delete roll;
}

/*
 * Print a catalog if the inverse map of operands enrolled has been recorded.
 */

void
OpndCatalog::print(FILE *out) const
{
  if (roll == NULL) return;

  for (int index = 0; index < size(); ++index)
  {
    fprintf(out, "%5d:  ", index);
    fprint (out, (*roll)[index]);
    fprintf(out, "\n");
  }
}

/*
 * Return the operand for position `index' if the catalog has record the
 * inverse map `roll'.  Otherwise, just return a null operand.
 */
Opnd
OpndCatalog::inverse(int index) const
{
  if (roll == NULL || (unsigned)index >= roll->size())
    return opnd_null();

  return (*roll)[index];
}

  void
OpndCatalog::enroll_inverse(unsigned index, Opnd opnd)
{
  if (roll != NULL)
  {
    claim((index == (unsigned)size()) &&
        (index == roll->size()),
        "Direct map is not in sync with inverse map");
    roll->push_back(opnd);
  }
}
