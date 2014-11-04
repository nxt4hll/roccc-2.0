// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include <cassert>

#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>
#include <suifkernel/command_line_parsing.h>
#include <suifkernel/suif_env.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

#include "exportPass.h"

ExportPass::ExportPass(SuifEnv* pEnv) : PipelinablePass(pEnv, "ExportPass")
{
  theEnv = pEnv ;
  repository = NULL ;
  constructedType = NULL ;
  constructedSymbol = NULL ;
  originalProcedure = NULL ;
}

ExportPass::~ExportPass()
{
  ; // Nothing to clean up yet...
}

void ExportPass::initialize()
{
  PipelinablePass::initialize() ;
  _command_line->set_description("Add either a system or a module to the repository from the current suif file") ;
  OptionString* directory = new OptionString("Local Dir", &localDirectory) ;
  OptionList* args = new OptionList() ;
  args->add(directory) ;
  _command_line->add(args) ;
}

void ExportPass::execute()
{
  assert(theEnv != NULL) ;

  OutputInformation("Export pass begins") ;

  // Get the information regarding the current procedure by looking at
  //  the first procedure from the first file in the file set block.

  Iter<FileBlock*> temp =
    theEnv->get_file_set_block()->get_file_block_iterator() ;
  Iter<ProcedureDefinition*> temp2 = (temp.current())->get_definition_block()->get_procedure_definition_iterator() ;

  originalProcedure = temp2.current() ;
  assert(originalProcedure != NULL) ;

  // Modules have been transformed into structs so all the previous 
  //  passes work, and here we have to do a little bit of backtracking
  //  and convert them to the straight model.
  BrickAnnote* rocccType = 
    dynamic_cast<BrickAnnote*>(originalProcedure->lookup_annote_by_name("FunctionType")) ;
  assert(rocccType != NULL) ;
  IntegerBrick* valueBrick = 
    dynamic_cast<IntegerBrick*>(rocccType->get_brick(0)) ;
  assert(valueBrick != NULL) ;
  int functionType = valueBrick->get_value().c_int() ;
  delete rocccType->remove_brick(0) ;
  delete originalProcedure->remove_annote_by_name("FunctionType") ;

  if (functionType == 1) // Module
  {
    // De-modulize it
    ConstructModuleSymbols() ;
  }
  else if (functionType == 2 || functionType == 3) // Systems
  {
    // Just create clones
    ConstructSystemSymbols() ;
  }
  else
  {
    assert(0 && "Trying to export something unknown") ;
  }

  ReadRepository() ;  
  AddSymbols() ;

  DumpRepository() ;

  OutputInformation("Export pass ends") ;
}

void ExportPass::ReadRepository()
{
  std::string fullPath = localDirectory.c_str() ;
  fullPath += "/repository.suif" ;
  theEnv->read(fullPath.c_str()) ;
  repository = theEnv->get_file_set_block() ;
}

void ExportPass::DumpRepository()
{
  assert(theEnv != NULL) ;
  std::string fullPath = localDirectory.c_str() ;
  fullPath += "/repository.suif" ;
  theEnv->write(fullPath.c_str()) ;
}

void ExportPass::ConstructModuleSymbols()
{
  CProcedureType* originalType = dynamic_cast<CProcedureType*>(originalProcedure->get_procedure_symbol()->get_type()) ;
  assert(originalType != NULL) ;

  // The original type takes and returns a struct.  We need to change this
  //  to a list of arguments.

  VoidType* newReturnType = create_void_type(theEnv, IInteger(0), 0) ;
  constructedType = create_c_procedure_type(theEnv,
					    newReturnType,
					    false, // has varargs
					    true, // arguments_known
					    0, // bit alignment
					    LString("ConstructedType")) ;

  StructType* returnType = 
    dynamic_cast<StructType*>(originalType->get_result_type()) ;
  assert(returnType != NULL) ;

  SymbolTable* structSymTab = returnType->get_group_symbol_table() ;
  assert(structSymTab != NULL) ;

  for (int i = 0 ; i < structSymTab->get_symbol_table_object_count() ; ++i)
  {
    VariableSymbol* nextVariable = 
      dynamic_cast<VariableSymbol*>(structSymTab->get_symbol_table_object(i));
    if (nextVariable != NULL)
    {
      // Check to see if this is an output or not
      QualifiedType* cloneType ;
      DataType* cloneBase = 
	dynamic_cast<DataType*>(nextVariable->get_type()->get_base_type()->deep_clone()) ;
      assert(cloneBase != NULL) ;
      cloneType = create_qualified_type(theEnv, cloneBase) ;
 
      if (nextVariable->lookup_annote_by_name("Output") != NULL)
      {
	cloneType->append_annote(create_brick_annote(theEnv, "Output")) ;
	// Why doesn't this stick around?
      }
      constructedType->append_argument(cloneType) ;
    }
  }

  constructedSymbol = create_procedure_symbol(theEnv,
					      constructedType,
					      originalProcedure->get_procedure_symbol()->get_name()) ;
  constructedSymbol->set_definition(NULL) ;

}

void ExportPass::ConstructSystemSymbols()
{
  ProcedureSymbol* originalSymbol = originalProcedure->get_procedure_symbol() ;
  assert(originalSymbol != NULL) ;
  CProcedureType* originalType = 
    dynamic_cast<CProcedureType*>(originalSymbol->get_type()) ;
  assert(originalType != NULL) ;

  constructedType = create_c_procedure_type(theEnv,
    dynamic_cast<DataType*>(originalType->get_result_type()->deep_clone()),
					    false, // has variable arguments
					    false, // arguments known
					    0) ; // bit alignment

  // The system has been written in one of two ways, either the old
  //  way where there are no arguments, or the new way where everything
  //  is put into the arguments.
  
  if (originalType->get_argument_count() > 0)
  {
    for (int i = 0 ; i < originalType->get_argument_count() ; ++i)
    {
      QualifiedType* originalArgument = originalType->get_argument(i) ;
      DataType* originalBase = originalArgument->get_base_type() ;
      DataType* constructedBase = CloneDataType(originalBase) ;
      QualifiedType* constructedArgument = 
	create_qualified_type(theEnv, constructedBase) ;
      constructedType->append_argument(constructedArgument) ;

      // Go through the symbol table and find the parameter symbol 
      //  that matches the parameter number, and check to see if it
      //  is an output or not...
      SymbolTable* symTab = originalProcedure->get_symbol_table() ;
      ParameterSymbol* correspondingSymbol = NULL ;
      for (int j = 0 ; j < symTab->get_symbol_table_object_count() ; ++j)
      {
	ParameterSymbol* currentSym = 
	  dynamic_cast<ParameterSymbol*>(symTab->get_symbol_table_object(j)) ;
	if (currentSym != NULL)
	{
	  BrickAnnote* orderAnnote = dynamic_cast<BrickAnnote*>(currentSym->lookup_annote_by_name("ParameterOrder")) ;
	  assert(orderAnnote != NULL) ;
	  IntegerBrick* orderBrick = 
	    dynamic_cast<IntegerBrick*>(orderAnnote->get_brick(0)) ;
	  assert(orderBrick != NULL) ;
	  if (orderBrick->get_value().c_int() == i)
	  {
	    correspondingSymbol = currentSym ;
	    break ;
	  }
	}
      }
      if (correspondingSymbol != NULL)
      {
        if (correspondingSymbol->lookup_annote_by_name("OutputScalar") != NULL ||
	    correspondingSymbol->lookup_annote_by_name("OutputVariable") != NULL ||
	    correspondingSymbol->lookup_annote_by_name("OutputFifo") != NULL)
        {
	  constructedArgument->append_annote(create_brick_annote(theEnv,
								 "Output")) ;
        }
      }
      //      if (dynamic_cast<ReferenceType*>(originalBase) != NULL)
      //{
      //	constructedArgument->append_annote(create_brick_annote(theEnv,
      //						       "Output")) ;
      //      }
    }
  }
  else
  {   
    SymbolTable* symTab = originalProcedure->get_symbol_table() ;
    assert(symTab != NULL) ;
    list<VariableSymbol*> inputScalars ;
    list<VariableSymbol*> inputFifos ; 
    list<VariableSymbol*> outputScalars ;
    list<VariableSymbol*> outputFifos ;

    for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
    {
      VariableSymbol* currentVar = 
	dynamic_cast<VariableSymbol*>(symTab->get_symbol_table_object(i)) ;
      if (currentVar != NULL &&
	  currentVar->lookup_annote_by_name("InputScalar") != NULL &&
	  currentVar->lookup_annote_by_name("TemporalFeedback") == NULL &&
	  currentVar->lookup_annote_by_name("NormalFeedback") == NULL &&
	  currentVar->lookup_annote_by_name("DebugRegister") == NULL)
      {
	inputScalars.push_back(currentVar) ;
      }
      if (currentVar != NULL &&
	  currentVar->lookup_annote_by_name("InputFifo") != NULL)
      {
	inputFifos.push_back(currentVar) ;
      }
      if (currentVar != NULL &&
	  currentVar->lookup_annote_by_name("OutputVariable") != NULL &&
	  currentVar->lookup_annote_by_name("Dummy") == NULL &&
	  currentVar->lookup_annote_by_name("FeedbackSource") == NULL)
      {
	outputScalars.push_back(currentVar) ;
      }
      if (currentVar != NULL &&
	  currentVar->lookup_annote_by_name("OutputFifo") != NULL)
      {
	outputFifos.push_back(currentVar) ;
      }
    }
    // Add the types of the input scalars, then the input fifos, 
    //  then the output scalars, and finally the output fifos.
    list<VariableSymbol*>::iterator varIter = inputScalars.begin() ;
    while (varIter != inputScalars.end())
    {
      QualifiedType* originalQual = (*varIter)->get_type() ;
      assert(originalQual != NULL) ;
      DataType* originalBase = originalQual->get_base_type() ;
      assert(originalBase != NULL) ;
      DataType* constructedBase = 
	dynamic_cast<DataType*>(originalBase->deep_clone()) ;
      QualifiedType* constructedQual = 
	create_qualified_type(theEnv, constructedBase) ;
      constructedType->append_argument(constructedQual) ;
      ++varIter ;
    }
    varIter = inputFifos.begin() ;
    while (varIter != inputFifos.end())
    {
      QualifiedType* originalQual = (*varIter)->get_type() ;
      assert(originalQual != NULL) ;
      DataType* originalBase = originalQual->get_base_type() ;
      assert(originalBase != NULL) ;
      // Fifos will have pointer types or reference types.  A 
      //  simple deep clone will not suffice, I need to build up a
      //  new type from the bottom up.
      DataType* constructedBase = CloneDataType(originalBase) ;
      assert(constructedBase != NULL) ;
      QualifiedType* constructedQual = 
	create_qualified_type(theEnv, constructedBase) ;
      assert(constructedBase != NULL) ;
      assert(constructedType != NULL) ;
      constructedType->append_argument(constructedQual) ;
      ++varIter ;
    }
    varIter = outputScalars.begin() ;
    while (varIter != outputScalars.end())
    {
      QualifiedType* originalQual = (*varIter)->get_type() ;
      DataType* originalBase = originalQual->get_base_type() ;
      DataType* constructedBase = CloneDataType(originalBase) ;
      QualifiedType* constructedQual = 
	create_qualified_type(theEnv, constructedBase) ;
      constructedQual->append_annote(create_brick_annote(theEnv, "Output")) ;
      constructedType->append_argument(constructedQual) ;
      ++varIter ;
    }
    varIter = outputFifos.begin() ;
    while (varIter != outputFifos.end())
    {
      QualifiedType* originalQual = (*varIter)->get_type() ;
      assert(originalQual != NULL) ;
      DataType* originalBase = originalQual->get_base_type() ;
      assert(originalBase != NULL) ;
      DataType* constructedBase = CloneDataType(originalBase) ;
      assert(constructedBase != NULL) ;
      QualifiedType* constructedQual = 
	create_qualified_type(theEnv, constructedBase) ;
      assert(constructedQual != NULL) ;
      constructedQual->append_annote(create_brick_annote(theEnv, "Output")) ;
      assert(constructedType != NULL) ;
      constructedType->append_argument(constructedQual) ;
      ++varIter ;
    }
    
  }
  constructedSymbol = create_procedure_symbol(theEnv, constructedType,
					      originalProcedure->get_procedure_symbol()->get_name()) ;
}

void ExportPass::AddSymbols()
{
  assert(repository != NULL) ;
  assert(constructedType != NULL) ;
  assert(constructedSymbol != NULL) ;

  // By this point, the repository has been read in and the symbol table
  //  has been completely overwritten.

  SymbolTable* symTab = repository->get_external_symbol_table() ;
  assert(symTab != NULL) ;

  DataType* resultType = constructedType->get_result_type() ;
  bool resultReplaced = false ;
  for (int j = 0 ; j < symTab->get_symbol_table_object_count() ; ++j)
  {
    DataType* existingType = 
      dynamic_cast<DataType*>(symTab->get_symbol_table_object(j)) ;
    if (existingType != NULL && EquivalentTypes(resultType, existingType))
    {
      constructedType->set_result_type(existingType) ;
      resultReplaced = true ;
      delete resultType ;
      break ;
    }
  }
  if (!resultReplaced)
  {
    symTab->append_symbol_table_object(resultType) ;
  }
  
  // Go through all of the arguments and replace them with appropriate types
  //  if they already exist in the symbol table.
  for (int i = 0 ; i < constructedType->get_argument_count() ; ++i)
  {
    QualifiedType* currentArg = constructedType->get_argument(i) ;
    assert(currentArg != NULL) ;
    bool replaced = false ;
    for (int j = 0 ; j < symTab->get_symbol_table_object_count() ; ++j)
    {
      QualifiedType* existingType = 
	dynamic_cast<QualifiedType*>(symTab->get_symbol_table_object(j)) ;
      if (existingType != NULL && EquivalentTypes(currentArg, existingType) &&
	  existingType->get_annote_count() == currentArg->get_annote_count())
      {
	constructedType->replace_argument(i, existingType) ;
	replaced = true ;
	break ;
      }
    }    
    if (replaced == false)
    {
      symTab->append_symbol_table_object(currentArg) ;
      symTab->append_symbol_table_object(currentArg->get_base_type()) ;
    }
  }

  symTab->append_symbol_table_object(constructedType) ;
  symTab->append_symbol_table_object(constructedSymbol) ;
}

DataType* ExportPass::CloneDataType(DataType* t)
{
  assert(t != NULL) ;
  PointerType* pointerClone = dynamic_cast<PointerType*>(t) ;
  ReferenceType* referenceClone = dynamic_cast<ReferenceType*>(t) ;
  ArrayType* arrayClone = dynamic_cast<ArrayType*>(t) ;
  if (pointerClone != NULL)
  {
    QualifiedType* refType = 
      dynamic_cast<QualifiedType*>(pointerClone->get_reference_type()) ;
    assert(refType != NULL) ;
    DataType* cloneType = CloneDataType(refType->get_base_type()) ;
    assert(cloneType != NULL) ;

    return create_pointer_type(theEnv, 
			       IInteger(32),
			       0,
			       create_qualified_type(theEnv, cloneType)) ;
  }
  if (referenceClone != NULL)
  {
    QualifiedType* refType = 
      dynamic_cast<QualifiedType*>(referenceClone->get_reference_type()) ;
    assert(refType != NULL) ;
    DataType* clonedType = CloneDataType(refType->get_base_type()) ;
    
    return create_reference_type(theEnv,
				 IInteger(32),
				 0,
				 create_qualified_type(theEnv, clonedType)) ;
  }
  if (arrayClone != NULL)
  {
    QualifiedType* elementType = arrayClone->get_element_type() ;
    DataType* internalType = CloneDataType(elementType->get_base_type()) ;
    QualifiedType* finalQual = create_qualified_type(theEnv, internalType) ;
    return create_pointer_type(theEnv,
			       IInteger(32),
			       0, 
			       finalQual) ;    
  }
  return dynamic_cast<DataType*>(t->deep_clone()) ;
}
