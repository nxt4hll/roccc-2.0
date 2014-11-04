// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef EXPORT_PASS_DOT_H
#define EXPORT_PASS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class ExportPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  FileSetBlock* repository ;
  ProcedureDefinition* originalProcedure ;

  String localDirectory ;

  CProcedureType* constructedType ;
  ProcedureSymbol* constructedSymbol ;
  
  void ReadRepository() ;
  void DumpRepository() ;
  void ConstructModuleSymbols() ;
  void ConstructSystemSymbols() ;
  void AddSymbols() ;

  DataType* CloneDataType(DataType* t) ;

 public:
  ExportPass(SuifEnv* pEnv) ;
  ~ExportPass() ;
  Module* clone() const { return (Module*) this ; }

  void initialize() ;
  void execute() ;
} ;

#endif
