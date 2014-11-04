// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

DESC

*/

#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Constants.h"
#include "llvm/Support/CFG.h"

#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/MessageLogger.h"
#include "rocccLibrary/ROCCCNames.h"
#include "rocccLibrary/DFFunction.h"
#include "rocccLibrary/CopyValue.h"
#include "rocccLibrary/DefinitionInst.h"
#include "rocccLibrary/GetValueName.h"
#include "rocccLibrary/DatabaseInterface.h"
#include "rocccLibrary/FileInfo.h"
#include "rocccLibrary/DatabaseHelpers.h"

#include <fstream>
#include <sstream>

using namespace llvm ;
using namespace Database;

namespace ROCCCGraph {

class Printable {
  bool has_printed;
protected:
  virtual std::string printImpl()=0;
public:
  Printable() : has_printed(false) {}
  bool hasPrinted(){return has_printed;}
  virtual std::string print()
  {
    has_printed = true;
    return printImpl();
  }
  virtual std::string getColor()=0;
};

class Connectible;

class Connection : public Printable {
private:
  Connectible* source;
  Connectible* sink;
  std::string label;
  std::string color;
protected:
  virtual std::string printImpl();
public:
  Connection(Connectible* src, Connectible* snk, std::string _label="", std::string _color="") : source(src), sink(snk), label(_label), color(_color){}
  Connectible* getSource() {return source;}
  Connectible* getSink() {return sink;}
  virtual std::string getColor();
};

class Graph;

class Connectible : public Printable {
public:
  typedef std::vector< Connection* > ConnectionVec;
  typedef ConnectionVec::iterator iterator;
private:
  Connectible::ConnectionVec incoming_connections;
  Connectible::ConnectionVec outgoing_connections;
  void addIncomingConnection(Connection* e){incoming_connections.push_back( e );}
  void addOutgoingConnection(Connection* e){outgoing_connections.push_back( e );}
protected:
  virtual std::string printImpl()
  {
    std::stringstream ss;
    for(Connectible::iterator IC = this->incomingBegin(); IC != this->incomingEnd(); ++IC)
    {
      ss << (*IC)->print();
    }
    for(Connectible::iterator OC = this->outgoingBegin(); OC != this->outgoingEnd(); ++OC)
    {
      ss << (*OC)->print();
    }
    return ss.str();
  }
public:
  Connectible() : Printable() {}
  Connectible* flowsInto(Connectible* rhs, std::string label="", std::string color="")
  {
    Connection* e = new Connection(this, rhs, label, color);
    addOutgoingConnection(e);
    rhs->addIncomingConnection(e);
    return rhs;
  }
  int numIncomingEdges(){return incoming_connections.size();}
  int numOutgoingEdges(){return outgoing_connections.size();}
  Connectible::iterator incomingBegin(){return incoming_connections.begin();}
  Connectible::iterator incomingEnd(){return incoming_connections.end();}
  Connectible::iterator outgoingBegin(){return outgoing_connections.begin();}
  Connectible::iterator outgoingEnd(){return outgoing_connections.end();}
  virtual std::string getNameID()=0;
  virtual void updateConnections()=0;
  virtual std::string getGroup()=0;
};

std::string Connection::printImpl()
{
  std::stringstream ss;
  if( this->getSource()->hasPrinted() and this->getSink()->hasPrinted() )
  {
    ss << this->getSource()->getNameID() << " -> " << this->getSink()->getNameID();
    ss << " [";
    if( label != "" )
      ss << "label=\"" << label << "\"";
    if( getColor() != "" )
      ss << "color=" << getColor() << ", fontcolor=" << getColor();
    ss << "]";
    ss << ";\n";
  }
  return ss.str();
}
std::string Connection::getColor()
{
  assert( source );
  if( color == "" and source->getColor() != "" )
  {
    return source->getColor();
  }
  return color;
}

class ConnectionPoint : public Connectible {
  Connectible* parent;
  std::string direction;
public:
  ConnectionPoint(Connectible* p, std::string d) : Connectible(), parent(p), direction(d) {}
  std::string getDirection(){return direction;}
  virtual std::string getNameID()
  {
    return parent->getNameID() + ":" + direction;
  }
  virtual void updateConnections()
  {
    parent->updateConnections();
  }
  virtual std::string getColor(){assert(parent);return parent->getColor();}
  virtual std::string getGroup(){assert(parent);return parent->getGroup();}
};

class Subgraph;

class Node : public Connectible {
  std::string name_id;
  std::map<std::string,ConnectionPoint*> connection_points;
  Subgraph* parent;
protected:
  virtual std::string getBodyText()=0;
  virtual std::string getShape()=0;
  virtual std::string printImpl()
  {
    std::stringstream ss;
    ss << this->getNameID() << " [shape=" << this->getShape() << ",color=\"" << this->getColor() << "\",label=\"";
    ss << this->getBodyText();
    ss << "\",group=\"" << this->getGroup() << "\"];\n";
    for(std::map<std::string,ConnectionPoint*>::iterator CPI = this->connectionBegin(); CPI != this->connectionEnd(); ++CPI)
    {
      ss << CPI->second->print();
    }
    ss << Connectible::printImpl();
    return ss.str();
  }
  std::map<std::string,ConnectionPoint*>::iterator connectionBegin(){return connection_points.begin();}
  std::map<std::string,ConnectionPoint*>::iterator connectionEnd(){return connection_points.end();}
public:
  Node(std::string name, Subgraph* p);
  virtual std::string getNameID(){return name_id;}
  Connectible* getConnectionPointStr(std::string cp)
  {
    if( connection_points.find(cp) == connection_points.end() )
      connection_points[cp] = new ConnectionPoint(this, cp);
    return connection_points[cp];
  }
  virtual Connectible* getConnectionPoint(llvm::Value* v)=0;
  Subgraph* getParent(){return parent;}
  virtual std::string getGroup(){return name_id;}
};

class TextNode : public Node {
  std::string body_text;
protected:
  std::string getShape(){return "box";}
public:
  TextNode(std::string name, std::string text, Subgraph* p) : Node(name, p), body_text(text){}
  virtual std::string getBodyText()
  {
    return body_text;
  }
  virtual void updateConnections(){}
  std::string getColor(){return "yellow";}
  Connectible* getConnectionPoint(llvm::Value* v)
  {
    return this;
  }
};

class HTMLNode : public Node {
protected:
  virtual std::string printImpl()
  {
    std::stringstream ss;
    ss << this->getNameID() << " [shape=" << this->getShape() << ",color=" << this->getColor() << ",label=<";
    ss << this->getBodyText();
    ss << ">];\n";
    for(std::map<std::string,ConnectionPoint*>::iterator CPI = this->connectionBegin(); CPI != this->connectionEnd(); ++CPI)
    {
      ss << CPI->second->print();
    }
    ss << Connectible::printImpl();
    return ss.str();
  }
public:
  HTMLNode(std::string name, Subgraph* p) : Node(name, p){}
};

template <class I,class T>
class GraphElementMap {
public:
  std::map<I,T> elements;
public:
  GraphElementMap(){}
  void setElement(I i, T e){assert(e); elements[i] = e;}
  T getElement(I i){assert(elementExists(i));return elements.find(i)->second;}
  int numElements(){return elements.size();}
  bool elementExists(I i){return elements.find(i) != elements.end();}
  typename std::map<I,T>::iterator elements_begin(){return elements.begin();}
  typename std::map<I,T>::iterator elements_end(){return elements.end();}
};

class Subgraph : public Printable, public GraphElementMap<Instruction*,Node*> {
  std::string label;
  Graph* parent;
protected:
  virtual std::string printImpl()
  {
    std::stringstream ss;     
#if 1
      ss << "subgraph cluster_" << this->getLabel() << " {\n";
      ss << "label=\"" << this->getLabel() << "\";\n";
      ss << "rank=same;\n";
#endif
      for(std::map<Instruction*,Node*>::iterator NI = this->elements_begin(); NI != this->elements_end(); ++NI)
      {
        ss << NI->second->print();
      }
#if 1
      /*
      for(int n = 10; n > this->numElements(); --n)
      {
        std::stringstream name;
        static int count = 0;
        name << "padding" << count++;
        ss << (new TextNode(name.str(), "", this))->print();
      }
      */
      ss << "};\n";
#endif
    return ss.str();
  }
public:
  Subgraph(std::string _label, Graph* p) : Printable(), GraphElementMap<llvm::Instruction*,Node*>(), label(_label), parent(p) {}
  std::string getLabel(){return label;}
  std::string getColor(){return "black";}
  Graph* getParent(){return parent;}
};

class Graph : public Printable, public GraphElementMap<int,Subgraph*> {
  std::string label;
protected:
  virtual std::string printImpl()
  {
    for(std::map<int,ROCCCGraph::Subgraph*>::iterator SGI = this->elements_begin(); SGI != this->elements_end(); ++SGI)
    {
      assert(SGI->second);
      for(std::map<Instruction*,ROCCCGraph::Node*>::iterator NI = SGI->second->elements_begin(); NI != SGI->second->elements_end(); ++NI)
      {
        assert(NI->second);
        NI->second->updateConnections();
      }
    }
    std::stringstream ss;
    ss << "digraph {\n";
	  ss << "label=\"" << label << "\";\n";
    ss << "rankdir=LR;\n";
    ss << "concentrate=true;\n";
    for(std::map<int,Subgraph*>::iterator LI = this->elements_begin(); LI != this->elements_end(); ++LI)
    {
      ss << LI->second->print();
    }
    ss << "};\n";
    return ss.str();
  }
public:
  Graph(std::string _label) : Printable(), GraphElementMap<int,Subgraph*>(), label(_label){}
  std::string getLabel(){return label;}
  std::string generateNextColor()
  {
    static int count = 0;
    const std::string colors[9] = {"black", "red", "blue", "green", "cyan", "purple", "orange", "grey", "brown"};
    ++count;
    count %= 9;
    return colors[count];
  }
  std::string getColor(){return "black";}
};


std::string generateTempNodeName()
{
  static int count = 0;
  std::stringstream ss;
  ss << "node" << count++;
  return ss.str();
}

Node::Node(std::string name, Subgraph* p) : name_id(name), parent(p)
{
  if( name_id == "" )
    name_id = generateTempNodeName();
}

}

bool shouldDisplayInstruction(Instruction* II)
{
  llvm::CallInst* CI = dynamic_cast<llvm::CallInst*>(&*II);
  if( isROCCCFunctionCall(CI, ROCCCNames::FunctionType) or
      isROCCCFunctionCall(CI, ROCCCNames::PortOrder) or
      isROCCCFunctionCall(CI, ROCCCNames::ModuleStructName) or
      isROCCCFunctionCall(CI, ROCCCNames::MaximizePrecision) or
      isROCCCFunctionCall(CI, ROCCCNames::InputStream) or
      isROCCCFunctionCall(CI, ROCCCNames::OutputStream) or
      isROCCCFunctionCall(CI, ROCCCNames::VariableSize) or
      isROCCCFunctionCall(CI, ROCCCNames::VariableName) or
      isROCCCFunctionCall(CI, ROCCCNames::VariableSigned) or
      isROCCCFunctionCall(CI, ROCCCNames::InductionVariableStartValue) or
      isROCCCFunctionCall(CI, ROCCCNames::InductionVariableEndValue) or
      isROCCCFunctionCall(CI, ROCCCNames::InductionVariableStepSize) or
      isROCCCFunctionCall(CI, ROCCCNames::InfiniteLoopCondition) or
      isROCCCFunctionCall(CI, ROCCCNames::NumberOfOutstandingMemoryRequests) or
      isROCCCFunctionCall(CI, ROCCCNames::NumberOfDataChannels) or
      isROCCCFunctionCall(CI, ROCCCNames::NumberOfAddressChannels)
  )
  {
    return false;
  }
  if( dynamic_cast<TerminatorInst*>(II) )
    return false;
  if( dynamic_cast<LoadInst*>(II) )
    return false;
  if( dynamic_cast<PHINode*>(II) )
    return false;
  return true;
}

bool shouldDisplayBlock(BasicBlock* BB)
{
  llvm::CallInst* CI = dynamic_cast<llvm::CallInst*>(BB->getFirstNonPHI());
  if( isROCCCFunctionCall(CI, ROCCCNames::SystolicNext) or
      isROCCCFunctionCall(CI, ROCCCNames::SystolicPrevious) )
    return true;
  if( BB->getDFBasicBlock()->getPipelineLevel() < 0 )
    return false;
  if( dynamic_cast<UnreachableInst*>(BB->getTerminator()) )
    return false;
  if( dynamic_cast<ReturnInst*>(BB->getTerminator()) )
    return false;
  return true;
}

std::string formatInstructionForDot(Instruction* II)
{
  if( dynamic_cast<TerminatorInst*>(&*II) )
    return "TERM";
  else
  {
    std::stringstream tempDump;
    tempDump << *II;
    std::string tempDumpRep = tempDump.str();
    std::string searchString = "\n";
    std::string replaceString = "\\l";
    std::string::size_type pos = 0;
    while ( (pos = tempDumpRep.find(searchString, pos)) != std::string::npos ) {
        tempDumpRep.replace( pos, searchString.size(), replaceString );
        ++pos;
    }
    searchString = ",";
    pos = 0;
    while ( (pos = tempDumpRep.find(searchString, pos)) != std::string::npos ) {
        tempDumpRep.replace( pos, searchString.size(), replaceString );
        ++pos;
    }
    return tempDumpRep;
  }
}

class ConstantNode : public ROCCCGraph::Node {
  std::string body_text;
protected:
  std::string getShape(){return "box";}
public:
  ConstantNode(std::string name, std::string text, ROCCCGraph::Subgraph* p) : Node(name,p), body_text(text){}
  virtual std::string getBodyText()
  {
    return body_text;
  }
  virtual void updateConnections(){}
  std::string getColor()
  {
    for(ROCCCGraph::Connectible::iterator OGI = this->outgoingBegin(); OGI != this->outgoingEnd(); ++OGI)
    {
      return (*OGI)->getSink()->getColor();
    }
    return "yellow";
  }
  Connectible* getConnectionPoint(llvm::Value* v)
  {
    return this;
  }
};

ROCCCGraph::Node* getDefinitionNodeOfOperandN(llvm::Instruction* inst, int n, ROCCCGraph::Graph* g)
{
  assert(inst);
  llvm::Value* op_v = inst->getOperand(n);
  llvm::Instruction* op_i = dynamic_cast<llvm::Instruction*>(op_v);
  ROCCCGraph::Node* op_n = NULL;
  if( op_i )
  {
    op_i = getDefinitionInstruction(op_i, inst->getParent());
    int pipeline_depth = op_i->getParent()->getDFBasicBlock()->getPipelineLevel() + op_i->getParent()->getDFBasicBlock()->getDelay() - 1;
    if( g->elementExists(pipeline_depth) )
    {
      ROCCCGraph::Subgraph* sg = g->getElement(pipeline_depth);
      if( sg->elementExists(op_i) )
      {
        op_n = sg->getElement(op_i);
      }
    }
  }
  else if (dynamic_cast<Constant*>(op_v) != NULL)
  {
    //we need to create a constant node!
    ConstantInt* ci = dynamic_cast<ConstantInt*>(op_v) ;
    ConstantFP* cf = dynamic_cast<ConstantFP*>(op_v) ;
    std::stringstream node_value;
    if (ci != NULL)
    {
      node_value << static_cast<int>(ci->getValue().getSExtValue());
    }
    else if (cf != NULL)
    {
      if( APFloat::semanticsPrecision(cf->getValueAPF().getSemantics()) < 32 )
      {
        node_value << cf->getValueAPF().convertToFloat();
      }
      else
        node_value << (float)cf->getValueAPF().convertToDouble();
    }
    else
    {
      return NULL;
    }
    int pipeline_depth = inst->getParent()->getDFBasicBlock()->getPipelineLevel() + inst->getParent()->getDFBasicBlock()->getDelay() - 1;
    if( g->elementExists(pipeline_depth) )
    {
      ROCCCGraph::Subgraph* sg = g->getElement(pipeline_depth);
      ROCCCGraph::Node* node = new ConstantNode(ROCCCGraph::generateTempNodeName(), node_value.str(), sg);
      sg->setElement(new llvm::UnreachableInst(), node);
      return node;
    }
  }
  return op_n;
}

class BoolSelectNode : public ROCCCGraph::Node {
  llvm::Instruction* inst;
  std::string color;
protected:
  std::string getShape(){return "invtrapezium";}
  std::string getColor(){return color;}
public:
  BoolSelectNode(std::string name, llvm::Instruction* i, ROCCCGraph::Subgraph* p) : ROCCCGraph::Node(name,p), inst(i), color("black"){}
  virtual std::string getBodyText()
  {
    std::stringstream ss;
    ss << "\",width=0.4, height=1,orientation=90,label=\"";
    return ss.str();
  }
  ROCCCGraph::Connectible* getConnectionPoint(llvm::Value* v)
  {
    assert( v == inst );
    return this;
  }
  virtual void updateConnections()
  {
    ROCCCGraph::Graph* g = this->getParent()->getParent();
    color = g->generateNextColor();
    ROCCCGraph::Node* comp_n = getDefinitionNodeOfOperandN(inst, 3, g);
    ROCCCGraph::Node* true_n = getDefinitionNodeOfOperandN(inst, 1, g);
    ROCCCGraph::Node* false_n = getDefinitionNodeOfOperandN(inst, 2, g);
    if( comp_n )
    {
      comp_n->getConnectionPoint(inst->getOperand(3))->flowsInto(this->getConnectionPointStr("s"), "");
    }
    if( true_n )
    {
      true_n->getConnectionPoint(inst->getOperand(1))->flowsInto(this->getConnectionPointStr("nw"), "true");
    }
    if( false_n )
    {
      false_n->getConnectionPoint(inst->getOperand(2))->flowsInto(this->getConnectionPointStr("sw"), "false");
    }
  }
};

class CopyNode : public ROCCCGraph::Node {
  llvm::Instruction* inst;
  std::string color;
  std::string group;
protected:
  std::string getShape(){return "box";}
  std::string getColor()
  {
    if( color == "" )
    {
      for(Connectible::iterator CI = this->incomingBegin(); CI != this->incomingEnd(); ++CI)
      {
        color = (*CI)->getColor();
      }
      for(std::map<std::string,ROCCCGraph::ConnectionPoint*>::iterator CPI = this->connectionBegin(); CPI != this->connectionEnd(); ++CPI)
      {
        for(Connectible::iterator CI = CPI->second->incomingBegin(); CI != CPI->second->incomingEnd(); ++CI)
        {
          color = (*CI)->getColor();
        }
      }
    }
    return color;
  }
public:
  CopyNode(std::string name, llvm::Instruction* i, ROCCCGraph::Subgraph* p) : ROCCCGraph::Node(name,p), inst(i), color(""), group(""){}
  virtual std::string getBodyText()
  {
    std::stringstream ss;
    ss << "";
    //ss << inst->getName() << " = " << inst->getOperand(0)->getName();
    //ss << 
    return ss.str();
  }
  ROCCCGraph::Connectible* getConnectionPoint(llvm::Value* v)
  {
    assert( v == inst );
    return this;
  }
  virtual void updateConnections()
  {
    ROCCCGraph::Graph* g = this->getParent()->getParent();
    ROCCCGraph::Node* copy_n = getDefinitionNodeOfOperandN(inst, 0, g);
    if( copy_n )
    {
      copy_n->getConnectionPoint(inst->getOperand(0))->flowsInto(this->getConnectionPointStr("w"), "");
    }
  }
  virtual std::string getGroup()
  {
    if( group == "" )
    {
      for(Connectible::iterator CI = this->incomingBegin(); CI != this->incomingEnd(); ++CI)
      {
        group = (*CI)->getSource()->getGroup();
      }
      for(std::map<std::string,ROCCCGraph::ConnectionPoint*>::iterator CPI = this->connectionBegin(); CPI != this->connectionEnd(); ++CPI)
      {
        for(Connectible::iterator CI = CPI->second->incomingBegin(); CI != CPI->second->incomingEnd(); ++CI)
        {
          group = (*CI)->getSource()->getGroup();
        }
      }
    }
    return group;
  }
};

class BinaryOpNode : public ROCCCGraph::Node {
  llvm::Instruction* inst;
  std::string color;
protected:
  std::string getShape(){return "circle";}
  std::string getColor(){return color;}
public:
  BinaryOpNode(std::string name, llvm::Instruction* i, ROCCCGraph::Subgraph* p) : ROCCCGraph::Node(name,p), inst(i), color("black"){}
  virtual std::string getBodyText()
  {
    std::stringstream ss;
    BinaryOperator* binary = dynamic_cast<BinaryOperator*>(inst);
    assert(binary);
    switch(binary->getOpcode())
    {
      case BinaryOperator::Add:
      {
   	    ss << "+";
   	    break;
      }
      case BinaryOperator::Sub:
      {
    	  ss << "-";
   	    break;
      }
      case BinaryOperator::Mul:
      {
        ss << "*";
   	    break;
      }
      case BinaryOperator::Shl:  // Shift left
      {
        ss << "&lt;&lt;";
   	    break;
      }
      case BinaryOperator::LShr: // Shift right logical
      case BinaryOperator::AShr: // Shift right arithmetic
      {
        ss << "&gt;&gt;";
   	    break;
      }
      case BinaryOperator::And:
      {
        ss << "and";
   	    break;
      }
      case BinaryOperator::Or:
      {
        ss << "or";
   	    break;
      }
      case BinaryOperator::Xor:
      {
        ss << "xor";
   	    break;
      }
      default:
      {
        INTERNAL_ERROR("Unknown binary operation " << *binary);
        assert( 0 and "Unknown binary operation!" );
      }
    }
    return ss.str();
  }  
  ROCCCGraph::Connectible* getConnectionPoint(llvm::Value* v)
  {
    assert( v == inst );
    return this;
  }
  virtual void updateConnections()
  {
    ROCCCGraph::Graph* g = this->getParent()->getParent();
    color = g->generateNextColor();
    ROCCCGraph::Node* op0_n = getDefinitionNodeOfOperandN(inst, 0, g);
    ROCCCGraph::Node* op1_n = getDefinitionNodeOfOperandN(inst, 1, g);
    if( op0_n )
    {
      op0_n->getConnectionPoint(inst->getOperand(0))->flowsInto(this->getConnectionPointStr("nw"), "");
    }
    if( op1_n )
    {
      op1_n->getConnectionPoint(inst->getOperand(1))->flowsInto(this->getConnectionPointStr("sw"), "");
    }
  }
};

class ComparisonNode : public ROCCCGraph::Node {
  llvm::Instruction* inst;
  std::string color;
protected:
  std::string getShape(){return "invtriangle";}
  std::string getColor(){return color;}
public:
  ComparisonNode(std::string name, llvm::Instruction* i, ROCCCGraph::Subgraph* p) : ROCCCGraph::Node(name,p), inst(i), color("black"){}
  virtual std::string getBodyText()
  {
    std::stringstream ss;
    ss << "\",orientation=90,label=\"";
    CmpInst* comp = dynamic_cast<CmpInst*>(inst);
    assert(comp);
    switch (comp->getPredicate())
    {
      case ICmpInst::ICMP_EQ: // equal
      {
        ss << "==";
        break;
      }
      case ICmpInst::ICMP_NE: // Not equal
      {
        ss << "!=";
        break;
      }
      case ICmpInst::ICMP_SGT: // Greater than (signed)
      case ICmpInst::ICMP_UGT: // Greater than (unsigned)
      {
        ss << "&gt;";
        break;
      }
      case ICmpInst::ICMP_SGE: // Greater than or equal (signed)
      case ICmpInst::ICMP_UGE: // Greater than or equal (unsigned)
      {
        ss << "gte";//">=";
        break;
      }
      case ICmpInst::ICMP_SLT: // Less than (signed)
      case ICmpInst::ICMP_ULT: // Less than (unsigned)
      {
        ss << "&lt;";
        break;
      }    
      case ICmpInst::ICMP_SLE: // Less than or equal (signed)
      case ICmpInst::ICMP_ULE: // Less than or equal (unsigned)
      {
        ss << "&lt;=";
        break;
      }
      default:
      {
        INTERNAL_ERROR("Unknown comparison " << *comp);
        assert( 0 and "Unknown operator!" );
      }
    }
    return ss.str();
  }
  ROCCCGraph::Connectible* getConnectionPoint(llvm::Value* v)
  {
    assert( v == inst );
    return this;
  }
  virtual void updateConnections()
  {
    ROCCCGraph::Graph* g = this->getParent()->getParent();
    color = g->generateNextColor();
    ROCCCGraph::Node* op0_n = getDefinitionNodeOfOperandN(inst, 0, g);
    ROCCCGraph::Node* op1_n = getDefinitionNodeOfOperandN(inst, 1, g);
    if( op0_n )
    {
      op0_n->getConnectionPoint(inst->getOperand(0))->flowsInto(this->getConnectionPointStr("nw"), "");
    }
    if( op1_n )
    {
      op1_n->getConnectionPoint(inst->getOperand(1))->flowsInto(this->getConnectionPointStr("sw"), "");
    }
  }
};

class OutputScalarCallNode : public ROCCCGraph::Node {
  llvm::Instruction* inst;
  class OutputScalarNode : public ROCCCGraph::Node {
    llvm::Instruction* inst;
  protected:
    std::string getShape(){return "box";}
    std::string getColor()
    {
      for(Connectible::iterator CI = this->incomingBegin(); CI != this->incomingEnd(); ++CI)
        return (*CI)->getColor();
      for(std::map<std::string,ROCCCGraph::ConnectionPoint*>::iterator CPI = this->connectionBegin(); CPI != this->connectionEnd(); ++CPI)
      {
        for(Connectible::iterator CI = CPI->second->incomingBegin(); CI != CPI->second->incomingEnd(); ++CI)
        {
          return (*CI)->getColor();
        }
      }
      return "black";
    }
  public:
    OutputScalarNode(std::string name, llvm::Instruction* i, ROCCCGraph::Subgraph* p) : ROCCCGraph::Node(name,p), inst(i){}
    virtual std::string getBodyText()
    {
      return getValueName(inst);
    }
    ROCCCGraph::Connectible* getConnectionPoint(llvm::Value* v)
    {
      assert( v == inst );
      return this;
    }
    virtual void updateConnections(){}
  };
  std::map<llvm::Value*,Node*> node_map;
protected:
  std::string getShape(){return "none";}
  std::string getColor(){return "yellow";}
public:
  OutputScalarCallNode(std::string name, llvm::Instruction* i, ROCCCGraph::Subgraph* p) : ROCCCGraph::Node(name,p), inst(i)
  {
    ROCCCGraph::Subgraph* sg = this->getParent();
    for(unsigned int OP = 1; OP != inst->getNumOperands(); ++OP)
    {
      llvm::Instruction* op_inst = dynamic_cast<llvm::Instruction*>(inst->getOperand(OP));
      if( op_inst ) //if we have an empty call list, llvm likes to put a constant 0 in there
      {
        ROCCCGraph::Node* node = new OutputScalarNode(ROCCCGraph::generateTempNodeName(), op_inst, sg);
        sg->setElement(new llvm::UnreachableInst(), node);
        node_map[op_inst] = node;
      }
    }
  }
  virtual std::string getBodyText()
  {
    return "";
  }
  ROCCCGraph::Connectible* getConnectionPoint(llvm::Value* v)
  {
    return this;
  }
  virtual void updateConnections()
  {
    ROCCCGraph::Graph* g = this->getParent()->getParent();
    for(unsigned int OP = 0; OP != inst->getNumOperands(); ++OP)
    {
      ROCCCGraph::Node* op_n = getDefinitionNodeOfOperandN(inst, OP, g);
      if( op_n and node_map[inst->getOperand(OP)] )
      {
        op_n->getConnectionPoint(inst->getOperand(OP))->flowsInto(node_map[inst->getOperand(OP)]);
      }
    }
  }
};

class InputScalarCallNode : public ROCCCGraph::Node {
  llvm::Instruction* inst;
  class InputScalarNode : public ROCCCGraph::Node {
    llvm::Instruction* inst;
    std::string color;
  protected:
    std::string getShape(){return "box";}
    std::string getColor(){return color;}
  public:
    InputScalarNode(std::string name, llvm::Instruction* i, ROCCCGraph::Subgraph* p) : ROCCCGraph::Node(name,p), inst(i), color("black"){}
    virtual std::string getBodyText()
    {
      return getValueName(inst);
    }
    ROCCCGraph::Connectible* getConnectionPoint(llvm::Value* v)
    {
      assert( v == inst );
      return this;
    }
    virtual void updateConnections()
    {
      ROCCCGraph::Graph* g = this->getParent()->getParent();
      color = g->generateNextColor();
    }
  };
protected:
  std::string getShape(){return "none";}
  std::string getColor(){return "yellow";}
public:
  InputScalarCallNode(std::string name, llvm::Instruction* i, ROCCCGraph::Subgraph* p) : ROCCCGraph::Node(name,p), inst(i)
  {
    ROCCCGraph::Subgraph* sg = this->getParent();
    for(unsigned int OP = 1; OP != inst->getNumOperands(); ++OP)
    {
      llvm::Instruction* op_inst = dynamic_cast<llvm::Instruction*>(inst->getOperand(OP));
      assert( op_inst );
      ROCCCGraph::Node* node = new InputScalarNode(ROCCCGraph::generateTempNodeName(), op_inst, sg);
      sg->setElement(op_inst, node);
    }
  }
  virtual std::string getBodyText()
  {
    return "";
  }
  ROCCCGraph::Connectible* getConnectionPoint(llvm::Value* v)
  {
    ROCCCGraph::Subgraph* sg = this->getParent();
    llvm::Instruction* i = dynamic_cast<llvm::Instruction*>(v);
    if( i and sg->elementExists(i) )
    {
      return sg->getElement(i);
    }
    return this;
  }
  virtual void updateConnections()
  {
  }
};

class OutputStreamCallNode : public ROCCCGraph::Node {
  llvm::Instruction* inst;
  class OutputStreamNode : public ROCCCGraph::TextNode {
    llvm::Instruction* inst;
  protected:
    std::string getShape(){return "egg";}
    std::string getColor()
    {
      for(Connectible::iterator CI = this->incomingBegin(); CI != this->incomingEnd(); ++CI)
        return (*CI)->getColor();
      for(std::map<std::string,ROCCCGraph::ConnectionPoint*>::iterator CPI = this->connectionBegin(); CPI != this->connectionEnd(); ++CPI)
      {
        for(Connectible::iterator CI = CPI->second->incomingBegin(); CI != CPI->second->incomingEnd(); ++CI)
        {
          return (*CI)->getColor();
        }
      }
      return "black";
    }
  public:
    OutputStreamNode(std::string name, std::string body_text, llvm::Instruction* i, ROCCCGraph::Subgraph* p) : ROCCCGraph::TextNode(name,body_text,p), inst(i){}
    ROCCCGraph::Connectible* getConnectionPoint(llvm::Value* v)
    {
      assert( v == inst );
      return this;
    }
    virtual void updateConnections(){}
  };
  std::map<llvm::Value*,Node*> node_map;
protected:
  std::string getShape(){return "none";}
  std::string getColor(){return "yellow";}
public:
  OutputStreamCallNode(std::string name, llvm::Instruction* i, ROCCCGraph::Subgraph* p) : ROCCCGraph::Node(name,p), inst(i)
  {
    ROCCCGraph::Subgraph* sg = this->getParent();
    unsigned int dim = getROCCCStreamDimension(dynamic_cast<CallInst*>(inst));
    for (unsigned int OP = dim+2; OP + dim < inst->getNumOperands(); OP+=dim+1)
    {
      llvm::Instruction* op_inst = dynamic_cast<llvm::Instruction*>(inst->getOperand(OP));
      assert( op_inst );
      std::stringstream name;
      name << getValueName(inst->getOperand(1));
      for(unsigned int d = 1; d <= dim; ++d)
      {
        ConstantInt* conInt = dynamic_cast<ConstantInt*>((inst->getOperand(OP+d)));
        assert(conInt);
        int index = conInt->getSExtValue();
        name << "[" << index << "]";
      }
      ROCCCGraph::Node* node = new OutputStreamNode(ROCCCGraph::generateTempNodeName(), name.str(), op_inst, sg);
      sg->setElement(op_inst, node);
      //sg->setElement(new llvm::UnreachableInst(), node);
      node_map[op_inst] = node;
    }
  }
  virtual std::string getBodyText()
  {
    return "";
  }
  ROCCCGraph::Connectible* getConnectionPoint(llvm::Value* v)
  {
    return this;
  }
  virtual void updateConnections()
  {
    ROCCCGraph::Graph* g = this->getParent()->getParent();
    unsigned int dim = getROCCCStreamDimension(dynamic_cast<CallInst*>(inst));
    for (unsigned int OP = dim+2; OP + dim < inst->getNumOperands(); OP+=dim+1)
    {
      ROCCCGraph::Node* op_n = getDefinitionNodeOfOperandN(inst, OP, g);
      if( op_n )
      {
        op_n->getConnectionPoint(inst->getOperand(OP))->flowsInto(node_map[inst->getOperand(OP)]);
      }
    }
  }
};

class InputStreamCallNode : public ROCCCGraph::Node {
  llvm::Instruction* inst;
  class InputStreamNode : public ROCCCGraph::TextNode {
    llvm::Instruction* inst;
    std::string color;
  protected:
    std::string getShape(){return "egg";}
    std::string getColor(){return color;}
  public:
    InputStreamNode(std::string name, std::string body_text, llvm::Instruction* i, ROCCCGraph::Subgraph* p) : ROCCCGraph::TextNode(name, body_text, p), inst(i), color("black"){}
    ROCCCGraph::Connectible* getConnectionPoint(llvm::Value* v)
    {
      assert( v == inst );
      return this;
    }
    virtual void updateConnections()
    {
      ROCCCGraph::Graph* g = this->getParent()->getParent();
      color = g->generateNextColor();
    }
  };
protected:
  std::string getShape(){return "none";}
  std::string getColor(){return "yellow";}
public:
  InputStreamCallNode(std::string name, llvm::Instruction* i, ROCCCGraph::Subgraph* p) : ROCCCGraph::Node(name,p), inst(i)
  {
    ROCCCGraph::Subgraph* sg = this->getParent();
    unsigned int dim = getROCCCStreamDimension(dynamic_cast<CallInst*>(inst));
    for (unsigned int OP = dim+2; OP + dim < inst->getNumOperands(); OP+=dim+1)
    {
      llvm::Instruction* op_inst = dynamic_cast<llvm::Instruction*>(inst->getOperand(OP));
      assert( op_inst );
      std::stringstream name;
      name << getValueName(inst->getOperand(1));
      for(unsigned int d = 1; d <= dim; ++d)
      {
        ConstantInt* conInt = dynamic_cast<ConstantInt*>((inst->getOperand(OP+d)));
        assert(conInt);
        int index = conInt->getSExtValue();
        name << "[" << index << "]";
      }
      ROCCCGraph::Node* node = new InputStreamNode(ROCCCGraph::generateTempNodeName(), name.str(), op_inst, sg);
      sg->setElement(op_inst, node);
    }
  }
  virtual std::string getBodyText()
  {
    return "";
  }
  ROCCCGraph::Connectible* getConnectionPoint(llvm::Value* v)
  {
    ROCCCGraph::Subgraph* sg = this->getParent();
    llvm::Instruction* i = dynamic_cast<llvm::Instruction*>(v);
    if( i and sg->elementExists(i) )
    {
      return sg->getElement(i);
    }
    return this;
  }
  virtual void updateConnections()
  {
  }
};

class InvokeHardwareNode : public ROCCCGraph::HTMLNode {
  llvm::Instruction* inst;
  std::string color;
  std::map<int,std::string> port_name_map;
  class InvokeHardwareNodeOutput : public ROCCCGraph::HTMLNode {
    llvm::Instruction* inst;
    InvokeHardwareNode* head;
  protected:
    std::string getShape(){return "component";}
    std::string getColor(){return head->getColor();}
  public:
    InvokeHardwareNodeOutput(std::string name, llvm::Instruction* i, ROCCCGraph::Subgraph* p, InvokeHardwareNode* h) : ROCCCGraph::HTMLNode(name,p), inst(i), head(h){}
    virtual std::string getBodyText(){return "";}
    virtual ROCCCGraph::Connectible* getConnectionPoint(llvm::Value* v){assert(0);}
    virtual void updateConnections()
    {
      head->flowsInto(this);
    }
    std::string getGroup(){return head->getGroup();}
  };
  InvokeHardwareNodeOutput* tail;
protected:
  std::string getShape(){return "component";}
  std::string getColor(){return color;}
public:
  InvokeHardwareNode(std::string name, llvm::Instruction* i, ROCCCGraph::Subgraph* p) : ROCCCGraph::HTMLNode(name,p), inst(i), color("black"), tail(NULL)
  {
    int tail_pipelineLevel = i->getParent()->getDFBasicBlock()->getPipelineLevel();
    ROCCCGraph::Subgraph* tail_subgraph = NULL;
    if( !p->getParent()->elementExists(tail_pipelineLevel) )
    {
      std::stringstream name;
      name << "pipeline_" << tail_pipelineLevel;
      tail_subgraph = new ROCCCGraph::Subgraph(name.str(), p->getParent());
      p->getParent()->setElement(tail_pipelineLevel, tail_subgraph);
    }
    else
      tail_subgraph = p->getParent()->getElement(tail_pipelineLevel);
    tail = new InvokeHardwareNodeOutput(name+"_output", i, tail_subgraph, this);
    tail_subgraph->setElement(new llvm::UnreachableInst(), tail);
    LibraryEntry entry = DatabaseInterface::getInstance()->LookupEntry(getComponentNameFromCallInst(dynamic_cast<llvm::CallInst*>(inst))) ;
    std::list<Port*> ports = entry.getNonDebugScalarPorts();
    std::list<Port*>::iterator PI = ports.begin();
    for(unsigned int OP = 2; OP != inst->getNumOperands(); ++OP)
    {
      if( (*PI)->getReadableName() != "" )
      {
        port_name_map[OP] = (*PI)->getReadableName();
      }
      else
      {
        port_name_map[OP] = (*PI)->getName();
      }
      ++PI;
    }
  }
  virtual std::string getBodyText()
  {
    std::stringstream ss;
    std::vector<int> inputs, outputs;
    for(unsigned int OP = 2; OP != inst->getNumOperands(); ++OP)
    {
      if( isDefinition(inst, inst->getOperand(OP)) )
      {
        outputs.push_back(OP);
      }
      else
        inputs.push_back(OP);
    }
    std::vector<int>::iterator inputI = inputs.begin(), outputI = outputs.begin();
    ss << "<TABLE BORDER=\"0\" CELLBORDER=\"0\" CELLSPACING=\"00\">\n";
    ss << "<TR><TD colspan=\"2\"> " << getComponentNameFromCallInst(dynamic_cast<llvm::CallInst*>(inst)) << "()" << "</TD></TR>\n";
    while( inputI != inputs.end() or outputI != outputs.end() )
    {
      ss << "<TR>";
      if( inputI != inputs.end() )
      {
        ss << "<TD PORT=\"" << port_name_map.at(*inputI) << "\" BORDER=\"1\">" << port_name_map.at(*inputI) << "</TD>";
        ++inputI;
      }
      else
        ss << "<TD></TD>";
      //ss << "<TD></TD>";
      if( outputI != outputs.end() )
      {
        ss << "<TD PORT=\"" << port_name_map.at(*outputI) << "\" BORDER=\"1\">" << port_name_map.at(*outputI) << "</TD>";
        ++outputI;
      }
      else
        ss << "<TD></TD>";
      ss << "</TR>\n";
    }
    ss << "</TABLE>";
    return ss.str();
  }
  virtual ROCCCGraph::Connectible* getConnectionPoint(llvm::Value* v)
  {
    for(unsigned int OP = 2; OP != inst->getNumOperands(); ++OP)
    {
      if( inst->getOperand(OP) == v )
        return this->getConnectionPointStr(port_name_map[OP]+":e");
    }
    assert(0);
  }
  virtual void updateConnections()
  {
    ROCCCGraph::Graph* g = this->getParent()->getParent();
    color = g->generateNextColor();
    for(unsigned int OP = 2; OP != inst->getNumOperands(); ++OP)
    {
      ROCCCGraph::Node* op_n = getDefinitionNodeOfOperandN(inst, OP, g);
      if( op_n and op_n != this )
      {
        op_n->getConnectionPoint(inst->getOperand(OP))->flowsInto(this->getConnectionPointStr(port_name_map[OP]+":w"));
      }
    }
  }
};

int getPipelineLevelForBlock(llvm::BasicBlock* BB)
{
  int pipelineLevel = BB->getDFBasicBlock()->getPipelineLevel() + BB->getDFBasicBlock()->getDelay() - 1;
  if( pipelineLevel < 0 )
  {
    for(llvm::BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
    {
      llvm::CallInst* CI = dynamic_cast<llvm::CallInst*>(&*II);
      if( isROCCCFunctionCall(CI, ROCCCNames::SystolicNext) or
          isROCCCFunctionCall(CI, ROCCCNames::StoreNext) )
      {
        return getDefinitionInstruction(dynamic_cast<llvm::Instruction*>(CI->getOperand(2)), CI->getParent())->getParent()->getDFBasicBlock()->getPipelineLevel() - 1;
      }
    }
  }
  if( pipelineLevel < 0 )
  {  
    INTERNAL_WARNING(*BB << " has negative pipeline depth!\n");
    pipelineLevel = 0;
  }
  return pipelineLevel;
}

class SystolicPreviousNode : public ROCCCGraph::Node {
  llvm::Instruction* inst;
protected:
  std::string getShape(){return "box";}
  std::string getColor()
  {
    for(Connectible::iterator CI = this->incomingBegin(); CI != this->incomingEnd(); ++CI)
      return (*CI)->getColor();
    for(std::map<std::string,ROCCCGraph::ConnectionPoint*>::iterator CPI = this->connectionBegin(); CPI != this->connectionEnd(); ++CPI)
    {
      for(Connectible::iterator CI = CPI->second->incomingBegin(); CI != CPI->second->incomingEnd(); ++CI)
      {
        return (*CI)->getColor();
      }
    }
    return "yellow";
  }
public:
  SystolicPreviousNode(std::string name, llvm::Instruction* i, ROCCCGraph::Subgraph* p) : ROCCCGraph::Node(name,p), inst(i) {}
  virtual std::string getBodyText()
  {
    std::stringstream ret;
    ret << getValueName(inst->getOperand(1));
    return ret.str();
  }
  ROCCCGraph::Connectible* getConnectionPoint(llvm::Value* v)
  {
    return this;
  }
  virtual void updateConnections()
  {
    ROCCCGraph::Graph* g = this->getParent()->getParent();
    llvm::Value* op_v = inst->getOperand(2);
    ROCCCGraph::Node* op_n = NULL;
    for(Value::use_iterator UI = op_v->use_begin(); UI != op_v->use_end(); ++UI)
    {
      llvm::CallInst* CI = dynamic_cast<llvm::CallInst*>(*UI);
      if( (isROCCCFunctionCall(CI, ROCCCNames::SystolicNext) or
           isROCCCFunctionCall(CI, ROCCCNames::StoreNext)) and
          CI->getOperand(1) == op_v )
      {
        int pipeline_depth = getPipelineLevelForBlock(CI->getParent());
        if( g->elementExists(pipeline_depth) )
        {
          ROCCCGraph::Subgraph* sg = g->getElement(pipeline_depth);
          if( sg->elementExists(CI) )
          {
            op_n = sg->getElement(CI);
          }
        }
      }
    }
    if( op_n and op_n != this )
    {
      op_n->getConnectionPoint(op_v)->flowsInto(this);
    }
  }
};

class SystolicNextNode : public ROCCCGraph::Node {
  llvm::Instruction* inst;
  std::string color;
protected:
  std::string getShape(){return "box";}
  std::string getColor()
  {
    for(Connectible::iterator CI = this->incomingBegin(); CI != this->incomingEnd(); ++CI)
      return (*CI)->getColor();
    for(std::map<std::string,ROCCCGraph::ConnectionPoint*>::iterator CPI = this->connectionBegin(); CPI != this->connectionEnd(); ++CPI)
    {
      for(Connectible::iterator CI = CPI->second->incomingBegin(); CI != CPI->second->incomingEnd(); ++CI)
      {
        return (*CI)->getColor();
      }
    }
    return "black";
  }
public:
  SystolicNextNode(std::string name, llvm::Instruction* i, ROCCCGraph::Subgraph* p) : ROCCCGraph::Node(name,p), inst(i), color("black"){}
  virtual std::string getBodyText()
  {
    std::stringstream ss;
    ss << "";
    return ss.str();
  }
  ROCCCGraph::Connectible* getConnectionPoint(llvm::Value* v)
  {
    assert( v == inst->getOperand(1) );
    return this;
  }
  virtual void updateConnections()
  {
    ROCCCGraph::Graph* g = this->getParent()->getParent();
    ROCCCGraph::Node* copy_n = getDefinitionNodeOfOperandN(inst, 2, g);
    if( copy_n )
    {
      copy_n->getConnectionPoint(inst->getOperand(2))->flowsInto(this, "");
    }
  }
};

ROCCCGraph::Node* getNodeFromInstruction(llvm::Instruction* II, ROCCCGraph::Subgraph* pipelineLevelGraph, ROCCCGraph::Graph* graph)
{
  llvm::CallInst* CI = dynamic_cast<llvm::CallInst*>(II);
  if( isROCCCFunctionCall(CI, ROCCCNames::OutputScalar) )
    return new OutputScalarCallNode(II->getName(), II, pipelineLevelGraph);
  if( isROCCCFunctionCall(CI, ROCCCNames::InputScalar) )
    return new InputScalarCallNode(II->getName(), II, pipelineLevelGraph);
  if( isROCCCFunctionCall(CI, ROCCCNames::BoolSelect) )
    return new BoolSelectNode(II->getName(), II, pipelineLevelGraph);
  if( isCopyValue(II) or dynamic_cast<BitCastInst*>(II) )
    return new CopyNode(II->getName(), II, pipelineLevelGraph);
  if( dynamic_cast<BinaryOperator*>(II) )
    return new BinaryOpNode(II->getName(), II, pipelineLevelGraph);
  if( dynamic_cast<CmpInst*>(II) )
    return new ComparisonNode(II->getName(), II, pipelineLevelGraph);
  if( isROCCCFunctionCall(CI, ROCCCNames::InvokeHardware) )
    return new InvokeHardwareNode(ROCCCGraph::generateTempNodeName(), II, pipelineLevelGraph);
  if( isROCCCInputStream(CI) )
    return new InputStreamCallNode(II->getName(), II, pipelineLevelGraph);
  if( isROCCCOutputStream(CI) )
    return new OutputStreamCallNode(II->getName(), II, pipelineLevelGraph);
  if( isROCCCFunctionCall(CI, ROCCCNames::SystolicPrevious) )
    return new SystolicPreviousNode(II->getName(), II, pipelineLevelGraph);
  if( isROCCCFunctionCall(CI, ROCCCNames::SystolicNext) )
    return new SystolicNextNode(II->getName(), II, pipelineLevelGraph);
  if( isROCCCFunctionCall(CI, ROCCCNames::LoadPrevious) )
    return new SystolicPreviousNode(II->getName(), II, pipelineLevelGraph);
  if( isROCCCFunctionCall(CI, ROCCCNames::StoreNext) )
    return new SystolicNextNode(II->getName(), II, pipelineLevelGraph);
  //we havent handled it yet . . . just do a default, UGHGLYLGLY node
  std::stringstream ss;
  ss << formatInstructionForDot(II);
  ROCCCGraph::Node* n = new ROCCCGraph::TextNode(ROCCCGraph::generateTempNodeName(), ss.str(), pipelineLevelGraph);
  for(succ_iterator SUI = succ_begin(II->getParent()); SUI != succ_end(II->getParent()); ++SUI)
  {
    int succ_pipelineLevel = (*SUI)->getDFBasicBlock()->getPipelineLevel() + (*SUI)->getDFBasicBlock()->getDelay() - 1;
    if( graph->elementExists(succ_pipelineLevel) )
    {
      ROCCCGraph::Subgraph* succ_subgraph = graph->getElement(succ_pipelineLevel);
      if( succ_subgraph->elementExists((*SUI)->getFirstNonPHI()) )
      {
        ROCCCGraph::Node* succ = succ_subgraph->getElement((*SUI)->getFirstNonPHI());
        n->getConnectionPointStr("e")->flowsInto(succ->getConnectionPointStr("w"));
      }
    }
  }
  for(pred_iterator PUI = pred_begin(II->getParent()); PUI != pred_end(II->getParent()); ++PUI)
  {
    int pred_pipelineLevel = (*PUI)->getDFBasicBlock()->getPipelineLevel() + (*PUI)->getDFBasicBlock()->getDelay() - 1;
    if( graph->elementExists(pred_pipelineLevel) )
    {
      ROCCCGraph::Subgraph* pred_subgraph = graph->getElement(pred_pipelineLevel);
      if( pred_subgraph->elementExists((*PUI)->getFirstNonPHI()) )
      {
        ROCCCGraph::Node* pred = pred_subgraph->getElement((*PUI)->getFirstNonPHI());
        pred->getConnectionPointStr("e")->flowsInto(n->getConnectionPointStr("w"));
      }
    }
  }
  return n;
}

struct PrintROCCCDatapathPass : public FunctionPass 
{
  static char ID;
  PrintROCCCDatapathPass() : FunctionPass((intptr_t)&ID) {}
  virtual bool runOnFunction(Function& f)
  {
    CurrentFile::set(__FILE__);
    bool changed = false;
    // Make sure this is a function that we can use
    if (f.isDeclaration() || !f.isDFFunction() )
    {
      return changed ;
    }
    std::string outputFileName ; 
    outputFileName = "cfg.";
    outputFileName += f.getName() ;
    outputFileName += ".dot" ;
    std::ofstream fout;
    fout.open(outputFileName.c_str(), std::ios_base::out) ;
    if (!fout)
    {
      llvm::cerr << "Cannot open output file:" << outputFileName << std::endl ;
      assert(0) ;
    }
    char buff[1024];
    if( !getcwd(buff, 1024) )
    {
      llvm::cout << "Could not get current directory!\n";
      assert(0);
      exit(0);
    }
    llvm::cout << "Writing datapath image to \'" << buff << "/" << outputFileName << "\'\n";
    Database::FileInfoInterface::addFileInfo(Database::getCurrentID(), Database::FileInfo(outputFileName, Database::FileInfo::DATAPATH_GRAPH, std::string(buff)+"/"));
    LOG_MESSAGE1("Datapath Graph", "<a href='" << outputFileName << ".png'>Datapath for " << f.getName() << ".</a>");
    ROCCCGraph::Graph graph("DFG for \'" + f.getName() + "\'.");
    for( Function::iterator BB = f.begin(); BB != f.end(); ++BB)
    {
      if( shouldDisplayBlock(BB) )
      {
        int pipelineLevel = getPipelineLevelForBlock(BB);
        ROCCCGraph::Subgraph* pipelineLevelGraph = NULL;
        if( graph.elementExists(pipelineLevel) )
          pipelineLevelGraph = graph.getElement(pipelineLevel);
        else
        {
          std::stringstream name;
          name << "pipeline_" << pipelineLevel;
          pipelineLevelGraph = new ROCCCGraph::Subgraph(name.str(), &graph);
          graph.setElement(pipelineLevel, pipelineLevelGraph);
        }
        for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
        {
          if( shouldDisplayInstruction(II) )
          {
            ROCCCGraph::Node* n = getNodeFromInstruction(&*II, pipelineLevelGraph, &graph);
            pipelineLevelGraph->setElement(&*II, n);
          }
        }
      }
    }
    fout << graph.print();
    fout.close() ;
    return changed;
  }
};
char PrintROCCCDatapathPass::ID = 0;
RegisterPass<PrintROCCCDatapathPass> X_PrintROCCCDatapathPass("printROCCCDFG", "Print ROCCC datapath.");

