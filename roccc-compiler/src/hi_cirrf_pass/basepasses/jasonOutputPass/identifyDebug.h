
#ifndef IDENTIFY_DEBUG_DOT_H
#define IDENTIFY_DEBUG_DOT_H

#include <fstream>
#include <list>

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>
#include <suifkernel/suif_env.h>

class IdentifyDebugPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  String filename ;

  std::ifstream fin ;

  void ProcessFile() ;

  bool InList(std::list<std::string>& toCheck, std::string& name) ;

  void ProcessSymbolTable(SymbolTable* symTab, 
			  std::list<std::string>& debugRegsisters,
			  std::list<int>& watchPoints,
			  std::list<int>& watchValid) ;

 public:
  IdentifyDebugPass(SuifEnv* pEnv) ;
  ~IdentifyDebugPass() ;
  Module* clone() const { return (Module*)this ; }
  void do_procedure_definition(ProcedureDefinition* proc_def) ;
  void initialize() ;
} ;

#endif
