#include "common/system_specific.h"
//#include <strings.h>
#include "common/formatted.h"
#include "smgn/macroBase.h"
#include "MacroObjAdapterBase.h"
#include "iokernel/aggregate_meta_class.h"
#include "iokernel/pointer_meta_class.h"
#include "iokernel/union_meta_class.h"
#include "iokernel/integer_meta_class.h"
#include "iokernel/i_integer_meta_class.h"
#include "iokernel/string_meta_class.h"
#include "iokernel/lstring_meta_class.h"
#include "iokernel/list_meta_class.h"
#include "iokernel/field_description.h"

#include "iokernel/cast.h"

//FormattedText x;

static int hash(const Address addr) {
    // FIXME - 64 bit
    //int i = (int)addr;
    int i = (long int)addr;
    return i + (i << 2);
    }



static const LString named_list_class_name("NamedList_MacroObjAdapter");

MacroObjectBuilder::MacroObjectBuilder() {}
MacroObjectBuilder::~MacroObjectBuilder() {
    }

bool MacroObjectKey::operator ==(const MacroObjectKey &x) const {
    return (_address == x._address) && (_id == x._id);
    }

MacroObjectKey::MacroObjectKey() : _address(0),_id(0) {}
MacroObjectKey::MacroObjectKey(Address addr,const MetaClass *meta) :
	_address(addr),_id(meta->get_meta_class_id()) {}

MacroObjectKey::MacroObjectKey(const ObjectWrapper &obj) :
	_address(obj.get_address()),_id(obj.get_meta_class()->get_meta_class_id()) {}

int MacroObjectKey::hash() const {
  return ::hash(_address) + _id;
}

MacroObjectPtr MacroObjectBuilder::build_macro_object(const ObjectWrapper &obj)
{
  return(build_macro_object(obj.get_address(), obj.get_meta_class()));
}

MacroObjectPtr MacroObjectBuilder::build_macro_object(Address address,
						    const MetaClass *meta) {
    if (!address)
        return 0;

    while (is_kind_of<PointerMetaClass>(meta)) {
	address = *(Address **)address;
  	if (!address)
            return 0;

	meta = to_const<PointerMetaClass>(meta)->get_base_type();
	}

    if (is_kind_of<AggregateMetaClass>(meta)) {
	meta = meta->get_meta_class(address);
	}

    MacroObjectKey key(address,meta);
    suif_hash_map<MacroObjectKey ,MacroObject *>::iterator iter = 
		built_ptrs.find(key);
    MacroObjectPtr obj;
    if (iter != built_ptrs.end())
	{
	obj = (*iter).second;
        return obj;
	}
    obj = build_macro_object_inner(address,meta);
    if(!obj.is_null())
	{
	built_ptrs.enter_value(key,obj);
	}
    return obj;
    }

MacroObjectPtr MacroObjectBuilder::build_macro_object_inner(const ObjectWrapper &obj) {
  return(build_macro_object_inner(obj.get_address(), obj.get_meta_class()));
}

MacroObjectPtr MacroObjectBuilder::build_macro_object_inner(Address address, const MetaClass *meta) {
    if (is_kind_of<AggregateMetaClass>(meta)) {
        NamedList_MacroObjAdapter* new_adapter =
            new NamedList_MacroObjAdapter(this);
        new_adapter->set_underlying((SuifObject *)address);
	new_adapter->set_meta(to<AggregateMetaClass>(meta));
        return new_adapter;
        }

    static LString IInteger_id("IInteger");
    if (meta->get_instance_name() == IInteger_id) {
        IInteger *iint = (IInteger *)address;
        String str;
        iint->write(str,10);
        StringMacroObject_MacroObjAdapter * obj = new StringMacroObject_MacroObjAdapter(str);
	obj->set_meta(meta);
        return obj;
        }

    if (is_kind_of<ListMetaClass>(meta)) {
        MacroListObject_MacroObjAdapter *list = new MacroListObject_MacroObjAdapter(this);
        list->set_underlying(address);
        list->set_meta(to<ListMetaClass>(meta));
        return list;
        }

    static LString bool_id("bool");
    if (meta->get_instance_name() == bool_id) {
	bool *i = (bool *)address;
	String str;
	if (*i)
	    str="true";
	else
	    str = "false";
        StringMacroObject_MacroObjAdapter * obj = new StringMacroObject_MacroObjAdapter(str);
        obj->set_meta(meta);
        return obj;
        }


    static LString String_id("String");

    if (meta->get_instance_name() == String_id) {
        String *str = (String *)address;
        StringMacroObject_MacroObjAdapter * obj = new StringMacroObject_MacroObjAdapter(*str);
	obj->set_meta(meta);
        return obj;
        }

    static LString LString_Id("LString");
    if (meta->get_instance_name() == LString_Id) {
        LString *str = (LString *)address;
        StringMacroObject_MacroObjAdapter * obj = new StringMacroObject_MacroObjAdapter(*str);
	obj->set_meta(meta);
        return obj;
        }

    static LString int_id("int");
    if (meta->get_instance_name() == int_id) {
        int *the_int = (int *)address;
        String str(*the_int);
        StringMacroObject_MacroObjAdapter * obj = new StringMacroObject_MacroObjAdapter(str);
   	obj->set_meta(meta);
        return obj;
        }

    static LString double_id("double");
    if (meta->get_instance_name() == double_id) {
        double *the_double = (double *)address;
        String str(*the_double);
        StringMacroObject_MacroObjAdapter * obj = new StringMacroObject_MacroObjAdapter(str);
        obj->set_meta(meta);
        return obj;
        }


    assert(0);
    return 0;
    }

NamedList_MacroObjAdapter::NamedList_MacroObjAdapter(MacroObjectBuilder *tbuilder) 
    : addedObjects(0),builder(tbuilder) {}

NamedList_MacroObjAdapter::~NamedList_MacroObjAdapter() {
    addedObjects.make_null();
    }

LString NamedList_MacroObjAdapter::object_type_name() const
{
    return _meta->get_instance_name();
}

LString NamedList_MacroObjAdapter::get_ClassName()
{
    return named_list_class_name;
}

bool NamedList_MacroObjAdapter::isKindOf( const LString &kind ) const 
{
    return ((kind == named_list_class_name) || AbstractNamedList::isKindOf(kind));
}

bool NamedList_MacroObjAdapter::is_instance_of(const LString &kind ) const {
    MetaClass *meta = _meta;
    //    LString str(kind);
    while (meta != 0) {
	if (meta->get_instance_name() == kind)
	    return true;
	meta = meta->get_link_meta_class();
	}
    return false;
    }

int NamedList_MacroObjAdapter::length() const {
    if (!underlying)
	return 0;
    return _meta->get_field_count((Address)underlying);
    }

MacroObjectPtr NamedList_MacroObjAdapter::get_item(int i) const
{
    if (!underlying)
        return 0;

    FieldDescription *desc = _meta->get_field_description((Address)underlying,i);
    if (!desc) 
	return 0;

    // have we created this object already?
    if (!addedObjects.is_null()) {
	MacroObject *obj = addedObjects->get_child(desc->get_member_name());
	if (obj)
	    return obj;
	}
   
    // build one 
    return const_cast<NamedList_MacroObjAdapter *>(this)->get_item(desc);
    }

static const LString macro_object_list_class_name("MacroListObject_MacroObjAdapter");

MacroObjectPtr NamedList_MacroObjAdapter::get_child(const LString &name) const {
    if (!addedObjects.is_null()) {
        MacroObjectPtr obj = addedObjects->get_child(name);
        if (!obj.is_null())
            return obj;
        }
    FieldDescription *desc;
    static LString parent_name("parent");
    static String uscore("_");
    if (name == parent_name) {
        desc = _meta->get_field_description((Address)underlying, name);
        }
    else {
        desc = _meta->get_field_description((Address)underlying, uscore + name.c_str());
        }

    if (!desc) {
	// Special case. We allow a name specified in the singular to be used for 
	// the first entry of a list. ie name is equivalent to names[0]
	String lname = name + "s";

	MacroObjectPtr list_obj;
	if (!addedObjects.is_null()) 
	    list_obj = addedObjects->get_child(lname);
	if (!list_obj.is_null()) {
	    desc = _meta->get_field_description((Address)underlying,
                                                          uscore + lname);
	    if (!desc)
		return 0;
	    list_obj = const_cast<NamedList_MacroObjAdapter *>(this)->get_item(desc);
	    if (list_obj)
		const_cast<NamedList_MacroObjAdapter *>(this)->AddObject(lname,list_obj);
	    }
	if (list_obj.is_null())
	    return 0;
	if (!list_obj->isKindOf(macro_object_list_class_name))
	    return 0;
  	MacroObjectPtr obj = ((MacroListObject_MacroObjAdapter *)list_obj.get_ptr())->get_item(0);
	return obj;
	}
    // build one
    return const_cast<NamedList_MacroObjAdapter *>(this)->get_item(desc);
    }

MacroObjectPtr NamedList_MacroObjAdapter::get_item(FieldDescription *desc) {
    Address address = (Address *)((char *)underlying + desc->get_offset());

    // Some of this code should really be in the meta_class in a suitable
    // virtual function.. This needs some reorg of macro code.

    MacroObjectPtr obj = builder->build_macro_object(address,desc->get_meta_class());
    if (!obj.is_null()) {
	String s  = desc->get_member_name();
	AddObject(s.Right(-1),obj);
	}
    return obj;
    }

void NamedList_MacroObjAdapter::set_meta(AggregateMetaClass *the_meta) {
    _meta = the_meta;
    }


void NamedList_MacroObjAdapter::AddObject(const LString &name, MacroObjectPtr object)
{
    if (addedObjects.is_null()) {
        addedObjects = new NamedList();
	}
    // kernel_assert_message(name[0] != '_',("putting _ into name table"));
    addedObjects->AddObject(name, object);
}

NamedList_MacroObjAdapter* to_NamedList_MacroObjAdapter(MacroObject *p)
    {
    if (p && p->isKindOf(NamedList_MacroObjAdapter::get_ClassName()))
        return (NamedList_MacroObjAdapter*)p;
    else
        return 0;
    }

LString MacroListObject_MacroObjAdapter::object_type_name() const
{
    return _meta->get_instance_name();
}

MacroListObject_MacroObjAdapter::MacroListObject_MacroObjAdapter(MacroObjectBuilder *tbuilder)
    : _underlying(0),_meta(0), _built_list(false),builder(tbuilder) {}

MacroListObject_MacroObjAdapter::~MacroListObject_MacroObjAdapter() {
  list.reset();
  }

void MacroListObject_MacroObjAdapter::set_underlying(Address the_underlying) {
    _underlying = the_underlying;
    reset();
    _built_list = false;
    }

LString MacroListObject_MacroObjAdapter::get_ClassName()
{
    return macro_object_list_class_name;
}

bool MacroListObject_MacroObjAdapter::isKindOf( const LString &kind ) const
{
    return ((kind == macro_object_list_class_name) || AbstractMacroListObject::isKindOf(kind));
}



void MacroListObject_MacroObjAdapter::set_meta(ListMetaClass *the_meta) {
    _meta = the_meta;
    reset();
    _built_list = false;
    }

void MacroListObject_MacroObjAdapter::AddObjectList(const AbstractMacroListObject &x)
{
    AbstractMacroListObject * cur = const_cast<AbstractMacroListObject *>(&x);
    MacroIterPtr iter = cur->get_iter();
    while (!iter->isDone())
        {
        AddObject(iter->Item());
        iter->Next();
        }
}

void MacroListObject_MacroObjAdapter::AddObject(MacroObjectPtr object) { 
    if (!_built_list)
        build_list();
    list.push(object); 
    }

void MacroListObject_MacroObjAdapter::CutBack(int new_len) {
    if (!_built_list)
        build_list();
    list.set_len(new_len);
    }

MacroIterPtr MacroListObject_MacroObjAdapter::get_iter() {
    if (!_built_list)
        build_list();
    return new simple_stack_MacroIter<MacroObjectPtr>(list);
    }

int MacroListObject_MacroObjAdapter::length() const {
    if (!_built_list)
        const_cast<MacroListObject_MacroObjAdapter*>(this)->build_list();
    return list.len();
    }

MacroObjectPtr MacroListObject_MacroObjAdapter::get_item(int i) const {
    if (!_built_list)
	const_cast<MacroListObject_MacroObjAdapter*>(this)->build_list();
    if ((i < 0) || (i >= list.len()))
	return 0;
    return list[i];
    }

void MacroListObject_MacroObjAdapter::reset() { 
    list.reset();
    _built_list = true;
    }

void MacroListObject_MacroObjAdapter::perform_final_cleanup() {
    while (list.len() > 0) {
	list.top()->perform_final_cleanup();
	list.pop();
	}
    }

void MacroListObject_MacroObjAdapter::build_list() {
    _built_list = true;
    Iterator *iter = _meta->get_iterator(_underlying,Iterator::Owned);
    while (iter->is_valid()) {
	MacroObjectPtr obj = builder->build_macro_object(iter->current(),iter->current_meta_class());
	if (obj != 0)
	    list.push(obj);
	iter->next();
	}
    delete iter;
    }


void MacroListObject_MacroObjAdapter::Print(int indent) const {
    
    print_start(indent,"MacroListObject",object_type_name());
    if (indent < 10) {
        if (!_built_list)
	    const_cast<MacroListObject_MacroObjAdapter*>(this)->build_list();
        for (int i = 0;i < list.len();i++)
            {
            list[i]->Print(indent + 4);
            print_indent(indent);
	    }
        }
    print_end(indent);
    }

void NamedList_MacroObjAdapter::Print(int indent) const {
    print_start(indent,"NamedList",object_type_name());
    if (indent < 10) {

        for (int i = 0;i < length();i++)
            {
	    FieldDescription *desc = _meta->get_field_description((Address)underlying,i);

	    if (desc != 0)
                fprintf(stdout,"%s=>",(const char *)desc->get_member_name());
	    else 
	        fprintf(stdout,"<unknown field>=>");
	    MacroObject * item = get_item(i);
	    if (!item)
	        fprintf(stdout,"NULL");
	    else
                item->Print(indent + 4);
            print_indent(indent);
	    }
        }
    }

String NamedList_MacroObjAdapter::child_name_list() const {
    String names;
    for (int i = 0;i < length();i++)
        {
        FieldDescription *desc = _meta->get_field_description((Address)underlying,i);
	names = names + desc->get_member_name() + " ";
	get_child(desc->get_member_name());
  	}
    return names;
    }

void StringMacroObject_MacroObjAdapter::Print(int indent) const {
    print_start(indent,"StringMacroObject",object_type_name());
    fprintf(stdout,"%s",(const char *)underlying);
    print_end(indent);
    }

static const LString string_macro_object_list_class_name("StringMacroObject_MacroObjAdapter");
LString StringMacroObject_MacroObjAdapter::object_type_name() const
{
    return _meta->get_instance_name();
}

LString StringMacroObject_MacroObjAdapter::get_ClassName()
{
    return string_macro_object_list_class_name;
}

void StringMacroObject_MacroObjAdapter::set_meta(const MetaClass *meta) {
    _meta = meta;
    }

void NamedList_MacroObjAdapter::perform_final_cleanup() {
    if (!addedObjects.is_null())
	addedObjects->perform_final_cleanup();
    }

void  MacroObjectBuilder::check() {
        suif_hash_map<MacroObjectKey ,MacroObject *>::iterator iter =
                built_ptrs.begin();
	while (iter != built_ptrs.end()) {
            if ((*iter).second->ref_count() > 0 && (*iter).second->ref_count()< 1000) {
        // FIXME - 64bit
		//printf("whats up at %X\n",(unsigned)(*iter).second);
		printf("whats up at %X\n",(unsigned long)(*iter).second);
		}
	    iter ++;
	    }
        }


