
#ifndef MACROOBJADAPTECBASE_H
#define  MACROOBJADAPTERBASE_H
#include "iokernel/meta_class.h"
#include "smgn/macroBase.h"
#include "smgn/macro.h"
#include "common/formatted.h"
#include "common/suif_hash_map.h"
#include "iokernel/object_wrapper.h"

//	Instantiate a MacroObjectBuilder to build objects


class MacroObjectKey {
    Address _address;
    MetaClassId _id;
  public:
    bool operator ==(const MacroObjectKey &x) const;
    MacroObjectKey();
    MacroObjectKey(Address addr,const MetaClass *meta);
    MacroObjectKey(const ObjectWrapper &obj);
    int hash() const;
    };

int hash(const MacroObjectKey &x) {return x.hash();}

class MacroObjectBuilder {
   	suif_hash_map<MacroObjectKey,MacroObject *>built_ptrs;
	MacroObjectPtr build_macro_object_inner(Address address,
					      const MetaClass *meta);
	MacroObjectPtr build_macro_object_inner(const ObjectWrapper &obj);
    public:
	MacroObjectBuilder();
	~MacroObjectBuilder();
    	MacroObjectPtr build_macro_object(Address address,
					const MetaClass *meta);
    	MacroObjectPtr build_macro_object(const ObjectWrapper &obj);
	void check();
    };

class MacroListObject_MacroObjAdapter : public AbstractMacroListObject {
  public:

    MacroListObject_MacroObjAdapter(MacroObjectBuilder *);
    virtual ~MacroListObject_MacroObjAdapter();

    virtual void AddObject(MacroObjectPtr object);
    virtual void CutBack(int new_len);
    virtual MacroIterPtr get_iter();
    virtual int length() const;
    MacroObjectPtr get_item(int i) const;
    virtual void reset();

    virtual void Print(int indent = 0) const;
        
    virtual void AddObjectList(const AbstractMacroListObject &x);

    virtual LString object_type_name() const;
    static LString get_ClassName();

    void set_meta(ListMetaClass *the_meta);

    void set_underlying(Address the_underlying);

    virtual bool isKindOf( const LString &kind ) const;

    void perform_final_cleanup();

  protected:
  private:
    void build_list();
    ref_stack<MacroObjectPtr>list;
    Address _underlying;
    ListMetaClass *_meta;
    bool _built_list;
    MacroObjectBuilder *builder;
  };

class NamedList_MacroObjAdapter : public AbstractNamedList {
  public:
    NamedList_MacroObjAdapter(MacroObjectBuilder *builder);
    virtual ~NamedList_MacroObjAdapter();

    virtual void AddObject(const LString &name, MacroObjectPtr object);
    virtual void CutBack(int new_len) {}

    virtual int length() const;
    virtual MacroIterPtr get_iter() {return new SingleMacroIter(this);}

    virtual void Print(int indent = 0) const;

    virtual MacroObjectPtr get_item(int i) const;
    String child_name_list() const;  // MORE string together field names???

    virtual MacroObjectPtr get_child(const LString &name) const;

    SuifObject* get_underlying() { return underlying;}
    void set_underlying(SuifObject* u) { underlying = u;}

    virtual LString object_type_name() const;
    static LString get_ClassName();
    virtual bool isKindOf( const LString &kind ) const;

    void set_meta(AggregateMetaClass *the_meta);

    virtual bool is_instance_of(const LString &kind ) const;

    void perform_final_cleanup();

  protected:
    SuifObject *underlying;
    NamedListPtr addedObjects;
    AggregateMetaClass *_meta;
  private:
    MacroObjectPtr get_item(FieldDescription *field);
    MacroObjectBuilder *builder;
  };

NamedList_MacroObjAdapter* to_NamedList_MacroObjAdapter(MacroObject *p);


class StringMacroObject_MacroObjAdapter : public AbstractStringMacroObject {
  public:
    StringMacroObject_MacroObjAdapter() {}
  //    StringMacroObject_MacroObjAdapter(String *u) : underlying(*u) {}
    StringMacroObject_MacroObjAdapter(const String &u) : underlying(u) {}
    virtual ~StringMacroObject_MacroObjAdapter() {}

    virtual MacroIterPtr get_iter()   {return new SingleMacroIter(this);}
    virtual void set_text(const String &the_text)  {};
    virtual const String get_text() const { return underlying; };
    void set_underlying(const String &u) { underlying = u;}

    virtual LString object_type_name() const;
    static LString get_ClassName();
    void set_meta(const MetaClass *meta);

    virtual void Print(int indent = 0) const;

  protected:
    String underlying;
    const MetaClass *_meta;
  };

#endif
