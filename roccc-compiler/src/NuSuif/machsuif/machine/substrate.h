/* file "machine/substrate.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef MACHINE_SUBSTRATE_H
#define MACHINE_SUBSTRATE_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "machine/substrate.h"
#endif

#include <stdlib.h>
#include <set>
#include <map>
#include <vector>
#include <functional>

				// Following suppresses inclusion of <vector>
				// under SGI STL, which defines bit_vector,
				// which conflicts with a basesuif typedef.
//#define __SGI_STL_VECTOR
//#include <vector.h>

#include <common/i_integer.h>
#include <common/formatted.h>
#include <common/suif_vector.h>
#include <common/suif_list.h>
#include <common/suif_map.h>
#include <common/suif_indexed_list.h>
#include <bit_vector/bit_vector.h>
#include <utils/type_utils.h>
#include <utils/symbol_utils.h>
#include <utils/expression_utils.h>
extern "C" void init_utils(SuifEnv*);

#include <suifkernel/suif_env.h>
#include <suifkernel/dll_subsystem.h>
#include <suifkernel/command_line_parsing.h>
#include <basicnodes/basic.h>
#include <basicnodes/basic_constants.h>
#include <suifnodes/suif.h>
#include <typebuilder/type_builder.h>
#include <suifcloning/cloner.h>

#include <basicnodes/basic_factory.h>
#include <suifnodes/suif_factory.h>

class InstrList;

template <class Iterator>
Iterator
before(Iterator iterator)
{
    return --iterator;
}

template <class Iterator>
Iterator
after(Iterator iterator)
{
    return ++iterator;
}

typedef AnnotableObject IrObject;
typedef ProcedureSymbol ProcSym;
typedef ProcedureDefinition ProcDef;
typedef ProcedureDefinition OptUnit;
typedef PointerType PtrType;
typedef Symbol Sym;
typedef SymbolTable SymTable;
typedef SymbolTable ScopeTable;
typedef VariableSymbol VarSym;
typedef VariableDefinition VarDef;
typedef CodeLabelSymbol LabelSym;

typedef Type* TypeId;

class Integer : public IInteger {
  public:
    Integer() { }				// returns the empty string
    Integer(const char *chars)			: IInteger(chars) { }
    Integer(const IInteger &i_integer)		: IInteger(i_integer) { }

    Integer(signed char integral)		: IInteger(integral) { }
    Integer(unsigned char integral)		: IInteger(integral) { }
    Integer(short integral)			: IInteger(integral) { }
    Integer(unsigned short integral)		: IInteger(integral) { }
    Integer(int integral)			: IInteger(integral) { }
    Integer(unsigned int integral)		: IInteger(integral) { }
    Integer(long integral)			: IInteger(integral) { }
    Integer(unsigned long integral)		: IInteger(integral) { }
#ifdef LONGLONG
    Integer(LONGLONG integral)			: IInteger(integral) { }
    Integer(unsigned LONGLONG integral)		: IInteger(integral) { }
#endif
    Integer(const char *initial_string, int base = 10);

    const char* chars() const { return c_string_int(); }
};


class IdString : public LString {
  public:
    IdString() { }				// returns the empty string
    IdString(const IdString &id_string)	: LString(id_string) { }
    IdString(const LString &l_string)	: LString(l_string) { }
    IdString(const String &string)	: LString(string) { }
    IdString(const char *chars)		: LString(chars) { }

    const char* chars() const { return c_str(); }
    bool is_empty()	const { return length() == 0; }
};

extern const IdString empty_id_string;

VarDef* get_def(VarSym*);
ProcSym* get_proc_sym(ProcDef*); 

ValueBlock* get_init(VarDef* d);
int get_bit_alignment(VarDef* d);

int subblocks_size(MultiValueBlock *mvb);

ValueBlock* get_subblock(MultiValueBlock *mvb, int i);

Integer get_subblock_offset(MultiValueBlock *mvb, int i);

int get_repetition_count(RepeatValueBlock *rvb);

ValueBlock* get_subblock(RepeatValueBlock *rvb);

TypeId get_type(ValueBlock *vb);

class Opnd;
Opnd get_value(ExpressionValueBlock *svb);

extern SuifEnv* the_suif_env;

IdString get_name(FileBlock*);
IdString get_name(ProcDef*);
IdString get_name(SymbolTableObject*);

const char* get_class_name(SuifObject*);

IdString get_option_string_value(OptionString*, int pos = 0);

IdString process_file_names(OptionString *file_names);

#define List list
#define Vector std::vector
#define Set std::set
#define Map std::map
#define HashMap suif_hash_map

extern SuifEnv* the_suif_env;

const char* get_class_name(SuifObject*);

#endif /* MACHINE_SUBSTRATE_H */
