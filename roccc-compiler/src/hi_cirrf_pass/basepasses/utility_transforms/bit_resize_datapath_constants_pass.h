// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef BIT_RESIZE_DATAPATH_CONSTANTS_PASS_H 
#define BIT_RESIZE_DATAPATH_CONSTANTS_PASS_H

#include "suifpasses/passes.h"
#include "suifnodes/suif.h"

class BitResizeDatapathConstantsPass : public PipelinablePass {
public:
  BitResizeDatapathConstantsPass( SuifEnv* suif_env );
  virtual ~BitResizeDatapathConstantsPass() {} 

  virtual Module* clone() const { return(Module*)this; }
  virtual void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif

