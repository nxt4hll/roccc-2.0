// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#include <cassert>

#include "verifier.h"

SuifTreeVerifier::SuifTreeVerifier()
{
  ; // Nothing to do here yet
}

SuifTreeVerifier::~SuifTreeVerifier()
{
  ; // Nothing to delete either...
}

void SuifTreeVerifier::VerifyFSB(FileSetBlock* fsb)
{
  assert(fsb != NULL) ;
  assert(fsb->get_external_symbol_table() != NULL) ;
  assert(fsb->get_file_set_symbol_table() != NULL) ;

  VerifySymbolTable(fsb->get_external_symbol_table()) ;
  VerifySymbolTable(fsb->get_file_set_symbol_table()) ;

  for (int i = 0 ; i < fsb->get_file_block_count() ; ++i)
  {
    FileBlock* fb = fsb->get_file_block(i) ;
    assert(fb != NULL) ;
    VerifyFileBlock(fb) ;
  }

  for (int i = 0 ; i < fsb->get_information_block_count() ; ++i)
  {
    GlobalInformationBlock* gb = fsb->get_information_block(i) ;
    assert(gb != NULL) ;
    VerifyInformationBlock(gb) ;
  }
}

void SuifTreeVerifier::PrintFSB(FileSetBlock* fsb)
{
  assert(fsb != NULL) ;
  
  std::cerr << "External symbol table: " << std::endl ;
  PrintSymbolTable(fsb->get_external_symbol_table()) ;
  std::cerr << "File set symbol table: " << std::endl ;
  PrintSymbolTable(fsb->get_file_set_symbol_table()) ;

  for (int i = 0 ; i < fsb->get_file_block_count() ; ++i)
  {
    std::cerr << "File Block " << i << std::endl ;
    FileBlock* fb = fsb->get_file_block(i) ;
    assert(fb != NULL) ;
    PrintFileBlock(fb) ;
  }

  for (int i = 0 ; i < fsb->get_information_block_count() ; ++i)
  {
    std::cerr << "Information block " << i << std::endl ;
    GlobalInformationBlock* gb = fsb->get_information_block(i) ;
    assert(gb != NULL) ;
    PrintInformationBlock(gb) ;
  }
}

void SuifTreeVerifier::VerifySymbolTable(SymbolTable* symTab) 
{
  assert(symTab != NULL) ;
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* s = symTab->get_symbol_table_object(i) ;
    assert(s != NULL) ;
    VerifyAnnotations(s) ;
  }
}

void SuifTreeVerifier::PrintSymbolTable(SymbolTable* symTab)
{
  assert(symTab != NULL) ;
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* s = symTab->get_symbol_table_object(i) ;
    std::cerr << "Symbol: " << s->get_name() << std::endl ;
    PrintAnnotations(s) ;
  }
}

void SuifTreeVerifier::VerifyFileBlock(FileBlock* fb)
{
  assert(fb != NULL) ;
  SymbolTable* symTab = fb->get_symbol_table() ;
  assert(symTab != NULL) ;
  VerifySymbolTable(symTab) ;

  DefinitionBlock* db = fb->get_definition_block() ;
  assert(db != NULL) ;
  VerifyDefinitionBlock(db) ;

}

void SuifTreeVerifier::PrintFileBlock(FileBlock* fb)
{
  assert(fb != NULL) ;
  std::cerr << "Symbol Table: " << std::endl ;
  PrintSymbolTable(fb->get_symbol_table()) ;
  std::cerr << "Definition Block: " << std::endl ;
  PrintDefinitionBlock(fb->get_definition_block()) ;
}

void SuifTreeVerifier::VerifyInformationBlock(GlobalInformationBlock* gb)
{
  assert(gb != NULL) ;
  TargetInformationBlock* tb = dynamic_cast<TargetInformationBlock*>(gb) ;
  CInformationBlock* cb = dynamic_cast<CInformationBlock*>(gb) ;
  
  if (tb != NULL)
  {
    assert(tb->get_word_type() != NULL) ;
    assert(tb->get_default_boolean_type() != NULL) ;
    assert(tb->get_default_void_type() != NULL) ;

    for (int i = 0 ; i < tb->get_integer_type_count() ; ++i)
    {
      assert(tb->get_integer_type(i) != NULL) ;
    }
    for (int i = 0 ; i < tb->get_floating_point_type_count() ; ++i)
    {
      assert(tb->get_floating_point_type(i) != NULL) ;
    }
  }
  else if (cb != NULL)
  {
    assert(cb->get_signed_char_type() != NULL) ;
    assert(cb->get_unsigned_char_type() != NULL) ;
    assert(cb->get_char_type() != NULL) ;
    assert(cb->get_signed_short_type() != NULL) ;
    assert(cb->get_unsigned_short_type() != NULL) ;
    assert(cb->get_signed_int_type() != NULL) ;
    assert(cb->get_unsigned_int_type() != NULL) ;
    assert(cb->get_signed_long_type() != NULL) ;
    assert(cb->get_unsigned_long_type() != NULL) ;
    assert(cb->get_signed_long_long_type() != NULL) ;
    assert(cb->get_unsigned_long_long_type() != NULL) ;
    assert(cb->get_float_type() != NULL) ;
    assert(cb->get_double_type() != NULL) ;
    assert(cb->get_long_double_type() != NULL) ;
    assert(cb->get_file_type() != NULL) ;
    assert(cb->get_ptrdiff_t_type() != NULL) ;
    assert(cb->get_size_t_type() != NULL) ;
    assert(cb->get_va_list_type() != NULL) ;
  }
  else
  {
    assert(0) ;
  }
}

void SuifTreeVerifier::PrintInformationBlock(GlobalInformationBlock* gb)
{
  assert(gb != NULL) ;
  TargetInformationBlock* tb = dynamic_cast<TargetInformationBlock*>(gb) ;
  CInformationBlock* cb = dynamic_cast<CInformationBlock*>(gb) ;
  
  if (tb != NULL)
  {
    std::cerr << "Target information block" << std::endl ;
  }
  else if (cb != NULL)
  {
    std::cerr << "C Information Block" << std::endl ;
  }
  else
  {
    assert(0) ;
  }
}

void SuifTreeVerifier::VerifyAnnotations(AnnotableObject* a) 
{
  assert(a != NULL) ;
  for (int i = 0 ; i < a->get_annote_count() ; ++i)
  {
    Annote* current = a->get_annote(i) ;
    assert(current != NULL) ;
    BrickAnnote* brick = dynamic_cast<BrickAnnote*>(current) ;
    if (brick != NULL) 
    {
      std::cerr << "Brick Annote: " << brick->get_name() << std::endl ;
      VerifyBricks(brick) ;
    }
  }
}

void SuifTreeVerifier::PrintAnnotations(AnnotableObject* a)
{
  for (int i = 0 ; i < a->get_annote_count() ; ++i)
  {
    Annote* current = a->get_annote(i) ;
    std::cerr << "Annotation: " << current->get_name() << std::endl ;
  }
}

void SuifTreeVerifier::VerifyBricks(BrickAnnote* b)
{
  assert(b != NULL) ;
  for (int i = 0 ; i < b->get_brick_count() ; ++i)
  {
    SuifBrick* sb = b->get_brick(i) ;
    assert(sb != NULL) ;
    StringBrick* strBrick = dynamic_cast<StringBrick*>(sb) ;
    IntegerBrick* intBrick = dynamic_cast<IntegerBrick*>(sb) ;
    SuifObjectBrick* sobBrick = dynamic_cast<SuifObjectBrick*>(sb) ;
    OwnedSuifObjectBrick* osobBrick = dynamic_cast<OwnedSuifObjectBrick*>(sb) ;

    if (sobBrick != NULL)
    {
      assert(sobBrick->get_object() != NULL) ;
    }
    else if (osobBrick != NULL)
    {
      assert(osobBrick->get_object() != NULL) ;
    }
    else if (strBrick == NULL && intBrick == NULL)
    {
      assert(0) ;
    }
  }
}

void SuifTreeVerifier::PrintBricks(BrickAnnote* b) 
{
}

void SuifTreeVerifier::VerifyDefinitionBlock(DefinitionBlock* db)
{
  assert(db != NULL) ;

  for (int i = 0 ; i < db->get_variable_definition_count() ; ++i)
  {
    VariableDefinition* v = db->get_variable_definition(i) ;
    assert(v != NULL) ;
    VerifyVariableDefinition(v) ;
  }

  for (int i = 0 ; i < db->get_procedure_definition_count() ; ++i)
  {
    ProcedureDefinition* p = db->get_procedure_definition(i) ;
    assert(p != NULL) ;
    VerifyProcedureDefinition(p) ;
  }  
}

void SuifTreeVerifier::PrintDefinitionBlock(DefinitionBlock* db)
{
  for (int i = 0 ; i < db->get_variable_definition_count() ; ++i)
  {
    VariableDefinition* v = db->get_variable_definition(i) ;
    PrintVariableDefinition(v) ;
  }
  for (int i = 0 ; i < db->get_procedure_definition_count() ; ++i)
  {
    PrintProcedureDefinition(db->get_procedure_definition(i)) ;
  }
}

void SuifTreeVerifier::VerifyVariableDefinition(VariableDefinition* v)
{
  assert(v != NULL) ;
  assert(v->get_variable_symbol() != NULL) ;
  assert(v->get_initialization() != NULL) ;
}

void SuifTreeVerifier::PrintVariableDefinition(VariableDefinition* v)
{
  std::cerr << "Variable definition: " << std::endl ;
  std::cerr << v->get_variable_symbol()->get_name() << " = " ;
  //PrintExpression(v->get_initialization()) ;
  std::cerr << std::endl ;
}

void SuifTreeVerifier::VerifyProcedureDefinition(ProcedureDefinition* p)
{
  assert(p != NULL) ;
  assert(p->get_procedure_symbol() != NULL) ;
  assert(p->get_body() != NULL) ;
  VerifyExecutionObject(p->get_body()) ;
  assert(p->get_symbol_table() != NULL) ;
  VerifySymbolTable(p->get_symbol_table()) ;
  assert(p->get_definition_block() != NULL) ;
  VerifyDefinitionBlock(p->get_definition_block()) ;
  
  for (int i = 0 ; i < p->get_formal_parameter_count() ; ++i)
  {
    assert(p->get_formal_parameter(i) != NULL) ;
  }
}

void SuifTreeVerifier::PrintProcedureDefinition(ProcedureDefinition* p)
{
  assert(p != NULL) ;
  PrintSymbol(p->get_procedure_symbol()) ;
  std::cerr << "{" << std::endl ;
  PrintExecutionObject(p->get_body()) ;
  std::cerr << "}" << std::endl ;
  
  for (int i = 0 ; i < p->get_formal_parameter_count() ; ++i)
  {
    std::cerr << "Parameter " << i << " " ;
    PrintSymbol(p->get_formal_parameter(i)) ;
    std::cerr << std::endl ;
  }
}

void SuifTreeVerifier::VerifyExecutionObject(ExecutionObject* e)
{
  assert(e != NULL) ;
  Statement* s = dynamic_cast<Statement*>(e) ;
  Expression* exp = dynamic_cast<Expression*>(e) ;
  if (s != NULL)
  {
    VerifyStatement(s) ;
  }
  else if (exp != NULL)
  {
    VerifyExpression(exp) ;
  }
  else
  {
    assert(0) ;
  }
}

void SuifTreeVerifier::PrintExecutionObject(ExecutionObject* e) 
{
  Statement* s = dynamic_cast<Statement*>(e) ;
  Expression* exp = dynamic_cast<Expression*>(e) ;
  if (s != NULL)
  {
    PrintStatement(s) ;
  }
  else if (exp != NULL) 
  {
    PrintExpression(exp) ;
  }
  else
  {
    assert(0) ;
  }
}

void SuifTreeVerifier::VerifyStatement(Statement* s)
{
  assert(s != NULL) ;

  VerifyAnnotations(s) ;
  PrintAnnotations(s) ;

  StatementList* stmtList = dynamic_cast<StatementList*>(s) ;
  EvalStatement* eval = dynamic_cast<EvalStatement*>(s) ;
  CallStatement* call = dynamic_cast<CallStatement*>(s) ;
  IfStatement* ifStmt = dynamic_cast<IfStatement*>(s) ;
  CForStatement* forStmt = dynamic_cast<CForStatement*>(s) ;
  StoreStatement* store = dynamic_cast<StoreStatement*>(s) ;
  StoreVariableStatement* storeVar = dynamic_cast<StoreVariableStatement*>(s) ;
  LabelLocationStatement* label = dynamic_cast<LabelLocationStatement*>(s) ;
  MarkStatement* mark = dynamic_cast<MarkStatement*>(s) ;
  
  if (stmtList != NULL)
  {
    std::cerr << "Verifying statement list" << std::endl ;
    for (int i = 0 ; i < stmtList->get_statement_count() ; ++i)
    {
      VerifyStatement(stmtList->get_statement(i)) ;
    }
  }
  else if (eval != NULL)
  {
    for (int i = 0 ; i < eval->get_expression_count() ; ++i)
    {
      VerifyExpression(eval->get_expression(i)) ;
    }
  }
  else if (call != NULL)
  {
    VerifyExpression(call->get_callee_address()) ;
    for (int i = 0 ; i < call->get_argument_count() ; ++i)
    {
      VerifyExpression(call->get_argument(i)) ;
    }
  }
  else if (ifStmt != NULL)
  {
    std::cerr << "Verifying an if statement" << std::endl ;
    VerifyExpression(ifStmt->get_condition()) ;
    VerifyStatement(ifStmt->get_then_part()) ;
    if (ifStmt->get_else_part() != NULL)
    {
      VerifyStatement(ifStmt->get_else_part()) ;
    }
  }
  else if (forStmt != NULL)
  {
    VerifyStatement(forStmt->get_before()) ;
    VerifyExpression(forStmt->get_test()) ;
    VerifyStatement(forStmt->get_step()) ;
    VerifyStatement(forStmt->get_body()) ;
  }
  else if (store != NULL)
  {
    VerifyExpression(store->get_value()) ;
    VerifyExpression(store->get_destination_address()) ;
  }
  else if (storeVar != NULL)
  {
    std::cerr << "Verifying a store variable statement: " 
	      << storeVar->get_destination()->get_name()
	      << std::endl ;
    assert(storeVar->get_destination() != NULL) ;
    VerifyExpression(storeVar->get_value()) ;
  }
  else if (label != NULL)
  {
    assert(label->get_defined_label() != NULL) ;
  }
  else if (mark != NULL)
  {
  }
  else
  {
    assert(0) ;
  }

}

void SuifTreeVerifier::PrintStatement(Statement* s)
{
  StatementList* stmtList = dynamic_cast<StatementList*>(s) ;
  EvalStatement* eval = dynamic_cast<EvalStatement*>(s) ;
  CallStatement* call = dynamic_cast<CallStatement*>(s) ;
  IfStatement* ifStmt = dynamic_cast<IfStatement*>(s) ;
  CForStatement* forStmt = dynamic_cast<CForStatement*>(s) ;
  StoreStatement* store = dynamic_cast<StoreStatement*>(s) ;
  StoreVariableStatement* storeVar = dynamic_cast<StoreVariableStatement*>(s) ;
  LabelLocationStatement* label = dynamic_cast<LabelLocationStatement*>(s) ;
  MarkStatement* mark = dynamic_cast<MarkStatement*>(s) ;
  
  if (stmtList != NULL)
  {
    std::cerr << "{" << std::endl ;
    for (int i = 0 ; i < stmtList->get_statement_count() ; ++i)
    {
      PrintStatement(stmtList->get_statement(i)) ;
    }
    std::cerr << "}" << std::endl ;
  }
  else if (eval != NULL)
  {
    for (int i = 0 ; i < eval->get_expression_count() ; ++i)
    {
      if (i != 0)
	std::cerr << ", " ;
      PrintExpression(eval->get_expression(i)) ;
    }
    std::cerr << "; " << std::endl ;
  }
  else if (call != NULL)
  {    
    PrintExpression(call->get_callee_address()) ;
    std::cerr << "(" ;
    for (int i = 0 ; i < call->get_argument_count() ; ++i)
    {
      if (i != 0)
      {
	std::cerr << ", " ;
      }
      PrintExpression(call->get_argument(i)) ;
    }
    std::cerr << ") ; " << std::endl ;
  }
  else if (ifStmt != NULL)
  {
    std::cerr << "if (" ;
    PrintExpression(ifStmt->get_condition()) ;
    std::cerr << ")" << std::endl << "{" << std::endl ;
    PrintStatement(ifStmt->get_then_part()) ;
    std::cerr << "}" << std::endl ;
    if (ifStmt->get_else_part() != NULL)
    {
      std::cerr << "else\n{" << std::endl ;
      PrintStatement(ifStmt->get_else_part()) ;
      std::cerr << "}" << std::endl ;
    }
  }
  else if (forStmt != NULL)
  {
    std::cerr << "for (" ;
    PrintStatement(forStmt->get_before()) ;
    PrintExpression(forStmt->get_test()) ;
    std::cerr << ";" ;
    PrintStatement(forStmt->get_step()) ;
    std::cerr << ")\n{" << std::endl ;
    PrintStatement(forStmt->get_body()) ;
    std::cerr << "}" << std::endl ;
  }
  else if (store != NULL)
  {
    PrintExpression(store->get_destination_address()) ;
    std::cerr << " = " ;
    PrintExpression(store->get_value()) ;
    std::cerr << ";" << std::endl ;
  }
  else if (storeVar != NULL)
  {
    PrintSymbol(storeVar->get_destination()) ;
    std::cerr << " = " ;
    PrintExpression(storeVar->get_value()) ;
    std::cerr << " ; " << std::endl ;
  }
  else if (label != NULL)
  {
    assert(label->get_defined_label() != NULL) ;
  }
  else if (mark != NULL)
  {
  }
  else
  {
    assert(0) ;
  }

  PrintAnnotations(s) ;
}

void SuifTreeVerifier::VerifyExpression(Expression* e) 
{
  assert(e != NULL) ;

  VerifyAnnotations(e) ;
  PrintAnnotations(e) ;

  Constant* con = dynamic_cast<Constant*>(e) ;
  BinaryExpression* bin = dynamic_cast<BinaryExpression*>(e) ;
  UnaryExpression* un = dynamic_cast<UnaryExpression*>(e) ;
  ArrayReferenceExpression* arrayRef = 
    dynamic_cast<ArrayReferenceExpression*>(e) ;
  FieldAccessExpression* field = dynamic_cast<FieldAccessExpression*>(e) ;
  LoadExpression* load = dynamic_cast<LoadExpression*>(e) ;
  LoadVariableExpression* loadVar = dynamic_cast<LoadVariableExpression*>(e) ;
  SymbolAddressExpression* symAddr = dynamic_cast<SymbolAddressExpression*>(e);
  AddressExpression* addr = dynamic_cast<AddressExpression*>(e) ;
  NonLvalueExpression* nonL = dynamic_cast<NonLvalueExpression*>(e) ;
  CallExpression* call = dynamic_cast<CallExpression*>(e) ;

  if (con != NULL)
  {
    std::cerr << "Verifying a constant" << std::endl ;
    IntConstant* intCon = dynamic_cast<IntConstant*>(con) ;
    FloatConstant* floatCon = dynamic_cast<FloatConstant*>(con) ;
    if (intCon != NULL)
    {
      std::cerr << "Int constant: " << intCon->get_value().c_int() 
		<< std::endl ;
    }
    else if (floatCon != NULL)
    {
      std::cerr << "Float constant: " << floatCon->get_value() << std::endl ;
    }
    else
    {
      assert(0) ;
    }
  }
  else if (bin != NULL)
  {
    std::cerr << "Verifying a binary expression" << std::endl ;
    VerifyExpression(bin->get_source1()) ;
    VerifyExpression(bin->get_source2()) ;
  }
  else if (un != NULL)
  {
    std::cerr << "Verifying a unary expression" << std::endl ;
    VerifyExpression(un->get_source()) ;
  }
  else if (arrayRef != NULL)
  {
    std::cerr << "Verifying an array reference" << std::endl ;
    VerifyExpression(arrayRef->get_base_array_address()) ;
    VerifyExpression(arrayRef->get_index()) ;
  }
  else if (field != NULL)
  {
    std::cerr << "Verifying a field access" << std::endl ;
    assert(field->get_field() != NULL) ;
    VerifyExpression(field->get_base_group_address()) ;
  }
  else if (load != NULL)
  {
    std::cerr << "Verifying a load expression" << std::endl ;
    VerifyExpression(load->get_source_address()) ;
  }
  else if (loadVar != NULL)
  {
    std::cerr << "Verifying a load variable expression: " 
	      << loadVar->get_source()->get_name() 
	      << std::endl ;
    VerifySymbol(loadVar->get_source()) ;
  }
  else if (symAddr != NULL)
  {
    std::cerr << "Verifying a symbol address expression" << std::endl ;
    assert(symAddr->get_addressed_symbol() != NULL) ;
  }
  else if (addr != NULL)
  {
    std::cerr << "Verifying an address expression" << std::endl ;
    VerifyExpression(addr->get_addressed_expression()) ;
  }
  else if (nonL != NULL)
  {
    std::cerr << "Verifying a non L value expression" << std::endl ;
    VerifyExpression(nonL->get_addressed_expression()) ;
  }
  else if (call != NULL)
  {
    std::cerr << "Verifying a call expression" << std::endl ;
    VerifyExpression(call->get_callee_address()) ;
    for (int i = 0 ; i < call->get_argument_count() ; ++i)
    {
      VerifyExpression(call->get_argument(i)) ;
    }
  }
  else
  {
    FormattedText tmpText ;
    e->print(tmpText) ;
    std::cerr << tmpText.get_value() << std::endl ;
    assert(0) ;
  }
}

void SuifTreeVerifier::PrintExpression(Expression* e)
{
  Constant* con = dynamic_cast<Constant*>(e) ;
  BinaryExpression* bin = dynamic_cast<BinaryExpression*>(e) ;
  UnaryExpression* un = dynamic_cast<UnaryExpression*>(e) ;
  ArrayReferenceExpression* arrayRef = 
    dynamic_cast<ArrayReferenceExpression*>(e) ;
  FieldAccessExpression* field = dynamic_cast<FieldAccessExpression*>(e) ;
  LoadExpression* load = dynamic_cast<LoadExpression*>(e) ;
  LoadVariableExpression* loadVar = dynamic_cast<LoadVariableExpression*>(e) ;
  SymbolAddressExpression* symAddr = dynamic_cast<SymbolAddressExpression*>(e);
  AddressExpression* addr = dynamic_cast<AddressExpression*>(e) ;
  NonLvalueExpression* nonL = dynamic_cast<NonLvalueExpression*>(e) ;
  CallExpression* call = dynamic_cast<CallExpression*>(e) ;

  if (con != NULL)
  {
    IntConstant* i = dynamic_cast<IntConstant*>(con) ;
    if (i != NULL)
    {
      std::cerr << i->get_value().c_int() ;
    }
  }
  else if (bin != NULL)
  {
    PrintExpression(bin->get_source1()) ;
    std::cerr << " + " ;
    PrintExpression(bin->get_source2()) ;
  }
  else if (un != NULL)
  {
    std::cerr << "~" ;
    PrintExpression(un->get_source()) ;
  }
  else if (arrayRef != NULL)
  {
    PrintExpression(arrayRef->get_base_array_address()) ;
    std::cerr << "[" ;
    PrintExpression(arrayRef->get_index()) ;
    std::cerr << "]" ;
  }
  else if (field != NULL)
  {
    assert(0) ;
  }
  else if (load != NULL)
  {
    std::cerr << "*" ;
    PrintExpression(load->get_source_address()) ;
  }
  else if (loadVar != NULL)
  {
    PrintSymbol(loadVar->get_source()) ;
  }
  else if (symAddr != NULL)
  {
    std::cerr << "&" ;
    PrintSymbol(symAddr->get_addressed_symbol()) ;
  }
  else if (addr != NULL)
  {
    std::cerr << "&(" ;
    PrintExpression(addr->get_addressed_expression()) ;
    std::cerr << ")" ;
  }
  else if (nonL != NULL)
  {
    assert(0) ;
  }
  else if (call != NULL)
  {
    PrintExpression(call->get_callee_address()) ;
    std::cerr << "(" ;
    for (int i = 0 ; i < call->get_argument_count() ; ++i)
    {
      if (i != 0)
      {
	std::cerr << ", " ;
      }
      PrintExpression(call->get_argument(i)) ;
    }
    std::cerr << ")" ;
  }
  else
  {
    FormattedText tmpText ;
    e->print(tmpText) ;
    std::cerr << tmpText.get_value() << std::endl ;
    assert(0) ;
  }
}

void SuifTreeVerifier::VerifySymbol(Symbol* s)
{
  assert(s != NULL) ;
  assert(s->get_symbol_table() != NULL) ;
  assert(s->get_type() != NULL) ;
  VerifyAnnotations(s) ;
  PrintAnnotations(s) ;
}

void SuifTreeVerifier::PrintSymbol(Symbol* s) 
{
  std::cerr << s->get_name() ;
}
