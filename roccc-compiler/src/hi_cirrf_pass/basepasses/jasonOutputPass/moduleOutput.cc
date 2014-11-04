
#include <cassert>
#include <string>

#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>

#include "moduleOutput.h"

#include "roccc_utils/warning_utils.h"

ModuleGenerator::ModuleGenerator(SuifEnv* e, ProcedureDefinition* p) :
  HiCirrfGenerator(e, p)
{
  ; 
}

ModuleGenerator::~ModuleGenerator()
{
  ;
}

void ModuleGenerator::Setup()
{
  CollectVariables() ;
  CleanupNames() ;
}

LString ModuleGenerator::CleanInputName(LString n)
{
  // If this module was transformed from a system (which could really be
  //  either a new style module or truely a system) we have attached "_in"
  //  to all of the inputs and we must remove them.
  if (procDef->lookup_annote_by_name("TransformedModule") != NULL)
  {
    std::string tmpName = n.c_str() ;
    int position = tmpName.rfind("_in") ;
    assert(position != std::string::npos) ;
    tmpName = tmpName.substr(0, position) ;
    LString cleanName = tmpName.c_str() ;
    return cleanName ;
  }
  return n ;
}

LString ModuleGenerator::CleanOutputName(LString n)
{
  // Transformed modules either have an extra _out appended, 
  //  an _ssaTmp appended, or both.
  
  if (procDef->lookup_annote_by_name("TransformedModule") != NULL)
  {
    std::string tmpName = n.c_str() ;
    int position = tmpName.rfind("_ssaTmp") ;
    if (position == std::string::npos)
    {
      position = tmpName.rfind("_out") ;
    }
    assert(position != std::string::npos) ;
    tmpName = tmpName.substr(0, position) ;
    
    // Make sure the clean name have _out at the end after we have cleaned
    //  the extraneous ones
    if (tmpName.rfind("_out") == std::string::npos)
    {
      tmpName = tmpName + "_out" ;
    }
    
    LString cleanName = tmpName.c_str() ;
    return cleanName ;
  }
  
  return n ;
}


void ModuleGenerator::Output()
{
  OutputHeader() ;
  OutputDeclarations() ;
  OutputFunctionType() ;

  OutputFakes() ;

  OutputLookupTables() ;
  OutputInputScalars() ;

  OutputDatapath(procDef->get_body()) ;

  OutputSizes() ;
  OutputModuleOrder() ;
  OutputModuleStructName() ;

  OutputDebugRegisters() ;
  OutputOutputScalars() ;
  OutputIntrinsicType() ;

  OutputMaximizePrecision() ;
  OutputLookupTableOrder() ;
  OutputFooter() ;

  // Also, print out the roccc.h file
  OutputDotH() ;
}

void ModuleGenerator::OutputDotH()
{
  // Print out the standard declarations
  HiCirrfGenerator::OutputDotH() ;

  rocccOut << std::endl << std::endl ;
  
  // Additionally, find any structs and output them
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* nextObj = symTab->get_symbol_table_object(i) ;
    VariableSymbol* nextVar = dynamic_cast<VariableSymbol*>(nextObj) ;
    if (nextVar == NULL)
    {
      continue ;
    }
    StructType* nextStruct = 
      dynamic_cast<StructType*>(nextVar->get_type()->get_base_type()) ;
    if (nextStruct == NULL)
    {
      continue ;
    }
    rocccOut << "typedef struct" << std::endl ;
    rocccOut << "{" << std::endl ;
    SymbolTable* internalTable = nextStruct->get_group_symbol_table() ;
    for (int j = 0 ; j < internalTable->get_symbol_table_object_count() ; ++j)
    {
      PrintDeclaration(internalTable->get_symbol_table_object(j), true) ;
    }
    rocccOut << "}" << nextStruct->get_name() << " ;" << std::endl ;
  }
}

void ModuleGenerator::OutputDeclarations()
{
  std::list<VariableSymbol*>::iterator printIt = inputs.begin() ;
  fout << "/* Inputs */" << std::endl ;
  while (printIt != inputs.end())
  {
    PrintDeclaration(*printIt) ;
    ++printIt ;
  }
  fout << "/* Outputs */" << std::endl ;
  printIt = outputs.begin() ;
  while (printIt != outputs.end())
  {
    PrintDeclaration(*printIt) ;
    ++printIt ;
  }

  HiCirrfGenerator::OutputDeclarations() ;

}

void ModuleGenerator::OutputFunctionType()
{
  fout << "ROCCCFunctionType(1) ;" << std::endl ;
}

// Modules get all of their inputs and outputs from the struct that
//  is both passed in and returned
void ModuleGenerator::CollectVariables()
{
  assert(procDef != NULL) ;
  ProcedureType* procType  = procDef->get_procedure_symbol()->get_type() ;
  CProcedureType* cProcType = dynamic_cast<CProcedureType*>(procType) ;
  assert(cProcType != NULL) ;
  assert(cProcType->get_argument_count() == 1) ;
  DataType* argType = cProcType->get_argument(0)->get_base_type() ;
  StructType* argStruct = dynamic_cast<StructType*>(argType) ;
  assert(argStruct != NULL) ;
  GroupSymbolTable* structSymTab = argStruct->get_group_symbol_table() ;

  for (int i = 0 ; i < structSymTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* nextObject = structSymTab->get_symbol_table_object(i) ;
    VariableSymbol* nextVar = dynamic_cast<VariableSymbol*>(nextObject) ;
    assert(nextVar != NULL) ;

    DataType* varType = nextVar->get_type()->get_base_type() ;
    if (dynamic_cast<ArrayType*>(varType) != NULL ||
	dynamic_cast<PointerType*>(varType) != NULL)
    {
      OutputError("Arrays are not supported in module interfaces");
      assert(0) ;
    }

    LString objName = nextVar->get_name() ;
    const char* parseMe = objName.c_str() ;
    
    if (strstr(parseMe, "_in") != NULL)
    {
      inputs.push_back(nextVar) ;
    }
    else if (strstr(parseMe, "_out") != NULL)
    {
      nextVar->append_annote(create_brick_annote(theEnv, "Output")) ;
      outputs.push_back(nextVar) ;
    }
    else
    {
      OutputError("Module interface has non input/output variable!") ;
      assert(0) ;
    }
  }  
}

// Because we are type one we can assume that we have a struct as an argument
void ModuleGenerator::PrintFieldAccessExpression(FieldAccessExpression* e)
{
  ProcedureSymbol* procSym = procDef->get_procedure_symbol() ;
  ProcedureType* procType = procSym->get_type() ;
  CProcedureType* cProcType = dynamic_cast<CProcedureType*>(procType) ;
  DataType* argType = cProcType->get_argument(0)->get_base_type() ;
  PointerType* pType = dynamic_cast<PointerType*>(e->get_base_group_address()->get_result_type()) ;
  assert(pType != NULL) ;
  if (pType->get_reference_type() == argType)
  {
    fout << e->get_field()->get_name() ;
  }
}

void ModuleGenerator::OutputModuleOrder()
{
  assert(symTab != NULL) ;

  fout << "ROCCCPortOrder(\"" ;

  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* nextObj = symTab->get_symbol_table_object(i) ;
    ParameterSymbol* nextVar = dynamic_cast<ParameterSymbol*>(nextObj) ;
    if (nextVar == NULL)
    {
      continue ;
    }
    StructType* nextStruct = 
      dynamic_cast<StructType*>(nextVar->get_type()->get_base_type()) ;
    if (nextStruct != NULL)
    {
      SymbolTable* internalTable = nextStruct->get_group_symbol_table() ;
      for (int j = 0 ; j < internalTable->get_symbol_table_object_count() ;
	   ++j)
      {
	if (j != 0)
	{
	  fout << ", " ;
	}
	fout << "/* " << internalTable->get_symbol_table_object(j)->get_name()
	     << " */" ;
      }
    }
  }

  fout << "\") ;" << std::endl ;
}

void ModuleGenerator::OutputModuleStructName()
{
  assert(procDef != NULL) ;
  ProcedureSymbol* procSym = procDef->get_procedure_symbol() ;
  ProcedureType* procType = procSym->get_type() ;
  CProcedureType* cProcType = dynamic_cast<CProcedureType*>(procType) ;
  QualifiedType* qualType = cProcType->get_argument(0) ;
  DataType* argType = qualType->get_base_type() ;
  StructType* inStruct = dynamic_cast<StructType*>(argType) ;

  fout << "ROCCCModuleStructName(\"" ;
  fout << inStruct->get_name() ;
  fout << "\") ;" << std::endl ;
}

void ModuleGenerator::OutputIntrinsicType()
{
  assert(procDef != NULL) ;
  Annote* intrinsicType = procDef->lookup_annote_by_name("Intrinsic") ;
  if (intrinsicType == NULL)
  {
    return ;
  }
  BrickAnnote* intrinsicBrick = dynamic_cast<BrickAnnote*>(intrinsicType) ;
  StringBrick* firstBrick =
    dynamic_cast<StringBrick*>(intrinsicBrick->get_brick(0)) ;
  assert(firstBrick != NULL) ;
  
  String toPrint = firstBrick->get_value() ;
  
  fout << "ROCCCIntrinsicType(\"" ;
  fout << toPrint ;
  fout << "\") ;" << std::endl ;
}
