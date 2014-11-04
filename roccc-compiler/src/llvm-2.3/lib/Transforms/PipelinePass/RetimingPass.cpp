// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

The retiming pass combines instructions into a single pipeline level,
allowing for a shallower pipeline at the potential cost of frequency. 
This is often a desirable optimization, as there are usually other constraints
(for example, the input controller) that limit the frequency, and so a
shallower pipeline can reduce area and latency. 
This is a renumbering algorithm, and does not actually combine multiple
instructions into a single BasicBlock.

The algorithm is based off of the FEAS algorithm presented in "Retiming
Synchronous Circuitry" by Charles E. Leiserson and James B. Saxe, page 16.

We first convert the ROCCC DFG into a more suitable, lightweight form to perform
the retiming calculations on. We construct a graph composed of Nodes and Edges,
with both Nodes and Edges having a weight associated with them,
that is representitive of the original ROCCC DFG. The weight of each Node is the
delay of that Node, given to us by the user, and the weight of each Edge is set
to 0. The retiming algorithm expects the weight of the edges to be the number of
registers at that Edge; by setting all Edges to 0, and by having an acyclic
graph, the algorithm inserts registers until the period of the graph is less than
or equal to the desired period.

Algorithm FEAS, as presented in "Retiming Synchronous Circuitry":
1) initialize the retiming r to 0 for each Node
2) repeat the following (number of Nodes) - 1 times:
    2.1) compute the retimed graph G_r with the retiming r
    2.2) determine the delay of each Node using algorithm CP on the graph G_r
    2.3) for each Node n, if the delay of n is > the desired delay,
           set r(n) = r(n) + 1
3) run CP on G_r. If clock period is > the desired delay, no retiming exists.
     Otherwise, r is the desired retiming.
     
Algorithm CP:
1) topologically sort graph G, ordering the Nodes so that if there is an
     Edge in G from Node u to Node v, then u precedes v in the total order.
2) go through the Nodes in the order defined by the topological sort.
3) compute the delay of each Node v as follows:
   3.1) if there is no incoming Edge to v, set the delay(v) = weight(v)
   3.2) otherwise, set delay(v) = weight(v) + max{delay(u) : Edge e in G, e=u->v, and weight(e)=0}
4) the delay of the graph G is the max {delay(v) : v in G}

Algorithm retime:
1) given a retiming r that maps Nodes to integers;
2) for each Edge e in G:
   2.1) define u,v such that e = u->v
   2.2) weight_new(e) = weight_old(e) + r(v) - r(u)
3) return weight_new

The topological sort in algorithm CP does not need to be done inside of the loop
body of FEAS, and can instead be done before entering step 2 of FEAS. This gives
a running time of O(number of Nodes * number of Edges) for algorithm CP, and a
running time of O(number of Nodes^2 * number of Edges) for algorithm FEAS.
*/

#include "llvm/Pass.h"
#include "llvm/Instructions.h"
#include "rocccLibrary/DFFunction.h"
#include "llvm/Support/CFG.h"
#include "llvm/Constants.h"

#include <list>
#include <algorithm>
#include <sstream>

#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/MessageLogger.h"
#include "rocccLibrary/ROCCCNames.h"
#include "TimingRequirements.h"

#include "rocccLibrary/PipelineBlocks.h"

namespace llvm
{
  class RetimingPass : public FunctionPass
  {
  private:
  public:
    static char ID ;
    RetimingPass() ;
    ~RetimingPass() ;
    virtual bool runOnFunction(Function& b) ;
    void getAnalysisUsage(AnalysisUsage &AU) const;
  } ;
}

using namespace llvm ;

char RetimingPass::ID = 0 ;

static RegisterPass<RetimingPass> X ("retime", 
					"Combine instructions into pipeline stages to minimize total number of pipeline stages.");

RetimingPass::RetimingPass() : FunctionPass((intptr_t)&ID) 
{
  ; // Nothing in here
}

RetimingPass::~RetimingPass()
{
  ; // Nothing to delete either
}

namespace RETIMING_LOCAL {

/*
Generic class to represent an edge between two Nodes of any type
*/
template< class N >
struct Edge {
  N* source;
  N* sink;
  int weight;
  Edge(N* src, N* snk, int w) : source(src), sink(snk), weight(w) {}
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
  int delay;
  T data;
  
  void addIncomingEdge(Edge<Node>* e){incomingEdge.push_back( e );}
  void addOutgoingEdge(Edge<Node>* e){outgoingEdge.push_back( e );}
public:
  Node(int del, T dat) : delay(del), data(dat) {}
  int getDelay(){return delay;}
  T getData(){return data;}
  Node& flowsInto(Node& rhs, int weight)
  {
    Edge<Node>* e = new Edge<Node>(this, &rhs, weight);
    addOutgoingEdge(e);
    rhs.addIncomingEdge(e);
    return rhs;
  }
  int numIncomingEdges(){return incomingEdge.size();}
  int numOutgoingEdges(){return outgoingEdge.size();}
  typename EdgeVec::iterator incomingBegin(){return incomingEdge.begin();}
  typename EdgeVec::iterator incomingEnd(){return incomingEdge.end();}
  typename EdgeVec::iterator outgoingBegin(){return outgoingEdge.begin();}
  typename EdgeVec::iterator outgoingEnd(){return outgoingEdge.end();}
};

/*
Retiming involves incrementally changing the weights of the edges. Instead
of acting upon the actual edge weights, we copy the weights into a map
and operate on that map. This function creates a map to initialize such
operations.
*/
template< class T >
std::map<Edge< Node<T> >*,int> getDefaultEdgeWeights(std::vector<Node<T>*> graph)
{
  std::map<Edge< Node<T> >*,int> weightMap;
  for(typename std::vector<Node<T>*>::iterator GI = graph.begin(); GI != graph.end(); ++GI)
  {
    for(typename Node<T>::EdgeVec::iterator OEI = (*GI)->outgoingBegin(); OEI != (*GI)->outgoingEnd(); ++OEI)
    {
      weightMap[*OEI] = (*OEI)->weight;
    }
  }
  return weightMap;
}

/*
A source node is one that has no incoming edges. This is included only for
completeness; it looks like only getSinkNodes are used in our implementation.
*/
template< class T >
std::list<Node<T>*> getSourceNodes(std::vector<Node<T>*> graph)
{
  std::list<Node<T>*> work_queue;
  for(typename std::vector<Node<T>*>::iterator GI = graph.begin(); GI != graph.end(); ++GI)
  {
    if( (*GI)->numIncomingEdges() == 0 )
      work_queue.push_back( *GI );
  }
  return work_queue;    
}

/*
A sink node is one that has no outgoing edges. These are important because
they are the start of our graph, which should always be acyclic.
This is more useful for a generic implementation; our implementation only
involves retiming the nodes that are reachable from the sink, and so our only
sink node should be the sink.
*/
template< class T >
std::list<Node<T>*> getSinkNodes(std::vector<Node<T>*> graph)
{
  std::list<Node<T>*> work_queue;
  for(typename std::vector<Node<T>*>::iterator GI = graph.begin(); GI != graph.end(); ++GI)
  {
    if( (*GI)->numOutgoingEdges() == 0 )
      work_queue.push_back( *GI );
  }
  return work_queue;    
}

/*
Implementation of algorithm CP.
Takes a topologically ordered graph, a map from Edges to weights, and returns
a mapping from Nodes to delays as integers.
*/
template< class T >
std::map<Node<T>*, int> computeNodeDelays(std::vector<Node<T>*> graph, std::map<Edge< Node<T> >*,int> edgeWeight)
{
  std::map<Node<T>*,int> delayMap;
  
  for(typename std::vector<Node<T>*>::iterator GI = graph.begin(); GI != graph.end(); ++GI )
  {
    Node<T>* toProcess = *GI;
    //if we have already processed this Node, dont process it again
    if( delayMap.find(toProcess) != delayMap.end() )
      continue;
    //the delay is equal to the delay of the Node itself,
    //plus the maximum incoming delay. If an edge is registered, then dont count
    //it in the max calculation.
    int maxIncomingDelay = 0;
    for(typename Node<T>::EdgeVec::iterator IEI = toProcess->incomingBegin(); IEI != toProcess->incomingEnd(); ++IEI)
    {
      assert( edgeWeight.find(*IEI) != edgeWeight.end() );
      //only consider those edges that arent registered
      if( edgeWeight[*IEI] != 0 )
        continue;
      //get the delay of the incoming value, not the raw delay but the calculated delay
      typename std::map<Node<T>*,int>::iterator DI = delayMap.find( (*IEI)->source );
      if( DI == delayMap.end() )
      {
        INTERNAL_WARNING( (*IEI)->source->getData()->getName() << " >> " << (*IEI)->sink->getData()->getName() << "\n" );
        INTERNAL_WARNING( "delayMap.size() == " << delayMap.size() << "\n" ); 
      }
      assert( DI != delayMap.end() );
      //find the max incoming delay
      if( DI->second > maxIncomingDelay )
        maxIncomingDelay = DI->second;
    }
    //delay is maxIn + currentDelay
    delayMap[toProcess] = maxIncomingDelay + toProcess->getDelay();
  }
  return delayMap;
}

/*
The delay of the entire graph is the max delay of the nodes in the graph.
This function takes a map from Nodes to delays, and returns the max.
*/
template< class T >
int getMaxNodeDelay(std::map<Node<T>*, int> nodeDelay)
{
  int maxDelay = 0;
  for(typename std::map<Node<T>*, int>::iterator NDI = nodeDelay.begin(); NDI != nodeDelay.end(); ++NDI)
  {
    if( NDI->second > maxDelay )
      maxDelay = NDI->second;
  }
  return maxDelay;
}

/*
Implementation of algorithm retime.
Given a graph G, an edge weighting w, and a retiming r, return an Edge weighting w_new such that:
w_new[e] = w[e] + r[v] - r[u]
for every e such that v and u are in G and e is the edge from u to v.
*/
template< class T >
std::map<Edge< Node<T> >*,int> getRetimedWeight(std::vector<Node<T>*> graph, std::map<Edge< Node<T> >*,int> edgeWeight, std::map<Node<T>*,int> retiming)
{
  std::map<Edge< Node<T> >*,int> weightMap;
  for(typename std::vector<Node<T>*>::iterator GI = graph.begin(); GI != graph.end(); ++GI)
  {
    for(typename Node<T>::EdgeVec::iterator OEI = (*GI)->outgoingBegin(); OEI != (*GI)->outgoingEnd(); ++OEI)
    {
      Node<T>* u = (*OEI)->source;
      Node<T>* v = (*OEI)->sink;
      assert(edgeWeight.find(*OEI) != edgeWeight.end() and retiming.find(u) != retiming.end() and retiming.find(v) != retiming.end());
      weightMap[*OEI] = edgeWeight[*OEI] + retiming[v] - retiming[u];
    }
  }
  return weightMap; 
}

/*
If FEAS succeeds, it returns the retiming, this is a 
structure that FEAS will return that encapsulates whether it
succeeded and, if so, what the retiming is.
*/
template< class T >
struct FEAS_ret {
  bool is_valid;
  std::map<Node<T>*,int> retiming;
};

/*
Strictly order the graph G.
If there is an Edge from Node u to Node v, u comes before v in the strict
ordering. This is only possible on acyclic graphs, so if we are planning on
running this on the entire DFG, it is necessary that we have assured there
are no loops with detectLoopsPass.
*/
template< class T >
std::vector<Node<T>*> orderGraph( std::vector<Node<T>*> graph )
{
  std::map<Node<T>*,bool> blockMap;
  std::list<Node<T>*> blockList;
  std::list<Node<T>*> work_queue = getSinkNodes(graph);
  assert( !work_queue.empty() and "Graph cannot be strictly ordered." );
  while( !work_queue.empty() )
  {
    Node<T>* toProcess = work_queue.front();
    work_queue.pop_front();
    blockMap[toProcess] = true;
    blockList.push_front( toProcess );
    for(typename Node<T>::EdgeVec::iterator IEI = toProcess->incomingBegin(); IEI != toProcess->incomingEnd(); ++IEI)
    {
      if( blockMap.find((*IEI)->source) != blockMap.end() )
        continue;
      bool shouldProcess = true;
      for(typename Node<T>::EdgeVec::iterator OEI = (*IEI)->source->outgoingBegin(); OEI != (*IEI)->source->outgoingEnd(); ++OEI)
      {
        if( blockMap.find((*OEI)->sink) == blockMap.end() )
        {
          shouldProcess = false;
        }
      }
      if( shouldProcess )
        work_queue.push_back( (*IEI)->source );
    }
  }
  std::vector<Node<T>*> ret;
  for(typename std::list<Node<T>*>::iterator BLI = blockList.begin(); BLI != blockList.end(); ++BLI)
  {
    ret.push_back( *BLI );
  }
  assert( ret.size() == graph.size() );
  return ret;
}

/*
algorithm FEAS
*/
template< class T >
FEAS_ret<T> feasibleClockPeriodTest(std::vector<Node<T>*> graph, int desiredDelay)
{
  graph = orderGraph( graph );
  assert( graph.size() >= 1 );
  std::map<Node<T>*,int> retiming;
  for(typename std::vector<Node<T>*>::iterator GI = graph.begin(); GI != graph.end(); ++GI)
  {
    retiming[*GI] = 0;
  }
  std::map<Edge< Node<T> >*,int> retimedWeight = getDefaultEdgeWeights(graph);
  int last_percent = 0, percent_done;
  for(unsigned int i = 0; i < graph.size() - 1; ++i)
  {
    retimedWeight = getRetimedWeight(graph, getDefaultEdgeWeights(graph), retiming);
    std::map<Node<T>*, int> nodeDelay = computeNodeDelays(graph, retimedWeight);
    bool has_changed = false;
    for(typename std::map<Node<T>*, int>::iterator NDI = nodeDelay.begin(); NDI != nodeDelay.end(); ++NDI)
    {
      if( NDI->second > desiredDelay )
      {
        ++retiming[NDI->first];
        has_changed = true;
      }
    }
    percent_done = 100 * (i+1) / (graph.size() - 1);
    if( last_percent != percent_done )
    {
      //LOG_MESSAGE2("Pipelining", "Pipelining", percent_done << "% done.\n");
      last_percent = percent_done;
    }
    if( !has_changed )
    {
      //LOG_MESSAGE2("Pipelining", "Pipelining", "Timing has not changed, reached stable state.");
      break;
    }
  }
  std::map<Node<T>*, int> nodeDelay = computeNodeDelays(graph, retimedWeight);
  FEAS_ret<T> ret;
  ret.is_valid = (getMaxNodeDelay(nodeDelay) <= desiredDelay);
  ret.retiming = retiming;
  return ret;
}

}
using namespace RETIMING_LOCAL;

bool RetimingPass::runOnFunction(Function& f)
{
  CurrentFile::set(__FILE__);
  bool changed = false ;
  if (f.isDeclaration() || f.getDFFunction() == NULL)
  {
    return changed ;
  }
  
  std::vector<Node<DFBasicBlock*>*> graph;
  //LOG_MESSAGE2("Pipelining", "Pipelining", "Starting pipelining.");
  
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
    int weight = Pipelining::TimingRequirements::getCurrentRequirements(&f)->getBasicBlockDelay(BB->first);

    Node<DFBasicBlock*>* node = new Node<DFBasicBlock*>(weight, BB->first);
    nodeMap[BB->first] = node;
    graph.push_back(node);
  }
  //for each edge in the DFG, we need to create an Edge in the graph
  for(std::map<DFBasicBlock*, bool>::iterator BB = pipeBlocks.begin(); BB != pipeBlocks.end(); ++BB)
  {
    if( BB->second == false )
      continue;
    for(pred_iterator pred = pred_begin(BB->first); pred != pred_end(BB->first); ++pred)
    {
      assert( (*pred)->getDFBasicBlock() );
      std::map<DFBasicBlock*,Node<DFBasicBlock*>*>::iterator predNode = nodeMap.find((*pred)->getDFBasicBlock());
      if( predNode != nodeMap.end() )
      {
        assert( nodeMap[BB->first] );
        int weight = 0;
        if( f.getDFFunction()->getSource() == predNode->first )
          weight = 1;
        predNode->second->flowsInto(*nodeMap[BB->first], weight);
      }
      else
      {
        INTERNAL_WARNING((*pred)->getName() << " was not found!\n");
      }
    }
  }
  //order the graph, perform retiming
  std::vector<Node<DFBasicBlock*>*> orderedGraph = orderGraph(graph);
  FEAS_ret<DFBasicBlock*> mpr = feasibleClockPeriodTest(graph, Pipelining::TimingRequirements::getCurrentRequirements(&f)->getDesiredDelay());
  assert(mpr.is_valid and "Retiming failed!");
  //the weight after applying retiming is whats important - it says what
  //edges should have registers on them
  std::map<Edge< Node<DFBasicBlock*> >*,int> retimedWeight = getRetimedWeight(graph, getDefaultEdgeWeights(graph), mpr.retiming);
  //go through the graph and renumber, using the max predecessor level
  for(std::vector<Node<DFBasicBlock*>*>::reverse_iterator OGI = orderedGraph.rbegin(); OGI != orderedGraph.rend(); ++OGI)
  {
    //Each edge will "suggest" a pipeline level for the source of that Edge -
    //the sink of that Edge's pipeline level + the Edge's weight + the sink of
    //that Edge's delay. We need to find the maximum "suggested" level, and go
    //with that value.
    int maxLevel = -1;
    for(Node<DFBasicBlock*>::EdgeVec::iterator OEI = (*OGI)->outgoingBegin(); OEI != (*OGI)->outgoingEnd(); ++OEI)
    {
      if(dynamic_cast<PHINode*>(&*(*OGI)->getData()->begin()))
      {
        retimedWeight[*OEI] = 1;
      }
      //if its a component call, we only want to insert registers if its more
      //  than our real delay
      int max_delay = 0;
      if( retimedWeight[*OEI] > (*OEI)->source->getData()->getDelay() - 1 )
      {
        max_delay = retimedWeight[*OEI];
      }
      else
      {
        if( (*OEI)->source->getData()->getDelay() == 1 )
          max_delay = 0;
        else
          max_delay = 1;//(*OEI)->source->getData()->getDelay() - 1;
      }
      int curLevel = (*OEI)->sink->getData()->getPipelineLevel() + (*OEI)->sink->getData()->getDelay() - 1 + max_delay;
      //int curLevel = (*OEI)->sink->getData()->getPipelineLevel() + (*OEI)->sink->getData()->getDelay() - 1 + retimedWeight[*OEI];
      if( curLevel  > maxLevel )
        maxLevel = curLevel;
    }
    //set our new level to the maxLevel previously found
    int newLevel = maxLevel;
    assert( (*OGI)->getData() );
    //make sure that the successors arent greater pipeline level (sanity check)
    //    for(succ_iterator succ = succ_begin((*OGI)->getData()); succ != succ_end((*OGI)->getData()); ++succ)
    //    {
    //      assert( newLevel >= succ->getDFBasicBlock()->getPipelineLevel() );
    //    }
    (*OGI)->getData()->setPipelineLevel( newLevel );
    (*OGI)->getData()->setDataflowLevel( newLevel );
    //make sure the sink stays at level 0
    f.getDFFunction()->getSink()->setPipelineLevel( 0 );
    f.getDFFunction()->getSink()->setDataflowLevel( 0 );
    //also, make sure that each ROCCCOutput, connected to the sink,
    //also stays at level 0
    for(pred_iterator pred = pred_begin(f.getDFFunction()->getSink()); pred != pred_end(f.getDFFunction()->getSink()); ++pred)
    {
      (*pred)->getDFBasicBlock()->setPipelineLevel(0);
      (*pred)->getDFBasicBlock()->setDataflowLevel(0);
    }
  }
  //finally, make sure that each source is set to one less than the max,
  //except for (BLEGH) systolicPrevious
  for(succ_iterator succ = succ_begin(f.getDFFunction()->getSource()); succ != succ_end(f.getDFFunction()->getSource()); ++succ)
  {
    int level = f.getDFFunction()->getSource()->getPipelineLevel() - 1;
    //renumber all of the predecessors, as well
    std::list<DFBasicBlock*> currentlyProcessing;
    currentlyProcessing.push_back((*succ)->getDFBasicBlock());
    while( !currentlyProcessing.empty() )
    {
      DFBasicBlock* currentElement = currentlyProcessing.front();
      currentlyProcessing.pop_front();
      //process current element
      if( level > currentElement->getPipelineLevel() )
      {
        currentElement->setPipelineLevel(level);
        currentElement->setDataflowLevel(level);
      }
      //add all predecessors that arent systolicPrevious
      for(pred_iterator pred = pred_begin(currentElement); pred != pred_end(currentElement); ++pred)
      {
        if( !isROCCCFunctionCall(dynamic_cast<CallInst*>((*pred)->getFirstNonPHI()), ROCCCNames::SystolicPrevious) )
        {
          currentlyProcessing.push_back((*pred)->getDFBasicBlock());
        }
      }
    }
  }
  //last step - go through and name each block with the pipeline stage #
  for(std::vector<Node<DFBasicBlock*>*>::reverse_iterator OGI = orderedGraph.rbegin(); OGI != orderedGraph.rend(); ++OGI)
  {
    std::stringstream ss;
    ss << (*OGI)->getData()->getName() << "_" << (*OGI)->getData()->getPipelineLevel();
    (*OGI)->getData()->setName(ss.str());
  }
  //set the functions overall delay
  int delay = f.getDFFunction()->getSource()->getPipelineLevel() - f.getDFFunction()->getSink()->getPipelineLevel() - 1;
  LOG_MESSAGE2("Pipelining", "Pipelining", "Pipelining finished. Pipeline depth changed from " << f.getDFFunction()->getDelay() << " to " << delay + 1 << ".\n");
  f.getDFFunction()->setDelay( delay );
  
  return changed ;
}

void RetimingPass::getAnalysisUsage(AnalysisUsage &AU) const
{
}
