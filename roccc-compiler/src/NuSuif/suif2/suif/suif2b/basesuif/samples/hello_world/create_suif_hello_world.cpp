/*
 *  This is an example of how to use the lowest-level stuff in the
 *  SUIF libraries to create a small SUIF file.  Ordinarily
 *  higher-level routines would take care of a lot of these details.
 *  Also, this code assumes a particular target machine.
 */


#include "suifkernel/suifkernel_forwarders.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"
#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "common/formatted.h"
#include "iokernel/cast.h"
#include "typebuilder/type_builder.h"

#include <iostream>//jul modif
using namespace std;

#define BITS_PER_BYTE 8

const char *HELLO_STRING="hello world";
const size_t hello_string_len = 11;

void build_tree(SuifEnv *suif);

int main( int argc, char* argv[] ) {
  suif_assert_message( argc == 1, ( "usage hello_world\n" ) );
  SuifEnv* suif = new SuifEnv;
  suif->init();
  build_tree(suif);
  suif->write("hello_world.suif");
  return 0;
}

static BasicObjectFactory *basic_of;
static SuifObjectFactory *suif_of;
/*
PointerType* pointer_to(Type *type)
    {
    return suif_of->create_pointer_type(sizeof(void *)*BITS_PER_BYTE,sizeof(void *)*BITS_PER_BYTE,type);
    }
*/


void build_tree(SuifEnv *suif)
  {

    init_basicnodes(suif);
    init_suifnodes(suif);
    init_typebuilder(suif);
    basic_of = (BasicObjectFactory *)suif->get_object_factory(BasicObjectFactory::get_class_name());
    suif_assert_message(basic_of,( "initialization error in basic") );
    suif_of = (SuifObjectFactory *)suif->get_object_factory(SuifObjectFactory::get_class_name());
    suif_assert_message(suif_of,( "initialization error in suif" ) );
    TypeBuilder *tb = (TypeBuilder *)
      suif->get_object_factory(TypeBuilder::get_class_name());
    suif_assert_message(tb, ("initialization error in typebuilder"));

    BasicSymbolTable *external_symbol_table = basic_of->create_basic_symbol_table(NULL);
    BasicSymbolTable *file_symbol_table = basic_of->create_basic_symbol_table(NULL);
    BasicSymbolTable *file_set_symbol_table = basic_of->create_basic_symbol_table(NULL);
 
    FileSetBlock *the_file_set_block = basic_of->create_file_set_block(external_symbol_table,file_set_symbol_table);
    suif->set_file_set_block(the_file_set_block);

    DefinitionBlock* file_def_block = basic_of->create_definition_block();

    FileBlock * the_file_block = basic_of->create_file_block("hello_world.c",file_symbol_table,
			file_def_block);
    the_file_set_block->append_file_block(the_file_block);

    IntegerType *int_type =
      tb->get_integer_type(sizeof(int)*BITS_PER_BYTE,
			   sizeof(int)*BITS_PER_BYTE, true);
    IntegerType *argc_type = int_type;
    QualifiedType *q_argc_type = tb->get_qualified_type(argc_type);
    //    Type *t = to<Type>(argc_type); 
    IntegerType *char_type = 
      tb->get_integer_type(sizeof(char)*BITS_PER_BYTE,
			   sizeof(char)*BITS_PER_BYTE,true);
    QualifiedType *q_char_type = tb->get_qualified_type(char_type);

    PointerType *char_ptr_type = tb->get_pointer_type(q_char_type);
    QualifiedType *q_char_ptr_type = tb->get_qualified_type(char_ptr_type);

    PointerType *argv_type = tb->get_pointer_type(q_char_ptr_type);
    QualifiedType *q_argv_type = tb->get_qualified_type(argv_type);

    list<QualifiedType *> ql;
    ql.push_back(q_argc_type);
    ql.push_back(q_argv_type);
    CProcedureType *main_type =
      tb->get_c_procedure_type(argc_type, ql, false, true);
    ProcedureSymbol *main_symbol =
	    basic_of->create_procedure_symbol( main_type, "main", true );
    the_file_set_block->get_external_symbol_table()->add_symbol(main_symbol);

    StatementList *main_body = basic_of->create_statement_list();
    BasicSymbolTable* main_symbol_table = basic_of->create_basic_symbol_table(file_symbol_table);
    DefinitionBlock* main_def_block = basic_of->create_definition_block();
    ProcedureDefinition *main_definition =
            basic_of->create_procedure_definition(main_symbol, main_body,main_symbol_table,main_def_block);

    ParameterSymbol *argc_symbol = basic_of->create_parameter_symbol( q_argc_type, "argc", true );

    main_definition->get_symbol_table()->add_symbol(argc_symbol);
    main_definition->append_formal_parameter(argc_symbol);

    ParameterSymbol *argv_symbol = basic_of->create_parameter_symbol( q_argv_type, "argv", false );

    main_definition->get_symbol_table()->add_symbol(argv_symbol);
    main_definition->append_formal_parameter(argv_symbol);

    list<QualifiedType *> arg_list;
    arg_list.push_back(q_char_ptr_type);
    CProcedureType *printf_type = 
      tb->get_c_procedure_type(argc_type,arg_list, false,true,sizeof(int)*BITS_PER_BYTE);
    ProcedureSymbol *printf_symbol =
	    basic_of->create_procedure_symbol( printf_type, "printf", true );
    the_file_set_block->get_external_symbol_table()->add_symbol(printf_symbol);

    FloatingPointType* fpt = tb->get_floating_point_type(64,64);
    QualifiedType* q_fpt = tb->get_qualified_type(fpt);
    VariableSymbol *float_literal_symbol = basic_of->create_variable_symbol(
                        q_fpt, emptyLString, true );

    MultiValueBlock *float_literal_initialization =
            suif_of->create_multi_value_block(fpt);
    float_literal_initialization->add_sub_block(0,
                suif_of->create_expression_value_block( 
		basic_of->create_float_constant( fpt, 7.0 ) ) );

    VariableDefinition *float_literal_definition =
            basic_of->create_variable_definition(float_literal_symbol,sizeof(float)*BITS_PER_BYTE,
                                    float_literal_initialization, true);

    main_definition->get_definition_block()->
            append_variable_definition(float_literal_definition);


    

    Expression *i0 = basic_of->create_int_constant( int_type, 0);
    Expression *i1 = basic_of->create_int_constant( int_type, hello_string_len + 1);
    ArrayType *string_literal_type =
      tb->get_array_type(IInteger((hello_string_len + 1) * sizeof(char)),
			      (int)sizeof(char)*BITS_PER_BYTE,
			      q_char_type,i0,i1);
    
    QualifiedType *q_string_literal_type =
      tb->get_qualified_type(string_literal_type);
    VariableSymbol *string_literal_symbol = basic_of->create_variable_symbol(
									     q_string_literal_type, emptyLString, true );
    main_definition->get_symbol_table()->append_symbol_table_object(string_literal_symbol);
    MultiValueBlock *string_literal_initialization =
	    suif_of->create_multi_value_block(string_literal_type);
    for (size_t char_num = 0; char_num <= hello_string_len; ++char_num)
      {
        string_literal_initialization->add_sub_block(
		char_num,
		suif_of->create_expression_value_block(
                   basic_of->create_int_constant( char_type,HELLO_STRING[char_num]) ));
      }
    VariableDefinition *string_literal_definition =
	    basic_of->create_variable_definition(string_literal_symbol,sizeof(char)*BITS_PER_BYTE,
                                    string_literal_initialization, true);

    main_definition->get_definition_block()->
            append_variable_definition(string_literal_definition);

    Expression *printf_address_op = suif_of->create_symbol_address_expression(
			tb->get_pointer_type(printf_type),printf_symbol);
    Expression *printf_argument_op =
      suif_of->create_symbol_address_expression(char_ptr_type,
						string_literal_symbol);
    // @@@ I believe there is a latent bug here.
    // get_type will return a possibly qualified type
    CallStatement *printf_call =
	    suif_of->create_call_statement(NULL, 
					   printf_address_op);
    //    printf_call->append_data_result_type(argc_type);
    printf_call->append_argument(printf_argument_op);

    //    EvalStatement *printf_statement = suif_of->create_eval_statement();
    //    printf_statement->append_expression(printf_call);
    main_body->append_statement(printf_call);

    the_file_block->get_definition_block()->append_procedure_definition(main_definition);

    suif->set_file_set_block( the_file_set_block );
    FormattedText fd;
    the_file_set_block->print(fd);
    cout << fd.get_value() << endl;
  }

