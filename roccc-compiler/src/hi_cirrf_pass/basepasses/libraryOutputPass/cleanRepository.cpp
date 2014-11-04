// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include <cassert>

#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>
#include <suifkernel/command_line_parsing.h>

#include "roccc_utils/warning_utils.h"

#include "cleanRepository.h"

CleanRepositoryPass::CleanRepositoryPass(SuifEnv* pEnv) : 
  PipelinablePass(pEnv, "CleanRepositoryPass")
{
  theEnv = pEnv ;
  repository = NULL ;
}

CleanRepositoryPass::~CleanRepositoryPass()
{
  ; // Nothing to delete 
}

void CleanRepositoryPass::initialize()
{
  PipelinablePass::initialize() ;
  _command_line->set_description("Cleans the repository suif file") ;
  OptionString* directory = new OptionString("Local Dir", &localDirectory) ;
  OptionList* args = new OptionList() ;
  args->add(directory) ;
  _command_line->add(args) ;
}

void CleanRepositoryPass::execute()
{
  OutputInformation("Clean Repository Pass begins") ;
  InitializeRepository() ;
  CleanRepository() ;
  DumpRepository() ;
  OutputInformation("Clean Repository Pass ends") ;
}

void CleanRepositoryPass::InitializeRepository()
{
  std::string fullPath = localDirectory.c_str() ;
  fullPath += "/repository.suif" ;
  theEnv->read(fullPath.c_str()) ;
  repository = theEnv->get_file_set_block() ;
}

void CleanRepositoryPass::CleanRepository()
{
  assert(repository != NULL) ;
  if (repository->lookup_annote_by_name("Cleaned") != NULL)
  {
    return ;
  }
  else
  {
    repository->append_annote(create_brick_annote(theEnv, "Cleaned")) ;
  }
  list<SymbolTableObject*> removeList ;
  Iter<SymbolTableObject*> findName = 
   repository->get_external_symbol_table()->get_symbol_table_object_iterator();
  while (findName.is_valid())
  {
    ProcedureSymbol* currentSym = 
      dynamic_cast<ProcedureSymbol*>(findName.current()) ;
    if (currentSym != NULL && currentSym->get_name() == LString("unknown"))
    {
      removeList.push_back(currentSym) ;
      removeList.push_back(currentSym->get_type()) ;
      currentSym->set_type(NULL) ;
      currentSym->get_definition()->set_symbol_table(NULL) ;
      currentSym->get_definition()->set_definition_block(NULL) ;
      currentSym->get_definition()->set_body(NULL) ;
      currentSym->get_definition()->set_procedure_symbol(NULL) ;
      currentSym->set_definition(NULL) ;
    }
    findName.next() ;    
  }
  for (list<SymbolTableObject*>::iterator removeIter = removeList.begin() ;
       removeIter != removeList.end() ;
       ++removeIter)
  {
    repository->get_external_symbol_table()->remove_symbol_table_object(*removeIter) ;
  }
}

void CleanRepositoryPass::DumpRepository()
{
  assert(theEnv != NULL) ;
  std::string fullPath = localDirectory.c_str() ;
  fullPath += "/repository.suif" ;
  theEnv->write(fullPath.c_str()) ;
}
