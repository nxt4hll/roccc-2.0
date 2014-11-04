// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include <common/suif_copyright.h>
#include <common/suif_list.h>

#include <iostream>
#include <iokernel/cast.h>
#include <iokernel/clone_stream.h>
#include <common/i_integer.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>
#include <basicnodes/basic.h>
#include <basicnodes/basic_constants.h>
#include <suifkernel/suifkernel_messages.h>
#include <suifkernel/utilities.h> 
#include <suifkernel/group_walker.h> 
#include "transforms/procedure_walker_utilities.h"
#include <utils/expression_utils.h>
#include <utils/symbol_utils.h>
#include <utils/type_utils.h>
#include <utils/cloning_utils.h>
#include <cfenodes/cfe.h>
#include "print_utils.h"

/**************************** Declerations ************************************/

String print_if_stmt(Statement *s);
String print_while_stmt(Statement *s);
String print_call_stmt(Statement *s);
String print_scope_stmt(Statement *s);
String print_store_variable_stmt(Statement *s);
String print_store_stmt(Statement *s);
String print_statement_list_stmt(Statement *s);
String print_c_for_stmt(Statement *s);
String print_return_stmt(Statement *s);

String print_binary_expr(Expression *e);
String print_unary_expr(Expression *e);
String print_load_variable_expr(Expression *e);
String print_load_expr(Expression *e);
String print_array_reference_expr(Expression *e);
String print_symbol_address_expr(Expression *e);

String print_opcode(String op);

String print_symbol(Symbol *s);
String print_procedure_symbol(Symbol *s);
String print_variable_symbol(Symbol *s);

/************************** Implementations ***********************************/


String print_execution_object(ExecutionObject *e){

	if(is_kind_of<Statement>(e))
           return print_statement(to<Statement>(e));

    	if(is_kind_of<Expression>(e))
           return print_expression(to<Expression>(e));

	return 0;
}

String print_statement(Statement *s){

        if (is_a<IfStatement>(s))
            return print_if_stmt(s);
        if (is_a<WhileStatement>(s))
            return print_while_stmt(s);
        if (is_a<CallStatement>(s))
            return print_call_stmt(s);
        if (is_a<ScopeStatement>(s))
            return print_scope_stmt(s);
        if (is_a<StoreVariableStatement>(s))
            return print_store_variable_stmt(s);
        if (is_a<StoreStatement>(s))
            return print_store_stmt(s);
        if (is_a<StatementList>(s))
            return print_statement_list_stmt(s);
        if (is_a<CForStatement>(s))
            return print_c_for_stmt(s);
        if (is_a<ReturnStatement>(s))
            return print_return_stmt(s);
        if (is_a<MarkStatement>(s))
            return "";

        String output = String("ERROR") + s->getClassName() + "\n";
        return output;
}

String print_if_stmt(Statement *s){
        IfStatement *if_stmt = to<IfStatement>(s);

        String output = "if(";

        Expression *condition_expr = if_stmt->get_condition();

        output += print_expression(condition_expr);
        output += ")";

        Statement *then_stmt = if_stmt->get_then_part();
        output += print_statement(then_stmt);

        Statement *else_stmt = if_stmt->get_else_part();
        if(else_stmt){
           output += "else";
           output += print_statement(else_stmt);
        }

        return output;
}

String print_while_stmt(Statement *s){
        WhileStatement *while_stmt = to<WhileStatement>(s);

        String output = "while(";

        Expression *condition_expr = while_stmt->get_condition();

        output += print_expression(condition_expr);
        output += ")";

        Statement *body_stmt = while_stmt->get_body();
        output += print_statement(body_stmt);

        return output;
}

String print_call_stmt(Statement *s){
        CallStatement *call_stmt = to<CallStatement>(s);

        String output = "";

        VariableSymbol *dest_op = call_stmt->get_destination();

        output += print_symbol(dest_op);

        output += " = ";

        SymbolAddressExpression *callee_address = to<SymbolAddressExpression>(call_stmt->get_callee_address());
        ProcedureSymbol *macro_name = to<ProcedureSymbol>(callee_address->get_addressed_symbol());

        output += String("ROCCC_") + print_procedure_symbol(macro_name) + "(";

        for(Iter<Expression*> argument_iter = call_stmt->get_argument_iterator();
            argument_iter.is_valid(); argument_iter.next()){
            output += print_expression(argument_iter.current());
            output += ", ";
        }
        output = output.Left(-2);
        output += ")";

        return output + ";\n";
}

String print_scope_stmt(Statement *s){
        ScopeStatement *scope_stmt = to<ScopeStatement>(s);

        Statement *body = scope_stmt->get_body();
        String output = print_statement(body);

        return output;
}

String print_store_variable_stmt(Statement *s){
        StoreVariableStatement *store_var_stmt = to<StoreVariableStatement>(s);

        String output = "";

        VariableSymbol *dest_op = store_var_stmt->get_destination();
        output += print_symbol(dest_op);

        output += " = ";

        Expression *stored_value = store_var_stmt->get_value();
        output += print_expression(stored_value);

        return output + ";\n";
}

String print_store_stmt(Statement *s){
        StoreStatement *store_stmt = to<StoreStatement>(s);

        String output = "";

        Expression *dest_address_expr = store_stmt->get_destination_address();
        output += print_expression(dest_address_expr);

        output += " = ";

        Expression *stored_value = store_stmt->get_value();
        output += print_expression(stored_value);

        return output + ";\n";
}

String print_statement_list_stmt(Statement *s){
        StatementList *stmt_list = to<StatementList>(s);

        String output = "{\n";

        for (Iter<Statement*> iter = stmt_list->get_child_statement_iterator();
             iter.is_valid(); iter.next()){

             Statement *stmt = iter.current();
             output += print_statement(stmt);
        }

        return output + "}\n";
}

String print_c_for_stmt(Statement *s){
        CForStatement *c_for_stmt = to<CForStatement>(s);

        String output = "for(";

        Statement *before_stmt = c_for_stmt->get_before();
        output += print_statement(before_stmt);
        output = output.Left(-1);

        Expression *test_expr = c_for_stmt->get_test();
        output += print_expression(test_expr) + "; ";
        Statement *step_stmt = c_for_stmt->get_step();
        output += print_statement(step_stmt);

        output = output.Left(-2);
        output += ")";

        Statement *body_stmt = c_for_stmt->get_body();
	if(!is_a<StatementList>(body_stmt))
	   output += "\n";	
        output += print_statement(body_stmt);

        return output + "\n";
}

String print_return_stmt(Statement *s){
        ReturnStatement *return_stmt = to<ReturnStatement>(s);

        String output = "return ";

        Expression *return_val = return_stmt->get_return_value();
        output += print_expression(return_val);

        return output + ";\n";
}

String print_expression(Expression *e){

        if (is_a<BinaryExpression>(e))
            return print_binary_expr(e);
        if (is_a<UnaryExpression>(e))
            return print_unary_expr(e);
        if (is_a<LoadVariableExpression>(e))
            return print_load_variable_expr(e);
        if (is_a<LoadExpression>(e))
            return print_load_expr(e);
        if (is_a<ArrayReferenceExpression>(e))
            return print_array_reference_expr(e);
        if (is_a<SymbolAddressExpression>(e))
            return print_symbol_address_expr(e);
        if (is_a<IntConstant>(e))
            return (to<IntConstant>(e))->get_value().to_String();
        if (is_a<FloatConstant>(e))
            return (to<FloatConstant>(e))->get_value();
        if (is_a<CStringConstant>(e))
            return (to<CStringConstant>(e))->get_value();

        return "ERROR expr\n";
}

String print_binary_expr(Expression *e){
        BinaryExpression *binary_expr = to<BinaryExpression>(e);
        String output = "(";

        Expression *src1 = binary_expr->get_source1();
        output += print_expression(src1);

        String opcode = String(binary_expr->get_opcode());
        output += print_opcode(opcode);

        Expression *src2 = binary_expr->get_source2();
        output += print_expression(src2);

        return output + ")";
}

String print_unary_expr(Expression *e){
        UnaryExpression *unary_expr = to<UnaryExpression>(e);
        String output = "";

        String opcode = String(unary_expr->get_opcode());
        output += print_opcode(opcode);

        Expression *src = unary_expr->get_source();
        output += print_expression(src);

        return output;
}

String print_load_variable_expr(Expression *e){
        LoadVariableExpression *load_var_expr = to<LoadVariableExpression>(e);

        VariableSymbol *load_source = load_var_expr->get_source();

        return print_symbol(load_source);
}

String print_load_expr(Expression *e){
        LoadExpression *load_expr = to<LoadExpression>(e);

        return print_expression(load_expr->get_source_address());
}

String print_array_reference_expr(Expression *e){
        ArrayReferenceExpression *array_ref_expr = to<ArrayReferenceExpression>(e);
	
	String output = "";

        Expression *base_address_expr = array_ref_expr->get_base_array_address();
        output += print_expression(base_address_expr);

	output += "[";

        Expression *index_expr = array_ref_expr->get_index();
        output += print_expression(index_expr);

        return output + "]";
}

String print_symbol_address_expr(Expression *e){
	SymbolAddressExpression *symbol_address_expr = to<SymbolAddressExpression>(e);
	
        return print_symbol(symbol_address_expr->get_addressed_symbol());
}

String print_symbol(Symbol *s){

	if(is_a<ProcedureSymbol>(s))
	   return print_procedure_symbol(s);
	if(is_a<VariableSymbol>(s))
	   return print_variable_symbol(s);

        return "ERROR symbol\n";
}

String print_procedure_symbol(Symbol *s){
	ProcedureSymbol *procedure_sym = to<ProcedureSymbol>(s);

        int roccc_macro_count = 17;
        String roccc_macros[17] =  {"ROCCC_min2", "ROCCC_min3", "ROCCC_max2",
                                "ROCCC_max3", "ROCCC_bitcmb", "ROCCC_boollut", "ROCCC_boolsel",
                                "ROCCC_sin", "ROCCC_cos", "ROCCC_allzero", "ROCCC_abs",
                                "ROCCC_bit_array_lookup", "ROCCC_lut_lookup", "ROCCC_lut_write",
                                "ROCCC_gipcore", "ROCCC_sat_add", "ROCCC_sat_clamp"};


        String macro_name = String(procedure_sym->get_name());

        int i;
        for(i = 0; i<roccc_macro_count; i++){
            if(roccc_macros[i] == macro_name)
               break;
        }

        if(i>=roccc_macro_count)
           return String("ERROR called macro name does not exist") + "\n";

        return macro_name ;
}

String print_variable_symbol(Symbol *s){
	VariableSymbol *variable_sym = to<VariableSymbol>(s);

        return String(variable_sym->get_name());
}

String print_type(Type *t){

        String output = "";

        if(is_a<QualifiedType>(t)){
           QualifiedType *qt = to<QualifiedType>(t);
           for(Iter<LString> iter = qt->get_qualification_iterator();
               iter.is_valid(); iter.next())
               output += String(iter.current()) + " ";
           output += print_type(qt->get_base_type());
        }
        else if(is_a<IntegerType>(t))
           output += String("int");
        else if(is_a<BooleanType>(t))
           output += String("bool");
        else if(is_a<PointerType>(t)){
           PointerType *pt = to<PointerType>(t);
           output += print_type(pt->get_reference_type());
        }
        else output += "ERROR type";

        return output;
}

String print_opcode(String op){

        if(op == String("add"))
           return "+";
        if(op == String("subtract"))
           return "-";
        if(op == String("multiply"))
           return "*";
        if(op == String("divide"))
           return "/";
        if(op == String("remainder"))
           return "%";
        if(op == String("bitwise_and"))
           return "&";
        if(op == String("bitwise_or"))
           return "|";
        if(op == String("bitwise_xor"))
           return "^";
        if(op == String("left_shift"))
           return "<<";
        if(op == String("right_shift"))
           return ">>";
        if(op == String("is_equal_to"))
           return "==";
        if(op == String("is_not_equal_to"))
           return "!=";
        if(op == String("is_less_than"))
           return "<";
        if(op == String("is_less_than_or_equal_to"))
           return "<=";
        if(op == String("is_greater_than"))
           return ">";
        if(op == String("is_greater_than_or_equal_to"))
           return ">=";
        if(op == String("logical_and"))
           return "&&";
        if(op == String("logical_or"))
           return "||";

        if(op == String("negate"))
           return "-";
        if(op == String("invert"))
           return "1/";
        if(op == String("bitwise_not"))
           return "~";
        if(op == String("logical_not"))
           return "!";

        return "ERROR op";
}





