#ifndef IOKERNEL__VIRTUAL_ITERATOR_H
#define IOKERNEL__VIRTUAL_ITERATOR_H

#include "iokernel_forwarders.h"
#include "meta_class.h"

class VirtualNode {
public:
  virtual ~VirtualNode();
  virtual const MetaClass* current_meta_class( const VirtualIterator* state ) const;
  virtual const String current_name( const VirtualIterator* state ) const;
  virtual void* current( const VirtualIterator* state ) const;  
  virtual bool first( VirtualIterator* state, ConstAddress address ) = 0;

  virtual bool next( VirtualIterator* state );
  virtual void delete_state( IteratorState& state );
};


class VirtualIterator : public Iterator {
public:
  VirtualIterator( ConstAddress address, VirtualNode* start_node );

  virtual const MetaClass* current_meta_class() const;
  virtual const LString& current_name() const;

  virtual Address current() const;

  virtual bool is_valid() const;

  virtual void next();

  virtual void previous();

  virtual void first();

  virtual void set_to( size_t index );

  virtual IteratorState& top() const;

  virtual bool pop();

  virtual void push( const IteratorState& state ) ;
  virtual VirtualNode* current_node() const;

  virtual ~VirtualIterator();

  virtual Iterator *clone() const;  
protected:
  bool _is_valid;

  suif_vector<IteratorState>* _state;
  VirtualNode* _start_node;
private:
  VirtualIterator(const VirtualIterator &);
  VirtualIterator &operator=(const VirtualIterator &);

};



#endif
