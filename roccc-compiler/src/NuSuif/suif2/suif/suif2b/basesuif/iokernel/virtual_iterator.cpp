#include "common/system_specific.h"
#include "virtual_iterator.h"
#include "iokernel_forwarders.h"
#include "list_meta_class.h"
#include "aggregate_meta_class.h"
#include "stl_meta_class.h"
#include "pointer_meta_class.h"
#include "helper.h"
#include "field_description.h"


#include "common/suif_vector.h"
#include "common/suif_map.h"

// In order to be slightly more efficient,
// the Iterator here is NOT owned.
// It is owned by the owner of this class.
class IteratorState {
  VirtualNode* _node;
  ConstAddress _address;
  bool _is_iterator;
  union {
    Iterator* _iterator;
    size_t _iteration_num;
  };
public:
  // This is needed for the Vector to work properly
  IteratorState() :
    _node(0),
    _address(0),
    _is_iterator(false),
    _iteration_num(0) {}
  IteratorState(VirtualNode *node, ConstAddress address, 
		sf_owned Iterator *iter) :
    _node(node),
    _address(address),
    _is_iterator(true),
    _iterator(iter) {}
  IteratorState(VirtualNode *node, ConstAddress address) :
    _node(node),
    _address(address),
    _is_iterator(false),
    _iteration_num(0) {}

  IteratorState(const IteratorState &other) :
    _node(other._node),
    _address(other._address),
    _is_iterator(other._is_iterator) {
      if (other.is_iterator()) {
	_iterator = other._iterator; //->clone();
      } else {
	_iteration_num = other._iteration_num;
      }
  }
  ~IteratorState() {
    //    if (is_iterator()) delete _iterator;
  }
  IteratorState &operator=(const IteratorState &other) {
    _node = other._node;
    _address = other._address;
    //    if (is_iterator()) {
    //      delete _iterator;
    //    }
    _is_iterator = other._is_iterator;
    if (other.is_iterator()) {
      _iterator = other._iterator; //->clone();
    } else {
      _iteration_num = other._iteration_num;
    }
    return(*this);
  }

  VirtualNode *get_node() const { return _node; }
  ConstAddress get_address() const { return _address; }
  Iterator *get_iterator() const { kernel_assert(is_iterator()); return _iterator; }
  bool is_iterator() const { return(_is_iterator); }
  void increment() { kernel_assert(!is_iterator()); _iteration_num++; }
  size_t get_iteration_num() const { kernel_assert(!is_iterator()); return _iteration_num; }

};


struct AggregateElement {
  AggregateElement( int offset,
                    const MetaClass* meta_class,
                    String name,
                    sf_owned VirtualNode* continuation = 0,
		    bool cont_owned = true )
             : _name( name ),
               _offset( offset ),
               _meta_class( meta_class ),
               _continuation( continuation ),
	       _cont_owned(cont_owned) {
  }

  ~AggregateElement() {
    if (_continuation && _cont_owned) {
      delete  _continuation;
    }
  }
  String _name;
  int _offset;
  const MetaClass* _meta_class;
  sf_owned VirtualNode* _continuation;
  bool _cont_owned;
public:
private:
  AggregateElement(const AggregateElement &other) :
    _name(other._name), _offset(other._offset),
    _meta_class(other._meta_class), _continuation(other._continuation),
    _cont_owned(other._cont_owned)
    {}
  AggregateElement &operator=(const AggregateElement &other) {
    _name = other._name; _offset = other._offset;
    _meta_class = other._meta_class; _continuation = other._continuation;
    _cont_owned = other._cont_owned;
    return(*this);
  }
};



VirtualNode::~VirtualNode() {
}

const MetaClass* VirtualNode::current_meta_class( const VirtualIterator* state ) const {
  kernel_assert( false );
  return 0;
}

const String VirtualNode::current_name( const VirtualIterator* state ) const {
  kernel_assert( false );
  return emptyString;
}


Address VirtualNode::current( const VirtualIterator* state ) const {
  kernel_assert( false );
  return 0;
}



bool VirtualNode::next( VirtualIterator* state ) {
  kernel_assert( false );
  return false;
}


void VirtualNode::delete_state( IteratorState& state ) {
}



class AggregateVirtualNode : public VirtualNode {
public:

AggregateVirtualNode( suif_vector<AggregateElement*> sf_owned * members )
    : _members( members ) {
}

  virtual const MetaClass* current_meta_class( const VirtualIterator* state ) const {
     int index = state->top().get_iteration_num();
     kernel_assert( ! (*_members)[ index ]->_continuation );
     return (*_members)[ index ] -> _meta_class;
  }

  virtual const String current_name( const VirtualIterator* state ) const {
     int index = state->top().get_iteration_num();
     kernel_assert( ! (*_members)[ index ]->_continuation );
     return (*_members)[ index ] -> _name;
  }


  virtual Address current( const VirtualIterator* state ) const {
    IteratorState& it_state = state->top();
    int offset = (*_members)[ it_state.get_iteration_num() ]->_offset;
    Byte* base = (Byte*)it_state.get_address() + offset;
    return base;
  }


  virtual bool first( VirtualIterator* state, ConstAddress address ) {
    IteratorState it_state(this, address);
    //    it_state.address = address;
    //    it_state.node = this;
    bool is_valid = false;
    size_t number_of_elements = _members->size();
    for ( ;
          it_state.get_iteration_num() < number_of_elements;
          it_state.increment() ) {
      state->push( it_state );
      is_valid = !( (*_members)[ it_state.get_iteration_num() ]->_continuation );
      if ( !is_valid ) {
        is_valid = (*_members)[ it_state.get_iteration_num() ]->_continuation->first( state,
               (Byte*)address + (*_members)[ it_state.get_iteration_num() ]->_offset );
      }
      if ( is_valid ) {
        break;
      } else {
        state->pop();
      }
    }
    return is_valid;
  }

  virtual bool next( VirtualIterator* state ) {
    bool is_valid = false;
    state->top().increment();
    size_t offset = state->top().get_iteration_num();
    //    state->top().integer = offset;
    for (;
	 offset <  _members->size();
	 state->top().increment(), offset = state->top().get_iteration_num()){
      //	 ) {
      //    while ( offset <  _members->size() ) {
      is_valid = !( (*_members)[ offset ]->_continuation );
      if ( !is_valid ) {
        is_valid = (*_members)[ offset ]->_continuation->first(
              state, (Byte*)state->top().get_address() + (*_members)[ offset ] -> _offset );
      }
      if ( is_valid ) return true;
      state->top().increment();
      offset = state->top().get_iteration_num();
      //      offset = state->top().integer + 1;
      //      state->top().integer = offset;
    }
    is_valid = state->pop();
    if ( is_valid ) {
      return state->top().get_node()->next( state );
    }
    return is_valid;
  }

 virtual bool previous( VirtualIterator* state ) {
#ifdef AG
    IteratorState& it_state = state->top();
    it_state.integer--;
    bool is_valid = ( _members->size() >= it_state.integer );
    if ( !is_valid ) {
      state->pop();
      kernel_assert(0);
//      is_valid =  state->top().node->previous( state );
    }
    return is_valid;
#endif
   return false;
  }

protected:
  virtual ~AggregateVirtualNode() {
    delete_list_and_elements( _members ); 
  }

  suif_vector<AggregateElement*> sf_owned * _members;
private:
  AggregateVirtualNode(const AggregateVirtualNode &other) :
    _members(0) { kernel_assert(0); }
  AggregateVirtualNode &operator=(const AggregateVirtualNode &other) {
    kernel_assert(0); 
    return(*this);
  }
    
};




class PointerVirtualNode : public VirtualNode {
public:
  PointerVirtualNode( VirtualNode* target ) :
    _target( target ) {}

  virtual ~PointerVirtualNode() {
    delete _target;
  }

  virtual bool first( VirtualIterator* state, ConstAddress address ) {
    Byte* target = (Byte*)*(Address*)address;
    if ( !target ) return false;
    return _target->first( state, target );
  }

private:
  VirtualNode* _target;
  PointerVirtualNode(const PointerVirtualNode &other);
#ifdef AG
 :
    _target(other._target) {}
#endif
  PointerVirtualNode &operator=(const PointerVirtualNode &other);
#ifdef AG
    _target = other._target;
    return(*this);
  }
#endif
};


class ListVirtualNode : public VirtualNode {
public:
  ListVirtualNode( ListMetaClass* list_meta_class, VirtualNode* element ) :
    _list_meta_class( list_meta_class ), _start_node( element ) {}

  virtual ~ListVirtualNode() {
    delete _start_node;
  }

  virtual bool first( VirtualIterator* state, ConstAddress address ) {
    Iterator* iter = _list_meta_class -> get_iterator( address );
    if ( !iter ) return false;
    if ( !iter->is_valid() ) {
      delete iter;
      return false;
    }
    IteratorState s(this, address, iter);
//    s.address = address;
//    s.node = this;
//    s.ptr = iter;
    state->push( s );
    bool is_valid = _start_node->first( state, iter->current() );
    if ( !is_valid ) is_valid = next( state );
    return is_valid;
  }

  virtual bool next( VirtualIterator* state ) {
    Iterator* iter = state->top().get_iterator();
    iter->next();
    bool is_valid = iter->is_valid();
    if ( !is_valid ) {
      state->pop();
      delete iter;
    } else {
      return _start_node->first( state, iter->current() );
    }
    return false;
  }

  virtual void delete_state( IteratorState& state ) {
    delete state.get_iterator();
  }

private:
  ListMetaClass* _list_meta_class;
  VirtualNode* _start_node;
  ListVirtualNode(const ListVirtualNode &other);
#ifdef AG
 :
    _list_meta_class(other._list_meta_class),
    _start_node(other._start_node)
    {}
#endif
  ListVirtualNode &operator=(const ListVirtualNode &other);
#ifdef AG
{
    _list_meta_class = other._list_meta_class;
    _start_node = other._start_node;
    return(*this);
  }
#endif    
};




VirtualIterator::VirtualIterator( ConstAddress address, VirtualNode* start_node ) :
  _is_valid(false), _state( new suif_vector<IteratorState> ), _start_node( start_node ) {
    IteratorState start_state(start_node, address);
//    start_state.address = address;
    // DLH - I think that the start node should be stored in the
    // first one.  Why does it need to be duplicated
    // 
//    start_state.node = start_node;
    _state -> push_back( start_state );
    _is_valid = _start_node && _start_node->first( this, address );
}

VirtualIterator::~VirtualIterator() {
    suif_vector<IteratorState>::iterator current = _state->begin(),
                                        end = _state->end();
    while ( current != end ) {
      IteratorState& s = *current;
      if (s.get_node()) {
        s.get_node()->delete_state( s );
      }
      current++;
    }
  delete _state;
}

const MetaClass* VirtualIterator::current_meta_class() const {
  if ( !_is_valid ) return 0;
  return current_node()->current_meta_class( this );
}


const LString& VirtualIterator::current_name() const {
  kernel_assert( false ); // not implemented yet
  return emptyLString;
}


Iterator *VirtualIterator::clone() const {
  VirtualIterator *n = new VirtualIterator( (*_state)[0].get_address(), _start_node );
  kernel_assert(n->_state->size() >= 1);
  for (size_t i = 1; i < _state->size(); i++ ) {
    IteratorState new_iter = (*_state)[i];
    if (new_iter.is_iterator()) { 
      new_iter = IteratorState(new_iter.get_node(), new_iter.get_address(),
                               new_iter.get_iterator()->clone());
    }
    n->_state->push_back(new_iter);
  }
  n->_is_valid = _is_valid;
  return n;
}


Address VirtualIterator::current() const {
  if ( !_is_valid ) return 0;
  return current_node()->current( this );
}

bool VirtualIterator::is_valid() const {
  return _is_valid;
}


void VirtualIterator::next() {
  if ( !_is_valid ) return;
  _is_valid = current_node()->next( this );
}


void VirtualIterator::previous() {
  if ( !_is_valid ) return;
  assert( false );
  //    _is_valid = current_node()->previous( this );
}

void VirtualIterator::first() {
  kernel_assert(_state->size() != 0);
  if (!current_node())
    _is_valid = false;
  else
    _is_valid = current_node()->first( this, (*_state)[0].get_address() );
}


void VirtualIterator::set_to( size_t index ) {
 first();
 while ( _is_valid && ( index-- ) ) {
   next();
 }
}

IteratorState& VirtualIterator::top() const {
  return (*_state)[_state->size()-1];
}


bool VirtualIterator::pop() {
  bool is_ok = ( _state->size() > 2 );
  if ( is_ok) {
    _state->pop_back();
  }
  return is_ok;
}


void VirtualIterator::push( const IteratorState& state ) {
  _state->push_back( state );
}


VirtualNode* VirtualIterator::current_node() const {
  return top().get_node();
}


VirtualNode* MetaClass::get_virtual_node( const LString &name, const String &what ) const {
  String spec = what;
  suif_vector<AggregateElement*>* members = new suif_vector<AggregateElement*>;
  if ( !spec.size() ) {
    members->push_back( new AggregateElement( 0, (MetaClass*)this, "" ) );
  } else {
    kernel_assert_message( false, ("Expected empty what string")  );
  }
  return new AggregateVirtualNode( members );

}


VirtualNode* AggregateMetaClass::get_virtual_node( const LString &name, const String &what ) const {
  String spec = what;
  if ( !spec.size() ) return 0;
  String current_element;
  suif_vector<AggregateElement*>* members = new suif_vector<AggregateElement*>;
  while ( spec.size() ) {
    current_element = cut_off( spec, ';' );
    if ( current_element == String( "^" ) ) {
      // insert the inherited ones
      for ( AggregateMetaClass* m = _base_class;
            m;
            m = m -> _base_class ) {
        NodeMap* virtual_nodes = m->_virtual_nodes;
        if ( !virtual_nodes ) {
          m->init_virtual_nodes();
          virtual_nodes = m->_virtual_nodes;
        }
        NodeMap::iterator iter = virtual_nodes->find( name );
        if ( iter != virtual_nodes -> end()  ) {
           AggregateElement* element = new AggregateElement(
                    0,0, "" ,(*iter).second, false );
           members->push_back( element );
           break;
        }
      }
      continue;
    }
    String current_member = cut_off( current_element, '/' );
    FieldDescription* field_description =
        get_field_description( current_member );
    VirtualNode* continuation = 0;
    if ( current_element.size() ) {
      continuation =  field_description->get_meta_class()->get_virtual_node( name, current_element );
    }
    members->push_back( new AggregateElement( field_description->get_offset(),
				      field_description->get_meta_class(), 
				      field_description->get_member_name(), 
				      continuation, true ));
  }
  return new AggregateVirtualNode( members );
}


VirtualNode* PointerMetaClass::get_virtual_node( const LString &name, const String &what ) const {
  String spec = what;
  VirtualNode* virtual_node = 0;

  if ( spec.size() ) {
    String current_member = cut_off( spec, '/' );

    if ( current_member == String("*") ) {
      // indirection specified => return the Object it points to
      suif_vector<AggregateElement*>* members = new suif_vector<AggregateElement*>;
      VirtualNode* continuation = _base_type->get_virtual_node( name, spec );
      members->push_back( new AggregateElement( 0, _base_type, "", continuation ) );
      virtual_node =  new PointerVirtualNode( new AggregateVirtualNode( members ) );
    }
  }
  return virtual_node;
}


sf_owned VirtualNode* ListMetaClass::get_virtual_node( const LString &name, const String &what ) const {
  String spec = what;
  VirtualNode* virtual_node = 0;

  if ( spec.size() ) {
    String current_member = cut_off( spec, '/' );

    kernel_assert( current_member == String("*") ); // This must always be the case

    MetaClass* elementMetaClass = get_element_meta_class();
    VirtualNode* element = elementMetaClass ->get_virtual_node( name, spec );
    if ( !element ) {
      suif_vector<AggregateElement*>* members = new suif_vector<AggregateElement*>;
      members->push_back( new AggregateElement( 0, (MetaClass*)this, "" ) );
      element = new AggregateVirtualNode( members );
    }
    virtual_node = new ListVirtualNode( (ListMetaClass*)this, (VirtualNode *)element );
  }
  return virtual_node;
}






VirtualIterator::VirtualIterator(const VirtualIterator &) :
  _is_valid(0), _state(0), _start_node(0)
{
  kernel_assert(false);
}
VirtualIterator &VirtualIterator::operator=(const VirtualIterator &) {
  kernel_assert(false);
  return(*this);
}

