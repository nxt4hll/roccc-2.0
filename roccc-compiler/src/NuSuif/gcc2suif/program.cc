/*

  This file contains the definitions of the Program class functions.

*/

#include <assert.h>
#include <fstream>
#include <iostream>
#include <math.h>

#include "program.h"
#include "suifGenerator.h"

#include <common/lstring.h>

using namespace std ;

Program::Program()
{
  converter = new SuifGenerator("NoInput", "DefaultOutput.suif") ;
}

Program::Program(const char* fileName)
{
  converter = new SuifGenerator(fileName, "DefaultOutput.suif") ;
}

Program::~Program()
{
  // should I not delete the functions?
  list<Function*>::iterator deleteIter = allFunctions.begin() ;
  while (deleteIter != allFunctions.end())
  {
    delete (*deleteIter) ;
    ++deleteIter ;
  }
  if (converter != NULL)
    delete converter ;
}

void Program::addFunction(Function* toAdd)
{
  assert(toAdd != NULL) ;
  allFunctions.push_back(toAdd) ;
}

void Program::connect()
{
  // Connect and flatten the tree (remove the "option" indirection)
  list<Function*>::iterator connectIter = allFunctions.begin() ;
  while (connectIter != allFunctions.end())
  {
    (*connectIter)->connectTree() ;
    (*connectIter)->flatten() ;
    ++connectIter ;
  }
}

/*

  Function: Program::generateSuif()

  This function represents the main entry point to the suif building process.
  It should go through all of the individual functions seperately and
  make sure that suif is generated for each one that has some code 
  associated with it.

*/

void Program::generateSuif()
{
  list<Function*>::iterator generateIter = allFunctions.begin() ;
  while (generateIter != allFunctions.end())
  {
    // Find out the information regarding the function (it's name,
    //  it's arguments, and its return type

    list<Symbol*> args = (*generateIter)->findParameters(this) ;

    DataType* returnType = ((*generateIter)->findReturnType(this))->get_base_type() ; 

    LString functionName = (*generateIter)->name() ;

    converter->StartNewFunction(functionName, args, returnType) ;

    (*generateIter)->generateSuif(this) ;
    
    converter->FinalizeCurrentFunction() ;

    ++generateIter ;
    
  }

  converter->WriteSuifFile() ;
}

/*
BasicSymbolTable* Program::createSymTab()
{
  return NULL ;
}
*/

/*  Private Functions */

// I probably don't need this anymore, but I'll keep it for a little
//  while
/*
int Program::logTwo(int x) 
{
  return (int) (log((double)x) / log(2.0)) ;
}

LString Program::binary(int number, int maximum)
{
  // determine the number of bits
  int numBits = logTwo(maximum) ;

  // Now go through and convert number to a string
  LString toReturn = "" ;

  for(int i = 0 ; i <= numBits; ++i)
  {
    if ((number % 2) == 1)
    {
      toReturn = '1' + toReturn ;
    }
    else
    {
      toReturn = '0' + toReturn ;
    }
    number >>= 1 ;
  }

  return toReturn ;  
}
*/
void Program::setOutputFile(LString& newName)
{
  converter->setOutputFile(newName) ;
}
