// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/* 
  This file contains the declaration of the pass that is responsible
   for identifying uses of fifos in ROCCC 2.0.

*/

#ifndef __FIFO_IDENTIFICATION_DOT_H__
#define __FIFO_IDENTIFICATION_DOT_H__

#include <string>

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "cfenodes/cfe.h"

class FifoIdentificationPass : public PipelinablePass 
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  int tempNumber ;

  std::string tempName() ;

  VariableSymbol* DetermineIndex(Expression* e) ;
  Constant* DetermineOffset(Expression* e) ;

  void HandleComposedSystems() ;

  void IdentifyInputFifos(CForStatement* c) ;
  void IdentifyOutputFifos(CForStatement* p) ;
  void LabelSmartBuffers(CForStatement* p, Symbol* array) ;  

  // This boolean value is used to tell the later passes if we have fifos
  //  only or a mixture of fifos and smart buffers
  bool fifosOnly ; 

 public:
  FifoIdentificationPass(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* p) ;
} ;

#endif
