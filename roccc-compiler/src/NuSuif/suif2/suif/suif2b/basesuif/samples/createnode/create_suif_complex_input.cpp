/*
 *  This is an example of how to use the lowest-level stuff in the
 *  SUIF libraries to create a small SUIF file.  Ordinarily
 *  higher-level routines would take care of a lot of these details.
 *  Also, this code assumes a particular target machine.
 */

#include "create_suif_complex_input.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"
#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "common/formatted.h"
#include "iokernel/cast.h"
#include "typebuilder/type_builder.h"

// #include <iostream.h>
// #include <fstream.h>
#include <iostream>
#include <fstream>

#ifdef MSVC
#include <string.h>
#endif

const char *HELLO_STRING="hello world";

static DataType *unqualify_data_type(Type *t) {
  while (is_kind_of<QualifiedType>(t)) {
    t = to<QualifiedType>(t)->get_base_type();
  }
  if (is_kind_of<DataType>(t)) return(to<DataType>(t));
  suif_assert( false );
  return 0;
}

template<class T>
T *deep_suif_clone(SuifEnv *s, T *obj) { return(to<T>(obj->deep_clone(s))); }


static PointerType* pointer_to(SuifEnv *s, Type *type) {
  PointerType* pt = create_pointer_type(s, sizeof(void *),sizeof(void *),type);
  to<SymbolTable>( type->get_parent() ) -> append_symbol_table_object( pt );
  return pt;
}


CreateSuifComplexInputPass::~CreateSuifComplexInputPass() {}

CreateSuifComplexInputPass::CreateSuifComplexInputPass(SuifEnv *env,
						       const LString &name) :
  FrontendPass(env, name),
  _tb( 0 ),
  _char_type( 0 ),
  _q_char_type( 0 ),
  _fpt( 0 ),
  _q_fpt( 0 ),
  _printf_type(0),
  _string_literal_type( 0 ),
  _main_type( 0 ),
  _argc_type( 0 ),
  _q_argc_type( 0 ),
  _argv_type( 0 ),
  _q_argv_type( 0 )
{
}

//extern "C" void init_suifcloning(SuifEnv *);

FileSetBlock *build_the_file_set_block(SuifEnv *suif) {

    BasicObjectFactory *basic_of =
      (BasicObjectFactory *)suif->get_object_factory(BasicObjectFactory::get_class_name());
    suif_assert_message(basic_of,("initialization error in basic"));
    SuifObjectFactory *suif_of =
      (SuifObjectFactory *)suif->get_object_factory(SuifObjectFactory::get_class_name());
    suif_assert_message(suif_of,("initialization error in suif"));

    BasicSymbolTable *external_symbol_table = create_basic_symbol_table(suif, NULL);
    BasicSymbolTable *file_symbol_table = create_basic_symbol_table(suif, NULL);

    FileSetBlock *the_file_set_block = create_file_set_block(suif, external_symbol_table,file_symbol_table);
    return(the_file_set_block);
}

// This builds a blank main routine into a FileSetBlock.

ProcedureDefinition *CreateSuifComplexInputPass::build_main( const char *main_file ) {
  DefinitionBlock* file_def_block = create_definition_block( _suif_env );

  SymbolTable *file_symbol_table = create_basic_symbol_table( _suif_env );
  FileSetBlock* fsb = _suif_env -> get_file_set_block();
  //  SymbolTable* ext_table = fsb->get_file_set_symbol_table();

  FileBlock * the_file_block =
    create_file_block( _suif_env, main_file,
		      file_symbol_table,
		      file_def_block);

  fsb->append_file_block(the_file_block);

  ProcedureSymbol *main_symbol =
    create_procedure_symbol( _suif_env,_main_type, "main", true );
  fsb -> get_external_symbol_table() -> add_symbol(main_symbol);

  StatementList *main_body = create_statement_list( _suif_env );
  BasicSymbolTable* main_symbol_table =
    create_basic_symbol_table( _suif_env, file_symbol_table );
  DefinitionBlock* main_def_block = create_definition_block( _suif_env );
  ProcedureDefinition *main_definition =
    create_procedure_definition( _suif_env, main_symbol, main_body,main_symbol_table,main_def_block);

  ParameterSymbol *argc_symbol = create_parameter_symbol( _suif_env, _q_argc_type, "argc", false );


  main_definition->get_symbol_table()->add_symbol(argc_symbol);
  main_definition->append_formal_parameter(argc_symbol);

  ParameterSymbol *argv_symbol = 
    create_parameter_symbol( _suif_env, _q_argv_type, "argv", false );

  main_definition->get_symbol_table()->add_symbol(argv_symbol);
  main_definition->append_formal_parameter(argv_symbol);

  the_file_block->get_definition_block()->
    append_procedure_definition(main_definition);

  return main_definition;
}

Module *CreateSuifComplexInputPass::clone() const {
  return (Module*)this;
}


FileSetBlock *CreateSuifComplexInputPass::build_file_set_block() {
    SuifEnv *suif = get_suif_env();

    // Build a blank fileset
    FileSetBlock *the_file_set_block = build_the_file_set_block(suif);
    _suif_env->set_file_set_block( the_file_set_block );

    SymbolTable* ext_table = the_file_set_block->get_external_symbol_table();

    _tb = (TypeBuilder*)suif->get_object_factory(TypeBuilder::get_class_name());
    suif_assert_message(_tb, ("Could not initialize typebuilder"));

    // build the types
    _char_type =
      _tb->get_integer_type( sizeof(char),sizeof(char),true);
    //    ext_table -> append_symbol_table_object( _char_type );
    _q_char_type = _tb->get_qualified_type(_char_type);

    _fpt = _tb->get_floating_point_type( 64 ,64);
    _q_fpt = _tb->get_qualified_type(_fpt);
    //    ext_table -> append_symbol_table_object( _fpt );

    _argc_type = _tb->get_integer_type(sizeof(int),sizeof(int)*8,true);
    _q_argc_type = _tb->get_qualified_type(_argc_type);

    list<QualifiedType *> ql;
    _printf_type =
      _tb->get_c_procedure_type(_argc_type, ql, false,true,sizeof(int));
    //     ext_table -> append_symbol_table_object( _printf_type );


    //    Expression *i0 = create_int_constant(suif, 0, 0);
    //    Expression *i1 = create_int_constant(suif, 0, strlen(HELLO_STRING) + 1);
    _string_literal_type =
      _tb->get_array_type( //IInteger((strlen(HELLO_STRING) + 1) * sizeof(char)),
			  //			   (int)sizeof(char),
			   _q_char_type,
			   0, strlen(HELLO_STRING)+1);
    //			   i0,i1);
    //    ext_table -> append_symbol_table_object( _string_literal_type );

    //    _argc_type = create_integer_type(suif, sizeof(int),sizeof(int)*8,true);
    //    ext_table -> append_symbol_table_object( _argc_type );
    //    _q_argc_type = tb->get_qualified_type(_argc_type);

    _argv_type = _tb->get_pointer_type(_tb->get_pointer_type(_char_type));
    _q_argv_type = _tb->get_qualified_type(_argv_type);


    list<QualifiedType *> qlist;
    qlist.push_back(_q_argc_type);
    qlist.push_back(_q_argv_type);

    _main_type =
      _tb->get_c_procedure_type( _argc_type, qlist, false,true,sizeof(int));
    //    _main_type->append_argument( _q_argc_type );
    //    _main_type->append_argument( _q_argv_type );
    //    ext_table -> append_symbol_table_object( _main_type );


    // Build a blank main routine.
    ProcedureDefinition *main_definition =
      build_main( "complex_input.c");

    // Pull info out of main.
    ParameterSymbol *argc_symbol =
      main_definition->get_formal_parameter(0);

    CProcedureType *main_type =
      to<CProcedureType>(main_definition->get_procedure_symbol()->get_type());
    StatementList *main_body =
      to<StatementList>(main_definition->get_body());


    // Now create the printf statement and add it in
    main_type->append_argument(_tb->get_qualified_type(_tb->get_pointer_type(_char_type)));

    ProcedureSymbol *printf_symbol =
      create_procedure_symbol(suif, _printf_type, "printf", true );
    the_file_set_block->get_external_symbol_table()->
      add_symbol(printf_symbol);


    VariableSymbol *float_literal_symbol =
      create_variable_symbol( suif, _q_fpt, " float symbol", true );
    ext_table -> add_symbol(float_literal_symbol );

    MultiValueBlock *float_literal_initialization =
      create_multi_value_block( _suif_env, _fpt);
    float_literal_initialization->add_sub_block(0,
		create_expression_value_block( _suif_env,
					    create_float_constant(_suif_env, _fpt, 7.0)));

    VariableDefinition *float_literal_definition =
      create_variable_definition(suif, float_literal_symbol,sizeof(float),
				 float_literal_initialization, true);

    main_definition->get_definition_block()->
            append_variable_definition(float_literal_definition);





    VariableSymbol *string_literal_symbol =
      create_variable_symbol(suif, _q_string_literal_type, emptyLString, true );
    main_definition->get_symbol_table()->append_symbol_table_object(string_literal_symbol);
    MultiValueBlock *string_literal_initialization =
      create_multi_value_block(suif, _string_literal_type);
    for (size_t char_num = 0; char_num <= strlen(HELLO_STRING); ++char_num)
      {
        string_literal_initialization->
	  add_sub_block(char_num,
     create_expression_value_block( _suif_env,
	    create_int_constant( _suif_env, _char_type, HELLO_STRING[char_num] )));
      }
    VariableDefinition *string_literal_definition =
      create_variable_definition( _suif_env, string_literal_symbol,sizeof(char),
                                    string_literal_initialization, true);

    main_definition->get_definition_block()->
            append_variable_definition(string_literal_definition);

    SymbolAddressExpression* printf_address_expression =
      create_symbol_address_expression( _suif_env,
                                      pointer_to( _suif_env, _printf_type),
                                      printf_symbol );

    SymbolAddressExpression* printf_argument_expression =
      create_symbol_address_expression( _suif_env,
				      pointer_to(_suif_env, _char_type),
				      string_literal_symbol );
    CallStatement *printf_call =
      create_call_statement( _suif_env,
			     NULL, 
			     //			     _argc_type,
			     printf_address_expression );

    printf_call->append_argument(printf_argument_expression);

    //    EvalStatement *printf_statement = create_eval_statement( _suif_env );
    //    printf_statement->append_expression(printf_call);

    main_body->append_statement(printf_call);

    Expression *condition = 
      create_load_variable_expression(_suif_env, 
				      unqualify_data_type(argc_symbol->get_type()),
				      argc_symbol);

    StatementList *then_part =
      create_statement_list(suif);

    Statement* s = deep_suif_clone( _suif_env, printf_call);

    then_part->append_statement( s );

    StatementList *else_part = create_statement_list( _suif_env );
    else_part->append_statement(deep_suif_clone<Statement>( _suif_env, printf_call));
    else_part->append_statement(deep_suif_clone<Statement>( _suif_env, printf_call));


    IfStatement *the_if =
      create_if_statement( _suif_env,
			  condition,
			  then_part,
			  else_part);

    main_body->append_statement(the_if);

    return the_file_set_block;
}


