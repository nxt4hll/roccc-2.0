#include "cfgraph_algs.h"
#include "sgraph_algs.h"
#include "sgraph/sgraph_bit_subgraph.h"

DFBuild::DFBuild(SGraph *graph, 
		 SGraphNode entry,
		 SGraphNode the_exit) :
  _initial_graph(graph),
  _reachable(NULL),
  _reverse_postorder_list(NULL),
  _dominators(NULL),
  _immediate_dominators(NULL),
  _dominance_frontier(NULL),
  _iterated_dominance_frontier(NULL),
  _entries(new SGraphNodeList),
  _exits(new SGraphNodeList)
{
  _entries->push_back(entry);
  _exits->push_back(the_exit);
}

DFBuild::~DFBuild(){
	delete _entries; delete _exits;

	if(_reachable) delete _reachable;
	if(_reverse_postorder_list) delete _reverse_postorder_list;
	if(_dominators) delete _dominators;
	if(_immediate_dominators) delete _immediate_dominators;
	if(_dominance_frontier) delete _dominance_frontier;
	if(_iterated_dominance_frontier) delete _iterated_dominance_frontier;
}

void DFBuild::do_build_reachable() {
  if (_reachable != NULL) return;

  SGraphBitSubgraph *reachable = new SGraphBitSubgraph(_initial_graph);

  for (SGraphNodeList::iterator iter = _entries->begin();
       iter != _entries->end(); iter++) {
    
    BitVector *bits = build_reachable(_initial_graph,
				      *iter,
				      true);
    reachable->add_nodes_from_bits(bits);
  }
  _reachable = reachable;
}



void DFBuild::do_build_dominance_frontier() {
  if (_dominance_frontier != NULL) return;
  do_build_reachable();
  do_build_dominators();
  do_build_immediate_dominators();

  SGraphBit *df = new SGraphBit;
  df->add_nodes_from_sgraph(_reachable);

  for (SGraphNodeList::iterator iter = _entries->begin();
       iter != _entries->end(); iter++) {
    build_dominance_frontiers(df,
			      _reachable,
			      _immediate_dominators,
			      *iter,
			      true);
  }
  _dominance_frontier = df;
}

void DFBuild::do_build_iterated_dominance_frontier() {
  if (_iterated_dominance_frontier != NULL) return;
  do_build_dominance_frontier();

  SGraphBit *idf = new SGraphBit;
  idf->add_nodes_from_sgraph(_reachable);

  build_iterated_dominance_frontiers(idf,
				     _reachable,
				     _dominance_frontier);
  _iterated_dominance_frontier = idf;
}

void DFBuild::do_build_reverse_postorder_list() {
  
  if (_reverse_postorder_list != NULL) return;
  do_build_reachable();

  //  slist_tos<SGraphNode> entries;

  //  for (SGraphNodeList::iterator iter = _entries->begin();
  //       iter != _entries->end(); iter++) {
  //    SGraphNode entry = *iter;
  //    entries.push_back(*iter);
  //  }

  _reverse_postorder_list = 
    build_reverse_postorder_list(_reachable,
				 _entries,
				 true);
}

void DFBuild::do_build_immediate_dominators() {
  if (_immediate_dominators != NULL) return;
  do_build_reachable();
  do_build_reverse_postorder_list();
  do_build_dominators();

  SGraphList *id = new SGraphList;
  id->add_nodes_from_sgraph(_reachable);

  for (SGraphNodeList::iterator iter = _entries->begin();
       iter != _entries->end(); iter++) {
    SGraphNode entry = *iter;
    build_immediate_dominators(id,
			       _reachable,
			       _dominators,
			       _reverse_postorder_list,
			       entry,
			       true);
    // add the idom of the entry is itself
    //    id->add_edge(entry,entry);
  }
  _immediate_dominators = id;
}

void DFBuild::do_build_dominators() {
  if (_dominators != NULL) return;
  do_build_reachable();
  do_build_reverse_postorder_list();

  SGraphBit *dom = new SGraphBit;
  dom->add_nodes_from_sgraph(_reachable);

  for (SGraphNodeList::iterator iter = _entries->begin();
       iter != _entries->end(); iter++) {
    build_dominators(dom,
		     _reachable,
		     _reverse_postorder_list,
		     *iter,
		     true);
  }
  _dominators = dom;
}

SGraph *DFBuild::get_initial_graph() const{
  return _initial_graph;
}

SGraph *DFBuild::get_reachable_graph() const{
  return _reachable;
}

SGraph *DFBuild::get_dominators() const{
  return _dominators;
}

SGraphNodeList *DFBuild::get_reverse_postorder_list() const {
  return _reverse_postorder_list;
}

SGraph *DFBuild::get_immediate_dominators() const {
  return _immediate_dominators;
}

SGraph *DFBuild::get_dominance_frontier() const {
  return _dominance_frontier;
}

SGraph *DFBuild::get_iterated_dominance_frontier() const {
  return _iterated_dominance_frontier;
}

