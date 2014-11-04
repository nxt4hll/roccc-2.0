#ifndef CREATE_SUIF_COMPLEX_INPUT_H
#define CREATE_SUIF_COMPLEX_INPUT_H
/* file "create_suif_complex_input.h" */



#include "suifkernel/suifkernel_forwarders.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/suif_env.h"

#include "suifpasses/suifpasses.h"
#include "basicnodes/basic_forwarders.h"
#include "suifnodes/suif_forwarders.h"
#include "typebuilder/type_builder_forwarders.h"
//class TypeBuilder;
//class TypeBuilder;

class CreateSuifComplexInputPass : public FrontendPass {
public:
  CreateSuifComplexInputPass(SuifEnv *env,
		     const LString &name = "create_suif_complex_input");
  virtual ~CreateSuifComplexInputPass(void);


  virtual Module* clone() const;

  virtual FileSetBlock *build_file_set_block();
private:
  virtual ProcedureDefinition* build_main( const char* main_file );
  TypeBuilder *_tb;
  IntegerType* _char_type;
  QualifiedType* _q_char_type;
  FloatingPointType* _fpt;
  QualifiedType* _q_fpt;
  CProcedureType* _printf_type;
  ArrayType* _string_literal_type;
  QualifiedType* _q_string_literal_type;
  CProcedureType* _main_type;
  IntegerType* _argc_type;
  QualifiedType* _q_argc_type;
  PointerType* _argv_type;
  QualifiedType* _q_argv_type;
};

#endif /* CREATE_SUIF_COMPLEX_INPUT_H */
