#include "common/system_specific.h"
#include "suifpasses/suifpasses.h"
#include <stdio.h>
#include <stdlib.h>
#ifndef MSVC
#include <unistd.h>
#else
#include <direct.h>
#endif
#include "suifkernel/command_line_parsing.h"
#include "common/text_util.h"
#include "macro_suifobj_adapter/MacroObjAdapterBase.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/suifkernel_messages.h"
#include "suifkernel/module_subsystem.h"

#include "basicnodes/basic.h"
#include "suifnodes/suif.h"
#include "cfenodes/cfe.h"
#include "iokernel/object_factory.h"

  inline bool is_directory_separator(char ch) {
	return ((ch == '/') || (ch == '\\')); // stupid msdos trick
	}

  static String get_default_file_name(const char *nci_based_name) {
    // first try to find NCIHOME variable

    char *ncihome = getenv("NCIHOME");
    char *buff = NULL;
    String filename;
    if (ncihome == NULL) {
	// try to incant nci from current directory
	int len = 64;
 	char *cur_dir = NULL;
	while ((cur_dir == NULL) && (len < 16536)) {
	    delete buff;
	    len += len;
	    buff = new char[len];
	    cur_dir = getcwd(buff,len);
	    }
	if (cur_dir != NULL) {
	    int i = 0;
	    while (cur_dir[i])i++; 
	    i--;
	    while (i > 0) {
		if (is_directory_separator(cur_dir[i])) {
		    cur_dir[i] = 0;
		    i --;
		    }
		if ((cur_dir[i-2] == 'n') && (cur_dir[i-1] == 'c') && (cur_dir[i] == '0')) {
		    ncihome = cur_dir;
		    break;
		    }
		while ((i > 0) && !is_directory_separator(cur_dir[i])) 
		    i--;
	   	}
	    }
	}
    // if we could not find ncihome, try a simple file name in our local directory
    if (ncihome == NULL) {
  	int i = 0;
	int j = 0;
	while (nci_based_name[i]) {
	    if (is_directory_separator(nci_based_name[i]))
		j = i + 1;
	    i ++;
	    }
	if (nci_based_name[j] != 0) {
	    filename = String(nci_based_name + j);
	    }
	}
    else {
	    filename = String(ncihome) + String(
#ifndef MSVC
            "/"
#else
            "\\"
#endif
            ) + String(nci_based_name);;
	}
    // ok, we have developed a file name

    delete buff;

    return 
#ifndef MSVC
            filename;
#else
            String("\\")+filename;
#endif
    }

class S2CMacroExpansion : public MacroExpansion {
	SuifEnv *_env;
    public:
	int derivation_distance(const LString &required,const MacroObjectPtr found);
	S2CMacroExpansion(SuifEnv *env,int len);
    };

S2CMacroExpansion::S2CMacroExpansion(SuifEnv *env,int len) : MacroExpansion(len,len),_env(env) {}

int S2CMacroExpansion::derivation_distance(
			const LString &required,
			const MacroObjectPtr found) {
    LString required_name(required);
    LString found_name(found->object_type_name());
    MetaClass *found_meta = _env->get_object_factory()->lookupMetaClass(found_name);
    int count = 0;
    while (found_meta && (found_meta->get_instance_name() != required_name)) {
	count ++;
	found_meta = found_meta->get_link_meta_class();
	}
    if (!found_meta)
        return -1;
    return count;
    }

class S2CPass : public Pass {

  OptionLoop       *_def_loop_argument;
  OptionList       *_def_list_argument;
  OptionLiteral    *_def_flag_argument;
  OptionMultiString*_def_opts_argument;
  OptionSelection  *_def_selector;
  bool def;

  OptionLoop       *_incl_loop_argument;
  OptionList       *_incl_list_argument;
  OptionLiteral    *_incl_flag_argument;
  OptionMultiString*_incl_opts_argument;
  OptionSelection  *_incl_selector;
  bool incl;

  OptionLiteral   *_debug_argument;
  OptionSelection *_debug_selector;

  suif_vector<String> _d_opts;
  suif_vector<String> _i_opts;

  OptionString *_macro_file_name_argument;
  OptionString *_output_file_name_argument;
  OptionSelection * _option_selector;
  FILE *c_file;
  simple_stack<String *> defines;
  simple_stack<String *> includes;
  bool debug_it;
  String _macro_file_name;

public:

  S2CPass(SuifEnv *s, const LString &name= "s2c") :
    Pass(s, name),
    _def_loop_argument(0),
    _def_list_argument(0),
    _def_flag_argument(0),
    _def_opts_argument(0),
    def(false),
    _incl_loop_argument(0),
    _incl_list_argument(0),
    _incl_flag_argument(0),
    _incl_opts_argument(0),
    incl(false),
    _debug_argument(0),
    _macro_file_name_argument(0),
    _output_file_name_argument(0),
    c_file(0),
    defines(),
    includes(),
    debug_it(false)
    {}


  FILE *open_c_file(const String &filespec) {
    if (filespec == String("-")) {
      return(stdout);
    }
    FILE *f = fopen(filespec.c_str(), "w");
    if (f == NULL) {
      suif_assert_message(f == NULL,
			  ("Could not open %s for writing.\n", filespec.c_str()));
    }
    
    return f;
  }


    
  void do_file_set_block(FileSetBlock *fsb) {
    String output_file_name = 
      _output_file_name_argument->get_string()->get_string();
    c_file = open_c_file(output_file_name);

    SuifEnv *s = get_suif_env();
    MacroObjectBuilder builder;
    MacroObjectPtr root =  builder.build_macro_object((Address)fsb,fsb->get_meta_class());

    char *text = get_file_text(_macro_file_name.c_str());
    if (text == NULL)
    {
      suif_error(s, "could not open macro file %s\n",_macro_file_name.c_str());
      //        fprintf(stderr,"could not open macro file %s\n",macro_filename);
      return;
      //        return -1;
    }
    size_t i;
    for (i = 0;i < _d_opts.size(); i++)
      AddDefine(root,_d_opts[i]);

    int len = strlen(text);
    S2CMacroExpansion expand(s,len);

    for (i = 0;i < _i_opts.size(); i++)
    {
      String value = _i_opts[i];
      expand.add_include(value);
    }

    // root->Print(0);

    expand.set_debug(debug_it);
    expand.Expand(_macro_file_name,text,root);
    for (int j=1;j<expand.file_count();j++) {
	const String x = expand.get_file_text(j);
        write_text(expand.get_file_name(j),x);
	}

    const String def = expand.get_file_text(0);
    if (def.length() > 0)
        fprintf(c_file,"%s",def.c_str());
    root->perform_final_cleanup();
    if (c_file  && c_file != stdout) 
      fclose(c_file);
    delete [] text;
    }

  void AddDefine(MacroObjectPtr &mac,const String &s)
    {
      int pos = s.find("=");
      if (pos < 0)
        return;
      String name = s.Left(pos);
      String value = s.Right(-pos-1);
      NamedList_MacroObjAdapter *p = (NamedList_MacroObjAdapter *)mac.get_ptr();
      // printf("adding name %s value %s\n",(const char *)name,(const char *)value);
      p->AddObject(name,new StringMacroObject(value));
      // p->Print(0);
    }

  virtual void initialize() {
    Pass::initialize();

    _def_flag_argument = new OptionLiteral("-D", &def, true);
    _def_opts_argument = new OptionMultiString("definition", &_d_opts);
    _def_list_argument = new OptionList();
    _def_list_argument->add(_def_flag_argument);
    _def_list_argument->add(_def_opts_argument);
    _def_loop_argument = new OptionLoop(_def_list_argument, true);

    _def_selector = new OptionSelection(true);
    _def_selector->add(_def_loop_argument);
    _command_line->add(_def_selector);

    _incl_flag_argument = new OptionLiteral("-I", &incl, true);
    _incl_opts_argument = new OptionMultiString("include", &_i_opts);
    _incl_list_argument = new OptionList();
    _incl_list_argument->add(_incl_flag_argument);
    _incl_list_argument->add(_incl_opts_argument);
    _incl_loop_argument = new OptionLoop(_incl_list_argument, true);

    _incl_selector = new OptionSelection(true);
    _incl_selector->add(_incl_loop_argument);
    _command_line->add(_incl_selector);
    
    _debug_argument = new OptionLiteral("-d", &debug_it, true);
    _debug_selector = new OptionSelection(true);
    _debug_selector-> add( _debug_argument );
    _command_line-> add(_debug_selector);

#ifndef MSVC
    _macro_file_name = get_default_file_name("suif/suif2b/basesuif/s2c/c_text.mac");
#else
    _macro_file_name = get_default_file_name("suif\\suif2b\\basesuif\\s2c\\c_text.mac");
#endif

    _output_file_name_argument = new OptionString( "output-file" );
    _command_line-> add( _output_file_name_argument );
    _macro_file_name_argument = new OptionString( "macro-file",&_macro_file_name );
    _option_selector = new OptionSelection(true);
    _option_selector-> add( _macro_file_name_argument );
    _command_line-> add(_option_selector);
    
    _command_line-> set_description("Outputs C form of a SUIF tree." );
    // SuifEnv *s = get_suif_env();

  }

  virtual Module *clone() const { 
    return(new S2CPass(get_suif_env(), get_module_name()));
  }
  virtual bool delete_me() const { return(true); }
};
#ifndef EXPORT
#define EXPORT
#endif

extern "C" void init_s2c(SuifEnv *suif_env) {
  suif_env->require_module("suifnodes");
  suif_env->require_module("cfenodes");
  suif_env->require_module("suifpasses");

  ModuleSubSystem* mSubSystem = suif_env->get_module_subsystem();
  if (!mSubSystem->retrieve_module("s2c")) {
    mSubSystem -> register_module( new S2CPass( suif_env ) );
    }

  
}

