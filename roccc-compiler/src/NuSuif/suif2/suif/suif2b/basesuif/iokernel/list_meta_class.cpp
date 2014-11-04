#include "common/system_specific.h"
#include "common/suif_list.h"
#include "list_meta_class.h"

#include "iokernel_forwarders.h"
#include "object_stream.h"
#include "object_factory.h"
#include "cast.h"
#include "pointer_meta_class.h"



// #ifdef PGI_BUILD
// #include <new>
// #else
// #include <new.h>
// #endif
#include <new>
#include <iostream>//jul modif
using namespace std;//jul modif

#ifdef DEBUG
//#include <iostream.h>
extern void inc_indent();
extern void dec_indent();
extern char *indent_text();
#endif

#include <stdio.h>



static const LString list_meta_class_class_name("ListMetaClass");



ListMetaClass::ListMetaClass( LString meta_class_name ) :
             MetaClass( meta_class_name ),
             _element_meta_class( 0 ) {
}

ListMetaClass::ListMetaClass(const ListMetaClass &other) :
  MetaClass(),
  _element_meta_class( other._element_meta_class ) {
  kernel_assert(false);
}
ListMetaClass &ListMetaClass::operator=(const ListMetaClass &) {
  kernel_assert(false);
  return(*this);
}


void ListMetaClass::write( const ObjectWrapper &obj,
			   OutputStream* outputStream ) const {
  kernel_assert(obj.get_meta_class() == this);
  Address instance = obj.get_address();

  static int counter = 0;
  counter ++;

  kernel_assert( _element_meta_class );
  Iterator* iterator = get_iterator( instance );
  int len = iterator->length();
  //bool valid = iterator->is_valid();
  outputStream->write_unsigned_int( len );
  int count = 0;
#ifdef DEBUG
  cerr<<"LIST::write "<< iterator->length() << endl;
  inc_indent();
#endif

  while ( iterator->is_valid() ) {
    count ++;
    kernel_assert( _element_meta_class == iterator->current_meta_class() );
    outputStream->write( iterator->current_object(),
			 false );
    iterator->next();
    }
  kernel_assert(count == len);
  delete iterator;
#ifdef DEBUG
  dec_indent();
  cerr << "END LIST write" << endl;
#endif
}


void ListMetaClass::read( const ObjectWrapper &obj,
			  InputStream* inputStream ) const {
  kernel_assert(obj.get_meta_class() == this);
  Address instance = obj.get_address();
  kernel_assert( _element_meta_class );
  unsigned int size = inputStream->read_unsigned_int();
  //unsigned int size_save = size;
#ifdef DEBUG
  cerr<<"LIST::read "<<size << endl;
  inc_indent();
#endif


  int instanceSize = _element_meta_class->get_size_of_instance();

  GenericList* genericList = new GenericList;
  genericList->count = size;
  genericList->space = size ? ::operator new(size*instanceSize) : 0;
  // if (size)debugit("list size ",size);

  inputStream->store_data( instance, genericList );
  Address space = genericList->space;

  while (size--) {
    inputStream->get_object_factory()->
                 create_empty_object_in_space( _element_meta_class, space );
    inputStream->read( ObjectWrapper(space, _element_meta_class),
		       false ); // third argument might need fixing @@@
    space = ((Byte*)space) + instanceSize;
  }
#ifdef DEBUG
  dec_indent();
  cerr << "END LIST read: " << endl;
#endif
}

Iterator* ListMetaClass::get_iterator( ConstAddress instance,
                                  Iterator::Contents contents ) const {
  Iterator* return_value = 0;
  if ( instance && ( contents == Iterator::Owned ) ) {
    return_value =
       new ListIterator( (GenericList*)instance, _element_meta_class );
  }
  return return_value;
}


bool ListMetaClass::is_elementary() const {
  return false;
}


void ListMetaClass::initialize( const ObjectWrapper &obj,
				InputStream* inputStream ) const {
  Address address = obj.get_address();
  kernel_assert(obj.get_meta_class() == this);
 
  // debugit("initializing",address);
  GenericList* genericList = (GenericList*)inputStream->retrieve_data( address );

  int instanceSize = _element_meta_class->get_size_of_instance();
  int i = genericList->count;
  Address space = genericList->space;

  while (i--) {
    _element_meta_class->initialize( ObjectWrapper(space, _element_meta_class),
				     inputStream );
    space = ((Byte*)space) + instanceSize;
  }
  copy_over( address, genericList );
  delete genericList;
}


void ListMetaClass::copy_over( Address target, GenericList* source ) const {
  *(GenericList*)target = *source;
}


void ListMetaClass::set_meta_class_of_object( Address address ) const {
  //  ((Object*)instance)->set_meta_class( this );
}


void ListMetaClass::constructor_function( Address place ) {
  new (place) ListMetaClass();
}


MetaClass* ListMetaClass::get_element_meta_class() const {
  return _element_meta_class;
}

Walker::ApplyStatus ListMetaClass::walk(const Address instance,Walker &walk) const {
    // would like to avoid expense of an interator here
    Iterator* iterator = get_iterator( instance );
    if(walk.get_makes_changes()) {
        list<Address> elements;
   	if (is_kind_of<PointerMetaClass>(_element_meta_class)) {
	    while ( iterator->is_valid() ) {
		Address* ptr = (Address * ) iterator->current();
	    	elements.push_back(*ptr);
	    	iterator->next();
	    	}
	    list<Address>::iterator iter = elements.begin();
            while (iter != elements.end()) {
                Address current = *iter;
                Walker::ApplyStatus status;
                status = _element_meta_class ->walk(&current,walk);
                switch (status) {
                    case Walker::Continue:
                    case Walker::Truncate:
                    case Walker::Replaced:
                        break;
                    case Walker::Stop:
                    case Walker::Abort:
                        return status;
                    }
                iter ++;
		}
	    }
	else {
       	    while ( iterator->is_valid() ) {
	    	elements.push_back(iterator->current());
	    	iterator->next();
	    	}
            list<Address>::iterator iter = elements.begin();
            while (iter != elements.end()) {
                Address current = *iter;
                Walker::ApplyStatus status;
                status = _element_meta_class ->walk(current,walk);
                switch (status) {
                    case Walker::Continue:
                    case Walker::Truncate:
                    case Walker::Replaced:
                        break;
                    case Walker::Stop:
                    case Walker::Abort:
                        return status;
                    }
                iter ++;
                }
	    }
        delete iterator;
	}
    else {
	while ( iterator->is_valid() ) {
            Address current = iterator->current();
            Walker::ApplyStatus status = _element_meta_class ->walk(current,walk);          
            switch (status) {
                case Walker::Continue:
                case Walker::Truncate:
                case Walker::Replaced:
                    break;
                case Walker::Stop:
                case Walker::Abort:
                    return status;
                }
            iterator->next();
            }
	delete iterator;
	}
    return Walker::Continue;
    }

const LString &ListMetaClass::get_class_name() {
    return list_meta_class_class_name;
}


void ListMetaClass::set_element_meta_class( MetaClass* mc ) {
  _element_meta_class = mc;
}


void ListMetaClass::destruct( const ObjectWrapper &obj,
			      bool called_from_destructor ) const {
  Address address = obj.get_address();
  kernel_assert(obj.get_meta_class() == this);

  // a GenericListObject object is never destroyed from the destructor
  kernel_assert( !called_from_destructor );

  // destruct the elements in the list
  Iterator* it;
  for ( it = get_iterator( address );
        it->is_valid();
        it->next() ) {
    it->current_meta_class()->destruct( it->current_object(),
					false );
  }
  delete it;
}

struct X_GenericList {
    char ch;
    GenericList field;
    };

static size_t alignment_of_GenericList() {
    return OFFSETOF(X_GenericList,field);
    }

void ListMetaClass::adjust_field_offsets() {
    _element_meta_class->adjust_field_offsets();
    set_alignment(alignment_of_GenericList());
    MetaClass::adjust_field_offsets();
    }


BaseListIterator::BaseListIterator( const MetaClass* element_meta_class ) :
  _is_valid(false),
  _element_meta_class( element_meta_class )
{}


BaseListIterator::BaseListIterator(const BaseListIterator &) :
  _is_valid(false),
  _element_meta_class(0)
{
  kernel_assert(false);
}
BaseListIterator &BaseListIterator::operator=(const BaseListIterator &) {
  kernel_assert(false);
  return(*this);
}

const MetaClass* BaseListIterator::current_meta_class() const {
  return _is_valid ? (MetaClass*)_element_meta_class : 0;
}

const LString& BaseListIterator::current_name() const {
  return emptyLString;
}


bool BaseListIterator::is_valid() const {
  return _is_valid;
}

ListIterator::ListIterator( GenericList* lst,
			    MetaClass* element_meta_class ) :
  BaseListIterator( element_meta_class ),
  currentIndex(0),
  genericList(0)
  {
  genericList = lst;
  _element_meta_class = element_meta_class;
  _is_valid = lst->count > 0;
}

ListIterator::ListIterator(const ListIterator &other) :
  BaseListIterator( other._element_meta_class ),
  currentIndex(0),
  genericList(0)
{
  kernel_assert(false);
}
ListIterator &ListIterator::operator=(const ListIterator &) {
  kernel_assert(false);
  return(*this);
}

Iterator *ListIterator::clone() const {
    return new ListIterator(*this);
    }

Address ListIterator::current() const {
  return _is_valid ? ((Byte*)genericList->space)+_element_meta_class->get_size_of_instance()*currentIndex : 0;
}

void ListIterator::next() {
  if ( ! _is_valid ) return;
  currentIndex++;
  if ( currentIndex >= genericList->count ) _is_valid = false;
}


void ListIterator::previous() {
  if ( ! _is_valid ) return;
  currentIndex--;
  if ( currentIndex < 0 ) _is_valid = false;
}


void ListIterator::first() {
  currentIndex = 0;
  _is_valid = (genericList->count!=0);
}



void ListIterator::set_to( size_t index ) {
  currentIndex = index;
  if ( (index < 0) || ( index > (size_t)genericList->count ) ) 
      _is_valid = false;
}


size_t ListIterator::length() const {
  return genericList->count;
}

void ListMetaClass::walk_referenced_meta_classes(MetaClassApplier *x) {
    (*x)(_element_meta_class);
    }




