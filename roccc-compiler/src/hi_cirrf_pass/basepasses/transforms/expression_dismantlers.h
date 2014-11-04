// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef TRANSFORMS__DISMANTLE_EXPRESSIONS
#define TRANSFORMS__DISMANTLE_EXPRESSIONS


#include "suifkernel/module_subsystem.h"
#include "suifkernel/suif_env.h"
#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "procedure_walker_utilities.h"

class field_access_expression_walker : public ProcedureWalker {
    public:
        field_access_expression_walker(SuifEnv *the_env,ProcedureDefinition *def);
        Walker::ApplyStatus operator () (SuifObject *x);
    };

class call_expression_walker : public ProcedureWalker {
    public:
        call_expression_walker(SuifEnv *the_env,ProcedureDefinition *def);
        Walker::ApplyStatus operator () (SuifObject *x);
  
    };

class CallExpressionDismantlerPass : public PipelinablePass {
public:
  CallExpressionDismantlerPass(SuifEnv *the_env,  const LString &name = 
			       "dismantle_call_expressions");
  Module *clone() const;
  void initialize();
  void do_procedure_definition(ProcedureDefinition *pd);
};


class LoadExpressionDismantlerPass : public PipelinablePass {
public:
  LoadExpressionDismantlerPass(SuifEnv *the_env,  const LString &name = 
			       "dismantle_load_expressions");
  Module *clone() const;
  void initialize();
  void do_procedure_definition(ProcedureDefinition *pd);
};

class FieldBuilderPass : public PipelinablePass {
public:
  FieldBuilderPass(SuifEnv *the_env,  const LString &name = 
		   "build_field_access_expressions");
  Module *clone() const;
  void initialize();
  void do_procedure_definition(ProcedureDefinition *pd);
protected:
  virtual bool process_a_field_expression(Expression *expr);
};

class RepeatValueBlockBuilderPass : public Pass {
public:
  RepeatValueBlockBuilderPass(SuifEnv *the_env,  const LString &name = 
			      "build_repeat_value_blocks");
  void initialize();
  Module *clone() const;
  void do_file_set_block(FileSetBlock *fsb);
};


// no longer needed
//class extract_fields_expression_walker : public ProcedureWalker{
//    public:
//        extract_fields_expression_walker(SuifEnv *the_env,ProcedureDefinition *def);
//        Walker::ApplyStatus operator () (Object *x);
//    };

// not yet implemented
// class extract_elements_expression_walker : public ProcedureWalker{
//     public:
//         extract_elements_expression_walker(SuifEnv *the_env,ProcedureDefinition *def);
//         Walker::ApplyStatus operator () (Object *x);
//     };

// class byte_size_of_expression_walker : public ProcedureWalker{
//     public:
//         byte_size_of_expression_walker(SuifEnv *the_env,ProcedureDefinition *def);
//         Walker::ApplyStatus operator () (Object *x);
//     };

class FoldStatementsPass : public PipelinablePass {
public:
  FoldStatementsPass(SuifEnv *the_env,  const LString &name = 
		    "fold_statements");
  void initialize();
  Module *clone() const;
  void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif /* TRANSFORMS__DISMANTLE_IF_H */
