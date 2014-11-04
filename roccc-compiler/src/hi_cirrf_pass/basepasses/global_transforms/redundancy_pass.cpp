// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details. 

#include <cassert>

#include <suifkernel/command_line_parsing.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif_factory.h>
#include <typebuilder/type_builder.h>
#include <suifkernel/utilities.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

#include "redundancy_pass.h"

RedundancyPass::RedundancyPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "RedundancyPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
  tripleVoteAddress = NULL ;
}

void RedundancyPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;
  OutputInformation("Redundancy Pass Begins") ;

  StatementList* bodyList = InnermostList(procDef) ;
  assert(bodyList != NULL) ;

  CreateVoteProcedures() ;
  
  // Find all label location statements.
  list<LabelLocationStatement*>* allLabels =
    collect_objects<LabelLocationStatement>(bodyList) ;
  list<LabelLocationStatement*>::iterator labelIter = allLabels->begin() ;
  while (labelIter != allLabels->end())
  {
    Statement* next = NextStatement(*labelIter) ;
    CallStatement* c = dynamic_cast<CallStatement*>(next) ;
    if (c != NULL && (*labelIter)->lookup_annote_by_name("Redundant") != NULL) 
    {
      ClearList() ;
      BrickAnnote* redundantBrick = 
	dynamic_cast<BrickAnnote*>((*labelIter)->lookup_annote_by_name("Redundant")) ;
      assert(redundantBrick != NULL) ;
      IntegerBrick* iBrick = 
	dynamic_cast<IntegerBrick*>(redundantBrick->get_brick(0)) ; 
      int doubleOrTriple = iBrick->get_value().c_int() ;
      switch(doubleOrTriple)
      {
      case 0:
	DoubleRedundify(c) ;
	break ;
      case 1:
	TripleRedundify(c) ;
	break ;
      default:
	assert(0 && "Unknown type of redundancy requested") ;
      }
    }
    ++labelIter ;
  }
  delete allLabels ;
  
  OutputInformation("Redundancy Pass Ends") ;
}

void RedundancyPass::DoubleRedundify(CallStatement* s)
{
  assert(procDef != NULL) ;
  assert(s != NULL) ;

  // ---------------
  // Create two call statments to replace the original call
  // ---------------

  CallStatement* callCopy1 =
    create_call_statement(theEnv,
			  NULL,
	   dynamic_cast<Expression*>(s->get_callee_address()->deep_clone())) ;
  CallStatement* callCopy2 =
    create_call_statement(theEnv,
			  NULL,
	   dynamic_cast<Expression*>(s->get_callee_address()->deep_clone())) ;

  // ---------------
  // Go through all the variables and create copies for each output variable
  // ---------------

  // Start with the destination
  if (s->get_destination() != NULL)
  {
    VariableSymbol* originalDest = s->get_destination() ;
    LString copyName = originalDest->get_name() ;
    copyName = copyName + LString("_TMR") ;
    VariableSymbol* destCopy1 = 
      create_variable_symbol(theEnv, originalDest->get_type(), 
			     TempName(copyName)) ;
    VariableSymbol* destCopy2 =
      create_variable_symbol(theEnv, originalDest->get_type(),
			     TempName(copyName)) ;

    procDef->get_symbol_table()->append_symbol_table_object(destCopy1) ;
    procDef->get_symbol_table()->append_symbol_table_object(destCopy2) ;

    callCopy1->set_destination(destCopy1) ;
    callCopy2->set_destination(destCopy2) ;
    
    RedundantGroup toAdd ;
    toAdd.input1 = destCopy1 ;
    toAdd.input2 = destCopy2 ;
    toAdd.input3 = NULL ;
    toAdd.output1 = originalDest ;
    toAdd.output2 = NULL ;
    toAdd.output3 = NULL ;
    voteCalls.push_back(toAdd) ;    
  }
  
  // Now go through each of the arguments
  for (int i = 0 ; i < s->get_argument_count() ; ++i)
  {
    Expression* currentArg = s->get_argument(i) ;
    if (!IsOutputVariable(currentArg))
    {
      if (IsStream(currentArg))
      {
	VariableSymbol* currentVar = NULL ;
	LoadVariableExpression* inExpr =
	  dynamic_cast<LoadVariableExpression*>(currentArg) ;
	assert(inExpr != NULL) ;
	currentVar = inExpr->get_source() ;
	CallStatement* splitCall = CreateDoubleSplitter(currentVar) ;

	// Insert this call statement right before this statement
	StatementList* parentList = 
	  dynamic_cast<StatementList*>(s->get_parent()) ;
	assert(parentList != NULL) ;
	AddChildStatementBefore(parentList, splitCall, s) ;

	SymbolAddressExpression* streamCopy1 = 
	  dynamic_cast<SymbolAddressExpression*>(splitCall->get_argument(1)) ;
	SymbolAddressExpression* streamCopy2 = 
	  dynamic_cast<SymbolAddressExpression*>(splitCall->get_argument(2)) ;

	assert(streamCopy1 != NULL) ;
	assert(streamCopy2 != NULL) ;

	VariableSymbol* varCopy1 =
	  dynamic_cast<VariableSymbol*>(streamCopy1->get_addressed_symbol());
	VariableSymbol* varCopy2 =
	  dynamic_cast<VariableSymbol*>(streamCopy2->get_addressed_symbol());

	assert(varCopy1 != NULL) ;
	assert(varCopy2 != NULL) ;

	LoadVariableExpression* loadCopy1 =
	  create_load_variable_expression(theEnv,
				  varCopy1->get_type()->get_base_type(),
				  varCopy1) ;

	LoadVariableExpression* loadCopy2 =
	  create_load_variable_expression(theEnv,
				  varCopy2->get_type()->get_base_type(),
				  varCopy2) ;

	callCopy1->append_argument(loadCopy1) ;
	callCopy2->append_argument(loadCopy2) ;
      }
      else
      {
	callCopy1->
	  append_argument(dynamic_cast<Expression*>(currentArg->deep_clone()));
	callCopy2->
	  append_argument(dynamic_cast<Expression*>(currentArg->deep_clone()));
      }
    }
    else
    {
      VariableSymbol* currentOutput = GetOutputVariable(currentArg) ;
      LString copyName = currentOutput->get_name() ;
      copyName = copyName + LString("_TMR") ;
      VariableSymbol* outputCopy1 =
	create_variable_symbol(theEnv,
			       currentOutput->get_type(),
			       TempName(copyName)) ;
      VariableSymbol* outputCopy2 =
	create_variable_symbol(theEnv,
			       currentOutput->get_type(),
			       TempName(copyName)) ;

      procDef->get_symbol_table()->append_symbol_table_object(outputCopy1) ;
      procDef->get_symbol_table()->append_symbol_table_object(outputCopy2) ;

      SymbolAddressExpression* outputExpression1 =
	create_symbol_address_expression(theEnv,
				     outputCopy1->get_type()->get_base_type(),
				     outputCopy1) ;

      SymbolAddressExpression* outputExpression2 =
	create_symbol_address_expression(theEnv,
				     outputCopy2->get_type()->get_base_type(),
				     outputCopy2) ;

      callCopy1->append_argument(outputExpression1) ;
      callCopy2->append_argument(outputExpression2) ;

      RedundantGroup toAdd ;
      toAdd.input1 = outputCopy1 ;
      toAdd.input2 = outputCopy2 ;
      toAdd.input3 = NULL ;
      toAdd.output1 = currentOutput ;
      toAdd.output2 = NULL ;
      toAdd.output3 = NULL ;
      voteCalls.push_back(toAdd) ;
    }
  }

  // Create a statement list that will replace the original call
  StatementList* replacementList = create_statement_list(theEnv) ;
  
  // Append all of the copies
  replacementList->append_statement(callCopy1) ;
  replacementList->append_statement(callCopy2) ;
  
  // Append all of the voting calls
  list<RedundantGroup>::iterator voteIter = voteCalls.begin() ;
  int numCalls = 1 ;
  while (voteIter != voteCalls.end())
  {
    replacementList->append_statement(CreateDoubleCall(*voteIter, numCalls)) ;
    ++voteIter ;
    ++numCalls ;
  }

  // Finally, make the replacement
  s->get_parent()->replace(s, replacementList) ;  

}

void RedundancyPass::TripleRedundify(CallStatement* s)
{
  assert(procDef != NULL) ;
  assert(s != NULL) ;
  assert(theEnv != NULL) ;

  // ---------------
  // Create three call statements to replace the original call
  // ---------------
  CallStatement* callCopy1 =
    create_call_statement(theEnv, 
			  NULL,
	   dynamic_cast<Expression*>(s->get_callee_address()->deep_clone())) ;
  CallStatement* callCopy2 = 
    create_call_statement(theEnv,
			  NULL,
	   dynamic_cast<Expression*>(s->get_callee_address()->deep_clone())) ;
  CallStatement* callCopy3 = 
    create_call_statement(theEnv,
			  NULL,
	   dynamic_cast<Expression*>(s->get_callee_address()->deep_clone())) ;

  // ---------------
  // Go through every variable and add it to all of the copies
  // ---------------

  // Start with the destination
  if (s->get_destination() != NULL)
  {
    VariableSymbol* originalDest = s->get_destination() ;
    LString copyName = originalDest->get_name() ;
    copyName = copyName + LString("_TMR") ;
    VariableSymbol* destCopy1 = 
      create_variable_symbol(theEnv, originalDest->get_type(), 
			     TempName(copyName)) ;
    VariableSymbol* destCopy2 =
      create_variable_symbol(theEnv, originalDest->get_type(),
			     TempName(copyName)) ;
    VariableSymbol* destCopy3 = 
      create_variable_symbol(theEnv, originalDest->get_type(),
			     TempName(copyName)) ;

    procDef->get_symbol_table()->append_symbol_table_object(destCopy1) ;
    procDef->get_symbol_table()->append_symbol_table_object(destCopy2) ;
    procDef->get_symbol_table()->append_symbol_table_object(destCopy3) ;

    callCopy1->set_destination(destCopy1) ;
    callCopy2->set_destination(destCopy2) ;
    callCopy3->set_destination(destCopy3) ;    
    
    RedundantGroup toAdd ;
    toAdd.input1 = destCopy1 ;
    toAdd.input2 = destCopy2 ;
    toAdd.input3 = destCopy3 ;
    toAdd.output1 = originalDest ;
    toAdd.output2 = NULL ;
    toAdd.output3 = NULL ;
    voteCalls.push_back(toAdd) ;    
  }

  // Now go through each of arguments
  for (int i = 0 ; i < s->get_argument_count() ; ++i)
  {
    Expression* currentArg = s->get_argument(i) ;
    if (!IsOutputVariable(currentArg))
    {
      if (IsStream(currentArg))
      {
	VariableSymbol* currentVar = NULL ;
	LoadVariableExpression* inExpr =
	  dynamic_cast<LoadVariableExpression*>(currentArg) ;
	assert(inExpr != NULL) ;
	currentVar = inExpr->get_source() ;
	CallStatement* splitCall = CreateTripleSplitter(currentVar) ;

	// Insert this call statement right before this statement
	StatementList* parentList = 
	  dynamic_cast<StatementList*>(s->get_parent()) ;
	assert(parentList != NULL) ;
	AddChildStatementBefore(parentList, splitCall, s) ;

	SymbolAddressExpression* streamCopy1 = 
	  dynamic_cast<SymbolAddressExpression*>(splitCall->get_argument(1)) ;
	SymbolAddressExpression* streamCopy2 = 
	  dynamic_cast<SymbolAddressExpression*>(splitCall->get_argument(2)) ;
	SymbolAddressExpression* streamCopy3 = 
	  dynamic_cast<SymbolAddressExpression*>(splitCall->get_argument(3)) ;

	assert(streamCopy1 != NULL) ;
	assert(streamCopy2 != NULL) ;
	assert(streamCopy3 != NULL) ;

	VariableSymbol* varCopy1 =
	  dynamic_cast<VariableSymbol*>(streamCopy1->get_addressed_symbol());
	VariableSymbol* varCopy2 =
	  dynamic_cast<VariableSymbol*>(streamCopy2->get_addressed_symbol());
	VariableSymbol* varCopy3 =
	  dynamic_cast<VariableSymbol*>(streamCopy3->get_addressed_symbol());

	assert(varCopy1 != NULL) ;
	assert(varCopy2 != NULL) ;
	assert(varCopy3 != NULL) ;

	LoadVariableExpression* loadCopy1 =
	  create_load_variable_expression(theEnv,
				  varCopy1->get_type()->get_base_type(),
				  varCopy1) ;

	LoadVariableExpression* loadCopy2 =
	  create_load_variable_expression(theEnv,
				  varCopy2->get_type()->get_base_type(),
				  varCopy2) ;

	LoadVariableExpression* loadCopy3 =
	  create_load_variable_expression(theEnv,
				  varCopy3->get_type()->get_base_type(),
				  varCopy3) ;
	callCopy1->append_argument(loadCopy1) ;
	callCopy2->append_argument(loadCopy2) ;
	callCopy3->append_argument(loadCopy3) ;
      }
      else
      {
	callCopy1->
	  append_argument(dynamic_cast<Expression*>(currentArg->deep_clone()));
	callCopy2->
	  append_argument(dynamic_cast<Expression*>(currentArg->deep_clone()));
	callCopy3->
	  append_argument(dynamic_cast<Expression*>(currentArg->deep_clone()));
      }
    }
    else
    {
      VariableSymbol* currentOutput = GetOutputVariable(currentArg) ;
      LString copyName = currentOutput->get_name() ;
      copyName = copyName + LString("_TMR") ;
      VariableSymbol* outputCopy1 =
	create_variable_symbol(theEnv,
			       currentOutput->get_type(),
			       TempName(copyName)) ;
      VariableSymbol* outputCopy2 =
	create_variable_symbol(theEnv,
			       currentOutput->get_type(),
			       TempName(copyName)) ;
      VariableSymbol* outputCopy3 =
	create_variable_symbol(theEnv,
			       currentOutput->get_type(),
			       TempName(copyName)) ;

      procDef->get_symbol_table()->append_symbol_table_object(outputCopy1) ;
      procDef->get_symbol_table()->append_symbol_table_object(outputCopy2) ;
      procDef->get_symbol_table()->append_symbol_table_object(outputCopy3) ;

      SymbolAddressExpression* outputExpression1 =
	create_symbol_address_expression(theEnv,
				     outputCopy1->get_type()->get_base_type(),
				     outputCopy1) ;

      SymbolAddressExpression* outputExpression2 =
	create_symbol_address_expression(theEnv,
				     outputCopy2->get_type()->get_base_type(),
				     outputCopy2) ;

      SymbolAddressExpression* outputExpression3 =
	create_symbol_address_expression(theEnv,
				     outputCopy3->get_type()->get_base_type(),
				     outputCopy3) ;

      callCopy1->append_argument(outputExpression1) ;
      callCopy2->append_argument(outputExpression2) ;
      callCopy3->append_argument(outputExpression3) ;

      RedundantGroup toAdd ;
      toAdd.input1 = outputCopy1 ;
      toAdd.input2 = outputCopy2 ;
      toAdd.input3 = outputCopy3 ;
      toAdd.output1 = currentOutput ;
      toAdd.output2 = NULL ;
      toAdd.output3 = NULL ;
      voteCalls.push_back(toAdd) ;
    }
  }

  // Create a statement list that will replace the original call
  StatementList* replacementList = create_statement_list(theEnv) ;
  
  // Append all of the copies
  replacementList->append_statement(callCopy1) ;
  replacementList->append_statement(callCopy2) ;
  replacementList->append_statement(callCopy3) ;

  // Annotate all of the copy statements as redundant calls
   BrickAnnote* copyAnnote1 = create_brick_annote(theEnv, "RedundantCall") ;
   BrickAnnote* copyAnnote2 = create_brick_annote(theEnv, "RedundantCall") ;
   BrickAnnote* copyAnnote3 = create_brick_annote(theEnv, "RedundantCall") ;

   IntegerBrick* copyInteger1 = create_integer_brick(theEnv, IInteger(1)) ;
   IntegerBrick* copyInteger2 = create_integer_brick(theEnv, IInteger(2)) ;
   IntegerBrick* copyInteger3 = create_integer_brick(theEnv, IInteger(3)) ;

   copyAnnote1->append_brick(copyInteger1) ;
   copyAnnote2->append_brick(copyInteger2) ;
   copyAnnote3->append_brick(copyInteger3) ;
  
   callCopy1->append_annote(copyAnnote1) ;
   callCopy2->append_annote(copyAnnote2) ;
   callCopy3->append_annote(copyAnnote3) ;

  // Append all of the voting calls
  list<RedundantGroup>::iterator voteIter = voteCalls.begin() ;
  int numCalls = 1 ;
  while (voteIter != voteCalls.end())
  {
    replacementList->append_statement(CreateTripleCall(*voteIter, numCalls)) ;
    ++voteIter ;
    ++numCalls ;
  }

  // Finally, make the replacement
  s->get_parent()->replace(s, replacementList) ;  
}

CallStatement* RedundancyPass::CreateTripleCall(RedundantGroup x, 
						int copyNumber)
{
  assert(procDef != NULL) ;
  assert(tripleVoteAddress != NULL) ;
  assert(tripleVoteStreamsAddress != NULL) ;

  CallStatement* voteCall ;
  DataType* tmpType = DeReference(x.input1->get_type()->get_base_type()) ;

  SymbolAddressExpression* voteAddress = NULL ;
  bool streamsVoter = false ;

  if (dynamic_cast<PointerType*>(tmpType) != NULL)
  {
    voteAddress = tripleVoteStreamsAddress ;
    streamsVoter = true ;
  }
  else
  {
    voteAddress = tripleVoteAddress ;
  }

  voteCall =
    create_call_statement(theEnv,
			 NULL, // No destination
			 dynamic_cast<Expression*>(voteAddress->deep_clone()));
  
  IntegerType* errorType = create_integer_type(theEnv, IInteger(1), 0, false) ;
  QualifiedType* errorQualType = create_qualified_type(theEnv, errorType);
  VariableSymbol* errorVariable =
    create_variable_symbol(theEnv,
			   errorQualType,
			   TempName(LString("TMRError"))) ;

  procDef->get_symbol_table()->append_symbol_table_object(errorType) ;
  procDef->get_symbol_table()->append_symbol_table_object(errorQualType) ;
  procDef->get_symbol_table()->append_symbol_table_object(errorVariable) ;

  // I treat error variables as debug registers so they head straight out
  errorVariable->append_annote(create_brick_annote(theEnv, "DebugRegister")) ;

  CProcedureType* tripleType =
    dynamic_cast<CProcedureType*>(voteAddress->get_addressed_symbol()->get_type()) ;
  assert(tripleType != NULL) ;

  // Now append all the arguments
  voteCall->append_argument(create_load_variable_expression(theEnv,
  		errorVariable->get_type()->get_base_type(),
  	        errorVariable)) ;
  tripleType->append_argument(errorVariable->get_type()) ;

  voteCall->append_argument(create_load_variable_expression(theEnv,
			x.input1->get_type()->get_base_type(),
		        x.input1)) ;
  tripleType->append_argument(x.input1->get_type()) ;

  voteCall->append_argument(create_load_variable_expression(theEnv,
			x.input2->get_type()->get_base_type(),
		        x.input2)) ;
  tripleType->append_argument(x.input2->get_type()) ;

  voteCall->append_argument(create_load_variable_expression(theEnv,
			    x.input3->get_type()->get_base_type(),
		            x.input3)) ;
  tripleType->append_argument(x.input3->get_type()) ;    

  // The last three are symbol address expressions because they are
  //  outputs (and not specially designed to go straight out).
  voteCall->append_argument(create_symbol_address_expression(theEnv,
  		        x.output1->get_type()->get_base_type(),
		        x.output1)) ;
  tripleType->append_argument(x.output1->get_type()) ;

  if (x.output2 != NULL)
  {
    voteCall->append_argument(create_symbol_address_expression(theEnv,
  		          x.output2->get_type()->get_base_type(),
		          x.output2)) ;
    tripleType->append_argument(x.output2->get_type()) ;
  }
  else if (streamsVoter == false)
  {
    // Create a dummy output variable the same type as the original output
    VariableSymbol* dummy1 = 
      create_variable_symbol(theEnv, x.output1->get_type(), 
			     TempName(LString("outputCopy"))) ;
    dummy1->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;
    dummy1->append_annote(create_brick_annote(theEnv, "Dummy")) ;
    procDef->get_symbol_table()->append_symbol_table_object(dummy1) ;
    
    voteCall->append_argument(create_symbol_address_expression(theEnv,
			      dummy1->get_type()->get_base_type(),
			      dummy1)) ;
    tripleType->append_argument(dummy1->get_type()) ;
  }

  if (x.output3 != NULL)
  {
    voteCall->append_argument(create_symbol_address_expression(theEnv,
  		          x.output3->get_type()->get_base_type(),
		          x.output3)) ;
    tripleType->append_argument(x.output2->get_type()) ;
  }
  else if (streamsVoter == false)
  {
    // Create a dummy output variable the same type as the original output
    VariableSymbol* dummy1 = 
      create_variable_symbol(theEnv, x.output1->get_type(), 
			     TempName(LString("outputCopy"))) ;
    dummy1->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;
    dummy1->append_annote(create_brick_annote(theEnv, "Dummy")) ;
    procDef->get_symbol_table()->append_symbol_table_object(dummy1) ;
    
    voteCall->append_argument(create_symbol_address_expression(theEnv,
			      dummy1->get_type()->get_base_type(),
			      dummy1)) ;
    tripleType->append_argument(dummy1->get_type()) ;
  }

  return voteCall ;  
}

CallStatement* RedundancyPass::CreateDoubleCall(RedundantGroup x, 
						int copyNumber)
{
  assert(procDef != NULL) ;
  assert(doubleVoteAddress != NULL) ;
  assert(doubleVoteStreamsAddress != NULL) ;

  CallStatement* voteCall ;
  DataType* tmpType = DeReference(x.input1->get_type()->get_base_type()) ;
  SymbolAddressExpression* voteAddress = NULL ;

  bool streamsVoter = false ;

  if (dynamic_cast<PointerType*>(tmpType) != NULL)
  {
    voteAddress = doubleVoteStreamsAddress ;
    streamsVoter = true ;
  }
  else
  {
    voteAddress = doubleVoteAddress ;
  }
  voteCall =
    create_call_statement(theEnv,
			 NULL, // No destination
			 dynamic_cast<Expression*>(voteAddress->deep_clone()));

  // Create the error variable
  IntegerType* errorType = create_integer_type(theEnv, IInteger(1), 0, false) ;
  QualifiedType* errorQualType = create_qualified_type(theEnv, errorType);
  VariableSymbol* errorVariable =
    create_variable_symbol(theEnv,
			   errorQualType,
			   TempName(LString("DMRError"))) ;

  procDef->get_symbol_table()->append_symbol_table_object(errorType) ;
  procDef->get_symbol_table()->append_symbol_table_object(errorQualType) ;
  procDef->get_symbol_table()->append_symbol_table_object(errorVariable) ;

  // I treat error variables as debug registers so they head straight out
  errorVariable->append_annote(create_brick_annote(theEnv, "DebugRegister")) ;

  CProcedureType* doubleType =
    dynamic_cast<CProcedureType*>(voteAddress->get_addressed_symbol()->get_type()) ;
  assert(doubleType != NULL) ;

  // Now append all the arguments
  voteCall->append_argument(create_load_variable_expression(theEnv,
  			errorVariable->get_type()->get_base_type(),
  		        errorVariable)) ;
  doubleType->append_argument(errorVariable->get_type()) ;

  voteCall->append_argument(create_load_variable_expression(theEnv,
			x.input1->get_type()->get_base_type(),
		        x.input1)) ;
  doubleType->append_argument(x.input1->get_type()) ;

  voteCall->append_argument(create_load_variable_expression(theEnv,
			x.input2->get_type()->get_base_type(),
		        x.input2)) ;
  doubleType->append_argument(x.input2->get_type()) ;

  assert(x.input3 == NULL) ;
  voteCall->append_argument(create_load_variable_expression(theEnv,
			    x.input2->get_type()->get_base_type(),
			    x.input2)) ;
  doubleType->append_argument(x.input2->get_type()) ;

  // The last three are symbol address expressions because they are
  //  outputs (and not specially designed to go straight out).
  voteCall->append_argument(create_symbol_address_expression(theEnv,
  		        x.output1->get_type()->get_base_type(),
		        x.output1)) ;
  doubleType->append_argument(x.output1->get_type()) ;

  if (x.output2 != NULL)
  {
    voteCall->append_argument(create_symbol_address_expression(theEnv,
  		          x.output2->get_type()->get_base_type(),
		          x.output2)) ;
    doubleType->append_argument(x.output2->get_type()) ;
  }
  else if (streamsVoter == false)
  {
    // Create a dummy output variable the same type as the original output
    VariableSymbol* dummy1 = 
      create_variable_symbol(theEnv, x.output1->get_type(), 
			     TempName(LString("outputCopy"))) ;
    dummy1->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;
    dummy1->append_annote(create_brick_annote(theEnv, "Dummy")) ;
    procDef->get_symbol_table()->append_symbol_table_object(dummy1) ;
     
    voteCall->append_argument(create_symbol_address_expression(theEnv,
			      dummy1->get_type()->get_base_type(),
			      dummy1)) ;
    doubleType->append_argument(dummy1->get_type()) ;
  }

  if (x.output3 != NULL)
  {
    voteCall->append_argument(create_symbol_address_expression(theEnv,
			      x.output3->get_type()->get_base_type(),
			      x.output3)) ;
    doubleType->append_argument(x.output3->get_type()) ;
  }
  else if (streamsVoter == false)
  {
    VariableSymbol* dummy1 = 
      create_variable_symbol(theEnv, 
			     x.output1->get_type(),
			     TempName(LString("outputCopy"))) ;
    dummy1->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;
    dummy1->append_annote(create_brick_annote(theEnv, "Dummy")) ;
    procDef->get_symbol_table()->append_symbol_table_object(dummy1) ;
    
    voteCall->append_argument(create_symbol_address_expression(theEnv,
							       dummy1->get_type()->get_base_type(),
							       dummy1)) ;
    doubleType->append_argument(dummy1->get_type()) ;
  }

  return voteCall ;
}


// This function is responsible for creating the symbol that we call.
//  Both double redundancy and triple redundancy call the same vote function
//  though.
void RedundancyPass::CreateVoteProcedures()
{
  CProcedureType* tripleVoteType =
    create_c_procedure_type(theEnv,
			    get_void_type(theEnv), // return type
			    true, // has varargs
			    false, // arguments known
			    0, // bit alignment
			    LString("ROCCCTripleVoteType")) ;

  CProcedureType* doubleVoteType =
    create_c_procedure_type(theEnv,
			    get_void_type(theEnv), // return type
			    true, // has varargs
			    false, // arguments known
			    0, // bit alignment
			    LString("ROCCCDoubleVoteType")) ;

  CProcedureType* tripleVoteStreamsType = 
    create_c_procedure_type(theEnv,
			    get_void_type(theEnv),
			    true, // has varargs
			    false, // arguments known
			    0, // bit alignment
			    LString("ROCCCTripleVoteStreamsType")) ;

  CProcedureType* doubleVoteStreamsType = 
    create_c_procedure_type(theEnv,
			    get_void_type(theEnv),
			    true, // has varargs
			    false, // arguments known
			    0, // bit alignment
			    LString("ROCCCDoubleVoteStreamsType")) ;

  ProcedureSymbol* tripleVoteSymbol = 
    create_procedure_symbol(theEnv,
			    tripleVoteType,
			    LString("ROCCCTripleVote")) ;

  ProcedureSymbol* doubleVoteSymbol =
    create_procedure_symbol(theEnv,
			    doubleVoteType,
			    LString("ROCCCDoubleVote")) ;

  ProcedureSymbol* tripleVoteStreamsSymbol = 
    create_procedure_symbol(theEnv, 
			    tripleVoteStreamsType,
			    LString("ROCCCStreamsTripleVote")) ;
  
  ProcedureSymbol* doubleVoteStreamsSymbol = 
    create_procedure_symbol(theEnv, 
			    doubleVoteStreamsType,
			    LString("ROCCCStreamsDoubleVote")) ;

  procDef->get_symbol_table()->append_symbol_table_object(tripleVoteType) ;
  procDef->get_symbol_table()->append_symbol_table_object(doubleVoteType) ;
  procDef->get_symbol_table()->
    append_symbol_table_object(tripleVoteStreamsType) ;
  procDef->get_symbol_table()->
    append_symbol_table_object(doubleVoteStreamsType) ;
  procDef->get_symbol_table()->append_symbol_table_object(tripleVoteSymbol) ;
  procDef->get_symbol_table()->append_symbol_table_object(doubleVoteSymbol) ;
  procDef->get_symbol_table()->
    append_symbol_table_object(tripleVoteStreamsSymbol) ;
  procDef->get_symbol_table()->
    append_symbol_table_object(doubleVoteStreamsSymbol) ;
  
  tripleVoteAddress = 
    create_symbol_address_expression(theEnv,
				     tripleVoteType->get_result_type(),
				     tripleVoteSymbol) ;

  doubleVoteAddress = 
    create_symbol_address_expression(theEnv,
				     doubleVoteType->get_result_type(),
				     doubleVoteSymbol) ;

  tripleVoteStreamsAddress = 
    create_symbol_address_expression(theEnv,
				     tripleVoteStreamsType->get_result_type(),
				     tripleVoteStreamsSymbol) ;
  
  doubleVoteStreamsAddress = 
    create_symbol_address_expression(theEnv,
				     doubleVoteStreamsType->get_result_type(),
				     doubleVoteStreamsSymbol) ;
}

void RedundancyPass::ClearList()
{
  while (!voteCalls.empty())
  {
    voteCalls.erase(voteCalls.begin()) ;
  }
}

CallStatement* RedundancyPass::CreateDoubleSplitter(VariableSymbol* x)
{
  assert(theEnv != NULL) ;
  assert(procDef != NULL) ;

  // Create a new function for splitting
  CProcedureType* splitType = 
    create_c_procedure_type(theEnv,
			    get_void_type(theEnv), // return type
			    false, // has varargs
			    true, // arguments known
			    0, // bit alignment
			    TempName(LString("ROCCCSplitterType"))) ;

  ProcedureSymbol* splitSymbol = 
    create_procedure_symbol(theEnv,
			    splitType,
			    TempName(LString("ROCCCSplitter"))) ;

  SymbolAddressExpression* splitAddress = 
    create_symbol_address_expression(theEnv,
				     splitType->get_result_type(),
				     splitSymbol) ;
  
  CallStatement* constructed = 
    create_call_statement(theEnv,
			  NULL, // No destination
			  splitAddress) ;

  // Add all of the types to the CProcedureType
  QualifiedType* inType = 
    dynamic_cast<QualifiedType*>(x->get_type()->deep_clone()) ;
  QualifiedType* outType1 = 
    dynamic_cast<QualifiedType*>(x->get_type()->deep_clone()) ;
  QualifiedType* outType2 = 
    dynamic_cast<QualifiedType*>(x->get_type()->deep_clone()) ;

  assert(inType != NULL) ;
  assert(outType1 != NULL) ;
  assert(outType2 != NULL) ;

  procDef->get_symbol_table()->append_symbol_table_object(inType) ;
  procDef->get_symbol_table()->append_symbol_table_object(outType1) ;
  procDef->get_symbol_table()->append_symbol_table_object(outType2) ;

  splitType->append_argument(inType) ;
  splitType->append_argument(outType1) ;
  splitType->append_argument(outType2) ;

  LString copyNameBase = x->get_name() ;
  copyNameBase = copyNameBase + LString("_TMR") ;

  // Create all of the output variables
  VariableSymbol* outVar1 =
    create_variable_symbol(theEnv, 
			   outType1,
			   TempName(copyNameBase)) ;
  VariableSymbol* outVar2 =
    create_variable_symbol(theEnv,
			   outType2,
			   TempName(copyNameBase)) ;

  procDef->get_symbol_table()->append_symbol_table_object(outVar1) ;
  procDef->get_symbol_table()->append_symbol_table_object(outVar2) ;

  outVar1->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;
  outVar1->append_annote(create_brick_annote(theEnv, "Dummy")) ;
  outVar2->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;
  outVar2->append_annote(create_brick_annote(theEnv, "Dummy")) ;  

  LoadVariableExpression* inLoad = 
    create_load_variable_expression(theEnv,
				    x->get_type()->get_base_type(),
				    x) ;

  SymbolAddressExpression* outLoad1 = 
    create_symbol_address_expression(theEnv,
				     outVar1->get_type()->get_base_type(),
				     outVar1) ;

  SymbolAddressExpression* outLoad2 =
    create_symbol_address_expression(theEnv,
				     outVar2->get_type()->get_base_type(),
				     outVar2) ;

  // Add all of the variables to the Call statement
  constructed->append_argument(inLoad) ;
  constructed->append_argument(outLoad1) ;
  constructed->append_argument(outLoad2) ;

  return constructed ;
}

CallStatement* RedundancyPass::CreateTripleSplitter(VariableSymbol* x)
{
  assert(theEnv != NULL) ;
  assert(procDef != NULL) ;

  // Create a new function for splitting
  CProcedureType* splitType = 
    create_c_procedure_type(theEnv,
			    get_void_type(theEnv), // return type
			    false, // has varargs
			    true, // arguments known
			    0, // bit alignment
			    TempName(LString("ROCCCSplitterType"))) ;

  ProcedureSymbol* splitSymbol = 
    create_procedure_symbol(theEnv,
			    splitType,
			    TempName(LString("ROCCCSplitter"))) ;

  SymbolAddressExpression* splitAddress = 
    create_symbol_address_expression(theEnv,
				     splitType->get_result_type(),
				     splitSymbol) ;
  
  CallStatement* constructed = 
    create_call_statement(theEnv,
			  NULL, // No destination
			  splitAddress) ;

  // Add all of the types to the CProcedureType
  QualifiedType* inType = 
    dynamic_cast<QualifiedType*>(x->get_type()->deep_clone()) ;
  QualifiedType* outType1 = 
    dynamic_cast<QualifiedType*>(x->get_type()->deep_clone()) ;
  QualifiedType* outType2 = 
    dynamic_cast<QualifiedType*>(x->get_type()->deep_clone()) ;
  QualifiedType* outType3 =
    dynamic_cast<QualifiedType*>(x->get_type()->deep_clone()) ;

  assert(inType != NULL) ;
  assert(outType1 != NULL) ;
  assert(outType2 != NULL) ;
  assert(outType3 != NULL) ;

  procDef->get_symbol_table()->append_symbol_table_object(inType) ;
  procDef->get_symbol_table()->append_symbol_table_object(outType1) ;
  procDef->get_symbol_table()->append_symbol_table_object(outType2) ;
  procDef->get_symbol_table()->append_symbol_table_object(outType3) ;

  splitType->append_argument(inType) ;
  splitType->append_argument(outType1) ;
  splitType->append_argument(outType2) ;
  splitType->append_argument(outType3) ;

  LString copyNameBase = x->get_name() ;
  copyNameBase = copyNameBase + LString("_TMR") ;

  // Create all of the output variables
  VariableSymbol* outVar1 =
    create_variable_symbol(theEnv, 
			   outType1,
			   TempName(copyNameBase)) ;
  VariableSymbol* outVar2 =
    create_variable_symbol(theEnv,
			   outType2,
			   TempName(copyNameBase)) ;
  VariableSymbol* outVar3 = 
    create_variable_symbol(theEnv,
			   outType3,
			   TempName(copyNameBase)) ;

  procDef->get_symbol_table()->append_symbol_table_object(outVar1) ;
  procDef->get_symbol_table()->append_symbol_table_object(outVar2) ;
  procDef->get_symbol_table()->append_symbol_table_object(outVar3) ;

  outVar1->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;
  outVar1->append_annote(create_brick_annote(theEnv, "Dummy")) ;
  outVar2->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;
  outVar2->append_annote(create_brick_annote(theEnv, "Dummy")) ;
  outVar3->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;
  outVar3->append_annote(create_brick_annote(theEnv, "Dummy")) ;

  LoadVariableExpression* inLoad = 
    create_load_variable_expression(theEnv,
				    x->get_type()->get_base_type(),
				    x) ;

  SymbolAddressExpression* outLoad1 = 
    create_symbol_address_expression(theEnv, 
				     outVar1->get_type()->get_base_type(),
				     outVar1) ;
  SymbolAddressExpression* outLoad2 =
    create_symbol_address_expression(theEnv,
				     outVar2->get_type()->get_base_type(),
				     outVar2) ;

  SymbolAddressExpression* outLoad3 =
    create_symbol_address_expression(theEnv,
				     outVar3->get_type()->get_base_type(),
				     outVar3) ;

  // Add all of the variables to the Call statement
  constructed->append_argument(inLoad) ;
  constructed->append_argument(outLoad1) ;
  constructed->append_argument(outLoad2) ;
  constructed->append_argument(outLoad3) ;

  return constructed ;
}
