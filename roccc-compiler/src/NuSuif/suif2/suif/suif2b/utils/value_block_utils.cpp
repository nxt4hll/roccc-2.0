#include "common/system_specific.h"
#include "value_block_utils.h"
#include "basicnodes/basic.h"
#include "suifnodes/suif.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif_factory.h"
#include "typebuilder/type_builder.h"

// Assumptions: elements are entered in ascending order
//                      padding is done by the front-end
void append_to_multi_value_block( MultiValueBlock* block, ValueBlock* value_block ) {
  IInteger offset = 0;
  int number = block->get_sub_block_count();

  if ( number ) {
    MultiValueBlock::sub_block_pair sub_block = block -> get_sub_block( number - 1 );
    offset = sub_block.first + sub_block.second->get_type()->get_bit_size();
  }
  block->add_sub_block( offset, value_block );
}

//	Create a variable for a string

VariableSymbol *build_string_constant_variable(SuifEnv *env,const char *string) {
    TypeBuilder *tb = (TypeBuilder *)
        env->get_object_factory(TypeBuilder::get_class_name());
    IntegerType *char_type =
        tb->get_integer_type(sizeof(char)*BITSPERBYTE,
                           sizeof(char)*BITSPERBYTE,true);

    size_t len = 0;
    while (string[len])
	len ++;
    QualifiedType *q_char_type = tb->get_qualified_type(char_type);
    ArrayType *string_literal_type =
      tb->get_array_type(q_char_type,0,len + 1);
   
    MultiValueBlock *string_literal_initialization =
            create_multi_value_block(env,string_literal_type);
    for (size_t char_num = 0; char_num <= len; ++char_num) {
        string_literal_initialization->add_sub_block(
                char_num,
                create_expression_value_block(
			env,
                   	create_int_constant( 
				env,char_type,string[char_num])));
        }
    return build_initialized_variable(env,emptyLString,string_literal_type,
				      string_literal_initialization,true);
    }

VariableSymbol *build_initialized_variable(
        SuifEnv *env,
	const LString &name,
	DataType *type,
	ValueBlock *vb,
	bool make_static) {
    TypeBuilder *tb = (TypeBuilder *)
        env->get_object_factory(TypeBuilder::get_class_name());
    FileSetBlock *fsb = env->get_file_set_block();
    suif_assert_message(fsb->get_file_block_count() == 1,("File is ambiguous"));
    FileBlock *fb = fsb->get_file_block(0);
    
    BasicSymbolTable *symtab = to<BasicSymbolTable>(fb->get_symbol_table());
    DefinitionBlock *def  = fb->get_definition_block ();
    QualifiedType *q_type = tb->get_qualified_type(type);

    VariableSymbol *var = create_variable_symbol(
                env,q_type, name, false );
    symtab->append_symbol_table_object(var);
    VariableDefinition *vardef =
            create_variable_definition(env,var,type->get_bit_alignment(),
					vb,make_static);
    def->append_variable_definition(vardef);
    return var;
    }
