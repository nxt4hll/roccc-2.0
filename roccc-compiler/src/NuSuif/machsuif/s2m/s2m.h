/* file "s2m/s2m.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file. 
*/

#ifndef S2M_S2M_H
#define S2M_S2M_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "s2m/s2m.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>


/* Purpose of program: This program lowers the instruction-level suif2
 * IR into the Machine-SUIF IR.  It is capable of handling only the
 * instruction-like statement constructs. */

class S2mSuifPass : public PipelinablePass {
  protected:
    InstrList *mil;		// where we list the converted instructions
    OptionString *file_names;	// Names of input and/or output files
    IdString o_fname;		// optional output file name
    IdString source_file;	// source file name from file_block
    OptUnit *cur_unit;		// unit currently being processed
    Annote *cur_line_annote;	// prevailing line annote for new instrs
    bool is_varargs;		// true <=> va_start occurs in current unit
    bool is_leaf;		// true <=> no call  occurs in current unit

  public:
    S2mSuifPass(SuifEnv *suif_env, const IdString &name = "s2m");

    void initialize();
    void execute();

    // We only need a single copy of this pass, since we should never
    // need more than one s2m pass in any compiler.
    Module* clone() const { return (Module*)this; }

    bool parse_command_line(TokenStream *command_line_stream);

    void do_file_set_block(FileSetBlock *the_file_set_block);
    void do_file_block(FileBlock *the_file_block);
    void do_procedure_definition(ProcedureDefinition *pd);

    // generic conversions
    void convert_eo(ExecutionObject *the_eo);
    void convert_statement(Statement *the_statement);
    Opnd convert_expression(Expression *the_expression);
    Opnd convert_address_op(Expression *the_expression);

    // specific expression conversions
    Opnd convert_constant_expression(
	    Constant *e);
    Opnd convert_load_variable_expression(
	    LoadVariableExpression *the_load_variable_expression);
    Opnd convert_binary_expression(
	    BinaryExpression *the_binary_expression);
    Opnd convert_unary_expression(
	    UnaryExpression *the_unary_expression);
    Opnd convert_select_expression(
	    SelectExpression *the_select_expression);
    Opnd convert_load_expression(
	    LoadExpression *the_load_expression);
    Opnd convert_symbol_address_expression(
	    SymbolAddressExpression *the_symbol_address_expression);
    Opnd convert_call_expression(
	    CallExpression *the_call_expression);
    Opnd convert_array_reference_expression(
	    ArrayReferenceExpression *the_array_reference_expression);
    Opnd convert_va_arg_expression(
	    VaArgExpression *the_va_arg_expression);
    Opnd convert_generic_expression(
	    Expression *the_expression);

    // specific statement conversions
    void convert_eval_statement(
            EvalStatement *the_eval_statement);
    void convert_store_statement(
            StoreStatement *the_store_statement);
    void convert_store_variable_statement(
            StoreVariableStatement *the_store_variable_statement);
    void convert_call_statement(
            CallStatement *the_call_statement);
    void convert_return_statement(
            ReturnStatement *the_return_statement);
    void convert_jump_statement(
            JumpStatement *the_jump_statement);
    void convert_jump_indirect_statement(
	    JumpIndirectStatement *the_jump_indirect_statement);
    void convert_branch_statement(
            BranchStatement *the_branch_statement);
    void convert_multiway_branch_statement(
            MultiWayBranchStatement *the_multi_way_branch_statement);
    void convert_label_location_statement(
            LabelLocationStatement *the_label_location_statement);
    void convert_mark_statement(
            MarkStatement *the_mark_statement);
    void convert_va_start_statement(
            VaStartStatement *the_va_start_statement);
    void convert_va_end_statement(
            VaEndStatement *the_va_end_statement);
    void convert_generic_statement(
	    Statement *the_statement);

    // helpers
    void emit(Instr*);
    Opnd convert_callee_address_expression(Expression*, Sym **callee);
};

extern "C" void init_s2m(SuifEnv *suif_env);


#endif /* S2M_S2M_H */
