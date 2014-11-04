// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#ifndef REDUNDANCY_PASS_DOT_H
#define REDUNDANCY_PASS_DOT_H 

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

struct RedundantGroup
{
 public:
  VariableSymbol* input1 ;
  VariableSymbol* input2 ;
  VariableSymbol* input3 ;
  VariableSymbol* output1 ;
  VariableSymbol* output2 ;
  VariableSymbol* output3 ;
} ;

class RedundancyPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  SymbolAddressExpression* tripleVoteAddress ;
  SymbolAddressExpression* doubleVoteAddress ;

  // Used when replicating streams
  SymbolAddressExpression* tripleVoteStreamsAddress ;
  SymbolAddressExpression* doubleVoteStreamsAddress ;

  list<RedundantGroup> voteCalls ;

  void ClearList() ;
  void CreateVoteProcedures() ;

  CallStatement* CreateTripleCall(RedundantGroup x, int copyNumber) ;
  CallStatement* CreateDoubleCall(RedundantGroup x, int copyNumber) ;

  void TripleRedundify(CallStatement* s) ;
  void DoubleRedundify(CallStatement* s) ;

  // Handling redundancy for system-to-system code
  CallStatement* CreateDoubleSplitter(VariableSymbol* x) ;
  CallStatement* CreateTripleSplitter(VariableSymbol* x) ;

 public:
  RedundancyPass(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*)this ; }
  void do_procedure_definition(ProcedureDefinition* proc_def) ;  
} ;

#endif
