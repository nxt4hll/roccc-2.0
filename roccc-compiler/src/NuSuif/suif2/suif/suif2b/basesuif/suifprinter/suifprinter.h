#ifndef PRINT_SUIFPRINTERMODULE_H_
#define PRINT_SUIFPRINTERMODULE_H_

#include <iostream>
#include "suifkernel/module.h"
#include "iokernel/iokernel_forwarders.h"
#include "suifkernel/suifkernel_forwarders.h"
#include "common/suif_vector.h"
#include "suifkernel/print_subsystem.h"
#include "common/system_specific.h"


extern DLLIMPORT LString defaultPrinterModuleName;

//const int MAX_TAGS = 100000;
class ObjectTags {
public:
  ObjectTags();
  ~ObjectTags();
  size_t retrieve_tag(const ObjectWrapper &obj);
  size_t get_tag(const ObjectWrapper &obj);
  bool has_tag(const ObjectWrapper &obj);
private:
  typedef suif_hash_map<Address, size_t> TagMap;
  TagMap *_tags;
  size_t _next_tag;
};


class SuifPrinterModule : public Module {
public:
  SuifPrinterModule(SuifEnv* suif);
  void initialize();
  void execute();
  virtual Module *clone() const;
  std::ostream& get_default_stream() { return output; }
  //void set_default_stream(std::ostream &o) { output = o; }
  
  // This is the only one that counts...
  void print(std::ostream& output, const ObjectWrapper &obj);


  void print(const Address what, const MetaClass* type);
  void print(std::ostream& output);
  void print(const SuifObject *root);
  void print(std::ostream& output, const SuifObject *root);
  //  void print(std::ostream& output, const Address what);
  void print(std::ostream& output, const SuifObject* s, const MetaClass* type);
  void print();

  void print(std::ostream& output, const Address what,
	     const MetaClass* type, int indent  = 2);

  bool print2(std::ostream& output, const ObjectWrapper &obj,
       const LString &name = emptyLString, int _indent = 2, int deref = 0);
  void init();
  void set_print_all() { _print_all = true; }
  void unset_print_all() { _print_all = false; }
  bool print_all() const { return _print_all; }
  void set_use_print_string() { _use_print_string = true; }
  void set_use_print_ref_string() { _use_print_ref_string = true; }
  void unset_use_print_string() { _use_print_string = false; }
  void unset_use_print_ref_string() { _use_print_ref_string = false; }

  bool use_print_string() const { return _use_print_string; }
  bool use_print_ref_string() const { return _use_print_ref_string; }

  String get_print_string(const LString &meta_class_name) {
    return _string_table->get_print_string(meta_class_name); }
  String get_print_ref_string(const LString &meta_class_name) {
    return _string_table->get_print_ref_string(meta_class_name);  }

  inline void indent(std::ostream& o, int _indent) { 
    int i = _indent; if (i > 0) while (i--) o <<' '; }

  static LString ClassName;
  size_t retrieve_tag(const ObjectWrapper &obj) { 
    return _t.retrieve_tag(obj); }
  size_t get_tag(const ObjectWrapper &obj) { 
    return _t.get_tag(obj); }
  bool has_tag(const ObjectWrapper &obj) { 
    return _t.has_tag(obj); }

  // interface for deferred initialization
  virtual void interface_object_created(Module *producer,
				          const LString &interface_name);

  // Support the "print" interface
  static void print_dispatch(Module *, std::ostream &, const ObjectWrapper &);

  typedef void (*print_init_fn)(SuifEnv *);

   /**	You can override this function to control actions at the start of
    *	the printing of an object. If the function returns false, no 
    *	printing is performed 
    */
    virtual bool start_of_object(std::ostream& output, const ObjectWrapper &obj,int derefs);
    /**	You can override this function to gain control at the end of printing
     *  an object. Note - not called if start_of_object returned false
     */
    virtual void end_of_object(std::ostream& output, const ObjectWrapper &obj);


protected:
private:
  PrintSubSystem *_pr;
  PrintStringRepository *_string_table;
  bool _print_all;
  bool _use_print_string;
  bool _use_print_ref_string;
  bool _initialized;
  std::ostream &output;

  ObjectTags _t;
  list<print_init_fn> *_print_inits;

  bool parse_and_print(std::ostream& output,const ObjectWrapper &obj,
		       const LString &name, const String &str, 
		       int indent, int deref);

  bool print_elementary(std::ostream& output,
			const ObjectWrapper &obj,
			//const Address what,const MetaClass*type,
			const LString &name, int indent, int deref);
  bool print_aggregate(std::ostream& output,
		       const AggregateWrapper &obj,
		       //   const Address what,const MetaClass* type,
		       const LString &name, int indent, int deref);
  //  bool print_pointer(std::ostream& output, const Address what, const PointerMetaClass* type,
  //             const LString &name, int indent, int deref);
  bool print_pointer(std::ostream& output, const PointerWrapper &ptr_obj,
             const LString &name, int indent, int deref);
  bool print_list(std::ostream& output, const ObjectWrapper &obj,
		  //const Address what, const MetaClass* type,
             const LString &name, int indent, int deref);
  bool print_stl(std::ostream& output, const ObjectWrapper &obj,
		 //		 const Address what, const MetaClass* type,
		 const LString &name, int indent, int deref);
  bool print_catchall(std::ostream& output, const ObjectWrapper &obj,
		      //const Address what,const MetaClass* type,
		      const LString &name, int indent, int deref);
};

extern "C" void EXPORT init_suifprinter(SuifEnv* suif);
#endif
