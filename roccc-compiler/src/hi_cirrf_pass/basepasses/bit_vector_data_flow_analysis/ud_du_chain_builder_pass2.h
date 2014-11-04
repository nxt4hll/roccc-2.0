// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

  This pass is responsible for annotating each load variable expression
   with a list of all the definitions that reach that load.  This pass
   also annotes each store variable statement with the list of all uses
   that reach it.  Each use will have to be a load variable expression.

   Each definition will be either a store variable expression or a call
   expression.  Call expressions, however, can define multiple different
   variables. Because of this, we have to be very careful about not confusing
   the definitions up.  For this, we have to be very dilligent in our
   chains going the other way.

*/

#ifndef UD_DU_CHAIN_BUILDER_PASS_TWO__H
#define UD_DU_CHAIN_BUILDER_PASS_TWO__H

#include <map>

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

class UD_DU_ChainBuilderPass2 : public PipelinablePass 
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ; // The current procDef

  void SetupAnnotations() ;
  void BuildChains() ;

  // This map is responsible for identifying the statement associated
  //  with the proper location in each bit vector that was passed in
  //  from the previous Pass.
  std::map< int, Statement* > reverseMap ;

  void InitializeMap() ;

  // If we have been called multiple times, then we need to clear
  //  the reverseMap
  void ClearMap() ;

  void DumpChains() ;

 public:
  UD_DU_ChainBuilderPass2(SuifEnv *pEnv);
  ~UD_DU_ChainBuilderPass2() ;
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
} ;

#endif
