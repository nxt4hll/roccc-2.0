
#include <cassert>
#include <cstdlib>

#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>
#include <suifkernel/command_line_parsing.h>

#include "identifyDebug.h"

#include "roccc_utils/warning_utils.h"

IdentifyDebugPass::IdentifyDebugPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "IdentifyDebugPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void IdentifyDebugPass::initialize()
{
  PipelinablePass::initialize() ;
  _command_line->set_description("Find registers marked for debug") ;
  OptionList* debugArgs = new OptionList() ;
  OptionString* debugFile = new OptionString("FileName", &filename) ;
  debugArgs->add(debugFile) ;
  _command_line->add(debugArgs) ;
}

IdentifyDebugPass::~IdentifyDebugPass()
{
  ; // Nothing to delete yet...
}

void IdentifyDebugPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;

  OutputInformation("Identify Debug Pass Begins") ;
  
  fin.open(filename.c_str()) ;
  if (!fin)
  {
    OutputError("Cannot open the debug register file!") ;
    return ;
  }

  ProcessFile() ;

  OutputInformation("Identify Debug Pass Ends") ;
}

void IdentifyDebugPass::ProcessFile()
{
  assert(procDef != NULL) ;
  SymbolTable* symTab = procDef->get_symbol_table() ;

  // Go through all of the registers in the file and put them in
  //  a container
  std::list<std::string> allDebugs ;
  std::list<int> allWatchPoints ;
  std::list<int> allWatchValid ;

  fin >> std::ws ;

  while (!fin.eof())
  {
    std::string nextReg ;
    std::string nextWatchPoint ;
    std::string nextWatchValid ;
    fin >> nextReg >> std::ws 
	>> nextWatchPoint >> std::ws 
	>> nextWatchValid >> std::ws ;
    allDebugs.push_back(nextReg) ;
    allWatchPoints.push_back(atoll(nextWatchPoint.c_str())) ;
    allWatchValid.push_back(atoi(nextWatchValid.c_str())) ;
  }

  fin.close() ;

  ProcessSymbolTable(symTab, allDebugs, allWatchPoints, allWatchValid) ;

}

// New code that handles watch points as well.
void IdentifyDebugPass::ProcessSymbolTable(SymbolTable* symTab,
					   std::list<std::string>& debugRegs,
					   std::list<int>& watchPts,
					   std::list<int>& watchValid)
{
  // Go through all of the lists at the same rate
  std::list<std::string>::iterator regIter = debugRegs.begin() ;
  std::list<int>::iterator watchIter = watchPts.begin() ;
  std::list<int>::iterator validIter = watchValid.begin() ;
  while(regIter != debugRegs.end())
  {
    // For each identified debug variable, go through the symbol table
    //  until we find the variable associated with this name
    for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
    {
      SymbolTableObject* currentObject = symTab->get_symbol_table_object(i) ;
      std::string compareString = currentObject->get_name().c_str() ;
      VariableSymbol* varSym = dynamic_cast<VariableSymbol*>(currentObject) ;
      if (varSym != NULL)
      {
	// Special case, if this is a struct variable, check the 
	//  symbol table associated with the struct
	StructType* recurseType = 
	  dynamic_cast<StructType*>(varSym->get_type()->get_base_type()) ;
	if (recurseType != NULL)
	{
	  ProcessSymbolTable(recurseType->get_group_symbol_table(),
			     debugRegs, watchPts, watchValid) ;
	}
	else if (dynamic_cast<ArrayType*>(varSym->get_type()->get_base_type())
		 == NULL)
	{
	  // Only check if the variable is not an array.  We cannot turn
	  //  arrays into debug variables
	  if (compareString == (*regIter))
	  {
	    BrickAnnote* watchAnnote =
	      create_brick_annote(theEnv, "WatchPoint") ;
	    BrickAnnote* validAnnote = 
	      create_brick_annote(theEnv, "WatchValid") ;
	    IntegerBrick* watchBrick = 
	      create_integer_brick(theEnv, IInteger(*watchIter)) ;
	    IntegerBrick* validBrick = 
	      create_integer_brick(theEnv, IInteger(*validIter)) ;
	    watchAnnote->append_brick(watchBrick) ;
	    validAnnote->append_brick(validBrick) ;

	    currentObject->append_annote(create_brick_annote(theEnv,
							     "DebugRegister"));
	    currentObject->append_annote(watchAnnote);
	    currentObject->append_annote(validAnnote);
	  }
	}
      }
    }
    
    ++regIter ;
    ++watchIter ;
    ++validIter ;
  }

}

// The previous code is below
/*
void IdentifyDebugPass::ProcessSymbolTable(SymbolTable* symTab,
					   std::list<std::string>& debugRegs)
{
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* currentObject = symTab->get_symbol_table_object(i) ;
    std::string compareString = currentObject->get_name().c_str() ;

    // If this is a struct, go into it's symbol table.
    VariableSymbol* varSym = dynamic_cast<VariableSymbol*>(currentObject) ;
    if (varSym != NULL)
    {
      StructType* recurseType = 
	dynamic_cast<StructType*>(varSym->get_type()->get_base_type()) ;
      if (recurseType != NULL)
      {
	ProcessSymbolTable(recurseType->get_group_symbol_table(),
			   debugRegs) ;
      }
      else if (dynamic_cast<ArrayType*>(varSym->get_type()->get_base_type()) == NULL)
      {
	if (InList(debugRegs, compareString))
	{
	  currentObject->append_annote(create_brick_annote(theEnv,
							   "DebugRegister")) ;
	}
      }
    }
  }
}
*/
bool IdentifyDebugPass::InList(std::list<std::string>& toCheck,
			       std::string& name)
{
  std::list<std::string>::iterator checkIter = toCheck.begin() ;
  while (checkIter != toCheck.end())
  {
    if (name.find(*checkIter) == 0 || (*checkIter) == name)
    {
      return true ;
    }
    ++checkIter ;
  }
  return false ;
}
