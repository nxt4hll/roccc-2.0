// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef LIB_OUTPUT_PASS_DOT_H
#define LIB_OUTPUT_PASS_DOT_H

#include <list>
#include <string>
#include <fstream>

#include <cfenodes/cfe.h>
#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "suifkernel/suif_env.h"
#include "common/suif_map.h"

// This class is responsible for the outputting of the hi-cirrf
//  and C libraries to allow the reuse of any compiled module

class Port
{
 public:
  std::string name ;
  bool isInput ;
  int size ;
  bool isFloat ;
} ;

class ModuleEntry 
{
 public:
  std::string name ;
  int delay ;
  std::list<Port> allPorts ;
  int bitSize() ;
} ;

struct LibraryType
{
  bool isFloat ;
  int bitSize ;
} ;

class LibraryOutputPass : public PipelinablePass 
{

 private:

  SuifEnv* theEnv ;

  std::ofstream hout ; // Used for the roccc-library.h file

  ProcedureDefinition* currentProcedure ;
  FileSetBlock* repository ;

  void PrintDeclaration(SymbolTableObject* s) ;
  String StringType(Type* t) ;

  // Adding a procedure declaration for a composable system
  void ExportSystem() ;

  // Adding a module when compiling and everything associated with it
  void UpdateRepository() ;

  // Removing a module and everything associated with it
  bool RemoveModule(String name) ;
  void CollectSymbolsToRemove(String name) ;
  void RemoveProcedure() ;
  void RemoveStruct() ;
  ProcedureSymbol* procedureToRemove ;
  StructType* structToRemove ;

  // Adding a module and everything associated with it
  bool AddModule(String filename) ;
  ModuleEntry ProcessFile(String filename) ;
  void AddStruct(ModuleEntry toAdd) ;
  void AddProcedure(ModuleEntry toAdd) ;
  StructType* structToAdd ;
  ProcedureSymbol* procedureToAdd ;
  
  // The functions that actually output the roccc-library.h file
  void OutputHeader() ;
  void OutputStructs() ;
  void OutputPrototypes() ;
  void OutputTypes() ;

  bool AlreadyPrinted(DataType* t) ;
  std::list<LibraryType> printedTypes ;
  
  void OutputPrototypeOne(ProcedureSymbol* p, CProcedureType* t) ;
  void OutputPrototypeTwo(ProcedureSymbol* p, CProcedureType* t) ;

  void OutputTypeDeclaration(DataType* t) ;
 
  Type* CloneType(Type* originalType) ;

  void InitializeRepository() ;

  void CleanRepository() ;
  void RemoveSystem(String x) ;

  std::list<String> identifiedOutputs ;
  std::list<int> outputNumbers ;

  bool IsOutput(String x) ;
  bool IsOutputNumber(int x) ;

  // This variable is just the type. 0 == Add, 1 == Remove
  int remove ;
  String moduleName ;
  String localDirectory ;

 public:

  // This is automatically called to set up the input parameters
  void initialize() ;

  LibraryOutputPass(SuifEnv *pEnv);
  ~LibraryOutputPass() ;
  Module* clone() const { return (Module*)this; }
  void execute() ;

};

#endif
