#include "common/system_specific.h"
#include "list_wrapper.h"
#include "list_meta_class.h"
#include "cast.h"

ListWrapper::ListWrapper(Address address,
			 const ListMetaClass *mc) :
  _meta_class(mc), _address(address)
{
}

ListWrapper::ListWrapper(const ObjectWrapper &obj) :
  _meta_class(NULL), _address(0)
{
  kernel_assert_message(is_list(obj), 
			("Non-list object passed to constructor"));
  _meta_class = to<ListMetaClass>(obj.get_meta_class());
  _address = obj.get_address();
}

ListWrapper::ListWrapper(const ListWrapper &other) :
  _meta_class(other.get_meta_class()), _address(other.get_address())
{}
  
ListWrapper &ListWrapper::operator=(const ListWrapper &other) {
  _meta_class = other.get_meta_class();
  _address = other.get_address();
  return(*this);
}

Address ListWrapper::get_address() const {
  return(_address);
}
const ListMetaClass *ListWrapper::get_meta_class() const {
  return(_meta_class);
}


// This does NOT allow for iteration over all of the
// fields for a union.  just the one the tag refers to
size_t ListWrapper::get_length() const {
  if (is_null()) return(0);
  Iterator *it = _meta_class->get_iterator(get_address());
  size_t len = it->length();
  delete it;
  return(len);
  //  return(_meta_class->get_iterator(get_object()).length());
  //  if (is_kind_of<UnionMetaClass>(_meta_class)) return(1);
  //  return(_meta_class->get_field_count(get_object()));
}
ObjectWrapper ListWrapper::get_element(size_t id) const {
  size_t len = get_length();
  kernel_assert_message(id < len, ("invalid element #%d",id));
  Iterator *it = _meta_class->get_iterator(get_address());
  for (size_t i = 0; i < id; i++, it->next()) 
    {}
  ObjectWrapper obj(it->current_object());
  delete it;
  return(obj);
}      

sf_owned Iterator *ListWrapper::get_iterator() const {
  Iterator *it = _meta_class->get_iterator(get_address());
  return(it);
}      

ObjectWrapper ListWrapper::get_object() const {
  return(ObjectWrapper(get_address(), get_meta_class()));
}

bool ListWrapper::is_null() const {
  return(get_address() == NULL);
}

bool ListWrapper::is_list(const ObjectWrapper &obj) {
  const MetaClass *mc = obj.get_meta_class();
  return(is_kind_of<ListMetaClass>(mc));
}
