/*
  This file contains the declaration of the function class, which contains a 
   complete C/C++ function.  
*/

#ifndef __FUNCTION_DOT_H__
#define __FUNCTION_DOT_H__

#include <common/suif_list.h>
#include <common/suif_vector.h>
#include <common/lstring.h>

using namespace std;

class Node; // Forward Declaration
class Program ;

// Forward declaration of suif classes
class Symbol ;
class QualifiedType ;

class Function
{
 private:

  // This is the name of the function we are currently in
  LString myName ;

  // This function retrieves the name of the function from the
  //  nodes in the function and updates the string variable "myName"
  void updateName() ;

 public:

  // I don't really like these variables being public, but 
  //  the way I constructed the whole project sort of necessitates this
  //  right now.  I would like to restructure the project, but I don't see
  //  that happening any time soon.

  suif_vector<Node*>* allNodes;

  // --------------------------------------------
  //  FUNCTIONS
  // --------------------------------------------

  // The constructor takes in a pointer to a pre-built vector of nodes
  Function(suif_vector<Node*>* a);
  ~Function();

  // These are the three main tasks and the main interfaces to this
  //  class.
  void connectTree();
  void flatten() ;
  void generateSuif(Program* p) ;

  // This will simply return the current name of the function
  LString name() ;

  // This function goes through all of the nodes associated with this
  //  function and determines which variables are parameters.
  list<Symbol*> findParameters(Program* p);

  QualifiedType* findReturnType(Program* p) ;

};

#endif
