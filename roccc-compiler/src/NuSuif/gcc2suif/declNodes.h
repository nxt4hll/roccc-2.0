/*

  This file contains the declarations of all of the declaration nodes used
   in the abstract syntax tree of gcc.

  These nodes represent the variables, procedures, arrays, parameters, 
   and other types of declared objects in a C program.

*/

#ifndef __DECLNODES_DOT_H__
#define __DECLNODES_DOT_H__

#include <common/lstring.h>
#include <common/suif_list.h>

#include "baseNode.h"
#include "option.h"
#include "function.h"
#include "program.h"

// Forward declarations of Suif Objects
class ExecutionObject ;
class VariableSymbol ;
class QualifiedType ;

class ConstDecl : public Node
{  
 private:
  Option* nameop;
  Option* typeop;
  Option* scpeop;
  Option* srcpop;
  Option* cnstop;

  Node* name ;
  Node* type ;
  Node* scpe ;
  LString srcp ;
  Node* cnst ;

 public:
  ConstDecl(Option* n, Option* t, Option* s, Option* s2, Option* c) ;
  ~ConstDecl() ;
  void connect(Function* t);
  void flatten() ;
};

/*

  The class VarDecl is one of the most complex of all the nodes.
    This class is responsible for holding all of the information
    regarding a variable on both the gcc side and the suif side, placing 
    itself into the appropriate suif symbol table.

*/
class VarDecl : public Node
{
 private:
  Option* nameop;
  Option* mnglop;
  Option* typeop;
  Option* scpeop;
  Option* srcpop;
  list<LString>* attributes;
  list<LString>* otherattributes;
  Option* initop;
  Option* sizeop;
  Option* algnop;
  Option* usedop;
  Option* chanop ;

  Node* name ;
  Node* mngl ;
 public:
  Node* type ; // Public just as a hack for structs right now.
 private:
  Node* scpe ;
  LString srcp ;
  Node* init ;
  Node* size ;
  int algn ;
  int used ;
  Node* chan ;

  bool enteredInSuif ;
  Symbol* suifSymbol ;

 public:

  VarDecl(Option* n, Option* m, Option* t, Option* s, Option* s2,
          list<LString>* a, list<LString>* o, Option* i, Option* s3,
          Option* a2, Option* u, Option* c) ;
  ~VarDecl() ;
  void connect(Function* t);
  void flatten() ;
  LString resultRegister() ;
  Node* getScope() ;

  ExecutionObject* generateSuif(Program* p) ;

  Symbol* getVariable(Program* p) ;
  Type* suifType(Program* p) ;

  // Function specific to VarDecl only
  VarDecl* getNextVariable() ;
};

class ParmDecl : public Node
{
 private:
  Option* nameop;
  Option* typeop;
  Option* scpeop;
  Option* srcpop;
  list<LString>* attributes;
  Option* argtop;
  Option* sizeop;
  Option* algnop;
  Option* usedop;
  Option* chanop ;

  bool enteredInSuif ;
  Symbol* suifSymbol ;

 public:
  Node* name ;
  Node* type ;
  Node* scpe ;
  LString srcp ;
  Node* argt ;
  Node* size ;
  int algn ;
  int used ;
  Node* chan ;

  ParmDecl(Option* n, Option* t, Option* s, Option* s2, list<LString>* a,
           Option* a2, Option* s3, Option* a3, Option* u, Option* c) ;
  ~ParmDecl() ;
  void connect(Function* t);
  void flatten() ;
  LString resultRegister() ;
  Node* getScope() ; 
  Node* getChan() ;

  Symbol* getVariable(Program* p) ;
  ExecutionObject* generateSuif(Program* p) ;

  Type* suifType(Program* p) ;
};

class FieldDecl : public Node
{
 private:
  Option* nameop;
  Option* mnglop;
  Option* typeop;
  Option* scpeop;
  Option* srcpop;
  list<LString>* attributes;
  Option* sizeop;
  Option* algnop;
  Option* bposop;
  Option* chanop;

  Node* name ;
  Node* mngl ;
  Node* type ;
  Node* scpe ;
  LString srcp ;
  Node* size ;
  int algn ;
  Node* bpos ;
  Node* chan;

  bool enteredInSuif ;
  Symbol* suifSymbol ;

 public:
  FieldDecl(Option* n, Option* m, Option* t, Option* s, Option* s2,
            list<LString>* a, Option* s3, Option* a2, Option* b, 
	    Option* c) ;
  ~FieldDecl() ;
  void connect(Function* t);
  void flatten() ;
  LString resultRegister() ;

  Symbol* getVariable(Program* p) ;

  ExecutionObject* generateSuif(Program* p) ;

};

class ResultDecl : public Node
{
 private:
  Option* typeop;
  Option* scpeop;
  Option* srcpop;
  list<LString>* attributes;
  Option* sizeop;
  Option* algnop;

  Node* type ;
  Node* scpe ;
  LString srcp ;
  Node* size ;
  int algn ;

 public:
  ResultDecl(Option* t, Option* s, Option* s2, list<LString>* attr, Option* s3, Option* a) ;
  ~ResultDecl() ;
  void connect(Function* t);
  void flatten() ;
};

class FunctionDecl : public Node
{
 private:
  Option* nameop;
  Option* mnglop;
  Option* typeop;
  Option* scpeop;
  Option* srcpop;
  list<LString>* attributes;
  Option* prioop;
  Option* dltaop;
  Option* vcllop;
  Option* argsop;
  Option* fnop;
  list<LString>* otherattributes;
  Option* bodyop;

  Node* name ;
  Node* mngl ;
  Node* type ;
  Node* scpe ;
  LString srcp ;
  int prio ;
  int dlta ;
  Node* vcll ;
  Node* args ;
  Node* fn ;
  Node* body ;

  bool enteredInSuif ;
  Symbol* suifSymbol ;

 public:

  FunctionDecl(Option* n, Option* m, Option* t, Option* s, Option* s2,
               list<LString>* a, Option* p, Option* d, Option* v,
               Option* f, Option* a2, list<LString>* o, Option* b) ;
  ~FunctionDecl() ;
  void connect(Function* t);
  void flatten() ;

  LString resultRegister() ;

  Symbol* getVariable(Program* p) ;

  Type* suifType(Program* p) ;

  QualifiedType* findReturnType(Program* p) ;

  Node* getArgs() ;
};

class LabelDecl : public Node
{
 private:
  Option* nameop ;
  Option* typeop ;
  Option* scpeop ;
  Option* srcpop ;

  Node* name ;
  Node* type ;
  Node* scpe ;
  LString srcp ;

  Symbol* suifSymbol ;

 public:
  LabelDecl(Option* n, Option* t, Option* s, Option* s2) ;
  ~LabelDecl() ;
  void connect(Function* t);
  void flatten() ;
  LString theName() ;

  Symbol* getVariable(Program* p) ;
};

class TypeDecl : public Node
{
 private:
  Option* nameop;
  Option* typeop;
  Option* scpeop;
  Option* srcpop;
  list<LString>* attributes;

  Node* name ;
  Node* type ;
  Node* scpe ;
  LString srcp ;

 public:
  TypeDecl(Option* n, Option* t, Option* s, Option* s2, list<LString>* a) ;
  ~TypeDecl() ;
  void connect(Function* t);
  void flatten() ;

  LString theName() ;

  Type* suifType(Program* p) ;

};

class NamespaceDecl : public Node
{
 private:
  Option* nameop;
  Option* typeop;
  Option* srcpop;
  Option* cop;

  Node* name ;
  Node* type ;
  LString srcp ;
  bool c ;
 public:
  NamespaceDecl(Option* n, Option* t, Option* s, Option* c) ;
  ~NamespaceDecl() ;
  void connect(Function* t);
  void flatten() ;
};

class TranslationUnitDecl : public Node
{
 private:
 public:
} ;

class TemplateDecl : public Node
{
 private:
 public:
};

class UsingDecl : public Node
{
 private:
 public:
};

#endif
