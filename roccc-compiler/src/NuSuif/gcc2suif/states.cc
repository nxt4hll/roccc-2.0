
#include <iostream>
#include <cassert>

#include "states.h"
#include "nodes.h"

using namespace std;

State::State() 
{
  nextState = NULL;
  isFinal = false;
  identifyingLabel = "No Label" ;
  visited = false ;
}

void State::setNextState(State* n)
{
  //  assert(n != NULL);
  nextState = n;
}

State* State::getNextState()
{
  return nextState ;
}

void State::setNumber(int n) 
{
  number = n;
}

int State::getNumber()
{
  return number;
}

/*

  By the time the State::generateVHDL function is called, the program
   will have been parsed and everything will be set up to be output.

*/

void State::generateVHDL(Function* t, Node* p)
{
  assert(nextState != NULL);

  cout << "when ST" << number << " =>" << endl ;

  cout << "-- " << identifyingLabel << endl ;

  list<Node*>::iterator printIter = instructionList.begin() ;
  while (printIter != instructionList.end())
  {
    (*printIter)->outputInstruction() ;
    ++printIter ;
  }

  if (isFinal)
  {
    cout << "outputPulse <= '1' ;" << endl;
  }

  cout << "currentState <= ST" << nextState->number << ";" << endl ;
}

void State::setLabel(LString s)
{
  identifyingLabel = s ;
}

LString State::getLabel()
{
  return identifyingLabel ;
}

void State::markAsVisited() 
{
  visited = true ;
}

bool State::marked()
{
  return visited ;
}

void State::clearMark()
{
  visited = false ;
}

void State::simplify(Function* t)
{
  // As long as the destination node is empty of instructions, and we have not
  //  looped, keep going until we find the final destination

  State* finalDestination = nextState ;

  // Stop eliminating states if we run into a state with instructions,
  //  an if statement, or a state we have already discovered.
  while(finalDestination->instructionList.empty() && 
	!(dynamic_cast<IfState*>(finalDestination)) &&
	!(finalDestination->marked()))
  {
    finalDestination->markAsVisited() ;
    finalDestination = finalDestination->nextState ;
  }

  nextState = finalDestination ;
 
  // Do I have to check if there is a label associated with the destination
  //  block?  If I do this is the place to do it.

  // This section of the code is where I try to merge this state with 
  //  the destination state if the destination state is an "if" state.
  //  This is only because I think the timing should be different than
  //  the way we are handling it now.

  // The more I think about this, the more I am inclined to change 
  //  how my if statements are being generated so that this situation 
  //  never even occurs.
  //  Looking deeper at the problem, that approach will not work, due to 
  //  the fact that the state is created during parsing and the condition
  //  state that I would not create MUST be created with the way I have coded 
  //  everything so far.  To change that would mean changing EVERYTHING.
  //  So, it is back to merging the "if" state with this current state at
  //  this point.

  // I still don't like this.  This code is more like a hack of a hack
  //  and might end up causing more trouble than it is worth.  The only
  //  reason I am coding it up now is so the tool works the same as 
  //  I described in the paper.

  if (dynamic_cast<IfState*>(finalDestination) != NULL)
  {
    list<Node*>::reverse_iterator mergeIter = instructionList.rbegin();
    while (mergeIter != instructionList.rend())
    {
      finalDestination->instructionList.push_front(*mergeIter) ;
      ++mergeIter ;
    }

    instructionList.clear();
    assert(instructionList.empty());

  }

}

void State::addChildren(list<State*>& toVisit)
{
  if (nextState != NULL)
  {
    if (!(nextState->marked()))
    {
      toVisit.push_back(nextState) ;
    }
  }
}

/***************************** IfState functions *****************************/

IfState::IfState() : State() 
{
  thenBranch = NULL;
  elseBranch = NULL;
  resultRegister = "" ;
}

IfState::~IfState()
{
  ;
}

void IfState::generateVHDL(Function* t, Node* p)
{
  assert(thenBranch != NULL);
  assert(elseBranch != NULL);
  cout << "when ST" << number << " =>" << endl ;
  list<Node*>::iterator printIter = instructionList.begin() ;
  while (printIter != instructionList.end())
  {
    (*printIter)->outputInstruction() ;
    ++printIter ;
  }
  cout << "if (" << resultRegister << ") then " << endl ;
  cout << "currentState <= ST" << thenBranch->getNumber() << ";" << endl ;
  cout << "else " << endl;
  cout << "currentState <= ST" << elseBranch->getNumber() << ";" << endl ;
  cout << "end if;" << endl ;
}

void IfState::setThenBranch(State* t) 
{
  assert(t != NULL);
  thenBranch = t;
}

void IfState::setElseBranch(State* e)
{
  assert(e != NULL);
  elseBranch = e;
}

void IfState::simplify(Function* t)
{
  // Simplify the then branch and simplify the else branch

  State* finalDestination = thenBranch ;

  // Stop eliminating states if we run into a state with instructions,
  //  an if statement, or a state we have already discovered.
  while(finalDestination->instructionList.empty() && 
	!(dynamic_cast<IfState*>(finalDestination)) &&
	!(finalDestination->marked()))
  {
    finalDestination->markAsVisited() ;
    finalDestination = finalDestination->getNextState() ;
  }

  thenBranch = finalDestination ;

  // I have to clear out all of the marks

  list<State*>::iterator clearIter = t->middleStates.begin() ;
  while (clearIter != t->middleStates.end())
  {
    (*clearIter)->clearMark() ;
    ++clearIter ;
  }

  finalDestination = elseBranch ;

  while(finalDestination->instructionList.empty() && 
	!(dynamic_cast<IfState*>(finalDestination)) &&
	!(finalDestination->marked()))
  {
    finalDestination->markAsVisited() ;
    finalDestination = finalDestination->getNextState() ;
  }

  elseBranch = finalDestination ;

}

void IfState::addChildren(list<State*>& toVisit)
{
  if (thenBranch != NULL)
  {
    if (!(thenBranch->marked()))
    {
      toVisit.push_back(thenBranch) ;
    }
  }
  if (elseBranch != NULL)
  {
    if (!(elseBranch->marked()))
    {
      toVisit.push_back(elseBranch) ;
    }
  }
}
