#ifndef SGRAPH_BIT_ITER_H 
#define SGRAPH_BIT_ITER_H 

#include "sgraph_iter.h"
#include "sgraph_bit.h"

/*
 * ******************************************
 * *
 * * iterator over a bitvector of nodes
 * *  This is useful elsewhere so it's in
 * *  the interface
 * *
 * ******************************************
 */
class SGraphBitIter : public SGraphNodeIter {
  const BitVector *_bits;
  bool _owned; // If owned, delete it afterwards
  BitVectorIter *_iter;
public:
  SGraphBitIter();
  SGraphBitIter(const SGraphBitIter &other);
  SGraphBitIter(const BitVector *bits, bool owned);
  SGraphBitIter &operator=(const SGraphBitIter &other);
  void reset(const BitVector *bits, bool owned);
  virtual void reset();

  virtual bool done() const;
  virtual SGraphNode get() const;
  virtual void increment();
  virtual SGraphNodeIter *clone() const;
};
#endif /* SGRAPH_BIT_ITER_H */
