// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/*

  This file contains code that will read in a repository of structs and
   functions, add the current module to that repository, and then write out
   only the repository in suif form.

  Additionally, the functions located in the repository will be output 
   to a ".h" file accessible to users.

*/

#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <list>

#include <string>
#include <cfenodes/cfe.h>
#include <iokernel/object_factory.h>
#include <suifnodes/suif_factory.h>
#include <basicnodes/basic_factory.h>
#include <suifkernel/command_line_parsing.h>

#include "roccc_extra_types/array_info.h"
#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

#include "libOutputPass.h"

const char* ROCCC_LIBRARY_FILE = "/roccc-library.h" ;

LibraryOutputPass::LibraryOutputPass(SuifEnv* pEnv) : 
  PipelinablePass(pEnv, "LibraryOutputPass")
{
  theEnv = pEnv ;

  // Give a default for remove if we somehow get here through
  //  past the defenses of the suif side.
  remove = 0 ; 

  currentProcedure = NULL ;
  repository = NULL ;

  // These are used when we remove a module.
  procedureToRemove = NULL ;
  structToRemove = NULL ;

  // These are used when we are adding a module
  procedureToAdd = NULL ;
  structToAdd = NULL ;
}

LibraryOutputPass::~LibraryOutputPass()
{
  hout.close() ;
}


void LibraryOutputPass::initialize()
{
  PipelinablePass::initialize() ;
  _command_line->set_description("Manages the repository of modules") ;
  OptionInt* removeInt = new OptionInt("Remove or Add", &remove) ;
  OptionString* name = new OptionString("Module Name", &moduleName) ;
  OptionString* directory = new OptionString("Local Dir", &localDirectory) ;
  OptionList* args = new OptionList() ;
  args->add(removeInt) ;
  args->add(name) ;
  args->add(directory) ;
  _command_line->add(args) ;
}

// This is done in place of do_procedure_definition, because we have no
//  attachment to any particular procedure definition
void LibraryOutputPass::execute()
{ 
  Iter<FileBlock*> temp = get_suif_env()->get_file_set_block()->get_file_block_iterator() ;

  Iter<ProcedureDefinition*> temp2 = (temp.current())->get_definition_block()->get_procedure_definition_iterator() ;

  currentProcedure = temp2.current() ;
  assert(currentProcedure != NULL) ;

  OutputInformation("Library Output pass begins") ;

  if (remove == 0)
  {
    // Standard export.  Check to see if we are a system or a module
    Annote* rocccAnnote = 
      currentProcedure->lookup_annote_by_name("FunctionType") ;
    BrickAnnote* rocccType = 
      dynamic_cast<BrickAnnote*>(rocccAnnote) ;
    assert(rocccType != NULL) ;
    
    IntegerBrick* valueBrick = 
      dynamic_cast<IntegerBrick*>(rocccType->get_brick(0)) ;
    assert(valueBrick != NULL) ;

    int functionType = valueBrick->get_value().c_int() ;
    if (functionType == 3)
    {
      delete currentProcedure->remove_annote_by_name("FunctionType") ;
      OutputInformation("Exporting a composable system") ;
      ExportSystem() ;
      OutputHeader() ;
      OutputInformation("Done exporting a composable system") ;

      std::string fullPath = localDirectory.c_str() ;
      assert(fullPath != "") ;
      fullPath += "/repository.suif" ;      
      get_suif_env()->write(fullPath.c_str()) ;
      return ;   
    }
    else if (functionType == 1)
    {
      UpdateRepository() ;
      OutputHeader() ;
    }
    else
    {
      // Nothing to do with normal systems
      OutputInformation("Library Output Pass ends") ;
      return ;
    }
  }
  else if (remove == 1)
  {
    if (!RemoveModule(moduleName))
    {
      return ;
    }
    
    OutputHeader() ;
  }
  else if (remove == 2)
  {
    if (!AddModule(moduleName))
    {
      return ;
    }
    OutputHeader() ;
  }

  OutputInformation("Header output") ;

  std::string fullPath = localDirectory.c_str() ;
  assert(fullPath != "") ;
  fullPath += "/repository.suif" ;      

  // Write the suif file we have.
  get_suif_env()->write(fullPath.c_str()) ;
  
}

/*

  This function is responsible for finding all the structs and the current
   procedure symbol in the compiling module code and adding it to the 
   repository.  This involves a very deep clone and detach procedure since 
   all the information associated with each SymbolTableObject will be
   wiped out when I read in the repository.

*/
void LibraryOutputPass::UpdateRepository()
{

  // Right now all I'm doing is trying to add the current function to the
  //  repository.

  ProcedureSymbol* currentSymbol = currentProcedure->get_procedure_symbol() ;
  // I don't need a definition, just the prototype
  currentSymbol->set_definition(NULL) ;

  Type* currentType = currentSymbol->get_type() ;

  CProcedureType* currentCType = dynamic_cast<CProcedureType*>(currentType) ;
  assert(currentCType != NULL) ;
  currentCType->get_symbol_table()->remove_symbol_table_object(currentCType) ;
  currentCType->set_parent(NULL) ;

  DataType* returnType = currentCType->get_result_type() ;
  StructType* returnStruct = dynamic_cast<StructType*>(returnType) ;
  assert(returnStruct != NULL) ;

  StructType* replacement = create_struct_type(get_suif_env(),
					       returnStruct->get_bit_size(),
					       returnStruct->get_bit_alignment(), 
					       returnStruct->get_name()) ;

  // Go through the symbol table in the return struct 
  //  and add each element to the replacement

  list<Type*> allTypesToAdd ;

  for(int i = 0 ; 
      i < returnStruct->get_group_symbol_table()->get_symbol_table_object_count();
      ++i)
  {

    SymbolTableObject* nextObject = returnStruct->get_group_symbol_table()->
      get_symbol_table_object(i) ;
    FieldSymbol* nextField = dynamic_cast<FieldSymbol*>(nextObject) ;
    assert(nextField != NULL) ;
    
    if (dynamic_cast<IntegerType*>(nextField->get_type()->get_base_type()) != NULL)
    {
      IntegerType* nextInt = dynamic_cast<IntegerType*>(nextField->
							get_type()->
							get_base_type()) ;

      DataType* baseTypeToAdd=create_integer_type(get_suif_env(),
			       	 	          nextInt->get_bit_size(),
			       		          nextInt->get_bit_alignment(),
			       		          nextInt->get_is_signed()) ;

      QualifiedType* qualTypeToAdd = create_qualified_type(get_suif_env(),
							   baseTypeToAdd) ;

      FieldSymbol* fieldToAdd = create_field_symbol(get_suif_env(),
						    qualTypeToAdd,
						    NULL,
						    nextField->get_name()) ;

      replacement->get_group_symbol_table()->append_symbol_table_object(fieldToAdd) ;

      allTypesToAdd.push_back(baseTypeToAdd) ;
      allTypesToAdd.push_back(qualTypeToAdd) ;

    }
    else if (dynamic_cast<FloatingPointType*>(nextField->get_type()->get_base_type()) != NULL)
    {
      FloatingPointType* nextFloat = dynamic_cast<FloatingPointType*>(nextField->
							get_type()->
							get_base_type()) ;

      DataType* baseTypeToAdd=create_floating_point_type(get_suif_env(),
			       	 	          nextFloat->get_bit_size(),
			       		          nextFloat->get_bit_alignment()
			       		          ) ;

      QualifiedType* qualTypeToAdd = create_qualified_type(get_suif_env(),
							   baseTypeToAdd) ;

      FieldSymbol* fieldToAdd = create_field_symbol(get_suif_env(),
						    qualTypeToAdd,
						    NULL,
						    nextField->get_name()) ;

      replacement->get_group_symbol_table()->append_symbol_table_object(fieldToAdd) ;

      allTypesToAdd.push_back(baseTypeToAdd) ;
      allTypesToAdd.push_back(qualTypeToAdd) ;
    }
    else
    {
      assert(nextField->get_type()->get_base_type() != NULL) ;
      FormattedText tmpText ;
      nextField->get_type()->get_base_type()->print(tmpText) ;
      std::cout << tmpText.get_value() << std::endl ;
      
      
      assert(0 && "Not a known type") ;
    }

  }


  currentCType->set_result_type(replacement) ;

  QualifiedType* qualReplacement = create_qualified_type(get_suif_env(),
							 replacement) ;

  currentCType->replace_argument(0, qualReplacement) ;


  // O.k, that should have actually gotten both the prototype AND the struct!

  // Detach the currentSymbol

  currentSymbol->get_symbol_table()->remove_symbol_table_object(currentSymbol);
  currentSymbol->set_parent(NULL) ;

  currentSymbol->set_type(currentCType) ;

  // Read in the new repository
  // Add the procedure symbol to the symbol table
  std::string fullPath = localDirectory.c_str() ;
  assert(fullPath != "") ;
  fullPath += "/repository.suif" ;      

  OutputInformation("Reading the repository") ;
  get_suif_env()->read(fullPath.c_str()) ;
  OutputInformation("Repository has been read") ;
  repository = get_suif_env()->get_file_set_block() ;

  // Look to see if a procedure symbol with the same name already 
  //  exists.  If it does, remove it.  Unfortunately, there is no
  //  way to lookup a symbol by name, so I'm going to have
  //  to iterate through all of them and check to see if any of them 
  //  match

  list<SymbolTableObject*> removeList ;

  Iter<SymbolTableObject*> findName = repository->get_external_symbol_table()->
    get_symbol_table_object_iterator() ;

  while(findName.is_valid())
  {
    
    if (findName.current()->get_name() == LString("unknown"))
    {
      // Remove the symbol, the type, and the definition
      removeList.push_back(findName.current()) ;
      assert(dynamic_cast<ProcedureSymbol*>(findName.current()) != NULL) ;
      removeList.push_back(dynamic_cast<ProcedureSymbol*>(findName.current())->get_type()) ;


      dynamic_cast<ProcedureSymbol*>(findName.current())->get_definition()->set_symbol_table(NULL) ;  
      dynamic_cast<ProcedureSymbol*>(findName.current())->get_definition()->set_definition_block(NULL) ;
      dynamic_cast<ProcedureSymbol*>(findName.current())->get_definition()->set_body(NULL) ;
      dynamic_cast<ProcedureSymbol*>(findName.current())->get_definition()->set_procedure_symbol(NULL) ;
      //      delete dynamic_cast<ProcedureSymbol*>(findName.current())->get_definition() ;
      dynamic_cast<ProcedureSymbol*>(findName.current())->set_definition(NULL) ;

    }
    
    if (findName.current()->get_name() == currentSymbol->get_name())
    {
      // Remove the symbol and the type associated with it
      removeList.push_back(findName.current()) ;
      assert(dynamic_cast<ProcedureSymbol*>(findName.current()) != NULL) ;     
      removeList.push_back(dynamic_cast<ProcedureSymbol*>(findName.current())->get_type()) ;
      
      // Also, since the return type of the procedure happens to be a struct...
      //  remove all of the types associated with that struct!!
      CProcedureType* internalType = 
	dynamic_cast<CProcedureType*>(dynamic_cast<ProcedureSymbol*>(findName.current())->get_type()) ;

      assert(internalType != NULL) ;

      StructType* returnStruct = dynamic_cast<StructType*>(internalType->get_result_type()) ;
      assert(returnStruct != NULL) ;

      Iter<SymbolTableObject*> elementIter = returnStruct->
	get_group_symbol_table()->get_symbol_table_object_iterator() ;
      while (elementIter.is_valid())
      {
	VariableSymbol* v = dynamic_cast<VariableSymbol*>(elementIter.current()) ;
	if (v != NULL)
	{
	  removeList.push_back(v->get_type()) ;
	}
	elementIter.next() ;
      }

      
    }

    if (findName.current()->get_name() == replacement->get_name())
    {
      // Remove both the struct type and all of the internal types
      //  associated with the struct type
      removeList.push_back(findName.current()) ;
      
      StructType* internal = dynamic_cast<StructType*>(findName.current()) ;
      Iter<SymbolTableObject*> elementIter = internal->
	get_group_symbol_table()->get_symbol_table_object_iterator() ;

      while(elementIter.is_valid())
      {
	// Get the type of each individual element
	VariableSymbol* v = dynamic_cast<VariableSymbol*>(elementIter.current()) ;
	if (v != NULL)
	{
	  if (dynamic_cast<QualifiedType*>(v->get_type()) != NULL)
	  {
	    DataType* another = 
	      dynamic_cast<QualifiedType*>(v->get_type())->get_base_type() ;
	    removeList.push_back(another) ;

	  }
	}
	elementIter.next() ;
      }
      
    }

    findName.next() ;
  }

  // Actually remove all of the items marked for deletion
  for(list<SymbolTableObject*>::iterator removeIter = removeList.begin() ;
      removeIter != removeList.end() ;
      ++removeIter)
  {
    repository->get_external_symbol_table()->remove_symbol_table_object(*removeIter) ;
  }

  repository->get_external_symbol_table()->
    append_symbol_table_object(currentSymbol) ;
  repository->get_external_symbol_table()->
    append_symbol_table_object(currentCType) ;
  repository->get_external_symbol_table()->
    append_symbol_table_object(replacement) ;

  for(list<Type*>::iterator addThem = allTypesToAdd.begin() ;
      addThem != allTypesToAdd.end() ;
      ++addThem)
  {
    repository->get_external_symbol_table()->
      append_symbol_table_object(*addThem) ;
  }

}

void LibraryOutputPass::OutputHeader()
{

  std::string fullPath = localDirectory.c_str() ;
  assert(fullPath != "") ;
  fullPath += ROCCC_LIBRARY_FILE ;

  hout.open(fullPath.c_str()) ;
  if (!hout)
  {
    std::cerr << "Cannot open " << ROCCC_LIBRARY_FILE << "!" << std::endl ;
    assert(0) ;
  }

  hout << "/*" << std::endl ;
  hout << " This file contains all of the exported functions and structs" 
       << std::endl ;
  hout << "  that can be called as hardware modules.  This file is" << std::endl ;
  hout << "  automatically generated and updated after every successful" 
       << std::endl ;
  hout << "  compilation of a module" << std::endl ;
  hout << "*/" << std::endl << std::endl ;

  hout << "#ifndef __ROCCC_LIBRARY_DOT_H__" << std::endl ;
  hout << "#define __ROCCC_LIBRARY_DOT_H__" << std::endl << std::endl ;

  OutputTypes() ;

  OutputStructs() ;

  hout << std::endl ;
  hout << "#ifdef __cplusplus" << std::endl ;

  OutputPrototypes() ;

  hout << "#endif" << std::endl ;
  hout << std::endl ;

  hout << "#endif" << std::endl ;  
}

void LibraryOutputPass::OutputStructs()
{
  assert(repository != NULL) ;
  SymbolTable* findStructs = repository->get_external_symbol_table();

  for (int i = 0 ; i < findStructs->get_symbol_table_object_count(); ++i)
  {
    SymbolTableObject* nextObj = findStructs->get_symbol_table_object(i) ;
    StructType* nextStruct = dynamic_cast<StructType*>(nextObj) ;
    if (nextStruct != NULL)
    {
      hout << "typedef struct" << std::endl ;
      hout << "{" << std::endl ;
      SymbolTable* internalTable = nextStruct->get_group_symbol_table() ;
      assert(internalTable != NULL) ;
      for(int j = 0 ; j < internalTable->get_symbol_table_object_count(); ++j)
      {
	SymbolTableObject* internalObj = internalTable->
	  get_symbol_table_object(j) ;
	PrintDeclaration(internalObj) ;
      }
      hout << "}" << nextStruct->get_name() << " ; " << std::endl ;
    }
  }
}

// This function is responsible for outputting all of the typedefs for 
//  integers and floats that are different sizes.
void LibraryOutputPass::OutputTypes()
{
  assert(repository != NULL) ;
  SymbolTable* symTab = repository->get_external_symbol_table() ;
  
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* nextObj = symTab->get_symbol_table_object(i) ;
    DataType* nextType = dynamic_cast<DataType*>(nextObj) ;
    if (nextType != NULL && !AlreadyPrinted(nextType))
    {
      OutputTypeDeclaration(nextType) ;
    }
  }
}

bool LibraryOutputPass::AlreadyPrinted(DataType* t)
{
  bool isFloat = (dynamic_cast<FloatingPointType*>(t) != NULL) ;
  int bitSize = t->get_bit_size().c_int() ;

  std::list<LibraryType>::iterator printIter = printedTypes.begin() ;
  while (printIter != printedTypes.end())
  {
    LibraryType current = *printIter ;
    if (current.isFloat == isFloat && current.bitSize == bitSize)
    {
      return true ;
    }
    ++printIter ;
  }
  return false ;
}

void LibraryOutputPass::OutputTypeDeclaration(DataType* t)
{
  IntegerType* intType = dynamic_cast<IntegerType*>(t) ;
  FloatingPointType* floatType = dynamic_cast<FloatingPointType*>(t) ;
  LibraryType toAdd ;

  if (intType != NULL)
  {
    hout << "typedef int ROCCC_int" ;
    toAdd.isFloat = false ;
  }
  else if (floatType != NULL)
  {
    hout << "typedef float ROCCC_float" ;
    toAdd.isFloat = true ;
  }
  else
  {
    // OutputWarning("Unknown type detected.  Possibly void?") ;
    return ;
  }

  hout << t->get_bit_size().c_int() << " ;" << std::endl ;
  toAdd.bitSize = t->get_bit_size().c_int()  ;
  printedTypes.push_back(toAdd) ;
}

void LibraryOutputPass::OutputPrototypes()
{
  SymbolTable* findFunctions = repository->get_external_symbol_table() ;

  for(int i = 0 ; i < findFunctions->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* nextObj = findFunctions->get_symbol_table_object(i) ;
    ProcedureSymbol* printOut = dynamic_cast<ProcedureSymbol*>(nextObj) ;
    if (printOut != NULL)
    { 
      Type* currentType = printOut->get_type() ;
      assert(currentType != NULL) ;
      CProcedureType* procType = dynamic_cast<CProcedureType*>(currentType) ;
      assert(procType != NULL) ;

      OutputPrototypeOne(printOut, procType) ;
      OutputPrototypeTwo(printOut, procType) ;
    }
    
  }
}

void LibraryOutputPass::OutputPrototypeOne(ProcedureSymbol* procSym, 
					   CProcedureType* procType)
{
  assert(procSym != NULL) ;
  assert(procType != NULL) ;
  assert(procType->get_argument_count() > 0) ;

  hout << StringType(procType->get_result_type())  << " " ;
  hout << procSym->get_name() << "(" ;
  for(unsigned int j = 0; j < procType->get_argument_count() ; ++j)
  {
    if (j != 0)
    {
      hout << ", " ;
    }
    hout << StringType(procType->get_argument(j)) ;
    if (procType->get_argument(j)->lookup_annote_by_name("Output") != NULL ||
        strstr(procType->get_argument(j)->get_name().c_str(), "_out") != NULL)
    {
      hout << "&" ;
    }
  }
  hout << ");" << std::endl ;
}

void LibraryOutputPass::OutputPrototypeTwo(ProcedureSymbol* procSym,
					   CProcedureType* procType)
{
  assert(procSym != NULL) ;
  assert(procType != NULL) ;
  assert(procType->get_argument_count() > 0) ;

  DataType* argType = procType->get_argument(0)->get_base_type() ;
  assert(argType != NULL) ;
  StructType* realArgType = dynamic_cast<StructType*>(argType) ;
  if (realArgType == NULL)
  {
    // This is a exportable system, so don't do anything
    return ;
  }
  assert(realArgType != NULL) ;

  hout << "void " << procSym->get_name() << "(" ;

  GroupSymbolTable* symTab = realArgType->get_group_symbol_table() ;
  for(int j = 0 ; j < symTab->get_symbol_table_object_count() ;
      ++j)
  {
    Symbol* sym = dynamic_cast<Symbol*>(symTab->get_symbol_table_object(j));
    assert(sym != NULL) ;
    if (j != 0)
    {
      hout << ", " ;
    }
    hout << StringType(sym->get_type()) ;
    if (strstr(sym->get_name().c_str(), "_out") != NULL)
    {
      hout << "&" ;
    }    
  }
  
  hout << ");" << std::endl ;
}

String LibraryOutputPass::StringType(Type* t)
{
  String toReturn = "" ;
  assert(t != NULL) ;
  assert(dynamic_cast<Type*>(t) != NULL) ;

  if (dynamic_cast<ArrayType*>(t) != NULL)
  {
    toReturn += StringType(dynamic_cast<ArrayType*>(t)->get_element_type()) ;
  }
  else if (dynamic_cast<MultiDimArrayType*>(t) != NULL)
  {
    toReturn += StringType(dynamic_cast<MultiDimArrayType*>(t)->
			   get_element_type()) ;
  }
  else if (dynamic_cast<DataType*>(t) != NULL)
  {
    if (dynamic_cast<PointerType*>(t) != NULL)
    {
      toReturn += 
	StringType(dynamic_cast<PointerType*>(t)->get_reference_type()) ;
      toReturn += "*" ;
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

void LibraryOutputPass::PrintDeclaration(SymbolTableObject* s)
{
  Symbol* printMe = dynamic_cast<Symbol*>(s) ;
  if (printMe == NULL)
    return ;

  // FIXME: Print out arrays
  //if (dynamic_cast<ArrayType*>(dType) != NULL)
  //  {
  //    return ;
  //  }

  hout << StringType(printMe->get_type()) ;
  hout << " " << printMe->get_name() << " ; "  << std::endl ;

}

void LibraryOutputPass::CollectSymbolsToRemove(String name)
{
  assert(repository != NULL) ;
  SymbolTable* symTab = repository->get_external_symbol_table() ;

  for(int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i) 
  {
    SymbolTableObject* nextObj = symTab->get_symbol_table_object(i) ;

    // We need to find the function with the same name as the module
    //  we are trying to remove.  That will also lead us to the 
    //  struct that we are removing.

    ProcedureSymbol* nextProc = dynamic_cast<ProcedureSymbol*>(nextObj) ;
    if (nextProc != NULL) 
    {
      if (nextProc->get_name() == name)
      {
	// We found it!  Grab both the procedure symbol and the 
	//  struct type.
	procedureToRemove = nextProc ;
	CProcedureType* procType = 
	  dynamic_cast<CProcedureType*>(nextProc->get_type()) ;
	assert(procType != NULL) ;
	StructType* passedStruct = 
	  dynamic_cast<StructType*>(procType->get_result_type()) ;
	assert(passedStruct != NULL) ;
	structToRemove = passedStruct ;
	return ;
      }
    }
  }
}

// This function is responsible for finding and removing all information 
//  regarding the module named "name" from the repository
bool LibraryOutputPass::RemoveModule(String name)
{
  // Initialize the repository and read in what was stored
  InitializeRepository() ;
  assert(repository != NULL) ;

  // We are going to collect two symbols, a struct type and a procedure
  //  type.
  CollectSymbolsToRemove(name) ;
  
  if (procedureToRemove == NULL || structToRemove == NULL)
  {
    // We didn't find the module to remove.  This could have been a module
    //  that wasn't added through ROCCC.  Just return.
    OutputInformation("Module to remove was not found in roccc-library.h") ;
    return false ;
  }

  // Now, actually remove the components that we need to.
  RemoveProcedure() ;
  RemoveStruct() ;

  return true ;
}

void LibraryOutputPass::RemoveProcedure()
{
  assert(procedureToRemove != NULL) ;
  assert(repository != NULL) ;
  SymbolTable* symTab = repository->get_external_symbol_table() ;

  // Clear out the definition
  ProcedureDefinition* defToRemove = procedureToRemove->get_definition() ;
  procedureToRemove->set_definition(NULL) ;
  if (defToRemove != NULL)
  {
    delete defToRemove ;
  }

  // Clear out all values associated with the procedure type
  CProcedureType* procTypeToRemove = 
    dynamic_cast<CProcedureType*>(procedureToRemove->get_type()) ;
  assert(procTypeToRemove != NULL) ;

  DataType* returnTypeToRemove = procTypeToRemove->get_result_type() ;
  procTypeToRemove->set_result_type(NULL) ;
  if (returnTypeToRemove != NULL && returnTypeToRemove != structToRemove)
  {
    delete returnTypeToRemove ;
  }

  // ...this includes all of the arguments and their types
  while (procTypeToRemove->get_argument_count() > 0)
  {
    QualifiedType* argToRemove = procTypeToRemove->get_argument(0) ;
    procTypeToRemove->remove_argument(0) ;

    DataType* baseToRemove = argToRemove->get_base_type() ;
    argToRemove->set_base_type(NULL) ;
    if (baseToRemove != structToRemove)
    {
      delete baseToRemove ;
    }

    delete argToRemove ;
  }

  procedureToRemove->set_type(NULL) ;
  symTab->remove_symbol_table_object(procTypeToRemove) ;
  delete procTypeToRemove ;
  
  symTab->remove_symbol_table_object(procedureToRemove) ;
  delete procedureToRemove ;
}

void LibraryOutputPass::RemoveStruct()
{
  assert(repository != NULL) ;
  assert(structToRemove != NULL) ;
  SymbolTable* symTab = repository->get_external_symbol_table() ;

  // Clean up the internal symbol table of the struct we are removing
  SymbolTable* internalSymTab = structToRemove->get_group_symbol_table() ;
  assert(internalSymTab != NULL) ;

  while (internalSymTab->get_symbol_table_object_count() > 0)
  {
    SymbolTableObject* nextInternalItem = 
      internalSymTab->remove_symbol_table_object(0) ;
    // This might have a type that needs to be removed from the symbol
    //  table
    FieldSymbol* nextFieldSymbol = 
      dynamic_cast<FieldSymbol*>(nextInternalItem) ;
    assert(nextFieldSymbol != NULL) ;
    QualifiedType* nextFieldQual = nextFieldSymbol->get_type() ;
    DataType* nextFieldType = nextFieldQual->get_base_type() ;
    
    nextFieldQual->set_base_type(NULL) ;
    nextFieldSymbol->set_type(NULL) ;
    symTab->remove_symbol_table_object(nextFieldType) ;
    symTab->remove_symbol_table_object(nextFieldQual) ;
    
    delete nextFieldType ;
    delete nextFieldQual ;

    delete nextInternalItem ;
  }

  structToRemove->set_group_symbol_table(NULL) ;
  internalSymTab->set_parent(NULL) ;
  delete internalSymTab ;
  
  symTab->remove_symbol_table_object(structToRemove) ;
  delete structToRemove ;
}

// This pass is responsible for setting up the repository links internal
//  to this class with the correct values according to the repository
//  suif file.
void LibraryOutputPass::InitializeRepository()
{
  std::string fullPath = localDirectory.c_str() ;
  assert(fullPath != "") ;
  fullPath += "/repository.suif" ;

  get_suif_env()->read(fullPath.c_str()) ;
  repository = get_suif_env()->get_file_set_block() ;

  // Remove any extraneous functions that might exist in the repository
  CleanRepository() ;

  // Initialize for removal
  procedureToRemove = NULL ;
  structToRemove = NULL ;

  // Initialize for addition
  procedureToAdd = NULL ;
  structToAdd = NULL ;
}

void LibraryOutputPass::CleanRepository()
{
  assert(repository != NULL) ;
  list<SymbolTableObject*> removeList ;
  Iter<SymbolTableObject*> findName = repository->get_external_symbol_table()->
    get_symbol_table_object_iterator() ;
  while(findName.is_valid())
  {    
    if (findName.current()->get_name() == LString("unknown"))
    {
      // Remove the symbol, the type, and the definition
      removeList.push_back(findName.current()) ;
      assert(dynamic_cast<ProcedureSymbol*>(findName.current()) != NULL) ;
      removeList.push_back(dynamic_cast<ProcedureSymbol*>(findName.current())->get_type()) ;


      dynamic_cast<ProcedureSymbol*>(findName.current())->get_definition()->set_symbol_table(NULL) ;  
      dynamic_cast<ProcedureSymbol*>(findName.current())->get_definition()->set_definition_block(NULL) ;
      dynamic_cast<ProcedureSymbol*>(findName.current())->get_definition()->set_body(NULL) ;
      dynamic_cast<ProcedureSymbol*>(findName.current())->get_definition()->set_procedure_symbol(NULL) ;
      //      delete dynamic_cast<ProcedureSymbol*>(findName.current())->get_definition() ;
      dynamic_cast<ProcedureSymbol*>(findName.current())->set_definition(NULL) ;

    }
    findName.next() ;
  }

  for(list<SymbolTableObject*>::iterator removeIter = removeList.begin() ;
      removeIter != removeList.end() ;
      ++removeIter)
  {
    repository->get_external_symbol_table()->remove_symbol_table_object(*removeIter) ;
  }

}

// This function is responsible for adding a module to the repository.
//  This is called when we have not compiled a file but instead just 
//  imported a component in the GUI
bool LibraryOutputPass::AddModule(String filename) 
{  
  InitializeRepository() ;
  assert(repository != NULL) ;
 
  ModuleEntry toAdd = ProcessFile(filename) ;
  
  //  AddTypes(toAdd) ;
  AddStruct(toAdd) ;
  AddProcedure(toAdd) ;

  return true ;
}

ModuleEntry LibraryOutputPass::ProcessFile(String filename)
{
  ModuleEntry constructed ;

  std::ifstream fin(filename.c_str()) ;
  if (!fin)
  {
    OutputError("Could not open the file with the module information!") ;
    assert(0) ;
  }

  fin >> std::ws >> constructed.name ;
  fin >> std::ws >> constructed.delay ;
  fin >> std::ws ;

  while (!fin.eof())
  {
    Port nextPort ;
    std::string inputValue ;
    std::string intValue ;
    fin >> std::ws >> nextPort.name >> std::ws ;
    fin >> inputValue >> std::ws ;
    if (inputValue == "IN")
    {
      nextPort.isInput = true ;
      nextPort.name += "_in" ;
    }
    else if (inputValue == "OUT")
    {
      nextPort.isInput = false ;
      nextPort.name += "_out" ;
    }
    else
    {
      assert(0 && "Incorrectly formatted component file!") ;
    }
    fin >> nextPort.size >> std::ws ;    
    fin >> intValue >> std::ws ;
    if (intValue == "int")
    {
      nextPort.isFloat = false ;
    }
    else
    {
      nextPort.isFloat = true ;
    }
    constructed.allPorts.push_back(nextPort) ;
  }
  fin.close() ;
  
  return constructed ;
}

void LibraryOutputPass::AddStruct(ModuleEntry toAdd)
{
  assert(theEnv != NULL) ;
  assert(repository != NULL) ;
  SymbolTable* symTab = repository->get_external_symbol_table() ;

  // Determine the name of the struct
  LString structName = toAdd.name.c_str() ;
  structName = structName + "_t" ;

  // Determine the bit size of the struct
  int totalBitSize = 0 ;
  std::list<Port>::iterator portIter = toAdd.allPorts.begin() ;
  while (portIter != toAdd.allPorts.end())
  {
    totalBitSize += (*portIter).size ;
    ++portIter ;
  }

  // Construct the group symbol table for the struct
  GroupSymbolTable* structSymTab = create_group_symbol_table(theEnv) ;
  portIter = toAdd.allPorts.begin() ;
  int currentOffset = 0 ;
  while (portIter != toAdd.allPorts.end())
  {
    // Create a field symbol for each port and add it to the group
    //  symbol table. 

    // Each field symbol requires a qualified type, which in turn requires
    //  a normal type.
    DataType* nextType ;
    
    if ((*portIter).isFloat == true)
    {
      nextType = create_floating_point_type(theEnv,
					    IInteger((*portIter).size),
					    0) ; // alignment
    }
    else
    {
      nextType = create_integer_type(theEnv,
				     IInteger((*portIter).size),
				     0, // alignment
				     false) ; // is signed
    }

    QualifiedType* qualFieldType = create_qualified_type(theEnv,
							 nextType) ;

    symTab->append_symbol_table_object(nextType) ;
    symTab->append_symbol_table_object(qualFieldType) ;

    // Each field also needs an offset expression associated with it.
    //  This should be an int constant, which in turn requires an integer type

    IntegerType* constantType = GetBaseInt(theEnv) ;

    IntConstant* offset = create_int_constant(theEnv,
					      constantType,
					      IInteger(currentOffset)) ;    
    FieldSymbol* nextField = 
      create_field_symbol(theEnv,
			  qualFieldType,
			  offset,
			  LString((*portIter).name.c_str()),
			  false) ; // is address taken

    // Finally, add the symbol to the group symbol table
    structSymTab->append_symbol_table_object(nextField) ;



    currentOffset += (*portIter).size ;
    ++portIter ;
  }

  structToAdd = create_struct_type(theEnv,
				   IInteger(totalBitSize),
				   0, // Bit alignment
				   structName,
				   true, // is complete
				   structSymTab) ;
  
  symTab->append_symbol_table_object(structToAdd) ;
}

void LibraryOutputPass::AddProcedure(ModuleEntry toAdd) 
{
  assert(repository != NULL) ;
  assert(structToAdd != NULL) ;
  SymbolTable* symTab = repository->get_external_symbol_table() ;

  // Before we create the procedure we need to create the procedure
  //  type, which will be the class CProcedureType.
  CProcedureType* cProcType = create_c_procedure_type(theEnv,
						      structToAdd,
						      false, // has varargs
						      true, // arguments known
						      0) ; // bit alignment
  QualifiedType* qualStructType = create_qualified_type(theEnv, structToAdd) ;
  cProcType->append_argument(qualStructType) ;

  procedureToAdd = create_procedure_symbol(theEnv,
					   cProcType,
					   LString(toAdd.name.c_str())) ;
  symTab->append_symbol_table_object(procedureToAdd) ;
  symTab->append_symbol_table_object(cProcType) ;
}

void LibraryOutputPass::ExportSystem()
{
  assert(currentProcedure != NULL) ;
  SymbolTable* tmpSymTab = currentProcedure->get_symbol_table() ;

  // A test to see if I can identify outputs
  for (int i = 0 ; i < tmpSymTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* nextObj = tmpSymTab->get_symbol_table_object(i) ;
    if (nextObj->lookup_annote_by_name("OutputFifo") != NULL || 
	nextObj->lookup_annote_by_name("OutputScalar") != NULL ||
	nextObj->lookup_annote_by_name("OutputVariable") != NULL)
    {
      // The original name annotation is created in the pointer conversion 
      //  pass
      BrickAnnote* originalNameAnnote = 
   dynamic_cast<BrickAnnote*>(nextObj->lookup_annote_by_name("OriginalName")) ;
      if (originalNameAnnote == NULL)
      {
	identifiedOutputs.push_back(nextObj->get_name()) ;
      }
      else
      {
	StringBrick* originalNameBrick = 
	  dynamic_cast<StringBrick*>(originalNameAnnote->get_brick(0)) ;
	assert(originalNameBrick != NULL) ;
	identifiedOutputs.push_back(originalNameBrick->get_value()) ;
      }

    }
  }

  for (int i = 0 ; i < currentProcedure->get_formal_parameter_count() ; ++i)
  {
    if (IsOutput(currentProcedure->get_formal_parameter(i)->get_name()))
    {
      outputNumbers.push_back(i) ;
    }
  }

  // Exporting a system requires both a procedure symbol and a
  //  procedure type to be created and placed in the repository.
  //  We will have to clone these from the original ones and
  //  then read in the repository and add them to the symbol table of the
  //  repository.

  // Clone the procedure symbol and set all of its parameters
  ProcedureSymbol* originalSymbol = currentProcedure->get_procedure_symbol() ;
  String originalName = originalSymbol->get_name() ;
  ProcedureSymbol* cloneSymbol = 
    dynamic_cast<ProcedureSymbol*>(originalSymbol->deep_clone()) ;  
  assert(cloneSymbol != NULL) ;
  cloneSymbol->set_definition(NULL) ;

  // Clone the type and set all of its parameters
  CProcedureType* originalType = 
    dynamic_cast<CProcedureType*>(originalSymbol->get_type()) ;
  assert(originalType != NULL) ;
  CProcedureType* cloneType = 
    dynamic_cast<CProcedureType*>(originalType->deep_clone()) ;

  DataType* originalResult = originalType->get_result_type() ;
  DataType* cloneResult = 
    dynamic_cast<DataType*>(originalResult->deep_clone()) ;
  assert(cloneResult != NULL) ;

  cloneType->set_result_type(cloneResult) ;

  for (int i = 0 ; i < originalType->get_argument_count() ; ++i)
  {
    QualifiedType* originalArg = originalType->get_argument(i) ;
    QualifiedType* cloneArg = 
      dynamic_cast<QualifiedType*>(CloneType(originalArg)) ;
    assert(cloneArg != NULL) ;

    cloneType->replace_argument(i, cloneArg) ;

    if (IsOutputNumber(i))
    {      
      cloneArg->append_annote(create_brick_annote(theEnv, "Output")) ;
      String newName = cloneArg->get_name() ;
      newName = newName + "_out" ;
      cloneArg->set_name(newName) ;
    }
  }

  cloneSymbol->set_type(cloneType) ;

  // Everything should be set up, so read in the repository and wipe
  //  out everything associated with the current procedure
  InitializeRepository() ;

  // Clean up the original value from the repository if it exists
  RemoveSystem(originalName) ;

  // Now add our information to the new symbol table
  SymbolTable* symTab = repository->get_external_symbol_table() ;
  assert(symTab != NULL) ;

  symTab->append_symbol_table_object(cloneSymbol) ;
  symTab->append_symbol_table_object(cloneType) ;
}

Type* LibraryOutputPass::CloneType(Type* originalType)
{
  Type* constructedType = dynamic_cast<Type*>(originalType->deep_clone()) ;

  if (dynamic_cast<QualifiedType*>(originalType) != NULL)
  {
    QualifiedType* originalQual = 
      dynamic_cast<QualifiedType*>(originalType) ;
    QualifiedType* constructedQual = 
      dynamic_cast<QualifiedType*>(constructedType) ;
    assert(originalType != NULL) ;
    assert(constructedQual != NULL) ;

    Type* cloneBase = CloneType(originalQual->get_base_type()) ;
    DataType* constructedBase = dynamic_cast<DataType*>(cloneBase) ;
    assert(constructedBase != NULL) ;

    constructedQual->set_base_type(constructedBase) ;
  }

  if (dynamic_cast<DataType*>(originalType) != NULL)
  {
    if (dynamic_cast<PointerType*>(originalType) != NULL)
    {
      PointerType* originalPointer = 
	dynamic_cast<PointerType*>(originalType) ;
      PointerType* constructedPointer = 
	dynamic_cast<PointerType*>(constructedType) ;
      assert(originalPointer != NULL) ;
      assert(constructedPointer != NULL) ;

      constructedPointer->set_reference_type(CloneType(originalPointer->get_reference_type())) ;
    }
    if (dynamic_cast<VoidType*>(originalType) != NULL)
    {
      // Nothing else to do
    }
    if (dynamic_cast<NumericType*>(originalType) != NULL)
    {
      // Nothing else to do
    }
    if (dynamic_cast<ReferenceType*>(originalType) != NULL)
    {
      ReferenceType* originalRef = 
	dynamic_cast<ReferenceType*>(originalType) ;
      ReferenceType* constructedRef = 
	dynamic_cast<ReferenceType*>(constructedType) ;
      constructedRef->set_reference_type(CloneType(originalRef->get_reference_type())) ;
    }
    if (dynamic_cast<ArrayType*>(originalType) != NULL)
    {
      ArrayType* originalArray =
	dynamic_cast<ArrayType*>(originalType) ;
      ArrayType* constructedArray = 
	dynamic_cast<ArrayType*>(constructedType) ;
      assert(originalArray != NULL) ;
      assert(constructedArray != NULL) ;

      Type* cloneElement = CloneType(originalArray->get_element_type()) ;
      QualifiedType* qualifiedClone = 
	dynamic_cast<QualifiedType*>(cloneElement) ;
      assert(qualifiedClone != NULL) ;

      constructedArray->set_element_type(qualifiedClone) ;
    }
    if (dynamic_cast<MultiDimArrayType*>(originalType) != NULL)
    {
      MultiDimArrayType* originalMulti =
	dynamic_cast<MultiDimArrayType*>(originalType) ;
      MultiDimArrayType* constructedMulti = 
	dynamic_cast<MultiDimArrayType*>(constructedType) ;
      assert(originalMulti != NULL) ;
      assert(constructedMulti != NULL) ;

      Type* cloneElement = CloneType(originalMulti->get_element_type()) ;
      QualifiedType* qualifiedClone = 
	dynamic_cast<QualifiedType*>(cloneElement) ;
      assert(qualifiedClone != NULL) ;
      
      constructedMulti->set_element_type(qualifiedClone) ;
    }
    if (dynamic_cast<GroupType*>(originalType) != NULL)
    {
      assert(0 && "Structs to composable systems are not yet supported") ;
    }
  }

  return constructedType ;
}

bool LibraryOutputPass::IsOutput(String x)
{
  std::list<String>::iterator outputIter = identifiedOutputs.begin() ;
  while (outputIter != identifiedOutputs.end())
  {
    if (x == (*outputIter)) 
    {
      return true ;
    }
    ++outputIter ;
  }
  return false ;
}

bool LibraryOutputPass::IsOutputNumber(int x)
{
  std::list<int>::iterator outputIter = outputNumbers.begin() ;
  while (outputIter != outputNumbers.end())
  {
    if (x == (*outputIter))
    {
      return true ;
    }
    ++outputIter ;
  }
  return false ;
}

void LibraryOutputPass::RemoveSystem(String x)
{
  assert(repository != NULL) ;
  list<SymbolTableObject*> removeList ;

  Iter<SymbolTableObject*> findName = repository->get_external_symbol_table()->
    get_symbol_table_object_iterator() ;

  while(findName.is_valid())
  {
    if (findName.current()->get_name() == x)
    {
      // Remove the symbol, the type, and the definition
      removeList.push_back(findName.current()) ;
      assert(dynamic_cast<ProcedureSymbol*>(findName.current()) != NULL) ;
    }
    findName.next() ;
  }
  for(list<SymbolTableObject*>::iterator removeIter = removeList.begin() ;
      removeIter != removeList.end() ;
      ++removeIter)
  {
    repository->get_external_symbol_table()->remove_symbol_table_object(*removeIter) ;
  }

}
