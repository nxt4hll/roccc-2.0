
#include "parameterUtilities.h"

struct WrappedParameter
{
public:
  int order ;
  VariableSymbol* param ;
  bool operator<(WrappedParameter& x) 
  {
    return order < x.order ;
  }
  WrappedParameter(int o, VariableSymbol* p) : order(o), param(p) { } 
} ;

std::list<VariableSymbol*> SortParameters(std::list<VariableSymbol*>& params)
{
  std::list<VariableSymbol*> sortedList ;

  std::list<WrappedParameter> wrappedList ;
  std::list<VariableSymbol*>::iterator paramIter = params.begin() ;
  while (paramIter != params.end())
  {
    int order ;
    BrickAnnote* orderAnnote = 
      dynamic_cast<BrickAnnote*>((*paramIter)->
				 lookup_annote_by_name("ParameterOrder")) ;
    if (orderAnnote != NULL)
    {
      IntegerBrick* orderBrick = 
	dynamic_cast<IntegerBrick*>(orderAnnote->get_brick(0)) ;
      assert(orderBrick != NULL) ;
      order = orderBrick->get_value().c_int() ;
    }
    else
    {
      order = -1 ;
    }    
    WrappedParameter nextParam(order, *paramIter) ;
    wrappedList.push_back(nextParam) ;
    ++paramIter ;
  }

  wrappedList.sort() ;

  std::list<WrappedParameter>::iterator wrapIter = wrappedList.begin() ;
  while (wrapIter != wrappedList.end())
  {
    sortedList.push_back((*wrapIter).param) ;
    ++wrapIter ;
  }

  return sortedList ;
}
