/* file "machine/opnd.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef MACHINE_OPND_H
#define MACHINE_OPND_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "machine/opnd.h"
#endif

#include <machine/substrate.h>

class IrOpnd;

namespace opnd {

enum { NONE,
       REG_HARD,
       REG_VIRTUAL,
       VAR,
       IMMED_INTEGER,
       IMMED_STRING,
       ADDR_SYM,
       SYM_DISP,
       INDEX_SYM_DISP,
       BASE_DISP,
       BASE_INDEX,
       BASE_INDEX_DISP,
       INDEX_SCALE_DISP,
       BASE_INDEX_SCALE_DISP,
};

} // namespace opnd

#define LAST_OPND_KIND opnd::BASE_INDEX_SCALE_DISP

typedef suif_vector<IrOpnd*>::iterator OpndHandle;

class Opnd {
  public:
    Opnd();
    Opnd(const Opnd &other) { o = other.o; }
    Opnd(IrOpnd *other_o) { o = other_o; }
    Opnd & operator=(const Opnd &other) { o = other.o; return *this; }
    ~Opnd() { }

    bool operator==(const Opnd &other) const;
    bool operator!=(const Opnd &other) const { return !(*this == other); }

    operator IrOpnd*() { return o; }
    operator bool();

  protected:
    IrOpnd *o;
};

void print_opnd_type(Opnd);

int get_kind(Opnd);
TypeId get_type(Opnd);
Opnd clone(Opnd);
size_t hash(Opnd);
void fprint(FILE*, Opnd);

Opnd opnd_null();
bool is_null(Opnd);

Opnd opnd_var(VarSym* var);
bool is_var(Opnd);
VarSym* get_var(Opnd);
char* get_var_name(Opnd var);

Opnd opnd_reg(int reg, TypeId, bool is_virtual = false);
Opnd opnd_reg(TypeId);
bool is_reg(Opnd);
bool is_hard_reg(Opnd);
bool is_virtual_reg(Opnd);
int get_reg(Opnd);

Opnd opnd_immed(int, TypeId);
Opnd opnd_immed(Integer, TypeId);
Opnd opnd_immed(IdString, TypeId);
Opnd opnd_immed(IdString);

bool is_immed(Opnd);
bool is_immed_integer(Opnd);
bool is_immed_string(Opnd);

int get_immed_int(Opnd);
Integer get_immed_integer(Opnd);
IdString get_immed_string(Opnd);

TypeId get_deref_type(Opnd);
void set_deref_type(Opnd, TypeId);
bool is_addr(Opnd);

Opnd opnd_addr_sym(Sym* sym);
Opnd opnd_addr_sym(Sym* sym, Type*);
bool is_addr_sym(Opnd);
Sym* get_sym(Opnd addr_sym);
char* get_addr_name(Opnd addr_sym);

bool is_addr_exp(Opnd);
Opnd opnd_addr_exp(int kind);
Opnd opnd_addr_exp(int kind, TypeId deref_type);

class AddrExpOpnd : public Opnd {
  public:
    AddrExpOpnd() { }
    AddrExpOpnd(const Opnd&);
    AddrExpOpnd(Opnd&);
    AddrExpOpnd(int kind, TypeId deref_type = 0)
	: Opnd(opnd_addr_exp(kind, deref_type)) { }

    TypeId get_deref_type() const;
    TypeId get_type() const;

    int srcs_size() const;
    OpndHandle srcs_start()const ;
    OpndHandle srcs_end() const;
    Opnd get_src(int pos) const;
    Opnd get_src(OpndHandle handle) const;
    void set_src(int pos, Opnd src);
    void set_src(OpndHandle handle, Opnd src);
};

class SymDispOpnd : public AddrExpOpnd {
  public:
    SymDispOpnd(Opnd addr_sym, Opnd disp, TypeId deref_type = 0);
    SymDispOpnd(const Opnd&);
    SymDispOpnd(Opnd&);

    Opnd get_addr_sym() const;
    void set_addr_sym(Opnd);
    Opnd get_disp() const;
    void set_disp(Opnd);
};

bool is_sym_disp(Opnd);

class IndexSymDispOpnd : public AddrExpOpnd {
  public:
    IndexSymDispOpnd(Opnd index, Opnd addr_sym, Opnd disp,
		     TypeId deref_type = 0);
    IndexSymDispOpnd(const Opnd&);
    IndexSymDispOpnd(Opnd&);

    Opnd get_index() const;
    void set_index(Opnd);
    Opnd get_addr_sym() const;
    void set_addr_sym(Opnd);
    Opnd get_disp() const;
    void set_disp(Opnd);
};

bool is_index_sym_disp(Opnd);

class BaseDispOpnd : public AddrExpOpnd {
  public:
    BaseDispOpnd(Opnd base, Opnd disp, TypeId deref_type = 0);
    BaseDispOpnd(const Opnd&);
    BaseDispOpnd(Opnd&);

    Opnd get_base() const;
    void set_base(Opnd);
    Opnd get_disp() const;
    void set_disp(Opnd);
};

bool is_base_disp(Opnd);

class BaseIndexOpnd : public AddrExpOpnd {
  public:
    BaseIndexOpnd(Opnd base, Opnd index, TypeId deref_type = 0);
    BaseIndexOpnd(const Opnd&);
    BaseIndexOpnd(Opnd&);

    Opnd get_base() const;
    void set_base(Opnd);
    Opnd get_index() const;
    void set_index(Opnd);
};

bool is_base_index(Opnd);

class BaseIndexDispOpnd : public AddrExpOpnd {
  public:
    BaseIndexDispOpnd(Opnd base, Opnd index, Opnd disp, TypeId deref_type = 0);
    BaseIndexDispOpnd(const Opnd&);
    BaseIndexDispOpnd(Opnd&);

    Opnd get_base() const;
    void set_base(Opnd);
    Opnd get_index() const;
    void set_index(Opnd);
    Opnd get_disp() const;
    void set_disp(Opnd);
};

bool is_base_index_disp(Opnd);

class IndexScaleDispOpnd : public AddrExpOpnd {
  public:
    IndexScaleDispOpnd(Opnd index, Opnd scale, Opnd disp,
		       TypeId deref_type = 0);
    IndexScaleDispOpnd(const Opnd&);
    IndexScaleDispOpnd(Opnd&);

    Opnd get_index() const;
    void set_index(Opnd);
    Opnd get_scale() const;
    void set_scale(Opnd);
    Opnd get_disp() const;
    void set_disp(Opnd);
};

bool is_index_scale_disp(Opnd);

class BaseIndexScaleDispOpnd : public AddrExpOpnd {
  public:
    BaseIndexScaleDispOpnd(Opnd base, Opnd index, Opnd scale, Opnd disp,
			   TypeId deref_type = 0);
    BaseIndexScaleDispOpnd(const Opnd&);
    BaseIndexScaleDispOpnd(Opnd&);

    Opnd get_base() const;
    void set_base(Opnd);
    Opnd get_index() const;
    void set_index(Opnd);
    Opnd get_scale() const;
    void set_scale(Opnd);
    Opnd get_disp() const;
    void set_disp(Opnd);
};

bool is_base_index_scale_disp(Opnd);

void focus(FileBlock*);
void focus(OptUnit*);

void defocus(OptUnit*);

extern FileBlock *the_file_block;
extern ScopeTable *the_file_scope;

extern OptUnit *the_local_unit;
extern ScopeTable *the_local_scope;

void audit_opnds(FILE*, const char *heading);

extern int the_vr_count;

int next_vr_number();

class OpndCatalog {
  public:
    virtual ~OpndCatalog();

    virtual int size() const = 0;
	    int num_slots() const { return size(); }	// deprecated

    virtual bool enroll(Opnd, int *index = NULL) = 0;
    virtual bool lookup(Opnd, int *index = NULL) const = 0;

    virtual void print(FILE* = stdout) const;
    virtual Opnd inverse(int index) const;

    protected:
      OpndCatalog(bool record = false, unsigned roll_reserve = 100);

      void enroll_inverse(unsigned index, Opnd);

    private:
      Vector<Opnd> *roll;
};

#endif /* MACHINE_OPND_H */
