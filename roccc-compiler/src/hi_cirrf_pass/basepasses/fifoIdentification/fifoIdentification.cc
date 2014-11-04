// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/*

  This pass has to go after something that identifies for loop info...

  This has to be a ROCCC 2.0 pass that is generating a system.  Modules 
    cannot have arrays or streams of data.

  3/24/2009 - Started restructuring to account for both 1 and 2 dimensional
              fifos/smart buffers.  Also, detected input and output buffers.

*/

#include <cassert>
#include <sstream>
#include <iostream>

#include <suifkernel/utilities.h>
#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif_factory.h>
#include <typebuilder/type_builder.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"
#include "fifoIdentification.h"

FifoIdentificationPass::FifoIdentificationPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "FifoIdentification")
{
  theEnv = pEnv ;
  tempNumber = 0 ;
  fifosOnly = false ;
}

std::string FifoIdentificationPass::tempName()
{
  ++tempNumber ;
  
  std::stringstream concat ;
  concat << "suifTmp" << tempNumber ;
  return concat.str() ;
}

// This function exists to take an expression that is used in index into an
//  array and determine the index variable that is being used
VariableSymbol* FifoIdentificationPass::DetermineIndex(Expression* e)
{
  // This expression should be either be a variable expression or a binary
  //  expression, everything else is not supported
  if (dynamic_cast<LoadVariableExpression*>(e) != NULL)
  {
    return dynamic_cast<LoadVariableExpression*>(e)->get_source() ;
  }
  else if (dynamic_cast<NonLvalueExpression*>(e) != NULL)
  {
    NonLvalueExpression* tmp = dynamic_cast<NonLvalueExpression*>(e) ;
    assert(dynamic_cast<LoadVariableExpression*>(tmp->get_addressed_expression()) != NULL) ;
    return dynamic_cast<LoadVariableExpression*>(tmp->get_addressed_expression())->get_source() ;
  }
  else if (dynamic_cast<BinaryExpression*>(e) != NULL)
  {
    // One of the sources should be a constant integer and the other should
    //  be a LoadVariableExpression.  Determine which is which and return
    //  the LoadVariableExpression

    BinaryExpression* binExp = dynamic_cast<BinaryExpression*>(e) ;
    Expression* leftExp = binExp->get_source1() ;
    Expression* rightExp = binExp->get_source2() ;

    if (binExp->get_opcode() != LString("add") &&
	binExp->get_opcode() != LString("subtract"))
    {
      assert(0 && "Inappropriate array access!") ;
    }
    
    if (dynamic_cast<LoadVariableExpression*>(leftExp) != NULL)
    {
      return dynamic_cast<LoadVariableExpression*>(leftExp)->get_source() ;
    }
    if (dynamic_cast<LoadVariableExpression*>(rightExp) != NULL)
    {
      return dynamic_cast<LoadVariableExpression*>(rightExp)->get_source() ;
    }
    // Somehow, we aren't accessing the index variable.  This must have 
    //  happened because we have a complex array access that is not supported.
    assert(0 && "Inappropriate array access!") ; 
  }
  else if (dynamic_cast<Constant*>(e) != NULL)
  {
    assert(0) ;
  }
  
  else
  {
    assert(0) ;
  }
  return NULL ; // To avoid any compiler warnings
}

// This function exists to take an indexing expression and return the
//  constant offset (either positive or negative) from the index variable
//  and return it
Constant* FifoIdentificationPass::DetermineOffset(Expression* e)
{
  if (dynamic_cast<LoadVariableExpression*>(e) != NULL || 
      dynamic_cast<NonLvalueExpression*>(e) != NULL)
  {
    // The offset is zero, so create a constant of zero and return it.
    IntConstant* zeroConst = create_int_constant(get_suif_env(), 
				     create_integer_type(get_suif_env(), 32, 32, true), 0) ;
    return zeroConst ;
  }
  else if (dynamic_cast<BinaryExpression*>(e) != NULL)
  {
    // One of these should be an integer constant and the other
    //  should be a load variable expression.  We must find the integer
    //  constant.
    BinaryExpression* binExp = dynamic_cast<BinaryExpression*>(e) ;
    Expression* leftExp = binExp->get_source1() ;
    Expression* rightExp = binExp->get_source2() ;

    if (binExp->get_opcode() != LString("add") &&
	binExp->get_opcode() != LString("subtract")) 
    {
      assert(0 && "Incorrectly formed array access!") ;
    }
        
    if (dynamic_cast<IntConstant*>(leftExp) != NULL)
    {
      IntConstant* toReturn = dynamic_cast<IntConstant*>(leftExp) ;
      if (dynamic_cast<LoadVariableExpression*>(rightExp) == NULL)
      {
	OutputError("Incorrectly formed array access!") ;
	assert(0) ;
      }
      if (binExp->get_opcode() == LString("subtract"))
      {
	OutputError("Incorrectly formed array access!") ;
	assert(0) ;
      }
      return toReturn ;
    }
    if (dynamic_cast<IntConstant*>(rightExp) != NULL)
    {
      IntConstant* toReturn = dynamic_cast<IntConstant*>(rightExp) ;
      if (dynamic_cast<LoadVariableExpression*>(leftExp) == NULL)
      {
	OutputError("Incorrectly formed array access!") ;
	assert(0) ;
      }
      if (binExp->get_opcode() == LString("subtract"))
      {
	// We need to flip the offset and change this to an addition.
	toReturn->set_value(IInteger(-(toReturn->get_value().c_int()))) ;
	binExp->set_opcode(LString("add")) ;
      }
      return toReturn ;
    }

    // This binary expression is not well formed
    assert(0 && "Incorrectly formed array access") ;
  }
  else if (dynamic_cast<ArrayReferenceExpression*>(e) != NULL)
  {
    // This is a two or more dimensional array.  We need to do something
    //  different
    assert(0) ;
  }
  else if (dynamic_cast<Constant*>(e) != NULL)
  {
    // The offset is a constant value, but for some reason it 
    //  wasn't propagated or scalar replaced (right now due 
    //  to multidimensionality) 
    return dynamic_cast<Constant*>(e) ;
  }
  else
  {
    // This is not yet supported
    FormattedText tmpText  ; 
    e->print(tmpText) ;
    std::cerr << "Checking symbol " << tmpText.get_value() << std::endl ;
    assert(0) ;
  }
  return NULL ; // Just to avoid any compiler warnings
}

void FifoIdentificationPass::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;

  OutputInformation("Fifo Identification begins") ;

  if (procDef->lookup_annote_by_name("ComposedSystem") != NULL)
  {
    HandleComposedSystems() ;
    OutputInformation("Fifo Identification ends") ;
    return ;
  }

  // We have nothing, so fifos only is true.  This might be changed in 
  //  some of the other member functions.
  fifosOnly = true ;

  // Fifos and smart buffers should only exist in the innermost loop
  CForStatement* innermost = InnermostLoop(procDef) ;
  if (innermost == NULL)
  {
    OutputInformation("Fifo Identification ends") ;
    return ;
  }

  IdentifyInputFifos(innermost) ;
  IdentifyOutputFifos(innermost) ;

  if (fifosOnly == true)
  {
    procDef->append_annote(create_brick_annote(theEnv, 
					       "FifosOnly"));
  }

  OutputInformation("Fifo Identification ends") ;
}

void FifoIdentificationPass::HandleComposedSystems()
{
  assert(procDef != NULL) ;
  SymbolTable* symTab = procDef->get_symbol_table() ;
  assert(symTab != NULL) ;
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* currentSymbol = symTab->get_symbol_table_object(i) ;
    ParameterSymbol* currentParam = 
      dynamic_cast<ParameterSymbol*>(currentSymbol) ;
    if (currentParam != NULL)
    {
      DataType* currentType = currentParam->get_type()->get_base_type() ;
      if (dynamic_cast<PointerType*>(currentType) != NULL)
      {
	currentParam->append_annote(create_brick_annote(theEnv, "InputFifo")) ;
	currentParam->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;
      }
      else if (dynamic_cast<ReferenceType*>(currentType) != NULL)
      {
	currentParam->append_annote(create_brick_annote(theEnv, "OutputFifo"));
	currentParam->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;
      }
    }
  }
}

// We will add to each array symbol the annotations that state if it
//  is an input fifo and what dimensionality it is.
void FifoIdentificationPass::IdentifyInputFifos(CForStatement* c)
{
  // First, collect all load expressions.  Since we've already run
  //  scalar replacement, these should all be located at the top of the
  //  for loop and we don't have to worry about creating any more
  //  replacements.

  list<LoadExpression*>* loadExpList ;
  loadExpList = collect_objects<LoadExpression>(c) ;

  assert(loadExpList != NULL) ;

  list<LoadExpression*>::iterator replaceIt ;
  replaceIt = loadExpList->begin() ;
  while (replaceIt != loadExpList->end())
  {
    // Check to see if the load expression is accessing an array reference
    //  or a straight pointer.  We should never see a straight pointer, but
    //  in the future this might be the case.
    Expression* loadRef = (*replaceIt)->get_source_address() ;
    if (dynamic_cast<ArrayReferenceExpression*>(loadRef) == NULL)
    {
      std::cerr << "Pointers not supported!" << std::endl ;
      assert(0) ;
    }

    ArrayReferenceExpression* topLevel ;
    topLevel = dynamic_cast<ArrayReferenceExpression*>(loadRef) ;

    // Make sure we are not looking at a previously identified lookup table
    if (GetArrayVariable(topLevel)->lookup_annote_by_name("LUT") != NULL)
    {
      ++replaceIt ;
      continue ;
    }

    // Find the symbol table object associated with this array reference
    //  and find the InputFifo annotation.  If the InputFifo annotation
    //  does not exist, then add it.
    Expression* baseAddress = topLevel->get_base_array_address() ;
    int dimensionality = 1 ;

    // The base address is either a symbol address expression, or
    //  another array reference expression
    // Added by Jason on 4/6/09
    while(dynamic_cast<ArrayReferenceExpression*>(baseAddress) != NULL)
    {
      baseAddress = dynamic_cast<ArrayReferenceExpression*>(baseAddress)->get_base_array_address() ;
      ++dimensionality ;
    }

    SymbolAddressExpression* symbolAddress ;
    symbolAddress = dynamic_cast<SymbolAddressExpression*>(baseAddress) ;
    assert(symbolAddress != NULL) ;
    Symbol* baseSymbol = symbolAddress->get_addressed_symbol() ; 
    BrickAnnote* inputFifoMark ;
    inputFifoMark = dynamic_cast<BrickAnnote*>(baseSymbol->lookup_annote_by_name("InputFifo")) ;
    if (inputFifoMark == NULL)
    {
      inputFifoMark = create_brick_annote(theEnv, "InputFifo") ;
      baseSymbol->append_annote(inputFifoMark) ;
      // Add a brick that states what the index variables are
      ArrayReferenceExpression* currentIndex = topLevel ;
      for(int k = 0 ; k < dimensionality; ++k)
      {
	SuifObjectBrick* indexVariableAnnote ;
	indexVariableAnnote = create_suif_object_brick(theEnv,
						       DetermineIndex(currentIndex->get_index())) ;
	inputFifoMark->append_brick(indexVariableAnnote) ;
	if (dynamic_cast<ArrayReferenceExpression*>(currentIndex->get_base_array_address()) != NULL)
	{
	  currentIndex = dynamic_cast<ArrayReferenceExpression*>(currentIndex->get_base_array_address()) ;
	}
      }
    }

    // Add an annotation to keep track of the dimensionality
    BrickAnnote* dimAnnote ;
    dimAnnote = dynamic_cast<BrickAnnote*>(baseSymbol->lookup_annote_by_name("DimensionAnnote")) ;
    if (dimAnnote == NULL)
    {
      dimAnnote = create_brick_annote(theEnv, "DimensionAnnote") ;
      baseSymbol->append_annote(dimAnnote) ;
      // Now add the integer brick that holds the dimensionalilty.

      IntegerBrick* dimValue ;
      dimValue = create_integer_brick(get_suif_env(), IInteger(dimensionality)) ;
      dimAnnote->append_brick(dimValue) ;
    }

    // Add an annotation that states what the index offset is and the
    //  variable that is associated with it
    BrickAnnote* indexAnnote ;
    indexAnnote = dynamic_cast<BrickAnnote*>(baseSymbol->lookup_annote_by_name("IndexAnnote")) ;
    if (indexAnnote == NULL)
    {
      indexAnnote = create_brick_annote(theEnv, "IndexAnnote") ;
      baseSymbol->append_annote(indexAnnote) ;
    }

    // I need to add the variable symbol of the suifTmp variable, which
    //  should be the destination of this load expression.  

    ExecutionObject* scalarReplacedVariable = (*replaceIt)->get_destination() ;
    assert(scalarReplacedVariable != NULL) ;
    assert(dynamic_cast<Statement*>(scalarReplacedVariable) != NULL) ;
    assert(dynamic_cast<StoreVariableStatement*>(scalarReplacedVariable) != NULL) ;
    StoreVariableStatement* scalarReplaceStatement ;
    scalarReplaceStatement = dynamic_cast<StoreVariableStatement*>(scalarReplacedVariable) ;
    
    SuifObjectBrick* replacementVarBrick ;
    replacementVarBrick = create_suif_object_brick(theEnv,
						   scalarReplaceStatement->get_destination()) ;

    indexAnnote->append_brick(replacementVarBrick) ;

    // Add the variable symbols of all the offset variables.
    //  There should be as many as the dimensionality.
    ArrayReferenceExpression* currentOffset = topLevel ;
    for(int i = 0 ; i < dimensionality; ++i)
    {
      // Find the i'th index variable.  Add them to the indexAnnote
      SuifObjectBrick* indexExpressionBrick ;
      indexExpressionBrick = create_suif_object_brick(theEnv,
						      DetermineOffset(currentOffset->get_index())) ;
      indexAnnote->append_brick(indexExpressionBrick) ;
      if (dynamic_cast<ArrayReferenceExpression*>(currentOffset->get_base_array_address()) != NULL)
      {
	currentOffset = dynamic_cast<ArrayReferenceExpression*>(currentOffset->get_base_array_address()) ;
      } 
    }    

    // Now, we must remove the original load expression.
    //  Each load expression is stored inside of a statement.
    SuifObject* parent = (*replaceIt)->get_parent() ;
    assert(parent != NULL) ;
    Statement* parentStatement = dynamic_cast<Statement*>(parent) ;
    assert(parentStatement != NULL) ;
    // Now get the statement list parent of this statement
    SuifObject* parentParent = parentStatement->get_parent() ;
    assert(parentParent != NULL) ;
    StatementList* parentList = dynamic_cast<StatementList*>(parentParent) ;
    assert(parentList != NULL) ;

    for(int i = 0 ; i < parentList->get_statement_count() ; ++i)
    {
      if (parent == parentList->get_statement(i))
      {
	parentList->remove_statement(i) ;
	break ;
      }
    }

    // Finally, check to see if this is a smart buffer in addition to 
    //  an input fifo.
    LabelSmartBuffers(c, baseSymbol) ;

    ++replaceIt ;
  }

  delete loadExpList ;
}

void FifoIdentificationPass::IdentifyOutputFifos(CForStatement* c) 
{
  // Identify output fifos in store instructions
  //  This should work almost exactly the same as the identification of 
  //  input fifos, with the exception of us collecting store instructions 
  //  as opposed to load instructions.
  list<StoreStatement*>* storeStmtList ;
  storeStmtList = collect_objects<StoreStatement>(c) ;
  assert(storeStmtList != NULL) ;

  list<StoreStatement*>::iterator replaceIt = storeStmtList->begin() ;
  while(replaceIt != storeStmtList->end())
  {
    Expression* leftHandSide = (*replaceIt)->get_destination_address() ;
    Expression* rightHandSide = (*replaceIt)->get_value() ;
    assert(rightHandSide != NULL) ;

    // Just like the load statements, we should not be dealing with any
    //  pointers.
    if (dynamic_cast<ArrayReferenceExpression*>(leftHandSide) == NULL)
    {
      std::cerr << "Pointers not yet supported" << std::endl ;
      assert(0) ;
    }

    ArrayReferenceExpression* topLevel ;
    topLevel = dynamic_cast<ArrayReferenceExpression*>(leftHandSide) ;

    // Make sure we are not working with a previously identified lookup table
    if (GetArrayVariable(topLevel)->lookup_annote_by_name("LUT") != NULL)
    {
      ++replaceIt ;
      continue ;
    }

    // Find the symbol associated with the array we are storing into.
    Expression* baseAddress = topLevel->get_base_array_address() ;
    int dimensionality = 1 ;
    while (dynamic_cast<ArrayReferenceExpression*>(baseAddress) != NULL)
    {
      baseAddress = dynamic_cast<ArrayReferenceExpression*>(baseAddress)->get_base_array_address() ;
      ++dimensionality ;
    }

    SymbolAddressExpression* symbolAddress ;
    symbolAddress = dynamic_cast<SymbolAddressExpression*>(baseAddress) ;
    assert(symbolAddress != NULL) ;
    Symbol* baseSymbol = symbolAddress->get_addressed_symbol() ; 

    // I'm going to see if there is an input fifo mark.  There cannot
    //  be one unless we are doing systolic array, which is done
    //  somewhere else.
    if (baseSymbol->lookup_annote_by_name("InputFifo") != NULL)
    {
      std::cerr << "We do not support reading and writing from the same "
		<< "memory unless performing systolic array!" 
		<< std::endl ;
      assert(0) ;
    }

    // Search for the OutputFifo mark.  If it doesn't exist, then add it
    BrickAnnote* outputFifoMark ;
    outputFifoMark = dynamic_cast<BrickAnnote*>(baseSymbol->lookup_annote_by_name("OutputFifo")) ;
    if (outputFifoMark == NULL)
    {
      outputFifoMark = create_brick_annote(theEnv, "OutputFifo") ;
      baseSymbol->append_annote(outputFifoMark) ;
      // Add a brick that states what the index variables are
      ArrayReferenceExpression* currentIndex = topLevel ;
      for(int k = 0 ; k < dimensionality; ++k)
      {
	SuifObjectBrick* indexVariableAnnote ;
	indexVariableAnnote = create_suif_object_brick(theEnv,
						       DetermineIndex(currentIndex->get_index())) ;
	outputFifoMark->append_brick(indexVariableAnnote) ;
	if (dynamic_cast<ArrayReferenceExpression*>(currentIndex->get_base_array_address()) != NULL)
	{
	  currentIndex = dynamic_cast<ArrayReferenceExpression*>(currentIndex->get_base_array_address()) ;
	}
      }
    }

    // Add an annotation to keep track of the dimensionality
    BrickAnnote* dimAnnote ;
    dimAnnote = dynamic_cast<BrickAnnote*>(baseSymbol->lookup_annote_by_name("DimensionAnnote")) ;
    if (dimAnnote == NULL)
    {
      dimAnnote = create_brick_annote(theEnv, "DimensionAnnote") ;
      baseSymbol->append_annote(dimAnnote) ;
      IntegerBrick* dimValue ;
      dimValue = create_integer_brick(get_suif_env(), IInteger(dimensionality));
      dimAnnote->append_brick(dimValue) ;
    }

    // Add an annotation that states what the index offset is and the
    //  variable that is associated with it
    BrickAnnote* indexAnnote ;
    indexAnnote = dynamic_cast<BrickAnnote*>(baseSymbol->lookup_annote_by_name("IndexAnnote")) ;
    if (indexAnnote == NULL)
    {
      indexAnnote = create_brick_annote(theEnv, "IndexAnnote") ;
      baseSymbol->append_annote(indexAnnote) ;
    }
    // I need to add the variable symbol of the suifTmp variable, which
    //  should be the destination of this load expression.  

    LoadVariableExpression* scalarReplacedExpression ;
    scalarReplacedExpression = 
      dynamic_cast<LoadVariableExpression*>(rightHandSide) ;
    SuifObjectBrick* replacementVarBrick ;
    assert(scalarReplacedExpression != NULL) ;
    replacementVarBrick = 
      create_suif_object_brick(theEnv,
			       scalarReplacedExpression->get_source()) ;    
    
    indexAnnote->append_brick(replacementVarBrick) ;

    // Determine all of the offset values
    //  There should be as many as the dimensionality
    ArrayReferenceExpression* currentOffset = topLevel ;
    for(int k = 0 ; k < dimensionality; ++k)
    {
      // For the i'th index variable.  Add them to the index Annote
      SuifObjectBrick* indexExpressionBrick ;
      indexExpressionBrick = create_suif_object_brick(theEnv, 
						      DetermineOffset(currentOffset->get_index()));
      indexAnnote->append_brick(indexExpressionBrick) ;
      if (dynamic_cast<ArrayReferenceExpression*>(currentOffset->get_base_array_address()) != NULL)
      {
	currentOffset = dynamic_cast<ArrayReferenceExpression*>(currentOffset->get_base_array_address()) ;
      }
    }

    // Now, we must remove the original store statement
    SuifObject* parent = (*replaceIt)->get_parent() ;
    assert(parent != NULL) ;
    StatementList* parentList = dynamic_cast<StatementList*>(parent) ;
    assert(parentList != NULL) ;

    for(int i = 0 ; i < parentList->get_statement_count() ; ++i)
    {
      if ((*replaceIt) == parentList->get_statement(i))
      {
	parentList->remove_statement(i) ;
	break ;
      }
    }

    ++replaceIt ;
  }  
  delete storeStmtList ;
}



// This function takes an array and the associated C for loop that it
//  was found in and checks to see if there is any reuse.  If so, then
//  that array is automatically a smart buffer.

// Currently, this is an N squared algorithm, but it shouldn't be too
//  horrible (at least for moderately sized systems).  This can be sped 
//  up later.
// Also, this only is going to work with one dimensional arrays for the 
//  time being.
void FifoIdentificationPass::LabelSmartBuffers(CForStatement* c, Symbol* array)
{

  // Have we already checked this array?  If so don't do it again.
  Annote* smartBufferAnnote = array->lookup_annote_by_name("isSmartBuffer") ;
  if (smartBufferAnnote != NULL)
  {
    return ;
  }

  // In order to determine reuse, I need to collect the step size of this
  //  loop.  This should have been determined in the LoopInfo pass, so
  //  all I need to do is read the annotation.
  BrickAnnote* loopInfo = to<BrickAnnote>(c->lookup_annote_by_name("c_for_info")) ;
  assert(loopInfo != NULL) ;
  int stepSize = to<IntegerBrick>(loopInfo->get_brick(loopInfo->get_brick_count() - 1))->get_value().c_int() ;

  // Now, I need to find the largest and smallest offset and use that
  //  to normalize.  If the normalized largest value is >= the step
  //  size then we have a reuse situation.  If not, then we have a fifo.
  int largestOffset = 0 ;
  int smallestOffset = 0 ;

  list<LoadExpression*>* allLoads =
    collect_objects<LoadExpression>(c) ;
  assert(allLoads != NULL) ;

  Symbol* currentSymbol ;
  
  list<LoadExpression*>::iterator loadIter = allLoads->begin() ;
  while (loadIter != allLoads->end())
  {
    // Check to see if this load expression is for the correct array
    ArrayReferenceExpression* loadRef = 
      dynamic_cast<ArrayReferenceExpression*>((*loadIter)->get_source_address()) ;
    assert(loadRef != NULL) ;

    // For N-dimensional arrays, we need to recurse
    while (dynamic_cast<ArrayReferenceExpression*>(loadRef->get_base_array_address()) != NULL)
    {
      loadRef = dynamic_cast<ArrayReferenceExpression*>(loadRef->get_base_array_address()) ;
    }

    SymbolAddressExpression* symAddr = 
      dynamic_cast<SymbolAddressExpression*>(loadRef->get_base_array_address());
    assert(symAddr != NULL) ;
    currentSymbol = symAddr->get_addressed_symbol() ;    
    if (currentSymbol != array)
    {
      ++loadIter ;
      continue ;
    }

    // Now we know we are looking at the correct array, so find the offset
    Constant* ourOffset = DetermineOffset(loadRef->get_index()) ;
    IntConstant* ourIntOffset = dynamic_cast<IntConstant*>(ourOffset) ;
    assert(ourIntOffset != NULL) ;

    int ourOffsetValue = ourIntOffset->get_value().c_int() ;

    // Now set the biggest and smallest
    if (ourOffsetValue > largestOffset)
    {
      largestOffset = ourOffsetValue ;
    }
    if (ourOffsetValue < smallestOffset)
    {
      smallestOffset = ourOffsetValue ;
    }

    ++loadIter ;
  }

  delete allLoads ;

  // Now we should have all the information we need.
  //  Perform the normalization (adding one because of index 0..
  //  the hidden index!)
  if (smallestOffset < 0)
  {
    largestOffset = largestOffset + (-smallestOffset) + 1 ;
  }
  else
  {
    largestOffset = largestOffset - smallestOffset + 1 ;
  }

  // Add the annotation to the array Symbol
  BrickAnnote* bufferAnnotation = create_brick_annote(get_suif_env(),
						      "isSmartBuffer") ;
  IntegerBrick* annoteValue ;
  if (largestOffset > stepSize)
  {
    // This one is a smart buffer
    annoteValue = create_integer_brick(get_suif_env(), IInteger(1)) ;
    // Also, set fifosOnly to false
    fifosOnly = false ;
  }
  else
  {
    annoteValue = create_integer_brick(get_suif_env(), IInteger(0)) ;
  }
  bufferAnnotation->append_brick(annoteValue) ;
  array->append_annote(bufferAnnotation) ;

  /*
  // Now the actual meat of the algorithm
  // First, collect all load expressions

  list<LoadExpression*>* allLoads ;
  allLoads = collect_objects<LoadExpression>(c) ;
  assert(allLoads != NULL) ;

  list<LoadExpression*>::iterator currentReference ;
  list<LoadExpression*>::iterator compareReference ;
  Symbol* currentSymbol ;
  bool isSmartBuffer = false ;

  for(currentReference = allLoads->begin() ;
      currentReference != allLoads->end() ;
      ++currentReference)
  {

    // First, check to see if this is an access to the correct array.
    ArrayReferenceExpression* loadRef = 
      dynamic_cast<ArrayReferenceExpression*>((*currentReference)->get_source_address()) ;
    assert(loadRef != NULL) ;

    // For N-dimensional arrays, we need to recurse
    while (dynamic_cast<ArrayReferenceExpression*>(loadRef->get_base_array_address()) != NULL)
    {
      loadRef = dynamic_cast<ArrayReferenceExpression*>(loadRef->get_base_array_address()) ;
    }

    SymbolAddressExpression* symAddr = 
      dynamic_cast<SymbolAddressExpression*>(loadRef->get_base_array_address());
    assert(symAddr != NULL) ;
    currentSymbol = symAddr->get_addressed_symbol() ;    
    if (currentSymbol != array)
    {
      continue ;
    }

    // It is the correct symbol, so check all of the other references.
    //  First, though, find our offset.
    Constant* ourOffset = DetermineOffset(loadRef->get_index()) ;
    IntConstant* ourIntOffset = dynamic_cast<IntConstant*>(ourOffset) ;
    assert(ourIntOffset != NULL) ;

    int ourOffsetValue = ourIntOffset->get_value().c_int() ;
    
    // This doesn't actually check if each other reference is to the 
    //  appropriate array.  That needs to be added.
    compareReference = currentReference ;
    ++compareReference ;
    while(compareReference != allLoads->end())
    {
      ArrayReferenceExpression* otherLoadRef = 
	dynamic_cast<ArrayReferenceExpression*>((*compareReference)->get_source_address()) ;
      assert(otherLoadRef != NULL) ;

      Constant* otherOffset = DetermineOffset(otherLoadRef->get_index()) ;
      IntConstant* otherIntOffset = dynamic_cast<IntConstant*>(otherOffset) ;
      assert(otherIntOffset != NULL) ;

      int otherOffsetValue = otherIntOffset->get_value().c_int() ;

      
      if (ourOffsetValue > otherOffsetValue)
      {
	if (ourOffsetValue - otherOffsetValue > stepSize)
	{
	  isSmartBuffer = true ;
	}
      }
      else
      {
	if (otherOffsetValue - ourOffsetValue > stepSize) 
	{
	  isSmartBuffer = true ;
	}
      }

      ++compareReference ;
    }

  }

  // Add the annotation to the array Symbol
  BrickAnnote* bufferAnnotation = create_brick_annote(get_suif_env(),
						      "isSmartBuffer") ;
  IntegerBrick* annoteValue ;
  if (isSmartBuffer)
  {
    annoteValue = create_integer_brick(get_suif_env(), IInteger(1)) ;
  }
  else
  {
    annoteValue = create_integer_brick(get_suif_env(), IInteger(0)) ;
  }
  bufferAnnotation->append_brick(annoteValue) ;
  array->append_annote(bufferAnnotation) ;

  delete allLoads ;
  */
}

