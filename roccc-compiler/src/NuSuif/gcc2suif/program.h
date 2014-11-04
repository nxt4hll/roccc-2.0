/*

  This file contains the declaration of the Program class.  This consists
   of a list of Functions that were located in the file.

*/

#ifndef __PROGRAM_DOT_H__
#define __PROGRAM_DOT_H__

#include <common/suif_list.h>
#include <common/lstring.h>

#include "function.h"

// Forward declaration
class SuifGenerator ;

class Program
{
 private:

  list<Function*> allFunctions ;

  // Helper functions that could be generalized, but are only
  //  used in this class, so they are private
  //  int logTwo(int x) ;
  //LString binary(int number, int maximum) ;

 public:

  // The suifGenerator keeps track of all the factories and 
  //  symbol tables associated with the program.  It is public because
  //  almost everyone uses it currently.  This is kind of bad, but
  //  it works for now (until I can refactor later).
  SuifGenerator* converter ;
  
  Program() ;
  Program(const char* fileName) ;
  ~Program() ;
  void addFunction(Function* toAdd) ;

  void generateSuif() ;

  void connect() ;

  // The output and input file are used in the SuifGenerator
  void setOutputFile(LString& newName) ;
  //  void setInputFile(LString& newName) ;

};

#endif
