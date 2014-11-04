#include "sgraph.h"

#ifndef SGRAPH_ITER_H
#define SGRAPH_ITER_H


/*
 * ******************************************
 * *
 * * iterators over nodes
 * *
 * ******************************************
 */
class SGraphNodeIter {
public:
  /* No initialization should be specified here */
  //  virtual SGraph *get_graph() const = 0;
  virtual SGraphNode get() const = 0;
  virtual bool done() const = 0;
  virtual void increment() = 0;
  //virtual void next() { increment(); }
  virtual void reset() = 0;
  
  // Helpers
  //virtual SGraphNode step() { SGraphNode node = get(); increment(); return(node); }

  // SUIF2 version
  virtual bool is_valid() const;
  virtual SGraphNode current() const;
  virtual void next();

  virtual ~SGraphNodeIter() {}

  virtual SGraphNodeIter *clone() const = 0;
};



class SGraphEdgeIter {
public:
  /* No initialization should be specified here */
  //  virtual SGraph *get_graph() const = 0;
  virtual SGraphEdge get() const = 0;
  virtual bool done() const = 0;
  virtual void increment() = 0;
  virtual void reset() = 0;

  // Helper
  //virtual SGraphEdge step() { SGraphEdge edge = get(); increment(); return(edge); }
  // SUIF2 version
  virtual bool is_valid() const;
  virtual SGraphEdge current() const;
  virtual void next();


  virtual ~SGraphEdgeIter() {};
  virtual SGraphEdgeIter *clone() const = 0;
 private:
  // Do not implement this
  SGraphEdgeIter &operator=(const SGraphEdgeIter &);
};

/*
 * Value classes for node and edge iterators
 * that can be passed around in common cases.
 */

// This is a mixed ownership bag that allows
// for a simple
class SNodeIter {
  SGraphNodeIter *_iter;
  bool _owned;
  void clone_iter();

public:
  SNodeIter();
  ~SNodeIter();
  SNodeIter(SGraphNodeIter *iter);
  SNodeIter(const SNodeIter &other);
  SNodeIter &operator=(const SNodeIter &other);

  SGraphNode get() const;
  bool done() const;
  void increment();
  void reset();

  // the suif2 form
  bool is_valid() const;
  SGraphNode current() const;
  void next();
  
  // Helper
  //  SGraphNode step();
  SNodeIter clone() const;
};


// value class to pass around
class SEdgeIter {
  SGraphEdgeIter *_iter;
  bool _owned;
  void clone_iter();

public:
  SEdgeIter();
  ~SEdgeIter();
  SEdgeIter(SGraphEdgeIter *iter);
  SEdgeIter(const SEdgeIter &other);
  SEdgeIter &operator=(const SEdgeIter &other);

  SGraphEdge get() const;
  bool done() const;
  void increment();
  void reset();

  // the suif2 form
  bool is_valid() const;
  SGraphEdge current() const;
  void next();

  // Helper
  SGraphEdge step();
  SGraphEdge clone() const;
};


/*
 *
 * Filters
 * 
 * implements filters that choose among nodes
 */

class SGraphNodeFilter {
public:
  virtual bool is_node_member(SGraphNode node) const = 0;
  virtual ~SGraphNodeFilter() {}
};


/*
 * implementation of an empty iterator
 */
// An empty implementation
class SGraphEmptyIter : public SGraphNodeIter {
public:
  SGraphEmptyIter() {}
  virtual bool done() const { return true; }
  virtual SGraphNode get() const { assert(0); return(0); }
  virtual void increment() { assert(0); }
  virtual void reset() {}
  virtual SGraphNodeIter *clone() const { return new SGraphEmptyIter(); }
};



#endif /* SGRAPH_ITER_H */
