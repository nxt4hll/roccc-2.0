#ifndef __MACRO__
#define __MACRO__

#include "common/MString.h"
#include <string.h>
#include "macroBase.h"

//	A macro exapnder
//
//
//	A Note on object_type_name
//
//		This is now overridden to provide a domain specific identifer. The default is
//		still the object ClassName, but it is overridden, for example, in the parser, to
//		give the name of the production that created the object
//
//		The old functionality is now given by get_instance_class_name


// 	Implementation of the simple case of a list of MacroObjects

class MacroListObject : public AbstractMacroListObject
    {
        static const LString ClassName;
        ref_stack<MacroObjectPtr > list;
        bool owns_contents;
	bool _performing_cleanup;

	LString type_name;


    public:

        MacroListObject(bool owner = false) :  owns_contents(owner),_performing_cleanup(false) {}
	MacroListObject(const MacroListObject &obj) : list(obj.list),owns_contents(obj.owns_contents),_performing_cleanup(false) {}

        void AddObject(MacroObjectPtr object);

	MacroObjectPtr  get_item(int i) const {return list[i];}

	virtual ~MacroListObject();

	virtual void Print(int indent = 0) const;

	virtual int length() const {return list.len();}

	virtual LString get_instance_class_name() const {return ClassName;}
        virtual LString object_type_name() const {return type_name;}
 	void set_type_name(const LString &type) {type_name = type;}
	static const LString & get_ClassName() {return ClassName;}
        virtual bool isKindOf( const LString &kind ) const
            { return ((kind == ClassName) || AbstractMacroListObject::isKindOf(kind));
            }

        virtual void CutBack(int new_top_pos) {
	    list.cut(new_top_pos);
	    }

	MacroIterPtr get_iter()   {return MacroIterPtr(new simple_stack_MacroIter<MacroObjectPtr >(list));}

        void reset() {list.reset();}

	void AddObjectList(const AbstractMacroListObject &x);

	void perform_final_cleanup();

    };

MacroListObject * to_MacroListObject(MacroObjectPtr );



//	Only one parameter gets passed in. This has no name. To pass in real parameters,
//	you generally want to pass an instance of the following class. The top level name resolution
// 	will then be done in the class object

typedef DerivedRCPointer<MacroListObject,MacroObjectPtr> MacroListObjectPtr;

class NamedList : public AbstractNamedList
    {
	simple_stack<LString> names;
	MacroListObjectPtr list;
        static const LString ClassName;
	LString type_name;
    public:

	NamedList(bool owner = false) : list(new MacroListObject(owner)) {}

	virtual void Print(int indent = 0) const;

	void set_type_name(const LString &type) {type_name = type;}
	virtual LString object_type_name() const {return type_name;}
	virtual LString get_instance_class_name() const {return ClassName;}
        static const LString & get_ClassName() {return ClassName;}
        virtual bool isKindOf( const LString &kind ) const
            { return ((kind == ClassName) || AbstractNamedList::isKindOf(kind));
            }


        virtual void AddObject(const LString &name, MacroObjectPtr object)
            {
            list->AddObject(object);
            names.push(name);
            }

	virtual int length() const {return list->length();}

        virtual void CutBack(int new_len)
            {
            while (names.len() > new_len)
                names.pop();
            list->CutBack(new_len);
            }

	MacroIterPtr get_iter() {return new SingleMacroIter(this);}

 	MacroObjectPtr  get_item(int i) const {return list->get_item(i);}

	MacroObjectPtr  get_child(const LString &name) const;

	virtual String child_name_list() const;

	MacroObjectPtr get_as_list() {return list;}

	void perform_final_cleanup() {
	    list->perform_final_cleanup();
	    }
    };

typedef DerivedRCPointer<NamedList,MacroObjectPtr> NamedListPtr;


NamedList * to_NamedList(MacroObjectPtr );

//	Implementation of the simple string valued macro object


class StringMacroObject : public AbstractStringMacroObject
    {
	String text;
        static const LString ClassName;
        LString type_name;

    public:
	StringMacroObject(String s) : text(s) {}

	void set_text(const String &the_text) {text = the_text;}

	const String get_text() const {return text;}

	MacroIterPtr get_iter() {return new SingleMacroIter(this);}

	virtual LString object_type_name() const {return type_name;}
 	void set_type_name(const LString &type) {type_name = type;}
	virtual LString get_instance_class_name() const {return ClassName;}
        static const LString & get_ClassName() {return ClassName;}
        virtual bool isKindOf( const LString &kind ) const
            { return ((kind == ClassName) || AbstractStringMacroObject::isKindOf(kind));
            }

    };

StringMacroObject*  to_StringMacroObject(MacroObjectPtr );

    

class MacroDef;

class parameter;

class foreach_stack_entry;

class OutputFile;



//	MacroExpansion is the class that performs the macro expansion. The expanded text is
//	returned as a set of "files". Each file has a name and a body of text. Either of these
//	may be empty - in particular the name of the first (index 0) file is always empty. This
//	is the default output file.

class Breakpoint
    {
    public:
        int lineno;
        const char *text;	// position in text of breakpoint
	int filenbr;		// non zero if stop when file a given no of lines
        Breakpoint(int line,const char *t) 
		: lineno(line),text(t),filenbr(0) {}
        Breakpoint() 
		: lineno(0),text(NULL),filenbr(0) {}
	Breakpoint(int line,int file) 
		: lineno(line),filenbr(file) {}
    };

class MacroExpansion 
    {
        simple_stack <MacroDef *>macro_defs;

	simple_stack<OutputFile *> files;
	simple_stack<foreach_stack_entry *> foreach_stack;
	bool expanded;


	simple_stack<const char *> filename_positions;
	simple_stack<const char *> start_positions;
	simple_stack<const char *> saved_positions;

	simple_stack<MacroDef *>macro_calls;

	MacroObjectPtr par;

        const char *text;       // current position in text
        const char *filename;   // current filename in text
        const char * start;     // start of text being processed
	int text_len;

	bool is_in_range(const char *text);

	bool expand_macro(String name);

        void expand_inner(const char *filename, const char *text);

	void error_message(const char *message,bool use_stdout = false);

	bool skip_keyword(const char * keyword,const char *error_msg);

	MacroObjectPtr get_named_object(const LString &name, bool diagnose, MacroObjectPtr add_object);

	MacroObjectPtr get_named_object(bool diagnose, MacroObjectPtr add_object);

	parameter condition_true();

	void skip_brackets(char left,char right);

	String get_expanded_value(const char *start,int len);

	foreach_stack_entry *get_foreach_object(const LString &name);

	void process_foreach_clause();

	void process_let();

 	bool skipping_white_space;

	void select_file(const String &x);
	void parse_file(const String &name);

	simple_stack<Breakpoint> breakpoints;

	simple_stack<String> include_dirs;

	void pop_foreach_stack();

 	bool debug_it;
	String last_command;
	bool step;
	bool step_over;

	const char * normalize_pos(const char *text);

	bool define_name(const char *par_name);

	class source_files {
		String name;
		const char *start;
		const char *end;
	    public:
		const String &get_name() {return name;}
		const char *get_text() {return start;}
	        bool is_in(const char *c) {return ((c >= start) && (c < end));}
		source_files(const String &n,const char *t) : name(n),start(t),end(t + strlen(t)) {}
		};

	simple_stack <source_files *> sources;

        int find_name(const LString &name);
        void add_macro(MacroDef *def);
        MacroDef *resolve_macro(const LString &name,
                                simple_stack<foreach_stack_entry *>&pars,int par_offset,int par_len);
        void reset();


    public:
	MacroExpansion(int start=100,int expand=100);
 	virtual ~MacroExpansion();
	bool is_expanded() {return expanded;}

	// Note that there is exactly one parameter. See NamedPars above.

	// Expand will expand the given text and return the text as output. Note that no
	// initialization is done by expand. Hence, any defs you already had from a previous
	// call are still present and, in addition, the text previously expanded is still
	// present, so the new expansion will be appended to the old
	//
	// (The reset_defs and reset_text routines clear these)

	void Expand(const char *filename, const char *text,MacroObjectPtr param);

	//	The following two virtual functions can be overridden to
	//	expand body text and parameter text respectively. This allows
	//	differeing translation to be supported
	//
	//	The implementations in this routine just copy characters 
	//	You can also use the push routine inherited from simple_stack to
	//	push single characters

	virtual void push_body_text(const char *text,int len);
	virtual void push_par_text(const char *text);

	// 	Execute a command ("function") on the text
	//
	//	commands take the form <!<command> <parameter_name> <extra text>
	//
	//	this version can handle the following commands (calling push_par_text to push output)
	//	!' enclose parameter in single quotes
	//	!" enclose in double quotes
	//	!CAPS output in all caps
	//	!LOWS output in all lower case
	//	!CPZ  capitalizes text. First character is captalized. Underlines are removed and the
	//				next character is capitalized
	//	!UNL  lower cases text and puts an underline in front of capitals
	//	!POS  position in iterator (number)
	//
	//	
	//
	//	NB = the shriek (!) is not passed as part of the command. It is used to invoke the command
	virtual void push_par_text_command(String command,const MacroObjectPtr p,String extras);

	//	Clear the macro definitions

	// 	Clear the text

	void reset_text();

	void inherit_pars(MacroExpansion &y);

	//	These routines retrieve the actual bodies of text.
	//	The first file is always nameless and is the default output device
	int file_count() {return files.len();}

	const String & get_file_name(int i);
	String get_file_text(int i);

	// set flag that this needs debugging

	void set_debug(bool d) {debug_it = d;}

	void add_break(int lineno);
        void show_breaks();
        void delete_break(int no);
        void clear_breaks();

        bool get_at_breakpoint();
        void set_at_breakpoint(bool b);
        void get_lineno(const char *text,int &line_no,int &file_no);
        void show_pos(const char *text);
        void print_lines(const char *start,int from,int to);

	void process_debug_commands();

	void add_include(const String &x)
	    {
	    include_dirs.push(x);
	    }

	virtual int derivation_distance(const LString &required,const MacroObjectPtr found);
    };

void print_indent(int indent);

void print_start(int indent,const char *type,const char * dynamic_type);

void print_end(int indent);

#endif
