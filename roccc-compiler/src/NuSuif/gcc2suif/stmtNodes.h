/*

  This file contains the declarations of all of the nodes that correspond
   to statement nodes in the gcc abstract syntax tree.

*/

#ifndef __STMTNODES_DOT_H__
#define __STMTNODES_DOT_H__

#include "baseNode.h"
#include "option.h"

// Forward declarations
class ExecutionObject ;
class Program ;
class StatementList ;
class CodeLabelSymbol ;

class ExprStmt : public Node
{
 private:
  Option* typeop ;
  Option* lineop;
  Option* exprop;
  Option* nextop;

  Node* type ;
  int line ;
  Node* expr ;
  Node* next ;

 public:
  ExprStmt(Option* t, Option* l, Option* e, Option* n) ;
  ~ExprStmt() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class ForStmt : public Node
{
 private:
  Option* typeop ;
  Option* lineop;
  Option* initop;
  Option* condop;
  Option* exprop;
  Option* bodyop;
  Option* nextop;

  Node* type ;
  int line ;
  Node* init ;
  Node* cond ;
  Node* expr ;
  Node* body ;
  Node* next ;

 public:
  ForStmt(Option* t, Option* l, Option* i, Option* c, Option* e, 
	  Option* b, Option* n) ;
  ~ForStmt();
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class IfStmt : public Node
{
 private:
  Option* typeop ;
  Option* lineop;
  Option* condop;
  Option* thenop;
  Option* elseop;
  Option* nextop;

  Node* type ;
  int line ;
  Node* cond ;
  Node* then ;
  Node* els ;
  Node* next ;

 public:

  int thenState;
  int elseState;

  IfStmt(Option* ty, Option* l, Option* c, Option* t, Option* e, Option* n) ;
  ~IfStmt() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class CleanupStmt : public Node
{
 private:
  Option* lineop;
  Option* declop;
  Option* exprop;
  Option* nextop;

  Node* line ;
  Node* decl ;
  Node* expr ;
  Node* next ;

 public:
  CleanupStmt(Option* l, Option* d, Option* e, Option* n) ;
  ~CleanupStmt() ;
  void connect(Function* t);
  void flatten() ;
};

class WhileStmt : public Node
{
 private:

  Option* typeop ;
  Option* lineop ;
  Option* condop ;
  Option* bodyop ;

  Node* type ;
  int line ;
  Node* cond ;
  Node* body; 

 public:

  WhileStmt(Option* t, Option* l, Option* c, Option* b) ;
  ~WhileStmt() ;
  void connect(Function* t) ;
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class DoStmt : public Node
{
 private:

  Option* typeop ;
  Option* lineop ;
  Option* bodyop ;
  Option* condop ;

  Node* type ;
  int line ;
  Node* body; 
  Node* cond ;

 public:

  DoStmt(Option* t, Option* l, Option* b, Option* c) ;
  ~DoStmt() ;
  void connect(Function* t) ;
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class SwitchStmt : public Node
{

 private:

  Option* typeop ;
  Option* lineop ;
  Option* condop ;
  Option* bodyop ;

  Node* type ;
  int line ;
  Node* cond ;
  Node* body; 

  // A helper function to find the location of the default label

  CodeLabelSymbol* findDefault(StatementList* caseCode, Program* p) ;

 public:
  
  SwitchStmt(Option* t, Option* l, Option* c, Option* b) ;
  ~SwitchStmt() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class BreakStmt : public Node
{
 private:

  Option* typeop ;
  Option* lineop ;

  Node* type ;
  int line ;

 public:

  BreakStmt(Option* t, Option* l) ;
  ~BreakStmt() ;

  void connect(Function* t) ;
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

} ;

#endif
