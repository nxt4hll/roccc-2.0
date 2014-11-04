
#include <common/lstring.h>
#include <sstream>
#include <cassert>
#include <iostream>

#include "option.h"
#include "nodes.h"
#include "function.h"

using namespace std;

/*  Base Class Stubs */

Option::Option()
{
  ;
}
Option::~Option()
{
  ;
}

void Option::setNodePointer(Node* value)
{
  return ;
}

void Option::connect(Function* t)
{
  return ;
}

/* Option connection functions */

void NameOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void TypeOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void VarsOption::connect(Function* t)
{
  assert(nodeNumber - 1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void ArgsOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void BodyOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void SizeOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void RetnOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void PrmsOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void ArgtOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void LowNodeOption::connect(Function* t)
{
  assert(nodeNumber - 1 >= 0) ;
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber) ;
}

void MinOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void MaxOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void ValuOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void ChanOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void UnqlOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void CstsOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void PtdOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void RefdOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void ClasOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void EltsOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void DomnOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void VfldOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void FldsOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void FncsOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void BinfOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void ScpeOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void CnstOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void MnglOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void BposOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void NextOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void DeclOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void ExprOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void CondOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void ThenOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void ElseOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void LablOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void FnOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void ClnpOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void BaseOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void StmtOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void InitOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void PurpOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void VcllOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void ClsOption::connect(Function* t)
{
  assert(nodeNumber -1 >= 0);
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber);
}

void DestOption::connect(Function* t)
{
  assert(nodeNumber - 1 >= 0) ;
  nodePointer = (*(t->allNodes))[nodeNumber-1] ;
  assert(nodePointer->getNumber() == nodeNumber) ;
}
