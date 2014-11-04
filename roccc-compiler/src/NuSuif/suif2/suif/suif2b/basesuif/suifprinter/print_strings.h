#ifndef PRINT_STRINGS_H
#define PRINT_STRINGS_H
#include "suifkernel/print_subsystem.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/* This file should only be included once or it will waste space */
static struct PrintSpecClass defaultPrintStrings[] = {
 { "LString",                   "LString %n = \"%ls\""},
 { "String",                    "String %n = \"%s\""},
 { "IInteger",                  "IInt %n = %ii"},
 { "int",                       "int %n = %i"},
 { "bool",                      "bool %n = %B"},
 //{ "float",                   "float %n = %f"}, // No class - float - yet.
 { "double",                    "double %n = %d"},
 { "PTR:O::SymbolTableObject",  "%P"},
 { "PTR:R::DataType",           "%P"},
 { "PTR:R::Type",               "%P"},
 { "SymbolTableObject",         "\"%**_name\""},
 { "SymbolTable",               "%P"},

 { "VoidType",              "type:%R void 0"},
 { "BooleanType",           "type:%R bool %**_bit_size align:%**_bit_alignment"},
 { "IntegerType",
   "type:%R int signed:%**_is_signed %**_bit_size align:%**_bit_alignment"},
 { "FloatingPointType",
   "type:%R float %**_bit_size align:%**_bit_alignment"},
 { "QualifiedType",
     "type:%R qual of %*_base_type qualifications:%***_qualifications"},
 { "PointerType",
     "type:%R ptr %**_bit_size align:%**_bit_alignment to %*_reference_type"},
 { "ArrayType",
     "type:%R array [%*_lower_bound..%*_upper_bound] of %*_element_type"},
 { "MultiDimArrayType",
     "type:%R multi_array [%**_lower_bounds..%**_upper_bounds] of %*_element_type"},
 { "GroupType",
    "type:%R group %**_bit_size align:%**_bit_alignment complete:%**_is_complete"
    "\n%*_group_symbol_table"},
 { "StructType",
    "type:%R struct %**_bit_size align:%**_bit_alignment complete:%**_is_complete"
    "\n%*_group_symbol_table"},
 { "UnionType",
    "type:%R union %**_bit_size align:%**_bit_alignment complete:%**_is_complete"
    "\n%*_group_symbol_table"},
 { "EnumeratedType",
    "type:%R enum num:\"%**_name\" %**_bit_size align:%**_bit_alignment "
    "signed:%**_is_signed\n%*_case"},
 { "CProcedureType",
     "type:%R Cfunc(%LC%**_arguments) vararg:%**_has_varargs "
     "argknwn:%**_arguments_known algn:%**_bit_alignment ret:%**_result_type"},


 { "meta_pair_LString_SymbolTable_ref", "%**_first"},
 { "meta_pair_LString_SymbolTableObject_ref", "%**_first"},
 { "meta_pair_LString_IInteger", "%**_first: %**_second"},
 { "meta_pair_IInteger_ValueBlock_owner", "(%**_first, %****_second)"},
 { "meta_pair_IInteger_CodeLabelSymbol_ref", "(%**_first, %****_second)"},

 // { "ConstantValueBlock",     "ConstVal%n: %_constant"},
 { "ExpressionValueBlock",   "ExprVal%n: %*_expression"},
 { "MultiValueBlock",        "MultiVals%n: t:%**_type\n%*_sub_blocks"},
 { "RepeatValueBlock",
	"RptVals%n: rpt:%***_num_repetitions t:%*_type %**_sub_block"},
 { "UndefinedValueBlock",    "UndefVal%n: t:%**_type"},

 //{ "FileSetBlock",     "FileSetBlock%n:\n%*_file_blocks\n%_external_symbol_table\n%_file_set_symbol_table\n%_information_blocks"},
 { "FileSetBlock",
   "************* ANNOTES ***************\n"
   "%ANNOTES\n"
   "************* EXTERNALS *************\nExternal "
   "%LN%_external_symbol_table\n"
   "************* GLOBALS *************\nGlobal "
   "%LN%_file_set_symbol_table\n%*_file_blocks\n"
   "************* INFORMATION BLOCKS *************\nGlobal Information Blocks\n"
   "%LN%*_information_blocks"},
 { "TargetInformationBlock",
   "************* TARGET_INFORMATION_BLOCK *************\nTarget Information Block\n"
   "integer_types=%*_integer_types\n"
   "floating_point_types=%*_floating_point_types\n"
   "pointer_size_calculation_rule=%**_pointer_size_calculation_rule\n"
   "pointer_alignment_calculation_rule=%**_pointer_alignment_calculation_rule\n"
   "array_alignment_calculation_rule=%**_array_alignment_calculation_rule\n"
   "group_alignment_calculation_rule=%**_group_alignment_calculation_rule\n"
   "procedure_alignment_calculation_rule=%**_procedure_alignment_calculation_rule\n"
   "integer_representation_rule=%**_integer_representation_rule\n"
   "floating_point_representation_rule=%**_floating_point_representation_rule\n"
   "is_big_endian=%**_is_big_endian\n"
   "byte_size=%**_byte_size\n"
   "word_type=%**_word_type\n"
   "default_boolean_type=%**_default_boolean_type\n"
   "default_void_type=%**_default_void_type\n"
   "pointer_size_fixed=%**_pointer_size_fixed\n"
   "pointer_size=%**_pointer_size\n"
   "pointer_alignment_fixed=%**_pointer_alignment_fixed\n"
   "pointer_alignment=%**_pointer_alignment\n"
   "array_alignment_calculation_is_standard=%**_array_alignment_calculation_is_standard\n"
   "array_alignment_minimum=%**_array_alignment_minimum\n"
   "group_alignment_calculation_is_standard=%**_group_alignment_calculation_is_standard\n"
   "group_alignment_minimum=%**_group_alignment_minimum\n"
   "procedure_alignment_fixed=%**_procedure_alignment_fixed\n"
   "procedure_alignment=%**_procedure_alignment\n"
   "integer_representation_is_twos_complement=%**_integer_representation_is_twos_complement\n" },
 { "CInformationBlock",
   "************* C_INFORMATION_BLOCK *************\nC Information Block\n"
   "va_start_builtin=%**_va_start_builtin\n"
   "va_start_old_builtin=%**_va_start_old_builtin\n"
   "va_arg_builtin=%**_va_arg_builtin\n"
   "va_end_builtin=%**_va_end_builtin\n"
   "signed_char_type=%*_signed_char_type\n"
   "unsigned_char_type=%*_unsigned_char_type\n"
   "char_type=%*_char_type\n"
   "signed_short_type=%*_signed_short_type\n"
   "unsigned_short_type=%*_unsigned_short_type\n"
   "signed_int_type=%*_signed_int_type\n"
   "unsigned_int_type=%*_unsigned_int_type\n"
   "signed_long_type=%*_signed_long_type\n"
   "unsigned_long_type=%*_unsigned_long_type\n"
   "signed_long_long_type=%*_signed_long_long_type\n"
   "unsigned_long_long_type=%*_unsigned_long_long_type\n"
   "float_type=%*_float_type\n"
   "double_type=%*_double_type\n"
   "long_double_type=%*_long_double_type\n"
   "file_type=%*_file_type\n"
   "ptrdiff_t_type=%*_ptrdiff_t_type\n"
   "size_t_type=%*_size_t_type\n"
   "va_list_type=%*_va_list_type\n"
   "plain_bit_field_is_signed=%**_plain_bit_field_is_signed\n"
 },


 //{ "FileBlock",     "%_source_file_name\n%*_definition_block\n%_symbol_table\n%_annotes"},
 { "FileBlock",
     "\n\n************* FILE %**_source_file_name *************\n"
     "Annotes:\n%ANNOTES\nFile %_symbol_table\n"
     "Definition Block:\n%**_definition_block"},
 { "BasicSymbolTable",     "BasicSymbolTable: %R\nExplicit Super Scope: %**_explicit_super_scope"
   "\b\n%*_symbol_table_objects"},
 { "GroupSymbolTable",     "GroupSymbolTable: %R\nExplicit Super Scope: %**_explicit_super_scope"
   "\b\n%*_symbol_table_objects"},
 { "DefinitionBlock",
      "Variable Definitions:\n%*_variable_definitions\n"
      "Procedure Definitions:\n%*_procedure_definitions\n"
      "Annotes:\n%ANNOTES"},
 { "ProcedureDefinition",
       "\nPROC%*_procedure_symbol\n%ANNOTES\n"
       "Proc %_symbol_table\nDefinitions:\n%_definition_block\n"
       "Args:\n%**_formal_parameters\nBody:\n%_body\nPROC END %*_procedure_symbol"},
 { "VariableDefinition","static: %**_is_static %*_variable_symbol = %**_initialization"},
 { "VariableSymbol",
     "var_sym:%R \"%**_name\" with t:%*_type addrTaken:%**_is_address_taken\n%ANNOTES"},
 { "ProcedureSymbol",       
   "proc_sym:%R \"%**_name\" with t:%*_type addrTaken:%**_is_address_taken\n%ANNOTES"},
 { "ParameterSymbol",
     "param_sym:%R \"%**_name\" with t:%*_type addrTaken:%**_is_address_taken\n%ANNOTES"},
 { "FieldSymbol",
     "field_sym:%R \"%**_name\" with t:%*_type addrTaken:%**_is_address_taken"
     " offset:%**_bit_offset\n%ANNOTES"},
 { "CodeLabelSymbol",      "label_sym:%R \"%**_name\"\n%ANNOTES"},

 { "GeneralAnnote",         "[\"%**_name\": ]"},
 { "BrickAnnote",         "[\"%**_name\": %LS%*_bricks]"},
 { "BrickList",           "%*_bricks"},
 { "IntegerBrick",        "%**_value"},
 { "StringBrick",         "\"%**_value\""},
 { "SuifObjectBrick",     "SuifObject (ref): %**_object\n"},
 { "OwnedSuifObjectBrick",  "SuifObject (owned): %**_object\n"},

 { "UnaryExpression",
      "(%*_result_type) %**_opcode <exp>\n%ANNOTES\t\t<exp>:%_source"},
 { "BinaryExpression", "(%*_result_type) <e1> %**_opcode <e2>\n%ANNOTES\t\t"
      "<e1>:%_source1\n\t\t<e2>:%_source2"},
 { "SymbolAddressExpression",  "(%*_result_type) SYMADDR %_addressed_symbol\n%ANNOTES"},
 // { "SymbolAddressExpression",  "(%*_result_type) & %_addressed_symbol"},
 { "LoadExpression",         "(%*_result_type) LOADEXP <addr>\n"
                             "%ANNOTES"
                             "\t\t<addr>:%*_source_address"},
 { "LoadVariableExpression", "(%*_result_type) %*_source"},
 { "IntConstant",            "(%*_result_type) %**_value"},
 { "FloatConstant",          "(%*_result_type) %**_value"},
 // { "LoadConstantExpression", "(%*_result_type) LDC %_constant"},
 { "SelectExpression",       "%_selector"},
 { "CallExpression",       "Call %_callee_address ()\n%ANNOTES"
			   "\t\tCall-Args: %LC%**_arguments"},
 { "CallStatement",       "CALL <dst> = *<addr>(<args>)"
   "\n%ANNOTES"
   "\t\t<dst>:%_destination\n"
   "\t\t<addr>: %_callee_address"
   "\t\t<args>:: %LC%**_arguments"},
 { "CopyExpression",         "(%*_result_type) COPY <cp>\n\t\t<cp>:%_source"},
 { "ArrayReferenceExpression",
    " (%*_result_type) array <base> [ <idx> ]\n\t\t<base>:%_base_array_address\n\t\t<idx>:%_index"},
 { "MultiDimArrayExpression",
    " (%*_result_type) multi_array <base> [ <idx> ] \n\t\t<base>:%_array_address\n\t\t<idx>:%LC%*_index"},
 { "FieldAccessExpression",
    " (%*_result_type) field <base> . %_field\n\t\t<base>:%_base_group_address%n"},

 { "StatementList",          "\b%LN%*_statements"},
 { "MarkStatement",          "MRK \n%ANNOTES"},
 { "StoreVariableStatement",
	"ASSIGN <dst> = <src>\n"
        "%ANNOTES"
        "\t\t<dst>:%*_destination\n"
        "\t\t<src>:%_value"},
 { "EvalStatement",     "EVAL <eval-exprs>\n\t<eval-exprs>: %*_expressions"},
 { "IfStatement",       "IF HEADER\n(%_condition)\nIF THEN\n%_then_part\n"
                        "IF ELSE\n%_else_part\nIF END"},
 { "WhileStatement",     "WHILE <cond>\n"
   "<cond>:%_condition\n"
   "CONT: %_continue_label\nBREAK: %_break_label\n"
   "%_body\nWHILE END"},
 { "DoWhileStatement",   "DO\n\t"
   "CONT: %_continue_label\n BREAK: %_break_label\n"
   "%_body\nDO-WHILE <cond>\n\t<cond>:%_condition"},
 { "ForStatement",
     "FOR STATEMENT\n%ANNOTES\n"
   "CONT: %_continue_label\n BREAK: %_break_label\n"
   "FOR LB\n%_lower_bound\nFOR UB\n%_upper_bound\n"
     "FOR STEP\n%_step\nFOR BODY\n%_body\nFOR END"},
 { "CForStatement",
     "CFOR STATEMENT\n%ANNOTES\n"
   "CONT: %_continue_label\nBREAK: %_break_label\n"
   "CFOR BEFORE\n%_before\nCFOR TEST\n%_test\n"
   "CFOR STEP\n%_step\nCFOR BODY\n%_body\nCFOR END"},
 { "ScopeStatement",         "SCOPE STATEMENT\n"
   "\n%ANNOTES\n"
   "Scope %_symbol_table\nScope Definition Block:\n%_definition_block\n"
   "SCOPE BODY\n%_body\nSCOPE END\n"},
 { "StoreStatement",
     "STORE *<addr> = <src>\n"
   "%ANNOTES\t"
   "<addr>:%_destination_address \n"
   "\t<src>:%_value"},
 { "ReturnStatement",        "RET <retval>\n%ANNOTES\t<retval>:%_return_value"},
 { "JumpStatement",          "GOTO %_target;\n%ANNOTES"},
 { "JumpIndirectStatement",  "GOTO(indirect) %_target;"},
 { "BranchStatement",        "BTRUE (%_decision_operand) \n\t %_target;"},
 { "MultiWayBranchStatement", "MULTI-WAY BRANCH opd\n\topd "
      "%***_decision_operand\ndefault: %**_default_target\ncase list:%*_case"},
 { "VaStartStatement", "VA-START addr:%**_ap_address par:%**_parmn\n%ANNOTES"},
 { "LabelLocationStatement", "%*_defined_label: \n%ANNOTES"}

};

#define PRINT_SIZE (sizeof(defaultPrintStrings)/sizeof(PrintSpecClass))





//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static PrintSpecClass defaultPrintRefStrings[] = {
 { "LString",               "\"%ls\""},
 { "String",                "\"%s\""},
 { "IInteger",              "%ii"},
 { "int",                   "%i"},
 { "bool",                  "%B"},
 { "double",                "%d"},

 { "VariableSymbol",           "(%*_type) \"%**_name\""},
 { "ProcedureSymbol",          "\"%**_name\""},
 { "ParameterSymbol",          "(%*_type) \"%**_name\""},
 { "FieldSymbol",              "(%*_type) \"%**_name\""},
 { "CodeLabelSymbol",          "\"%**_name\""},

 { "VoidType",          "void"},
 { "BooleanType",       "bool"},
 { "IntegerType",       "i.%**_bit_size"},
 { "FloatingPointType", "f.%**_bit_size"},
 // { "QualifiedType",     "q.(%***_qualifications.%*_base_type)"},
 { "QualifiedType",     "q.(%*_base_type)"},
 { "PointerType",       "p.%**_bit_size"},
 // { "ArrayType",         "arr[%*****_lower_bound..%*****_upper_bound]"},
 // { "ArrayType",         "a(%**_element_type[])"},
 { "ArrayType",         "a[]"},
 { "MultiDimArrayType",         "ma[]"},
 { "GroupType",         "g.%**_bit_size"},
 { "StructType",        "s.%**_bit_size"},
 { "UnionType",         "u.%**_bit_size"},
 { "EnumeratedType",    "e.\"%**_name\""},
 { "CProcedureType",    "f((%**_result_type)())"},

   { "BasicSymbolTable",     "symtab" },
   { "GroupSymbolTable",     "symtab" },

};

#define PRINT_REF_SIZE (sizeof(defaultPrintRefStrings)/sizeof(PrintSpecClass))





#endif /* PRINT_STRINGS_H */
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
