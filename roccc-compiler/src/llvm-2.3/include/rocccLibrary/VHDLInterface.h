#ifndef _VHDLINTERFACE_H__
#define _VHDLINTERFACE_H__

#include "llvm/Value.h"

#include <string>
#include <vector>
#include <list>
#include <map>

namespace VHDLInterface {

class ValueOwner;
class Wrap;

class Value {
  ValueOwner* owner;
protected:
  virtual void setOwner(ValueOwner* v); //from Value
  virtual ValueOwner* getOwner(); //from Value
public:
  Value();
  virtual int getSize()=0;
  virtual std::string generateCode(int size)=0;
  std::string generateDefaultCode();
  virtual std::string generateAssignmentOperator(); //from Value
  virtual std::string getName(); //from Value
  virtual std::string getInternalName(); //from Value
  virtual void setReadFrom()=0;
  virtual void setWrittenTo()=0;
  bool isOwned();
  friend class ValueOwner;
  friend class BitRangeImpl;
  virtual ~Value();
};

class Attribute {
  std::string name;
public:
  Attribute(std::string n);
  virtual ~Attribute();
  std::string getName();
};

class VHDLAttribute : public Attribute {
  std::string type;
  bool isDeclared;
public:
  VHDLAttribute(std::string n, std::string t);
  virtual std::string generateDeclarationCode(Value* v, std::string value); //from VHDLAttribute
};

class Attributeable {
  std::map<Attribute*, std::string> attributes;
  bool hasCalledGenerate;
public:
  Attributeable();
  virtual ~Attributeable();
  virtual void addAttribute(Attribute* a, std::string value);
  std::string getAttributeValue(Attribute*);
  virtual std::string generateAttributeCode();
};

class Variable : public Value, public Attributeable {
public:
#ifdef LLVM_VALUE_H
  typedef llvm::Value T;
#else
  typedef int T;
#endif
private:
  int size;
  T* value;
  bool is_declared;
protected:
  void setSize(int s);
public:
  virtual bool isUnsigned(); //from Variable
  Variable(int s, T* v = NULL);
  virtual int getSize(); //from Value
  T* getLLVMValue();
  virtual std::string generateCode(int size); //from Value
  virtual Value* generateResetValue(); //from Variable
protected:
  void declare();
};

class Wrap {
public:
  Value* val;
  explicit Wrap(Value* v) : val(v) {}
  operator Value*()
  {
    return val;
  }
};

class VWrap {
public:
  Variable* val;
  explicit VWrap(Variable* v) : val(v) {}
  operator Variable*()
  {
    return val;
  }
  operator Wrap()
  {
    return Wrap(val);
  }
};

class BitRange {
public:
  static VWrap get(Variable* v, int h, int l);
};

class ComponentDeclaration;
  
class Port : public Variable {
public:
  enum TYPE {INPUT,OUTPUT};
private:
  ComponentDeclaration* parent;
  std::string name;
  TYPE type;
  bool disp_ext;
public:
  Port(ComponentDeclaration* p, std::string n, int s, TYPE t, Variable::T* v=NULL, bool ext = true);
  TYPE getType();
  virtual std::string getName(); //from Value
  virtual std::string getInternalName(); //from Value
  virtual std::string generateDeclarationCode(); //from Port
  ComponentDeclaration* getParent();
  virtual void setReadFrom(); //from Value
  virtual void setWrittenTo(); //from Value
};

class Generic : public Value {
  ComponentDeclaration* parent;
  Value* default_value;
  std::string name;
protected:
  virtual std::string generateType()=0;
public:
  Generic(ComponentDeclaration* p, std::string n, Value* v=NULL);
  //virtual int getSize(); //from Value
  //virtual std::string generateCode(int size); //from Value
  virtual std::string generateDeclarationCode(); //from Generic
  ComponentDeclaration* getParent();
  virtual std::string getInternalName(); //from Value
  Value* getDefaultValue();
  virtual void setReadFrom(); //from Value
  virtual void setWrittenTo(); //from Value
};

class IntegerGeneric : public Generic {
  int value;
protected:
  virtual std::string generateType(); //from Generic
public:
  IntegerGeneric(ComponentDeclaration* p, std::string n, int v);
  virtual int getSize(); //from Value
  virtual std::string generateCode(int size); //from Value
  virtual std::string generateDeclarationCode(); //from Generic
  static Value* getTypeMatchedInteger(int v);
};

class IntegerTypedValue {
  IntegerTypedValue(); //DO NOT IMPLEMENT
public:
  static Wrap get(int v);
};

class ComponentDeclaration {
  std::string name;
  std::vector<Port*> ports;
  std::vector<Generic*> generics;
  Port* addPort(Port* p);
  void addStandardPorts();
  std::string generateCode();
  bool is_declared;
public:
  struct StandardPorts {
    Port* clk;
    Port* rst;
    Port* inputReady;
    Port* outputReady;
    Port* done;
    Port* stall;
    StandardPorts(Port* c, Port* r, Port* i, Port* o, Port* d, Port* s) : clk(c), rst(r), inputReady(i), outputReady(o), done(d), stall(s) {}
  };
  StandardPorts getStandardPorts();
  ~ComponentDeclaration();
  ComponentDeclaration(std::string n);
  std::vector<Port*>::iterator firstNonStandardPort();
  Port* addPortCopy(Variable* v, Port::TYPE t);
  Port* addPort(std::string n, int s, Port::TYPE t, Variable::T* v=NULL, bool show_extension=true);
  Generic* addGeneric(Generic* g);
  std::string getName();
  std::vector<Port*>& getPorts();
  std::vector<Generic*>& getGenerics();
  std::string generateEntityDeclaration();
  std::string generateComponentDeclaration();
  std::vector<Port*> getPort(Variable::T* v);
  std::vector<Port*> getPort(std::string n);
  Generic* getGeneric(std::string n);
  //bool isDeclared();
};

class Signal : public Variable {
  std::string name;
  bool read_from;
  bool written_to;
protected:
  bool isReadFrom();
  bool isWrittenTo();
public:
  Signal(std::string n, int s, Variable::T* v = NULL);
  virtual std::string getInternalName(); //from Value
  virtual std::string generateDeclarationCode(); //from Signal
  virtual void setReadFrom(); //from Value
  virtual void setWrittenTo(); //from Value
};

class ConstantInt {
public:
  static Wrap get(int v);
};

class ConstantFloat {
public:
  static Wrap get(float v);
};

class NamedConstant : public Signal {
  Value* value;
protected:
  virtual bool isUnsigned(); //from Variable
  bool is_unsigned;
public:
  NamedConstant(std::string n, int v, Variable::T* t=NULL);
  NamedConstant(std::string n, float v, Variable::T* t=NULL);
  virtual std::string generateDeclarationCode(); //from Signal
  virtual void setWrittenTo(); //from Value
  virtual Value* generateResetValue(); //from Variable
  void resize(int s);
};

class State : public Value {
  std::string name;
public:
  State(std::string n);
  virtual int getSize(); //from Value
  virtual std::string generateCode(int size); //from Value
  virtual void setReadFrom(); //from Value
  virtual void setWrittenTo(); //from Value
  virtual std::string getInternalName(); //from Value
};

class StateVar : public Signal {
  std::vector<State*> states;
public:
  StateVar(std::string n, int=-1, Variable::T* v=NULL); //int argument is only to allow StateVar to be initialized like a Signal
  virtual int getSize(); //from Value
  virtual std::string generateCode(int size); //from Value
  virtual std::string generateDeclarationCode(); //from Signal
  virtual Value* generateResetValue(); //from Variable
  State* insertState(std::string n, std::vector<State*>::iterator i);
  State* insertState(State* s, std::vector<State*>::iterator i);
  State* addState(std::string n);
  State* addState(State* s);
  std::vector<State*>& getStates();
};

Wrap operator + (Wrap lhs, Wrap rhs);
Wrap operator - (Wrap lhs, Wrap rhs);
Wrap operator * (Wrap lhs, Wrap rhs);
Wrap operator / (Wrap lhs, Wrap rhs);
Wrap operator & (Wrap lhs, Wrap rhs); //bitwise and
Wrap operator | (Wrap lhs, Wrap rhs); //bitwise or
Wrap operator ^ (Wrap lhs, Wrap rhs); //bitwise xor
Wrap shift_left_logical(Variable* lhs, int rhs);
Wrap shift_right_logical(Variable* lhs, int rhs);
Wrap shift_right_arithmetic(Variable* lhs, int rhs);
Wrap bitwise_concat(Value* lhs, Value* rhs);

class HasReplaceAllUsesOfWith {
  virtual HasReplaceAllUsesOfWith* cloneImpl()=0; //from HasReplaceAllUsesOfWith
protected:
  HasReplaceAllUsesOfWith(){}
public:
  virtual void replaceAllUsesOfWith(Value* of, Value* with)=0; //from HasReplaceAllUsesOfWith
  template<class ParentClass> ParentClass* clone()
  {
    ParentClass* ret = dynamic_cast<ParentClass*>(cloneImpl());
    assert(ret);
    return ret;
  }
};
template<class T>
T* getClonedCopyIfPossible(T* v)
{
  HasReplaceAllUsesOfWith* ra_v = dynamic_cast<HasReplaceAllUsesOfWith*>(v);
  if( ra_v )
    return ra_v->clone<T>();
  else
    return v;
}
template<class T>
T* ReplaceAllUsesOfWithIn(Value* of, Value* with, T* in)
{
  HasReplaceAllUsesOfWith* ra_v = dynamic_cast<HasReplaceAllUsesOfWith*>(in);
  if( ra_v )
  {
    ra_v->replaceAllUsesOfWith(of,with);
  }
  if( in == reinterpret_cast<T*>(of) )
  {
    assert(dynamic_cast<T*>(with) and "Cannot replace a value with a different type!");
    return dynamic_cast<T*>(with);
  }
  assert(in);
  return in;
}

class Condition : public HasReplaceAllUsesOfWith {
public:
  static Condition* NONE;
protected:
  Condition();
public:
  virtual std::string generateCode()=0;
  virtual void setReadFrom()=0;
};

class CWrap {
public:
  Condition* val;
  explicit CWrap(Condition* v) : val(v) {}
  operator Condition*()
  {
    return val;
  }
};

class ConstCondition {
public:
  static CWrap get(bool);
};

class EventCondition {
public:
  static CWrap rising_edge(Variable*);
  static CWrap falling_edge(Variable*);
  static CWrap event(Variable*);
};

CWrap operator == (Wrap lhs, Wrap rhs);
CWrap operator < (Wrap lhs, Wrap rhs);
CWrap operator > (Wrap lhs, Wrap rhs);
CWrap operator != (Wrap lhs, Wrap rhs);
CWrap operator >= (Wrap lhs, Wrap rhs);
CWrap operator <= (Wrap lhs, Wrap rhs);

CWrap SLT(Wrap lhs, Wrap rhs);
CWrap SGT(Wrap lhs, Wrap rhs);
CWrap SLTE(Wrap lhs, Wrap rhs);
CWrap SGTE(Wrap lhs, Wrap rhs);

CWrap operator and (CWrap lhs, CWrap rhs);
CWrap operator or (CWrap lhs, CWrap rhs);
CWrap operator not(CWrap rhs);

class ValueOwner {
protected:
  std::vector<Value*> vals;
public:
  ValueOwner();
  virtual void add(Value* v); //from ValueOwner, DO NOT CALL EXCEPT FROM DERIVED CLASSES
  bool owns(Value* v);
  virtual std::string getName()=0;
  virtual bool canTransferOwnershipOfTo(Value* v, ValueOwner* vo); //from ValueOwner
  friend class BitRangeImpl;
};

class ProcessVariable : public Variable {
  std::string name;
  bool read_from;
  bool written_to;
protected:
  bool isReadFrom();
  bool isWrittenTo();
public:
  ProcessVariable(std::string n, int s, Variable::T* v = NULL);
  virtual std::string generateAssignmentOperator(); //from Value
  virtual std::string getInternalName(); //from Value
  virtual std::string generateDeclarationCode(); //from ProcessVariable
  virtual void setReadFrom(); //from Value
  virtual void setWrittenTo(); //from Value
};

class Entity;

class Process : public ValueOwner, public HasReplaceAllUsesOfWith {
  Entity* parent;
  Variable* _clk;
  bool hasReset;
  virtual HasReplaceAllUsesOfWith* cloneImpl(); //from HasReplaceAllUsesOfWith
protected:
  virtual std::vector<VHDLInterface::Variable*> getSensitivityList(); //from Process
  virtual std::string generateResetCode(int level); //from Process
  Entity* getParent();
  virtual std::string generateSteadyState(int level)=0;
  virtual std::string getName(); //from ValueOwner
public:
  Process(Entity* p, Variable* clk=NULL);
  void setClockDomain(Variable* clk);
  Variable* getClockDomain();
  void setHasReset(bool hr);
  virtual std::string generateCode(); //from Process
  virtual void replaceAllUsesOfWith(Value* of, Value* with); //from HasReplaceAllUsesOfWith
};

class Statement : public HasReplaceAllUsesOfWith {
  Process* parent;
public:
  Statement(Process* p);
  Process* getParent();
  virtual std::string generateCode(int level)=0;
};

// This if statement class only has an else portion, it does not
//  have an elsif portion.
class IfStatement : public Statement {
  Condition* cond;
  Statement* true_statement;
  Statement* false_statement;
  virtual HasReplaceAllUsesOfWith* cloneImpl(); //from HasReplaceAllUsesOfWith
public:
  IfStatement(Process* p, Condition* c, Statement* t, Statement* f=NULL);
  virtual std::string generateCode(int level); //from Statement
  virtual void replaceAllUsesOfWith(Value* of, Value* with); //from HasReplaceAllUsesOfWith
};

// Added on March 5, 2013 in order to address an issue with code generation
//  on the Virtex 6.  This code could change to a case statement when
//  generating code if necessary.
class IfElsifStatement : public Statement {
  std::list<Condition*> conditions ;
  std::list<Statement*> statements ;
  virtual HasReplaceAllUsesOfWith* cloneImpl(); //from HasReplaceAllUsesOfWith
 public:
  IfElsifStatement(Process* p) ;
  IfElsifStatement(Process* p, 
		   std::list<Condition*> c, std::list<Statement*> s);
  void addCase(Condition* c, Statement* s) ;
  virtual std::string generateCode(int level) ; // from Statement
  virtual void replaceAllUsesOfWith(Value* of, Value* with) ;
};

class MultiStatement : public Statement {
  std::vector<Statement*> statements;
  virtual HasReplaceAllUsesOfWith* cloneImpl(); //from HasReplaceAllUsesOfWith
public:
  MultiStatement(Process* p);
  MultiStatement* addStatement(Statement*);
  bool isEmpty();
  virtual std::string generateCode(int level);//from Statement
  virtual void replaceAllUsesOfWith(Value* of, Value* with); //from HasReplaceAllUsesOfWith
};

class AssignmentStatement : public Statement, public ValueOwner {
  Value* lhs;
  std::vector<std::pair<Value*,Condition*> > rhs;
  virtual HasReplaceAllUsesOfWith* cloneImpl(); //from HasReplaceAllUsesOfWith
public:
  AssignmentStatement(Value* l, Process* p = NULL);
  AssignmentStatement(Value* l, Value* r, Process* p = NULL);
  virtual void add(Value* v); //from ValueOwner, DO NOT CALL
  AssignmentStatement* addCase(Value* v, Condition* c = Condition::NONE);
  virtual std::string getName(); //from ValueOwner
  virtual std::string generateCode(int level); //from Statement
  Value* getSingleValue();//returns NULL if there are multiple values
  virtual void replaceAllUsesOfWith(Value* of, Value* with); //from HasReplaceAllUsesOfWith
  void removeLastCase() ; // Removes the last element of the statement
};

// This case statement actually is for an FSM, not a general case statement
class CaseStatement : public Statement {
  std::vector< std::pair<State*,Statement*> > cases;
  StateVar* state_var;
  virtual HasReplaceAllUsesOfWith* cloneImpl(); //from HasReplaceAllUsesOfWith
public:
  CaseStatement(Process* p, StateVar* sv);
  CaseStatement* addCase(State* state, Statement* statement);
  CaseStatement* addStatement(State* state, Statement* statement);
  Statement* getCase(State* state);
  virtual std::string generateCode(int level); //from Statement
  virtual void replaceAllUsesOfWith(Value* of, Value* with); //from HasReplaceAllUsesOfWith
};

class MultiStatementProcess : public Process {
  MultiStatement* ms;
  virtual HasReplaceAllUsesOfWith* cloneImpl(); //from HasReplaceAllUsesOfWith
protected:
  MultiStatement* getSteadyState();
  virtual std::string generateSteadyState(int level); //from Process
public:
  MultiStatementProcess(Entity* p, Variable* clk=NULL);
  MultiStatementProcess* addStatement(Statement* s);
  virtual void replaceAllUsesOfWith(Value* of, Value* with); //from HasReplaceAllUsesOfWith
};

class CommentStatement : public Statement {
  std::string comment;
  virtual HasReplaceAllUsesOfWith* cloneImpl(); //from HasReplaceAllUsesOfWith
public:
  CommentStatement(Process* p, std::string c);
  virtual std::string generateCode(int level); //from Statement
  virtual void replaceAllUsesOfWith(Value* of, Value* with); //from HasReplaceAllUsesOfWith
};

class Entity;

class ComponentDefinition : public ValueOwner {
  std::string name;
  std::map<Port*, Variable*> mappings;
  std::map<Generic*, Value*> genericMap;
  ComponentDeclaration* declaration;
  ComponentDefinition& operator =(ComponentDefinition&);
public:
  ComponentDefinition(std::string n, ComponentDeclaration* d);
  void map(Variable* s, Port* p);
  void map(Variable* s, std::string name);
  void mapGeneric(Value* s, Generic* g);
  void mapGeneric(Value* s, std::string name);
  virtual std::string getName(); //from ValueOwner
  Signal* map(ComponentDefinition* cde, Port* external, Port* internal, std::string n="");
  Signal* map( ComponentDefinition* cde, std::string external, std::string internal, std::string n="" );
  bool isMapped(Port*);
  std::map<Port*, Variable*>& getMap();
  std::map<Generic*, Value*>& getGenericMap();
  ComponentDeclaration* getDeclaration();
  virtual std::string generateCode();
  virtual void setParent(Entity* e); //from ComponentDefinition
};

class Entity {
protected:
  std::vector<Signal*> signals;
  std::map< std::string,std::vector<Signal*> > signalNameMap;
  std::map< llvm::Value*,std::vector<Signal*> > signalValueMap;
  std::vector<AssignmentStatement*> synchronous_statements;
  std::vector<Process*> processes;
  std::vector<ComponentDefinition*> components;
  std::vector<Attribute*> attributes;
  ComponentDeclaration* declaration;
public:
  Entity(std::string name);
  Port* addPortCopy(Variable* v, Port::TYPE t); //calls the declaration's addPortCopy()
  Port* addPort(std::string name, int size, Port::TYPE direction, Variable::T* v = NULL); //adds a port to the entity def
  void addSignal(Signal*);
  template<class SignalClass> SignalClass* createSignal(std::string name, int size, Variable::T* v = NULL)
  {
    SignalClass* ret = new SignalClass(name, size, v);
    Signal* conv = ret;
    addSignal(conv);
    return ret;
  }
  AssignmentStatement* createSynchronousStatement(Value* v, Value* rhs=NULL);
  template<class ProcessClass> ProcessClass* createProcess(Variable* clk=NULL)
  {
    ProcessClass* ret = new ProcessClass(this);
    if( clk )
      ret->setClockDomain(clk);
    else
      ret->setClockDomain(getDeclaration()->getStandardPorts().clk);
    Process* conv = ret;
    processes.push_back( conv );
    return ret;
  }
  Attribute* getAttribute(std::string);
  Attribute* addAttribute(Attribute* att);
  ComponentDefinition* addComponent(ComponentDefinition* cd);
  ComponentDefinition* createComponent(std::string name, ComponentDeclaration* cd);
  StateVar* createStateVar(std::string name, int num_elements = 0);
  State* insertBufferState(StateVar*);
  State* getFirstState(StateVar*);
  State* getLastState(StateVar*);
  int getNumBufferStates(StateVar*);
  State* getBufferState(StateVar*,int i);
  std::string generateCode();
  ComponentDeclaration* getDeclaration();
  Signal* mapSubComponentPorts(ComponentDefinition* c1, Port* p1, ComponentDefinition* c2, Port* p2); //maps a port on one component to a port from another component, and returns the signal used to make the connection
  void mapPortToSubComponentPort(Port* p1, ComponentDefinition* c2, Port* p2); //maps a port on the entity to a port on a subcomponent
  std::vector<Signal*> findSignal(Variable::T* t);
  std::vector<Signal*> findSignal(std::string n);
  std::vector<Port*> getPort(Variable::T* v);
  std::vector<Port*> getPort(std::string n);
  ComponentDeclaration::StandardPorts getStandardPorts();
  Variable* getVariableMappedTo(ComponentDefinition* cd, Port* p); //gets the value that is mapped to a components port, creating it if it doesnt exist
  Signal* getSignalMappedToPort(Port* p); //gets the signal that is mapped to the entitys port, creating if it doesnt exist
  //basic getter/setter functions
  //std::vector<AssignmentStatement*> getSynchronousStatements();
  //std::vector<Process*> getProcesses();
  //std::vector<Signal*> getSignals();
};

#if 0
class VHDLArray : public VHDLInterface::Signal {
  class ArrayElement : public VHDLInterface::Value {
    std::string name;
    VHDLInterface::Value* index;
    VHDLArray* parent;
  public:
    ArrayElement(std::string n, VHDLInterface::Value* i, VHDLArray* p) : Value(), name(n), index(i), parent(p)
    {
      assert(i);
      assert(p);
    }
    virtual int getSize()
    {
      return 32;
    }
    virtual std::string getInternalName() //from Value
    {
      return name;
    }
    virtual std::string generateCode(int size)
    {
      std::stringstream ss;
      ss << getName() << "(" << index->generateDefaultCode() << ")";
      return ss.str();
    }
    virtual void setReadFrom()
    {
      index->setReadFrom();
      parent->setReadFrom();
    }
    virtual void setWrittenTo()
    {
      assert(0 and "Cannot write to array element!");
    }
  };
  std::vector<int> values;
  
public:
  VHDLArray(std::string n, int=-1, Variable::T* v=NULL) : VHDLInterface::Signal(n,1)
  {
  }
  virtual int getSize() //from Value
  {
    return 32;
  }
  virtual std::string generateCode(int size) //from Value
  {
    assert(0 and "Cannot generate a ConstArray!");
  }
  virtual std::string generateDeclarationCode() //from Signal
  {
    Variable::declare();
    std::stringstream ss;
    assert( values.size() > 0 );
    ss << "type " << getName() << "_type is array (0 to " << values.size()-1 << ") of std_logic_vector(31 downto 0) ;\n";
    ss << "constant " << getName() << " : " << getName() << "_type := (\n";
    int count = 0;
    for(std::vector<int>::iterator DI = values.begin(); DI != values.end(); ++DI, ++count)
    {
       if( DI != values.begin() )
         ss << ",\n";
       ss << "    "  << count << " => " << "x\"" << std::setw(8) << std::setfill('0') << std::hex << *DI << std::setfill(' ') << std::dec << "\"";
    }
    ss << ") ;\n";
    return ss.str();
  }
  virtual VHDLInterface::Value* generateResetValue() //from Variable
  {
    assert(0 and "Cannot reset a ConstArray!");
  }
  VHDLInterface::Value* getElement(VHDLInterface::Value* v)
  {
    return new ArrayElement(getName(), v, this);
  }
  void addElement(int v)
  {
    values.push_back(v);
  }
};
#endif

}

#endif
