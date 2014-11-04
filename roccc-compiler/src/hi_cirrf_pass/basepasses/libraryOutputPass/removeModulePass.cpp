
#include <cassert>

#include <suifkernel/command_line_parsing.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

#include "removeModulePass.h"

RemoveModulePass::RemoveModulePass(SuifEnv* pEnv) : 
  PipelinablePass(pEnv, "RemoveModulePass")
{
  theEnv = pEnv ;
  repository = NULL ;
}

RemoveModulePass::~RemoveModulePass()
{
  ; // Nothing to clean up
}

void RemoveModulePass::initialize()
{
  PipelinablePass::initialize() ;
  _command_line->set_description("Remove a module from the repository") ;
  OptionString* name = new OptionString("Module Name", &moduleName) ;
  OptionString* directory = new OptionString("Local Directory",
					     &localDirectory) ;
  OptionList* args = new OptionList() ;
  args->add(name) ; 
  args->add(directory) ;
  _command_line->add(args) ;
}

void RemoveModulePass::execute()
{
  OutputInformation("Remove module pass begins") ;
  
  InitializeRepository() ;

  RemoveProcedure(FindProcedure(moduleName)) ;  

  DumpRepository() ;

  OutputInformation("Remove module pass ends") ;
}

void RemoveModulePass::InitializeRepository()
{
  assert(theEnv != NULL) ;
  std::string fullPath = localDirectory.c_str() ;
  fullPath += "/repository.suif" ;
  theEnv->read(fullPath.c_str()) ;
  repository = theEnv->get_file_set_block() ;
}

ProcedureSymbol* RemoveModulePass::FindProcedure(String name)
{
  assert(repository != NULL) ;
  SymbolTable* symTab = repository->get_external_symbol_table() ;

  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    ProcedureSymbol* nextProc = 
      dynamic_cast<ProcedureSymbol*>(symTab->get_symbol_table_object(i)) ;
    if (nextProc != NULL && nextProc->get_name() == name)
    {
      return nextProc ;
    }
  }
  return NULL ;
}

// Do a sort of garbage collection.  Take all of the types that we want to
//  delete and collect them.  If there are no uses of them after this function
//  has finished, then remove them.
void RemoveModulePass::RemoveProcedure(ProcedureSymbol* p)
{
  assert(repository != NULL) ;
  list<Type*> usedTypes ;

  if (p == NULL)
  {
    return ;
  }

  // There should be no definition, but just in case remove it
  ProcedureDefinition* defToRemove = p->get_definition() ;
  p->set_definition(NULL) ;
  if (defToRemove != NULL)
  {
    delete defToRemove ;
  }

  // Clear out all of the values associated with the procedure type
  CProcedureType* procType = 
    dynamic_cast<CProcedureType*>(p->get_type()) ;
  assert(procType != NULL) ;

  DataType* returnTypeToRemove = procType->get_result_type() ;
  procType->set_result_type(NULL) ;
  if (returnTypeToRemove != NULL)
  {
    usedTypes.push_back(returnTypeToRemove) ;
  }
  
  while (procType->get_argument_count() > 0)
  {
    QualifiedType* currentArg = procType->get_argument(0) ;
    procType->remove_argument(0) ;
    if (!InList(usedTypes, currentArg))
    {
      usedTypes.push_back(currentArg) ;
    }
  }
  SymbolTable* symTab = repository->get_external_symbol_table() ;
  p->set_type(NULL) ;
  symTab->remove_symbol_table_object(procType) ;
  delete procType ;
  symTab->remove_symbol_table_object(p) ;
  delete p ;

  // Now, go through each used type and see if it is used anywhere
  list<Type*>::iterator typeIter = usedTypes.begin() ;
  while (typeIter != usedTypes.end())
  {
    bool removeMe = true ;
    for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
    {
      CProcedureType* currentType = 
	dynamic_cast<CProcedureType*>(symTab->get_symbol_table_object(i)) ;
      if (currentType != NULL) 
      {
	if (IsUsed(*typeIter, currentType))
	{
	  removeMe = false ;
	  break ;
	}
      }
    }
    if (removeMe)
    {
      if ((*typeIter)->lookup_annote_by_name("Output") != NULL)
      {
	delete (*typeIter)->remove_annote_by_name("Output") ;
      }
      QualifiedType* q = dynamic_cast<QualifiedType*>(*typeIter) ;
      DataType* d = dynamic_cast<DataType*>(*typeIter) ;
      if (q != NULL)
      {	
	DataType* internalD = q->get_base_type() ;
	q->set_base_type(NULL) ;
	symTab->remove_symbol_table_object(internalD) ;
	symTab->remove_symbol_table_object(q) ;
	delete internalD ;
	delete q ;
      }
      else if (d != NULL)
      {
	symTab->remove_symbol_table_object(d) ;
	delete d ;
      }
      else
      {
	assert(0 && "Trying to remove something weird...") ;
      }

    }
    ++typeIter ;
  }

}

void RemoveModulePass::DumpRepository()
{
  assert(theEnv != NULL) ;
  std::string fullPath = localDirectory.c_str() ;
  fullPath += "/repository.suif" ;
  theEnv->write(fullPath.c_str()) ;
}

bool RemoveModulePass::InList(list<Type*>& used, Type* x)
{
  list<Type*>::iterator typeIter = used.begin() ;
  while (typeIter != used.end())
  {
    if (*typeIter == x)
    {
      return true ;
    }
    ++typeIter ;
  }
  return false ;
}
