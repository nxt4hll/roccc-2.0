// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef TRANSFORMS__FUNCTION_DISMANTLERS_H
#define TRANSFORMS__FUNCTION_DISMANTLERS_H

#include "common/suif_copyright.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"
#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"


class DismantleStructuredReturns : public Pass {
    public:
        Module* clone() const {return (Module*)this;}
        void initialize();
        void do_file_set_block( FileSetBlock* file_set_block );

        DismantleStructuredReturns(SuifEnv *env, const LString &name =
                           "dismantle_structured_returns");
    };

class NormalizeProcedureReturns : public PipelinablePass {
public:
  Module* clone() const {return (Module*)this;}
  void initialize();  
  void do_procedure_definition( ProcedureDefinition *pd );
  
  NormalizeProcedureReturns(SuifEnv *env, const LString &name =
			     "normalize_procedure_returns");
};

class RequireProcedureReturns : public PipelinablePass {
public:
  Module* clone() const {return (Module*)this;}
  void initialize();
  void do_procedure_definition( ProcedureDefinition *pd );
  
  RequireProcedureReturns(SuifEnv *env, const LString &name =
			  "require_procedure_returns");
};

#endif


