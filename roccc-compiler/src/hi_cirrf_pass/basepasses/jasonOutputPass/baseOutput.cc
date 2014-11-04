
#include <cassert>

#include <suifkernel/utilities.h>

#include "roccc_utils/roccc2.0_utils.h"

#include "baseOutput.h"

HiCirrfGenerator::HiCirrfGenerator(SuifEnv* e, ProcedureDefinition* p)
{
  theEnv = e ;
  procDef = p ;
  symTab = procDef->get_symbol_table() ;

  // Open the output files we will use
  fout.open("hi_cirrf.c") ;
  rocccOut.open("roccc.h") ;
  if (!fout || !rocccOut)
  {
    assert(0 && "Cannot open output files at the hi-cirrf level!") ;
  }

  InvokeHardwareCounter = 0 ;
}

HiCirrfGenerator::~HiCirrfGenerator()
{
  fout.close() ;
  rocccOut.close() ;
}

void HiCirrfGenerator::Setup()
{
  assert(0 && "Unsupported type of code!") ;
}

void HiCirrfGenerator::Output()
{
  assert(0 && "Non module or system!") ;
}

void HiCirrfGenerator::CleanupNames()
{
  // Only the inputs and outputs need to be cleaned, everything else
  //  can remain as ugly as they want to be.
  std::list<VariableSymbol*>::iterator varIter = inputs.begin() ;
  while (varIter != inputs.end())
  {
    (*varIter)->set_name(CleanInputName((*varIter)->get_name())) ;
    ++varIter ;
  }
  varIter = outputs.begin() ;
  while (varIter != outputs.end())
  {
    (*varIter)->set_name(CleanOutputName((*varIter)->get_name())) ;
    ++varIter ;
  }
}

// The base functions don't do any cleaning
LString HiCirrfGenerator::CleanInputName(LString n)
{
  return n ;
}

LString HiCirrfGenerator::CleanOutputName(LString n)
{
  return n ;
}

// This function outputs the standard prototypes for all functions
//  that might be used in either type one or type two code.  This function
//  must be called after the hi-cirrf has been generated as beforehand we
//  don't know how many invoke hardware calls to create.
void HiCirrfGenerator::OutputDotH()
{
  rocccOut << "void ROCCCCompilerVersion(const char*) ; "    << std::endl ;
  rocccOut << "void ROCCCFunctionType(int) ;"                << std::endl ;
  rocccOut << "void ROCCCIntrinsicType(const char*) ;"       << std::endl ;
  rocccOut << "int ROCCCInfinity() ;"                        << std::endl ;
  rocccOut << "void ROCCCName(const char*, ...) ;"           << std::endl ;
  rocccOut << "void ROCCCSigned(int, ...) ;"                 << std::endl ;
  rocccOut << "void ROCCCModuleStructName(const char*) ;"    << std::endl ;
  rocccOut << "void ROCCCPortOrder(const char*) ;"           << std::endl ;
  rocccOut << "void ROCCCNumDataChannels(int, ...) ;"        << std::endl ;
  rocccOut << "void ROCCCNumAddressChannels(int, ...) ; "    << std::endl ;
  rocccOut << "void ROCCCNumMemReq(int, ...) ;"              << std::endl ;
  rocccOut << "void ROCCCSize(int, ...) ;"                   << std::endl ;
  rocccOut << "void ROCCCStep(int, int) ;"                   << std::endl ;
  rocccOut << "int ROCCCFPToInt(float, int) ;"               << std::endl ;
  rocccOut << "float ROCCCIntToFP(int, int) ;"               << std::endl ;
  rocccOut << "int ROCCCIntToInt(int, int) ;"                << std::endl ;
  rocccOut << "long long ROCCCIntToInt64(long long, int) ;"  << std::endl ;
  rocccOut << "float ROCCCFPToFP(float, int) ;"              << std::endl ;
  rocccOut << "float ROCCCFloatPosInfinity() ; "             << std::endl ;
  rocccOut << "float ROCCCFloatNegInfinity() ; "             << std::endl ;
  rocccOut << "float ROCCCFloatNan() ; "                     << std::endl ;
  
  // These functions are used for redundancy
  //  rocccOut << "void ROCCCDoubleVote(int, ...) ; "            << std::endl ;
  // rocccOut << "void ROCCCTripleVote(int, ...) ;"             << std::endl ;

  // Both modules and systems have input and output scalars
  rocccOut << "void ROCCC_init_inputscalar(" ;
  for (std::list<VariableSymbol*>::iterator printIter = inputs.begin() ;
       printIter != inputs.end() ;
       ++printIter)
  {
    if (printIter != inputs.begin())
    {
      rocccOut << ", " ;
    }
    rocccOut << StringType((*printIter)->get_type())  ;
  }
  rocccOut << ") ;" << std::endl ;

  rocccOut << "void ROCCC_output_C_scalar(" ;
  for (std::list<VariableSymbol*>::iterator printIter = outputs.begin() ;
       printIter != outputs.end() ;
       ++printIter)
  {
    if (printIter != outputs.begin())
    {
      rocccOut << ", " ;
    }
    rocccOut << StringType((*printIter)->get_type()) ;
  }
  rocccOut << ") ;" << std::endl ;

  // Print out the prototypes for all of the invoke hardware function calls
  for (int i = 0 ; i < InvokeHardwareCounter ; ++i) 
  {
    rocccOut << "void ROCCCInvokeHardware" << i 
	     << "(const char*, ...) ;" << std::endl ;
  }

}

void HiCirrfGenerator::OutputHeader()
{
  assert(procDef != NULL) ;
  fout << "#include \"roccc.h\"" << std::endl << std::endl ;
  fout << "void " ;
  fout << procDef->get_procedure_symbol()->get_name() << "()" << std::endl ;
  fout << "{" << std::endl ;
}

void HiCirrfGenerator::OutputDeclarations()
{
  assert(symTab != NULL) ;
  fout << "/* Temporaries */" << std::endl ;
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {    
    PrintDeclaration(symTab->get_symbol_table_object(i)) ;
  }
  fout << "/* End Temporaries */" << std::endl ;
}

void HiCirrfGenerator::CollectVariables()
{
  assert(0 && "Base class CollectVariables should not be called!") ;
}

// Should I do this the same way I do it in constant array propagation?
void HiCirrfGenerator::OutputLookupTables()
{
  static int LUTCounter = 0 ;
  assert(symTab != NULL) ;
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* currentObject = symTab->get_symbol_table_object(i) ;
    VariableSymbol* currentVar = dynamic_cast<VariableSymbol*>(currentObject) ;
    if (currentVar != NULL && 
	currentVar->lookup_annote_by_name("LUT") != NULL)
    {
      fout << "ROCCCInternalLUTDeclaration" << LUTCounter << "("  ;
      fout << currentVar->get_name() ;
      fout << "" ;

      rocccOut << "void ROCCCInternalLUTDeclaration" << LUTCounter << "(" ;
      rocccOut << StringType(currentVar->get_type(), true) ;

      // Now I need to go through and output all of the values one
      //  at a time.  In order to do this, I must find the definition block
      //  associated with this variable.  If it doesn't exist, don't do 
      //  anything.
      DefinitionBlock* procDefBlock = procDef->get_definition_block() ;
      Iter<VariableDefinition*> varDefIter = 
	procDefBlock->get_variable_definition_iterator() ;
      ValueBlock* topLevelBlock = NULL ;
      while (varDefIter.is_valid())
      {
	VariableDefinition* varDef = varDefIter.current() ; 
	VariableSymbol* varSym = varDef->get_variable_symbol() ;	
	if (varSym == currentVar)
	{
	  topLevelBlock = varDef->get_initialization() ;
	}
	varDefIter.next() ;
      }

      // Print out the number elements
      ArrayType* lutType = 
	dynamic_cast<ArrayType*>(currentVar->get_type()->get_base_type()) ;
      assert(lutType != NULL) ;
      QualifiedType* elementType = GetQualifiedTypeOfElement(currentVar) ;
      int elementSize = elementType->get_base_type()->get_bit_size().c_int();
      int numElements = lutType->get_bit_size().c_int() / elementSize ;
      fout << ", " << numElements ;
      rocccOut << ", int" ;
      PrintConstantValues(topLevelBlock) ;
      fout << ") ;" << std::endl ;
      rocccOut << ") ; " << std::endl ;
      ++LUTCounter ;
    }
  }
}

void HiCirrfGenerator::OutputLookupTableOrder()
{
  assert(symTab != NULL) ;

  fout << "ROCCCLUTs(" ;
  rocccOut << "void ROCCCLUTs(" ;
  bool first = true ;
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* currentObject = symTab->get_symbol_table_object(i) ;
    VariableSymbol* currentVar = dynamic_cast<VariableSymbol*>(currentObject) ;
    if (currentVar != NULL && 
	currentVar->lookup_annote_by_name("LUT") != NULL)
    {
      if (!first)
      {
	fout << ", " ;
	rocccOut << ", " ;
      }
      fout << currentVar->get_name() << ", 0" ;
      rocccOut << StringType(currentVar->get_type(), true) << ", int" ;
      first = false ;
    }    
  }
  fout << ") ; " << std::endl ;
  rocccOut << ") ;" << std::endl ;
}

// Print function that only the base class will use
void HiCirrfGenerator::PrintConstantValues(ValueBlock* v)
{
  MultiValueBlock* multi = dynamic_cast<MultiValueBlock*>(v) ;
  ExpressionValueBlock* expr = dynamic_cast<ExpressionValueBlock*>(v) ;
  if (expr != NULL)
  {
    fout << ", " ;
    rocccOut << ", " ;
    PrintExpression(expr->get_expression()) ;
    rocccOut << StringType(expr->get_expression()->get_result_type()) ;
    return ;
  }
  if (multi != NULL)
  {
    for (int i = 0 ; i < multi->get_sub_block_count() ; ++i)
    {
      PrintConstantValues(multi->lookup_sub_block(IInteger(i))) ;
    }
  }
}

// Print functions that all derived classes will use

void HiCirrfGenerator::PrintDeclaration(SymbolTableObject* s, bool toHeader)
{
  assert(theEnv != NULL) ;

  std::ofstream& myOut = toHeader ? rocccOut : fout ;

  Symbol* printMe = dynamic_cast<Symbol*>(s) ;
  if (printMe == NULL)
  {
    return ;
  }

  // Don't print procedure declarations
  if (dynamic_cast<ProcedureSymbol*>(s) != NULL)
  {
    return ;
  }
  
  Annote* constAnnote = s->lookup_annote_by_name("ConstPropArray") ;
  if (constAnnote != NULL)
  {
    return ;
  }
  
  Type* theType = printMe->get_type() ;

  if (dynamic_cast<LabelType*>(theType) != NULL)
  {
    return ;
  }

  DataType* dType = NULL ;
  QualifiedType* qType = dynamic_cast<QualifiedType*>(theType) ;  
  if (qType != NULL)
  {
    PrintQualifications(qType, toHeader) ;
    dType = qType->get_base_type() ;
  }
  else
  {
    dType = dynamic_cast<DataType*>(theType) ;
  }

  if (dynamic_cast<ArrayType*>(dType) != NULL)
  {
    PrintArrayDeclaration(dynamic_cast<ArrayType*>(dType), printMe, toHeader) ;
    return ;
  }

  myOut << StringType(printMe->get_type()) ;
  myOut << " " << printMe->get_name() << " ;" << std::endl ;
}

void HiCirrfGenerator::PrintQualifications(QualifiedType* qType, bool toHeader)
{
  std::ofstream& myOut = toHeader ? rocccOut : fout ;

  for (int i = 0 ; i < qType->get_qualification_count() ; ++i)
  {
    myOut << qType->get_qualification(i) << " " ;
  }
}

void HiCirrfGenerator::PrintArrayDeclaration(ArrayType* aType,
					     Symbol* s, 
					     bool toHeader)
{
  std::ofstream& myOut = toHeader ? rocccOut : fout ;

  Type* baseType = aType->get_element_type() ;
  
  if (dynamic_cast<QualifiedType*>(baseType) != NULL)
  {
    baseType = dynamic_cast<QualifiedType*>(baseType)->get_base_type() ;
  }

  int dimensionality = 1 ;
  while (dynamic_cast<ArrayType*>(baseType) != NULL)
  {
    baseType = dynamic_cast<ArrayType*>(baseType)->get_element_type() ;
    if (dynamic_cast<QualifiedType*>(baseType) != NULL)
    {
      baseType = dynamic_cast<QualifiedType*>(baseType)->get_base_type() ;
    }
    ++dimensionality ;
  }
  
  myOut << StringType(baseType) ;
  for (int i = 0 ; i < dimensionality ; ++i)
  {
    myOut << "*" ;
  }
  myOut << " " << s->get_name() << " ;" << std::endl ;
}

void HiCirrfGenerator::OutputFunctionType()
{
  assert(0 && "Output function type should not be called from the base class");
}

void HiCirrfGenerator::OutputFakes()
{
  assert(procDef != NULL) ;

  // Output fakes for each input scalar
  std::list<VariableSymbol*>::iterator inputIter = inputs.begin() ;
  while (inputIter != inputs.end())
  {
    OutputFake(*inputIter) ;
    ++inputIter ;
  }
  fout << std::endl ;

  // Output fakes for each output scalar
  std::list<VariableSymbol*>::iterator outputIter = outputs.begin() ;
  while (outputIter != outputs.end())
  {
    OutputFake(*outputIter) ;
    ++outputIter ;
  }

  // Fakes need to be output for any variable that is the 
  //  destination variable of a call statement that don't have previous
  //  defintions (which they shouldn't).
  list<CallStatement*>* allCalls = 
    collect_objects<CallStatement>(procDef->get_body()) ;
  list<CallStatement*>::iterator callIter = allCalls->begin() ;
  while (callIter != allCalls->end())
  {
    for (unsigned int i = 0 ; i < (*callIter)->get_argument_count() ; ++i)
    {
      Expression* nextArg = (*callIter)->get_argument(i) ;
      if (dynamic_cast<SymbolAddressExpression*>(nextArg) != NULL)
      {
	Symbol* tmpSymbol = 
	  dynamic_cast<SymbolAddressExpression*>(nextArg)->get_addressed_symbol() ;
	VariableSymbol* nextVariable = 
	  dynamic_cast<VariableSymbol*>(tmpSymbol) ;
	if (nextVariable != NULL)
	{
	  OutputFake(nextVariable) ;
	}
      }
    }
    ++callIter ;
  }
  delete allCalls ;

  // Also, the temporaries must have fakes output
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* nextObj = symTab->get_symbol_table_object(i) ;
    VariableSymbol* nextSym = dynamic_cast<VariableSymbol*>(nextObj) ;
    if (nextSym == NULL)
    {
      continue ;
    }
    if (nextSym->lookup_annote_by_name("LUT") != NULL)
    {
      int dimensionality = 0 ;
      DataType* baseType = nextSym->get_type()->get_base_type() ;
      while (dynamic_cast<ArrayType*>(baseType) != NULL)
      {
	baseType = dynamic_cast<ArrayType*>(baseType)->get_element_type()->get_base_type() ;
	++dimensionality ;
      }
      OutputFake(nextSym, dimensionality) ;
    }
    else if (nextSym->lookup_annote_by_name("NeedsFake") != NULL)
    {
      OutputFake(nextSym) ;
    }
    if (nextSym->lookup_annote_by_name("FeedbackVariable") != NULL)
    {
      // All variables that are temporarily created as destinations
      //  for load/store pairs must have fakes output for them as well.
      BrickAnnote* feedbackAnnote = 
	to<BrickAnnote>(nextSym->lookup_annote_by_name("FeedbackVariable")) ;
      SuifBrick* storeBrick = feedbackAnnote->get_brick(0) ;
      SuifObjectBrick* storeSOB = dynamic_cast<SuifObjectBrick*>(storeBrick) ;
      assert(storeSOB != NULL) ;
      
      VariableSymbol* loadVar = 
	dynamic_cast<VariableSymbol*>(storeSOB->get_object()) ;

      if (loadVar != NULL)
      {
	OutputFake(loadVar) ;
      }
      OutputFake(nextSym) ;
    }
  }
}

void HiCirrfGenerator::OutputInputScalars()
{
  fout << "ROCCC_init_inputscalar(" ;
  for (std::list<VariableSymbol*>::iterator printIter = inputs.begin() ;
       printIter != inputs.end() ;
       ++printIter)
  {
    if (printIter != inputs.begin())
    {
      fout << ", " ;
    }
    fout << (*printIter)->get_name() ;
  }
  fout << ") ;" << std::endl ;
}

void HiCirrfGenerator::OutputOutputScalars()
{
  fout << "ROCCC_output_C_scalar(" ;
  for (std::list<VariableSymbol*>::iterator printIter = outputs.begin() ;
       printIter != outputs.end() ;
       ++printIter)
  {
    if (printIter != outputs.begin())
    {
      fout << ", " ;
    }
    fout << (*printIter)->get_name() ;
  }
  fout << ") ;" << std::endl ;
}

void HiCirrfGenerator::OutputDatapath(ExecutionObject* body)
{
  if (dynamic_cast<Statement*>(body) != NULL)
  {
    PrintStatement(dynamic_cast<Statement*>(body)) ;
  }
  else if (dynamic_cast<Expression*>(body) != NULL)
  {
    PrintExpression(dynamic_cast<Expression*>(body)) ;
  }
  else
  {
    assert(0 && "Unknown function body type!") ;
  }
}

void HiCirrfGenerator::PrintStatement(Statement* s)
{
  // Added for systolic array generation.  We remove non-printable instructions
  //  but leave NULL in their place.  If we ever encounter one of these
  //  NULLs we should just move on.
  if (s == NULL) 
  {
    return ;
  }
  if (s->lookup_annote_by_name("NonPrintable") != NULL)
  {
    delete s->remove_annote_by_name("NonPrintable") ;
    return ;
  }
  
  if (dynamic_cast<StatementList*>(s) != NULL)
  {
    PrintStatementList(dynamic_cast<StatementList*>(s)) ;
  }
  else if (dynamic_cast<IfStatement*>(s) != NULL)
  {
    PrintIfStatement(dynamic_cast<IfStatement*>(s)) ;
  }
  else if (dynamic_cast<WhileStatement*>(s) != NULL)
  {
    assert(0 && "While loops not currently supported") ;
  }
  else if (dynamic_cast<ScopeStatement*>(s) != NULL)
  {
    fout << "/* Scope statement */" << std::endl ;
  }
  else if (dynamic_cast<CallStatement*>(s) != NULL)
  {
    PrintCallStatement(dynamic_cast<CallStatement*>(s)) ;
  }
  else if (dynamic_cast<StoreStatement*>(s) != NULL)
  {
    PrintStoreStatement(dynamic_cast<StoreStatement*>(s)) ;
  }
  else if (dynamic_cast<StoreVariableStatement*>(s) != NULL)
  {
    PrintStoreVariableStatement(dynamic_cast<StoreVariableStatement*>(s)) ;
  }
  else if (dynamic_cast<MarkStatement*>(s) != NULL)
  {
    fout << "/* Mark Statement */" << std::endl ;
  }
  else if (dynamic_cast<LabelLocationStatement*>(s) != NULL)
  {
    fout << "/* Label Location Statement: " ;
    fout << dynamic_cast<LabelLocationStatement*>(s)->get_defined_label()->get_name() ;
    fout << " */" << std::endl ;
  }
  else if (dynamic_cast<EvalStatement*>(s) != NULL)
  {
    fout << "/* Eval Statement */" << std::endl ;
  }
  else if (dynamic_cast<BranchStatement*>(s) != NULL)
  {
    fout << "/* Branch statement */" << std::endl ;
  }
  else if (dynamic_cast<ReturnStatement*>(s) != NULL)
  {
    fout << "/* Return statement */" << std::endl ;
  }
  else if (dynamic_cast<CForStatement*>(s) != NULL)
  {
    PrintForStatement(dynamic_cast<CForStatement*>(s)) ;
  }
  else
  {
    assert(0 && "Statement not yet supported") ;
  }
}

void HiCirrfGenerator::PrintExpression(Expression* e, bool toHeader)
{
  std::ofstream& myOut = toHeader ? rocccOut : fout ;
  if (dynamic_cast<IntConstant*>(e) != NULL)
  {
    if (dynamic_cast<IntConstant*>(e)->get_value().is_c_int())
    {
      myOut << dynamic_cast<IntConstant*>(e)->get_value().c_int() ;
    }
    else if (dynamic_cast<IntConstant*>(e)->get_value().is_c_string_int())
    {
      myOut << dynamic_cast<IntConstant*>(e)->get_value().c_string_int() ;
      myOut << "LL" ;
    }
    else
    {
      assert(0 && "Unknown integer constant format") ;
    }
  }
  else if (dynamic_cast<FloatConstant*>(e) != NULL) 
  {
    // Sometimes, the string that is stored in a float constant
    //  looks like an integer, so we shouldn't print out "f".
    //  Other times, it looks like a float, so we need "f".

    String toPrint = dynamic_cast<FloatConstant*>(e)->get_value() ;

    if (toPrint == String("inf"))
    {
      myOut << "ROCCCFloatPosInfinity() /* infinity */" ;
    }
    else if (toPrint == String("-inf"))
    {
      myOut << "ROCCCFloatNegInfinity() /* -infinity */" ;
    }
    else if (toPrint == String("nan"))
    {
      myOut << "ROCCCFloatNan()" ;
    }
    else
    {
      myOut << toPrint ;
      if (FloatFormat(toPrint))
      {
	myOut << "f" ;
      }
    }
  }
  else if (dynamic_cast<BinaryExpression*>(e) != NULL)
  {
    PrintBinaryExpression(dynamic_cast<BinaryExpression*>(e)) ;
  }
  else if (dynamic_cast<UnaryExpression*>(e) != NULL)
  {
    PrintUnaryExpression(dynamic_cast<UnaryExpression*>(e)) ;
  }
  else if (dynamic_cast<LoadExpression*>(e) != NULL)
  {
    PrintExpression(dynamic_cast<LoadExpression*>(e)->get_source_address()) ;
  }
  else if (dynamic_cast<LoadVariableExpression*>(e) != NULL)
  {
    fout << dynamic_cast<LoadVariableExpression*>(e)->get_source()->get_name();
  }
  else if (dynamic_cast<SymbolAddressExpression*>(e) != NULL)
  {
    PrintSymbolAddressExpression(dynamic_cast<SymbolAddressExpression*>(e)) ;
  }
  else if (dynamic_cast<AddressExpression*>(e) != NULL)
  {
    PrintAddressExpression(dynamic_cast<AddressExpression*>(e)) ;
  }
  else if (dynamic_cast<FieldAccessExpression*>(e) != NULL)
  {
    PrintFieldAccessExpression(dynamic_cast<FieldAccessExpression*>(e)) ;
  }
  else if (dynamic_cast<MultiDimArrayExpression*>(e) != NULL)
  {
    assert(0 && "MultiDimArrayExpression should not have been generated!") ;
  }
  else if (dynamic_cast<ArrayReferenceExpression*>(e) != NULL)
  {
    PrintArrayReferenceExpression(dynamic_cast<ArrayReferenceExpression*>(e));
  }
  else if (dynamic_cast<CallExpression*>(e) != NULL)
  {
    PrintCallExpression(dynamic_cast<CallExpression*>(e)) ;
  }
  else if (dynamic_cast<NonLvalueExpression*>(e) != NULL)
  {
    PrintExpression(dynamic_cast<NonLvalueExpression*>(e)->get_addressed_expression()) ;
  }
  else
  {
    assert(0 && "Unknown expression detected") ;
  }
}

void HiCirrfGenerator::PrintStatementList(StatementList* s)
{
  for (Iter<Statement*> iter = s->get_child_statement_iterator() ;
       iter.is_valid() ;
       iter.next())
  {
    PrintStatement(iter.current()) ;
  }
}

void HiCirrfGenerator::PrintIfStatement(IfStatement* s)
{
  fout << "if (" ;
  PrintExpression(s->get_condition()) ;
  fout << ")" << std::endl ;
  fout << "{" << std::endl ;
  PrintStatement(s->get_then_part()) ;
  fout << "}" << std::endl ;
  if (s->get_else_part() != NULL)
  {
    fout << "else " << std::endl << "{" << std::endl ;
    PrintStatement(s->get_else_part()) ;
    fout << "}" << std::endl ;    
  }
}

void HiCirrfGenerator::PrintCallStatement(CallStatement* s)
{
  SymbolAddressExpression* procSymExpr = 
    dynamic_cast<SymbolAddressExpression*>(s->get_callee_address()) ;
  assert(procSymExpr != NULL) ;

  ProcedureSymbol* procSym = 
    dynamic_cast<ProcedureSymbol*>(procSymExpr->get_addressed_symbol()) ;
  assert(procSym != NULL) ;
  ProcedureType* procType = procSym->get_type() ;
  assert(procType != NULL) ;
  CProcedureType* cProcType = dynamic_cast<CProcedureType*>(procType) ;
  assert(cProcType != NULL) ;

  LString findRoccc = procSym->get_name() ;
  
  //if (strstr(findRoccc.c_str(), "ROCCC") == NULL)
  if (!IsBuiltIn(s))
  {
    // This must be a hardware invocation.
    fout << "ROCCCInvokeHardware" << InvokeHardwareCounter << "(\"" ;
    fout << findRoccc << "\"" ;

    // The way the function was originally called and the way we have
    //  to print it out to go to the lo-cirrf are different.
    //  The original C call is going to have inputs and outputs mixed
    //  while the invoke hardware call is going to have to have inputs
    //  followed by outputs.  All other transformations and 
    //  identifications work based upon the original C ordering, so we
    //  are going to have to just output them in the correct order.
    //  First, all inputs, then all outputs.

    for (unsigned int i = 0 ; i < s->get_argument_count() ; ++i)
    {
      Expression* nextArg = s->get_argument(i) ;
      if (dynamic_cast<SymbolAddressExpression*>(nextArg) == NULL)
      {
	// This is not a reference variable, so print it out first
	fout << ", " ;
	PrintExpression(s->get_argument(i)) ;
      }
    }
    for (unsigned int i = 0 ; i < s->get_argument_count() ; ++i)
    {
      Expression* nextArg = s->get_argument(i) ;
      if (dynamic_cast<SymbolAddressExpression*>(nextArg) != NULL)
      {
	// This is a reference variable, so print it out at the end
	SymbolAddressExpression* currentSymAddrExp = 
	  dynamic_cast<SymbolAddressExpression*>(nextArg) ;
	VariableSymbol* varSym = dynamic_cast<VariableSymbol*>(currentSymAddrExp->get_addressed_symbol()) ;
	fout << ", " << varSym->get_name() ;
      }
    }
    ++InvokeHardwareCounter ;    
  }
  else
  {
    // This is a built in.  We need to output a header for it.
    LString functionName = TempName(findRoccc) ;

    if (s->get_destination() != NULL)
    {
      rocccOut << StringType(s->get_destination()->get_type()) ;
    }
    else
    {
      rocccOut << "void" ;
    }

    rocccOut << " " << functionName << "(" ;
    for (unsigned int i = 0 ; i < s->get_argument_count() ; ++i)
    {
      if (i != 0) 
      {
	rocccOut << ", " ;
      }      
      rocccOut << StringType(cProcType->get_argument(i), true) ;
    }    
    rocccOut << ") ;" << std::endl ;     

    if (s->get_destination() != NULL)
    {
      fout << s->get_destination()->get_name() ;
      fout << " = " ;
    }
 
    fout << functionName << "(" ;
    // Print out the arguments to this call
    for (unsigned int i = 0 ; i < s->get_argument_count() ; ++i)
    {
      if (i != 0)
      {
	fout << ", " ;	
      }
      PrintExpression(s->get_argument(i)) ;
    }    
  }
  fout << ") ;" << std::endl ;
}

void HiCirrfGenerator::PrintStoreStatement(StoreStatement* s)
{
  PrintExpression(s->get_destination_address()) ;
  fout << " = " ;
  PrintExpression(s->get_value()) ;
  fout << " ;" << std::endl ;
}

void HiCirrfGenerator::PrintStoreVariableStatement(StoreVariableStatement* s)
{
  fout << s->get_destination()->get_name() ;
  fout << " = " ;
  PrintExpression(s->get_value()) ;
  fout << " ;" << std::endl ;
}

// For the base class, just print out the for statement as written
void HiCirrfGenerator::PrintForStatement(CForStatement* s)
{
  fout << "for (" ;
  PrintStatement(s->get_before()) ;
  fout << " " ;
  PrintExpression(s->get_test()) ;
  fout << " ; " ;
  
  // The step is a statement, most likely a store variable statement
  Statement* step = s->get_step() ;
  StoreVariableStatement* storeVarStep = 
    dynamic_cast<StoreVariableStatement*>(step) ;

  if (storeVarStep != NULL)
  {
    fout << storeVarStep->get_destination()->get_name() ;
    fout << " = " ;
    PrintExpression(storeVarStep->get_value()) ;
  }
  else
  {
    assert(0) ;
  }
  fout << ")" << std::endl << "{" << std::endl ;
  PrintStatement(s->get_body()) ;
  fout << "}" << std::endl ;
}

void HiCirrfGenerator::PrintBinaryExpression(BinaryExpression* e)
{
  fout << "(" ;
  PrintExpression(e->get_source1()) ;
  PrintOpcode(e->get_opcode()) ;
  PrintExpression(e->get_source2()) ;
  fout << ")" ;
}

void HiCirrfGenerator::PrintUnaryExpression(UnaryExpression* e)
{
  // Converts are special
  if (e->get_opcode() == LString("convert"))
  {
    PrintConvertExpression(e) ;
  }
  else
  {
    fout << "(" ;
    PrintOpcode(e->get_opcode()) ;
    PrintExpression(e->get_source()) ;
    fout << ")" ;
  }
}

void HiCirrfGenerator::PrintConvertExpression(UnaryExpression* e)
{
  DataType* myType = e->get_result_type() ;
  DataType* internalType = e->get_source()->get_result_type() ;
  assert(myType != NULL) ; 
  assert(internalType != NULL) ;

  // If either of these are reference types, just treat them as the
  //  type that is being referenced.
  ReferenceType* refType = dynamic_cast<ReferenceType*>(myType) ;
  
  while (refType != NULL)
  {
    QualifiedType* referencedType = 
      dynamic_cast<QualifiedType*>(refType->get_reference_type()) ;
    assert(referencedType != NULL) ;
    myType = referencedType->get_base_type() ;
    refType = dynamic_cast<ReferenceType*>(myType) ;
  }

  refType = dynamic_cast<ReferenceType*>(internalType) ;
  while (refType != NULL)
  {
    QualifiedType* referencedType = 
      dynamic_cast<QualifiedType*>(refType->get_reference_type()) ;
    assert(referencedType != NULL) ;
    internalType = referencedType->get_base_type() ;
    refType = dynamic_cast<ReferenceType*>(internalType) ;
  }

  IntegerType* outerInt = dynamic_cast<IntegerType*>(myType) ;
  IntegerType* innerInt = dynamic_cast<IntegerType*>(internalType) ;
  FloatingPointType* outerFloat = dynamic_cast<FloatingPointType*>(myType) ;
  FloatingPointType* innerFloat = 
    dynamic_cast<FloatingPointType*>(internalType) ;

  int destinationSize = myType->get_bit_size().c_int() ;

  if (outerInt != NULL && innerFloat != NULL)
  {
    fout << "ROCCCFPToInt" ;

  }
  else if (outerFloat != NULL && innerInt != NULL)
  {
    fout << "ROCCCIntToFP" ;
  }
  else if (outerInt != NULL && innerInt != NULL)
  {
    if (outerInt->get_bit_size().c_int() > 32)
    {
      fout << "ROCCCIntToInt64" ;
    }
    else
    {
      fout << "ROCCCIntToInt" ;
    }
  }
  else if (outerFloat != NULL && innerFloat != NULL)
  {
    fout << "ROCCCFPToFP" ;
  }
  else
  {
    assert(0 && "Unknown conversion") ;
  }

  fout << "(" ;
  PrintExpression(e->get_source()) ;
  fout << ", " ;
  fout << destinationSize ;
  fout << ")" ;
}

void HiCirrfGenerator::PrintSymbolAddressExpression(SymbolAddressExpression* e)
{
  fout << e->get_addressed_symbol()->get_name() ;
}

void HiCirrfGenerator::PrintAddressExpression(AddressExpression* e)
{
  fout << "&" ;
  PrintExpression(e->get_addressed_expression()) ;
}

// Overwritten by derived classes like modules output
void HiCirrfGenerator::PrintFieldAccessExpression(FieldAccessExpression* e)
{
  PrintExpression(e->get_base_group_address()) ;
  fout << "." << e->get_field()->get_name() ;
}

void HiCirrfGenerator::PrintArrayReferenceExpression(ArrayReferenceExpression* e)
{
  PrintExpression(e->get_base_array_address()) ;
  fout << "[" ;
  PrintExpression(e->get_index()) ;
  fout << "]" ;
}

void HiCirrfGenerator::PrintCallExpression(CallExpression* e)
{
  // For call expressions, we are either a boolean select or a conversion.
  //  For boolean selects we need to output a header.  For conversions, we
  //  explictily should not.
  SymbolAddressExpression* addressOfFunction = 
    dynamic_cast<SymbolAddressExpression*>(e->get_callee_address()) ;
  assert(addressOfFunction != NULL) ;
  ProcedureSymbol* procSym = 
    dynamic_cast<ProcedureSymbol*>(addressOfFunction->get_addressed_symbol()) ;
  assert(procSym != NULL) ;
  CProcedureType* cProcType = 
    dynamic_cast<CProcedureType*>(procSym->get_type()) ;
  assert(cProcType != NULL) ;

  std::string procName = (procSym->get_name()).c_str() ;
  
  if (procName.find("boolsel") != std::string::npos)
  {
    rocccOut << StringType(cProcType->get_result_type()) ;
    rocccOut << " " ;
    rocccOut << procSym->get_name() << "(" ;
    for (int i = 0 ; i < cProcType->get_argument_count() ; ++i)
    {
      if (i != 0)
      {
	rocccOut << ", " ;
      }
      rocccOut << StringType(cProcType->get_argument(i)) ;
    }
    rocccOut << ") ;" << std::endl ;
  }
  
  // Output the name of the function
  fout << dynamic_cast<SymbolAddressExpression*>(e->get_callee_address())->get_addressed_symbol()->get_name() ;

  fout << "(" ;
  Iter<Expression*> findArgs = e->get_argument_iterator() ;
  for (unsigned int i = 0 ; i < e->get_argument_count() ; ++i)
  {
    if (i != 0)
    {
      fout << ", " ;
    }
    PrintExpression(findArgs.current()) ;
    findArgs.next() ;
  }
  fout << ")" ;
}

void HiCirrfGenerator::PrintOpcode(String op)
{
  if (op == String("add"))
  {
    fout << " + " ;
  }
  if (op == String("subtract"))
  {
    fout << " - " ;
  }
  if (op == String("multiply"))
  {
    fout << " * " ;
  }
  if (op == String("divide"))
  {
    fout << " / " ;
  }
  if (op == String("remainder"))
  {
    fout << " % " ;
  }
  if (op == String("bitwise_and"))
  {
    fout << " & " ;
  }
  if (op == String("bitwise_or"))
  {
    fout << " | " ;
  }
  if (op == String("bitwise_xor"))
  {
    fout << " ^ " ;
  }
  if (op == String("left_shift"))
  {
    fout << " << " ;
  }
  if (op == String("right_shift"))
  {
    fout << " >> " ;
  }
  if (op == String("is_equal_to"))
  {
    fout << " == " ;
  }
  if (op == String("is_not_equal_to"))
  {
    fout << " != " ;
  }
  if (op == String("is_less_than"))
  {
    fout << " < " ;
  }
  if (op == String("is_less_than_or_equal_to"))
  {
    fout << " <= " ;
  }
  if (op == String("is_greater_than"))
  {
    fout << " > " ;
  }
  if (op == String("is_greater_than_or_equal_to"))
  {
    fout << " >= " ;
  }
  if (op == String("logical_and"))
  {
    fout << " && " ;
  }
  if (op == String("logical_or"))
  {
    fout << " || " ;
  }
  if (op == String("negate"))
  {
    fout << "-" ;
  }
  if (op == String("invert"))
  {
    // Invert is the same as bitwise not
    fout << "~" ;
  }
  if (op == String("bitwise_not"))
  {
    fout << "~" ;
  }
  if (op == String("logical_not"))
  {
    fout << "!" ;
  }
  if (op == String("convert"))
  {
    // Don't print anything out    
  }
}

void HiCirrfGenerator::OutputFake(VariableSymbol* v, int dimensionality)
{
  std::string typeName ;
  DataType* baseType = v->get_type()->get_base_type() ;

  while (dynamic_cast<ReferenceType*>(baseType) != NULL)
  {
    Type* curType =
      dynamic_cast<ReferenceType*>(baseType)->get_reference_type() ;
    if (dynamic_cast<QualifiedType*>(curType) != NULL)
    {
      curType = dynamic_cast<QualifiedType*>(curType)->get_base_type() ;
    }
    baseType = dynamic_cast<DataType*>(curType) ;
  }
  assert(baseType != NULL) ;

  if (dynamic_cast<PointerType*>(baseType) != NULL)
  {
    OutputPointerFake(v) ;    
    return ;
  }

  while (dynamic_cast<ArrayType*>(baseType) != NULL)
  {
    baseType = 
      dynamic_cast<ArrayType*>(baseType)->get_element_type()->get_base_type() ;
  }
  
  //  if (dynamic_cast<FloatingPointType*>(baseType) != NULL)
  //  {
  //    typeName = "float" ;
  //  }
  //  else if (dynamic_cast<IntegerType*>(baseType) != NULL)
  //  {
  //    typeName = "int" ;
  //  }
  //  else
  //  {
  //    assert(0) ;
  //  }
  

  fout << v->get_name() << " = *((" ;
  //fout << typeName ;
  fout << StringType(baseType) ;
  for (int i = 0 ; i < dimensionality ; ++i)
  {
    fout << "*" ;
  }
  fout << "*)(0)) ;" << std::endl ;
}

void HiCirrfGenerator::OutputPointerFake(VariableSymbol* v)
{
  //  std::string typeName ;
  DataType* baseType = v->get_type()->get_base_type() ;

  while (dynamic_cast<ReferenceType*>(baseType) != NULL)
  {
    Type* curType =
      dynamic_cast<ReferenceType*>(baseType)->get_reference_type() ;
    if (dynamic_cast<QualifiedType*>(curType) != NULL)
    {
      curType = dynamic_cast<QualifiedType*>(curType)->get_base_type() ;
    }
    baseType = dynamic_cast<DataType*>(curType) ;
  }
  assert(baseType != NULL) ;

  PointerType* pType = dynamic_cast<PointerType*>(baseType) ;
  assert(pType != NULL) ;
  QualifiedType* refType = 
    dynamic_cast<QualifiedType*>(pType->get_reference_type()) ;
  assert(refType != NULL) ;
  
  int indirection = 1 ;
  while (dynamic_cast<PointerType*>(refType->get_base_type()) != NULL)
  {
    pType = dynamic_cast<PointerType*>(refType->get_base_type()) ;
    refType = 
      dynamic_cast<QualifiedType*>(pType->get_reference_type()) ;
    assert(refType != NULL) ;
    ++indirection ;
  }

  fout << v->get_name() << " = *((" ;
  fout << StringType(refType) ;
  for (int i = 0 ; i < indirection ; ++i)
  {
    fout << "*" ;
  }

  fout << "*)(0)) ; " << std::endl ;
}

void HiCirrfGenerator::OutputSizes()
{
  assert(symTab != NULL) ;
  OutputSizesWorkhorse(symTab) ;
}

void HiCirrfGenerator::OutputSizesWorkhorse(SymbolTable* currentSymTab)
{
  assert(currentSymTab != NULL) ;
  
  for (int i = 0 ; i < currentSymTab->get_symbol_table_object_count() ; ++i) 
  {
    SymbolTableObject* nextObj = currentSymTab->get_symbol_table_object(i) ;
    VariableSymbol* nextVar = dynamic_cast<VariableSymbol*>(nextObj) ;

    int totalSize ;
    if (nextVar == NULL)
    {
      // Don't do anything
      continue ;
    }
    
    Annote* constAnnote = nextObj->lookup_annote_by_name("ConstPropArray") ;
    if (constAnnote != NULL)
    {
      // If the array has been propagated, don't output a size for it
      continue ;
    }

    Annote* removedAnnote = nextObj->lookup_annote_by_name("RemovedVariable") ;
    if (removedAnnote != NULL)
    {
      // Some variables are removed but still declared when we perform 
      //  systolic array generation.  These must not have sizes printed
      //  out.
      continue ;
    }

    DataType* baseType = nextVar->get_type()->get_base_type() ;

    // The size I output for Lookup tables should be the size of the
    //  elements, not the total size of the lookup table.  This is the
    //  same as all of the other Array types.

    if (dynamic_cast<ArrayType*>(baseType) != NULL)
    {
      // Total size should be the element size
      Type* nextType = dynamic_cast<ArrayType*>(baseType)->get_element_type() ;
      if (dynamic_cast<QualifiedType*>(nextType) != NULL)
      {
	nextType = dynamic_cast<QualifiedType*>(nextType)->get_base_type() ;
      }
      while (dynamic_cast<ArrayType*>(nextType) != NULL)
      {
	nextType = dynamic_cast<ArrayType*>(nextType)->get_element_type() ;
	if (dynamic_cast<QualifiedType*>(nextType) != NULL)
	{
	  nextType = dynamic_cast<QualifiedType*>(nextType)->get_base_type() ;
	}
      }
      totalSize = dynamic_cast<DataType*>(nextType)->get_bit_size().c_int() ;
    }
    else if (dynamic_cast<StructType*>(baseType) != NULL)
    {
      // If the variable is a struct, we need to print out the sizes of
      //  all the internal symbols.  I should do this by recursively calling
      //  the workhorse function.
      SymbolTable* structTable = 
	dynamic_cast<StructType*>(baseType)->get_group_symbol_table() ;
      OutputSizesWorkhorse(structTable) ;
      continue ;
    }
    else
    {
      totalSize = baseType->get_bit_size().c_int() ;
    }

    fout << "ROCCCSize(" ;
    fout << totalSize ;
    fout << ", " ;
    fout << nextVar->get_name() ;
    fout << ") ;" << std::endl ;

    // Also, print out the signedness of the variable
    fout << "ROCCCSigned(" ;

    DataType* signedType = nextVar->get_type()->get_base_type() ;
    while (dynamic_cast<ArrayType*>(signedType) != NULL && signedType != NULL)
    {
      signedType = dynamic_cast<ArrayType*>(signedType)->get_element_type()->get_base_type() ;
    }
    assert(signedType != NULL) ;
    IntegerType* intType = dynamic_cast<IntegerType*>(signedType) ;
    FloatingPointType* floatType = 
      dynamic_cast<FloatingPointType*>(signedType) ;
    
    // 1 bit numbers should not be signed no matter what
    if (intType != NULL && intType->get_is_signed() && 
	intType->get_bit_size().c_int() != 1)
    {
      fout << "1" ;
    }
    else if (floatType != NULL)
    {
      fout << "1" ;
    }
    else
    {
      fout << "0" ;
    }
    fout << ", " ;
    fout << nextVar->get_name() ;
    fout << ") ;" << std::endl ;
    
    // Also, print out the name for the variable
    fout << "ROCCCName(" ;
    fout << "\"" << nextVar->get_name() << "\"" ;
    fout << ", " ;
    fout << nextVar->get_name() ;
    fout << ") ;" << std::endl ;
  }
}

String HiCirrfGenerator::StringType(Type* t, bool convertArraysToPointers)
{
  String toReturn = "" ;
  if (dynamic_cast<ArrayType*>(t) != NULL)
  {
    toReturn += StringType(dynamic_cast<ArrayType*>(t)->get_element_type(), 
			   convertArraysToPointers) ;
    if (convertArraysToPointers)
    {
      toReturn += "*" ;
    }
    else
    {
      toReturn += "[]" ;
    }
  }
  else if (dynamic_cast<MultiDimArrayType*>(t) != NULL)
  {
    assert(0 && "MultiDimArrayType should not have been generated!") ;
  }
  else if (dynamic_cast<DataType*>(t) != NULL)
  {
    if (dynamic_cast<IntegerType*>(t) != NULL)
    {
      // All integer types are ints with their size specified
      //  by the ROCCCSize function.  Unfortunately some shifts don't work
      //  correctly unless I output long long in some instances.
      if (dynamic_cast<IntegerType*>(t)->get_bit_size().c_int() > 32)
      {
	toReturn += "long long" ;
      }
      else
      {
	toReturn += "int" ;
      }
    }
    if (dynamic_cast<FloatingPointType*>(t) != NULL)
    {    
      // All floating point types are floats with their size 
      //  specified by the ROCCCSize function
      toReturn += "float" ; 
    }
    if (dynamic_cast<GroupType*>(t) != NULL)
    {
      toReturn += t->get_name() ;
    }
    if (dynamic_cast<PointerType*>(t) != NULL)
    {
      PointerType* pType = dynamic_cast<PointerType*>(t) ;
      toReturn += StringType(pType->get_reference_type(),
			     convertArraysToPointers) ;
      toReturn += "*" ;
    }
    if (dynamic_cast<BooleanType*>(t) != NULL)
    {
      // C does not have boolean types
      toReturn += "int" ;
    }
    if (dynamic_cast<ReferenceType*>(t) != NULL)
    {
      toReturn += StringType(dynamic_cast<ReferenceType*>(t)->get_reference_type()) ;
    }
  }
  else if (dynamic_cast<ProcedureType*>(t) != NULL)
  {
    assert(0 && "Procedure type should not be output!") ;
  }
  else if (dynamic_cast<QualifiedType*>(t) != NULL)
  {
    QualifiedType* qType = dynamic_cast<QualifiedType*>(t) ;
    toReturn += StringQualifications(qType) ;
    toReturn += StringType(qType->get_base_type(),
			   convertArraysToPointers) ;
  }
  else
  {
    assert(0 && "Unknown type detected") ;
  }
  return toReturn ;
}

String HiCirrfGenerator::StringQualifications(QualifiedType* qType)
{
  String toReturn = "" ;
  for (int i = 0 ; i < qType->get_qualification_count() ; ++i)
  {
    toReturn += qType->get_qualification(i) ;
    toReturn += " " ;
  }
  return toReturn ;
}

// Functions that pass information to the lo-cirrf side
void HiCirrfGenerator::OutputMaximizePrecision()
{
  assert(procDef != NULL) ;
  return ;

  // This is no longer handled at this stage, but instead is a 
  //  user selectable optimization that creates a script for the 
  //  lo-cirrf side.
}

void HiCirrfGenerator::OutputCompilerVersion()
{
  Annote* versionAnnote = procDef->lookup_annote_by_name("CompilerVersion") ;
  BrickAnnote* versionBrickAnnote = dynamic_cast<BrickAnnote*>(versionAnnote) ;
  assert(versionBrickAnnote != NULL) ;
  StringBrick* versionStringBrick = 
    dynamic_cast<StringBrick*>(versionBrickAnnote->get_brick(0)) ;
  assert(versionStringBrick != NULL) ;
  fout << "ROCCCCompilerVersion(\"" ;
  fout << versionStringBrick->get_value() ;
  fout << "\") ;" << std::endl ;
}

void HiCirrfGenerator::OutputDebugRegisters()
{
  OutputDebugRegistersWorkhorse(symTab) ;
}

void HiCirrfGenerator::OutputDebugRegistersWorkhorse(SymbolTable* currentSymTab)
{
  for (int i = 0 ; i < currentSymTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* currentObj = currentSymTab->get_symbol_table_object(i) ;
    assert(currentObj != NULL) ;
    if (currentObj->lookup_annote_by_name("DebugRegister") != NULL)
    {
      BrickAnnote* watchAnnote = dynamic_cast<BrickAnnote*>
	(currentObj->lookup_annote_by_name("WatchPoint")) ;
      BrickAnnote* validAnnote = dynamic_cast<BrickAnnote*>
	(currentObj->lookup_annote_by_name("WatchValid")) ;
      assert(watchAnnote != NULL) ;
      assert(validAnnote != NULL) ;
      IntegerBrick* watchBrick = 
	dynamic_cast<IntegerBrick*>(watchAnnote->get_brick(0)) ;
      IntegerBrick* validBrick =
	dynamic_cast<IntegerBrick*>(validAnnote->get_brick(0)) ;
      assert(watchBrick != NULL) ;
      assert(validBrick != NULL) ;

      fout << "ROCCCDebugScalarOutput(" ;
      fout << currentObj->get_name() ;
      fout << ", " ;
      fout << watchBrick->get_value().c_int() ;
      fout << ", " ;
      fout << validBrick->get_value().c_int() ;
      fout << ") ;" << std::endl ;
    }
    VariableSymbol* varSym = dynamic_cast<VariableSymbol*>(currentObj) ;
    if (varSym != NULL)
    {
      StructType* recurseType = 
	dynamic_cast<StructType*>(varSym->get_type()->get_base_type()) ;
      if (recurseType != NULL)
      {
	OutputDebugRegistersWorkhorse(recurseType->get_group_symbol_table()) ;
      }
    }
  }
}

void HiCirrfGenerator::OutputFooter()
{
  fout << "return ; " << std::endl ;
  fout << "}" << std::endl ;
}

// Helper functions
DataType* HiCirrfGenerator::GetBaseType(Type* top)
{
  if (dynamic_cast<QualifiedType*>(top) != NULL)
  {
    return GetBaseType(dynamic_cast<QualifiedType*>(top)->get_base_type()) ;
  }
  if (dynamic_cast<ArrayType*>(top) != NULL)
  {
    return GetBaseType(dynamic_cast<ArrayType*>(top)->get_element_type()) ;
  }
  if (dynamic_cast<DataType*>(top) != NULL)
  {
    return dynamic_cast<DataType*>(top) ;
  }
  assert(0 && "Unknown type!") ;
  return NULL ; // To avoid warnings
}

bool HiCirrfGenerator::InInputList(VariableSymbol* value)
{
  std::list<VariableSymbol*>::iterator checkIter = inputs.begin() ;
  while (checkIter != inputs.end())
  {
    if ((*checkIter) == value)
    {
      return true ;
    }
    ++checkIter ;
  }
  return false ;
}

bool HiCirrfGenerator::FloatFormat(String s)
{
  // Assume the string is a number.  Look for a period or "e".
  if ((s.find(String(".")) != -1) || (s.find(String("e")) != -1))
  {
    return true ;
  }
  return false ;
}
