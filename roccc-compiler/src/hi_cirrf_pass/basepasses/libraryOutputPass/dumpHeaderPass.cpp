// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include <cassert>
#include <cstring>
#include <sstream>

#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>
#include <suifkernel/command_line_parsing.h>

#include "roccc_utils/warning_utils.h"

#include "dumpHeaderPass.h"

DumpHeaderPass::DumpHeaderPass(SuifEnv* pEnv) : 
  PipelinablePass(pEnv, "DumpHeaderPass")
{
  theEnv = pEnv ;
  repository = NULL ;
}

DumpHeaderPass::~DumpHeaderPass()
{
  ; // Nothing to delete yet
}

void DumpHeaderPass::initialize()
{
  PipelinablePass::initialize() ;
  _command_line->set_description("Dumps the roccc-library.h file") ;
  OptionString* directory = new OptionString("Local Dir", &localDirectory) ;
  OptionList* args = new OptionList() ;
  args->add(directory) ;
  _command_line->add(args) ;
}

void DumpHeaderPass::execute()
{
  OutputInformation("Dump Header Pass begins") ;
  InitializeRepository() ;
  OutputHeader() ;
  OutputInformation("Dump Header Pass ends") ;
}

void DumpHeaderPass::InitializeRepository()
{
  std::string fullPath = localDirectory.c_str() ;
  fullPath += "/repository.suif" ;
  theEnv->read(fullPath.c_str()) ;
  repository = theEnv->get_file_set_block() ;
}

void DumpHeaderPass::DumpRepository()
{
  assert(theEnv != NULL) ;
  std::string fullPath = localDirectory.c_str() ;
  fullPath += "/repository.suif" ;
  theEnv->write(fullPath.c_str()) ;
}

void DumpHeaderPass::OutputHeader()
{
  // Open the file
  std::string fullPath = localDirectory.c_str() ;
  fullPath += "/roccc-library.h" ;
  
  hout.open(fullPath.c_str()) ;
  if (!hout)
  {
    std::stringstream errorMsg ;
    errorMsg << "Cannot open " << fullPath << "!" << std::endl ;
    OutputError(errorMsg.str().c_str()) ;
    assert(0) ;
  }

  OutputPreamble() ;
  OutputTypes() ;
  hout << "#ifdef __cplusplus" << std::endl ;
  OutputPrototypes() ;
  hout << "#endif" << std::endl ;
  OutputPostamble() ;
}

void DumpHeaderPass::OutputPreamble()
{
  hout << "/*" << std::endl ;
  hout << " This file contains all of the exported functions" << std::endl ;
  hout << "  that can be called as hardware modules.  This file is" 
       << std::endl ;
  hout << "  automatically generated and updated.  Do not modify." 
       << std::endl ;
  hout << "*/" << std::endl ;
  hout << "#ifndef ROCCC_LIBRARY_DOT_H" << std::endl ;
  hout << "#define ROCCC_LIBRARY_DOT_H" << std::endl ;  
}

void DumpHeaderPass::OutputPostamble()
{
  hout << std::endl << "#endif" << std::endl ;
}

void DumpHeaderPass::OutputTypes()
{
  assert(repository != NULL) ;
  SymbolTable* symTab = repository->get_external_symbol_table() ;
  
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    DataType* nextType = 
      dynamic_cast<DataType*>(symTab->get_symbol_table_object(i)) ;
    if (nextType != NULL)
    {
      OutputTypeDeclaration(nextType) ;
    }
  }
}

void DumpHeaderPass::OutputTypeDeclaration(DataType* t)
{
  IntegerType* intType = dynamic_cast<IntegerType*>(t) ;
  FloatingPointType* floatType = dynamic_cast<FloatingPointType*>(t) ;
  PointerType* pointerType = dynamic_cast<PointerType*>(t) ;
  ReferenceType* refType = dynamic_cast<ReferenceType*>(t) ;
  if (intType != NULL)
  {
    hout << "typedef int ROCCC_int" ;
  }
  else if (floatType != NULL)
  {
    hout << "typedef float ROCCC_float" ;
  }
  else if (pointerType != NULL)
  {
    QualifiedType* pointQual = 
      dynamic_cast<QualifiedType*>(pointerType->get_reference_type()) ;
    assert(pointQual != NULL) ;
    OutputTypeDeclaration(pointQual->get_base_type()) ;
    return ;
  }
  else if (refType != NULL)
  {
    QualifiedType* refQual =
      dynamic_cast<QualifiedType*>(refType->get_reference_type()) ;
    assert(refQual != NULL) ;
    OutputTypeDeclaration(refQual->get_base_type()) ;
    return ;
  }
  else
  {  
    return ;
  }
  hout << t->get_bit_size().c_int() << " ;" << std::endl ;
}

void DumpHeaderPass::OutputPrototypes()
{
  assert(repository != NULL)  ;
  SymbolTable* symTab = repository->get_external_symbol_table() ;
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    ProcedureSymbol* currentProc = 
      dynamic_cast<ProcedureSymbol*>(symTab->get_symbol_table_object(i)) ;
    if (currentProc != NULL)
    {
      OutputPrototype(currentProc) ;
    }
  }
}

void DumpHeaderPass::OutputPrototype(ProcedureSymbol* p)
{
  assert(p != NULL) ;
  CProcedureType* procType =
    dynamic_cast<CProcedureType*>(p->get_type()) ;
  assert(procType != NULL) ;
  //  assert(procType->get_argument_count() > 0) ;

  hout << StringType(procType->get_result_type()) << " " ;
  hout << p->get_name() << "(" ;
  for (unsigned int i = 0 ; i < procType->get_argument_count() ; ++i)
  {
    if (i != 0)
    {
      hout << ", " ;
    }
    hout << StringType(procType->get_argument(i)) ;
    if (procType->get_argument(i)->lookup_annote_by_name("Output") != NULL)
    {
      QualifiedType* qualType = 
	dynamic_cast<QualifiedType*>(procType->get_argument(i)) ;
      assert(qualType != NULL) ;
      if (dynamic_cast<ReferenceType*>(qualType->get_base_type()) == NULL)
      {
	hout << "&" ;
      }
    }
  }
  hout <<  ") ;" << std::endl ;

}

String DumpHeaderPass::StringType(Type* t)
{
  String toReturn = "" ;
  assert(t != NULL) ;

  if (dynamic_cast<ArrayType*>(t) != NULL)
  {
    toReturn += StringType(dynamic_cast<ArrayType*>(t)->get_element_type()) ;
  }
  else if (dynamic_cast<MultiDimArrayType*>(t) != NULL)
  {
    toReturn += StringType(dynamic_cast<MultiDimArrayType*>(t)->get_element_type()) ;
  }
  else if (dynamic_cast<DataType*>(t) != NULL)
  {
    if (dynamic_cast<PointerType*>(t) != NULL)
    {
      toReturn += StringType(dynamic_cast<PointerType*>(t)->get_reference_type());
      toReturn +="*" ;
    }
    if (dynamic_cast<ReferenceType*>(t) != NULL)
    {
      toReturn += StringType(dynamic_cast<ReferenceType*>(t)->get_reference_type()) ;
      toReturn += "&" ;
    }
    if (dynamic_cast<IntegerType*>(t) != NULL)
    {
      std::stringstream convert ; 
      convert << "ROCCC_int" 
	      << dynamic_cast<IntegerType*>(t)->get_bit_size().c_int() ;
      toReturn += convert.str().c_str() ;
    }
    if (dynamic_cast<FloatingPointType*>(t) != NULL)
    {
      std::stringstream convert ;
      convert << "ROCCC_float"
	      << dynamic_cast<FloatingPointType*>(t)->get_bit_size().c_int() ;
      toReturn += convert.str().c_str() ;
    }
    if (dynamic_cast<GroupType*>(t) != NULL)
    {
      toReturn += t->get_name() ;
    }
    if (dynamic_cast<VoidType*>(t) != NULL)
    {
      toReturn += "void" ;
    }
  }
  else if (dynamic_cast<QualifiedType*>(t) != NULL)
  {
    toReturn += StringType(dynamic_cast<QualifiedType*>(t)->get_base_type()) ;
  }
  return toReturn ;
}
