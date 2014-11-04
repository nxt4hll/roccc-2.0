#include "common/system_specific.h"
#include "aggregate_meta_class.h"
#include "object_stream.h"
#include "virtual_iterator.h"
#include "object_factory.h"
#include "field_description.h"
#include "aggregate_wrapper.h"

#include "common/suif_map.h"
#include "common/suif_vector.h"
#include "helper.h"
#include "cast.h"
#include <new>

#ifdef DEBUG
// #include <iostream.h>
#include <iostream> //jul modif
using namespace std;//jul modif

static int debug_indent = 0;
const char * indent_text() {
    static char text[40];
    static int last_pos = 0;
    int i = debug_indent;
    if (i > 38)
	i = 38;
    while (last_pos < i)
	text[last_pos ++] = ' ';
    text[i] = 0;
    last_pos = i;
    return text;
    }

void inc_indent() {
    debug_indent += 2;
    }

void dec_indent() {
    debug_indent -= 2;
    if (debug_indent < 0)
	debug_indent = 0;
    }
#endif


/**
 * AggregateIterator will iterate over all of the declared fields
 * of an aggregate object.
 * For union objects, each of the possible fields will be
 * iterated over.
 */

class AggregateIterator : public Iterator {
public:
  static AggregateIterator* create( const AggregateWrapper &object );

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
  AggregateIterator();

private:

  int _currentFieldIndex;
  bool _is_valid;
  AggregateWrapper _object;
private:
  AggregateIterator(const AggregateIterator &other) :
    _currentFieldIndex(other._currentFieldIndex),
    _is_valid(other._is_valid),
    _object(other._object)
    {}
  AggregateIterator &operator=(const AggregateIterator &) {
    kernel_assert(false); return(*this); }
};


class NewAggregateIterator : public Iterator {
public:
  NewAggregateIterator();
  virtual ~NewAggregateIterator();

  static NewAggregateIterator* create( 
			const AggregateWrapper &object,
                        AggregateMetaClassVector* base_types,
                        const MetaClass* start_meta_class,
                        const MetaClass* end_meta_class );


  virtual const MetaClass* current_meta_class() const;

  virtual const LString& current_name() const;

  virtual void* current() const;

  virtual bool is_valid() const;

  virtual void next();

  virtual void previous();

  virtual void first();

  virtual Iterator *clone() const;

private:
  AggregateWrapper _object;
  const AggregateMetaClass* _start_meta_class;
  AggregateMetaClassVector* _base_types;
  int _start_offset;
  int _end_offset;
  int _current_class_index;
  int _current_field_index;
  bool _is_valid;
  Iterator* _current_iterator;
private:
  // this constructor is only needed for cloning
  NewAggregateIterator(const NewAggregateIterator &other)
 :
    _object(other._object),
    _start_meta_class(other._start_meta_class),
    _base_types(other._base_types),
    _start_offset(other._start_offset),
    _end_offset(other._end_offset),
    _current_class_index(other._current_class_index),
    _current_field_index(other._current_field_index),
    _is_valid(other._is_valid),
    _current_iterator(other._current_iterator->clone())
    {}

   // no implementation for this declarations!
  NewAggregateIterator &operator=(const NewAggregateIterator &);
};

Iterator *NewAggregateIterator::clone() const {
    NewAggregateIterator* iter = new NewAggregateIterator(*this);
    return iter;
    }


static const LString aggregate_meta_class_class_name("AggregateMetaClass");

const LString &AggregateMetaClass::get_class_name() {
  return aggregate_meta_class_class_name;
}



static const LString object_aggregate_meta_class_class_name("ObjectAggregateMetaClass");

const LString &ObjectAggregateMetaClass::get_class_name()
    {
    return object_aggregate_meta_class_class_name;
    }



AggregateMetaClass::AggregateMetaClass( LString metaClassName ) :
  MetaClass( metaClassName ),
  _base_class( 0 ),
  _fields( 0 ),
  _virtual_nodes( 0 ), // initialization is delayed (init_virtual_nodes )
  _virtual_field_description( 0 ),
  _base_types( 0 ),
  _number_of_base_classes(0),
  _destructor_function( 0 ) {
  _fields = new suif_vector<FieldDescription*>;
  _virtual_field_description = new VirtualFieldDescription;
}


AggregateMetaClass::~AggregateMetaClass() {
  delete_list_and_elements( _fields );
  delete_map_and_value( _virtual_nodes );
  delete _virtual_field_description;
  _virtual_field_description = 0;
  delete _base_types;
}

FieldDescription* AggregateMetaClass::add_field_description( LString fieldName, MetaClass* metaClass, size_t offset ) {
     FieldDescription* field =
       new FieldDescription(offset, metaClass, fieldName);
     _fields->push_back( field );
     size_t align = metaClass->get_alignment_of_instance();
     if (align > get_alignment_of_instance())
	set_alignment(align);
     return field;
}


FieldDescription* AggregateMetaClass::
get_field_description( const LString& field_name ) const 
{
  for (FieldDescriptionList::iterator current = _fields->begin(),
	 end = _fields->end();
       current != end ; current++ ) {
    if ( (*current)->get_member_name() == field_name ) {
      return (*current);
    }
  }
  if ( _base_class )
    return _base_class->get_field_description( field_name );

  return(0);
}


size_t AggregateMetaClass::get_field_count(const Address address) const {
  size_t fields = get_proper_field_count(address);
  if ( _base_class)
    fields += _base_class->get_field_count(address);
  return fields;
}

FieldDescription* AggregateMetaClass::
get_field_description(const Address address, size_t i) const {
  size_t field_count = get_proper_field_count(address);
  if (i < field_count)
    return(get_proper_field_description(address, i));

  if (_base_class)
    return _base_class->get_field_description(address,i - field_count);
  return NULL;
}

FieldDescription* AggregateMetaClass::
get_field_description( const Address address,
		       const LString& field_name ) const {
  FieldDescription *fd = get_proper_field_description(address, field_name);
  if (fd) return(fd);
  if ( _base_class ) {
     return _base_class->get_field_description( address, field_name );
  } else {
    return 0;
  }
}


size_t AggregateMetaClass::get_proper_field_count(const Address address) const
{
  //  kernel_assert(!is_kind_of<UnionMetaClass>(this));
  return(_fields->size());
}

FieldDescription* AggregateMetaClass::
get_proper_field_description(const Address address, size_t i) const {
  //  kernel_assert(!is_kind_of<UnionMetaClass>(this));
  size_t field_count = get_proper_field_count(address);
  if (i < field_count)
    return((*_fields)[i]);
  return(NULL);
}

FieldDescription* AggregateMetaClass::
get_proper_field_description( const Address address,
			      const LString& field_name ) const {
  for (FieldDescriptionList::iterator current = _fields->begin(),
	 end = _fields->end();
       current != end ; current++ ) {
    if ( (*current)->get_member_name() == field_name ) {
      return (*current);
    }
  }
  return(0);
}


void AggregateMetaClass::inherits_from( AggregateMetaClass* metaClass ) {
    _base_class = metaClass;
    if (metaClass) {
    	int parent_alignment = metaClass->get_alignment_of_instance();
    	int alignment = get_alignment_of_instance();
    	if (alignment < parent_alignment) alignment = parent_alignment;
	}
}


void AggregateMetaClass::write( const ObjectWrapper &obj, 
                                OutputStream* outputStream ) const {
  Address instance = obj.get_address();
  kernel_assert(obj.get_meta_class() == this);

  //@@@ offset check!!
  AggregateWrapper agg_obj(instance, this);
  if ( _base_class ) 
    _base_class->write( ObjectWrapper(instance, _base_class), outputStream );
//  fprintf( stderr, "WRITING %s  %p\n", _meta_class_name.c_str(), instance );

//  Byte* address = (Byte*)instance;
  suif_vector<FieldDescription*>::iterator it = _fields->begin(), end = _fields->end();
  FieldDescription* fieldDescription;
#ifdef DEBUG
  cerr<<"START WRITING STRUCTURE "<<get_instance_name().c_str()<<endl;
  inc_indent();
#endif
  while ( it != end ) {
   fieldDescription = *it;
#ifdef DEBUG
   cerr<<"  WRITING MEMBER OF STRUCTURE "<<fieldDescription->memberName.c_str()<<" "<<fieldDescription->metaClass->get_instance_name()<<endl;
#endif
   ObjectWrapper field = fieldDescription->build_object(agg_obj);
   outputStream->write( field,
                        false );
   it++;
  }
#ifdef DEBUG
  dec_indent();
  cerr<<"END WRITING STRUCTURE "<<get_instance_name().c_str()<<endl;
#endif
}


void AggregateMetaClass::read( const ObjectWrapper &obj,
                               InputStream* inputStream ) const {
  Address instance = obj.get_address();
  kernel_assert(obj.get_meta_class() == this);
  AggregateWrapper agg_obj(instance, this);
#ifdef DEBUG
  cerr<<indent_text() << "READING STRUCTURE "<<get_instance_name().c_str()<<endl;
 inc_indent();
#endif
  if ( _base_class ) 
    _base_class->read( ObjectWrapper(instance, _base_class), inputStream );
//  Byte* address = (Byte*)instance;
  suif_vector<FieldDescription*>::iterator it = _fields->begin(), end = _fields->end();
  FieldDescription* fieldDescription;
  while ( it != end ) {
   fieldDescription = *it;
#ifdef DEBUG
 cerr<<indent_text() <<"  READING MEMBER OF STRUCTURE "<<fieldDescription->memberName.c_str()<<" "<<fieldDescription->metaClass->get_instance_name()<<endl;
#endif
   ObjectWrapper field = fieldDescription->build_object(agg_obj);
   inputStream->read( field,
                      false );
   it++;
  }
#ifdef DEBUG
  dec_indent();
  cerr << indent_text() <<"FINISHED READING OBJECT " << get_instance_name().c_str() << endl;
#endif
}


Iterator* AggregateMetaClass::get_iterator( ConstAddress instance, Iterator::Contents contents ) const {
  Iterator* return_value = 0;
  if ( contents != Iterator::Referenced ) {
    return_value = get_aggregate_iterator( instance );
  }
  return return_value;
}


void AggregateMetaClass::initialize_base_types() const {
  // create a volatile this
  AggregateMetaClass *vol_this = (AggregateMetaClass *)this;
  // build cached stuff
  if ( !_base_types ) {
    vol_this->_base_types = new AggregateMetaClassVector;
    AggregateMetaClassVector temp;
    AggregateMetaClass* current = vol_this;
    while ( current ) {
      temp.push_back( current );
      current = current -> _base_class;
    }
    for ( int i = temp.size(); i; i-- ) {
      vol_this->_base_types -> push_back( temp[ i-1 ] );
    }
    vol_this->_number_of_base_classes = _base_types->size();
  }
}


Iterator* AggregateMetaClass::get_local_iterator( Address
instance ) const {
  return AggregateIterator::create( AggregateWrapper(instance, this ));
}


Iterator* AggregateMetaClass::get_aggregate_iterator(
                         ConstAddress instance,
                         const LString& start_class,
                         const LString& end_class ) const {
  MetaClass* start_meta = 0;
  MetaClass* end_meta = 0;

  if ( start_class != emptyLString ) {
     start_meta = _owning_factory -> find_meta_class( start_class );
     }
  if ( end_class != emptyLString ) {
     end_meta = _owning_factory -> find_meta_class( end_class );
     }
  return get_aggregate_iterator( instance, start_meta, end_meta );
}


Iterator* AggregateMetaClass::get_aggregate_iterator(
                         ConstAddress instance,
                         const MetaClass* start_class,
                         const MetaClass* end_class ) const {

  initialize_base_types();
  return NewAggregateIterator::create( AggregateWrapper((void*)instance, this),
				       _base_types,
				       start_class, end_class );
}


bool AggregateMetaClass::is_elementary() const {
  return false;
}


/*
bool AggregateMetaClass::isKindOf( const LString& name ) const {
    if ( isA( name ) ) {
      return true;
    } else if ( _base_class ) {
      return _base_class->isKindOf( name );
    }
    return false;
}
*/


const MetaClass* AggregateMetaClass::get_meta_class( Address address ) const {
    return this;
  }


void AggregateMetaClass::initialize( const ObjectWrapper &obj,
                                     InputStream* inputStream ) const {
  Address address = obj.get_address(); 
  kernel_assert(obj.get_meta_class() == this);
  AggregateWrapper agg_obj(address, this);

   if ( _base_class ) 
     _base_class->initialize( ObjectWrapper(address, _base_class), 
			      inputStream );
   suif_vector<FieldDescription*>::iterator it = _fields->begin(), end = _fields->end();
   FieldDescription* fieldDescription;
   for ( ; it != end ; it++ ) {
     fieldDescription = *it;
     ObjectWrapper field = fieldDescription->build_object(agg_obj);
//     Address instanceAddress = (Address)( ( (Byte*)address ) + fieldDescription->offset );
     field.initialize( inputStream );
   }
}

String AggregateMetaClass::get_debug_text() const {
  String result;
  if (get_base_class()) {
    result += get_base_class()->get_debug_text();
  }
  
  result += MetaClass::get_debug_text();
  
  for (size_t i = 0; i < _fields->size(); i++) {
    result += " ";
    result += ((*_fields)[i])->get_debug_text();
    result += "\n";
  }
  return(result);
}


void AggregateMetaClass::adjust_field_offsets() {
    size_t current_offset = 0;
    size_t total_align = 1;
    if (_base_class) {
	current_offset = _base_class->get_size_of_instance();
	total_align = _base_class->get_alignment_of_instance();
	}

    suif_vector<FieldDescription*>::iterator it = _fields->begin(), end = _fields->end();
    FieldDescription* fieldDescription;
    for ( ; it != end ; it++ ) {
      	fieldDescription = *it;
      	const MetaClass* currentType = fieldDescription->get_meta_class();
      	// adjust offset and alignment
      	size_t memberSize = currentType -> get_size_of_instance();
	// align to next boundary
	size_t memberAlign = currentType -> get_alignment_of_instance();
	if (memberAlign > total_align)
	    total_align = memberAlign;
	int displacement = current_offset % memberAlign;
	if (displacement)
	    current_offset += memberAlign - displacement;
        if (fieldDescription->get_offset() != current_offset) {
	  if (has_constructed_object()) {
	    printf("old offset %d new offset %d for %s (%s:%d,%d) in %s\n",
		   fieldDescription->get_offset(),
		current_offset,
		(const char *)fieldDescription->get_member_name(),
		(const char *)currentType->get_instance_name(),
		memberSize,memberAlign,
	        (const char *)_meta_class_name);
	  }
	}
      	fieldDescription->set_offset(current_offset);
      	current_offset = current_offset + memberSize;
    	}
    int displacement = current_offset % total_align;
    if (displacement)
        current_offset += total_align - displacement;

    if (get_size() != current_offset) {
      if (has_constructed_object()) {
	printf("size changed from %d to %d for %s\n",
	       get_size(),current_offset,(const char *)_meta_class_name);
      }
    }
    fflush(stdout);
    set_size( current_offset );
    set_alignment(total_align);

    MetaClass::adjust_field_offsets();
    }

void AggregateMetaClass::construct_object( Address address ) const {
  MetaClass::construct_object( address );
  for ( const AggregateMetaClass* base_class = this;
        base_class && !base_class->has_its_own_constructor();
        base_class = base_class->_base_class ) {

     suif_vector<FieldDescription*>::iterator it = base_class->_fields->begin(),
                                         end = base_class->_fields->end();
     FieldDescription* fieldDescription;
     for ( ;it != end; it++ ) {
       fieldDescription = *it;
       fieldDescription->get_meta_class()->construct_object(
                     ((Byte*)address)+fieldDescription->get_offset() );
     }
  }
}

void AggregateMetaClass::constructor_function( Address address ) {
  new (address) AggregateMetaClass();
}


void AggregateMetaClass::set_destructor_function( DestructorFunction destructor_function ) {
  _destructor_function = destructor_function;
}


ConstructorFunction AggregateMetaClass::get_constructor_function() const {
  ConstructorFunction func = MetaClass::get_constructor_function();
  if ( !func ) {
   if ( _base_class ) {
     func = _base_class->get_constructor_function();
   } else {
     func = emptyConstructorFunction;
   }
   ((AggregateMetaClass*)this)->set_constructor_function( func );
  }
  return func;
}

bool AggregateMetaClass::has_its_own_constructor() const {
  ConstructorFunction current_func = get_constructor_function();
  if ( current_func == emptyConstructorFunction ) {
    return true;
  }
  if ( _base_class &&
       current_func == _base_class->get_constructor_function() ) {
    return false;
  }
  return true;
}

void AggregateMetaClass::destruct( const ObjectWrapper &obj,
				   bool called_from_destructor ) const {
  Address address = obj.get_address();
  kernel_assert(this == obj.get_meta_class());
  AggregateWrapper agg_obj(address, this);
  if ( called_from_destructor ) {
    // release any allocated memory
    for ( const AggregateMetaClass* current = this;
          current && !current->has_its_own_constructor(); // no constructor => no destructor
          current = current -> _base_class ) {
      int number_of_entries = current->_fields->size();
      for ( int i = 0; i< number_of_entries; i++ ) {
        FieldDescription* field = (*current->_fields)[ i ];
        ObjectWrapper obj = field->build_object(agg_obj);
        obj.destruct(false);
       //field->metaClass->destruct( ((Byte*)address) + field->offset, false );
      }
    }
  } else {
    call_destructor( agg_obj );
  }
}




MetaClass* AggregateMetaClass::get_link_meta_class() const {
  return _base_class;
}


void AggregateMetaClass::set_meta_class_of_object( Address instance ) const {
  //  ((Object*)instance)->set_meta_class( this );
}


void AggregateMetaClass::init_virtual_nodes() {
  assert( !_virtual_nodes );
  _virtual_nodes = new NodeMap;
  VirtualFieldDescription::iterator
              current = _virtual_field_description->begin(),
              end = _virtual_field_description->end();
   while ( current != end ) {
     add_virtual_field( (*current).first, (*current).second );
     current++;
   }
}

void AggregateMetaClass::add_virtual_field( const LString& name,
                                            const String& description ) {
  if ( !_virtual_nodes ) {
    init_virtual_nodes();
  }
  //  (*_virtual_field_description)[ name ] = description;
  _virtual_field_description->enter_value(name, description);
  VirtualNode* node = get_virtual_node( name, description );
  //(*_virtual_nodes)[ name ] = node;
  _virtual_nodes->enter_value(name, node);
}


Iterator* AggregateMetaClass::get_virtual_iterator(
       ConstAddress address,
       const LString& name ) const {
  VirtualNode* node = 0;

  for ( const AggregateMetaClass* m = this;
            m;
            m = m -> _base_class ) {
    NodeMap* virtual_nodes = m->_virtual_nodes;
    if ( !virtual_nodes ) {
	// this is a mache.  ideally, we'd mark the cache mutable.
      ((AggregateMetaClass*)m)->init_virtual_nodes();
      virtual_nodes = m->_virtual_nodes;
    }
    NodeMap::iterator iter = virtual_nodes->find( name );
    if ( iter != virtual_nodes -> end()  ) {
      node = (*iter).second;
      break;
    }
  }
  /*
  kernel_assert_message( node != 0,
			 ("Could not find field '%s' in metaClass '%s' for virtual iteration",
			  name.c_str(),
			  this->get_instance_name().c_str()));
  */

  return new VirtualIterator( address, node );
}


void AggregateMetaClass::call_destructor( const AggregateWrapper &obj) const {
  kernel_assert(obj.get_meta_class() == this);
  for ( const AggregateMetaClass* current = this;
        current;
        current = current -> _base_class ) {
    if ( current->_destructor_function ) {
      current->_destructor_function( obj.get_object() );
      break;
    }
  }
}

AggregateMetaClass* AggregateMetaClass::get_base_class() const {
    return _base_class;
    }

Walker::ApplyStatus AggregateMetaClass::walk_fields(const Address instance,Walker &w) const {
    // printf("walking aggregate at %08X %s\n",instance,
// 		(const char *)get_instance_name());
    const AggregateMetaClass *current = this;
    AggregateWrapper agg_obj(instance, current);
    while (current != NULL) {
        suif_vector<FieldDescription*>::iterator it = current->_fields->begin(),end = current->_fields->end();
        while ( it != end) {
            FieldDescription* fieldDescription = *it;
	    Address old_parent = w.get_parent();
	    w.set_parent(instance);
            ObjectWrapper obj = fieldDescription->build_object(agg_obj);
            Walker::ApplyStatus status =
		fieldDescription->get_meta_class()->walk(obj.get_address(),w);
	    w.set_parent(old_parent);
	    switch (status)
                {
                case Walker::Continue:
                    break;
                case Walker::Stop:
                case Walker::Abort:
                    return status;
                case Walker::Truncate:
                    return Walker::Continue;
                case Walker::Replaced:
		    assert(false);
                    break;
                }
            it ++;
	    }
        current = current->get_base_class();
    	}
    return Walker::Continue;
    }

Walker::ApplyStatus AggregateMetaClass::walk(const Address address,Walker &w) const {
    Address instance = address;
    Walker::ApplyStatus status = Walker::Continue;

     if (w.get_is_pre_order() && w.is_visitable(address,this))
	{

	w.set_address(0); // make sure we detect failures to set on refetch
	Walker::ApplyStatus status =w(instance,this);
	switch (status)
	    {
	    case Walker::Continue:
		break;
	    case Walker::Stop:
	    case Walker::Abort:
		return status;
	    case Walker::Truncate:
		return Walker::Continue;
	    case Walker::Replaced:
	    	instance = w.get_address();
		assert(instance);
		break;
	    }
	}

    status = walk_fields(address,w);
    switch (status)
        {
        case Walker::Continue:
            break;
        case Walker::Stop:
        case Walker::Abort:
            return status;
        case Walker::Truncate:
            return Walker::Continue;
        case Walker::Replaced:
            assert(false);
            break;
	}

    if (!w.get_is_pre_order() && w.is_visitable(address,this)) {
	return w(instance,this);
	}
    return status;
    }


ObjectAggregateMetaClass::ObjectAggregateMetaClass( LString metaClassName )
  : AggregateMetaClass( metaClassName ) {
}




const MetaClass* ObjectAggregateMetaClass::get_meta_class( Address address ) const {
  //  return this;
  const MetaClass *mc = NULL;
  if (address) {
    mc = ((Object*)address)->get_meta_class();
    kernel_assert(is_kind_of<MetaClass>(mc));
  }
  return(mc);
}


void ObjectAggregateMetaClass::set_meta_class_of_object( Address instance ) const{
  ((Object*)instance)->set_meta_class( this );
}

void ObjectAggregateMetaClass::construct_object( Address instance ) const {
  AggregateMetaClass::construct_object( instance );
  set_meta_class_of_object( instance );
}


void ObjectAggregateMetaClass::constructor_function( Address address ) {
  new (address) ObjectAggregateMetaClass();
}

void ObjectAggregateMetaClass::set_destructor_function( DestructorFunction destructor_function ) {
  kernel_assert( false ); // should never be called
}


void ObjectAggregateMetaClass::call_destructor( const AggregateWrapper &obj ) const {
  kernel_assert(obj.get_meta_class() == this);
  (obj.get_object().get_object())->~Object(); // this is a virtual call
}

Walker::ApplyStatus ObjectAggregateMetaClass::walk(const Address address,Walker &w) const
    {
    Address instance = address;
    ObjectAggregateMetaClass *mc = (ObjectAggregateMetaClass *)get_meta_class(instance);

     Walker::ApplyStatus status = Walker::Continue;

     if (w.get_is_pre_order() && w.is_visitable(address,mc))
	{
	w.set_address(0); // make sure we detect failures to set on refetch
	Walker::ApplyStatus status =w(instance,this);
	switch (status)
	    {
	    case Walker::Continue:
		break;
	    case Walker::Stop:
	    case Walker::Abort:
		return status;
	    case Walker::Truncate:
		return Walker::Continue;
	    case Walker::Replaced:
	    	instance = w.get_address();
		assert(instance);
		mc = (ObjectAggregateMetaClass *)get_meta_class(instance);
		break;
	    }
	}

    status = mc->walk_fields(instance,w);
    switch (status)
        {
        case Walker::Continue:
            break;
        case Walker::Stop:
        case Walker::Abort:
            return status;
        case Walker::Truncate:
            return Walker::Continue;
        case Walker::Replaced:
            assert(false);
            break;
	}

    if (!w.get_is_pre_order() && w.is_visitable(address,mc)) {
	return w(instance,this);
	}
    return status;
    }



AggregateIterator::AggregateIterator() :
  _currentFieldIndex(0),
  _is_valid(false),
  _object(0, 0)
{
}


AggregateIterator* AggregateIterator::create( const AggregateWrapper &object ) {
   AggregateIterator* it = new AggregateIterator;
   it->_object = object;
   it->first();
   return it;
}


const MetaClass* AggregateIterator::current_meta_class() const {
  if (!_is_valid) return 0;
  FieldDescription *fd = _object.get_proper_field_description(_currentFieldIndex);
  return(fd->get_meta_class());
}


const LString& AggregateIterator::current_name() const {
  if (!_is_valid) return emptyLString;
  FieldDescription *fd = _object.get_proper_field_description(_currentFieldIndex);
  return(fd->get_member_name());
}


Address AggregateIterator::current() const {
  if (!_is_valid) return 0;
  FieldDescription *fd = _object.get_proper_field_description(_currentFieldIndex);
  ObjectWrapper obj = fd->build_object(_object);
  return(obj.get_address());
}


bool AggregateIterator::is_valid() const {
  return _is_valid;
}


void AggregateIterator::next() {
  if ( !_is_valid ) return;
  _currentFieldIndex++;
  _is_valid = ( _currentFieldIndex < (int)_object.get_proper_field_count());
}


void AggregateIterator::previous() {
  if ( !_is_valid ) return;
  _currentFieldIndex--;
  _is_valid =  ( ( _currentFieldIndex >= 0 ) 
		 && ( _object.get_proper_field_count() > 0 ) );
}


void AggregateIterator::first() {
  _currentFieldIndex = 0;
  _is_valid =  ( _object.get_proper_field_count() > 0 );
}


size_t AggregateIterator::length() const {
  return _object.get_proper_field_count();
}

Iterator *AggregateIterator::clone() const {
    AggregateIterator* iter = new  AggregateIterator(*this);
    return iter;
    }



NewAggregateIterator::NewAggregateIterator() :
  _object(0, 0),
  _start_meta_class(0),
  _base_types(0),
  _start_offset(0),
  _end_offset(0),
  _current_class_index(0),
  _current_field_index(0),
  _is_valid(0),
  _current_iterator(0)
{
}



NewAggregateIterator::~NewAggregateIterator() {
  delete _current_iterator;
}



NewAggregateIterator* NewAggregateIterator::create(
           const AggregateWrapper &object,
           AggregateMetaClassVector* base_types,
           const MetaClass* start_class,
           const MetaClass* end_class ) {
 NewAggregateIterator* it = new NewAggregateIterator;
 it->_object = object;
 it->_base_types = base_types;
 it->_start_offset = 0;
 it->_current_iterator = 0;
 it->_end_offset =  it->_base_types->size()-1;

 int vec_size = it->_base_types->size();
 // adjust _start_offset
 if ( start_class ) {
   int i;
   for ( i = 0 ; i < vec_size; i++ ) {
     if ( start_class == (*it->_base_types)[i] ) {
       it->_start_offset = i;
       break;
     }
   }
   kernel_assert( i != vec_size );
 }
 // adjust _end_offset
 if ( end_class ) {
   int i;
   for ( i = 0 ; i < vec_size; i++ ) {
     if ( end_class == (*it->_base_types)[i] ) {
       it->_end_offset = i;
       break;
     }
   }
  kernel_assert( i != vec_size );
 }
 it->first();
 return it;
}


void NewAggregateIterator::first() {
  // check for valid address
  _is_valid = ( !_object.is_null() );
  if ( !_is_valid ) return;

  _is_valid = false;
#if 0
  // BUG!  unsigned.

  delete _current_iterator;
  _current_iterator = NULL;

  _current_class_index = _start_offset;

  for (_current_class_index = _start_offset; 
       _current_class_index <= _end_offset; _current_class_index++) {
    delete _current_iterator;
    _current_iterator = 
      (*_base_types)[_current_class_index] ->
      get_local_iterator( _object.get_address() );
 
   _is_valid = _current_iterator->is_valid();
    if (_is_valid)
      break;
  }
    
#endif
  int idx = _start_offset -1;
  //  _current_class_index = _start_offset - 1; // will be incremented in loop
  delete _current_iterator;
  _current_iterator= 0;
  while ( ( !_is_valid ) &&  ( idx < _end_offset ) ) {
    idx++;
    //    _current_class_index++;
    delete _current_iterator;
    //    _current_iterator = (*_base_types)[_current_class_index] -> get_local_iterator( _object.get_address() );
    _current_iterator = (*_base_types)[idx] -> get_local_iterator( _object.get_address() );
    _is_valid = _current_iterator -> is_valid();
   }
  _current_class_index = idx;

}



const MetaClass* NewAggregateIterator::current_meta_class() const {
   return _is_valid ? _current_iterator->current_meta_class() : 0;
}



const LString& NewAggregateIterator::current_name() const {
  return _is_valid ? _current_iterator->current_name() : emptyLString;
}


Address NewAggregateIterator::current() const {
  return _is_valid ? _current_iterator->current() : 0;
}


bool NewAggregateIterator::is_valid() const {
  return _is_valid;
}


void NewAggregateIterator::next() {
  if ( !_is_valid ) return;
  _current_iterator->next();
  _is_valid = _current_iterator->is_valid();
  if ( !_is_valid ) {
     delete _current_iterator;
     _current_iterator = 0;
     while ( ( !_is_valid ) &&  ( _current_class_index < _end_offset ) ) {
       _current_class_index++;
       delete _current_iterator;
       _current_iterator = (*_base_types)[_current_class_index]->get_local_iterator( _object.get_address() );
       _is_valid = _current_iterator -> is_valid();
     }
  }
}


void NewAggregateIterator::previous() {
  if ( !_is_valid ) return;
  assert( false );   // not implemented
}

  // walk the meta-classes referenced by a given meta class.
void AggregateMetaClass::walk_referenced_meta_classes(MetaClassApplier *x) {
    if (_base_class) {
	(*x)(_base_class);
        }

    suif_vector<FieldDescription*>::iterator it = _fields->begin(), end = _fields->end();
    FieldDescription* fieldDescription;
    for ( ; it != end ; it++ ) {
        fieldDescription = *it;
        MetaClass* currentType = 
	  const_cast<MetaClass*>(fieldDescription->get_meta_class());
	(*x)(currentType);
        }
    }







