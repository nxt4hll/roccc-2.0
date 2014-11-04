// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include <cassert>

#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>
#include <suifkernel/command_line_parsing.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

#include "addModulePass.h"
#include "libOutputPass.h"

AddModulePass::AddModulePass(SuifEnv* pEnv) : 
  PipelinablePass(pEnv, "AddModulePass")
{
  theEnv = pEnv ;
  repository = NULL ;
}

AddModulePass::~AddModulePass()
{
  ; // Nothing to delete yet...
}

void AddModulePass::initialize()
{
  PipelinablePass::initialize() ;
  _command_line->set_description("Adds a module to the repository") ;
  OptionString* name = new OptionString("Module file", &moduleFile) ;
  OptionString* directory = new OptionString("Local Dir", &localDirectory) ;
  OptionList* args = new OptionList() ;
  args->add(name) ;
  args->add(directory) ;
  _command_line->add(args) ;
}

void AddModulePass::execute()
{
  assert(theEnv != NULL) ;
  OutputInformation("Add Module Pass begins") ;
  InitializeRepository() ;
  ModuleEntry toAdd = ProcessFile(moduleFile) ;
  AddProcedure(toAdd) ;
  DumpRepository() ;
  OutputInformation("Add Module Pass ends") ;
}

ModuleEntry AddModulePass::ProcessFile(String filename)
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
    fin >> std::ws >> nextPort.name ;
    fin >> std::ws >> inputValue ;
    if (inputValue == "IN")
    {
      nextPort.isInput = true ;
    }
    else if (inputValue == "OUT")
    {
      nextPort.isInput = false ;
    }
    else
    {
      assert(0 && "Incorrectly formatted component file!") ;
    }
    fin >> std::ws >> nextPort.size ;
    fin >> std::ws >> intValue >> std::ws ;
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

void AddModulePass::AddProcedure(ModuleEntry s)
{
  assert(repository != NULL) ;

  SymbolTable* symTab = repository->get_external_symbol_table() ;

  // Create a procedure symbol.  This requires a procedure type.
  //  The procedure should return void and take in all of the types.
  //  If one of the arguments is an output, I must annotate it with the 
  //  "Output" annotation.

  // When adding a module, we have to create a new void type just in case
  //  the user wants to remove this module later on.  This could lead to 
  //  a bunch of void types, but that is acceptable.
  VoidType* returnType = create_void_type(theEnv, IInteger(32), 0) ;
  symTab->append_symbol_table_object(returnType) ;

  CProcedureType* cProcType =
    create_c_procedure_type(theEnv,
			    returnType,
			    false, // has varargs
			    true, // arguments_known
			    0) ; // bit alignment

  // I need a qualified type for every argument
  std::list<Port>::iterator portIter = s.allPorts.begin() ;
  while (portIter != s.allPorts.end())
  {
    DataType* baseType  = NULL ;
    if ((*portIter).isFloat)
    {
      baseType = create_floating_point_type(theEnv,
					    IInteger((*portIter).size),
					    0) ; // bit alignment
    }
    else
    {
      baseType = create_integer_type(theEnv, 
				     IInteger((*portIter).size),
				     0,      // bit alignment
				     true) ; // is signed
    }
    assert(baseType != NULL) ;
    QualifiedType* qualType = create_qualified_type(theEnv, baseType) ;
    assert(qualType != NULL) ;
    if (!((*portIter).isInput))
    {
      qualType->append_annote(create_brick_annote(theEnv, "Output")) ;      
    }
    assert(symTab != NULL) ;

    // Look through the symbol table and see if any equivalent types exist
    //  and use that instead.
    QualifiedType* foundType = NULL ;    
    for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
    {
      QualifiedType* existingType = 
	dynamic_cast<QualifiedType*>(symTab->get_symbol_table_object(i)) ;
      if (existingType != NULL && EquivalentTypes(existingType, qualType) &&
	  existingType->get_annote_count() == qualType->get_annote_count())
      {
	foundType = existingType ;
	break ;
      }
    }
    
    if (foundType != NULL)
    {
      // Clean up and replace      
      delete baseType ;
      if (qualType->lookup_annote_by_name("Output") != NULL)
      {
	delete qualType->remove_annote_by_name("Output") ;
      }
      delete qualType ;      
      qualType = foundType ;
    }
    else
    {
      symTab->append_symbol_table_object(baseType) ;
      symTab->append_symbol_table_object(qualType) ;
    }
    
    cProcType->append_argument(qualType) ;
    ++portIter ;
  }
  
  symTab->append_symbol_table_object(cProcType) ;
  ProcedureSymbol* procSym = 
    create_procedure_symbol(theEnv,
			    cProcType,
			    s.name.c_str()) ;
  symTab->append_symbol_table_object(procSym) ;
}

void AddModulePass::InitializeRepository()
{
  assert(theEnv != NULL) ;
  std::string fullPath = localDirectory.c_str() ;
  fullPath += "/repository.suif" ;
  theEnv->read(fullPath.c_str()) ;
  repository = theEnv->get_file_set_block() ;
}

void AddModulePass::DumpRepository()
{
  assert(theEnv != NULL) ;
  std::string fullPath = localDirectory.c_str() ;
  fullPath += "/repository.suif" ;
  theEnv->write(fullPath.c_str()) ;
}
