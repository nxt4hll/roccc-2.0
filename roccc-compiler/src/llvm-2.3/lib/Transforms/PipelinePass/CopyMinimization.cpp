#include <vector>
#include <map>
#include <string>

#include "llvm/Pass.h"
#include "llvm/Instructions.h"
#include "rocccLibrary/DFFunction.h"
#include "llvm/Support/CFG.h"
#include "llvm/Constants.h"

#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/MessageLogger.h"
#include "rocccLibrary/ROCCCNames.h"
#include "rocccLibrary/CopyValue.h"
#include "rocccLibrary/DefinitionInst.h"
#include "rocccLibrary/SizeInBits.h"
#include "rocccLibrary/GetValueName.h"

#include "AnalyzeCopyPass.h"
#include "rocccLibrary/PipelineBlocks.h"

namespace llvm
{
  class CopyMinimizationPass : public FunctionPass
  {
  private:
  public:
    static char ID ;
    CopyMinimizationPass() ;
    ~CopyMinimizationPass() ;
    virtual bool runOnFunction(Function& b) ;
    void getAnalysisUsage(AnalysisUsage &AU) const;
  } ;
}

using namespace llvm ;

char CopyMinimizationPass::ID = 0 ;

static RegisterPass<CopyMinimizationPass> X ("minimizeCopies", 
					"Rearrange operation timing so as to minimize total number of copies");

CopyMinimizationPass::CopyMinimizationPass() : FunctionPass((intptr_t)&ID) 
{
  ; // Nothing in here
}

CopyMinimizationPass::~CopyMinimizationPass()
{
  ; // Nothing to delete either
}
namespace COPY_MINIMIZATION_LOCAL {
/*
Generic class to represent an edge between two Nodes of any type
*/
template< class N >
struct Edge {
  N* source;
  N* sink;
  int bitwidth;
  int tripCount;
  Edge(N* src, N* snk, int w) : source(src), sink(snk), bitwidth(w), tripCount(1) {}
  float getWeight()const{return static_cast<float>(bitwidth) / static_cast<float>(tripCount);}
};

/*
Generic class that represents a Node in the retiming graph.
This node has a vector of incoming and outgoing edges, a delay,
and a corresponding data that this Node maps to.
*/
template< class T >
class Node {
public:
  typedef std::vector< Edge<Node>* > EdgeVec;
private:
  EdgeVec outgoingEdge;
  EdgeVec incomingEdge;
  T data;
  int position;
  int height;
  
  void addIncomingEdge(Edge<Node>* e){incomingEdge.push_back( e );}
  void addOutgoingEdge(Edge<Node>* e){outgoingEdge.push_back( e );}
public:
  Node(T dat, int p, int h) : data(dat), position(p), height(h) {}
  T getData()const{return data;}
  int getPosition()const{return position;}
  void setPosition(int p){if(p < 0) {INTERNAL_ERROR("Position cannot be negative!\nprevious position = " << position << "; new position = " << p << "\n");}assert(p >= 0);position = p;}
  int getHeight()const{return height;}
  int getTopPosition()const{return getPosition() + getHeight() - 1;}
  void setTopPosition(int p){setPosition(p - getHeight() + 1);}
  Node& flowsInto(Node& rhs, int weight)
  {
    assert(rhs.position <= this->position);
    Edge<Node>* e = new Edge<Node>(this, &rhs, weight);
    addOutgoingEdge(e);
    rhs.addIncomingEdge(e);
    return rhs;
  }
  int numIncomingEdges()const{return incomingEdge.size();}
  int numOutgoingEdges()const{return outgoingEdge.size();}
  typename EdgeVec::iterator incomingBegin(){return incomingEdge.begin();}
  typename EdgeVec::iterator incomingEnd(){return incomingEdge.end();}
  typename EdgeVec::iterator outgoingBegin(){return outgoingEdge.begin();}
  typename EdgeVec::iterator outgoingEnd(){return outgoingEdge.end();}
  typename EdgeVec::const_iterator incomingBegin()const{return incomingEdge.const_begin();}
  typename EdgeVec::const_iterator incomingEnd()const{return incomingEdge.const_end();}
  typename EdgeVec::const_iterator outgoingBegin()const{return outgoingEdge.const_begin();}
  typename EdgeVec::const_iterator outgoingEnd()const{return outgoingEdge.const_end();}
};

template< class T >
float getPositionedWeight(const Edge<T>* e)
{
  if( !e )
    return 0;
  if( e->sink->getTopPosition() > e->source->getPosition() )
  {
    INTERNAL_ERROR(*e->sink->getData() << "  top position (" << e->sink->getTopPosition() << ") is greater than" <<
                   *e->source->getData() << "  position (" << e->source->getPosition() << ", " << e->source->getData()->getPipelineLevel() << ")!\n");
  }
  assert( e->sink->getTopPosition() <= e->source->getPosition() );
  //subtract 1 because we are not counting registers at boundary edges, only copies
  int pos_diff = e->source->getPosition() - e->sink->getTopPosition() - 1;
  if( pos_diff < 0 )
    pos_diff = 0;
  return static_cast<float>(pos_diff) * e->getWeight();
}

template< class T >
std::map<Edge< Node<T> >*, bool> getEdges(const std::vector<Node<T>*>& graph)
{
  std::map<Edge< Node<T> >*, bool> ret;
  for(typename std::vector<Node<T>*>::const_iterator GI = graph.begin(); GI != graph.end(); ++GI)
  {
    for(typename Node<T>::EdgeVec::const_iterator EI = (*GI)->outgoingBegin(); EI != (*GI)->outgoingEnd(); ++EI)
    {
      ret[*EI] = true;
    }
  }
  return ret;
}

template< class T >
Edge< Node<T> >* selectEdgeWithLargestWeight(const std::vector<Node<T>*>& graph)
{
  std::map<Edge< Node<T> >*, bool> edges = getEdges(graph);
  Edge< Node<T> >* largest = NULL;
  for(typename std::map<Edge< Node<T> >*, bool>::const_iterator EI = edges.begin(); EI != edges.end(); ++EI)
  {
    if( getPositionedWeight(EI->first) > getPositionedWeight(largest) )
    {
      largest = EI->first;
    }
  }
  return largest;
}

template< class T >
//ROCCCInputs are pulled to the top of the DFG. These are all values that
//will be controlled by the inputController.
bool isROCCCInput( Node<T>* hold  )
{
  CallInst* CI = dynamic_cast<CallInst*>(hold->getData()->getFirstNonPHI());
  if (!CI)
    return false;
  return (isROCCCFunctionCall(CI, ROCCCNames::InputScalar) or
          isROCCCFunctionCall(CI, ROCCCNames::LoadPrevious) or
          isROCCCInputStream(CI));
}

template< class T >
//ROCCCOutputs are pulled to the bottom of the DFG.
bool isROCCCOutput( Node<T>* hold )
{
  CallInst* CI = dynamic_cast<CallInst*>(hold->getData()->getFirstNonPHI());
  if (!CI)
    return false;
  return (isROCCCFunctionCall(CI, ROCCCNames::OutputScalar) or 
          isROCCCOutputStream(CI) or
          isROCCCFunctionCall(CI, ROCCCNames::StoreNext) or
          isROCCCFunctionCall(CI, ROCCCNames::OutputInductionVariableEndValue) );
}

template< class T >
void tightenEdge(const Edge< Node<T> >* e)
{
  assert(e);
  int source_lowest = e->source->getPosition();
  for(typename Node<T>::EdgeVec::const_iterator SOI = e->source->outgoingBegin(); SOI != e->source->outgoingEnd(); ++SOI)
  {
    //getTopPosition() gives us the topmost pipeline level the operation fits in,
    //  so add 1 to find the lowest that an operation can be scheduled over that
    //  (we arent going to worry about moving things into the same pipeline
    //    level)
    if( SOI == e->source->outgoingBegin() )
      source_lowest = (*SOI)->sink->getTopPosition() + 1;
    else if( (*SOI)->sink->getTopPosition() + 1 > source_lowest )
      source_lowest = (*SOI)->sink->getTopPosition() + 1;
  }
  if( source_lowest > e->source->getPosition() )
    source_lowest = e->source->getPosition();
  int sink_highest = e->sink->getTopPosition();
  for(typename Node<T>::EdgeVec::const_iterator SII = e->sink->incomingBegin(); SII != e->sink->incomingEnd(); ++SII)
  {
    if( SII == e->sink->incomingBegin() )
      sink_highest = (*SII)->source->getPosition() - e->sink->getHeight();
    else if( (*SII)->source->getPosition() - e->sink->getHeight() < sink_highest )
      sink_highest = (*SII)->source->getPosition() - e->sink->getHeight();
  }
  if( sink_highest < e->sink->getTopPosition() )
    sink_highest = e->sink->getTopPosition();
  if( source_lowest <= sink_highest )
  {
    int median = (source_lowest + sink_highest) / 2;
    int total_into_source = 0;
    for(typename Node<T>::EdgeVec::const_iterator SII = e->source->incomingBegin(); SII != e->source->incomingEnd(); ++SII)
    {
      total_into_source += (*SII)->bitwidth;
    }
    int total_outof_sink = 0;
    for(typename Node<T>::EdgeVec::const_iterator SOI = e->sink->outgoingBegin(); SOI != e->sink->outgoingEnd(); ++SOI)
    {
      total_outof_sink += (*SOI)->bitwidth;
    }
    if( total_into_source > total_outof_sink )
    {
      assert( median + 1 >= source_lowest );
      assert( median + 1 <= e->source->getPosition() );
      assert( median <= sink_highest );
      assert( median >= e->sink->getTopPosition() );
      if( !isROCCCInput(e->source) )
      {
        //LOG_MESSAGE2("Pipelining", "Register Minimization", "Moved " << getValueName(e->source->getData()->getFirstNonPHI()) << " from pipeline level " << e->source->getPosition() << " to pipeline level " << median + 1 << ".\n");
        e->source->setPosition(median + 1);
      }
      if( !isROCCCOutput(e->sink) )
      {
        //LOG_MESSAGE2("Pipelining", "Register Minimization", "Moved " << getValueName(e->sink->getData()->getFirstNonPHI()) << " from pipeline level " << e->sink->getPosition() << " to pipeline level " << median << ".\n");
        e->sink->setTopPosition(median);
      }
      assert( e->sink->getTopPosition() <= e->source->getPosition() );
      for(typename Node<T>::EdgeVec::iterator EII = e->source->incomingBegin(); EII != e->source->incomingEnd(); ++EII)
      {
        assert( (*EII)->sink->getTopPosition() <= (*EII)->source->getPosition() );
      }
      for(typename Node<T>::EdgeVec::iterator EOI = e->source->outgoingBegin(); EOI != e->source->outgoingEnd(); ++EOI)
      {
        assert( (*EOI)->sink->getTopPosition() <= (*EOI)->source->getPosition() );
      }
      for(typename Node<T>::EdgeVec::iterator EII = e->sink->incomingBegin(); EII != e->sink->incomingEnd(); ++EII)
      {
        assert( (*EII)->sink->getTopPosition() <= (*EII)->source->getPosition() );
      }
      for(typename Node<T>::EdgeVec::iterator EOI = e->sink->outgoingBegin(); EOI != e->sink->outgoingEnd(); ++EOI)
      {
        assert( (*EOI)->sink->getTopPosition() <= (*EOI)->source->getPosition() );
      }
    }
    else
    {
      assert( median >= source_lowest );
      assert( median <= e->source->getPosition() );
      assert( median-1 <= sink_highest );
      assert( median-1 >= e->sink->getTopPosition() );
      if( !isROCCCInput(e->source) )
      {
        //LOG_MESSAGE2("Pipelining", "Register Minimization", "Moved " << getValueName(e->source->getData()->getFirstNonPHI()) << " from pipeline level " << e->source->getPosition() << " to pipeline level " << median << ".\n");
        e->source->setPosition(median);
      }
      if( !isROCCCOutput(e->sink) )
      {
        //LOG_MESSAGE2("Pipelining", "Register Minimization", "Moved " << getValueName(e->sink->getData()->getFirstNonPHI()) << " from pipeline level " << e->sink->getPosition() << " to pipeline level " << median-1 << ".\n");
        e->sink->setTopPosition(median-1);
      }
      assert( e->sink->getTopPosition() <= e->source->getPosition() );
      for(typename Node<T>::EdgeVec::iterator EII = e->source->incomingBegin(); EII != e->source->incomingEnd(); ++EII)
      {
        assert( (*EII)->sink->getTopPosition() <= (*EII)->source->getPosition() );
      }
      for(typename Node<T>::EdgeVec::iterator EOI = e->source->outgoingBegin(); EOI != e->source->outgoingEnd(); ++EOI)
      {
        assert( (*EOI)->sink->getTopPosition() <= (*EOI)->source->getPosition() );
      }
      for(typename Node<T>::EdgeVec::iterator EII = e->sink->incomingBegin(); EII != e->sink->incomingEnd(); ++EII)
      {
        assert( (*EII)->sink->getTopPosition() <= (*EII)->source->getPosition() );
      }
      for(typename Node<T>::EdgeVec::iterator EOI = e->sink->outgoingBegin(); EOI != e->sink->outgoingEnd(); ++EOI)
      {
        assert( (*EOI)->sink->getTopPosition() <= (*EOI)->source->getPosition() );
      }
    }
  }
  else
  {
    if( !isROCCCInput(e->source) )
    {
      //LOG_MESSAGE2("Pipelining", "Register Minimization", "Moved " << getValueName(e->source->getData()->getFirstNonPHI()) << " from pipeline level " << e->source->getPosition() << " to pipeline level " << source_lowest << ".\n");
      e->source->setPosition(source_lowest);
    }
    if( !isROCCCOutput(e->sink) )
    {
      //LOG_MESSAGE2("Pipelining", "Register Minimization", "Moved " << getValueName(e->sink->getData()->getFirstNonPHI()) << " from pipeline level " << e->sink->getPosition() << " to pipeline level " << sink_highest << ".\n");
      e->sink->setTopPosition(sink_highest);
    }
    assert( e->sink->getTopPosition() <= e->source->getPosition() );
    for(typename Node<T>::EdgeVec::iterator EII = e->source->incomingBegin(); EII != e->source->incomingEnd(); ++EII)
    {
      assert( (*EII)->sink->getTopPosition() <= (*EII)->source->getPosition() );
    }
    for(typename Node<T>::EdgeVec::iterator EOI = e->source->outgoingBegin(); EOI != e->source->outgoingEnd(); ++EOI)
    {
      assert( (*EOI)->sink->getTopPosition() <= (*EOI)->source->getPosition() );
    }
    for(typename Node<T>::EdgeVec::iterator EII = e->sink->incomingBegin(); EII != e->sink->incomingEnd(); ++EII)
    {
      assert( (*EII)->sink->getTopPosition() <= (*EII)->source->getPosition() );
    }
    for(typename Node<T>::EdgeVec::iterator EOI = e->sink->outgoingBegin(); EOI != e->sink->outgoingEnd(); ++EOI)
    {
      assert( (*EOI)->sink->getTopPosition() <= (*EOI)->source->getPosition() );
    }
  }
}

template< class T >
int getTotalCopiedBits(const std::vector<Node<T>*>& graph)
{
  int total = 0;
  for(typename std::vector<Node<T>*>::const_iterator GI = graph.begin(); GI != graph.end(); ++GI)
  {
    int max_out = 0;
    for(typename Node<T>::EdgeVec::const_iterator EI = (*GI)->outgoingBegin(); EI != (*GI)->outgoingEnd(); ++EI)
    {
      int position_diff = (*EI)->source->getPosition() - (*EI)->sink->getTopPosition() - 0;//1;
      if( position_diff < 0 )
        position_diff = 0;
      int cur_out = position_diff * (*EI)->bitwidth;
      if( cur_out > max_out )
        max_out = cur_out;
    }
    total += max_out;
  }
  return total;
}

template< class T >
std::vector<Node<T>*> createCopyOfGraph(const std::vector<Node<T>*>& graph)
{
  std::vector<Node<T>*> ret;
  std::map<Node<T>*, Node<T>*> originalToNew;
  for(typename std::vector<Node<T>*>::const_iterator NI = graph.begin(); NI != graph.end(); ++NI)
  {
    Node<T>* n = new Node<T>((*NI)->getData(), (*NI)->getPosition(), (*NI)->getHeight());
    originalToNew[*NI] = n;
    ret.push_back(n);
  }
  for(typename std::vector<Node<T>*>::const_iterator NI = graph.begin(); NI != graph.end(); ++NI)
  {
    for(typename Node<T>::EdgeVec::const_iterator EI = (*NI)->outgoingBegin(); EI != (*NI)->outgoingEnd(); ++EI)
    {
      originalToNew[(*EI)->source]->flowsInto(*originalToNew[(*EI)->sink], (*EI)->bitwidth);
    }
  }
  return ret;
}

template< class T >
int maxTripCount(const std::vector<Node<T>*>& graph)
{
  int maxTC = 0;
  for(typename std::vector<Node<T>*>::const_iterator NI = graph.begin(); NI != graph.end(); ++NI)
  {
    for(typename Node<T>::EdgeVec::const_iterator EI = (*NI)->outgoingBegin(); EI != (*NI)->outgoingEnd(); ++EI)
    {
      if( (*EI)->tripCount > maxTC )
        maxTC = (*EI)->tripCount;
    }
  }
  return maxTC;
}

template< class T >
void display( const std::vector<Node<T>*>& graph )
{
  for(typename std::vector<Node<T>*>::const_iterator GI = graph.begin(); GI != graph.end(); ++GI)
  {
    cout << *(*GI)->getData()/*->getName()*/ << "(" << (*GI)->getTopPosition() << "-" << (*GI)->getPosition() << ") -> {";
    for(typename Node<T>::EdgeVec::const_iterator EI = (*GI)->outgoingBegin(); EI != (*GI)->outgoingEnd(); ++EI)
    {
      if( EI != (*GI)->outgoingBegin() )
        cout << ", ";
      cout << (*EI)->sink->getData()->getName() << "(" << (*EI)->bitwidth << ", " << (*EI)->tripCount << ", " << getPositionedWeight(*EI) << ")";
    }
    cout << "}\n";
  }
  cout << "Total - " << getTotalCopiedBits(graph) << "\n";
  cout << "\n";
}

}
using namespace COPY_MINIMIZATION_LOCAL;

bool CopyMinimizationPass::runOnFunction(Function& f)
{
  CurrentFile::set(__FILE__);
  bool changed = false ;
  if (f.isDeclaration() || f.getDFFunction() == NULL)
  {
    return changed ;
  }
  
  std::vector<Node<DFBasicBlock*>*> graph;
  
  //We need to start with an acyclic graph to do retiming on -
  //the analyzeCopyPass conveniently provides a function that returns the 
  //blocks reachable by the sink. These are the same blocks that the
  //detectLoopsPass goes through, so they should be acyclical.
  std::map<DFBasicBlock*, bool> pipeBlocks = getPipelineBlocks(f);
  std::map<DFBasicBlock*,Node<DFBasicBlock*>*> nodeMap;
  //for each block in the DFG, create a Node for it.
  //this means we need to provide a weight for the block,
  //and we also want to save it in a map for later
  for(std::map<DFBasicBlock*, bool>::iterator BB = pipeBlocks.begin(); BB != pipeBlocks.end(); ++BB)
  {
    if( BB->second == false )
      continue;
    Node<DFBasicBlock*>* node = new Node<DFBasicBlock*>(BB->first, BB->first->getPipelineLevel(), BB->first->getDelay());
    nodeMap[BB->first] = node;
    graph.push_back(node);
  }
  //for each edge in the DFG, we need to create an Edge in the graph
  for(std::map<DFBasicBlock*, bool>::iterator BB = pipeBlocks.begin(); BB != pipeBlocks.end(); ++BB)
  {
    if( BB->second == false )
      continue;
    CallInst* CI = dynamic_cast<CallInst*>(BB->first->getFirstNonPHI());
    if( isROCCCFunctionCall(CI, ROCCCNames::LoadPrevious) )
      continue;
    for(pred_iterator pred = pred_begin(BB->first); pred != pred_end(BB->first); ++pred)
    {
      assert( (*pred)->getDFBasicBlock() );
      std::map<DFBasicBlock*,Node<DFBasicBlock*>*>::iterator predNode = nodeMap.find((*pred)->getDFBasicBlock());
      if( predNode != nodeMap.end() )
      {
        assert( nodeMap[BB->first] );
        int weight = 0;
        std::map<llvm::Value*,bool> valsUsed;
        for(BasicBlock::iterator II = BB->first->begin(); II != BB->first->end(); ++II)
        {
          for(User::op_iterator OP = II->op_begin(); OP != II->op_end(); ++OP)
          {
            valsUsed[*OP] = true;
          }
        }
        for(BasicBlock::iterator PII = (*pred)->begin(); PII != (*pred)->end(); ++PII)
        {
           for(std::map<llvm::Value*,bool>::iterator VUI = valsUsed.begin(); VUI != valsUsed.end(); ++VUI)
           {
             if( isDefinition(PII, VUI->first) )
             {
               weight += getSizeInBits(VUI->first);
             }
           }
        }
        predNode->second->flowsInto(*nodeMap[BB->first], weight);
      }
      else
      {
        INTERNAL_WARNING((*pred)->getName() << " was not found!\n");
      }
    }
  }
  int minimum_copies = getTotalCopiedBits(graph);
  int num_original_copied_bits = minimum_copies;
  std::vector< Node<DFBasicBlock*>* > min_graph = createCopyOfGraph(graph);
  int iterationCount = 0;
  int minIterationCount = 0;
  LOG_MESSAGE2("Pipelining", "Register Minimization", "Bit registers needed to pipeline original graph, including both copies and pipeline boundaries: " << minimum_copies << ".\n");
  while(maxTripCount(graph) < 10)
  {
    //cout << iterationCount << " - " << maxTripCount(graph) << "\n";
    ++iterationCount;
    //display(graph);
    Edge< Node<DFBasicBlock*> >* largest = selectEdgeWithLargestWeight(graph);
    if( largest )
    {
      //LOG_MESSAGE2("Pipelining", "Register Minimization", "Edge with most bit registers needed: " << largest->source->getData() << " -> " << largest->sink->getData() << "\n");
      tightenEdge(largest);
      largest->tripCount++;
    }
    else
    {
      LOG_MESSAGE2("Pipelining", "Register Minimization", "Completely reduced.\n");
      break;
    }
    //LOG_MESSAGE2("Pipelining", "Register Minimization", "Iteration " << iterationCount << " - total number of bits registers needed: " << getTotalCopiedBits(graph) << ".\n");
    if( getTotalCopiedBits(graph) < minimum_copies )
    {
      min_graph = createCopyOfGraph(graph);
      minimum_copies = getTotalCopiedBits(graph);
      ++minIterationCount;
    }
  }
  LOG_MESSAGE2("Pipelining", "Register Minimization", "Tested " << iterationCount << " total iterations of minimizing; minimum number of bit registers needed, " << minimum_copies << ", was found after " << minIterationCount << " iterations.\n");
  //display(min_graph);
  
  if( minimum_copies != num_original_copied_bits )
  {
    for(std::vector<Node<DFBasicBlock*>*>::const_iterator GI = graph.begin(); GI != graph.end(); ++GI)
    {
      int level = (*GI)->getPosition();
      (*GI)->getData()->setPipelineLevel(level);
      (*GI)->getData()->setDataflowLevel(level);
      std::stringstream ss;
      ss << (*GI)->getData()->getName() << "_" << (*GI)->getData()->getPipelineLevel();
      (*GI)->getData()->setName(ss.str());
    }
    changed = true;
  }
  
  return changed ;
}

void CopyMinimizationPass::getAnalysisUsage(AnalysisUsage &AU) const
{
  AU.addRequired<AnalyzeCopyPass>();
}
