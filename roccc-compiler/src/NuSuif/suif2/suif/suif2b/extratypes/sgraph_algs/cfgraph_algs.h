#ifndef CFGRAPH_ALGS
#define CFGRAPH_ALGS

#include "sgraph/sgraph.h"

class DFBuild {
  SGraph *_initial_graph;

  SGraph *_reachable;
  SGraphNodeList *_reverse_postorder_list;
  SGraph *_dominators;
  SGraph *_immediate_dominators;
  SGraph *_dominance_frontier;
  SGraph *_iterated_dominance_frontier;
  SGraphNodeList *_entries;
  SGraphNodeList *_exits;
 public:

  DFBuild(SGraph *graph, SGraphNode entry, SGraphNode the_exit);
  ~DFBuild();

  void do_build_reachable();
  void do_build_reverse_postorder_list();
  void do_build_dominators();
  void do_build_immediate_dominators();
  void do_build_dominance_frontier();
  void do_build_iterated_dominance_frontier();

  SGraph *get_initial_graph() const;
  SGraph *get_reachable_graph() const;
  SGraph *get_dominators() const;
  SGraphNodeList *get_reverse_postorder_list() const;
  SGraph *get_immediate_dominators() const;
  SGraph *get_dominance_frontier() const;
  SGraph *get_iterated_dominance_frontier() const;

  SGraphNode get_immed_dom(SGraphNode node) const;
  SGraphNode get_immed_postdom(SGraphNode node) const;
private:
    /* avoid default assignment ops, don't define these */
  DFBuild &operator=(const DFBuild&);
  DFBuild(const DFBuild&);
};

#endif /* CFGRAPH_ALGS */
