/*
  
  This code will connect different options with different nodes.  It is an
   intermediate step and after flattening, will be deleted.

*/

#ifndef __OPTION_DOT_H__
#define __OPTION_DOT_H__

using namespace std;

#include <common/lstring.h>
#include "generic.h"

class Node; // Forward declaration
class Function; // Forward declaration

// The base class for the Option hierarchy does not contain any data 
//  and its functions don't do anything major
class Option 
{
 public:
  Option();
  virtual ~Option();
  virtual void setNodePointer(Node* value);
  virtual void connect(Function* t);
  //  virtual LString theType() { return "void" ; }
  virtual Node* theNodePointer() { return NULL ; } 
  virtual int theNumber() { return -1 ; } 
};

class NameOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  NameOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~NameOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class TypeOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  TypeOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~TypeOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class VarsOption : public Option
{
 private:
  Node* nodePointer ;
  int nodeNumber ;
 public:
  VarsOption(int n) : nodeNumber(n) { nodePointer = NULL ; }
  ~VarsOption() { ; } 
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class SrcpOption : public Option
{
 private:
  LString value;
 public:
  SrcpOption(LString v) : value(v) { ; }
  ~SrcpOption() { ; }
  // No connect required
  LString theValue() { return value; } 
};

class COption : public Option
{
 private:
 public:
  COption() { ; }
  ~COption() { ; }
  // No connect required
};

class ArgsOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  ArgsOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~ArgsOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class BodyOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  BodyOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~BodyOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class StrgOption : public Option
{
 private:
  LString value;
 public:
  StrgOption(LString v) : value(v) { ; }
  ~StrgOption() { ; }
  // No connect necessary
  LString theName() { return value; }
};

class LngtOption : public Option
{
 private:
  int number;
 public:
  LngtOption(int n) : number(n) { ; }
  ~LngtOption() { ; }
  // No connect necessary
  int theNumber() { return number; } 
};

class SizeOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  SizeOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~SizeOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class AlgnOption : public Option
{
 private:
  int number;
 public:
  AlgnOption(int n) : number(n) { ; }
  ~AlgnOption() { ; }
  // No connect necessary
  int theNumber() { return number; } 
};

class RetnOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  RetnOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~RetnOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class PrmsOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  PrmsOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~PrmsOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class ArgtOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  ArgtOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~ArgtOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class UsedOption : public Option
{
 private:
  int number;
 public:
  UsedOption(int n) : number(n) { ; }
  ~UsedOption() { ; }
  // No connect necessary
  int theNumber() { return number ; } 
};

class LineOption : public Option
{
 private:
  int number;
 public:
  LineOption(int n) : number(n) { ; }
  ~LineOption() { ; }
  // No connect necessary
  int theNumber() { return number; } 
};

class LowOption : public Option
{
 private:
  int number;
 public:
  LowOption(int n) : number(n) { ; }
  ~LowOption() { ; }
  // No connect necessary
  int theNumber() { return number; } 
};

class LowNodeOption : public Option
{
 private:
  Node* nodePointer ;
  int nodeNumber ;
 public:
  LowNodeOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~LowNodeOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
} ;

class HighOption : public Option
{
 private:
  int number;
 public:
  HighOption(int n) : number(n) { ; }
  ~HighOption() { ; }
  // No connect necessary
  int theNumber() { return number; }
};

class PrecOption : public Option
{
 private:
  int number;
 public:
  PrecOption(int n) : number(n) { ; }
  ~PrecOption() { ; }
  // No connect necessary
  int theNumber() { return number; } 
};

class MinOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  MinOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~MinOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class MaxOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  MaxOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~MaxOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class ValuOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  ValuOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~ValuOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class ChanOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  ChanOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~ChanOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class QualOption : public Option
{
 private:
  char value;
 public:
  QualOption(char v) : value(v) { ; }
  ~QualOption() { ; }
  // No connect is necessary
  char theValue() { return value; } 
};

class UnqlOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  UnqlOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~UnqlOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class CstsOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  CstsOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~CstsOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class PtdOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  PtdOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~PtdOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class RefdOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  RefdOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~RefdOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class ClasOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  ClasOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~ClasOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class EltsOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  EltsOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~EltsOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class DomnOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  DomnOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~DomnOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class VfldOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  VfldOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~VfldOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class FldsOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  FldsOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~FldsOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class FncsOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  FncsOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~FncsOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class BinfOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  BinfOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~BinfOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* );
  Node* theNodePointer() { return nodePointer; } 
};

class ScpeOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  ScpeOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~ScpeOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class CnstOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  CnstOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~CnstOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class MnglOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  MnglOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~MnglOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class BposOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  BposOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~BposOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class NextOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  NextOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~NextOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class DeclOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  DeclOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~DeclOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class ExprOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  ExprOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~ExprOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class CondOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  CondOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~CondOption() { ; } 
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class ThenOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  ThenOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~ThenOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class ElseOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  ElseOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~ElseOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class LablOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  LablOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~LablOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class FnOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  FnOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~FnOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class ClnpOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  ClnpOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~ClnpOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class BaseOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  BaseOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~BaseOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class StmtOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  StmtOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~StmtOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class InitOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  InitOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~InitOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class PurpOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  PurpOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~PurpOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class DltaOption : public Option
{
 private:
  int number;
 public:
  DltaOption(int n) : number(n) { }
  ~DltaOption() { ; } 
  // No connect necessary
  int theNumber() { return number; } 
};

class VcllOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  VcllOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~VcllOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class ClsOption : public Option
{
 private:
  Node* nodePointer;
  int nodeNumber;
 public:
  ClsOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~ClsOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value; }
  void connect(Function* t);
  Node* theNodePointer() { return nodePointer; } 
};

class BasesOption : public Option
{
 private:
  int number ;
 public:
  BasesOption(int n) : number(n) { ; } 
  ~BasesOption() { ; } 
  // No connect necessary
  int theNumber() { return number; } 
};

class PrioOption : public Option
{
 private:
  int number;
 public:
  PrioOption(int n) : number(n) { };
  ~PrioOption() { };
  // No connect necessary
  int theNumber() { return number; } 
};

class DestOption : public Option
{
 private:
  Node* nodePointer ;
  int nodeNumber ;
 public:
  DestOption(int n) : nodeNumber(n) { nodePointer = NULL; }
  ~DestOption() { ; }
  void setNodePointer(Node* value) { nodePointer = value ; }
  void connect(Function* t) ;
  Node* theNodePointer() { return nodePointer ; }
};

#endif
