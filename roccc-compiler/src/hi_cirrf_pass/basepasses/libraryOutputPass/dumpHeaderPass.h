// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/*
  The purpose of this pass is to read in the repository and print out the
   roccc-library.h file from all the information in there.
 
*/
#ifndef DUMP_HEADER_PASS_DOT_H
#define DUMP_HEADER_PASS_DOT_H

#include <fstream>
#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>
#include <suifkernel/suif_env.h>

class DumpHeaderPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;

  String localDirectory ;
  
  FileSetBlock* repository ;

  std::ofstream hout ;

  void InitializeRepository() ;
  void DumpRepository() ;

  void OutputHeader() ;
  void OutputPreamble() ;
  void OutputPostamble() ;
  void OutputTypes() ;
  void OutputPrototypes() ;

  void OutputTypeDeclaration(DataType* t) ;
  void OutputPrototype(ProcedureSymbol* p) ;

  String StringType(Type* t) ;

 public:
  DumpHeaderPass(SuifEnv* pEnv) ;
  ~DumpHeaderPass() ;

  void initialize() ;
  void execute() ;

  Module* clone() const { return (Module*) this ; }
} ;

#endif
