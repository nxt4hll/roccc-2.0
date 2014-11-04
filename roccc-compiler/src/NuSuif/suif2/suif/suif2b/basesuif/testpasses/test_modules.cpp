#include "common/system_specific.h"
#include "test_modules.h"

#include "iokernel/cast.h"
#include "iokernel/aggregate_meta_class.h"
#include "suifkernel/suifkernel_forwarders.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/suif_object.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/command_line_parsing.h"
#include "suifkernel/utilities.h"
#include "suifkernel/group_walker.h"
#include "basicnodes/basic.h"
#include "suifnodes/suif.h"
#include "typebuilder/type_builder.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif_factory.h"


#include <iostream>//jul modif
#include <stdlib.h>

using namespace std;

LString TestCloneModule::ClassName("testClone");
LString TestWalkerModule::ClassName("testWalker");


//static int nObjects = 0;

extern "C" void EXPORT init_testpasses( SuifEnv* suif ) {
  ModuleSubSystem* mSubSystem = suif->get_module_subsystem();
  suif->require_module("suifnodes");
  suif->require_module("typebuilder");
  suif->require_module("suifcloning");

  if (!mSubSystem->retrieve_module(TestCloneModule::ClassName)) {
    mSubSystem -> register_module( new TestCloneModule( suif ) );
    mSubSystem -> register_module( new TestWalkerModule( suif ) );
    mSubSystem -> register_module( new NodeCount( suif ) );
    mSubSystem -> register_module( new VirtualTests( suif ) );
    }
}


TestCloneModule::TestCloneModule( SuifEnv* suif ) : Module( suif ) {
}


LString TestCloneModule::get_module_name() const {
  return TestCloneModule::ClassName;
}

void print(SuifObject  *st) {
    FormattedText ft;
    st->print(ft);
    cout << ft.get_value() << endl;
    }



//	Build a tree for use by the cloning test. This is a hack
//	of the hello world build_tree with more fields added to
//	test certain aspects of cloning
StatementList *build_tree(SuifEnv *suif) {
    const char * HELLO_STRING = "Hello Salesman";
    TypeBuilder *tb = (TypeBuilder *)
          suif->get_object_factory(TypeBuilder::get_class_name());

    FileSetBlock *the_file_set_block = create_file_set_block(suif);

    suif->set_file_set_block( the_file_set_block );

    FileBlock * the_file_block = create_file_block(suif,"hello_world.c");

    the_file_set_block->append_file_block(the_file_block);

    IntegerType *argc_type = tb->get_integer_type(true);
    QualifiedType *q_argc_type = tb->get_qualified_type(argc_type);

    IntegerType *char_type = tb->get_integer_type(sizeof(char)*8,sizeof(char)*8,((char)255) < 0);
    QualifiedType *q_char_type = tb->get_qualified_type(char_type);

    PointerType *char_ptr_type = tb->get_pointer_type(q_char_type);
    QualifiedType *q_char_ptr_type = tb->get_qualified_type(char_ptr_type);
    PointerType *argv_type = tb->get_pointer_type(q_char_ptr_type);
    QualifiedType *q_argv_type = tb->get_qualified_type(argv_type);

    CProcedureType *main_type =
	create_c_procedure_type(suif,argc_type,false,true,sizeof(int)*8);
    main_type->append_argument(q_argc_type);
    main_type->append_argument(q_argv_type);
    the_file_set_block->get_external_symbol_table()->append_symbol_table_object(main_type);
    ProcedureSymbol *main_symbol = create_procedure_symbol( suif,
							    main_type, 
							    "main", true );
    //    the_file_set_block->get_external_symbol_table()->add_symbol("main",main_symbol);
    the_file_set_block->get_external_symbol_table()->add_symbol(main_symbol);

    StatementList *main_body = create_statement_list(suif);
    ProcedureDefinition *main_definition =
            create_procedure_definition(suif,main_symbol, main_body);

    ParameterSymbol *argc_symbol = create_parameter_symbol( suif,q_argc_type, "argc", true );


    //    main_definition->get_symbol_table()->add_symbol("argc",argc_symbol);
    main_definition->get_symbol_table()->add_symbol(argc_symbol);
    main_definition->append_formal_parameter(argc_symbol);

    ParameterSymbol *argv_symbol = create_parameter_symbol( suif,q_argv_type, "argv", false );

    //    main_definition->get_symbol_table()->add_symbol("argv",argv_symbol);
    main_definition->get_symbol_table()->add_symbol(argv_symbol);
    main_definition->append_formal_parameter(argv_symbol);

    CProcedureType *printf_type = create_c_procedure_type(suif,argc_type, false,true,sizeof(int)*8);
    
    printf_type->append_argument(q_char_ptr_type);
    main_definition->get_symbol_table()->append_symbol_table_object(printf_type);
    ProcedureSymbol *printf_symbol = create_procedure_symbol( suif,printf_type, "printf", true );
    //    the_file_set_block->get_external_symbol_table()->add_symbol("printf",printf_symbol);
    the_file_set_block->get_external_symbol_table()->add_symbol(printf_symbol);

    FloatingPointType* fpt = tb->get_double_floating_point_type();
    QualifiedType* q_fpt = tb->get_qualified_type(fpt);
    VariableSymbol *float_literal_symbol = create_variable_symbol(suif, q_fpt, "fval", true );
    the_file_set_block->get_external_symbol_table()->add_symbol(float_literal_symbol);
    
    FloatConstant *seven = create_float_constant( suif, fpt, 7.0 );

    MultiValueBlock *float_literal_initialization = create_multi_value_block(suif,fpt);
    float_literal_initialization->add_sub_block(0, create_expression_value_block(suif, seven));

    VariableDefinition *float_literal_definition =
            create_variable_definition(suif,float_literal_symbol,sizeof(float)*8,
                                    float_literal_initialization, true);

    main_definition->get_definition_block()->
            append_variable_definition(float_literal_definition);

    ArrayType *string_literal_type =
      tb->get_array_type(q_char_type,0, strlen(HELLO_STRING) +1);

    QualifiedType *q_string_literal_type = 
      tb->get_qualified_type(string_literal_type);
    VariableSymbol *string_literal_symbol = 
      create_variable_symbol(suif, q_string_literal_type, emptyLString, true );
    main_definition->get_symbol_table()->append_symbol_table_object(string_literal_symbol);
    MultiValueBlock *string_literal_initialization =
	    create_multi_value_block(suif,string_literal_type);
    for (size_t char_num = 0; char_num <= strlen(HELLO_STRING); ++char_num) {
        string_literal_initialization->add_sub_block(
		char_num,
		create_expression_value_block(suif,
                   create_int_constant( suif,char_type,HELLO_STRING[char_num]) ));
      }
    VariableDefinition *string_literal_definition =
	    create_variable_definition(suif,string_literal_symbol,sizeof(char)*8,
                                    string_literal_initialization, true);

    main_definition->get_definition_block()->
            append_variable_definition(string_literal_definition);

    Expression *printf_address_op = 
	create_symbol_address_expression(suif, tb->get_pointer_type(printf_type),printf_symbol);
    Expression *printf_argument_op =
        create_symbol_address_expression(suif,char_ptr_type, string_literal_symbol);
    // @@@ I believe there is a latent bug here.
    // get_type will return a possibly qualified type
    CallStatement *printf_call =
	    create_call_statement(suif,NULL, printf_address_op);
    //    printf_call->append_data_result_type(argc_type);
    printf_call->append_argument(printf_argument_op);

    //    EvalStatement *printf_statement = create_eval_statement(suif);
    //    printf_statement->append_expression(printf_call);
    main_body->append_statement(printf_call);

    LoadVariableExpression * lve = create_load_variable_expression(suif,string_literal_type,string_literal_symbol);
    LoadVariableExpression * lvec = to<LoadVariableExpression>(lve->deep_clone(suif));
    print(lve);
    print(lvec);
    EvalStatement *lve_statement = create_eval_statement(suif);
    printf_call->append_argument(lve);
    main_body->append_statement(lve_statement);

    LabelType *clt = tb->get_label_type();
    CodeLabelSymbol *cls = create_code_label_symbol(suif, clt);

    main_definition->get_symbol_table()->append_symbol_table_object(cls);

    LabelLocationStatement *lab_def = create_label_location_statement(suif,cls);

    main_body->append_statement(lab_def);

    the_file_block->get_definition_block()->append_procedure_definition(main_definition);

    return main_body;
    }

void TestCloneModule::execute() {
#if 0
    StatementList *sl = build_tree(_suif_env);
    ::print(sl);
    StatementList *slc = to<StatementList>(sl->deep_clone(_suif_env));
    cout << "result of deep clone \n";
    ::print(slc);
    StatementList *slsc = to<StatementList>(sl->shallow_clone(_suif_env));
    cout << "result of shallow clone\n";
    ::print(slsc);
#else
	// Hansel's version
  for (Iter<StatementList> iter =
         object_iterator<StatementList>(get_suif_env()->get_file_set_block());
       iter.is_valid(); iter.next()) {
      	StatementList *sl = &iter.current();
      	//    StatementList *sl = build_tree(_suif_env);
      	::print(sl);
      	StatementList *slc = to<StatementList>(sl->deep_clone(_suif_env));
      	cout << "result of deep clone \n";
      	::print(slc);
      	StatementList *slsc = to<StatementList>(sl->shallow_clone(_suif_env));
      	cout << "result of shallow clone\n";
      	::print(slsc);
    	}

#endif

}

Module *TestCloneModule::clone() const {
  return(Module*)this;
}


TestWalkerModule::TestWalkerModule( SuifEnv* suif ) : Module( suif ) { }

LString TestWalkerModule:: get_module_name() const {
  return TestWalkerModule::ClassName;
  }

class TestWalker : public SelectiveWalker {
    public:
	TestWalker(SuifEnv *env,const LString &type);
	ApplyStatus operator ()(SuifObject *obj);
    };

TestWalker::TestWalker(SuifEnv *env,const LString &type) : SelectiveWalker(env,type) {}

Walker::ApplyStatus TestWalker::operator()(SuifObject *obj) {
    cout << obj->getClassName() << " " <<  (Address) obj << endl;
    return Continue;
    }


void TestWalkerModule::execute() {
  TestWalker walk(_suif_env,SuifObject::get_class_name());
  SuifObject* rootNode = _suif_env->get_file_set_block();
  cout << "List of all nodes " << endl;
  rootNode->walk(walk);
  cout << "List of Statement nodes" << endl;
  TestWalker walk1(_suif_env,Statement::get_class_name());
  rootNode->walk(walk1);
  }

Module *TestWalkerModule::clone() const {
  return(Module*)this;
}

NodeCount::NodeCount( SuifEnv* suif_env ) :
  Module( suif_env ) {
  _module_name = "node-count";
}


void NodeCount::execute() {
  SuifObject* root = (SuifObject*)_suif_env->get_file_set_block();
  printf("The number of Suif objects is: %d\n", object_iterator<SuifObject>( root ).length() );
}

Module* NodeCount::clone() const {
  return (Module*)this;
}

VirtualTests::VirtualTests( SuifEnv* suif_env ) :
  Module( suif_env ) {
  _module_name = "virtual-tests";
}


void VirtualTests::execute() {
  SuifObject* root = (SuifObject*)_suif_env->get_file_set_block();
  int cnt = 0;
  for (Iter<SuifObject> iter = object_iterator<SuifObject>( root );
       iter.is_valid(); iter.next()) {
    SuifObject *so = &iter.current();
    if (is_kind_of<Symbol>(so)) {
      cnt++;
      Symbol *sym = to<Symbol>(so);
      Type *ty = sym->get_type();
      suif_assert(ty != NULL);
    }
  }
  printf("The number of Symbol objects is: %d\n", cnt);
}

Module* VirtualTests::clone() const {
  return (Module*)this;
}
