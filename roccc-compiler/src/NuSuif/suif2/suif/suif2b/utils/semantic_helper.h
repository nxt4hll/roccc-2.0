#ifndef _UTILS__SEMANTIC_HELPER_H_
#define _UTILS__SEMANTIC_HELPER_H_

/**
  * @file
  * This file contains utilities that deal with semantics of the IR.
  */

#include "basicnodes/basic.h"


/** This class understands the semantics of IR nodes.
  *
  * @class SemanticHelper semantic_helper.h utils/semantic_helper.h
  *
  * Example:
  *  To retrieve all source var into a suif_vector:
  *\code
  *    suif_vector<VariableSymbol*> vars;
  *    SemanticHelper::get_src_var( execution_Object, &vars);
  *\endcode
  *
  *  To iterate over all source vars in an execution object:
  *\code
  *    for (SemanticHelper::SrcVarIter iter(execution_object);
  *         iter.is_valid();
  *         iter.next()) {
  *      ... iter.current() ...
  *\endcode
  *
  *  To retrieve all destination variables in a statement
  *\code
  *     suif_vector<VariableSymbol*> vars;
  *     SemanticHelper::get_dst_var( statement, &vars );
  *\endcode
  *
  *  To iterate over all destination variables in a statement
  *\code
  *     for (SemanticHelper::DstVarIter iter(statement);
  *          iter.is_valid();
  *          iter.next()) {
  *       ... iter.current() ...
  *\endcode
  */
class SemanticHelper {

  /** Iterator that returns only VariableSymbols.
    * @internal
    */
 private:
  class VarIter {
  private:
    suif_vector<VariableSymbol*> _var_vector;
    suif_vector<VariableSymbol*>::iterator _vector_iter;
  protected:
    VarIter(void);      
    void            start(void);
    suif_vector<VariableSymbol*>* get_var_vector(void);
  public: 
    bool            is_valid(void);
    void            next(void);
    VariableSymbol* current(void);  
  };
  
 public:

  /** An iterator that returns source variable from an ExecutionObject.
    */
  class SrcVarIter : public VarIter {
  public:
    SrcVarIter(const ExecutionObject*);
  };


  /** An iterator that returns destination variable from a Statement.
    */
  class DstVarIter : public VarIter {
  public:
    DstVarIter(const Statement*);
  };
  
  /** Collect all source variables from an ExecutionObject.
    * @param exp the ExecutionObject.
    * @param lst a vector to collect the variables.
    * @return number of source variables found in \a exp.
    *
    * The source variables will be appended to \a lst, and the number of
    * variables appended will be returned.
    */
  static unsigned get_src_var(const ExecutionObject* exp,
				 suif_vector<VariableSymbol*>* lst = NULL);

  /** Collect all destination variables from a Statement.
    * @param exp the Statement.
    * @param lst a vector to collect the variables.
    * @return number of destination variables found in \a exp.
    *
    * The destination variables will be appended to \a lst, and the number of 
    * variables appended will be returned.
    */
  static unsigned get_dst_var(const Statement* stmt,
			      suif_vector<VariableSymbol*>* lst = NULL);

};

#endif // _UTILS__SEMANTIC_HELPER_H_
