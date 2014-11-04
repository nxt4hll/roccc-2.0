#include "cprint_style.h"
#include "suifkernel/cascading_map.h"
#include "basicnodes/basic_constants.h"
#include "basicnodes/basic.h"
#include "suifnodes/suif.h"
#include "iokernel/object_wrapper.h"
#include "suifkernel/module_subsystem.h"
#include "basicnodes/basic_constants.h"
// #ifndef MSVC
// #include <iostream>
// #else
// #include <iostream.h>
// #endif

//jul modif
#include <iostream>
using namespace std;

static String default_string_gen_fn(CPrintStyleModule *map, const SuifObject *obj) {
  return(String("?") + obj->getClassName());
}


// #define HANDLE_DISPATCH(lower, name) 
// static String handle_static_ ## lower(CPrintStyleModule *map, const SuifObject * ## lower);
// #include "obj.defs"
// #undef HANDLE_DISPATCH

//jul modif
#define STAR_SYMBOL *
#define HANDLE_DISPATCH(lower, name) \
static String handle_static_##lower(CPrintStyleModule* map, const SuifObject STAR_SYMBOL name); //lower);
#include "obj.defs"
#undef HANDLE_DISPATCH
#undef STAR_SYMBOL



LString CPrintStyleModule::get_class_name() {
  static LString name("cprint");
  return(name);
}

CPrintStyleModule::~CPrintStyleModule() {
   delete _print_map;
}

void CPrintStyleModule::set_print_function(const LString &class_name,
					   string_gen_fn fn)
{
  _print_map->assign(class_name, fn);
}

void CPrintStyleModule::initialize() {
  Module::initialize();

  _print_map = 
    new CascadingMap<string_gen_fn>(get_suif_env(), default_string_gen_fn);
  
  // #define HANDLE_DISPATCH(lower, name) 
  //   set_print_function( name ## ::get_class_name(), 
  // 		     handle_static_ ## lower ## );
  // #include "obj.defs"
  // #undef HANDLE_DISPATCH
  //  set_interface("print", (Address)CPrintStyleModule::print_dispatch);
  
//jul modif
#define ACCESS_SYMBOL ::
#define HANDLE_DISPATCH(lower, name) \
  set_print_function( name::get_class_name(), \
                     handle_static_ ## lower  );
#include "obj.defs"
#undef HANDLE_DISPATCH
#undef ACCESS_SYMBOL

}

void CPrintStyleModule::print_dispatch(Module *module,
				       ostream &str,
				       const ObjectWrapper &obj)
{
  CPrintStyleModule *pm = (CPrintStyleModule*)module;
  String result;
  if (is_kind_of_suif_object_meta_class(obj.get_meta_class())) {
    result = pm->print_to_string((const SuifObject*)obj.get_address());
  }
  str << result.c_str();
}

CPrintStyleModule::CPrintStyleModule(SuifEnv *s, const LString &name) :
  Module(s, name),
  _print_map(0)
{

}
Module *CPrintStyleModule::clone() const {
  return((Module*)this);
}

String CPrintStyleModule::print_to_string(const SuifObject *obj) {
  // need a visitor for this.
  if (obj == NULL) return("(nil)");
  string_gen_fn fn = _print_map->lookup(obj);
  String str = (*fn)(this, obj);
  return(str);
}

String to_cstring(const SuifObject *obj) {
	if (obj == NULL) return("(nil)");
	SuifEnv* suif_env = obj->get_suif_env();
	ModuleSubSystem *ms = suif_env->get_module_subsystem();
    CPrintStyleModule* cprint = (CPrintStyleModule*)
        ms->retrieve_module("cprint");
	suif_assert_message(cprint, ("CPrintStyleModule is not loaded"));
    return cprint->print_to_string(obj);
}

static String handle_static_variable_symbol(CPrintStyleModule *map,
					    const SuifObject *obj) {
  VariableSymbol *var = to<VariableSymbol>(obj);
  // I'd like to figure out what procedure and BLOCK scope this
  // is in...
  return(String(var->get_name()));
}

static String handle_static_procedure_symbol(CPrintStyleModule *map,
					     const SuifObject *obj) {
  ProcedureSymbol *ps = to<ProcedureSymbol>(obj);
  return(String("P:") + ps->get_name());
}

static String handle_static_code_label_symbol(CPrintStyleModule *map,
					     const SuifObject *obj) {
  CodeLabelSymbol *ps = to<CodeLabelSymbol>(obj);
  return(String("L:") + ps->get_name());
}



//
//  Here are the visit methods
//  for the c-like printing
//
// All of these handle_
// will set the string in the state before returning.
static
String handle_static_expression(CPrintStyleModule *state,
				const SuifObject *obj)
{
  Expression *expr = to<Expression>(obj);

  // Use the iterator over source ops and
  // get the classname
  String opname = expr->getClassName();
  String return_str = String("?") + opname + "(";
  bool needs_comma = false;
  for (Iter<Expression *> iter = expr->get_source_op_iterator();
       iter.is_valid();
       iter.next()) {
    Expression *opn = iter.current();
    if (needs_comma) {
      return_str += ",";
    } else {
      needs_comma = true;
    }
    String op = state->print_to_string(opn);
    return_str += op;
  }
  return_str += ")";
  return(return_str);
}


struct OperatorTableE {
  LString* _oper;
  const char *_op_char;
};

struct OperatorTableE binary_oper_table[] =
{
    { &k_add, " + " },
    { &k_multiply, " * " },
    { &k_subtract, " - " },
    { &k_divide, " / " },
    { &k_bitwise_and, " & " },
    { &k_bitwise_or, " | " },
      //    { &k_bitwise_nand, " !& " },
      //    { &k_bitwise_nor, " !| " },
      //    { &k_bitwise_xor, " ^ " },
    { &k_left_shift, " << " },
    { &k_right_shift, " << " },
      // &k_rotate;
      // &k_remainder;
    { &k_is_equal_to, " == " },
    { &k_is_not_equal_to, " != " },
    { &k_is_less_than, " < " },
    { &k_is_less_than_or_equal_to, " <= " },
    { &k_is_greater_than, " > " },
    { &k_is_greater_than_or_equal_to, " >= " },
    { &k_logical_and, " && " },
    { &k_logical_or, " || " }
		//&k_maximum, 
		//&k_minimum;
      //    { &k_negate;
      //&k_invert;
      //&k_absolute_value;
      //&k_bitwise_not;
      //&k_logical_not;
      //&k_convert;
      //&k_treat_as;
};
#define NUM_BINARY_OPER (sizeof(binary_oper_table)/sizeof(struct OperatorTableE))

struct OperatorTableE unary_oper_table[] =
{ 

    { &k_negate, " -" },
    { &k_invert, " 1/" },
      //&k_absolute_value;
    { &k_bitwise_not, " ~" },
    { &k_logical_not, " !" }
      //&k_convert;
      //&k_treat_as;
};
#define NUM_UNARY_OPER (sizeof(unary_oper_table)/sizeof(struct OperatorTableE))

static
String handle_static_unary_expression(CPrintStyleModule *state,
				      const SuifObject *obj)
{
  UnaryExpression *expr = to<UnaryExpression>(obj);

  LString opc =expr->get_opcode();
  String src_string = state->print_to_string(expr->get_source());
  
  for (size_t i = 0; i < NUM_UNARY_OPER; i++ ) {
    if (*(unary_oper_table[i]._oper) == opc) {
      const char *op_char = unary_oper_table[i]._op_char;
      return(String("(") + op_char + src_string + ")");
    }
  }

  //  if (opc == &k_convert) { return(src_string); }
  if (opc == k_copy) { return(src_string); }
  return(String(opc) + "(" + src_string + ")");
}

static
String handle_static_binary_expression(CPrintStyleModule *state,
				       const SuifObject *obj)
{
  BinaryExpression *expr = to<BinaryExpression>(obj);
  LString opc = expr->get_opcode();
  String src_string1 = state->print_to_string(expr->get_source1());
  String src_string2 = state->print_to_string(expr->get_source2());
  // Should we special case: add, mod, etc?
  for (size_t i = 0; i < NUM_BINARY_OPER; i++ ) {
    if (*(binary_oper_table[i]._oper) == opc) {
      const char *op_char = binary_oper_table[i]._op_char;
      return(String("(") + src_string1 + op_char + src_string2 + ")");
    }
  }
      
  return(String(opc) + "(" + src_string1 + "," + src_string2 + ")");
}

static
String handle_static_array_reference_expression(CPrintStyleModule *state,
					      const SuifObject *obj)
{
  ArrayReferenceExpression *expr =
    to<ArrayReferenceExpression>(obj);

  String base = state->print_to_string(expr->get_base_array_address());
  String idx = state->print_to_string(expr->get_index());

  return(String("&(") + base + "[" + idx + "])");
}

static
String handle_static_load_expression(CPrintStyleModule *state,
				     const SuifObject *obj)
{
  LoadExpression *expr =
    to<LoadExpression>(obj);
  String source = state->print_to_string(expr->get_source_address());
  // If the previous began with a "&", return the rest of it.
  if(source.length() >= 1 &&
     source.c_str()[0] == '&') {
    // This is clearly cheating with the string class.
    return(String(&(source.c_str()[1])));
  } else {
    return(String("*") + source);
  }
}
static
String handle_static_load_variable_expression(CPrintStyleModule *state,
					      const SuifObject *obj)
{
  LoadVariableExpression *expr =
    to<LoadVariableExpression>(obj);
  String source = state->print_to_string(expr->get_source());
  return(source);
}

static
String handle_static_call_statement(CPrintStyleModule *state,
				    const SuifObject *obj)
{
  CallStatement *expr =
    to<CallStatement>(obj);
  String addr = state->print_to_string(expr->get_callee_address());
  String return_str;
  if (expr->get_destination() != NULL) {
    return_str += state->print_to_string(expr->get_destination());
    return_str += " = ";
  }

  return_str += String("(") + addr + ")(";
  bool needs_comma = false;
  for (Iter<Expression *> iter = expr->get_argument_iterator();
       iter.is_valid();
       iter.next()) {
    Expression *opn = iter.current();
    if (needs_comma) {
      return_str += ",";
    } else {
      needs_comma = true;
    }
    String op = state->print_to_string(opn);
    return_str += op;
  }
  return_str += ")";
  return(return_str);
}

/*
static
void handle_static_call_expression(CPrintStyleModule *state,
				   CallExpression *expr)
{
  String addr = state->build_expression_string(expr->get_callee_address());
  String return_str = String("(") + addr + ")(";
  bool needs_comma = false;
  for (Iter<Expression *> iter = expr->get_argument_iterator();
       iter.is_valid();
       iter.next()) {
    Expression *opn = iter.current();
    if (needs_comma) {
      return_str += ",";
    } else {
      needs_comma = true;
    }
    String op = state->build_expression_string(opn);
    return_str += op;
  }
  return_str += ")";
  state->set_string(return_str);
}
*/

static
String handle_static_symbol_address_expression(CPrintStyleModule *state,
					       const SuifObject *obj)
{
  SymbolAddressExpression *expr =
    to<SymbolAddressExpression>(obj);
  Symbol *sym = expr->get_addressed_symbol();
  return(String("&") + state->print_to_string(sym));
}

static
String handle_static_va_arg_expression(CPrintStyleModule *state,
				       const SuifObject *obj)
{
  VaArgExpression *expr =
    to<VaArgExpression>(obj);
  String op = state->print_to_string(expr->get_ap_address());
  return(String("va_arg(") + op + ")");
}

static
String handle_static_int_constant(CPrintStyleModule *state,
				  const SuifObject *obj)
{
  IntConstant *expr =
    to<IntConstant>(obj);
  IInteger v = expr->get_value();
  return(v.to_String());
}
static
String handle_static_float_constant(CPrintStyleModule *state,
				    const SuifObject *obj)
{
  FloatConstant *expr = to<FloatConstant>(obj);
  return(expr->get_value().c_str());
}

/* Still need to add some support for finding field annotations
 * (Makes the output MUCH more readable.
 */

static
String handle_static_statement(CPrintStyleModule *state,
			       const SuifObject *obj)
{
  Statement *stmt = to<Statement>(obj);
  // Use the iterator over
  //    destination_vars
  //    source ops
  //    source variables
  // Use the iterator over source ops and
  // get the classname

  String return_str = "(";
  bool needs_comma = false;

  {for (Iter<VariableSymbol *> iter = stmt->get_destination_var_iterator();
       iter.is_valid();
       iter.next()) {
    VariableSymbol *var = iter.current();
    if (needs_comma) {
      return_str += ",";
    } else {
      needs_comma = true;
    }
    String op = state->print_to_string(var);
    return_str += op;
  }
  }
  return_str += ") = ";

  String opname = stmt->getClassName();
  return_str += String("?") + opname + "(";
  needs_comma = false;
  {for (Iter<Expression *> iter = stmt->get_source_op_iterator();
       iter.is_valid();
       iter.next()) {
    Expression *opn = iter.current();
    if (needs_comma) {
      return_str += ",";
    } else {
      needs_comma = true;
    }
    String op = state->print_to_string(opn);
    return_str += op;
  }}
  return_str += ")";

  needs_comma = false;
  {for (Iter<Statement *> iter = stmt->get_child_statement_iterator();
       iter.is_valid();
       iter.next()) {
    Statement *statement = iter.current();
    if (needs_comma) {
      return_str += "; ";
    } else {
      needs_comma = true;
    }
    String op = state->print_to_string(statement);
    return_str += op;
  }}
  return_str += ";";

  return(return_str);
}

static
String handle_static_return_statement(CPrintStyleModule *state,
				    const SuifObject *obj)
{
  ReturnStatement *stmt = to<ReturnStatement>(obj);
  String return_str = "Return";
  if (stmt->get_return_value() != NULL) {
    String op = state->print_to_string(stmt->get_return_value());
    return_str = return_str + "(" + op + ")";
  }
  return(return_str);
}

static
String handle_static_store_variable_statement(CPrintStyleModule *state,
					      const SuifObject *obj)
{
  StoreVariableStatement *stmt = to<StoreVariableStatement>(obj);
  String return_str =
    state->print_to_string(stmt->get_destination()) + " = ";
  String op = state->print_to_string(stmt->get_value());
  return_str += op;
  return(return_str);
}

static
String handle_static_store_statement(CPrintStyleModule *state,
				   const SuifObject *obj)
{
  StoreStatement *stmt = to<StoreStatement>(obj);
  String return_str = "*(";
  return_str +=
    state->print_to_string(stmt->get_destination_address());

  return_str += ") = ";
  return_str +=
    state->print_to_string(stmt->get_value());
  return(return_str);
}

static
String handle_static_mark_statement(CPrintStyleModule *state,
				    const SuifObject *obj)
{
  MarkStatement *stmt = to<MarkStatement>(obj);
  String return_str;
  BrickAnnote *br = to<BrickAnnote>(stmt->peek_annote("line"));
  return_str = "Mark";
  if (br != 0) {
    String file = to<StringBrick>(br->get_brick(1))->get_value();
    IInteger line = to<IntegerBrick>(br->get_brick(0))->get_value();
    return_str += " ";
    return_str += file + ":" + line.to_String();
  }
  return(return_str);
}

static
String handle_static_if_statement(CPrintStyleModule *state,
				  const SuifObject *obj)
{
  IfStatement *stmt = to<IfStatement>(obj);
  String return_str = "If (";
  return_str += state->print_to_string(stmt->get_condition());
  return_str += ") Then { ";
  return_str += state->print_to_string(stmt->get_then_part());
  return_str += " }";
  if (stmt->get_else_part()) {
    return_str += " Else { ";
    return_str += state->print_to_string(stmt->get_else_part());
    return_str += " }";
  }
  return(return_str);
}

static
String handle_static_branch_statement(CPrintStyleModule *state,
				      const SuifObject *obj)
{
  BranchStatement *stmt = to<BranchStatement>(obj);
  String return_str = "If (";
  return_str += state->print_to_string(stmt->get_decision_operand());
  return_str += ") Goto ";
  return_str += state->print_to_string(stmt->get_target());
  return(return_str);
}
static
String handle_static_jump_statement(CPrintStyleModule *state,
				    const SuifObject *obj)
{
  JumpStatement *stmt = to<JumpStatement>(obj);
  String return_str = "Goto ";
  return_str += state->print_to_string(stmt->get_target());
  return(return_str);
}
static
String handle_static_label_location_statement(CPrintStyleModule *state,
					      const SuifObject *obj)
{
  LabelLocationStatement *stmt = to<LabelLocationStatement>(obj);
  String return_str = state->print_to_string(stmt->get_defined_label());
  return_str += ":";
  return(return_str);
}
static
String handle_static_eval_statement(CPrintStyleModule *state,
				    const SuifObject *obj)
{
  EvalStatement *stmt = to<EvalStatement>(obj);
  String return_str = "Eval(";
  bool is_first = true;
  for (Iter<Expression*> iter = stmt->get_expression_iterator();
       iter.is_valid(); iter.next()) {
    if (!is_first)
      return_str += ", ";
    
    Expression *expr = iter.current();
    return_str += state->print_to_string(expr);
    is_first = false;
  }
  return(return_str);
}

extern "C" void EXPORT init_print_styles(SuifEnv* suif)
{
  suif->require_module("basicnodes");
  suif->require_module("suifnodes");

  ModuleSubSystem* mSubSystem = suif->get_module_subsystem();
  if (!mSubSystem->retrieve_module(CPrintStyleModule::get_class_name())) {
    Module *m = new CPrintStyleModule(suif);
    mSubSystem -> register_module(m);
    //suif->get_print_subsystem()->set_default_print_style(m->get_module_name());
  }
}
