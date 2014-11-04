/* file "machine/util.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef MACHINE_UTIL_H
#define MACHINE_UTIL_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "machine/util.h"
#endif

#include <machine/substrate.h>
#include <machine/machine_ir.h>
#include <machine/opnd.h>
#include <machine/note.h>

class OpndFilter {
  public:
    OpndFilter(bool thru_addr_exps = true)
	{ _thru_addr_exps = thru_addr_exps; }
    virtual ~OpndFilter() {}

    typedef enum { IN, OUT } InOrOut;

    virtual Opnd operator()(Opnd, InOrOut) = 0;

    bool thru_addr_exps() const { return _thru_addr_exps; }
  protected:
    bool _thru_addr_exps;
};

void map_opnds(Instr *instr, OpndFilter &filter);
void map_opnds(Opnd addr_exp, OpndFilter &filter);
void map_src_opnds(Instr *instr, OpndFilter &filter);
void map_dst_opnds(Instr *instr, OpndFilter &filter);

bool is_reg_param(VarSym*);
int get_param_reg(VarSym*);
void set_param_reg(VarSym*, int reg);

int get_formal_param_count(OptUnit*);
VarSym* get_formal_param(OptUnit*, int pos);

extern void move_notes(IrObject *from, IrObject *to);
extern void copy_notes(IrObject *from, IrObject *to);

extern Set<IdString> nonprinting_notes;

bool is_global(Sym*);
bool is_external(Sym*);
bool is_defined(Sym*);
bool is_private(Sym*);
bool is_auto(VarSym*);

VarSym* lookup_local_var(IdString name);
VarSym* lookup_external_var(IdString name);
ProcSym* lookup_external_proc(IdString name);

VarSym* new_named_var(TypeId, IdString name);
VarSym* new_unique_var(TypeId, const char *prefix = "_var");
VarSym* new_unique_var(Opnd init, const char *prefix = "_var");
LabelSym* new_unique_label(const char *prefix = "_label");

VarSym* new_empty_table_var(TypeId elem_type, int length);
VarSym* new_dispatch_table_var(MbrNote&);

ProcSym* find_proc_sym(TypeId, IdString name);

TypeId get_type(VarSym*);
void set_type(VarSym*, TypeId);

TypeId get_type(ProcSym*);
void set_type(ProcSym*, TypeId);

bool is_addr_taken(VarSym*);
void set_addr_taken(VarSym*, bool);

void update_dispatch_table_var(MbrNote&, int index);

void strip_dispatch_table_var(MbrNote&);

bool is_void(TypeId);
bool is_scalar(TypeId);		// data type other than array or record
bool is_boolean(TypeId);
bool is_integral(TypeId);
bool is_signed(TypeId);		// apply only to an integral type
bool is_floating_point(TypeId);
bool is_pointer(TypeId);
bool is_enum(TypeId);
bool is_record(TypeId);		// e.g., struct or union type
bool is_struct(TypeId);
bool is_union(TypeId);
bool is_array(TypeId);

int get_bit_size(TypeId);
int get_bit_alignment(TypeId);

TypeId pointer_type(TypeId referent);
TypeId get_referent_type(TypeId);

TypeId array_type(TypeId element_type, int lower_bound, int upper_bound);
TypeId get_element_type(TypeId array_type);

TypeId get_result_type(TypeId type);

TypeId find_proc_type(TypeId result, TypeId arg0, TypeId arg1);

bool is_global(SymTable*);
bool is_external(SymTable*);
bool is_private(SymTable*);

SymTable* external_sym_table();
SymTable* file_set_sym_table();
SymTable* get_sym_table(FileBlock*);
SymTable* get_sym_table(ProcDef*);

template<class T>
T*
deep_clone(T *object)
{
    return to<T>(object->deep_clone(the_suif_env));
}

size_t hash(const unsigned long);

void fprint(FILE*, IrObject*);
void fprint(FILE*, Integer);

template <class Item>
void clear(list<Item> &l)
{
    l.clear_list();
}

template <class Item>
typename list<Item>::iterator
get_last_handle(list<Item> &l)
{
    return l.get_nth(l.size() - 1);
}

template <class Item>
typename list<Item>::iterator
find(list<Item> &l, const Item &item)
{
    for (typename list<Item>::iterator h = l.begin(); h != l.end(); ++h)
	if (*h == item)
	    return h;
    return l.end();
}

template <class Item>
bool
contains(list<Item> &l, const Item &item)
{
    return find(l, item) != l.end();
}

template <class Item>
void
maybe_expand(Vector<Item> &v, size_t index, const Item &init)
{
    if (index >= v.size())
	v.resize(index + 1, init);
}

template <class Item>
void
end_splice(List<Item> &l, List<Item> &x)
{
    while (!x.empty())
    {
	l.insert(l.end(), x.front());
	x.pop_front();
    }
}

AnyBody* get_body(OptUnit*);
void set_body(OptUnit*, AnyBody*);

#endif /* MACHINE_UTIL_H */
