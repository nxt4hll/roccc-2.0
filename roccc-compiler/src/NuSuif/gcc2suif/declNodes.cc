/*

  This file contains all of the definitions of the functions for the 
   Declaration nodes in the abstract syntax tree of gcc.

*/

#include <cassert>
#include <iostream>
#include <fstream>

#include "suifGenerator.h"

#include "nodes.h"
#include "typeNodes.h"
#include "declNodes.h"
#include "cstNodes.h"

using namespace std ; 

/*********************  ConstDecl **************************/

ConstDecl::ConstDecl(Option* n, Option* t, Option* s, Option* s2, Option*c) :
  Node(), nameop(n), typeop(t), scpeop(s), srcpop(s2), cnstop(c)
{
  ;
}

ConstDecl::~ConstDecl()
{
  if (nameop != NULL)
    delete nameop;
  if (typeop != NULL)
    delete typeop;
  if (scpeop != NULL)
    delete scpeop;
  if (srcpop != NULL)
    delete srcpop;
  if (cnstop != NULL)
    delete cnstop;
}

void ConstDecl::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;
  if (nameop != NULL)
    nameop->connect(t);
  if (typeop != NULL)
    typeop->connect(t);
  if (scpeop != NULL)
    scpeop->connect(t);
  if (srcpop != NULL)
    srcpop->connect(t);
  if (cnstop != NULL)
    cnstop->connect(t);
}

void ConstDecl::flatten()
{
  if (nameop != NULL)
  {
    name = nameop->theNodePointer() ;
    delete nameop ;
    nameop = NULL ;
  }
  else
    name = NULL ;

  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;

  if (scpeop != NULL)
  {
    scpe = scpeop->theNodePointer() ;
    delete scpeop ;
    scpeop = NULL ;
  }
  else
    scpe = NULL ;

  if (srcpop != NULL)
  {
    srcp = (dynamic_cast<SrcpOption*>(srcpop))->theValue() ;
    delete srcpop ;
    srcpop = NULL ;
  }
  else
    srcp = "" ;

  if (cnstop != NULL)
  {
    cnst = cnstop->theNodePointer() ;
    delete cnstop ;
    cnstop = NULL ;
  }
  else
    cnst = NULL ;
}

/*********************  VarDecl **************************/

VarDecl::VarDecl(Option* n, Option* m, Option* t, Option* s, Option* s2,
		 list<LString>* a, list<LString>* o, Option* i, Option* s3,
		 Option* a2, Option* u, Option* c) :
  Node(), nameop(n), mnglop(m), typeop(t), scpeop(s), srcpop(s2), 
  attributes(a), otherattributes(o), initop(i), sizeop(s3), algnop(a2), 
  usedop(u), chanop(c), enteredInSuif(false), suifSymbol(NULL)
{
  ; 
}

VarDecl::~VarDecl()
{
  if (nameop != NULL)
    delete nameop;
  if (mnglop != NULL)
    delete mnglop;
  if (typeop != NULL)
    delete typeop;
  if (scpeop != NULL)
    delete scpeop;
  if (srcpop != NULL)
    delete srcpop;
  if (attributes != NULL)
    delete attributes;
  if (otherattributes != NULL)
    delete otherattributes;
  if (initop != NULL)
    delete initop;
  if (sizeop != NULL)
    delete sizeop;
  if (algnop != NULL)
    delete algnop;
  if (usedop != NULL)
    delete usedop;
  if (chanop != NULL)
    delete chanop ;
  // Do I delete the suifSymbol?
}

void VarDecl::connect(Function* t) 
{
  if (!connected)
    connected = true ;
  else
    return ;
  if (nameop != NULL)
    nameop->connect(t);
  if (mnglop != NULL)
    mnglop->connect(t);
  if (typeop != NULL)
    typeop->connect(t);
  if (scpeop != NULL)
    scpeop->connect(t);
  if (srcpop != NULL)
    srcpop->connect(t);
  if (initop != NULL)
    initop->connect(t);
  if (sizeop != NULL)
    sizeop->connect(t);
  if (algnop != NULL)
    algnop->connect(t);
  if (usedop != NULL)
    usedop->connect(t);
  if (chanop != NULL)
    chanop->connect(t) ;
}

void VarDecl::flatten()
{
  if (nameop != NULL)
  {
    name = nameop->theNodePointer();
    delete nameop ;
    nameop = NULL ;
  }
  else
    name = NULL ;

  if (mnglop != NULL)
  {
    mngl = mnglop->theNodePointer() ;
    delete mnglop ;
    mnglop = NULL ;
  }
  else
    mngl = NULL ;

  if (typeop != NULL)
  {
    type = typeop->theNodePointer();
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;

  if (scpeop != NULL)
  {
    scpe = scpeop->theNodePointer() ;
    delete scpeop ;
    scpeop = NULL ;
  }
  else
    scpe = NULL ;

  if (srcpop != NULL)
  {
    srcp = (dynamic_cast<SrcpOption*>(srcpop))->theValue() ;
    delete srcpop;
    srcpop = NULL ;
  }
  else
    srcp = "" ;

  if (initop != NULL)
  {
    init = initop->theNodePointer() ;
    delete initop ;
    initop = NULL ;
  }
  else
    init = NULL ;

  if (sizeop != NULL)
  {
    size = sizeop->theNodePointer() ;
    delete sizeop ;
    sizeop = NULL ;
  }
  else
    size = NULL ;

  if (algnop != NULL)
  {
    algn = algnop->theNumber() ;
    delete algnop ;
    algnop = NULL ;
  }
  else
    algn = -1 ;

  if (usedop != NULL)
  {
    used = usedop->theNumber() ;
    delete usedop ;
    usedop = NULL ;
  }
  else
    used = -1 ;

  if (chanop != NULL)
  {
    chan = chanop->theNodePointer() ;
    delete chanop ;
    chanop = NULL ;    
  }
  else
    chan = NULL ;    
}

LString VarDecl::resultRegister()
{
  assert(name != NULL) ;
  return name->resultRegister() ;
}

Node* VarDecl::getScope()
{
  return scpe ;
}

/*
   This should be called when we are generating code and referring to
    this variable not as a declaration, but an instance of that variable
*/
ExecutionObject* VarDecl::generateSuif(Program* p)
{
  // If this is a constant, generate a constant expression.  Otherwise,
  //  create a load variable Expression
  Symbol* mySym = getVariable(p) ;
  VariableSymbol* myVarSym = dynamic_cast<VariableSymbol*>(mySym) ;
  assert(myVarSym != NULL) ;

  QualifiedType* myType = myVarSym->get_type() ;
  assert(myType != NULL) ;
  if (myType->has_qualification_member("const"))
  {
    assert(init != NULL) ;
    if (dynamic_cast<IntegerCst*>(init) != NULL)
    {
      return p->converter->createIntegerConstantExpression(init->theSize()) ;
    }
    else if (dynamic_cast<RealCst*>(init) != NULL)
    {
      return init->generateSuif(p) ;
    }
    else
    {
      assert(0 && "Unknown constant") ;
      return NULL ;
    }
  }
  else
  {
    return p->converter->createLoadVariableExpression(myVarSym) ;
  }
}

/*
   This function should be called when we are referring to the actual
    variable and not its use in a line of code.  For example, if you want
    to figure out the type of this variable, call getVariable, if you 
    want to process the expression (5 + a), call generateSuif.

   This function will create the variable and place it in the SUIF 
    symbol table the first time it is called.
*/
Symbol* VarDecl::getVariable(Program* p)
{
  if (enteredInSuif == true)
  {
    return suifSymbol ;
  }
  else
  {
    enteredInSuif = true ;

    ValueBlock* initialization = NULL ;
    
    if (init != NULL)
    {
      
      initialization = init->generateSuifInitialization(p) ;
      
    }

    // Figure out my type
    assert(type != NULL) ;

    QualifiedType* myType ;

    if (type->isConst())
    {
      myType = 
	p->converter->globalTb->get_qualified_type(dynamic_cast<DataType*>(type->suifType(p)),
						   "const") ;
    }
    else if (type->isVolatile())
    {
      myType = 
	p->converter->globalTb->get_qualified_type(dynamic_cast<DataType*>(type->suifType(p)),
						   "volatile") ;
    }
    else
    {
      myType =
	p->converter->globalTb->get_qualified_type(type->suifType(p)) ;
    }

    if (is_a<FunctionDecl*>(scpe))
    {
      suifSymbol=p->converter->addVariableSymbol(resultRegister(), myType,
						 initialization) ;
    }
    else
    {
      // I'm not sure where initialization would go in this case
      delete initialization ;
      suifSymbol=p->converter->addGlobalSymbol(resultRegister(), myType) ;
    }

    // Make sure that the rest of the variables are added into the symbol
    //  table
    if (chan != NULL)
    {
      Symbol* tempSymbol = chan->getVariable(p) ;
    }

    return suifSymbol ;
  }
}

Type* VarDecl::suifType(Program* p)
{
  assert(type != NULL) ;
  return type->suifType(p) ;
}

VarDecl* VarDecl::getNextVariable()
{
  return dynamic_cast<VarDecl*>(chan) ;
}

/*********************  ParmDecl **************************/

ParmDecl::ParmDecl(Option* n, Option* t, Option* s, Option* s2, 
		   list<LString>* a, Option* a2, Option* s3, Option* a3, 
		   Option* u, Option* c) :
  Node(), nameop(n), typeop(t), scpeop(s), srcpop(s2), attributes(a), 
  argtop(a2), sizeop(s3), algnop(a3), usedop(u), chanop(c)
{
  enteredInSuif = false ;
  suifSymbol = false ;
}

ParmDecl::~ParmDecl()
{
  if (nameop != NULL)
    delete nameop;
  if (typeop != NULL)
    delete typeop;
  if (scpeop != NULL)
    delete scpeop;
  if (attributes != NULL)
    delete attributes;
  if (argtop != NULL)
    delete argtop;
  if (sizeop != NULL)
    delete sizeop;
  if (algnop != NULL)
    delete algnop;
  if (usedop != NULL)
    delete usedop;
  if (chanop != NULL)
    delete chanop ;
}

void ParmDecl::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;
  if (nameop != NULL)
    nameop->connect(t);
  if (typeop != NULL)
    typeop->connect(t);
  if (scpeop != NULL)
    scpeop->connect(t);
  if (srcpop != NULL)
    srcpop->connect(t);
  if (argtop != NULL)
    argtop->connect(t);
  if (sizeop != NULL)
    sizeop->connect(t);
  if (algnop != NULL)
    algnop->connect(t);
  if (usedop != NULL)
    usedop->connect(t);
  if (chanop != NULL)
    chanop->connect(t) ;
}

void ParmDecl::flatten()
{
  if (nameop != NULL)
  {
    name = nameop->theNodePointer() ;
    delete nameop ;
    nameop = NULL ;
  }
  else
    name = NULL ;

  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;

  if (scpeop != NULL)
  {
    scpe = scpeop->theNodePointer() ;
    delete scpeop ;
    scpeop = NULL ;
  }
  else
    scpe = NULL ;

  if (srcpop != NULL)
  {
    srcp = (dynamic_cast<SrcpOption*>(srcpop))->theValue() ;
    delete srcpop ;
    srcpop = NULL ;
  }
  else
    srcp = "" ;

  if (argtop != NULL)
  {
    argt = argtop->theNodePointer() ;
    delete argtop ;
    argtop = NULL ;
  }
  else
    argt = NULL ;

  if (sizeop != NULL)
  {
    size = sizeop->theNodePointer() ;
    delete sizeop ;
    sizeop = NULL ;
  }
  else
    size = NULL ;

  if (algnop != NULL)
  {
    algn = algnop->theNumber() ;
    delete algnop ;
    algnop = NULL ;
  }
  else
    algn = -1 ;

  if (usedop != NULL)
  {
    used = usedop->theNumber() ;
    delete usedop;
    usedop = NULL ;
  }
  else
    used = -1 ;
  
  if (chanop != NULL)
  {
    chan = chanop->theNodePointer() ;
    delete chanop ;
    chanop = NULL ;
  }
  else
  {
    chan = NULL ;
  }
}

LString ParmDecl::resultRegister()
{
  assert(name != NULL);
  return name->resultRegister() ;
}

Symbol* ParmDecl::getVariable(Program* p)
{
  if (enteredInSuif == true)
  {
    return suifSymbol ;
  }
  else
  {
    enteredInSuif = true ;

    // Figure out my type
    assert(type != NULL) ;
    QualifiedType* myType ;

    if (type->isConst())
    {
      myType =  p->converter->globalTb->get_qualified_type(dynamic_cast<DataType*>(type->suifType(p)),
							   "const") ;
    }
    else if (type->isVolatile())
    {
      myType =  p->converter->globalTb->get_qualified_type(dynamic_cast<DataType*>(type->suifType(p)),
							   "volatile") ;
    }
    else
    {
      myType =  p->converter->globalTb->get_qualified_type(type->suifType(p)) ;
    }

    suifSymbol=p->converter->globalBasicObjfactory->create_parameter_symbol(myType, resultRegister(), false) ;

    return suifSymbol ;
  }

}

Node* ParmDecl::getScope()
{
  return scpe ;
}

Node* ParmDecl::getChan()
{
  return chan ;
}

ExecutionObject* ParmDecl::generateSuif(Program* p)
{
  /*  // A special case for structs...
   if (dynamic_cast<RecordType*>(type) != NULL)
  {
    return p->converter->globalSuifObjfactory->
      create_symbol_address_expression(p->converter->globalSuifObjfactory->
				       create_pointer_type(32, 32, type->suifType(p)), getVariable(p)) ;
  }
  */
  return p->converter->createLoadVariableExpression(dynamic_cast<VariableSymbol*>(getVariable(p))) ;
}

Type* ParmDecl::suifType(Program* p)
{
  assert(type != NULL) ;
  return type->suifType(p) ;
}

/*********************  FieldDecl **************************/

FieldDecl::FieldDecl(Option* n, Option* m, Option* t, Option* s, Option* s2,
		     list<LString>* a, Option* s3, Option* a2, Option* b,
		     Option* c) :
  Node(), nameop(n), mnglop(m), typeop(t), scpeop(s), srcpop(s2), 
  attributes(a), sizeop(s3), algnop(a2), bposop(b), chanop(c)
{
  enteredInSuif = false ;
  suifSymbol = NULL ;
}

FieldDecl::~FieldDecl()
{
  if (nameop != NULL)
    delete nameop;
  if (mnglop != NULL)
    delete mnglop;
  if (typeop != NULL)
    delete typeop;
  if (scpeop != NULL)
    delete scpeop;
  if (srcpop != NULL)
    delete srcpop;
  if (attributes != NULL)
    delete attributes;
  if (sizeop != NULL)
    delete sizeop;
  if (algnop != NULL)
    delete algnop;
  if (bposop != NULL)
    delete bposop;
  if (chanop != NULL)
    delete chanop ;
}

void FieldDecl::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;
  if (nameop != NULL)
    nameop->connect(t);
  if (mnglop != NULL)
    mnglop->connect(t);
  if (typeop != NULL)
    typeop->connect(t);
  if (scpeop != NULL)
    scpeop->connect(t);
  if (srcpop != NULL)
    srcpop->connect(t);
  if (sizeop != NULL)
    sizeop->connect(t);
  if (algnop != NULL)
    algnop->connect(t);
  if (bposop != NULL)
    bposop->connect(t);
  if (chanop != NULL)
    chanop->connect(t) ;
}

void FieldDecl::flatten()
{
  if (nameop != NULL)
  {
    name = nameop->theNodePointer() ;
    delete nameop ;
    nameop = NULL ;
  }
  else
    name = NULL ;

  if (mnglop != NULL)
  {
    mngl = mnglop->theNodePointer() ;
    delete mnglop ;
    mnglop = NULL ;
  }
  else
    mngl = NULL ;

  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;

  if (scpeop != NULL)
  {
    scpe = scpeop->theNodePointer() ;
    delete scpeop;
    scpeop = NULL ;
  }
  else
    scpe = NULL ;

  if (srcpop != NULL)
  {
    srcp = (dynamic_cast<SrcpOption*>(srcpop))->theValue() ;
    delete srcpop ;
    srcpop = NULL ;
  }
  else
    srcp = "" ;

  if (sizeop != NULL)
  {
    size = sizeop->theNodePointer() ;
    delete sizeop ;
    sizeop = NULL ;
  }
  else
    size = NULL ;

  if (algnop != NULL)
  {
    algn = algnop->theNumber() ;
    delete algnop ;
    algnop = NULL ;
  }
  else
    algn = -1 ;

  if (bposop != NULL)
  {
    bpos = bposop->theNodePointer() ;
    delete bposop ;
    bposop = NULL ;
  }
  else
    bpos = NULL ;

  if (chanop != NULL)
  {
    chan = chanop->theNodePointer() ;
    delete chanop ;
    chanop = NULL ;
  }
  else
    chan = NULL ;

}

LString FieldDecl::resultRegister()
{
  assert(name != NULL) ;
  return name->resultRegister() ;
}

Symbol* FieldDecl::getVariable(Program* p)
{
  if (enteredInSuif == true)
  {
    assert(suifSymbol != NULL) ;
    return suifSymbol ;
  }

  enteredInSuif = true ;

  // Figure out my type
  assert(type != NULL) ;
  QualifiedType* myType ;

  if (type->isConst())
  {
    myType = p->converter->globalTb->get_qualified_type(dynamic_cast<DataType*>(type->suifType(p)),
							"const") ;
  }
  else if (type->isVolatile())
  {
    myType = p->converter->globalTb->get_qualified_type(dynamic_cast<DataType*>(type->suifType(p)),
							"volatile") ;
  }
  else
  {
    myType =  p->converter->globalTb->get_qualified_type(type->suifType(p)) ;
  }

  assert(bpos != NULL) ;
  suifSymbol=p->converter->globalSuifObjfactory->
    create_field_symbol(myType, 
			dynamic_cast<Expression*>(bpos->generateSuif(p)),
			resultRegister(), false) ;

  // I also must add myself to the struct's group symbol table
  assert(scpe != NULL) ;
  assert(is_a<RecordType*>(scpe)) ;

  dynamic_cast<RecordType*>(scpe)->AddSymbolToTable(suifSymbol) ;

  return suifSymbol ;

}

ExecutionObject* FieldDecl::generateSuif(Program* p)
{
  return p->converter->createLoadVariableExpression(dynamic_cast<VariableSymbol*>(getVariable(p))) ;  
}

/*********************  ResultDecl **************************/

ResultDecl::ResultDecl(Option* t, Option* s, Option* s2, list<LString>* attr, 
		       Option* s3, Option* a) :
  Node(), typeop(t), scpeop(s), srcpop(s2), attributes(attr), sizeop(s3), 
  algnop(a)  
{
  ;
}

ResultDecl::~ResultDecl()
{
  if (typeop != NULL)
    delete typeop;
  if (scpeop != NULL)
    delete scpeop;
  if (srcpop != NULL)
    delete srcpop;
  if (attributes != NULL)
    delete attributes ;
  if (sizeop != NULL)
    delete sizeop;
  if (algnop != NULL)
    delete algnop;
}

void ResultDecl::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;
  if (typeop != NULL)
    typeop->connect(t);
  if (scpeop != NULL)
    scpeop->connect(t);
  if (srcpop != NULL)
    srcpop->connect(t);
  if (sizeop != NULL)
    sizeop->connect(t);
  if (algnop != NULL)
    algnop->connect(t);
}

void ResultDecl::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;

  if (scpeop != NULL)
  {
    scpe = scpeop->theNodePointer() ;
    delete scpeop ;
    scpeop = NULL ;
  }
  else
    scpe = NULL ;

  if (srcpop != NULL)
  {
    srcp = (dynamic_cast<SrcpOption*>(srcpop))->theValue() ;
    delete srcpop ;
    srcpop = NULL ;
  }
  else
    srcp = "" ;

  if (sizeop != NULL)
  {
    size = sizeop->theNodePointer() ;
    delete sizeop ;
    sizeop = NULL ;
  }
  else
    size = NULL ;

  if (algnop != NULL)
  {
    algn = algnop->theNumber() ;
    delete algnop ;
    algnop = NULL ;
  }
  else
    algn = -1 ;
}

/*********************  FunctionDecl **************************/

FunctionDecl::FunctionDecl(Option* n, Option* m, Option* t, Option* s, 
			   Option* s2, list<LString>* a, Option* p, Option* d,
			   Option* v, Option* f, Option* a2, list<LString>* o,
			   Option* b) : 
  Node(), nameop(n), mnglop(m), typeop(t), scpeop(s), srcpop(s2), 
  attributes(a), prioop(p), dltaop(d), vcllop(v), fnop(f), argsop(a2), 
  otherattributes(o), bodyop(b)
{
  enteredInSuif = false ;
  suifSymbol = NULL ;
}

FunctionDecl::~FunctionDecl()
{
  if (nameop != NULL)
    delete nameop;
  if (mnglop != NULL)
    delete mnglop;
  if (typeop != NULL)
    delete typeop;
  if (scpeop != NULL)
    delete scpeop;
  if (attributes != NULL)
    delete attributes;
  if (prioop != NULL)
    delete prioop;
  if (dltaop != NULL)
    delete dltaop;
  if (vcllop != NULL)
    delete vcllop;
  if (argsop != NULL)
    delete argsop;
  if (fnop != NULL)
    delete fnop;
  if (otherattributes != NULL)
    delete otherattributes;
  if (bodyop != NULL)
    delete bodyop;
}

void FunctionDecl::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;
  if (nameop != NULL)
    nameop->connect(t);
  if (mnglop != NULL)
    mnglop->connect(t);
  if (typeop != NULL)
    typeop->connect(t);
  if (scpeop != NULL)
    scpeop->connect(t);
  if (srcpop != NULL)
    srcpop->connect(t);
  if (prioop != NULL)
    prioop->connect(t);
  if (dltaop != NULL)
    dltaop->connect(t);
  if (vcllop != NULL)
    vcllop->connect(t);
  if (argsop != NULL)
    argsop->connect(t);
  if (fnop != NULL)
    fnop->connect(t);
  if (bodyop != NULL)
    bodyop->connect(t);
}


void FunctionDecl::flatten()
{
  if (nameop != NULL)
  {
    name = nameop->theNodePointer() ;
    delete nameop ;
    nameop = NULL ;
  }
  else
    name = NULL ;

  if (mnglop != NULL)
  {
    mngl = mnglop->theNodePointer() ;
    delete mnglop ;
    mnglop = NULL ;
  }
  else
    mngl = NULL ;

  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;

  if (scpeop != NULL)
  {
    scpe = scpeop->theNodePointer() ;
    delete scpeop ;
    scpeop = NULL ;
  }
  else
    scpe = NULL ;

  if (srcpop != NULL)
  {
    srcp = (dynamic_cast<SrcpOption*>(srcpop))->theValue() ;
    delete srcpop ;
    srcpop = NULL ;
  }
  else
    srcp = "" ;

  if (prioop != NULL)
  {
    prio = prioop->theNumber() ;
    delete prioop ;
    prioop = NULL ;
  }
  else
    prio = -1 ;

  if (dltaop != NULL)
  {
    dlta = dltaop->theNumber() ;
    delete dltaop ;
    dltaop = NULL ;
  }
  else
    dlta = -1 ;

  if (vcllop != NULL)
  {
    vcll = vcllop->theNodePointer() ;
    delete vcllop ;
    vcllop = NULL ;
  }
  else
    vcll = NULL ;

  if (argsop != NULL)
  {
    args = argsop->theNodePointer() ;
    delete argsop ;
    argsop = NULL ;
  }
  else
    args = NULL ;

  if (fnop != NULL)
  {
    fn = fnop->theNodePointer() ;
    delete fnop ;
    fnop = NULL ;
  }
  else
    fn = NULL ;

  if (bodyop != NULL)
  {
    body = bodyop->theNodePointer() ;
    delete bodyop ;
    bodyop = NULL ;
  }
  else
    body = NULL ;
}

LString FunctionDecl::resultRegister()
{
  if (name == NULL)
    return "tmpName" ;
  else
    return name->resultRegister() ;
}

Symbol* FunctionDecl::getVariable(Program* p)
{
  if (enteredInSuif == true)
  {
    return suifSymbol ;
  }
  else
  {
    // Place this function in the file symbol table
    enteredInSuif = true ;

    assert(type != NULL) ;
    CProcedureType* myType = dynamic_cast<CProcedureType*>(type->suifType(p)) ;
    assert(myType != NULL) ;
    
    suifSymbol=p->converter->globalBasicObjfactory->create_procedure_symbol(myType, resultRegister(), true) ;

    // Add it to the current symbol table as well

    p->converter->addProcedureSymbolToCurrentSymbolTable(suifSymbol) ;
    
    return suifSymbol ;
  }
}

Type* FunctionDecl::suifType(Program* p)
{
  assert(type != NULL) ;
  return type->suifType(p) ;
}

QualifiedType* FunctionDecl::findReturnType(Program* p)
{
  assert(type != NULL) ;
  return type->findReturnType(p) ;
}

Node* FunctionDecl::getArgs()
{
  return args ;
}

/*********************  LabelDecl **************************/

LabelDecl::LabelDecl(Option* n, Option* t, Option* s, Option* s2) : 
  Node(), nameop(n), typeop(t), scpeop(s), srcpop(s2) 
{
  suifSymbol = NULL ;
}

LabelDecl::~LabelDecl()
{
  if (nameop != NULL)
    delete nameop ;
  if (typeop != NULL)
    delete typeop ;
  if (scpeop != NULL)
    delete scpeop;
  if (srcpop != NULL)
    delete srcpop;
}

void LabelDecl::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;
  if (nameop != NULL)
    nameop->connect(t) ;
  if (typeop != NULL)
    typeop->connect(t) ;
  if (scpeop != NULL)
    scpeop->connect(t) ;
  if (srcpop != NULL)
    srcpop->connect(t) ;
}

void LabelDecl::flatten()
{
  if (nameop != NULL)
  {
    name = nameop->theNodePointer() ;
    delete nameop ;
    nameop = NULL ;
  }

  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }

  if (scpeop != NULL)
  {
    scpe = scpeop->theNodePointer() ;
    delete scpeop ;
    scpeop = NULL ;
  }
  else
    scpe = NULL ;

  if (srcpop != NULL)
  {
    srcp = (dynamic_cast<SrcpOption*>(srcpop))->theValue() ;
    delete srcpop ;
    srcpop = NULL ;
  }
  else
    srcp = "" ;
}

LString LabelDecl::theName()
{
  assert(name != NULL) ;
  return (dynamic_cast<IdentifierNode*>(name))->theName() ;
}

Symbol* LabelDecl::getVariable(Program* p)
{
  if (suifSymbol == NULL)
  {
    // Create a Code Label Symbol
    suifSymbol = p->converter->globalBasicObjfactory->create_code_label_symbol(p->converter->globalTb->get_label_type() , theName()) ;

    // Also add it to the current file symbol table

    p->converter->addLabelSymbolToCurrentSymbolTable(suifSymbol) ;

  }

  return suifSymbol ;
 
}

/********************* TypeDecl **************************/

TypeDecl::TypeDecl(Option* n, Option* t, Option* s, Option* s2, 
		   list<LString>* a) :
  Node(), nameop(n), typeop(t), scpeop(s), srcpop(s2), attributes(a)
{
  ;
}

TypeDecl::~TypeDecl()
{
  if (nameop != NULL)
    delete nameop;
  if (typeop != NULL)
    delete typeop;
  if (scpeop != NULL)
    delete scpeop;
  if (srcpop != NULL)
    delete srcpop;
}

void TypeDecl::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;
  if (nameop != NULL)
    nameop->connect(t);
  if (typeop != NULL)
    typeop->connect(t);
  if (scpeop != NULL)
    srcpop->connect(t);
}

void TypeDecl::flatten()
{
  if (nameop != NULL)
  {
    name = nameop->theNodePointer() ;
    delete nameop ;
    nameop = NULL ;
  }
  else
    name = NULL ;

  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;

  if (scpeop != NULL)
  {
    scpe = scpeop->theNodePointer() ;
    delete scpeop ;
    scpeop = NULL ;
  }
  else
    scpe = NULL ;

  if (srcpop != NULL)
  {
    srcp = (dynamic_cast<SrcpOption*>(srcpop))->theValue() ;
    delete srcpop ;
    srcpop = NULL ;
  }
  else
    srcp = "" ;
}

LString TypeDecl::theName()
{
  if (name != NULL)
  {
    return name->theName(); 
  }
  else
  {
    return "" ;
  }
}

Type* TypeDecl::suifType(Program* p)
{
  assert(type != NULL) ;
  return type->suifType(p) ;
}

NamespaceDecl::NamespaceDecl(Option* n, Option* t, Option* s, Option* c) :
  Node(), nameop(n), typeop(t), srcpop(s), cop(c) 
{
  ;
}

NamespaceDecl::~NamespaceDecl()
{
  if (nameop != NULL)
    delete nameop;
  if (typeop != NULL)
    delete typeop;
  if (srcpop != NULL)
    delete srcpop;
  if (cop != NULL)
    delete cop;
}

void NamespaceDecl::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;
  if (nameop != NULL)
    nameop->connect(t);
  if (typeop != NULL)
    typeop->connect(t);
  if (srcpop != NULL)
    srcpop->connect(t);
  if (cop != NULL)
    cop->connect(t);
}

void NamespaceDecl::flatten()
{
  if (nameop != NULL)
  {
    name = nameop->theNodePointer() ;
    delete nameop ;
    nameop = NULL ;
  }
  else
    name = NULL ;

  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;

  if (srcpop != NULL)
  {
    srcp = (dynamic_cast<SrcpOption*>(srcpop))->theValue() ;
    delete srcpop ;
    srcpop = NULL ;
  }
  else
    srcp = "" ;

  if (cop != NULL)
  {
    c = true ;
    delete cop ;
    cop = NULL ;
  }
  else
    c = false ;
}
