
#include "typeUtilities.h"

#include <string>

#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif_factory.h>

bool IsARocccType(QualifiedType* q)
{
  std::string typeName = q->get_base_type()->get_name().c_str() ;
  if (typeName.find("ROCCC") != std::string::npos)
  {
    return true ;
  }
  return false ;
}

IntegerType* GetBaseInt(SuifEnv* theEnv)
{
  static IntegerType* baseInt = NULL ;
  if (baseInt == NULL)
  {
    baseInt = create_integer_type(theEnv, 
				  IInteger(32), 
				  0, 
				  true,
				  LString("BaseInt")) ;
  }
  return baseInt ;
}

QualifiedType* GetQualifiedBaseInt(SuifEnv* theEnv)
{
  static QualifiedType* qualBaseInt = NULL ;
  if (qualBaseInt == NULL)
  {
    qualBaseInt = create_qualified_type(theEnv, GetBaseInt(theEnv)) ;
  }
  return qualBaseInt ;
}

DataType* DeReference(DataType* t)
{
  DataType* toReturn = t ;
  while (dynamic_cast<ReferenceType*>(toReturn) != NULL)
  {
    Type* tmpType = dynamic_cast<ReferenceType*>(toReturn)->get_reference_type();
    QualifiedType* qType = dynamic_cast<QualifiedType*>(tmpType) ;
    assert(qType != NULL) ;
    toReturn = qType->get_base_type() ;
  }
  return toReturn ;
}

/*
String StringType(Type* t, bool convertArraysToPointers)
{
  assert(t != NULL) ;
  String toReturn = "" ;
  if (dynamic_cast<QualifiedType*>(t) != NULL)
  {
    QualifiedType* qType = dynamic_cast<QualifiedType*>(t) ;
    
    for (int i = 0 ; i < qType->get_qualification_count() ; ++i)
    {
      toReturn += qType->get_qualification(i) ;
      toReturn += " " ;
    }
    toReturn += StringType(qType->get_base_type(), convertArraysToPointers) ;
    return toReturn ;
  }
  
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
    return toReturn ;
  }

  if (dynamic_cast<MultiDimArrayType*>(t) != NULL)
  {
    toReturn += StringType(dynamic_cast<MultiDimArrayType*>(t)->get_element_type()) ;
    return toReturn ;
  }

  if (dynamic_cast<DataType*>(t) != NULL)
  {
    if (dynamic_cast<IntegerType*>(t) != NULL)
    {
      toReturn += "int" ;
    }
    if (dynamic_cast<FloatingPointType*>(t) != NULL)
    {
      std::stringstream convert ;
      convert << "ROCCC_float"
	      << dynamic_cast<FloatingPointType*>(t)->get_bit_size().c_int() ;
      toReturn +=  convert.str().c_str() ;
    }
    if (dynamic_cast<GroupType*>(t) != NULL)
    {
      toReturn += t->get_name() ;
    }
    if (dynamic_cast<VoidType*>(t) != NULL)
    {
      toReturn += "void" ;
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
      toReturn += "int" ;
    }
    if (dynamic_cast<ReferenceType*>(t) != NULL)
    {
      ReferenceType* rType = dynamic_cast<ReferenceType*>(t) ;
      toReturn += StringType(rType->get_reference_type()) ;
      toReturn += "&" ;
    }
    return toReturn ;
  }

  // Any other case gets blank
  return toReturn ;
}
*/
