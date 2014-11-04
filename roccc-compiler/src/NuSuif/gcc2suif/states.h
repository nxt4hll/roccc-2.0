/*

  This file contains the declaration of State class.  Each State will
   contain all the code for that state in a stringstream. Normal states 
   will only have one next state.  Conditional branches will have two.

  States can also be viewed as basic blocks.  They will contain a list of 
   instructions which will be as close to C-minor as possible.

*/

#ifndef __STATES_DOT_H__
#define __STATES_DOT_H__

#include <common/suif_list.h>
#include <common/lstring.h>

#include "generic.h"

using namespace std;

// Forward Declarations
class Node;
class Function;

class State
{

 private:
  
  bool visited ;

 protected:

  State* nextState;
  
  int number;

  // This string holds the value of the label that starts this state (if any)

  LString identifyingLabel ; 

 public:

  bool isFinal;

  list<Node*> instructionList ;
  // Removed on 6/11/2004.  I don't think this is used anywhere anymore.
  //  stringstream instructions;

  State();
  virtual ~State() { };

  void setNextState(State* n);
  State* getNextState();
  void setNumber(int n);
  int getNumber();
  virtual void generateVHDL(Function* t, Node* p);

  void setLabel(LString s) ;
  LString getLabel() ;

  virtual void simplify(Function* t) ;
  void markAsVisited();
  bool marked() ;
  void clearMark() ;

  virtual void addChildren(list<State*>& toVisit) ;

};

/*

  The IfState class is the currently the only derived class.  It 
    is used in every looping structure and every conditional branch.

*/
class IfState : public State
{
 private:
  State* thenBranch;
  State* elseBranch;
 public:

  LString resultRegister;

  IfState();
  ~IfState();
  void generateVHDL(Function* t, Node* p);

  void setThenBranch(State* t);
  void setElseBranch(State* e);
  void simplify(Function* t) ;

  void addChildren(list<State*>& toVisit) ;

};

#endif
