/*

  This file contains all of the nodes in the gcc abstract syntax tree that 
   correspond to types.

*/

#ifndef __TYPENODES_DOT_H__
#define __TYPENODES_DOT_H__

#include <common/lstring.h>
#include <common/suif_list.h>

#include "baseNode.h"
#include "option.h"
#include "function.h"

// Forward declarations
class QualifiedType ;
class Program ;
class GroupSymbolTable ;

class VoidTypeMine : public Node
{
 private:
  Option* qualop ;
  Option* unqlop ;
  Option* nameop ;
  Option* algnop ;
  
  char qual ;
  Node* unql ;
  Node* name ;
  int algn ;
  
 public:
  VoidTypeMine(Option* q, Option* u, Option* n, Option* a) ;
  ~VoidTypeMine() ;
  void connect(Function* t) ;
  void flatten() ;

  Type* suifType(Program* p) ;

};

class IntegerTypeMine : public Node
{
 private:
  Option* qualop;
  Option* nameop;
  Option* unqlop;
  Option* sizeop;
  Option* algnop;
  Option* precop;
  list<LString>* attributes;
  Option* minop;
  Option* maxop;

  char qual ;
  Node* name ;
  Node* unql ;
  Node* size ;
  int algn ;
  int prec ;
  Node* min ;
  Node* max ;

 public:
  IntegerTypeMine(Option* q, Option* n, Option* u, Option* s, Option* a,
              Option* p, list<LString>* a2, Option* m, Option* m2) ;
  ~IntegerTypeMine() ;
  void connect(Function* t);
  void flatten() ;
  int theSize(int arraylevel) ;
  int theSize() ;

  Type* suifType(Program* p) ;

  ExecutionObject* getMax(Program* p) ;
  ExecutionObject* getMin(Program* p) ;

  int getMaxNumber() ;

  bool isConst() ;
  bool isVolatile() ;

  LString theName() ;

};

class EnumeralType : public Node
{
 private:
  Option* qualop;
  Option* nameop;
  Option* unqlop;
  Option* sizeop;
  Option* algnop;
  Option* precop;
  list<LString>* attributes;
  Option* minop;
  Option* maxop;
  Option* cstsop;

  char qual ;
  Node* name ;
  Node* unql ;
  Node* size ;
  int algn ;
  int prec ;
  Node* min ;
  Node* max ;
  Node* csts ;

 public:
  EnumeralType(Option* q, Option* n, Option* u, Option* s,
               Option* a, Option* p, list<LString>* a2, Option* m,
               Option* m2, Option* c) ;
  ~EnumeralType() ;
  void connect(Function* t);
  void flatten() ;

  Type* suifType(Program* p) ;

};

class RealType : public Node
{
 private:
  Option* qualop ;
  Option* nameop;
  Option* unqlop ;
  Option* sizeop;
  Option* algnop;
  Option* precop;

  char qual ;
  Node* name ;
  Node* unql ;
  Node* size ;
  int algn ;
  int prec ;

 public:
  RealType(Option* q, Option* n, Option* u, Option* s, Option* a, Option* p) ;
  ~RealType();
  void connect(Function* t);
  void flatten() ;  

  bool isConst() ;

  Type* suifType(Program* p) ;
  int theSize() ;
};

class PointerTypeMine : public Node
{
 private:
  Option* qualop;
  Option* nameop;
  Option* unqlop;
  Option* sizeop;
  Option* algnop;
  Option* ptdop;

  char qual ;
  Node* name ;
  Node* unql ;
  Node* size ;
  int algn ;
  Node* ptd ;

 public:
  PointerTypeMine(Option* q, Option* n, Option* u, Option* s, Option* a, 
	      Option* p) ;
  ~PointerTypeMine() ;
  void connect(Function* t);
  void flatten() ;

  Type* suifType(Program* p) ;

};

class ReferenceTypeMine : public Node
{
 private:
  Option* sizeop;
  Option* algnop;
  Option* refdop;

  Node* size ;
  int algn ;
  Node* refd ;

 public:
  ReferenceTypeMine(Option* s, Option* a, Option* r) ;
  ~ReferenceTypeMine() ;
  void connect(Function* t);
  void flatten() ;

  Type* suifType(Program* p) ;

};

class MethodType : public Node
{
 private:
  Option* unqlop;
  Option* sizeop;
  Option* algnop;
  Option* clasop;
  Option* retnop;
  Option* prmsop;

  Node* unql ;
  Node* size ;
  int algn ;
  Node* clas ;
  Node* retn ;
  Node* prms ;

 public:
  MethodType(Option* u, Option* s, Option* a, Option* c, Option* r, Option* p);
  ~MethodType() ;
  void connect(Function* t);
  void flatten() ;

  Type* suifType(Program* p) ;

};

class FunctionType : public Node
{
 private:
  Option* unqlop;
  Option* sizeop;
  Option* algnop;
  Option* retnop;
  Option* prmsop;

  Node* unql ;
  Node* size ;
  int algn ;
  Node* retn ;
  Node* prms ;

 public:
  FunctionType(Option* u, Option* s, Option* a, Option* r, Option* p) ;
  ~FunctionType() ;
  void connect(Function* t);
  void flatten() ;

  Type* suifType(Program* p) ;
  QualifiedType* findReturnType(Program* p) ;

  list<QualifiedType*> findParameters(Program* p) ;

};

class RecordType : public Node
{
 private:
  Option* qualop;
  Option* nameop;
  Option* unqlop;
  Option* sizeop;
  Option* algnop;
  Option* vfldop;
  list<LString>* attributes;
  Option* fldsop;
  Option* fncsop;
  Option* binfop;
  Option* ptdop;
  Option* clsop;

  char qual ;
  Node* name ;
  Node* unql ;
  Node* size ;
  int algn ;
  Node* vfld ;
  Node* flds ;
  Node* fncs ;
  Node* binf ;
  Node* ptd ;
  Node* cls ;

  GroupSymbolTable* fieldsInStruct ;
  Type* myCreatedType ;

 public:
  RecordType(Option* q, Option* n, Option* u, Option* s, Option* a,
             Option* v, list<LString>* a2, Option* f, Option* f2,
             Option* b, Option* p, Option* c) ;
  ~RecordType() ;
  void connect(Function* t);
  void flatten() ;

  Type* suifType(Program* p) ;

  void AddSymbolToTable(Symbol* s) ;

};

class BooleanTypeMine : public Node
{
 private:
  Option* qualop;
  Option* nameop;
  Option* unqlop;
  Option* sizeop;
  Option* algnop;

  char qual ;
  Node* name ;
  Node* unql ;
  Node* size ;
  int algn ;

 public:
  BooleanTypeMine(Option* q, Option* n, Option* u, Option* s, Option* a) ;
  ~BooleanTypeMine() ;
  void connect(Function* t);
  void flatten() ;

  Type* suifType(Program* p) ;

  int theSize() ;

};

class ArrayTypeMine : public Node
{
 private:
  Option* qualop;
  Option* unqlop;
  Option* sizeop;
  Option* algnop;
  Option* eltsop;
  Option* domnop;

  // Just to make one thing easier, this will probably change later
 public:

  char qual ;
  Node* unql ;
  Node* size ;
  int algn ;
  Node* elts ;
  // The domn tells the high and low values that the array can be
  //  indexed at.  For example, domn would point to an integer type
  //  that has a min of 0 and a max of 7, this represents an array
  //  declared as x[8] ;
  Node* domn ;

 public:
  ArrayTypeMine(Option* q, Option* u, Option* s, Option* a, Option* e, Option* d);
  ~ArrayTypeMine() ;
  void connect(Function* t);
  void flatten() ;
  //  int theSize(int arrayLevel) ;

  int theSize() ;

  Type* suifType(Program* p) ;

};

class TypenameType : public Node
{
 private:
 public:
};

class TypeofType : public Node
{
 private:
 public:
};

#endif
