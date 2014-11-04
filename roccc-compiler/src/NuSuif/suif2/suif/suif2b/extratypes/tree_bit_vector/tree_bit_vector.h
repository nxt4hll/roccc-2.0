#ifndef __TREE_BIT_VECTOR__
#define __TREE_BIT_VECTOR__
//#include <values.h>
#include <stdio.h>
#include <stddef.h>
#include "oosplay/oosplay.h"
#include <assert.h>
#include <gc_cpp.h>

//unsigned long n_changed_unions = 0;
//unsigned long n_unchanged_unions = 0; 

#define BITS_PER_LONG (sizeof(long) * 8)

class BitVectorMapFcn/*: public gc*/ {
public:
  virtual void apply(unsigned i) = 0;
};

class CountingFcn: public BitVectorMapFcn {
public:
  unsigned count;
  CountingFcn() { count = 0; }
  void apply(unsigned i) { ++count; }
};

class BitVectorChunk/*: public gc*/ {
  unsigned long _data;
public:
  BitVectorChunk() { _data = 0; }
  int bitor_into(const BitVectorChunk &other) {
    unsigned long old_data = _data;
    _data |= other._data;
    return old_data != _data;
  }
  void set_bit(size_t which_bit, bool value) {
    assert(value);
    assert(which_bit < BITS_PER_LONG);
    _data |= (1 << which_bit);
  }
  int  get_bit(size_t which_bit) {
    return _data & (1 << which_bit);
  }
  void print(FILE *f, int offset) {
    int i = 0;
    unsigned long d = _data;
    while (d != 0) {
      if (d & 1) {
	fprintf(f, "%d ", i + offset);
      }
      d >>= 1;
      ++i;
    }
  }
  void map(BitVectorMapFcn *clo, int offset) {
    int i = 0;
    unsigned long d = _data;
    while (d != 0) {
      if (d & 1) {
	clo->apply(i+offset);
      }
      d >>= 1;
      ++i;
    }
  }

  virtual void print_stdout(int offset) {
    print(stdout, offset);
    fflush(stdout);
  }
  bool is_zero() {
    return _data == 0;
  }
  void set_to_zero() {
      _data = 0;
  }
};

typedef oosplay_node_elt<unsigned, BitVectorChunk> bit_vector_node;
typedef oosplay_tree<unsigned, bit_vector_node> bit_vector_tree;


class BitVector; 

class OrOneNode: public IterClosure<bit_vector_node> {
  BitVector *_or_with;
public:
  int changed;
  OrOneNode(BitVector *or_with) { _or_with = or_with; changed = 0; }
  virtual void apply(bit_vector_node *n);
};


class MapOneNode: public IterClosure<bit_vector_node> {
  BitVectorMapFcn *_map_with;
public:
  MapOneNode(BitVectorMapFcn *map_with) {
    _map_with = map_with;
  }
  void apply(bit_vector_node *n) {
    n->get_elt_addr()->map(_map_with, n->get_key1() * BITS_PER_LONG);
  }
};

class PrintOneNode: public IterClosure<bit_vector_node> {
  FILE *_f;
public:
  PrintOneNode(FILE *f) { _f = f; }
  virtual void apply(bit_vector_node *n) {
    assert(!n->get_elt_addr()->is_zero());
    n->get_elt_addr()->print(_f, n->get_key1() * BITS_PER_LONG);
  }
};

class BitVector/*: public gc*/ {
  friend class OrOneNode;
  bit_vector_tree _tree;
public:
  BitVector() {}
  void operator|=(BitVector &other) {
    if (_tree.is_empty()) {
      _tree.copy(&other._tree);
      //      n_changed_unions++;
    }
    else {
      OrOneNode OON(this);
      other._tree.iterate(&OON);
      //      if (OON.changed) n_changed_unions++; else n_unchanged_unions++;
    }
  }
  void set_bit(size_t which_bit, bool value) {
    /* Compute index */
    unsigned int index = which_bit / BITS_PER_LONG;
    unsigned int offset = which_bit % BITS_PER_LONG;
    /* Find appropriate node */
    bit_vector_node *lookup;
    /* If node exits, then modify it else create new node */
    if (_tree.lookup(index, &lookup)) {
      lookup->get_elt_addr()->set_bit(offset, 1);
    } 
    else {
      bit_vector_node *n = new bit_vector_node(index, BitVectorChunk());
      n->get_elt_addr()->set_bit(offset, 1);
      _tree.insert_node(n);
    }
  }
  bool get_bit(size_t which_bit) {
    /* Compute index */
    unsigned int index = which_bit / BITS_PER_LONG;
    unsigned int offset = which_bit % BITS_PER_LONG;
    /* Find appropriate node */
    bit_vector_node *lookup;
    /* If node exits, then modify it else create new node */
    if (_tree.lookup(index, &lookup)) {
      return lookup->get_elt_addr()->get_bit(offset);
    } 
    return 0;    
  }
  void print(FILE *f) {
    PrintOneNode PON(f);
    _tree.iterate(&PON);
  }

  void map(BitVectorMapFcn *clo) {
    MapOneNode handle_node(clo);
    _tree.iterate(&handle_node);
  }

  void set_to_zero() {
    _tree.erase();
  }
  void copy(BitVector *from) {
    _tree.copy(&from->_tree);
  }
  int count() {
    CountingFcn cfcn;
    map(&cfcn);
    return cfcn.count;
  }
  int is_empty() {
    return _tree.is_empty();
  }
  virtual void print_stdout() {
    print(stdout);
    printf("size: %d\n", _tree.tree_size());
    fflush(stdout);
  }
};

void OrOneNode::apply(bit_vector_node *n) {
  bit_vector_node *lookup;
  if (_or_with->_tree.lookup(n->get_key1(), &lookup)) {
    changed = lookup->get_elt_addr()->bitor_into(n->get_elt()) || changed;
  }
  else {
    _or_with->_tree.insert_node(new bit_vector_node(n->get_key1(), 
						    n->get_elt()));
  }
}
#endif
