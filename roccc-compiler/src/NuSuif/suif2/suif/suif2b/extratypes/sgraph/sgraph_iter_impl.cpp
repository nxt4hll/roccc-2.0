#include "sgraph_iter_impl.h"


/*
 * now some implementations
 */



//
// An edge list iterator that potntially owns a tos of edges.
//


SGraphEdgeListIter::SGraphEdgeListIter() : 
  _list(NULL), _owned(false), 
  _iter() {}
SGraphEdgeListIter::SGraphEdgeListIter(const SGraphEdgeListIter &other) :
  _list(NULL), _owned(false), _iter() {
  reset(other._list, false);
}
SGraphEdgeListIter &SGraphEdgeListIter::operator=(const SGraphEdgeListIter &other) {
  reset(other._list, false);
  return(*this);
}
SGraphEdgeListIter::SGraphEdgeListIter(SGraphEdgeList *lst, bool owned) :
  _list(NULL), _owned(false), _iter(){
  reset(lst, owned); 
}

SGraphEdgeIter *SGraphEdgeListIter::clone() const {
  return(new SGraphEdgeListIter(_list, false));
}

void SGraphEdgeListIter::reset(SGraphEdgeList *lst, bool owned) {
  if (_owned && _list != NULL) delete lst;
  _list = lst;
  _owned = owned;
  reset();
}
void SGraphEdgeListIter::reset() {
  _iter = _list->begin();
}
bool SGraphEdgeListIter::done() const { return (_iter == _list->end()); }
SGraphEdge SGraphEdgeListIter::get() const { return *_iter; }
void SGraphEdgeListIter::increment() { _iter++; }
SGraphEdge SGraphEdgeListIter::step() {
  SGraphEdge node = get(); increment(); return(node); }



//
// An node list iterator that potentially owns the list
//


SGraphNodeListIter::SGraphNodeListIter() : 
  _list(NULL), _owned(false), 
  _iter() {}
SGraphNodeListIter::SGraphNodeListIter(const SGraphNodeListIter &other) :
  _list(NULL), _owned(false), _iter() {
  reset(other._list, false);
}
SGraphNodeListIter &SGraphNodeListIter::operator=(const SGraphNodeListIter &other) {
  reset(other._list, false);
  return(*this);
}
SGraphNodeListIter::SGraphNodeListIter(SGraphNodeList *lst, bool owned) :
  _list(NULL), _owned(false), _iter(){
  reset(lst, owned); 
}

SGraphNodeIter *SGraphNodeListIter::clone() const {
  return(new SGraphNodeListIter(_list, false));
}

void SGraphNodeListIter::reset(SGraphNodeList *lst, bool owned) {
  if (_owned && _list != NULL) delete lst;
  _list = lst;
  _owned = owned;
  reset();
}
void SGraphNodeListIter::reset() {
  _iter = _list->begin();
}
bool SGraphNodeListIter::done() const { return (_iter == _list->end()); }
SGraphNode SGraphNodeListIter::get() const { return *_iter; }
void SGraphNodeListIter::increment() { _iter++; }
SGraphNode SGraphNodeListIter::step() {
  SGraphNode node = get(); increment(); return(node); }





SGraphFilteredNodeIter::SGraphFilteredNodeIter() :
  _base_iter(NULL), _filter(NULL), _owned(false), _last(0), _done(true) { }
SGraphFilteredNodeIter::SGraphFilteredNodeIter(const SGraphFilteredNodeIter &other) :
  _base_iter(NULL), _filter(NULL), _owned(false), _last(0), _done(true) {
  reset(other._base_iter, other._filter, false);
}
SGraphFilteredNodeIter::SGraphFilteredNodeIter(SNodeIter base_iter,
					       SGraphNodeFilter *filter,
					       bool owned) :
  _base_iter(NULL), _filter(NULL), _owned(false), _last(0), _done(true) {
  reset(base_iter, filter, owned);
}
SGraphFilteredNodeIter &SGraphFilteredNodeIter::operator=(const SGraphFilteredNodeIter &other) {
  reset(other._base_iter, other._filter, false);
  return(*this);
}

SGraphNodeIter *SGraphFilteredNodeIter::clone() const {
  return(new SGraphFilteredNodeIter(_base_iter.clone(), _filter, false));
}

void SGraphFilteredNodeIter::reset(SNodeIter base_iter,
				   SGraphNodeFilter *filter,
				   bool owned) {
  if (_owned && _filter != NULL) delete filter;
  _filter = filter;
  _base_iter = base_iter;
  _done = false;
  _last = 0;
  for(;!_base_iter.done(); _base_iter.increment()) {
    SGraphNode node = _base_iter.get();
    if (_filter->is_node_member(node)) {
      _last = node; return;
    }
  }
  _done = true;
}
void SGraphFilteredNodeIter::reset() {
  _base_iter.reset();
  _done = false;
  _last = 0;
  for(;!_base_iter.done(); _base_iter.increment()) {
    SGraphNode node = _base_iter.get();
    if (_filter->is_node_member(node)) {
      _last = node; return;
    }
  }
  _done = true;
}
bool SGraphFilteredNodeIter::done() const { return _done; }
size_t SGraphFilteredNodeIter::get() const { return _last; }
void SGraphFilteredNodeIter::increment() {
  if (_done) return;
  _base_iter.increment();
  for(;!_base_iter.done(); _base_iter.increment()) {
    SGraphNode node = _base_iter.get();
    if (_filter->is_node_member(node)) {
      _last = node; return;
    }
  }
  _done = true;
}
