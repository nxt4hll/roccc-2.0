/*

  This file contains all of the function definitions for the type nodes

*/

#include <cassert>

#include <iostream>

// Because of stupid SUIF LStrings
#include <cstring>
#include <cstdlib>

#include "typeNodes.h"
#include "cstNodes.h"
#include "generic.h"

#include "program.h"
#include "suifGenerator.h"

#include "nodes.h"

/*********************  VoidTypeMine **************************/

VoidTypeMine::VoidTypeMine(Option* q, Option* u, Option* n, Option* a) : 
  Node(), qualop(q), unqlop(u), nameop(n), algnop(a)
{
  ;
}

VoidTypeMine::~VoidTypeMine()
{
  if (qualop != NULL)
    delete qualop ;
  if (unqlop != NULL)
    delete unqlop ;
  if (nameop != NULL)
    delete nameop ;
  if (algnop != NULL)
    delete algnop ;
}

void VoidTypeMine::connect(Function* t) 
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (qualop != NULL)
    qualop->connect(t) ;
  if (unqlop != NULL)
    unqlop->connect(t) ;
  if (nameop != NULL)
    nameop->connect(t) ;
  if (algnop != NULL)
    algnop->connect(t) ;
}

void VoidTypeMine::flatten()
{
  if (qualop != NULL)
  {
    qual = (dynamic_cast<QualOption*>(qualop))->theValue() ;
    delete qualop ;
    qualop = NULL ;
  }
  else
    qual = '\0' ;
  
  if (unqlop != NULL)
  {
    unql = unqlop->theNodePointer() ;
    delete unqlop ;
    unqlop = NULL ;
  }
  else
    unql = NULL ;

  if (nameop != NULL)
  {
    name = nameop->theNodePointer() ;
    delete nameop;
    nameop = NULL ;
  }
  else
    name = NULL ;

  if (algnop != NULL)
  {
    algn = algnop->theNumber();
    delete algnop;
    algnop = NULL ;
  }
  else
    algn = -1;
}

Type* VoidTypeMine::suifType(Program* p) 
{
  return p->converter->globalTb->get_void_type() ;
}

/*********************  IntegerTypeMine **************************/

IntegerTypeMine::IntegerTypeMine(Option* q, Option* n, Option* u, Option* s, 
				 Option* a, Option* p, list<LString>* a2, 
				 Option* m, Option* m2) :
  Node(), qualop(q), nameop(n), unqlop(u), sizeop(s), algnop(a), precop(p),
  attributes(a2), minop(m), maxop(m2)
{
  ;
}

IntegerTypeMine::~IntegerTypeMine()
{
  if (qualop != NULL)
    delete qualop;
  if (nameop != NULL)
    delete nameop;
  if (unqlop != NULL)
    delete unqlop;
  if (sizeop != NULL)
    delete sizeop;
  if (algnop != NULL)
    delete algnop;
  if (precop != NULL)
    delete precop;
  if (attributes != NULL)
    delete attributes;
  if (minop != NULL)
    delete minop;
  if (maxop != NULL)
    delete maxop;
}

void IntegerTypeMine::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;
  if (qualop != NULL)
    qualop->connect(t);
  if (nameop != NULL)
    nameop->connect(t);
  if (unqlop != NULL)
    unqlop->connect(t);
  if (sizeop != NULL)
    sizeop->connect(t);
  if (algnop != NULL)
    algnop->connect(t);
  if (precop != NULL)
    precop->connect(t);
  if (minop != NULL)
    minop->connect(t);
  if (maxop != NULL)
    maxop->connect(t);
}

void IntegerTypeMine::flatten()
{
  if (qualop != NULL)
  {
    qual = (dynamic_cast<QualOption*>(qualop))->theValue() ;
    delete qualop;
    qualop = NULL ;
  }
  else
    qual = '\0' ;

  if (nameop != NULL)
  {
    name = nameop->theNodePointer();
    delete nameop;
    nameop = NULL ;
  }
  else
    name = NULL;

  if (unqlop != NULL)
  {
    unql = unqlop->theNodePointer();
    delete unqlop;
    unqlop = NULL ;
  }
  else
    unql = NULL ;

  if (sizeop != NULL)
  {
    size = sizeop->theNodePointer();
    delete sizeop ;
    sizeop = NULL ;
  }
  else
    size = NULL ;

  if (algnop != NULL)
  {
    algn = algnop->theNumber();
    delete algnop;
    algnop = NULL ;
  }
  else
    algn = -1 ;

  if (precop != NULL)
  {
    prec = precop->theNumber() ;
    delete precop;
    precop = NULL ;
  }
  else
    prec = -1 ;

  if (minop != NULL)
  {
    min = minop->theNodePointer();
    delete minop;
    minop = NULL ;
  }
  else
    min = NULL ;

  if (maxop != NULL)
  {
    max = maxop->theNodePointer() ;
    delete maxop;
    maxop = NULL ;
  }
  else
    max = NULL ;
}


int IntegerTypeMine::theSize(int arrayLevel)
{
  return 32 ;
}

int IntegerTypeMine::theSize()
{
  if (name != NULL)
  {
    LString parseMe = name->theName() ;

    // Check to see if parseMe has ROCCC in it
    //  Stupid suif LStrings don't have the find operation implemented, 
    //  so I'll do it the old fashioned way...
    const char* reallyParseMe = parseMe.c_str() ;
    
    if (strstr(reallyParseMe, "ROCCC_") != NULL)
    {
      char* numberPortion = strpbrk(reallyParseMe, "0123456789") ;
      return atoi(numberPortion) ;
    }
  }
 
  // If not named ROCCC_ something, we fall through to here.
  assert(size != NULL) ;
  return size->theSize() ;
}

Type* IntegerTypeMine::suifType(Program* p) 
{

  bool isSigned = true ; // signed by default
  LString compare = "unsigned" ; // Stupid SUIF

  if (attributes != NULL)
  {
    list<LString>::iterator findSignedIter = attributes->begin() ;
    while(findSignedIter != attributes->end())
    {
      if ((*findSignedIter) == compare)
      {
	isSigned = false ;
	break ;
      }
      ++findSignedIter ;
    }
  }

  // Figure out the size
  int typeSize = theSize() ;  

  Type* toReturn = p->converter->globalTb->get_integer_type(typeSize, typeSize, isSigned);

  if (name != NULL)
  {

    LString parseMe = name->theName() ;

    // Check to see if parseMe has ROCCC in it
    //  Stupid suif LStrings don't have the find operation implemented, 
    //  so I'll do it the old fashioned way...
    const char* reallyParseMe = parseMe.c_str() ;
    
    if (strstr(reallyParseMe, "ROCCC_") != NULL)
    {
      toReturn->set_name(name->theName()) ;
    }
    if (strstr(reallyParseMe, "_fixed") != NULL)
    {
      toReturn->append_annote(create_brick_annote(p->converter->getEnv(),
						  "FixedPoint")) ;
    }
  }
  return toReturn ;

}

LString IntegerTypeMine::theName()
{
  if (name != NULL)
    return name->theName() ;
  else
    return "ERROR_INT_TYPE" ;
}

ExecutionObject* IntegerTypeMine::getMax(Program* p)
{
  if (max == NULL)
    return NULL ;

  if (is_a<IntegerCst*>(max))
  {
    return max->generateSuif(p) ;
  }
  else
  {
    return NULL ;
  }
}

int IntegerTypeMine::getMaxNumber()
{
  assert(max != NULL) ;
  // A little weird here, but max should be an int constant
  //  (in all cases I can imagine) and theSize for an int constant
  //  returns the actual value.
  return max->theSize() ;
}

ExecutionObject* IntegerTypeMine::getMin(Program* p)
{
  if (min == NULL)
    return NULL ;

  if (is_a<IntegerCst*>(min))
    return min->generateSuif(p) ;
  else
    return NULL ;
}

bool IntegerTypeMine::isConst()
{
  return (qual == 'c') ;
}

bool IntegerTypeMine::isVolatile()
{
  return (qual == 'v') ;
}

/*********************  EnumeralType **************************/

EnumeralType::EnumeralType(Option* q, Option* n, Option* u, Option* s, 
			   Option* a, Option* p, list<LString>* a2, 
			   Option* m, Option* m2, Option* c) :
  Node(), qualop(q), nameop(n), unqlop(u), sizeop(s), algnop(a), precop(p), 
  attributes(a2), minop(m), maxop(m2), cstsop(c)
{
  ;
}

EnumeralType::~EnumeralType()
{
  if (qualop != NULL)
    delete qualop;
  if (nameop != NULL)
    delete nameop;
  if (unqlop != NULL)
    delete unqlop;
  if (sizeop != NULL)
    delete sizeop;
  if (algnop != NULL)
    delete algnop;
  if (precop != NULL)
    delete precop;
  if (attributes != NULL)
    delete attributes;
  if (minop != NULL)
    delete minop;
  if (maxop != NULL)
    delete maxop;
  if (cstsop != NULL)
    delete cstsop;
}

void EnumeralType::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (qualop != NULL)
    qualop->connect(t);
  if (nameop != NULL)
    nameop->connect(t);
  if (unqlop != NULL)
    unqlop->connect(t);
  if (sizeop != NULL)
    sizeop->connect(t);
  if (algnop != NULL)
    algnop->connect(t);
  if (precop != NULL)
    precop->connect(t);
  if (minop != NULL)
    minop->connect(t);
  if (maxop != NULL)
    maxop->connect(t);
  if (cstsop != NULL)
    cstsop->connect(t);
}

void EnumeralType::flatten()
{
  if (qualop != NULL)
  {
    qual = (dynamic_cast<QualOption*>(qualop))->theValue() ;
    delete qualop ;
    qualop = NULL ;
  }
  else
    qual = '\0' ;

  if (nameop != NULL)
  {
    name = nameop->theNodePointer() ;
    delete nameop ;
    nameop = NULL ;
  }
  else
    name = NULL ;

  if (unqlop != NULL)
  {
    unql = unqlop->theNodePointer() ;
    delete unqlop ;
    unqlop = NULL ;
  }
  else
    unql = NULL ;

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
    delete algnop;
    algnop = NULL ;
  }
  else
    algn = -1 ;

  if (precop != NULL)
  {
    prec = precop->theNumber() ;
    delete precop ;
    precop = NULL ;
  }
  else
    prec = -1 ;

  if (minop != NULL)
  {
    min = minop->theNodePointer() ;
    delete minop ;
    minop = NULL ;
  }
  else
    min = NULL ;

  if (maxop != NULL)
  {
    max = maxop->theNodePointer() ;
    delete maxop ;
    maxop = NULL ;
  }
  else
    max = NULL ;

  if (cstsop != NULL)
  {
    csts = cstsop->theNodePointer() ;
    delete cstsop ;
    cstsop = NULL ;
  }
  else
    csts = NULL ;
}

Type* EnumeralType::suifType(Program* p) 
{
  return NULL ; // Not yet supported
}

/*********************  RealType **************************/

RealType::RealType(Option* q, Option* n, Option* u, Option* s, Option* a, 
		   Option* p) :
  Node(), qualop(q), nameop(n), unqlop(u), sizeop(s), algnop(a), precop(p)
{
  ;
}

RealType::~RealType()
{
  if (qualop != NULL)
    delete qualop ;
  if (nameop != NULL)
    delete nameop;
  if (unqlop != NULL)
    delete unqlop ;
  if (sizeop != NULL)
    delete sizeop;
  if (algnop != NULL)
    delete algnop;
  if (precop != NULL)
    delete precop;
}

void RealType::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else 
    return ;
  if (qualop != NULL)
    qualop->connect(t) ;
  if (nameop != NULL)
    nameop->connect(t);
  if (unqlop != NULL)
    unqlop->connect(t) ;
  if (sizeop != NULL)
    sizeop->connect(t);
  if (algnop != NULL)
    algnop->connect(t);
  if (precop != NULL)
    precop->connect(t);
}

void RealType::flatten()
{
  if (qualop != NULL)
  {
    qual = (dynamic_cast<QualOption*>(qualop))->theValue() ;
    delete qualop ;
    qualop = NULL ;
  }
  else
    qual = '\0' ;

  if (nameop != NULL)
  {
    name = nameop->theNodePointer() ;
    delete nameop ;
    nameop = NULL ;
  }
  else
    name = NULL ;

  if (unqlop != NULL)
  {
    unql = unqlop->theNodePointer() ;
    delete unqlop ;
    unqlop = NULL ;
  }
  else
    unql = NULL ;

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

  if (precop != NULL)
  {
    prec = precop->theNumber() ;
    delete precop ;
    precop = NULL ;
  }
  else
    prec = -1 ;
}

bool RealType::isConst()
{
  return (qual == 'c') ;
}

Type* RealType::suifType(Program* p)
{
  // We can actually have floating point numbers with bit sizes
  //  other than 32 and 64!  Use the same ROCCC_ usage

  int mySize = theSize() ;

  Type* toReturn = p->converter->globalTb->get_floating_point_type(mySize, 
								   mySize) ;
  if (name != NULL)
  {

    LString parseMe = name->theName() ;

    // Check to see if parseMe has ROCCC in it
    //  Stupid suif LStrings don't have the find operation implemented, 
    //  so I'll do it the old fashioned way...
    const char* reallyParseMe = parseMe.c_str() ;
    
    if (strstr(reallyParseMe, "ROCCC_") != NULL)
    {
      toReturn->set_name(name->theName()) ;
    }

  }

  return toReturn ;
}

int RealType::theSize()
{
  if (name != NULL)
  {
    LString parseMe = name->theName() ;

    // Check to see if parseMe has ROCCC in it
    //  Stupid suif LStrings don't have the find operation implemented, 
    //  so I'll do it the old fashioned way...
    const char* reallyParseMe = parseMe.c_str() ;
    
    if (strstr(reallyParseMe, "ROCCC_") != NULL)
    {
      char* numberPortion = strpbrk(reallyParseMe, "0123456789") ;
      return atoi(numberPortion) ;
    }
  }

  assert(size != NULL) ;
  return size->theSize() ;
}

/*********************  PointerTypeMine **************************/

PointerTypeMine::PointerTypeMine(Option* q, Option* n, Option* u, Option* s, 
			 Option* a, Option* p) :
  Node(), qualop(q), nameop(n), unqlop(u), sizeop(s), algnop(a), ptdop(p)
{
  ;
}

PointerTypeMine::~PointerTypeMine()
{
  if (qualop != NULL)
    delete qualop;
  if (nameop != NULL)
    delete nameop;
  if (unqlop != NULL)
    delete unqlop;
  if (sizeop != NULL)
    delete sizeop;
  if (algnop != NULL)
    delete algnop;
  if (ptdop != NULL)
    delete ptdop;
}

void PointerTypeMine::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;
  if (qualop != NULL)
    qualop->connect(t);
  if (nameop != NULL)
    nameop->connect(t);
  if (unqlop != NULL)
    unqlop->connect(t);
  if (sizeop != NULL)
    sizeop->connect(t);
  if (algnop != NULL)
    algnop->connect(t);
  if (ptdop != NULL)
    ptdop->connect(t);
}

void PointerTypeMine::flatten()
{
  if (qualop != NULL)
  {
    qual = (dynamic_cast<QualOption*>(qualop))->theValue() ;
    delete qualop ;
    qualop = NULL ;
  }
  else
    qual = '\0' ;

  if (nameop != NULL)
  {
    name = nameop->theNodePointer() ;
    delete nameop ;
    nameop = NULL ;
  }
  else
    name = NULL ;

  if (unqlop != NULL)
  {
    unql = unqlop->theNodePointer() ;
    delete unqlop ;
    unqlop = NULL ;
  }
  else
    unql = NULL ;

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

  if (ptdop != NULL)
  {
    ptd = ptdop->theNodePointer() ;
    delete ptdop ;
    ptdop = NULL ;
  }
  else
    ptd = NULL ;
}

Type* PointerTypeMine::suifType(Program* p)
{
  // The original code
    return p->converter->globalTb->get_pointer_type(ptd->suifType(p)) ;


  // The code that returns a type with a different number of bits
   /*
  return p->converter->globalTb->get_pointer_type(ptd->theSize(), 
						  ptd->theSize(), 
                          p->converter->globalTb->get_void_type());

						  //ptd->suifType(p)) ;
                          */
}

/*********************  ReferenceTypeMine **************************/

ReferenceTypeMine::ReferenceTypeMine(Option* s, Option* a, Option* r) :
  Node(), sizeop(s), algnop(a), refdop(r)
{
  ;
}

ReferenceTypeMine::~ReferenceTypeMine()
{
  if (sizeop != NULL)
    delete sizeop;
  if (algnop != NULL)
    delete algnop;
  if (refdop != NULL)
    delete refdop;
}

void ReferenceTypeMine::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;
  if (sizeop != NULL)
    sizeop->connect(t);
  if (algnop != NULL)
    algnop->connect(t);
  if (refdop != NULL)
    refdop->connect(t);
}

void ReferenceTypeMine::flatten()
{
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

  if (refdop != NULL)
  {
    refd = refdop->theNodePointer() ;
    delete refdop ;
    refdop = NULL ;
  }
  else
    refd = NULL ;
}

Type* ReferenceTypeMine::suifType(Program* p)
{
  return p->converter->globalTb->get_reference_type(refd->suifType(p)) ;
}

/*********************  MethodType **************************/

MethodType::MethodType(Option* u, Option* s, Option* a, Option* c, 
		       Option* r, Option* p) :
  Node(), unqlop(u), sizeop(s), algnop(a), clasop(c), retnop(r), prmsop(p)
{
  ;
}

MethodType::~MethodType()
{
  if (unqlop != NULL)
    delete unqlop;
  if (sizeop != NULL)
    delete sizeop;
  if (algnop != NULL)
    delete algnop;
  if (clasop != NULL)
    delete clasop;
  if (retnop != NULL)
    delete retnop;
  if (prmsop != NULL)
    delete prmsop;
}

void MethodType::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;
  if (unqlop != NULL)
    unqlop->connect(t);
  if (sizeop != NULL)
    sizeop->connect(t);
  if (algnop != NULL)
    algnop->connect(t);
  if (clasop != NULL)
    clasop->connect(t);
  if (retnop != NULL)
    retnop->connect(t);
  if (prmsop != NULL)
    prmsop->connect(t);
}

void MethodType::flatten()
{
  if (unqlop != NULL)
  {
    unql = unqlop->theNodePointer() ;
    delete unqlop ;
    unqlop = NULL ;
  }
  else
    unql = NULL ;

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

  if (clasop != NULL)
  {
    clas = clasop->theNodePointer() ;
    delete clasop ;
    clasop = NULL ;
  }
  else
    clas = NULL ;

  if (retnop != NULL)
  {
    retn = retnop->theNodePointer() ;
    delete retnop ;
    retnop = NULL ;
  }
  else
    retn = NULL ;

  if (prmsop != NULL)
  {
    prms = prmsop->theNodePointer() ;
    delete prmsop ;
    prmsop = NULL ;
  }
  else
    prms = NULL ;
}

Type* MethodType::suifType(Program* p)
{
  return NULL ; // Not yet implemented
}

/*********************  FunctionType **************************/

FunctionType::FunctionType(Option* u, Option* s, Option* a, Option* r, 
			   Option* p) :
  Node(), unqlop(u), sizeop(s), algnop(a), retnop(r), prmsop(p)
{
  ;
}

FunctionType::~FunctionType()
{
  if (unqlop != NULL)
    delete unqlop;
  if (sizeop != NULL)
    delete sizeop;
  if (algnop != NULL)
    delete algnop;
  if (retnop != NULL)
    delete retnop;
  if (prmsop != NULL)
    delete prmsop;
}

void FunctionType::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;
  if (unqlop != NULL)
    unqlop->connect(t);
  if (sizeop != NULL)
    sizeop->connect(t);
  if (algnop != NULL)
    algnop->connect(t);
  if (retnop != NULL)
    retnop->connect(t);
  if (prmsop != NULL)
    prmsop->connect(t);
}

void FunctionType::flatten()
{
  if (unqlop != NULL)
  {
    unql = unqlop->theNodePointer() ;
    delete unqlop ;
    unqlop = NULL ;
  }
  else
    unql = NULL ;

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

  if (retnop != NULL)
  {
    retn = retnop->theNodePointer() ;
    delete retnop ;
    retnop = NULL ;
  }
  else
    retn = NULL ;

  if (prmsop != NULL)
  {
    prms = prmsop->theNodePointer() ;
    delete prmsop ;
    prmsop = NULL ;
  }
  else
    prms = NULL ;
}

Type* FunctionType::suifType(Program* p)
{
  // In order to get the suif type of this function, I need the return
  //  type ...

  DataType* returnType = findReturnType(p)->get_base_type() ;

  // ... and the argument list

  list<QualifiedType*> argList = findParameters(p) ;

  return p->converter->globalTb->get_c_procedure_type(returnType, argList) ;

  //  return NULL ;

}

list<QualifiedType*> FunctionType::findParameters(Program* p)
{
  if (prms != NULL)
  {
    return prms->findParameters(p) ;
  }
  else
  {
    list<QualifiedType*> empty ;
    return empty ;
  }
}

QualifiedType* FunctionType::findReturnType(Program* p)
{
  if (retn == NULL)
  {
  return p->converter->globalTb->get_qualified_type(p->converter->globalTb->get_void_type()) ;
  }
  else
  {
    if (retn->isConst())
    {
      return p->converter->globalTb->get_qualified_type(dynamic_cast<DataType*>(retn->suifType(p)), "const") ;
    }
    else if (retn->isVolatile())
    {
      return p->converter->globalTb->get_qualified_type(dynamic_cast<DataType*>(retn->suifType(p)), "volatile") ;
    }
    else
    {
      return p->converter->globalTb->get_qualified_type(retn->suifType(p)) ;
    }
  }
}

/*********************  RecordType **************************/

RecordType::RecordType(Option* q, Option* n, Option* u, Option* s, Option* a,
		       Option* v, list<LString>* a2, Option* f, Option* f2,
		       Option* b, Option* p, Option* c) :
  Node(), qualop(q), nameop(n), unqlop(u), sizeop(s), algnop(a), vfldop(v),
  attributes(a2), fldsop(f), fncsop(f2), binfop(b), ptdop(p), clsop(c)
{
  fieldsInStruct = NULL ;
  myCreatedType = NULL ;
}

RecordType::~RecordType()
{
  if (qualop != NULL)
    delete qualop;
  if (nameop != NULL)
    delete nameop;
  if (unqlop != NULL)
    delete unqlop;
  if (sizeop != NULL)
    delete sizeop;
  if (algnop != NULL)
    delete algnop;
  if (vfldop != NULL)
    delete vfldop;
  if (attributes != NULL)
    delete attributes;
  if (fldsop != NULL)
    delete fldsop;
  if (fncsop != NULL)
    delete fncsop;
  if (binfop != NULL)
    delete binfop;
  if (ptdop != NULL)
    delete ptdop;
  if (clsop != NULL)
    delete clsop;
}

void RecordType::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;
  if (qualop != NULL)
    qualop->connect(t);
  if (nameop != NULL)
    nameop->connect(t);
  if (unqlop != NULL)
    unqlop->connect(t);
  if (sizeop != NULL)
    sizeop->connect(t);
  if (algnop != NULL)
    algnop->connect(t);
  if (vfldop != NULL)
    vfldop->connect(t);
  if (fldsop != NULL)
    fldsop->connect(t);
  if (fncsop != NULL)
    fncsop->connect(t);
  if (binfop != NULL)
    binfop->connect(t);
  if (ptdop != NULL)
    ptdop->connect(t);
  if (clsop != NULL)
    clsop->connect(t);
}

void RecordType::flatten()
{
  if (qualop != NULL)
  {
    qual = (dynamic_cast<QualOption*>(qualop))->theValue() ;
    delete qualop ;
    qualop = NULL ;
  }
  else
    qual = '\0' ;

  if (nameop != NULL)
  {
    name = nameop->theNodePointer() ;
    delete nameop ;
    nameop = NULL ;
  }
  else
    name = NULL ;

  if (unqlop != NULL)
  {
    unql = unqlop->theNodePointer() ;
    delete unqlop ;
    unqlop = NULL ;
  }
  else
    unql = NULL ;

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

  if (vfldop != NULL)
  {
    vfld = vfldop->theNodePointer() ;
    delete vfldop ;
    vfldop = NULL ;
  }
  else
    vfld = NULL ;

  if (fldsop != NULL)
  {
    flds = fldsop->theNodePointer() ;
    delete fldsop ;
    fldsop = NULL ;
  }
  else
    flds = NULL ;

  if (fncsop != NULL)
  {
    fncs = fncsop->theNodePointer() ;
    delete fncsop ;
    fncsop = NULL ;
  }
  else
    fncs = NULL ;

  if (binfop != NULL)
  {
    binf = binfop->theNodePointer() ;
    delete binfop ;
    binfop = NULL ;
  }
  else
    binf = NULL ;

  if (ptdop != NULL)
  {
    ptd = ptdop->theNodePointer() ;
    delete ptdop ;
    ptdop = NULL ;
  }
  else
    ptd = NULL ;

  if (clsop != NULL)
  {
    cls = clsop->theNodePointer() ;
    delete clsop ;
    clsop = NULL ;
  }
  else
    cls = NULL ;
}

Type* RecordType::suifType(Program* p)
{

  if (unql != NULL)
    return unql->suifType(p) ;

  if (fieldsInStruct == NULL || myCreatedType == NULL)
  {
    fieldsInStruct = p->converter->globalSuifObjfactory->
      create_group_symbol_table() ;

    myCreatedType = p->converter->globalSuifObjfactory->
      create_struct_type(size->theSize(), algn, name->theName(),
			 0 /* not complete */, fieldsInStruct) ;

    p->converter->addStructTypeToCurrentSymbolTable(myCreatedType) ;
  }

  return myCreatedType ;

}

void RecordType::AddSymbolToTable(Symbol* s)
{
  fieldsInStruct->append_symbol_table_object(s) ;
}

/*********************  BooleanTypeMine **************************/

BooleanTypeMine::BooleanTypeMine(Option* q, Option* n, Option* u, Option* s,
			 Option* a) : 
  Node(), qualop(q), nameop(n), unqlop(u), sizeop(s), algnop(a) 
{
  ;
}

BooleanTypeMine::~BooleanTypeMine()
{
  if (qualop != NULL)
    delete qualop;
  if (nameop != NULL)
    delete nameop;
  if (unqlop != NULL)
    delete unqlop;
  if (sizeop != NULL)
    delete sizeop;
  if (algnop != NULL)
    delete algnop;
}

void BooleanTypeMine::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;
  if (qualop != NULL)
    qualop->connect(t);
  if (nameop != NULL)
    nameop->connect(t);
  if (unqlop != NULL)
    unqlop->connect(t);
  if (sizeop != NULL)
    sizeop->connect(t);
  if (algnop != NULL)
    algnop->connect(t);
}

void BooleanTypeMine::flatten()
{
  if (qualop != NULL)
  {
    qual = (dynamic_cast<QualOption*>(qualop))->theValue() ;
    delete qualop ;
    qualop = NULL ;
  }
  else
    qual = '\0' ;

  if (nameop != NULL)
  {
    name = nameop->theNodePointer() ;
    delete nameop ;
    nameop = NULL ;
  }
  else
    name = NULL ;

  if (unqlop != NULL)
  {
    unql = unqlop->theNodePointer() ;
    delete unqlop ;
    unqlop = NULL ;
  }
  else
    unql = NULL ;

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

Type* BooleanTypeMine::suifType(Program* p)
{

  Type* toReturn = p->converter->globalTb->get_boolean_type(theSize(), 
							    theSize()) ;

  if (name != NULL)
  {
    LString parseMe = name->theName() ;

    const char* reallyParseMe = parseMe.c_str() ;
   
    if (strstr(reallyParseMe, "ROCCC_") != NULL)
    {
      toReturn->set_name(name->theName()) ;
    }
  }

  return toReturn ;
}

int BooleanTypeMine::theSize()
{
  if (name != NULL)
  {
    LString parseMe = name->theName() ;

    // Check to see if parseMe has ROCCC in it
    //  Stupid suif LStrings don't have the find operation implemented, 
    //  so I'll do it the old fashioned way...
    const char* reallyParseMe = parseMe.c_str() ;
    
    if (strstr(reallyParseMe, "ROCCC_") != NULL)
    {
      char* numberPortion = strpbrk(reallyParseMe, "0123456789") ;
      return atoi(numberPortion) ;
    }
  }
 
  // If not named ROCCC_ something, we fall through to here.
  assert(size != NULL) ;
  return size->theSize() ;
}

/*********************  ArrayTypeMine **************************/

ArrayTypeMine::ArrayTypeMine(Option* q, Option* u, Option* s, Option* a, Option* e, 
		     Option* d) :
  Node(), qualop(q), unqlop(u), sizeop(s), algnop(a), eltsop(e), domnop(d)
{
  ;
}

ArrayTypeMine::~ArrayTypeMine()
{
  if (qualop != NULL)
    delete qualop;
  if (unqlop != NULL)
    delete unqlop;
  if (sizeop != NULL)
    delete sizeop;
  if (algnop != NULL)
    delete algnop;
  if (eltsop != NULL)
    delete eltsop;
  if (domnop != NULL)
    delete domnop;
}

void ArrayTypeMine::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;
  if (qualop != NULL)
    qualop->connect(t);
  if (unqlop != NULL)
    unqlop->connect(t);
  if (sizeop != NULL)
    sizeop->connect(t);
  if (algnop != NULL)
    algnop->connect(t);
  if (eltsop != NULL)
    eltsop->connect(t);
  if (domnop != NULL)
    domnop->connect(t);
}

void ArrayTypeMine::flatten()
{
  if (qualop != NULL)
  {
    qual = (dynamic_cast<QualOption*>(qualop))->theValue() ;
    delete qualop ;
    qualop = NULL ;
  }
  else
    qual = '\0' ;

  if (unqlop != NULL)
  {
    unql = unqlop->theNodePointer() ;
    delete unqlop ;
    unqlop = NULL ;
  }
  else
    unql = NULL ;

  if (sizeop != NULL)
  {
    size = sizeop->theNodePointer() ;
    delete sizeop;
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

  if (eltsop != NULL)
  {
    elts = eltsop->theNodePointer() ;
    delete eltsop ;
    eltsop = NULL ;
  }
  else
    elts = NULL ;

  if (domnop != NULL)
  {
    domn = domnop->theNodePointer() ;
    delete domnop ;
    domnop = NULL ;
  }
  else
    domn = NULL ;
}

Type* ArrayTypeMine::suifType(Program* p)
{
  assert(domn != NULL) ;
  assert(elts != NULL) ;
  assert(size != NULL) ;

  // I need the lower bound and upper bound
  Expression* lowerBound = dynamic_cast<Expression*>(domn->getMin(p)) ;
  Expression* upperBound = dynamic_cast<Expression*>(domn->getMax(p)) ;

  // I need the qualified type as well
  //  Are the elements themselves const, or can they even be const?
  //  It looks like they can, so I have to worry about this.

  QualifiedType* elementType ;

  if (elts->isConst())
  {
    elementType = p->converter->globalTb->get_qualified_type(dynamic_cast<DataType*>(elts->suifType(p)), "const") ;
  }
  else if (elts->isVolatile())
  {
    elementType = p->converter->globalTb->get_qualified_type(dynamic_cast<DataType*>(elts->suifType(p)), "volatile") ;
  }
  else
  {
    elementType = p->converter->globalTb->get_qualified_type(dynamic_cast<DataType*>(elts->suifType(p))) ;
  }

  // The element size actually has to be determined differently due
  //  to our special types.

  //  int elementSize = size->theSize() ;
  //  int elementSize = elts->theSize() ;

  // I need to pass in the total size, not the element size!

  return p->converter->globalTb->get_array_type(theSize(), algn, elementType, lowerBound, upperBound) ;

  // As a test, I'm printing out all of the array type info here
  /*
  FormattedText toPrint ;
  tempType->print(toPrint) ;

  cerr << toPrint.get_value() << endl ;

  
  return tempType ;
  */
}

int ArrayTypeMine::theSize()
{
  assert(domn != NULL) ;
  assert(elts != NULL) ;

  // The lower bound should always be zero, so determine the upper bound
  int upperBound = domn->getMaxNumber() ;

  return elts->theSize() * (upperBound + 1) ;

}
