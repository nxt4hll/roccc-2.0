#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>

#include "composedOutput.h"

ComposedGenerator::ComposedGenerator(SuifEnv* e, ProcedureDefinition* p,
					 String s) :
  SystemGenerator(e, p, s)
{
  ; // Nothing yet...
}

ComposedGenerator::~ComposedGenerator()
{
  ; // Nothing to delete yet...
}

void ComposedGenerator::Output()
{
  OutputHeader() ;
  OutputDeclarations() ;
  OutputFunctionType() ;
  OutputFakes() ;
  OutputInputScalars() ;
  OutputInputArrays() ;
  //  PrintInputLoopFifos() ;
 
  OutputDatapath(procDef->get_body()) ;
  
  OutputSizes() ;
  OutputSteps() ;
  OutputNumChannels() ;
  OutputDebugRegisters() ;
  //  PrintOutputLoopFifos() ;
  OutputOutputScalars() ;
  OutputOutputArrays() ;

  OutputMaximizePrecision() ;
  OutputSystolicInitialization() ;
  OutputFooter() ;
  OutputDotH() ;
}

void ComposedGenerator::OutputDotH()
{
  HiCirrfGenerator::OutputDotH() ;
}

void ComposedGenerator::OutputFakes()
{
  assert(symTab != NULL) ;

  // Everything should have a fake
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* currentObject = symTab->get_symbol_table_object(i) ;
    VariableSymbol* currentVar = dynamic_cast<VariableSymbol*>(currentObject) ;
    if (currentVar != NULL)
    {
      OutputFake(currentVar) ;
    }    
  }
  HiCirrfGenerator::OutputFakes() ;
}

void ComposedGenerator::OutputDeclarations()
{
  assert(symTab != NULL) ;

  fout << "/* Inputs */" << std::endl ;
  std::list<VariableSymbol*>::iterator printIter = inputs.begin() ;
  while (printIter != inputs.end())
  {
    PrintDeclaration(*printIter) ;
    (*printIter)->append_annote(create_brick_annote(theEnv, "Printed")) ;
    ++printIter ;
  }
  fout << "/* Outputs */" << std::endl ;
  printIter = outputs.begin() ;
  while (printIter != outputs.end())
  {
    PrintDeclaration(*printIter) ;
    (*printIter)->append_annote(create_brick_annote(theEnv, "Printed")) ;
    ++printIter ;
  }
  fout << "/* Temporaries */" << std::endl ;

  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* currentObject = symTab->get_symbol_table_object(i) ;
    if (dynamic_cast<VariableSymbol*>(currentObject) != NULL &&
	currentObject->lookup_annote_by_name("Printed") == NULL)
    {
      PrintDeclaration(currentObject) ;
    }
  }
}

void ComposedGenerator::OutputFunctionType()
{
  // Until the lo-cirrf side recognizes exportable systems, just act like
  //  this is a normal system
  fout << "ROCCCFunctionType(3) ;" << std::endl ;
}
