#include "sgraph_iter.h"
#include "bit_vector/bit_vector.h"


/*
 * SGraphNodeIter 
 * The value class
 */

bool SGraphNodeIter::is_valid() const { return(!done()); }
SGraphNode SGraphNodeIter::current() const { return(get()); }
void SGraphNodeIter::next() { increment(); }

bool SGraphEdgeIter::is_valid() const { return(!done()); }
SGraphEdge SGraphEdgeIter::current() const { return(get()); }
void SGraphEdgeIter::next() { increment(); }


/*
 * SNodeIter 
 * The value class
 */
void SNodeIter::clone_iter() {
  _iter = _iter->clone();
  _owned = true;
}
SNodeIter::~SNodeIter() {
  if (_owned) delete _iter;
}

SNodeIter::SNodeIter() :
  _iter(0),
  _owned(false)
{}

SNodeIter::SNodeIter(SGraphNodeIter *iter) :
  _iter(iter),
  _owned(true)
{}

SNodeIter::SNodeIter(const SNodeIter &x) :
  _iter(x._iter), _owned(x._owned) {
  const_cast<SNodeIter&>(x)._owned = false;
}

SNodeIter &SNodeIter::operator=(const SNodeIter &x) {
  _iter = x._iter;
  _owned = x._owned;
  const_cast<SNodeIter&>(x)._owned = false;
  return *this;
}

SGraphNode SNodeIter::get() const { return(_iter->get()); }
bool SNodeIter::done() const { return(_iter->done()); }
void SNodeIter::increment() { if (!_owned)clone_iter(); _iter->increment(); }
void SNodeIter::reset() { if (!_owned)clone_iter(); _iter->reset(); }
SNodeIter SNodeIter::clone() const { return(_iter->clone()); }

bool SNodeIter::is_valid() const { return(!done()); }
SGraphNode SNodeIter::current() const { return(get()); }
void SNodeIter::next() { increment(); }

  
// Helper
/*
SGraphNode SNodeIter::step() { if(!_owned)clone_iter(); return(_iter->step()); }
*/

/*
 * SEdgeIter 
 * The value class
 */
void SEdgeIter::clone_iter() {
  _iter = _iter->clone();
  _owned = true;
}

SEdgeIter::~SEdgeIter() {
  if (_owned) delete _iter;
}

SEdgeIter::SEdgeIter() :
  _iter(0),
  _owned(false)
{}

SEdgeIter::SEdgeIter(SGraphEdgeIter *iter) :
  _iter(iter),
  _owned(true)
{}

SEdgeIter::SEdgeIter(const SEdgeIter &x) :
  _iter(x._iter), _owned(x._owned) {
  const_cast<SEdgeIter&>(x)._owned = false;
}

SEdgeIter &SEdgeIter::operator=(const SEdgeIter &x) {
  _iter = x._iter;
  _owned = x._owned;
  const_cast<SEdgeIter&>(x)._owned = false;
  return *this;
}

SGraphEdge SEdgeIter::get() const { return(_iter->get()); }
bool SEdgeIter::done() const { return(_iter->done()); }
void SEdgeIter::increment() { if (!_owned)clone_iter(); _iter->increment(); }
void SEdgeIter::reset() { if (!_owned)clone_iter(); _iter->reset(); }
  
// Helper
/*
SGraphEdge SEdgeIter::step() { 
  if(!_owned)clone_iter(); return(_iter->step()); 
}
*/

bool SEdgeIter::is_valid() const { return(!done()); }
SGraphEdge SEdgeIter::current() const { return(get()); }
void SEdgeIter::next() { increment(); }
