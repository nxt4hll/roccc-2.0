#ifndef SGRAPH_ITER_IMPL_H
#define SGRAPH_ITER_IMPL_H

#include "sgraph_iter.h"

/*
 * build an iterator over an edge list
 */
class SGraphNodeListIter : public SGraphNodeIter {
  SGraphNodeList * _list;
  bool _owned; // If owned, delete it afterwards
  SGraphNodeList::iterator _iter;
public:
  SGraphNodeListIter();
  SGraphNodeListIter(const SGraphNodeListIter &other);
  SGraphNodeListIter &operator=(const SGraphNodeListIter &other);
  SGraphNodeListIter(SGraphNodeList *list, bool owned);
  void reset(SGraphNodeList* list, bool owned);
  void reset();
  virtual bool done() const;
  virtual SGraphNode get() const;
  virtual void increment();
  virtual SGraphNode step();
  virtual SGraphNodeIter *clone() const;
};



/*
 * build an iterator over an edge list
 */
class SGraphEdgeListIter : public SGraphEdgeIter {
  SGraphEdgeList *_list;
  bool _owned; // If owned, delete it afterwards
  SGraphEdgeList::iterator _iter;
public:
  SGraphEdgeListIter();
  SGraphEdgeListIter(const SGraphEdgeListIter &other);
  SGraphEdgeListIter &operator=(const SGraphEdgeListIter &other);
  SGraphEdgeListIter(SGraphEdgeList *list, bool owned);
  void reset(SGraphEdgeList *list, bool owned);
  void reset();
  virtual bool done() const;
  virtual SGraphEdge get() const;
  virtual void increment();
  virtual SGraphEdge step();
  virtual SGraphEdgeIter *clone() const;
};


/*
 *
 * Filters
 * 
 * implements filters that choose among nodes
 */


// This is a simple example of a filter to filter only
// the nodes that are in a graph.
class SGraphNodeGraphFilter : public SGraphNodeFilter {
  const SGraph *_the_sgraph;
  SGraphNodeGraphFilter(const SGraphNodeGraphFilter &) :
    _the_sgraph(NULL) {}
  SGraphNodeGraphFilter &operator=(const SGraphNodeGraphFilter &other) {
    _the_sgraph = other._the_sgraph; return *this;}
public:
  SGraphNodeGraphFilter(const SGraph *the_sgraph) :
    _the_sgraph(the_sgraph) {}
  virtual bool is_node_member(SGraphNode node) const {
    return(_the_sgraph->is_node_member(node)); }
};

class SGraphFilteredNodeIter : public SGraphNodeIter {
  SNodeIter _base_iter;
  SGraphNodeFilter *_filter;
  bool _owned;
  size_t _last;
  bool _done;
public:
  SGraphFilteredNodeIter();
  SGraphFilteredNodeIter(const SGraphFilteredNodeIter &other);
  SGraphFilteredNodeIter(SNodeIter base_iter,
			 SGraphNodeFilter *filter,
			 bool owned);
  SGraphFilteredNodeIter &operator=(const SGraphFilteredNodeIter &other);
  void reset(SNodeIter base_iter, SGraphNodeFilter *filter,
	     bool owned);
  virtual void reset();
  virtual bool done() const;
  virtual SGraphNode get() const;
  virtual void increment();
  virtual SGraphNodeIter *clone() const;
};

#endif /* SGRAPH_ITER_IMPL_H */
