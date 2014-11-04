
#include <iostream>
#include <fstream>
#include <assert.h>

#include "function.h"
#include "declNodes.h"
#include "program.h"

#include "suifGenerator.h"

#include <common/lstring.h>

using namespace std;

/************************* Public functions **************************/

/*

  The constructor sets initial values for all of the Function's 
   variables.  This function takes in one parameter, 
   which is used to initialize the vector of all nodes in the function.

*/
Function::Function(suif_vector<Node*>* a) : allNodes(a)
{
  myName = "unknown" ;
}

/*

  The destructor for Function reclaims all memory that has been allocated.

*/
Function::~Function()
{
  // Delete both the elements of the vector of Node pointers
  //  and the vector of node pointers itself
  if (allNodes == NULL)
  {
    return ;
  }
  for (unsigned int i = 0; i < allNodes->size(); ++i)
  {
    delete (*allNodes)[i];
  }
  delete allNodes;
}

/*
   This function connects all of the nodes.  It might be that I only
    have to connect the root, but just to make sure I will
    go through each node
*/
void Function::connectTree()
{
  assert(allNodes != NULL);
  for (unsigned int i = 0; i < allNodes->size(); ++i)
  {
     (*allNodes)[i]->connect(this);
  }
}

/*

  This function makes sure that each node removes the intermediary
   links to Option pointers and links directly to nodes or values.

*/
void Function::flatten()
{
  assert(allNodes != NULL);
  for (unsigned int i = 0; i < allNodes->size(); ++i)
  {
     (*allNodes)[i]->flatten();
  }
  // I placed this here because once the tree has been flattened, the
  //  name is accessible and I want to make sure that any other 
  //  function can see my name.
  updateName() ;
}

// This function goes through the function and retrieves all of the 
//  parameters of that function (assuming that each parm_decl
//  in that function belongs to that function, an assumption I
//  don't know is true or not in all cases).  (This assumption turned out 
//  to be incorrect.  I must come up with a different way of doing this).
// Actually a further problem is that if a parameter is not used it
//  does not appear, and parameters are not necessarily in the correct
//  order.
// I can't seem to find a difference between a parameter used in another 
//  function and a parameter used in the current function.  This makes it
//  very difficult to find arguments to the current function.

list<Symbol*> Function::findParameters(Program* p)
{

  list<Symbol*> parmList ;
  
  // Find the function declaration node that corresponds to this function
  for (unsigned int i = 0 ; i < allNodes->size() ; ++i)
  {
    if (is_a<FunctionDecl*>((*allNodes)[i]))
    {
      if (((*allNodes)[i])->resultRegister() == name())
      {
	FunctionDecl* currentFunction = 
	  dynamic_cast<FunctionDecl*>((*allNodes)[i]) ;
	assert(currentFunction != NULL) ;
	Node* argList = currentFunction->getArgs() ;
	int order = 0 ; 
	while (argList != NULL)
	{
	  ParmDecl* currentParam = dynamic_cast<ParmDecl*>(argList) ;
	  assert(currentParam != NULL) ;
	  Symbol* currentSymbol = currentParam->getVariable(p) ;
	  BrickAnnote* orderAnnote = 
	    create_brick_annote(p->converter->getEnv(), "ParameterOrder") ;
	  IntegerBrick* orderBrick = 
	    create_integer_brick(p->converter->getEnv(), IInteger(order)) ;
	  orderAnnote->append_brick(orderBrick) ;
	  currentSymbol->append_annote(orderAnnote) ;
	  parmList.push_back(currentSymbol) ;
	  argList = currentParam->getChan() ;
	  ++order ;
	}
	return parmList ;
      }
    }
  }
  /*  
  for (unsigned int i = 0; i < allNodes->size(); ++i)
  {    
    if (is_a<ParmDecl*>((*allNodes)[i]))
    {
      if (((*allNodes)[i])->getScope() != NULL)
	parmList.push_back((*allNodes)[i]->getVariable(p)) ;
    }
  }
  */
  // Just in case we don't find it.
  return parmList ;
}

// This function should only be called after the function has been flattened

QualifiedType* Function::findReturnType(Program* p)
{
  for(unsigned int i = 0 ; i < allNodes->size() ; ++i)
  {
    if (is_a<VarDecl*>((*allNodes)[i]))
    {
      Node* scpe = ((*allNodes)[i])->getScope() ;
      if (is_a<FunctionDecl*>(scpe))
      {
	return scpe->findReturnType(p);
      }
    }
    if (is_a<ParmDecl*>((*allNodes)[i]))
    {
      Node* scpe = ((*allNodes)[i])->getScope() ;
      if(scpe != NULL)
      {
	return scpe->findReturnType(p) ;
      }
    }
  }
  // I don't know, so return void
  return p->converter->globalTb->get_qualified_type(p->converter->globalTb->get_void_type()) ;
}

/************************* Private functions *************************/

// Now, I must search for a variable whose scope is a function_decl
//  node, then I can get the name from the function_decl node.
//  If that does not exist, I have no way of finding out the name of this
//  function.
//  This is a hack, I know, but I can't see any other way of doing this other
//  than if I started parsing the automatically generated comments.
void Function::updateName()
{
  for(unsigned int i = 0 ; i < allNodes->size() ; ++i)
  {
    if (is_a<VarDecl*>((*allNodes)[i]))
    {
      Node* scpe = ((*allNodes)[i])->getScope() ;
      if (is_a<FunctionDecl*>(scpe))
      {
	myName = scpe->resultRegister() ;
	break ;
      }
    }
    if (is_a<ParmDecl*>((*allNodes)[i]))
    {
      Node* scpe = ((*allNodes)[i])->getScope() ;
      if (scpe != NULL)
      {
	myName = scpe->resultRegister() ;
	break ;
      }
    }
  }
}

LString Function::name()
{
  return myName ;
}

void Function::generateSuif(Program* p)
{
  // Go through starting at the root and generate Suif

  assert(allNodes != NULL) ;  
  assert(!(allNodes->empty())) ;

  Statement* result = 
    dynamic_cast<Statement*>((*allNodes)[0]->generateSuif(p));

  if (result == NULL)
    return ;

  //  assert(result != NULL) ;

  p->converter->appendStatementToMainBody(result) ;

}

/*
ProcedureDefinition* Function::generateSuif(Program* p)
{

  // All of the statements in this function belong to a statement list
  procedureBody = p->getObjFactory()->create_statement_list() ;

  // For this to work I have to come up with the type of the
  //  function represented by this tree.  This is now only available in
  //  the comment at the top of the tree, but I might be able to
  //  find it if it exists in the tree (no guarantee on that though).
  //  Worse, if I find a function_decl node simply by searching all
  //  the nodes, I might end up finding one that was a function call and not 
  //  the definition!  The only way I can be sure is to find a var_decl 
  //  that happens to have a scope associated with it that happens to be 
  //  a function decl node (Which is very ugly, but probably doable).
  //  If I can't find something that matches this template, I will just
  //  make a generic function name and type.

  Node* functionDefinition = NULL ;
  
  for(unsigned int i = 0 ; i < allNodes->size(); ++i)
  {
    if (is_a<VarDecl*>((*allNodes)[i]) || is_a<ParmDecl*>((*allNodes)[i]))
    {

      functionDefinition = ((*allNodes)[i])->getScope() ;
      if (is_a<FunctionDecl*>(functionDefinition))
      {
	break ;
      }
      else
      {
	functionDefinition = NULL ;
      }      
    }
  }

  LString functionName ;
  list<QualifiedType*> arguments ;
  CProcedureType* procType ;

  if (functionDefinition == NULL)
  {
    // We must use the default
    functionName = "UnknownFunction" ;
    // There are no arguments that we know of, so don't add anything to 
    //  the list of qualified types
    
  }
  else
  {
    // We can figure out all information from this pointer (hopefully)
    functionName = functionDefinition->resultRegister() ;
  }

  // Create the procedure symbol
  ProcedureSymbol* procSym = 
    p->getObjFactory()->create_procedure_symbol(procType, functionName, true) ;


  return NULL ;

}
*/
