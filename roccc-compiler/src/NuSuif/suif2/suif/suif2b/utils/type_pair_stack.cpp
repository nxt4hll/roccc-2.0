#include "common/system_specific.h"
#include "type_pair_stack.h"


TypePairStack::TypePairStack(Type *t1, Type *t2, TypePairStack *stack) :
  _type1(t1),
  _type2(t2),
  _more(stack)
{
}

TypePairStack::TypePairStack(const TypePairStack &other) :
  _type1(other._type1),
  _type2(other._type2),
  _more(other._more)
{
}

TypePairStack &
TypePairStack::operator=(const TypePairStack &other)
{
    _type1 = other._type1;
    _type2 = other._type2;
    _more = other._more;
    return *this;
}

bool TypePairStack::is_in(Type *t1, Type *t2)
{
  if (t1 == _type1 && t2 == _type2)
    return true;
  if (_more == NULL)
    return false;
  return _more->is_in(t1, t2);
}
