// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#include <iostream>
#include <map>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

#include "transform_systems_to_modules.h"

#include <suifnodes/suif_factory.h>
#include <basicnodes/basic_factory.h>
#include <suifkernel/utilities.h>

TransformSystemsToModules::TransformSystemsToModules(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "TransformSystemsToModules") 
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void TransformSystemsToModules::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;
  OutputInformation("Transform Systems To Modules Pass Begins") ;

  if (!HasLoop() && !PassedStruct() && !ComposedSystem())
  {
    HandleFeedback() ;
    Transform() ;
    procDef->append_annote(create_brick_annote(theEnv, "TransformedModule")) ;
  }

  OutputInformation("Transform Systems To Modules Pass Ends") ;  
}

bool TransformSystemsToModules::HasLoop()
{
  assert(procDef != NULL) ;
  Iter<CForStatement> forIter = 
    object_iterator<CForStatement>(procDef->get_body()) ;
  return forIter.is_valid() ;
}

bool TransformSystemsToModules::PassedStruct()
{
  assert(procDef != NULL) ;
  ProcedureSymbol* procSym = procDef->get_procedure_symbol() ;
  assert(procSym != NULL) ;
  ProcedureType* procType = procSym->get_type() ;
  assert(procType != NULL) ;
  CProcedureType* cProcType = dynamic_cast<CProcedureType*>(procType) ;
  assert(cProcType != NULL) ;
  
  return ((cProcType->get_argument_count() == 1) &&
	  (dynamic_cast<StructType*>(cProcType->get_argument(0)->get_base_type()))) ;
}

bool TransformSystemsToModules::ComposedSystem()
{
  assert(procDef != NULL) ;
  return (procDef->lookup_annote_by_name("ComposedSystem") != NULL) ;
}

void TransformSystemsToModules::HandleFeedback() 
{
  assert(procDef != NULL) ;

  list<VariableSymbol*> toReplace ;

  // This function needs to find all variables that are both input and 
  //  output scalars (they used to be feedbacks, but now there is no loop)

  SymbolTable* procSymTab = procDef->get_symbol_table() ;
  for (int i = 0 ; i < procSymTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* nextObject = procSymTab->get_symbol_table_object(i) ;
    if (nextObject->lookup_annote_by_name("InputScalar") != NULL &&
	nextObject->lookup_annote_by_name("OutputVariable") != NULL)
    {
      VariableSymbol* var = dynamic_cast<VariableSymbol*>(nextObject) ;
      assert(var != NULL) ;
      toReplace.push_back(var) ;
    }
  }

  // Now, go through all of the different values that need to be replaced and
  //  find all of the uses and definitions and replace them...

  list<VariableSymbol*>::iterator replaceIter = toReplace.begin() ;
  while (replaceIter != toReplace.end())
  {
    LString inputName = (*replaceIter)->get_name() ;
    inputName = inputName + LString("_in") ;

    LString outputName = (*replaceIter)->get_name() ;
    outputName = outputName + LString("_out") ;

    // Create two variables, one for input and one for output
    VariableSymbol* inputReplacement = 
      create_variable_symbol(theEnv,
			     (*replaceIter)->get_type(),
			     inputName) ;
    VariableSymbol* outputReplacement = 
      create_variable_symbol(theEnv,
			     (*replaceIter)->get_type(),
			     outputName) ;
    procDef->get_symbol_table()->append_symbol_table_object(inputReplacement) ;
    procDef->get_symbol_table()->append_symbol_table_object(outputReplacement);

    inputReplacement->append_annote(create_brick_annote(theEnv,
							"InputScalar")) ;
    outputReplacement->append_annote(create_brick_annote(theEnv,
							 "OutputVariable")) ;

    // Find all uses before the first definition and replace them with the
    //  input variable.
    ReplaceUses((*replaceIter), inputReplacement) ;

    // Find the last definition and replace it with the output variable.
    ReplaceDefinition((*replaceIter), outputReplacement)  ;

    // Remove the annotes that started the whole thing
    delete (*replaceIter)->remove_annote_by_name("InputScalar") ;
    delete (*replaceIter)->remove_annote_by_name("OutputVariable") ;

    ++replaceIter ;
  }

}

void TransformSystemsToModules::Transform()
{
  assert(procDef != NULL) ;

  // Collect all the input scalars and output scalars
  list<VariableSymbol*> ports ;
  
  SymbolTable* procSymTab = procDef->get_symbol_table() ;
  bool foundInputs = false ;
  bool foundOutputs = false ;
 
  for (int i = 0 ; i < procSymTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* nextObject = procSymTab->get_symbol_table_object(i) ;

    if (nextObject->lookup_annote_by_name("InputScalar") != NULL)
    {
      VariableSymbol* toConvert = 
	dynamic_cast<VariableSymbol*>(nextObject) ;
      assert(toConvert != NULL) ;
      LString inputName = toConvert->get_name() ;
      inputName = inputName + "_in" ;
      toConvert->set_name(inputName) ;
      ports.push_back(toConvert) ;
      foundInputs = true ;
    }
    if (nextObject->lookup_annote_by_name("OutputVariable") != NULL)
    {
      VariableSymbol* toConvert = 
	dynamic_cast<VariableSymbol*>(nextObject) ;
      assert(toConvert != NULL) ;
      LString outputName = toConvert->get_name() ;
      outputName = outputName + "_out" ;
      toConvert->set_name(outputName) ;
      ports.push_back(toConvert) ;
      foundOutputs = true ;
    }
  }
  assert(foundInputs && 
	 "Could not identify inputs.  Were they removed via optimizations?") ;
  assert(foundOutputs && 
	 "Could not identify outputs.  Were they removed via optimizations?") ;

  // Determine the bit size and add everything to a new symbol table
  int bitSize = 0 ;
  GroupSymbolTable* structTable = 
    create_group_symbol_table(theEnv,
			      procDef->get_symbol_table()) ;

  std::map<VariableSymbol*, FieldSymbol*> replacementFields ;

  bool portsRemoved = false ;
  // If this was actually a new style module, we should make sure to
  //  put these in the correct order.
  if (isModule(procDef))
  {
    // Go through the original symbol table and remove any parameter 
    //  symbols that originally existed
    SymbolTable* originalSymTab = procDef->get_symbol_table() ;
    Iter<SymbolTableObject*> originalIter = 
      originalSymTab->get_symbol_table_object_iterator() ;
    while (originalIter.is_valid())
    {
      SymbolTableObject* currentObj = originalIter.current() ;
      originalIter.next() ;
      if (dynamic_cast<ParameterSymbol*>(currentObj) != NULL)
      {
	originalSymTab->remove_symbol_table_object(currentObj) ;
      }
    }
    portsRemoved = true ;

    // Sort the variable symbols in parameter order.  This is just an 
    //  insertion sort, so it could be done faster.
    list<VariableSymbol*> sortedPorts ;
    for (int i = 0 ; i < ports.size() ; ++i)
    {
      list<VariableSymbol*>::iterator portIter = ports.begin() ;
      while (portIter != ports.end())
      {
	BrickAnnote* orderAnnote = 
	  dynamic_cast<BrickAnnote*>((*portIter)->
				     lookup_annote_by_name("ParameterOrder")) ;
	if (orderAnnote == NULL)
	{
	  ++portIter ;
	  continue ;
	}
	IntegerBrick* orderBrick = 
	  dynamic_cast<IntegerBrick*>(orderAnnote->get_brick(0)) ;
	assert(orderBrick != NULL) ;
	if (orderBrick->get_value().c_int() == i)
	{
	  sortedPorts.push_back(*portIter) ;
	  break ;
	}
	++portIter ;
      }
    }
    if (sortedPorts.size() != ports.size())
    {
      OutputWarning("Warning! Analysis detected some input scalars not in"
		    " the parameter list") ;
    }
    // Replace ports with sortedPorts
    ports = sortedPorts ;
  }

  list<VariableSymbol*>::iterator portIter = ports.begin() ;  
  while (portIter != ports.end()) 
  {
    bitSize += 
      (*portIter)->get_type()->get_base_type()->get_bit_size().c_int() ;

    LString dupeName = (*portIter)->get_name() ;

    // Create offset expression:
    IntConstant* offset = 
      create_int_constant(theEnv,
			  create_data_type(theEnv,
					   IInteger(32),
					   0),
			  IInteger(bitSize)) ;


    QualifiedType* dupeType = (*portIter)->get_type() ;
    // Deal with the case where reference types were passed in
    ReferenceType* refType = 
      dynamic_cast<ReferenceType*>(dupeType->get_base_type()) ;
    while (refType != NULL)
    {
      dupeType = dynamic_cast<QualifiedType*>(refType->get_reference_type()) ;
      assert(dupeType != NULL) ;
      refType = dynamic_cast<ReferenceType*>(dupeType->get_base_type()) ;
    }

    // Create a new variable symbol clone
    FieldSymbol* dupe = 
      create_field_symbol(theEnv,
			  dupeType,
			  offset,
			  dupeName) ;
        
    structTable->append_symbol_table_object(dupe) ;

    // Make the connection with the duplicated symbol
    replacementFields[(*portIter)] = dupe ;

    // Remove the original variable symbol from the procedure definition
    //  symbol table.
    if (!portsRemoved)
    {
      procDef->get_symbol_table()->remove_symbol_table_object(*portIter) ;
    }
    
    ++portIter ;
  }
  assert(bitSize != 0);

  StructType* moduleStruct = 
    create_struct_type(theEnv,
		       IInteger(bitSize),
		       0, // bit_alignment
		       TempName(procDef->get_procedure_symbol()->get_name()),
		       0, // is_complete
		       structTable) ;

  
  Iter<FileBlock*> fBlocks = 
    theEnv->get_file_set_block()->get_file_block_iterator() ;
  
  assert(fBlocks.is_valid()) ;
  (fBlocks.current())->get_symbol_table()->append_symbol_table_object(moduleStruct) ;
  
  // This is commented out because it is in the file state block
  //procDef->get_symbol_table()->append_symbol_table_object(moduleStruct) ;

  QualifiedType* qualifiedModuleStruct =
    create_qualified_type(theEnv,
			  moduleStruct,
			  TempName(LString("qualifiedModuleStruct"))) ;
  
  procDef->get_symbol_table()->append_symbol_table_object(qualifiedModuleStruct) ;

  // Create an instance of this type and add it to the symbol table.
  ParameterSymbol* structInstance = 
    create_parameter_symbol(theEnv,
			   qualifiedModuleStruct,
			   TempName(LString("structInstance"))) ;

  procDef->get_symbol_table()->append_symbol_table_object(structInstance) ;

  // Now, set up the procedure symbol to take the struct and return the 
  //  struct.
  assert(procDef != NULL) ;
  ProcedureSymbol* procSym = procDef->get_procedure_symbol() ;
  assert(procSym != NULL) ;
  ProcedureType* procType = procSym->get_type() ;
  assert(procType != NULL) ;
  CProcedureType* cProcType = dynamic_cast<CProcedureType*>(procType) ;
  assert(cProcType != NULL) ;

  // Instead of appending the struct argument, we need to replace all of the 
  //  arguments with the struct.

  while (cProcType->get_argument_count() > 0)
  {
    cProcType->remove_argument(0) ;
  }

  cProcType->set_result_type(moduleStruct) ;
  cProcType->append_argument(qualifiedModuleStruct) ;

  // Now go through all load variable expressions and replace them all with
  //  field symbol values if appropriate
  
  list<LoadVariableExpression*>* allLoads = 
    collect_objects<LoadVariableExpression>(procDef->get_body()) ;

  list<LoadVariableExpression*>::iterator loadIter = allLoads->begin() ;
  while (loadIter != allLoads->end())
  {
    VariableSymbol* currentVariable = (*loadIter)->get_source() ;
    if (replacementFields.find(currentVariable) != replacementFields.end())
    {
      (*loadIter)->set_source(replacementFields[currentVariable]) ;
    }
    ++loadIter ;
  }
  delete allLoads ;

  // Also replace all of the definitions with the field symbol
  list<StoreVariableStatement*>* allStoreVars = 
    collect_objects<StoreVariableStatement>(procDef->get_body()) ;
  list<StoreVariableStatement*>::iterator storeVarIter = allStoreVars->begin();
  while (storeVarIter != allStoreVars->end())
  {
    VariableSymbol* currentDest = (*storeVarIter)->get_destination() ;
    if (replacementFields.find(currentDest) != replacementFields.end())
    {
      (*storeVarIter)->set_destination(replacementFields[currentDest]) ;
    }
    ++storeVarIter ;
  }
  delete allStoreVars ;

  list<SymbolAddressExpression*>* allSymAddr = 
    collect_objects<SymbolAddressExpression>(procDef->get_body()) ;
  list<SymbolAddressExpression*>::iterator symAddrIter = allSymAddr->begin() ;
  while (symAddrIter != allSymAddr->end())
  {
    VariableSymbol* currentVar = 
      dynamic_cast<VariableSymbol*>((*symAddrIter)->get_addressed_symbol()) ;
    if (currentVar != NULL &&
	replacementFields.find(currentVar) != replacementFields.end())
    {
      (*symAddrIter)->set_addressed_symbol(replacementFields[currentVar]) ;
    }
    ++symAddrIter ;
  }
  delete allSymAddr ;
  // One final for bool selects
  list<CallStatement*>* allCalls = 
    collect_objects<CallStatement>(procDef->get_body()) ;
  list<CallStatement*>::iterator callIter = allCalls->begin() ;
  while(callIter != allCalls->end())
  {
    VariableSymbol* currentVar = (*callIter)->get_destination() ;
    if (currentVar != NULL &&
	replacementFields.find(currentVar) != replacementFields.end())
    {
      (*callIter)->set_destination(replacementFields[currentVar]) ;
    }
    ++callIter ;
  }
  delete allCalls ;
}

void TransformSystemsToModules::ReplaceUses(VariableSymbol* original,
					    VariableSymbol* replacement)
{
  assert(procDef != NULL) ;

  StatementList* bodyList = dynamic_cast<StatementList*>(procDef->get_body()) ;
  assert(bodyList != NULL) ;

  for (int i = 0 ; i < bodyList->get_statement_count() ; ++i)
  {
    list<LoadVariableExpression*>* allLoads = 
      collect_objects<LoadVariableExpression>(bodyList->get_statement(i)) ;

    list<LoadVariableExpression*>::iterator loadIter = allLoads->begin() ;
    while (loadIter != allLoads->end())
    {
      if ((*loadIter)->get_source() == original)
      {
	(*loadIter)->set_source(replacement) ;
      }
      ++loadIter ;
    }

    delete allLoads ;

    if (IsDefinition(bodyList->get_statement(i), original))
    {
      // We no longer have to replace the value
      return ;
    }
  }
}

void TransformSystemsToModules::ReplaceDefinition(VariableSymbol* original,
						  VariableSymbol* replacement)
{
  assert(procDef != NULL) ;

  std::cout << "Replacing a definition: " << original << " " << replacement
	    << std::endl ;

  StatementList* bodyList = dynamic_cast<StatementList*>(procDef->get_body()) ;
  assert(bodyList != NULL) ;

  // From the back, find the last definition
  for (int i = bodyList->get_statement_count() - 1 ; i > 0 ; --i)
  {
    Statement* currentStatement = bodyList->get_statement(i) ;
    if (IsDefinition(currentStatement, original))
    {
      
      // This is either going to be a store variable statement or
      //  a symbol address expression (if the definition was in a call)

      list<StoreVariableStatement*>* allStores = 
	collect_objects<StoreVariableStatement>(currentStatement) ;

      list<StoreVariableStatement*>::iterator storeIter = allStores->begin() ;
      while (storeIter != allStores->end())
      {
	if ((*storeIter)->get_destination() == original)
	{
	  (*storeIter)->set_destination(replacement) ;
	}
	++storeIter ;
      }
      delete allStores ;

      list<SymbolAddressExpression*>* allSymAddr = 
	collect_objects<SymbolAddressExpression>(currentStatement) ;
      
      list<SymbolAddressExpression*>::iterator symAddrIter = 
	allSymAddr->begin();
      while (symAddrIter != allSymAddr->end())
      {
	if ((*symAddrIter)->get_addressed_symbol() == original)
	{
	  (*symAddrIter)->set_addressed_symbol(replacement) ;
	}
	++symAddrIter ;
      }
      delete allSymAddr ;

      return ;
    }
  }
  
}

