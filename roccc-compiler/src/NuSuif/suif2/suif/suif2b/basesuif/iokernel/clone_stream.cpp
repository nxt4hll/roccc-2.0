#include "common/system_specific.h"
#include "clone_stream.h"

#include "meta_class.h"
#include "object_factory.h"
#include "pointer_meta_class.h"
#include "pointer_wrapper.h"
#include "iokernel_forwarders.h"

#include "common/suif_list.h"
#include "common/suif_hash_map.h"

#include <stdio.h>
// #ifndef MSVC
// #include <strstream.h>
// #else
// #include <strstrea.h>
// #endif
#include <sstream>
using namespace std;

class CloneStreamObjectInstance {
  CloneStreamObjectInstance();
  Address _old_address;
  Address _new_address;
  const MetaClass* _meta_class;
  PTR_HANDLING _ref_kind;	// how to handle this object
  bool _is_orphan;
  bool _is_written;
  bool _already_visited;
  PTR_TYPE _ptr_type;

public:
  CloneStreamObjectInstance( void* a, const MetaClass* m );
  CloneStreamObjectInstance( const CloneStreamObjectInstance &x);

  Address get_old_address() {return _old_address;}
  Address get_new_address() {return _new_address;}
  const MetaClass *get_meta_class() {return _meta_class;}
  PTR_HANDLING get_ref_kind() {return _ref_kind;}

  void set_new_address(Address addr) {
	_new_address = addr;
	}

  void set_ref_kind(PTR_HANDLING ref_kind) {
	_ref_kind = ref_kind;
	}

  void advance_ref_kind(PTR_HANDLING ref_kind) {
        if( ref_kind > _ref_kind) {
	    _ref_kind = ref_kind;
	    if (ref_kind == CLONE_PTR)
		set_new_address(0);
	    else if (ref_kind == COPY_PTR)
		set_new_address(get_old_address());
	    }
        }

  void clear_is_orphan() {
	_is_orphan = false;
	}

  void set_is_written() {
 	_is_written = true;
	}

  bool get_is_written() {
	return _is_written;
	}

  bool get_is_orphan() {
	return _is_orphan;
	}

  bool get_is_visited() {
	return _already_visited;
	}

  void set_is_visited() {
	_already_visited = true;
	}

  void advance_ptr_type(PTR_TYPE t) {
	if (_ptr_type < t)
	     _ptr_type = t;
	}

  PTR_TYPE get_ptr_type() {
	return _ptr_type;
	}

private:
  CloneStreamObjectInstance &operator=( const CloneStreamObjectInstance &x);
};

CloneStreamObjectInstance::CloneStreamObjectInstance( const CloneStreamObjectInstance &x) :
    _old_address(x._old_address),_new_address(x._new_address),
    _meta_class(x._meta_class),_ref_kind(x._ref_kind),_is_orphan(x._is_orphan),
    _is_written(false),_already_visited(false),_ptr_type(x._ptr_type)
    { }

CloneStreamObjectInstance &CloneStreamObjectInstance::operator=
	( const CloneStreamObjectInstance &x) {
      _new_address = x._new_address;
      _old_address = x._old_address;
      _meta_class = x._meta_class;
      _ref_kind = x._ref_kind;
      _is_orphan = x._is_orphan;
      _is_written = x._is_written;
      _already_visited = x._already_visited;
      _ptr_type = _ptr_type;
      kernel_assert(false);
      return(*this);
    }

CloneStreamObjectInstance::CloneStreamObjectInstance() :
  _old_address(0), _new_address(0), _meta_class(0), _ref_kind(ENQUIRE_PTR),
  _is_orphan(true),_is_written(false),_already_visited(false),_ptr_type(NO_PTR)
{
}

CloneStreamObjectInstance::CloneStreamObjectInstance( void* a, const MetaClass* m ) :
  _old_address(a), _new_address(0),_meta_class( m ),_ref_kind(ENQUIRE_PTR),
  _is_orphan(true),_is_written(false),_already_visited(false),_ptr_type(NO_PTR
) {
}

CloneStream::CloneStream( ObjectFactory* of ) :
  InputStream( of ),
  _clone_table(0),
  _work_list(0),
  _object_buffer(0),
  _refed_objects(ENQUIRE_PTR),
  _defined_objects(ENQUIRE_PTR),
  _owned_objects(ENQUIRE_PTR),
  _is_open(false),
  _of(of)
{}


CloneStream::~CloneStream() {
    if(_is_open)
	close();
    }


Object* CloneStream::clone( const Object* object ) {
    Object* o = (Object*)clone( (Address)object, object->get_meta_class() );
    return o;
    }

CloneStreamObjectInstance *CloneStream::find_or_add_object(Address addr,
							   const MetaClass *meta) {
    suif_hash_map<Address,CloneStreamObjectInstance *>::iterator iter = _clone_table->find(addr);
    if (iter != _clone_table->end())
	return (*iter).second;
    CloneStreamObjectInstance *inst = 
      new CloneStreamObjectInstance(addr,meta);
    //    (*_clone_table)[addr] = inst;
    _clone_table->enter_value(addr, inst);
    _work_list->push_back(inst);
    kernel_assert(((addr != 0) && (meta != 0)));
    return inst;
    }

void CloneStream::open() {
    if (!_is_open) {
        _clone_table = new suif_hash_map<Address,CloneStreamObjectInstance *>;
        _work_list = new suif_vector<CloneStreamObjectInstance *>;
        _object_buffer = new stringstream;
	}
    _is_open = true;
    }

bool CloneStream::get_is_open() const {
    return _is_open;
    }

void CloneStream::push_for_clone(const Object *object) {
    push_for_clone((Address)object, object->get_meta_class());
    }

void CloneStream::push_for_clone(Address instance,
				 const MetaClass* meta_class_of_instance,
				 Address target) {
    CloneStreamObjectInstance *ob=find_or_add_object(
				instance,
				meta_class_of_instance);
    ob->set_new_address(target);
    ob->set_ref_kind(CLONE_PTR);
    ob->clear_is_orphan(); // do not treat explicitly pushed objects as
			   // orphans
    }

void CloneStream::push_for_replacement(Object *old_obj,Object *new_obj) {
    const MetaClass* meta_class_of_instance = old_obj->get_meta_class();
    CloneStreamObjectInstance *ob=find_or_add_object(
                                (Address)old_obj,
                                meta_class_of_instance);
    ob->set_new_address(new_obj);
    ob->set_ref_kind(COPY_PTR);
    }


Address CloneStream::clone( 
	Address instance, 
	const MetaClass* metaClassOfInstance, 
	Address target ) {

    //	First, build a list of objects to clone and write them out to the buffer
    //  (We should be able to get rid of the write to buffer step eventually)

    open();
    push_for_clone(instance,metaClassOfInstance,target);
    perform_cloning();
    Address addr = (*_work_list)[0]->get_new_address();
    close();
    return addr;
    }
   
void CloneStream::perform_cloning() { 
    size_t total_size = 0; // memory required for cloned objects
    size_t pos = 0;

    // start by writing out all objects. This can result in
    // more objects being added to clone list.
    // It is possible that objects already seen change state from 
    // something else to clone so we need to process the list until
    // nothing changes

    bool changed = true;
    while (changed) {
	pos = 0;
	changed = false;
    	while (pos < _work_list->size()) {
	    CloneStreamObjectInstance *next = (*_work_list)[pos];
	    if (next->get_ref_kind() == ENQUIRE_PTR) {
		object_enquiry((Object *)next->get_old_address(),next,
				next->get_ptr_type());
		kernel_assert(next->get_ref_kind() != ENQUIRE_PTR);
		}
	    if ((next->get_ref_kind() == CLONE_PTR) && (!next->get_is_written())) {
	    	const MetaClass *meta = next->get_meta_class();
		next->set_is_written();
	    	meta->write(ObjectWrapper(next->get_old_address(), meta),
			    this);
	    	if (next->get_new_address() == 0)
	            total_size += meta->get_size_of_instance();
		changed = true;
		}
	    pos ++;
	    }
	}

    // allocate memory in one fowl swoop (can fowl swoop?)
    // and set the addresses for all the objects

    // Byte * addr = (Byte *)::operator new(total_size);
    pos = 0;
    while (pos < _work_list->size()) {
        CloneStreamObjectInstance *next = (*_work_list)[pos];
	if ((next->get_ref_kind() == CLONE_PTR) 
	    && (next->get_new_address() == 0)) {
	    const MetaClass *meta = next->get_meta_class();
	    Byte * addr = (Byte *)::operator new(meta->get_size_of_instance());
	    // meta->debugit("new size ",meta->get_size_of_instance());
	    // meta->debugit("cloning object at ",next->get_old_address());
	    // meta->debugit("new address ",addr);
	    next->set_new_address(addr);
	    //	    (*_clone_table)[addr] = next;
	    _clone_table->enter_value(addr, next);
	    _of->create_empty_object_in_space(const_cast<MetaClass *>(meta),addr);
	    // addr += meta->get_size_of_instance();
  	    }
	else if (next->get_new_address() != 0) {
	    // (*_clone_table)[next->get_new_address()] = next;
	    _clone_table->enter_value(next->get_new_address(), next);
	    }
	    
        pos ++;
        }

    //  Now read all the objects back in. 

    pos = 0;
    while (pos < _work_list->size()) {
        CloneStreamObjectInstance *next = (*_work_list)[pos];
	if (next->get_is_written()) {
	   const MetaClass *meta = next->get_meta_class();
	    Address addr  = next->get_new_address();

	    // has the meta class been moved
	    const MetaClass *new_meta = 
	      (const MetaClass *)clone_address((Address)meta);
	    if (new_meta)
		meta = new_meta;
            meta->read(ObjectWrapper(addr, meta),
		       this);
	    if (!next->get_is_visited()) {
	        ObjectWrapper root(addr, meta);
	        // meta->debugit("pushing object",addr);
	        _root_objects->push_back( root );
	 	}
	    // meta->initialize(addr,this);
	    }
        pos ++;
        }

    // Finally, call the routine for orphans

    pos = 0;
    while (pos < _work_list->size()) {
        CloneStreamObjectInstance *next = (*_work_list)[pos];
        if ((next->get_is_orphan()) && (next->get_is_written()))
	    finish_orphan_object_cloning((Object *)next->get_old_address(),
					 (Object *)next->get_new_address());
        pos ++;
        }
    }

CloneStreamObjectInstance * CloneStream::write_ptr(
			PTR_HANDLING handling,
			PTR_TYPE ptr_type,
			const PointerWrapper &ptr_obj) {
    Address address = ptr_obj.get_address();
    const PointerMetaClass* meta_class_of_pointer = ptr_obj.get_meta_class();

    Address addressOfObject =  *(Address*)address;
    CloneStreamObjectInstance *ob = NULL;

    if (addressOfObject != 0) {

        ob=find_or_add_object(addressOfObject,
                               meta_class_of_pointer->get_base_type()->
                                    get_meta_class( addressOfObject ));
        ob->advance_ref_kind(handling);
	ob->advance_ptr_type(ptr_type);
	}
    write_byte_array((unsigned char *)address,sizeof(Address));
    return ob;
    }

void CloneStream::write_static_pointer(const PointerWrapper &ptr_obj) {
    CloneStreamObjectInstance *ob = write_ptr(CLONE_PTR,OWN_PTR,ptr_obj);
    ob -> set_is_visited();
    }

void CloneStream::write_defining_pointer( const PointerWrapper &ptr_obj ) {
    write_ptr(get_defined_object_handling(),DEFINE_PTR,ptr_obj);
    }

void CloneStream::write_owning_pointer( const PointerWrapper &ptr_obj ) {
    CloneStreamObjectInstance *ob =write_ptr(get_owned_object_handling(),
					     OWN_PTR,
					     ptr_obj);
    if (ob)ob->clear_is_orphan();
    }

void CloneStream::write_reference( const PointerWrapper &ptr_obj ) {
    write_ptr(get_refed_object_handling(),REF_PTR,ptr_obj);
    }

#if 0
static void put_debug_info(int dir,Byte b) {
    static int last_dir = -1;
    static int line_pos = 0;
    if ((last_dir != dir) && (last_dir >= 0)) {
	printf("\n");
	}
    if (last_dir != dir) {
	last_dir = dir;
	if (dir == 1)
	    printf("wrote ");
	else 
	    printf("read  ");
	line_pos = 7;
	}
    if (line_pos > 70) {
	printf("\n      ");
	line_pos = 7;
	}
    printf("%02X ",b);
    line_pos += 3;
    }
#endif

void CloneStream::write_byte( Byte b ) {
    // put_debug_info(1,b);
    // work around horrible bug in gcc 2.8.1 runtimes
    // trying to input ff causes eof flag to get set
    if ((b == 0xff) || (b == 0xfe)) {
	_object_buffer->put((Byte)0xfe);
	b --;
	}
    _object_buffer->put(b);
    kernel_assert(!_object_buffer->bad());
    }

void CloneStream::finish_orphan_object_cloning(
	Object *old_object,
	Object *new_object) {
    // default does nothing
    }


Byte CloneStream::read_byte() {
  kernel_assert(!_object_buffer->eof());
  //Byte b = _object_buffer->get();
  Byte b = _object_buffer->get();// jul modif
  // work around horrible bug in gcc 2.8.1 runtimes
  if (b == 0xfe) {
	b  = _object_buffer->get();
	b++;
	}
  // put_debug_info(0,b);
  //cerr<<"RB "<<(int)b<<endl;
  return b;
}

void CloneStream::read_ptr( const PointerWrapper &ptr_obj ) {
    Address addressOfPointer = ptr_obj.get_address();
    //const PointerMetaClass* metaClassOfRealObject = ptr_obj.get_meta_class();

    void ** address = (void **)addressOfPointer;
    void *old_value;
    read_byte_array((unsigned char *)(&old_value),sizeof(old_value));
    if (!old_value) {
        *address = 0;
	return;
        }

    suif_hash_map<Address,CloneStreamObjectInstance *>::iterator iter = 
		_clone_table->find(old_value);

    if (iter == _clone_table->end()) {
	*address = old_value;
	return;
	}
	
    CloneStreamObjectInstance *ob = (*iter).second;

    switch (ob->get_ref_kind())
        {
	case ENQUIRE_PTR:
	    kernel_assert(false);
	    break;
        case NULL_PTR:
	    *address = 0;
	    break;
        case COPY_PTR:
	case CLONE_PTR:
	        *address = ob->get_new_address();
	    break;
        }
    }

void CloneStream::read_static_pointer( const PointerWrapper &ptr_obj ) {
    read_ptr( ptr_obj );
    }

void CloneStream::read_defining_pointer( const PointerWrapper &ptr_obj ) {
    read_ptr( ptr_obj );
    }

void CloneStream::read_reference( const PointerWrapper &ptr_obj ) {
    read_ptr( ptr_obj );
    }

void CloneStream::read_owning_pointer( const PointerWrapper &ptr_obj ) {
    read_ptr( ptr_obj );
    }

void CloneStream::close() {
    if(!_is_open)
	return;
    read_close();
    _is_open = false;
    size_t pos = 0;
    while (pos < _work_list->size()) {
        CloneStreamObjectInstance *next = (*_work_list)[pos];
        delete next;
        pos ++;
        }

    delete _work_list;
    _work_list = NULL;
    delete _clone_table;
    _clone_table = NULL;
    delete _object_buffer;
    _object_buffer = NULL;
    }

void CloneStream::set_pointer_handling( PTR_HANDLING refed_objects,
                         PTR_HANDLING defined_objects,
                         PTR_HANDLING owned_objects) {
    _refed_objects = refed_objects;
    _defined_objects = defined_objects;
    _owned_objects = owned_objects;
    }

void CloneStream::set_refed_object_handling(PTR_HANDLING handling) {
    _refed_objects = handling;
    }

PTR_HANDLING CloneStream::get_refed_object_handling() const {
    return _refed_objects;
    }

void CloneStream::set_defined_object_handling(PTR_HANDLING handling) {
    _defined_objects = handling;
    }

PTR_HANDLING CloneStream::get_defined_object_handling() const {
    return _defined_objects;
    }

void CloneStream::set_owned_object_handling(PTR_HANDLING handling) {
    _owned_objects = handling;
    }

void CloneStream::set_deep_clone() {
    set_pointer_handling(COPY_PTR,CLONE_PTR,CLONE_PTR);
    }

void CloneStream::set_shallow_clone() {
    set_pointer_handling(NULL_PTR,NULL_PTR,NULL_PTR);
    }

PTR_HANDLING CloneStream::get_owned_object_handling() const {
    return _owned_objects;
    }

void CloneStream::object_enquiry(Object *x,CloneStreamObjectInstance *o,
				 PTR_TYPE ptr_type) {
    set_reference(0);
    }

void CloneStream::set_clone_object(CloneStreamObjectInstance *o) {
    o->set_ref_kind(CLONE_PTR);
    o->set_new_address(0);
    }

// Reference the object
void CloneStream::set_reference(CloneStreamObjectInstance *o) {
    o->set_ref_kind(COPY_PTR);
    o->set_new_address(o->get_old_address());
    }

void CloneStream::set_replacement(CloneStreamObjectInstance *o,Object *x) {
    o->set_ref_kind(COPY_PTR);
    o->set_new_address(x);
    
    }

bool CloneStream::is_cloned(Address addr) const {
    suif_hash_map<Address,CloneStreamObjectInstance *>::iterator iter = _clone_table->find(addr);
    if (iter != _clone_table->end())
        return !((*iter).second->get_is_written());
    return false;
    }

  // find address of clone. Returns NULL if not cloned
Address CloneStream::clone_address(Address addr) const {
    suif_hash_map<Address,CloneStreamObjectInstance *>::iterator iter = _clone_table->find(addr);
    if (iter == _clone_table->end())
	return 0;
    if ((*iter).second->get_old_address() != addr)
	// key must be new address
	return 0;
    return (*iter).second->get_new_address();
    }

bool CloneStream::get_is_cloned(Address addr) const {
    suif_hash_map<Address,CloneStreamObjectInstance *>::iterator iter = _clone_table->find(addr);
    if (iter == _clone_table->end())
        return false;
    return ((*iter).second->get_ref_kind() == CLONE_PTR);
    }

  // how many objects were cloned?
int CloneStream::get_clone_count() const {
    return _work_list->size();
    }

  // get information about nth cloned object

Address CloneStream::get_old_address(int n) const {
    return (*_work_list)[n]->get_old_address();
    }

Address CloneStream::get_new_address(int n) const {
    return (*_work_list)[n]->get_new_address();
    }

const MetaClass *CloneStream::get_meta_class(int n) const {
    return (*_work_list)[n]->get_meta_class();
    }

bool CloneStream::get_is_cloned(int n) const {
    return ((*_work_list)[n]->get_ref_kind() == CLONE_PTR);
    }

bool CloneStream::get_is_orphan(int n) const {
    return (*_work_list)[n]->get_is_orphan();
    }

void CloneStream::set_already_visited( Address addr ) {
   suif_hash_map<Address,CloneStreamObjectInstance *>::iterator iter = _clone_table->find(addr);
    if (iter == _clone_table->end())
        return;  // don't visit non-cloned objects
    (*iter).second->set_is_visited();
    }

bool CloneStream::was_already_visited( Address addr ) const {
   suif_hash_map<Address,CloneStreamObjectInstance *>::iterator iter = _clone_table->find(addr);
    if (iter == _clone_table->end())
        return true;  // don't visit non-cloned objects
    return (*iter).second->get_is_visited();
    }





