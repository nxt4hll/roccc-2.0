/* file "machine/substrate.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "machine/substrate.h"
#endif

#include <machine/problems.h>
#include <machine/opnd.h>
#include <machine/types.h>
#include <machine/substrate.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

SuifEnv *the_suif_env;

const IdString empty_id_string;

const char*
get_class_name(SuifObject *object)
{
    return object->getClassName().c_str();
}

IdString
get_name(FileBlock *fb)
{
    return fb->get_source_file_name();
}

IdString
get_name(ProcDef *pd)
{
    return pd->get_procedure_symbol()->get_name();
}

IdString
get_name(SymbolTableObject *sto)
{
    return sto->get_name();
}

ProcSym*
get_proc_sym(ProcDef *pd)
{
    return pd->get_procedure_symbol();
}

VarDef*
get_def(VarSym *s)
{
    return s->get_definition();
}

ValueBlock*
get_init(VarDef *d)
{
    return d->get_initialization();
}

int
get_bit_alignment(VarDef *d)
{
    return d->get_bit_alignment();
}


bool
is_undefined_value_block(ValueBlock *vb)
{
    return is_kind_of<UndefinedValueBlock>(vb);
}
bool
is_multi_value_block(ValueBlock *vb)
{
    return is_kind_of<MultiValueBlock>(vb);
}
bool
is_repeat_value_block(ValueBlock *vb)
{
    return is_kind_of<RepeatValueBlock>(vb);
}
bool
is_simple_value_block(ValueBlock *vb)
{
    return is_kind_of<ExpressionValueBlock>(vb);
}

// helpers for multi-value blocks
int
subblocks_size(MultiValueBlock *mvb)
{
    return mvb->get_sub_block_count();
}

ValueBlock*
get_subblock(MultiValueBlock *mvb, int i)
{
    return mvb->get_sub_block(i).second;
}

Integer
get_subblock_offset(MultiValueBlock *mvb, int i)
{
    return mvb->get_sub_block(i).first;
}

int
get_repetition_count(RepeatValueBlock *rvb)
{
    return rvb->get_num_repetitions();
}

ValueBlock*
get_subblock(RepeatValueBlock *rvb)
{
    return rvb->get_sub_block();
}

// helpers for value blocks
TypeId
get_type(ValueBlock *vb)
{
    return vb->get_type();
}

Opnd
get_value(ExpressionValueBlock *svb)
{
    Expression *e = svb->get_expression();

    if (is_kind_of<LoadVariableExpression>(e)) {
	return opnd_var(((LoadVariableExpression*)e)->get_source());

    } else if (is_kind_of<IntConstant>(e)) {
	IntConstant *c = (IntConstant *)e;
	claim(c->get_annote_count() == 0);
	return opnd_immed(c->get_value(), c->get_result_type());

    } else if (is_kind_of<FloatConstant>(e)) {
	FloatConstant *c = (FloatConstant *)e;
	claim(c->get_annote_count() == 0);
	return opnd_immed(c->get_value(), c->get_result_type());

    } else if (is_kind_of<SymbolAddressExpression>(e)) {
	SymbolAddressExpression *in = (SymbolAddressExpression *)e;
	return opnd_addr_sym(in->get_addressed_symbol());

    } else if (is_a<UnaryExpression>(e)) {
	UnaryExpression *ux = (UnaryExpression*)e;
	claim(ux->get_opcode() == k_convert,
	      "unexpected unary expression in expression block");
	DataType *tt = ux->get_result_type();	// target type
	claim(is_kind_of<PointerType>(tt),
	      "unexpected target type when converting initializer");
	e = ux->get_source();
	if (is_kind_of<IntConstant>(e))
	    return opnd_immed(((IntConstant*)e)->get_value(), tt);

	// Handle symbol-relative address.  Assume that the front end
	// validated the conversion.
	SymbolAddressExpression *sax;
	long delta;
	if (is_a<BinaryExpression>(e)) {
	    BinaryExpression *bx = (BinaryExpression*)e;
	    claim(bx->get_opcode() == k_add,
		  "unexpected binary expression in expression block");
	    Expression *s1 = bx->get_source1();
	    Expression *s2 = bx->get_source2();
	    sax = to<SymbolAddressExpression>(s1);
	    delta = to<IntConstant>(s2)->get_value().c_long();
	} else if (is_a<SymbolAddressExpression>(e)) {
	    sax = (SymbolAddressExpression*)e;
	    delta = 0;
	} else {
	    claim(false, "unexpected kind of expression block");
	}
	Opnd addr_sym = opnd_addr_sym(sax->get_addressed_symbol());
	if (delta == 0)
	    return addr_sym;
	else {
	    TypeId rt = ((PointerType*)tt)->get_reference_type();
	    return SymDispOpnd(addr_sym, opnd_immed(delta, type_s32), rt);
	}
    } else
	claim(false,
	      "unexpected kind of Expression in ExpressionValueBlock");

    return Opnd();
}

IdString
get_option_string_value(OptionString *option_string, int pos)
{
    if (pos < option_string->get_number_of_values())
	return option_string->get_string(pos)->get_string();
    return empty_id_string;
}

/*
 * Argument is the string-option specifier passed to an OptionLoop
 * constructor.  It has thus accumulated zero or more strings that are
 * taken as file names.
 */
IdString
process_file_names(OptionString *file_names)
{
    int file_count = file_names->get_number_of_values();
    int i = 0;

    // Process the input file name, if any.
    if (the_suif_env->get_file_set_block() != NULL) {	// expect no input file
	claim(file_count <= 1, "Too many file names: already have input");
    } else {
	claim(file_count > 0, "No input file");
	String s = file_names->get_string(i++)->get_string();
	the_suif_env->read(s.c_str());
    }

    // Return the output file name, if any.
    IdString result;		// empty

    if (i < file_count)
	result = file_names->get_string(i++)->get_string();

    claim(i == file_count,
	  "Too many file names: expected %d, got %d", i, file_count);
    return result;
}


/* -----------------------------  type helpers  ----------------------------- */

int
get_bit_size(TypeId type)
{
    TypeId ut = unqualify_type(type);
    if (is_kind_of<DataType>(ut)) {
	Integer bit_size = static_cast<DataType*>(ut)->get_bit_size();
	if (bit_size.is_c_int())
	    return bit_size.c_int();
    }
    return -1;
}

int
get_bit_alignment(TypeId type)
{
    return unqualify_data_type(type)->get_bit_alignment();
}
