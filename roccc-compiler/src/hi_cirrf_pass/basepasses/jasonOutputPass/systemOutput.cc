
#include <suifkernel/utilities.h>
#include <basicnodes/basic_factory.h>

#include "roccc_utils/roccc2.0_utils.h"
#include "roccc_utils/warning_utils.h"

#include "systemOutput.h"

SystemGenerator::SystemGenerator(SuifEnv* e, ProcedureDefinition* p,
				 String s) :
  HiCirrfGenerator(e, p) 
{
  streamFileName = s ;
  fifoCounter = 0 ;
}

SystemGenerator::~SystemGenerator()
{
}

void SystemGenerator::Setup()
{
  CollectVariables() ;
  CleanupNames() ;
  InitializeChannels() ;
  CollectSteps() ;
}

LString SystemGenerator::CleanOutputName(LString n)
{  
  // If there is anything with _ssaTmp, remove it
  std::string tmpName = n.c_str() ;
  LString cleanName ;
  int position = tmpName.rfind("_ssaTmp") ;  
  if (position != std::string::npos)
  {
    tmpName = tmpName.substr(0, position) ;
    cleanName = tmpName.c_str() ;    
  }
  else
  {
    cleanName = n ;
  }  

  tmpName = cleanName.c_str() ;
  position = tmpName.rfind("_out") ;
  if (position == std::string::npos)
  {
    cleanName = cleanName + "_out" ;
  }
  return cleanName ;
}

void SystemGenerator::Output()
{
  // Output the hi_cirrf.c file
  OutputHeader() ;
  OutputDeclarations() ;
  OutputFunctionType() ;

  OutputFakes() ;

  OutputLookupTables() ;
  OutputInputScalars() ;
  OutputInputArrays() ;

  OutputDatapath(procDef->get_body()) ;

  OutputMaximizePrecision() ;
  OutputSystolicInitialization() ;
  
  OutputFooter() ;

  // Also, output the roccc.h file
  OutputDotH() ;
}

// Remember, the system generator dot h file is generated after all of the
//  rest of the code has been processed.
void SystemGenerator::OutputDotH()
{
  // Print out the standard declarations
  HiCirrfGenerator::OutputDotH() ;

  // I also need to print out the header information for the input
  //  and output fifos.
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* nextObj = symTab->get_symbol_table_object(i) ;
    if (nextObj->lookup_annote_by_name("OutputFifo") != NULL)
    {
      rocccOut << "void ROCCCOutputFifo" ;
      BrickAnnote* dimAnnote =
	to<BrickAnnote>(nextObj->lookup_annote_by_name("DimensionAnnote")) ;
      assert(dimAnnote != NULL) ;
      IntegerBrick* dimValue = to<IntegerBrick>(dimAnnote->get_brick(0)) ;
      int dimensionality = dimValue->get_value().c_int() ;

      BrickAnnote* counterAnnote =
	to<BrickAnnote>(nextObj->lookup_annote_by_name("UniqueCounter")) ;
      assert(counterAnnote != NULL) ;
      IntegerBrick* counterBrick = 
	to<IntegerBrick>(counterAnnote->get_brick(0)) ;
      assert(counterBrick != NULL) ;
      int uniqueCounter = counterBrick->get_value().c_int() ;

      rocccOut << dimensionality ;
      rocccOut << "_" << uniqueCounter ;
      rocccOut << "(" ;
      Symbol* nextSym = dynamic_cast<Symbol*>(nextObj) ;
      assert(nextSym != NULL) ;
      rocccOut << StringType(GetBaseType(nextSym->get_type())) ;
      if (dynamic_cast<PointerType*>(GetBaseType(nextSym->get_type())) == NULL)
      {
	for(int j = 0 ; j < dimensionality ; ++j)
	{
	  rocccOut << "*" ;
	}
      }
      rocccOut << ", ...) ;" << std::endl ;
    }
    if (nextObj->lookup_annote_by_name("InputFifo") != NULL)
    {
      rocccOut << "void ROCCC" ;
      BrickAnnote* smartBufferAnnote = 
	to<BrickAnnote>(nextObj->lookup_annote_by_name("isSmartBuffer")) ;
      assert(smartBufferAnnote != NULL) ;
      IntegerBrick* isSmartBuffer = 
	to<IntegerBrick>(smartBufferAnnote->get_brick(0)) ;
      assert(isSmartBuffer != NULL) ;
      if (isSmartBuffer->get_value().c_int() == 1)
      {
	rocccOut << "InputSB" ;
      }
      else
      {
	rocccOut << "InputFifo" ;
      }
      BrickAnnote* dimAnnote = 
	to<BrickAnnote>(nextObj->lookup_annote_by_name("DimensionAnnote")) ;
      assert(dimAnnote != NULL) ;
      IntegerBrick* dimValue = to<IntegerBrick>(dimAnnote->get_brick(0)) ;
      int dimensionality = dimValue->get_value().c_int() ;
      rocccOut << dimensionality ;

      BrickAnnote* counterAnnote =
	to<BrickAnnote>(nextObj->lookup_annote_by_name("UniqueCounter")) ;
      assert(counterAnnote != NULL) ;
      IntegerBrick* counterBrick = 
	to<IntegerBrick>(counterAnnote->get_brick(0)) ;
      assert(counterBrick != NULL) ;
      int uniqueCounter = counterBrick->get_value().c_int() ;

      rocccOut << "_" << uniqueCounter ;
      rocccOut << "(" ;
      Symbol* nextSym = dynamic_cast<Symbol*>(nextObj) ;
      assert(nextSym != NULL) ;
      rocccOut << StringType(GetBaseType(nextSym->get_type())) ;
      if (dynamic_cast<PointerType*>(GetBaseType(nextSym->get_type())) == NULL)
      {
	for(int j = 0 ; j < dimensionality ; ++j)
	{
	  rocccOut << "*" ;
	}
      }
      rocccOut << ", ...) ;" << std::endl ;
    }    
  }
}

void SystemGenerator::OutputFunctionType()
{
  fout << "ROCCCFunctionType(2) ;" << std::endl ;
}

void SystemGenerator::OutputFakes()
{
  assert(symTab != NULL) ;

  HiCirrfGenerator::OutputFakes() ;

  // Input and output fifos need special fakes
  std::list<VariableSymbol*>::iterator inputArrayIter = inputArrays.begin() ;
  while (inputArrayIter != inputArrays.end())
  {
    BrickAnnote* dimAnnote = to<BrickAnnote>((*inputArrayIter)->lookup_annote_by_name("DimensionAnnote")) ;
    assert(dimAnnote != NULL) ;
    IntegerBrick* dimValue = to<IntegerBrick>(dimAnnote->get_brick(0)) ;
    int dimensionality = dimValue->get_value().c_int() ;

    OutputFake(*inputArrayIter, dimensionality) ;

    ++inputArrayIter ;
  }

  std::list<VariableSymbol*>::iterator outputArrayIter = outputArrays.begin() ;
  while (outputArrayIter != outputArrays.end())
  {
    BrickAnnote* dimAnnote = to<BrickAnnote>((*outputArrayIter)->lookup_annote_by_name("DimensionAnnote")) ;
    assert(dimAnnote != NULL) ;
    IntegerBrick* dimValue = to<IntegerBrick>(dimAnnote->get_brick(0)) ;
    int dimensionality = dimValue->get_value().c_int() ;

    OutputFake(*outputArrayIter, dimensionality) ;

    ++outputArrayIter ;
  }

  // All of the offsets require a fake as well
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* nextObj = symTab->get_symbol_table_object(i) ;
    VariableSymbol* nextSym = dynamic_cast<VariableSymbol*>(nextObj) ;
    if (nextSym == NULL)
    {
      continue ;
    }
    if (nextSym->lookup_annote_by_name("InputFifo") != NULL ||
	nextSym->lookup_annote_by_name("OutputFifo") != NULL)
    {
      BrickAnnote* dimAnnote = to<BrickAnnote>(nextSym->lookup_annote_by_name("DimensionAnnote")) ;
      assert(dimAnnote != NULL) ;
      IntegerBrick* dimValue = to<IntegerBrick>(dimAnnote->get_brick(0)) ;
      int dimensionality = dimValue->get_value().c_int() ;
     
      BrickAnnote* indexAnnote = 
	dynamic_cast<BrickAnnote*>(nextSym->
				   lookup_annote_by_name("IndexAnnote")) ;
      assert(indexAnnote != NULL) ;
      for (int j = 0 ; j < indexAnnote->get_brick_count() ; 
	   j += (1 + dimensionality))
      {
	SuifObjectBrick* variable = dynamic_cast<SuifObjectBrick*>(indexAnnote->get_brick(j)) ;
	assert(variable != NULL) ;
	SuifObject* varObj = variable->get_object() ;
	assert(varObj != NULL) ;
	VariableSymbol* nextVar = dynamic_cast<VariableSymbol*>(varObj) ;

	if (nextVar != NULL)
	{
	  OutputFake(nextVar) ;
	}
      }
    }
  }
}

void SystemGenerator::CollectVariables()
{
  CollectInputs() ;
  CollectOutputs() ;
  CollectFeedbacks() ;
}

void SystemGenerator::InitializeChannels()
{
  // Go through the stream info file and set up the map

  std::ifstream infoIn ;
  infoIn.open(streamFileName.c_str()) ;
  if (!infoIn)
  {
    return ;
  }
  infoIn >> std::ws ;

  while (!infoIn.eof())
  {
    std::string isInput ;
    std::string nextStreamName ;
    int numDataChannels ;
    int numAddressChannels ;

    infoIn >> isInput >> std::ws ; 
    infoIn >> nextStreamName >> std::ws ;
    infoIn >> numDataChannels >> std::ws ;
    infoIn >> numAddressChannels >> std::ws ;

    // Verify the names of the input and output streams
    if (isInput == "INPUT") 
    {
      std::list<VariableSymbol*>::iterator inputIter = inputArrays.begin() ;
      bool found = false ;
      while (inputIter != inputArrays.end())
      {
	if (nextStreamName == std::string((*inputIter)->get_name().c_str()))
	{
	  found = true;
	}
	++inputIter ;
      }
      assert(found && "Specifed an array as input that was not input!") ;      
    }
    else if (isInput == "OUTPUT") 
    {
      std::list<VariableSymbol*>::iterator outputIter = outputArrays.begin() ;
      bool found = false ;
      while (outputIter != outputArrays.end())
      {
	if (nextStreamName == std::string((*outputIter)->get_name().c_str()))
	{
	  found = true;
	}
	++outputIter ;
      }

      assert(found && 
	     "Array identified as output when it is not an output stream!") ;
    }
    else
    {
      assert(0 && "Incorrecly formatted stream file!") ;
    }

    streamAddressChannels[nextStreamName] = numAddressChannels ;
    streamDataChannels[nextStreamName] = numDataChannels ;
  }
}

void SystemGenerator::CollectFeedbacks()
{
  assert(procDef != NULL) ;
  assert(symTab != NULL) ;
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* nextObj = symTab->get_symbol_table_object(i) ;
    VariableSymbol* nextVar = dynamic_cast<VariableSymbol*>(nextObj) ;
    if (nextVar != NULL && 
	nextObj->lookup_annote_by_name("FeedbackSource") != NULL) 
    {
      feedbackScalars.push_back(nextVar) ;
    }
  }
}

void SystemGenerator::CollectInputs()
{
  assert(procDef != NULL) ;
  assert(symTab != NULL) ;
  
  // Inputs are either marked in the symbol table or end values of for 
  //  loop iterations.
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* nextObj = symTab->get_symbol_table_object(i) ;
    VariableSymbol* nextVar = dynamic_cast<VariableSymbol*>(nextObj) ;
    if (nextVar == NULL)
    {      
      continue ;
    }
    if (nextVar->lookup_annote_by_name("InputScalar") != NULL &&
	nextVar->lookup_annote_by_name("TemporalFeedback") == NULL &&
	nextVar->lookup_annote_by_name("NormalFeedback") == NULL &&
	nextVar->lookup_annote_by_name("DebugRegister") == NULL &&
	nextVar->lookup_annote_by_name("LUT") == NULL &&
	nextVar->lookup_annote_by_name("OutputVariable") == NULL)
    {      
      inputs.push_back(nextVar) ;
      if (dynamic_cast<ParameterSymbol*>(nextVar) == NULL)
      {
	OutputWarning("Found an input that was not a parameter!") ;
      }
    }
    if (nextVar->lookup_annote_by_name("InputFifo") != NULL)
    {
      inputArrays.push_back(nextVar) ;
      if (dynamic_cast<ParameterSymbol*>(nextVar) == NULL)
      {
	OutputWarning("Found an input fifo that was not a parameter!") ;
      }
    }
  }


  // Sort the input arrays by parameter order
  inputArrays = SortParameters(inputArrays) ;
  
  // Now handle the for loops
  list<CForStatement*>* allFors = 
    collect_objects<CForStatement>(procDef->get_body()) ;
  list<CForStatement*>::iterator forIter = allFors->begin() ;
  while (forIter != allFors->end())
  {
    Expression* nextTest = (*forIter)->get_test() ;
    BinaryExpression* nextBinTest = dynamic_cast<BinaryExpression*>(nextTest) ;
    if (nextBinTest == NULL)
    {
      ++forIter ;
      continue ;
    }
    Expression* endValue = nextBinTest->get_source2() ;
    LoadVariableExpression* nextLoadVar = 
      dynamic_cast<LoadVariableExpression*>(endValue) ;
    if (nextLoadVar != NULL)
    {
      VariableSymbol* toAdd = nextLoadVar->get_source() ;
      // Because we might have two of the same variable...
      if (!InInputList(toAdd))
      {
	toAdd->append_annote(create_brick_annote(theEnv, "InputScalar")) ;
	inputs.push_back(toAdd) ;
	if (dynamic_cast<ParameterSymbol*>(toAdd) == NULL)
	{
	  OutputWarning("Found an input scalar that was not a parameter!") ;
	}
      }
    }
    ++forIter ;
  }
  delete allFors ;

}

void SystemGenerator::CollectOutputs()
{
  assert(symTab != NULL) ;

  // Go through all of the elements of the symbol table and 
  //  check to see if they are marked as either an output variable
  //  or an output fifo.
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* nextObj = symTab->get_symbol_table_object(i) ;
    VariableSymbol* nextVar = dynamic_cast<VariableSymbol*>(nextObj) ;
    if (nextVar == NULL)
    {
      continue ;
    }
    if (nextVar->lookup_annote_by_name("OutputVariable") != NULL &&
	nextVar->lookup_annote_by_name("Dummy")          == NULL &&
	nextVar->lookup_annote_by_name("FeedbackSource") == NULL) 
    {
      outputs.push_back(nextVar) ;
      if (dynamic_cast<ParameterSymbol*>(nextVar) == NULL)
      {
	OutputWarning("Detected an output scalar that is not a parameter") ;
      }
    }
    if (nextVar->lookup_annote_by_name("OutputFifo") != NULL)
    {
      outputArrays.push_back(nextVar) ;
      if (dynamic_cast<ParameterSymbol*>(nextVar) == NULL)
      {
	OutputWarning("Detected an output array that is not a parameter!") ;
      }
    }

    // Sort the output arrays in parameter order
    outputArrays = SortParameters(outputArrays) ;
  }
}

void SystemGenerator::CollectSteps()
{
  assert(procDef != NULL) ;
  list<CForStatement*>* allFors = 
    collect_objects<CForStatement>(procDef->get_body()) ;
  list<CForStatement*>::iterator forIter = allFors->begin() ;
  while (forIter != allFors->end())
  {
    Statement* stepStatement = (*forIter)->get_step() ;

    StoreVariableStatement* storeVarStep =
      dynamic_cast<StoreVariableStatement*>(stepStatement) ;
    assert(storeVarStep != NULL) ;
    
    Expression* currentStep = storeVarStep->get_value() ;
    assert(currentStep != NULL) ;
    BinaryExpression* binStep = dynamic_cast<BinaryExpression*>(currentStep) ;
    assert(binStep != NULL) ;
    
    std::pair<VariableSymbol*, int> nextStep ;
    nextStep.first = storeVarStep->get_destination() ;
    
    // One of the values must be a constant
    if (dynamic_cast<Constant*>(binStep->get_source1()) != NULL)
    {
      IntConstant* stepConstant = 
	dynamic_cast<IntConstant*>(binStep->get_source1()) ;
      assert(stepConstant != NULL) ;
      nextStep.second = stepConstant->get_value().c_int() ;
    }
    else if (dynamic_cast<Constant*>(binStep->get_source2()) != NULL)
    {
      IntConstant* stepConstant =
	dynamic_cast<IntConstant*>(binStep->get_source2()) ;
      assert(stepConstant != NULL) ;
      nextStep.second = stepConstant->get_value().c_int() ;
    }
    else
    {
      assert(0 && "Step without a constant!") ;
    }
    stepSizes.push_back(nextStep) ;

    ++forIter ;
  }  
  delete allFors ;
}

// This function is responsible for outputting all of the values of the
//  various steps for the nested loops.
void SystemGenerator::OutputSteps()
{
  std::list< std::pair<VariableSymbol*, int> >::iterator stepIter = 
    stepSizes.begin() ;
  while (stepIter != stepSizes.end())
  {
    fout << "ROCCCStep(" ;
    fout << (*stepIter).first->get_name() ;
    fout << ", " ;
    fout << (*stepIter).second ;
    fout << ") ;" << std::endl ;
    ++stepIter ;
  }
}

void SystemGenerator::OutputNumChannelsWorkhorse(VariableSymbol* v, 
						 bool isInput)
{
  LString originalName = v->get_name() ;
  LString strippedName ;
  if (isInput)
  {
    strippedName = StripSuffix(originalName, "_input") ;
  }
  else
  {
    strippedName = StripSuffix(originalName, "_output") ;
  }

  std::string originalString = originalName.c_str() ;
  std::string strippedString = strippedName.c_str() ;

  int numDataChannels ;
  int numAddressChannels ;

  if (streamDataChannels.find(originalString) != streamDataChannels.end())
  {
    numDataChannels = streamDataChannels[originalString] ;
  }
  else if (streamDataChannels.find(strippedString) != streamDataChannels.end())
  {
    numDataChannels = streamDataChannels[strippedString] ;
  }
  else
  {
    // Default is 1
    numDataChannels = 1 ;
  }

  if (streamAddressChannels.find(originalString) != 
      streamAddressChannels.end())
  {
    numAddressChannels = streamAddressChannels[originalString] ;
  }
  else if (streamAddressChannels.find(strippedString) != 
	   streamAddressChannels.end())
  {
    numAddressChannels = streamAddressChannels[strippedString] ;
  }
  else
  {
    // Default is 1
    numAddressChannels = 1 ;
  }

  fout << "ROCCCNumDataChannels(" ;
  fout << numDataChannels ;
  fout << ", " ;
  fout << v->get_name() ;
  fout << ") ;" << std::endl ;
  
  fout << "ROCCCNumAddressChannels(" ;
  fout << numAddressChannels ;
  fout << ", " << v->get_name() << ") ;" << std::endl ;
}

void SystemGenerator::OutputNumChannels()
{
  std::list<VariableSymbol*>::iterator streamIter = inputArrays.begin() ;
  while (streamIter != inputArrays.end())
  {
    OutputNumChannelsWorkhorse(*streamIter, true) ;
    ++streamIter ;
  }

  streamIter = outputArrays.begin() ;
  while (streamIter != outputArrays.end())
  {
    OutputNumChannelsWorkhorse(*streamIter, false) ;
    ++streamIter ;
  }
}

// When generating systems, we print out a normalized loop as opposed to 
//  a loop as it appears in the original C code.  The actual information
//  such as step size and number of channels is passed through
//  informational functions.
void SystemGenerator::PrintForStatement(CForStatement* s)
{
  Statement* step = s->get_step() ;
  StoreVariableStatement* storeVarStep = 
    dynamic_cast<StoreVariableStatement*>(step) ;
  
  fout << "for (" ;
  PrintStatement(s->get_before()) ;
  fout << " " ;

  // If we are trying to do an infinite loop, we can't print out the expression
  //  but must instead print out a comparison to infinity
  if (dynamic_cast<IntConstant*>(s->get_test()) != NULL)
  {
    fout << storeVarStep->get_destination()->get_name() << " < " ;
    fout << "ROCCCInfinity()" ;
  }
  else
  {
    PrintExpression(s->get_test()) ;
  }
  fout << " ; " ;
  
  // For the increment, we must print out the normalized version
  fout << storeVarStep->get_destination()->get_name() << " = " 
       << storeVarStep->get_destination()->get_name() << " + 1" ;
  fout << ")" << std::endl << "{" << std::endl ;

  // If we are the innermost perfectly nested loop, print everything out.
  //  Otherwise just print out the inner loop.
  if (IsInnermostLoop(s))
  {
    PrintInputLoopFifos() ;
    PrintLoadPrevious() ;
    PrintTemporalLoads() ;
    PrintStatement(s->get_body()) ; // The datapath

    OutputSizes() ;
    OutputSteps() ;
    OutputNumChannels() ;

    PrintTemporalStores() ;
    PrintStoreToNext() ;
    OutputDebugRegisters() ;
    PrintOutputLoopFifos() ;
    OutputOutputScalars() ;    
    OutputOutputArrays() ;
    OutputLookupTableOrder() ;
  }
  else
  {
    PrintStatement(s->get_body()) ;
  }
  fout << "}" << std::endl ;
}

// This function is necessary because of systolic array generation
LString SystemGenerator::StripSuffix(LString& original, const char* suffix)
{
  const char* searchMe = original.c_str() ;
  bool inLastPosition = false ;

  char* suffixSubstring = strstr(searchMe, suffix) ;
  while(suffixSubstring != NULL &&
	suffixSubstring != (searchMe + strlen(searchMe) - strlen(suffix)))
  {
    suffixSubstring = strstr(suffixSubstring + 1,  suffix) ;
  }

  if (suffixSubstring != NULL)
  {
    inLastPosition = true ;
  }

  if (inLastPosition)
  {
    char* copySubstring = new char[strlen(searchMe) - strlen(suffix) + 1] ;
    strncpy(copySubstring, searchMe, strlen(searchMe) - strlen(suffix)) ;
    copySubstring[strlen(searchMe) - strlen(suffix)] = '\0' ;

    LString replacement = copySubstring ;
    delete copySubstring ;
    return replacement ;
  }
  else
  {
    return original ;
  }

}

void SystemGenerator::OutputSystolicInitialization()
{
  assert(procDef != NULL) ;
  Annote* systolicInit = procDef->lookup_annote_by_name("SystolicInit") ;
  BrickAnnote* systolicBrick = dynamic_cast<BrickAnnote*>(systolicInit) ;
  if (systolicBrick != NULL)
  {
    SuifBrick* brick = systolicBrick->get_brick(0) ;
    IntegerBrick* valueBrick = dynamic_cast<IntegerBrick*>(brick) ;
    assert(valueBrick != NULL) ;
    fout << "ROCCCSystolicNextInit(" << valueBrick->get_value().c_int()
	 << ") ;" << std::endl ;
  }
}

void SystemGenerator::PrintLoadPrevious()
{
  assert(procDef != NULL) ;
  static int loadPreviousNumber = 0 ;

  SymbolTable* symTab = procDef->get_symbol_table() ;
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* nextObj = symTab->get_symbol_table_object(i) ;
    if (nextObj->lookup_annote_by_name("FeedbackVariable") != NULL)
    {
      BrickAnnote* feedbackAnnote = 
	to<BrickAnnote>(nextObj->lookup_annote_by_name("FeedbackVariable")) ;
      SuifBrick* loadBrick = feedbackAnnote->get_brick(0) ;
      SuifObjectBrick* loadSOB = dynamic_cast<SuifObjectBrick*>(loadBrick) ;
      assert(loadSOB != NULL) ;
      
      VariableSymbol* loadVar = 
	dynamic_cast<VariableSymbol*>(loadSOB->get_object()) ;

      if (loadVar == NULL)
      {
	Expression* loadExp = dynamic_cast<Expression*>(loadSOB->get_object());
	assert(loadExp != NULL) ;
	rocccOut << "void ROCCCSystolicPrevious" << loadPreviousNumber 
		 << "(" ;
	fout << "ROCCCSystolicPrevious" << loadPreviousNumber << "(" ;
	++loadPreviousNumber ;
       
	PrintExpression(loadExp) ;
	fout << ", " ;
	fout << nextObj->get_name() ;
	fout << ") ;" << std::endl ;
	
	rocccOut << StringType(loadExp->get_result_type()) ;
	rocccOut << ", " ;
	assert(dynamic_cast<VariableSymbol*>(nextObj) != NULL) ;
	rocccOut << StringType(dynamic_cast<VariableSymbol*>(nextObj)->get_type()) ;
	rocccOut << ") ;" << std::endl ;
      }
      else
      {
	rocccOut << "void ROCCCSystolicPrevious" << loadPreviousNumber << "(" ;
	fout << "ROCCCSystolicPrevious" << loadPreviousNumber << "(" ;

	++loadPreviousNumber ;
	fout << loadVar->get_name() ;
	fout << ", " ;
	fout << nextObj->get_name() ;
	fout << ") ;" << std::endl ;
	
	rocccOut << StringType(loadVar->get_type()) ;
	rocccOut << ", " ;
	assert(dynamic_cast<VariableSymbol*>(nextObj) != NULL) ;
	rocccOut << StringType(dynamic_cast<VariableSymbol*>(nextObj)->get_type()) ;
	rocccOut << ") ;" << std::endl ;
      }
      //  assert(loadVar != NULL) ;


    }
  }
}

void SystemGenerator::PrintTemporalLoads()
{
  assert(procDef != NULL) ;
  SymbolTable* symTab = procDef->get_symbol_table() ;
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* currentObj = symTab->get_symbol_table_object(i) ;
    if (currentObj->lookup_annote_by_name("TemporalFeedbackVariable") != NULL)
    {
      fout << "ROCCC_SystolicPrevious(" ;
      BrickAnnote* temporalBrick = dynamic_cast<BrickAnnote*>(currentObj->lookup_annote_by_name("TemporalFeedbackVariable")) ;
      assert(temporalBrick != NULL) ;
      SuifBrick* brick0 = temporalBrick->get_brick(0) ;
      SuifObjectBrick* sob = dynamic_cast<SuifObjectBrick*>(brick0) ;
      assert(sob != NULL) ;
      VariableSymbol* var = dynamic_cast<VariableSymbol*>(sob->get_object()) ;
      assert(var != NULL) ;
      fout << var->get_name() << ", " ;
      fout << currentObj->get_name() << ") ;" << std::endl ;
    }
  }
}

void SystemGenerator::PrintTemporalStores()
{
  assert(procDef != NULL) ;
  SymbolTable* symTab = procDef->get_symbol_table() ;
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* currentObj = symTab->get_symbol_table_object(i) ;
    if (currentObj->lookup_annote_by_name("TemporalFeedbackVariable") != NULL)
    {
      BrickAnnote* currentBrick = dynamic_cast<BrickAnnote*>(currentObj->lookup_annote_by_name("TemporalFeedbackVariable")) ;
      SuifBrick* brick1 = currentBrick->get_brick(1) ;
      SuifObjectBrick* sob = dynamic_cast<SuifObjectBrick*>(brick1) ;
      assert(sob != NULL) ;
      VariableSymbol* var = 
	dynamic_cast<VariableSymbol*>(sob->get_object()) ;
      assert(var != NULL) ;

      fout << "ROCCC_systolicNext(" ;
      fout << currentObj->get_name() << ", " ;
      fout << var->get_name() ;
      fout << ") ;" << std::endl ;
    }
  }
}

// Due to the fact that ints or floats can be feedback variables
//  I'm going to have to create a separate function for each systolicNext

void SystemGenerator::PrintStoreToNext()
{
  assert(symTab != NULL) ;
  static int storeNextNumber = 0 ;
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* nextObj = symTab->get_symbol_table_object(i) ;
    if (nextObj->lookup_annote_by_name("FeedbackVariable") != NULL)
    {
      BrickAnnote* feedbackAnnote = 
	to<BrickAnnote>(nextObj->lookup_annote_by_name("FeedbackVariable")) ;
      assert(feedbackAnnote != NULL) ;
      
      SuifBrick* storeBrick = feedbackAnnote->get_brick(1) ;
      SuifObjectBrick* storeSOB = dynamic_cast<SuifObjectBrick*>(storeBrick) ;
      assert(storeSOB != NULL) ;

      VariableSymbol* storeVar = 
	dynamic_cast<VariableSymbol*>(storeSOB->get_object()) ;
      assert(storeVar != NULL) ;

      rocccOut << "void ROCCC_systolicNext" << storeNextNumber << "(" ;
      fout << "ROCCC_systolicNext"      << storeNextNumber << "(" ;      

      ++storeNextNumber ;

      fout << nextObj->get_name() ;
      fout << ", " ;
      fout << storeVar->get_name() ;
      fout << ") ;" << std::endl ;

      assert(dynamic_cast<VariableSymbol*>(nextObj) != NULL) ;
      rocccOut << StringType(dynamic_cast<VariableSymbol*>(nextObj)->get_type()) ;
      rocccOut << ", " ;
      rocccOut << StringType(storeVar->get_type()) ;
      rocccOut << ") ; " << std::endl ;

    }
  }
  // Also print out the call to the function that handles the feedback scalars
  fout << "ROCCCFeedbackScalar(" ;
  rocccOut << "void ROCCCFeedbackScalar(" ;

  std::list<VariableSymbol*>::iterator feedbackIter = feedbackScalars.begin() ;
  while (feedbackIter != feedbackScalars.end())
  {
    if (feedbackIter != feedbackScalars.begin())
    {
      fout << ", " ;
      rocccOut << ", " ;
    }
    fout << (*feedbackIter)->get_name() ;
    rocccOut << StringType((*feedbackIter)->get_type()) ;
    ++feedbackIter ;
  }

  fout << ") ;" << std::endl ;
  rocccOut << ") ; " << std::endl ;
}

void SystemGenerator::PrintInputLoopFifos()
{

  std::list<VariableSymbol*>::iterator arrayIter = inputArrays.begin() ;
  while (arrayIter != inputArrays.end())
  {
    PrintInputLoopFifo(*arrayIter) ;
    ++arrayIter ;
  }

}

void SystemGenerator::PrintInputLoopFifo(VariableSymbol* v)
{
  assert(v != NULL) ;

  // Get all of the annotations associated with this variable
  Annote* inputAnnote = v->lookup_annote_by_name("InputFifo") ;
  BrickAnnote* inputFifoBrick = dynamic_cast<BrickAnnote*>(inputAnnote) ;
  assert(inputFifoBrick != NULL) ;

  Annote* dimAnnote = v->lookup_annote_by_name("DimensionAnnote") ;
  BrickAnnote* dimBrick = dynamic_cast<BrickAnnote*>(dimAnnote) ;
  assert(dimBrick != NULL) ;
  IntegerBrick* dimValue = dynamic_cast<IntegerBrick*>(dimBrick->get_brick(0));
  int dimensionality = dimValue->get_value().c_int() ;

  Annote* indexAnnote = v->lookup_annote_by_name("IndexAnnote") ;
  BrickAnnote* indexBrick = dynamic_cast<BrickAnnote*>(indexAnnote) ;
  assert(indexBrick != NULL) ;
  

  Annote* sbAnnote = v->lookup_annote_by_name("isSmartBuffer") ;
  BrickAnnote* sbBrick = dynamic_cast<BrickAnnote*>(sbAnnote) ;
  assert(sbBrick != NULL) ;
  IntegerBrick* sbValue = dynamic_cast<IntegerBrick*>(sbBrick->get_brick(0)) ;
  assert(sbValue != NULL) ;
  int trueSmartBuffer = sbValue->get_value().c_int() ;
  
  if (trueSmartBuffer)
  {
    fout << "ROCCCInputSB" ;
  }
  else
  {
    fout << "ROCCCInputFifo" ;
  }

  IntegerBrick* counterValue = 
    create_integer_brick(theEnv, IInteger(fifoCounter)) ;
  BrickAnnote* counterBrick = 
    create_brick_annote(theEnv, LString("UniqueCounter")) ;
  counterBrick->append_brick(counterValue) ;
  v->append_annote(counterBrick) ;  

  fout << dimensionality ;
  fout << "_" << fifoCounter ;
  fout << "(" ;
  ++fifoCounter ;

  // First, the name of the array
  fout << v->get_name() ;

  // Next, all of the index variables
  for (int i = 0 ; i < dimensionality ; ++i)
  {
    fout << ", " ;
    SuifObjectBrick* indexBrick = 
      dynamic_cast<SuifObjectBrick*>(inputFifoBrick->get_brick(i)) ;
    assert(indexBrick != NULL) ;
    VariableSymbol* indexVar = 
      dynamic_cast<VariableSymbol*>(indexBrick->get_object()) ;
    assert(indexVar != NULL)  ;
    fout << indexVar->get_name() ;
  }

  // Now print out all of the scalar replaced variables and their 
  //  corresponding offsets in each dimension
  for (int i = 0 ; i < indexBrick->get_brick_count() ; i += 1 + dimensionality)
  {
    fout << ", " ;
 
    SuifObjectBrick* scalarReplacedVarBrick = 
      dynamic_cast<SuifObjectBrick*>(indexBrick->get_brick(i)) ; 
    assert(scalarReplacedVarBrick != NULL) ;
    VariableSymbol* scalarReplacedVar = 
      dynamic_cast<VariableSymbol*>(scalarReplacedVarBrick->get_object()) ;
    assert(scalarReplacedVar != NULL) ;

    fout << scalarReplacedVar->get_name() ;
    for (int j = 1 ; j <= dimensionality ; ++j)
    {
      fout << ", " ;
      SuifObjectBrick* offsetBrick = 
	dynamic_cast<SuifObjectBrick*>(indexBrick->get_brick(i+j)) ;
      assert(offsetBrick != NULL) ;
      Expression* offset = 
	dynamic_cast<Expression*>(offsetBrick->get_object()) ;
      assert(offset != NULL) ;
      PrintExpression(offset) ;
    }
  }
  // And we're done
  fout << ") ;" << std::endl ;

}

void SystemGenerator::PrintOutputLoopFifos()
{

  std::list<VariableSymbol*>::iterator arrayIter = outputArrays.begin() ;
  while (arrayIter != outputArrays.end())
  {
    PrintOutputLoopFifo(*arrayIter) ;
    ++arrayIter ;
  }
}

void SystemGenerator::PrintOutputLoopFifo(VariableSymbol* v)
{

  // Collect all the annotations
  Annote* outputFifoAnnote = v->lookup_annote_by_name("OutputFifo") ;
  BrickAnnote* outputFifoBrick = dynamic_cast<BrickAnnote*>(outputFifoAnnote) ;
  assert(outputFifoBrick != NULL) ;

  Annote* indexAnnote = v->lookup_annote_by_name("IndexAnnote") ;
  BrickAnnote* indexBrick = dynamic_cast<BrickAnnote*>(indexAnnote) ;

  Annote* dimAnnote = v->lookup_annote_by_name("DimensionAnnote") ;
  BrickAnnote* dimBrick = dynamic_cast<BrickAnnote*>(dimAnnote) ;
  assert(dimBrick != NULL) ;
  IntegerBrick* dimValue = dynamic_cast<IntegerBrick*>(dimBrick->get_brick(0));
  int dimensionality = dimValue->get_value().c_int() ;

  IntegerBrick* counterValue = 
    create_integer_brick(theEnv, IInteger(fifoCounter)) ;
  BrickAnnote* counterBrick = 
    create_brick_annote(theEnv, LString("UniqueCounter")) ;
  counterBrick->append_brick(counterValue) ;
  v->append_annote(counterBrick) ;

  fout << "ROCCCOutputFifo" ;
  fout << dimensionality ;
  fout << "_" << fifoCounter ;
  fout << "(" ;

  ++fifoCounter ;
  
  // First, print out the name
  fout << v->get_name() ;

  // Next, print out all of the index variables
  for (int i = 0 ; i < dimensionality ; ++i)
  {
    fout << ", " ;

    SuifObjectBrick* indexBrick = 
      dynamic_cast<SuifObjectBrick*>(outputFifoBrick->get_brick(i)) ;
    assert(indexBrick != NULL) ;
    VariableSymbol* indexVariable = 
      dynamic_cast<VariableSymbol*>(indexBrick->get_object()) ;
    assert(indexVariable != NULL) ;
    
    fout << indexVariable->get_name() ; 
  }

  // Now print out all of the scalar replaced variables and their 
  //  corresponding offsets
  for (int i = 0 ; i < indexBrick->get_brick_count() ; i += 1 + dimensionality)
  {
    fout << ", " ;
    SuifObjectBrick* scalarReplacedBrick = 
      dynamic_cast<SuifObjectBrick*>(indexBrick->get_brick(i)) ;
    assert(scalarReplacedBrick != NULL) ;
    VariableSymbol* scalarReplacedVar = 
      dynamic_cast<VariableSymbol*>(scalarReplacedBrick->get_object()) ;
    assert(scalarReplacedVar != NULL) ;

    fout << scalarReplacedVar->get_name() ;

    // And now, the corresponding offsets
    for (int j = 1 ; j <= dimensionality ; ++j)
    {
      fout << ", " ;
      SuifObjectBrick* offsetBrick = 
	dynamic_cast<SuifObjectBrick*>(indexBrick->get_brick(i+j)) ;
      assert(offsetBrick != NULL) ;
      Expression* offset = 
	dynamic_cast<Expression*>(offsetBrick->get_object()) ;
      PrintExpression(offset) ;
    }
  }
  fout << ") ;" << std::endl ;
}

void SystemGenerator::OutputInputArrays()
{
  fout << "ROCCCInputStreams(" ;
  rocccOut << "void ROCCCInputStreams(" ;

  std::list<VariableSymbol*>::iterator inputIter = inputArrays.begin() ;
  while (inputIter != inputArrays.end())
  {
    if (inputIter != inputArrays.begin())
    {
      fout << ", " ;
      rocccOut << ", " ;
    }
    fout << (*inputIter)->get_name() ;
    rocccOut << StringType((*inputIter)->get_type(), true) ;
    ++inputIter ;
  }

  rocccOut << ") ; " << std::endl ;
  fout << ") ;" << std::endl ;
} 

void SystemGenerator::OutputOutputArrays()
{
  fout << "ROCCCOutputStreams(" ;
  rocccOut << "void ROCCCOutputStreams(" ;
  std::list<VariableSymbol*>::iterator outputIter = outputArrays.begin() ;
  while (outputIter != outputArrays.end())
  {
    if (outputIter != outputArrays.begin())
    {
      fout << ", " ;
      rocccOut << ", " ;
    }

    fout << (*outputIter)->get_name() ;
    rocccOut << StringType((*outputIter)->get_type(), true) ;
    ++outputIter ;
  }
  rocccOut << ") ;" << std::endl ;
  fout << ") ;" << std::endl ;
}
