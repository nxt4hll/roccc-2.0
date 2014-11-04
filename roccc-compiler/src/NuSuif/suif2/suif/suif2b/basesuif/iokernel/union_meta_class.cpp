#include "common/system_specific.h"
#include "union_meta_class.h"
#include "object_stream.h"
#include "object_factory.h"
#include "helper.h"
#include "field_description.h"
#include "aggregate_wrapper.h"
#include "iokernel_messages.h"

#include "common/suif_vector.h"

// #ifdef PGI_BUILD
// #include <new>
// #else
// #include <new.h>
// #endif
#include <new>// jul modif




class UnionIterator : public Iterator {
public:
  static UnionIterator* create( Address address, UnionMetaClass* mc );

  virtual const MetaClass* current_meta_class() const;

  virtual const LString& current_name() const;
  virtual void* current() const;

  virtual bool is_valid() const;

  virtual void next();

  virtual void previous();

  virtual void first();
  virtual size_t length() const;
  virtual Iterator *clone() const;

protected:
  UnionIterator();

private:

  int _currentFieldIndex;
  bool _is_valid;
  Address _address;
  UnionMetaClass* _metaClass;
  FieldDescription* _current_field_description;
private:
  UnionIterator(const UnionIterator &other) :
    _currentFieldIndex(other._currentFieldIndex),
    _is_valid(other._is_valid),
    _address(other._address),
    _metaClass(other._metaClass),
    _current_field_description(other._current_field_description)
    {}
  UnionIterator &operator=(const UnionIterator &other) {
    _currentFieldIndex = other._currentFieldIndex;
    _is_valid = other._is_valid;
    _address = other._address;
    _metaClass = other._metaClass;
    _current_field_description =other._current_field_description;
    return(*this);
  }

};







static const LString union_meta_class_class_name("UnionMetaClass");

const LString& UnionMetaClass::get_class_name() {return union_meta_class_class_name;}

UnionMetaClass::UnionMetaClass( LString meta_class_name ) :
  AggregateMetaClass( meta_class_name ),
  _union_fields(new suif_vector<FieldDescription*>),
  _tag_offset(-1), _union_selector(0)
{
  //  _tag_offset = -1;
  //  _union_selector = 0;
  //  _union_fields = new suif_vector<FieldDescription*>;
}


UnionMetaClass::~UnionMetaClass() {
  delete_list_and_elements( _union_fields );
}

void UnionMetaClass::add_union_field( LString field_name,
                 MetaClass* meta_class,
				      size_t offset ) {
  FieldDescription* field =
    new FieldDescription(offset, meta_class, field_name);
  _union_fields->push_back( field );
}

void UnionMetaClass::set_union_selector( UnionSelector union_selector ) {
  _union_selector = union_selector;
  _tag_offset = -1;
}

void UnionMetaClass::set_offset( int offset ) {
  _tag_offset = offset;
  _union_selector = 0;
}


void UnionMetaClass::write( const ObjectWrapper &obj,
			    OutputStream* outputStream ) const {
  Address instance = obj.get_address();
  kernel_assert(obj.get_meta_class() == this);

  AggregateWrapper agg_obj(instance, this);
  AggregateMetaClass::write( obj, outputStream );

  int index = get_tag_num( instance );
  // if the tag number is not stored in a field that was already
  // written out => write out the tag number
  if ( _tag_offset == -1 ) {
    outputStream->write_unsigned_int( index );
  }

  if ( index >= 0 ) {
    FieldDescription* field_description = (*_union_fields)[ index ];
    ObjectWrapper field = field_description->build_object(agg_obj);
    outputStream->write( field, false );
  }
}


void UnionMetaClass::read( const ObjectWrapper &obj,
			   InputStream* inputStream ) const {
  Address instance = obj.get_address();
  kernel_assert(obj.get_meta_class() == this);

  AggregateWrapper agg_obj(instance, this);
  AggregateMetaClass::read( obj, inputStream );
  int index;
  if ( _tag_offset != -1 ) {
    index = *(int *) ((Byte*)instance + _tag_offset);
  } else {
    index = inputStream->read_unsigned_int();
  }
  if ( index >= 0 ) {
    FieldDescription* field_description = (*_union_fields)[ index ];
    ObjectWrapper field = field_description->build_object(agg_obj);

    inputStream->read( field, false );
  } else { // works for now as only Union is a Union of pointers
    zero_field(instance);
  }
}

void UnionMetaClass::zero_field(Address address) const {

    int i = get_size_of_instance();
    char *ptr = (char *)address;
    while (i > 0) { // could use memzero, but is this available on NT?
	*ptr = 0;
	ptr ++;
	i--;
	}
    }

void UnionMetaClass::initialize( const ObjectWrapper &obj,
				 InputStream* inputStream ) const {
  Address address = obj.get_address();
  kernel_assert(obj.get_meta_class() == this);
 
  // debugit("initializing union ",address);
  AggregateWrapper agg_obj(address, this);
  AggregateMetaClass::initialize( obj, inputStream );
  int index = get_tag_num( address );
  if (index < 0)
    zero_field(address);
  else {
    FieldDescription* field_description = (*_union_fields)[ index ];
    ObjectWrapper field = field_description->build_object(agg_obj);

    //Address instance_address = (Address)( ( (Byte*)address ) + field_description->offset );
    field.initialize(inputStream);
    //    field_description->get_meta_class()->initialize( field.get_address() ,inputStream );
    }
}

int UnionMetaClass::get_tag_num( Address instance ) const {
 int index;
 if ( _tag_offset != -1 ) {
     index = *(int*)((Byte*)instance +  _tag_offset);
  } else {
     index = _union_selector( instance );
  }
  return index;
}


void UnionMetaClass::adjust_field_offsets() {
  /*
    AggregateMetaClass::adjust_field_offsets();

    int currentOffset = _base_class ? _base_class->get_size_of_instance() : 0;
  */

#if 0
    // I see absolutely no way this code could be correct

    if ( _tag_offset == -1 ) {
      	// change the UnionMetaClass from being semantically controlled to tag controlled
        // insert an additional offset field
        MetaClass* intMC = synchronizer->get_object_factory()->lookupMetaClass("int");
        kernel_assert( intMC );
        int size = intMC -> get_size_of_instance();
        currentOffset = currentOffset + size - ( currentOffset % size );  //align
        _tag_offset = currentOffset;
        add_field_description("_offset", intMC, currentOffset );
        currentOffset += intMC -> get_size_of_instance();
        }
#endif

     // CAUTION - not corrected because we do not have any of these anymore
    kernel_assert_message(0,("This code should no longer have been used"));
#if 0
    // synchronize the offsets
    int startOffset = currentOffset;
    int endOffset = startOffset;
    suif_vector<FieldDescription*>::iterator it = _union_fields->begin(), end = _union_fields->end();
    FieldDescription* fieldDescription;
    for ( ; it != end ; it++ ) {
      	fieldDescription = *it;
      	MetaClass* currentType = fieldDescription->metaClass;
      	// adjust offset and alignment
      	int memberSize = currentType -> get_size_of_instance();
      	int offset = fieldDescription->offset;
      	if ( startOffset > offset ) offset = startOffset;
      	offset = offset + memberSize - ( offset % memberSize );
	changes |= fieldDescription->offset != currentOffset;
      	fieldDescription->offset = currentOffset;
      	fieldDescription->metaClass = currentType;
      	int currentEndOffset = offset + memberSize;
      	if ( currentEndOffset > endOffset ) endOffset = currentEndOffset;
    	}
    changes |= (get_size() != endOffset);
    set_size( endOffset );

    // don't call MetaClass::synchronize because it has already been called
    // indirectly through AggregateMetaClass::synchronize - DLM
    // MetaClass::synchronize( synchronizer );
#endif

    }




void UnionMetaClass::constructor_function( Address address ) {
  new (address) UnionMetaClass();
}


const MetaClass* UnionMetaClass::get_meta_class( Address address ) const {
  return (MetaClass*)this;
}

void UnionMetaClass::set_meta_class_of_object( Address instance ) const {
  //  ((Object*)instance)->set_meta_class( this );
}


Iterator* UnionMetaClass::get_local_iterator( Address instance ) const {
  return UnionIterator::create( instance, (UnionMetaClass*)this );
}





UnionIterator::UnionIterator() :
  _currentFieldIndex(0),
  _is_valid(false),
  _address(0),
  _metaClass(0),
  _current_field_description(0)
{
}


UnionIterator* UnionIterator::create( Address address, UnionMetaClass* metaClass ) {
   UnionIterator* it = new UnionIterator;
   it->_address = address;
   it->_metaClass = metaClass;
   it->first();
   return it;
}


const MetaClass* UnionIterator::current_meta_class() const {
  return  _is_valid ? _current_field_description->get_meta_class() : 0;
}


const LString& UnionIterator::current_name() const {
  return _is_valid ? _current_field_description->get_member_name() : emptyLString;
}


Address UnionIterator::current() const {
  if (!_is_valid) return(0);
  AggregateWrapper agg_obj(_address, _metaClass);
  ObjectWrapper obj = _current_field_description->build_object(agg_obj);
  return(obj.get_address());
  //  return _is_valid ?  (((Byte*) _address)+_current_field_description->offset)
  //                  : 0;
}


bool UnionIterator::is_valid() const {
  return _is_valid;
}


void UnionIterator::next() {
  if ( !_is_valid ) return;
  _currentFieldIndex++;
  int num_aggregate_fields = _metaClass->_fields->size();
  if ( _currentFieldIndex < num_aggregate_fields ) {
    _current_field_description = (*_metaClass->_fields)[ _currentFieldIndex ];
  } else if ( _currentFieldIndex == num_aggregate_fields ) {
     // the union
     int tag = _metaClass->get_tag_num( _address );
     if ( tag>=0 ) {
       _current_field_description = (*_metaClass->_union_fields)[ tag ];
     } else {
       _is_valid = false;
     }
  } else {
    _is_valid = false;
  }
}


void UnionIterator::previous() {
  if ( !_is_valid ) return;
  assert( 0 ); // not implemented
}


void UnionIterator::first() {
  _currentFieldIndex = 0;
  _is_valid =  ( _metaClass->_fields->size() > 0 );
  if ( _is_valid ) {
    _current_field_description = (*_metaClass->_fields)[ _currentFieldIndex ];
  } else {
    int tag = _metaClass->get_tag_num( _address );
    _is_valid = ( tag >= 0 );
    if ( _is_valid ) {
      _current_field_description = (*_metaClass->_union_fields)[ tag ];
    }
  }
}


size_t UnionIterator::length() const {
  int tag = _metaClass->get_tag_num( _address );

  return _metaClass->_fields->size() + (( tag>=0 ) ? 1 : 0);
}

Iterator *UnionIterator::clone() const {
    return new UnionIterator(*this);
    }


UnionMetaClass::UnionMetaClass(const UnionMetaClass &) :
  _union_fields(0), _tag_offset(0), _union_selector(0)
{
  kernel_assert(false);
}
UnionMetaClass &UnionMetaClass::operator=(const UnionMetaClass &) {
  kernel_assert(false);
  return(*this);
}

Walker::ApplyStatus UnionMetaClass::walk(const Address instance,Walker &walk) const {
  AggregateWrapper agg_obj(instance, this);
  AggregateMetaClass::walk_fields(instance,walk);
  int index = get_tag_num( instance );
  if ( index == -1 ) {
    return Walker::Continue;
    }
  FieldDescription* field_description = (*_union_fields)[ index ];
  ObjectWrapper obj = field_description->build_object(agg_obj);
  return field_description->get_meta_class()->walk(obj.get_address(), walk);
  }

size_t UnionMetaClass::get_proper_field_count(const Address address) const {
  int index = get_tag_num( address );
  if ( index >= 0 )
    return(1);
  return(0); // -1 is used to mark an empty union
}

FieldDescription* UnionMetaClass::
get_proper_field_description(const Address address,
			     size_t i) const {
  // This used to be an INT.  Just to make sure no one really
  // tried to get the -1 field assert here.
  kernel_assert(i < (unsigned)-10);
  //    if (i < 0)
  //        return NULL;

    int index = get_tag_num( address );
    if ( index > 0 ) {
	if (i == 0) {
            FieldDescription* field_description = (*_union_fields)[ index ];
    	    return field_description;
	    }
	i--;
	}
    return(NULL);
    }



FieldDescription* UnionMetaClass::get_proper_field_description(const Address address,const LString& field_name ) const
    {
    int index = get_tag_num( address );
    if ((index >= 0) && ((*_union_fields)[ index ]->get_member_name() == field_name))
	return (*_union_fields)[ index ];
    return(NULL);
    }

void UnionMetaClass::walk_referenced_meta_classes(MetaClassApplier *x) {
    AggregateMetaClass::walk_referenced_meta_classes(x);
    suif_vector<FieldDescription*>::iterator it = _union_fields->begin(), end = _union_fields->end();
    FieldDescription* fieldDescription;
    for ( ; it != end ; it++ ) {
        fieldDescription = *it;
        const MetaClass* currentType = fieldDescription->get_meta_class();
	(*x)(const_cast<MetaClass*>(currentType));
        }
    }
