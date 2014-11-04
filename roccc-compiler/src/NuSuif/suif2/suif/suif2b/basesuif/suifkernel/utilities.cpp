#include "common/system_specific.h"
#include "iokernel/meta_class.h"
#include "iokernel/cast.h"
#include "iokernel/object_wrapper.h"
#include "iokernel/pointer_meta_class.h"

#include "suifkernel_forwarders.h"
#include "utilities.h"
#include "suif_object.h"

#include <common/suif_vector.h>
//#include <iostream.h>
#include <iostream>
using namespace std;

static bool check_subtype(const ObjectWrapper &object,
			  const MetaClass *what) {
  const MetaClass *mc = object.get_meta_class();
  //  if (is_kind_of_suif_object_meta_class(mc)) {
  //    object.print_debug();
    //  }
  return(mc->defines_a_subtype_of(what));
}

static Iterator *build_object_iterator(const ObjectWrapper &object) {
  return(object.get_meta_class()->get_iterator(object));
}

static Iterator *build_object_iterator(const ObjectWrapper &object,
				       Iterator::Contents contents) {
  return(object.get_meta_class()->get_iterator(object, contents));
}


class ObjectIterator : public Iterator {
public:
   static sf_owned ObjectIterator* create_object_iterator( const ObjectWrapper &start_object,
							   const MetaClass* what,
							   bool itself_too = true );

   virtual ~ObjectIterator();

   virtual const MetaClass* current_meta_class() const;

   virtual void* current() const;

   virtual const LString& current_name() const;

   virtual bool is_valid() const;

   virtual void next();

   virtual void previous();

   virtual void first();

   sf_owned Iterator *clone() const;

   virtual sf_owned Iterator* go_deeper();
   virtual void print_to_default() const;
    ObjectIterator(const ObjectIterator &other) :
	Iterator(),
	_is_valid(other._is_valid),
	_what(other._what),
	_start_object(other._start_object),
	_it_stack(),
	_top(other._top ? other._top->clone(): 0)
    {
	for (size_t i=0; i < other._it_stack.size(); i++) {
	    _it_stack.push_back(other._it_stack[i]->clone());
	}
    }

protected:
   ObjectIterator( const ObjectWrapper &start_object,
		   const MetaClass* what, bool itself_too );

   bool _is_valid;
   const MetaClass* _what;
   ObjectWrapper _start_object;
  //   Address _start_object;
  //   const MetaClass* _object_type;
   sf_owned suif_vector<Iterator*> _it_stack;
   sf_owned Iterator* _top;
   bool _itself_too;
};

//sf_owned Iterator *ObjectIterator::clone() const {
sf_owned Iterator *ObjectIterator::clone() const {
  ObjectIterator *n = new ObjectIterator(*this);
  Iterator *i = (Iterator*)n;
  //Iterator *n = (Iterator) new ObjectIterator(*this);
  //return n;
  return i;
}


sf_owned ObjectIterator* ObjectIterator::
create_object_iterator( const ObjectWrapper &start_object, 
			const MetaClass* what, bool itself_too ) {
  ObjectIterator* it = 
    new ObjectIterator( start_object, what, itself_too );
  it->first();
  return it;
}



ObjectIterator::ObjectIterator( const ObjectWrapper &start_object,
				const MetaClass* what,
				bool itself_too) :
         _is_valid( false ),
         _what( what ),
         _start_object( start_object.update_meta_class() ),
         _top( 0 ),
         _itself_too( itself_too )  {
}


ObjectIterator::~ObjectIterator() {
  while ( _it_stack.size() ) {
    delete _it_stack.back();
    _it_stack.pop_back();
  }
  delete _top;
}

const MetaClass* ObjectIterator::current_meta_class() const {
  return _is_valid ? _what : 0;
}

const LString& ObjectIterator::current_name() const {
  return _is_valid ? _top->current_name() : emptyLString;
}


Address ObjectIterator::current() const {
  return _is_valid ? ( _top ? _top->current() : _start_object.get_address() ) : 0;
}


bool ObjectIterator::is_valid() const {
  return _is_valid;
}



void ObjectIterator::first() {
  // flush the stack
  delete _top;
  _top = 0;
  while ( _it_stack.size() ) {
    delete _it_stack.back();
    _it_stack.pop_back();
  }

  // get first element
  _is_valid = false;
  if ( _itself_too ) 
    _is_valid = check_subtype(_start_object, _what);
  if ( !_is_valid ) {
     suif_assert( !_top );
    _top = build_object_iterator(_start_object);
    if ( _top ) {
      _is_valid = true;
      if (check_subtype(_top->current_object(), _what)) 
	return;
      next();
    } else {
      _is_valid = false;
    }
  }
}


sf_owned Iterator* ObjectIterator::go_deeper() {
   suif_assert( _is_valid );
   suif_assert( _top );
   // get the current metaclass in the iteration
   // find the address of the object that it corresponds to
   // get an iterator on that address for the current meta_class
   ObjectWrapper object(_top->current_object());
   return(build_object_iterator(object));
}



void ObjectIterator::next() {
  if ( _is_valid ) {
    // the iterator currently points to a valid element
    // iterate till either the end is reached or
    // a matching object is found
    if ( !_top ) {
      suif_assert (check_subtype(_start_object, _what));
      _top = build_object_iterator(_start_object);
      _is_valid = _top;
    }
  }
  if ( _is_valid ) {
    suif_assert( _top );
    while ( _top->is_valid() ) {
      // advance. first try to go deeper
      sf_owned Iterator* new_top = go_deeper();
      if ( new_top && (!new_top->is_valid()) ) {
         delete new_top;
         new_top = 0;
      }
      if ( new_top ) {
        kernel_assert( new_top );
        _it_stack.push_back( _top );
        _top = new_top;
      } else {
        _top->next();
        while ( !_top->is_valid() && _it_stack.size() ) {
           delete _top;  // back up and restart
           _top = _it_stack.back();
           _it_stack.pop_back();
//	   cerr<<"POP"<<endl;
           _top -> next();
        }
      }
      _is_valid = _top->is_valid();

      if ( _is_valid ) {
//        cerr<<"utilities.cpp::x "<<_it_stack.size()<<"  "<<_top->current_meta_class()->get_instance_name().c_str()<<endl;
        if ( check_subtype(_top->current_object(), _what ))
	  break;
      }
    }
  }
  if (_is_valid) {
    suif_assert(check_subtype(_top->current_object(), 
			      _what )) ;
  }
}



void ObjectIterator::previous() {
  assert( false ); // not implemented
}


sf_owned Iterator* object_iterator_ut( const ObjectWrapper &start_object,
				    const MetaClass* what ) {
  ObjectIterator *o = ObjectIterator::create_object_iterator( start_object, what );
  //fprintf(stderr, "[JUL] utilities.cpp@object_iterator_ut :: %x\n",o);
  return o;
}





class StopObjectIterator : public ObjectIterator {
public:
  static StopObjectIterator* 
  create_stop_object_iterator( const ObjectWrapper &object,
			       const MetaClass* _dont_search_beyond, 
			       const MetaClass* what, 
			       bool itself_too );

   virtual Iterator* go_deeper();

protected:
   StopObjectIterator( const ObjectWrapper &start_object, 
		       const MetaClass* _dont_search_beyond, 
		       const MetaClass* what, bool itself_too );

  const MetaClass* _dont_search_beyond;
};



StopObjectIterator* StopObjectIterator::
create_stop_object_iterator( const ObjectWrapper &start_object,
			     const MetaClass* dont_search_beyond,
			     const MetaClass* what,
			     bool itself_too ) {
  StopObjectIterator* it = new StopObjectIterator( start_object, dont_search_beyond, what, itself_too );
  it->first();
  return it;
}

StopObjectIterator::StopObjectIterator( const ObjectWrapper &start_object, 
					const MetaClass* dont_search_beyond, 
					const MetaClass* what, 
					bool itself_too  ) :
  ObjectIterator( start_object, what, itself_too ), 
  _dont_search_beyond( dont_search_beyond ) {
}



Iterator* StopObjectIterator::go_deeper() {
   suif_assert( _is_valid );
   suif_assert( _top );
   ObjectWrapper object(_top->current_object());
   Iterator* it = 0;
   if ( !check_subtype(object, _dont_search_beyond )) {
      it = build_object_iterator(object, Iterator::Owned );
   }
   return it;
}



void ObjectIterator::print_to_default() const {
  for (unsigned i = 0; i < _it_stack.size(); i++) {
    Iterator *iter = _it_stack[i];
    cerr << "[" << i << "] ";
    iter->print_to_default();
    cerr << "\n";
  }
  {
    Iterator *iter = _top;
    if (iter->is_valid()) {
      cerr << "[TOP] ";
      iter->print_to_default();
      cerr << "\n";
    }
  }
  cerr << "\n";
}



Iterator* object_iterator_ut( const ObjectWrapper &start_object,
                           const MetaClass* dont_search_beyond,
                           const MetaClass* what ) {
  return StopObjectIterator::create_stop_object_iterator( start_object, dont_search_beyond, what, false );
}

class ObjectRefIterator : public Iterator {
public:
   static sf_owned ObjectRefIterator* 
   create_object_iterator( const SuifObject *start_object,
			   const MetaClass* what );

   virtual ~ObjectRefIterator();

   virtual const MetaClass* current_meta_class() const;

   virtual void* current() const;

   virtual ObjectWrapper current_obj() const;

   virtual const LString& current_name() const;

   virtual bool is_valid() const;

   virtual void next();

   virtual void previous();

   virtual void first();

   sf_owned Iterator *clone() const;

   virtual void print_to_default() const;
   ObjectRefIterator(const ObjectRefIterator &other) : 
       Iterator(), _what(other._what), _start_object(other._start_object)
    {
    };
   ObjectRefIterator &operator=(const ObjectRefIterator &other)
    {
	_what = other._what;
	_start_object = other._start_object;
	return *this;
    };

protected:
   void clear();
   Iterator *top_iter() const;
   void push_iter(Iterator *it, const ObjectWrapper &obj);
   void pop_iter();// remove the top iterator
   virtual bool go_deeper(const ObjectWrapper &obj);
   bool is_target_object() const;

   ObjectRefIterator( const SuifObject *start_object, 
		      const MetaClass* what );

  typedef suif_vector<ObjectWrapper> obj_stack;
  typedef suif_vector<Iterator *> iter_stack;

   // These are parallel and should have the same number of elements.

   obj_stack _obj_stack;
   sf_owned iter_stack _it_stack;
  
   const MetaClass* _what;
   const SuifObject  *_start_object;
};

Iterator *
ObjectRefIterator::top_iter() const {
  suif_assert(is_valid());
  return(_it_stack.back());
}

void
ObjectRefIterator::pop_iter() {
  suif_assert(is_valid());
  delete top_iter();
  _it_stack.pop_back();
  _obj_stack.pop_back();
}

void
ObjectRefIterator::push_iter(Iterator *iter, const ObjectWrapper &obj) {
  _it_stack.push_back(iter);
  _obj_stack.push_back(obj);
}
  
sf_owned Iterator *ObjectRefIterator::clone() const {
    ObjectRefIterator *other = new ObjectRefIterator(*this);
    for (iter_stack::const_iterator iter = _it_stack.begin();
	 iter != _it_stack.end(); iter++) {
      other->_it_stack.push_back(*iter);
    }
    for (obj_stack::const_iterator iter1 = _obj_stack.begin();
	 iter1 != _obj_stack.end(); iter1++) {
      other->_obj_stack.push_back(*iter1);
    }
    return other;
    }


sf_owned ObjectRefIterator* ObjectRefIterator::
create_object_iterator( const SuifObject *start_object, 
			const MetaClass* what) {
  ObjectRefIterator* it = new ObjectRefIterator( start_object, what );
  it->first();
  return it;
}



ObjectRefIterator::ObjectRefIterator( const SuifObject *start_object,
				      const MetaClass* what) :
  _what( what ),
  _start_object( start_object ) {
}


ObjectRefIterator::~ObjectRefIterator() {
  clear();
}

const MetaClass* ObjectRefIterator::current_meta_class() const {
  return is_valid() ? top_iter()->current_meta_class() : NULL;
}

ObjectWrapper ObjectRefIterator::current_obj() const {
  return(ObjectWrapper(current(), current_meta_class()));
}

const LString& ObjectRefIterator::current_name() const {
  return is_valid() ? top_iter()->current_name() : emptyLString;
}


Address ObjectRefIterator::current() const {
  return is_valid() ? top_iter()->current() : 0;
}


bool ObjectRefIterator::is_valid() const {
  return(!_it_stack.empty());
}

void ObjectRefIterator::clear() {
  while ( is_valid() ) {
    pop_iter();
  }
}
  

void ObjectRefIterator::first() {
  // flush the stack
  clear();
  if (!go_deeper(ObjectWrapper(_start_object))) return;
  //  Iterator *it = go_deeper(ObjectWrapper(_start_object));
  //  if (it == NULL) return; // nothing to do.
  //  ObjectWrapper obj(_start_object);
  //  push_iter(it, obj);

  if (is_target_object()) return;
  next();
}

//return false if we can't go deeper.
// return true if we can go deeper and push the ner iterator on the
// stack

sf_owned bool ObjectRefIterator::go_deeper(const ObjectWrapper &obj) {
  // This is special for the references
  if (is_kind_of_suif_object_meta_class(obj.get_meta_class())) {
    // don't go deeper on a SuifObject unless
    if (obj.get_address() != (void*)_start_object) return(false);
  }

  Iterator *it = NULL;
  if (is_kind_of<PointerMetaClass>(obj.get_meta_class())) {
    it = build_object_iterator(obj, Iterator::Referenced);
  } else {
    it = build_object_iterator(obj);
  }

  if (it == NULL) return false;
  if (!it->is_valid()) { delete it; return false; }

  //  ObjectWrapper obj(_start_object);
  push_iter(it, obj);
  return(true);
}

bool ObjectRefIterator::is_target_object() const {
  if (!is_valid()) return(false);
  return ( check_subtype(current_object(),
			 _what));
}  

void ObjectRefIterator::next() {
  suif_assert(is_valid());
  while ( is_valid() ) {
    // advance. first try to go deeper
    if (!go_deeper(current_obj())) {
      // If we can go no deeper, pop the stack until we can continue
      top_iter()->next();
      while (is_valid() && !top_iter()->is_valid()) {
	pop_iter();
	if (!is_valid()) break;
	top_iter()->next();
      }
    }
    if (is_target_object()) return;
  }
}



void ObjectRefIterator::previous() {
  assert( false ); // not implemented
}
void ObjectRefIterator::print_to_default() const {
  iter_stack::const_iterator it_iter = _it_stack.begin();
  obj_stack::const_iterator obj_iter = _obj_stack.begin();
  int i = 0;
  for (;
       it_iter != _it_stack.end(); it_iter++, obj_iter++, i++) {
    ObjectWrapper obj = *obj_iter;
    Iterator *it = *it_iter;

    cerr << "[" << i << "] ";
    obj.print_debug();
    it->print_to_default();
    cerr << "\n";
  }
  cerr << "\n";
}

sf_owned Iterator* object_ref_iterator_ut( SuifObject *start_object,
					   const MetaClass* what ) {
  return ObjectRefIterator::create_object_iterator( start_object,
						    what);
}

/*
 * these are for backward compatibility
 */

sf_owned Iterator* object_iterator( Address start_object,
				    const MetaClass* object_type,
				    const MetaClass* what ) {
   return ObjectIterator::create_object_iterator( ObjectWrapper(start_object, object_type), what );
}

sf_owned Iterator* object_ref_iterator( SuifObject *start_object,
					const MetaClass* what ) {
  return ObjectRefIterator::create_object_iterator( start_object,
						    what);
}


Iterator* object_iterator( Address start_object,
                           const MetaClass* start_object_type,
                           const MetaClass* dont_search_beyond,
                           const MetaClass* what ) {
  return StopObjectIterator::create_stop_object_iterator( ObjectWrapper(start_object, start_object_type), dont_search_beyond, what, false );
}

