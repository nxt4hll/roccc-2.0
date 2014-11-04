#include "common/system_specific.h"
#include "location_modules.h"

#include "suifkernel/suif_env.h"
#include "suifkernel/suif_object.h"
#include "suifkernel/command_line_parsing.h"
#include "suifkernel/error_subsystem.h"
#include "suifkernel/module_subsystem.h"

#include "basicnodes/basic.h"

static ObjectLocation read_line_number(const SuifObject* stmt){
  Annote* annote = (to<AnnotableObject>(stmt))->peek_annote("line");
  // have the line # -- return
  if ( annote != NULL ) {
    IInteger line = to<IntegerBrick>(to<BrickAnnote>(annote)->
				     get_brick(0))->get_value();
    String file = to<StringBrick>(to<BrickAnnote>(annote)->
				   get_brick(1))->get_value();
    return(ObjectLocation(file, line));
  }
  ObjectLocation loc;
  return(loc);
}


const LString LocationModule::get_class_name() {
  static LString name( "find_location" );
  return name;
}



LocationModule::LocationModule( SuifEnv * suif_env ) : Module( suif_env ) {
  _module_name = LocationModule::get_class_name();
}


void LocationModule::initialize() {
  Module::initialize();
  set_interface("get_object_location", 
		(Address)LocationModule::get_object_location);
}

static ObjectLocation get_object_filename(SuifObject *obj) {
  ObjectLocation loc;
  if (!obj) {
    return(loc);
  }
  while (obj && !is_kind_of<FileBlock>(obj)) {
    obj = obj->get_parent();
  }
  if (!obj) return loc;
  FileBlock *fb = to<FileBlock>(obj);
  return(ObjectLocation(fb->get_source_file_name(),
			0));
}


ObjectLocation LocationModule::get_object_location(SuifObject *obj) {
  ObjectLocation loc;
  if (!obj) {
    return(loc);
  }
  
  // part of a stmt list: walk from the beginning of the list
  // down until we find a line #
  const SuifObject* child = NULL;
  const SuifObject* parent = obj;
  for (; parent != NULL; child = parent, parent = parent->get_parent()) {

    loc = read_line_number(obj);
    if (loc.get_is_known()) return(loc);

    if(is_kind_of<StatementList>(parent)){
      StatementList* the_list = to<StatementList>(parent);
      for(Iter<Statement*> iter = the_list->get_statement_iterator();
	  iter.is_valid() && iter.current() != child; 
	  iter.next())
	{
	  loc = read_line_number(iter.current());
	  // for the original object, if it is a StatementList
	  // return the first mark found in it.
	  if (child == NULL && loc.get_is_known())
	    return(loc);
	}
    }
    if (loc.get_is_known())
      return loc;
  }
  // Now try to just get the input file
  return(get_object_filename(obj));
  //  return(loc);
};

 
void LocationModule::execute() {
}

Module *LocationModule::clone() const {
  return(Module *)this;
}
