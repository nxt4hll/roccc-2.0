
#include <cassert>

#include <suifkernel/utilities.h>

#include "roccc_utils/warning_utils.h"

#include "div_by_const_elimination_pass2.h"

DivByConstEliminationPass2::DivByConstEliminationPass2(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "DivByConstEliminationPass2")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void DivByConstEliminationPass2::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;

  OutputInformation("Division by constant elimination pass 2.0 begins") ;
  list<BinaryExpression*>* allBinary = 
    collect_objects<BinaryExpression>(procDef->get_body()) ;
  list<BinaryExpression*>::iterator binIter = allBinary->begin() ;
  while (binIter != allBinary->end())
  {
    ProcessBinaryExpression(*binIter) ;
    ++binIter ;
  }
  delete allBinary ;
  OutputInformation("Division by constant elimination pass 2.0 ends") ;
}

void DivByConstEliminationPass2::ProcessBinaryExpression(BinaryExpression* b)
{
  Expression* leftSide  = b->get_source1() ;
  Expression* rightSide = b->get_source2() ;
  LString opcode = b->get_opcode() ;

  if (opcode != LString("divide"))
  {
    return ;
  }

  if (!IsIntegerType(leftSide->get_result_type()))
  {
    return ;
  }

  IntConstant* rightConst = dynamic_cast<IntConstant*>(rightSide) ;
  
  if (rightConst == NULL)
  {
    return ;
  }

  int pow = PowerOfTwo(rightConst) ;

  if (pow >= 0)
  {
    // Exactly a power of two, replace with a shift
    b->set_opcode(LString("right_shift")) ;
    rightConst->set_value(IInteger(pow)) ;
  }
}

bool DivByConstEliminationPass2::IsIntegerType(Type* t)
{
  if (dynamic_cast<IntegerType*>(t) != NULL)
  {
    return true ;
  }
  if (dynamic_cast<QualifiedType*>(t) != NULL)
  {
    return IsIntegerType(dynamic_cast<QualifiedType*>(t)->get_base_type()) ;
  }
  if (dynamic_cast<ArrayType*>(t) != NULL)
  {
    return IsIntegerType(dynamic_cast<ArrayType*>(t)->get_element_type()) ;
  }
  return false ;
}

int DivByConstEliminationPass2::PowerOfTwo(IntConstant* i)
{
  int value = i->get_value().c_int() ;
  if (value < 0)
  {
    return -1 ;
  }

  // Powers of two should only have one bit set.  I also need to know which
  //  bit is set, in case I find a power of two, so I will do this the
  //  straightforward way.

  bool found = false ;
  int position = -1 ;
  
  for (int j = 0 ; j < sizeof(value) * 8 ; ++j) // number of bits
  {
    if (((value >> j) & 0x1) != 0 && found == false)
    {
      position = j ;
      found = true ;
    }
    else if (((value >> j) & 0x1) != 0 && found == true)
    {
      return -1 ;
    }
  }
 
  if (position == -1)
  {
    return -1 ;
  }

  return position ;

}
