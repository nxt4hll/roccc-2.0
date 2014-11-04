
#include "semantic_helper.h"
//#include <iostream.h>
#include <iostream>//jul modif
#include "utils/print_utils.h"

SemanticHelper::VarIter::VarIter(void) :
  _var_vector(), _vector_iter(_var_vector.begin())
{};

suif_vector<VariableSymbol*>* SemanticHelper::VarIter::get_var_vector(void)
{
  return &_var_vector;
};

bool SemanticHelper::VarIter::is_valid(void) {
   return (_vector_iter != _var_vector.end());
};

void SemanticHelper::VarIter::next(void)
{
  _vector_iter++;
};

VariableSymbol* SemanticHelper::VarIter::current(void) 
{
  return (*_vector_iter);
};

void SemanticHelper::VarIter::start(void)
{
  _vector_iter = _var_vector.begin();
};









SemanticHelper::SrcVarIter::SrcVarIter(const ExecutionObject* exe) :
  VarIter()
{
  get_src_var(exe, get_var_vector());
  start();
}





unsigned SemanticHelper::get_src_var(const ExecutionObject* exe,
				     suif_vector<VariableSymbol*>* var_vect)
{
  unsigned cnt = 0;
  for (Iter<VariableSymbol*> iter = exe->get_source_var_iterator();
       iter.is_valid();
       iter.next()) {
    cnt++;
    if (var_vect != 0)
      var_vect->push_back(iter.current());
  }
  for (Iter<Expression*> siter = exe->get_source_op_iterator();
       siter.is_valid();
       siter.next()) {
    Expression* exp = siter.current();
    if (exp == 0) continue;
    cnt += get_src_var(exp, var_vect);
  }
  return cnt;
}


  



SemanticHelper::DstVarIter::DstVarIter(const Statement* exe) :
  VarIter()
{
  get_dst_var(exe, get_var_vector());
  start();
}

unsigned
SemanticHelper::get_dst_var(const Statement* stmt,
			    suif_vector<VariableSymbol*>* var_vect)
{
  unsigned cnt = 0;
  for (Iter<VariableSymbol* > diter = stmt->get_destination_var_iterator();
       diter.is_valid();
       diter.next()) {
    cnt++;
    if (var_vect != NULL)
      var_vect->push_back(diter.current());
  }
  return cnt;
}
