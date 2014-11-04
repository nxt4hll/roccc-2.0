#include "common/system_specific.h"
#include <stdlib.h>
#include "common/MString.h"
#include <stdio.h>
#include "macro.h"
#include "common/simple_stack.h"
#include "common/text_util.h"

const LString MacroObject::ClassName("MacroObject");
const LString MacroIter::ClassName("MacroIter");
const LString MacroListObject::ClassName ("MacroListObject");
const LString NamedList::ClassName ("NamedList");
const LString StringMacroObject::ClassName ("StringMacroObject");


static const char * keywords[] = {
			"def",		// 0
			"echo",		// 1
			"else",
			"elseif",
			"endfor",
			"endif",
			"endlet",
			"endmac",
			"eval",
			"file",		// 9
			"foreach",
			"if",		// 11
			"ignore_linefeeds",
			"include",
			"let",		// 14
			"map",		// 15
			"notice_linefeeds", // 16
			"parse",	// 17
			"pos",
			"set",		// 19
			"show",
			"use"};		// 21


static const int def_ordinal = 0;
static const int echo_ordinal = 1;
static const int else_ordinal = 2;
static const int elseif_ordinal = 3;
static const int endfor_ordinal = 4;
static const int endif_ordinal = 5;
static const int endlet_ordinal = 6;
static const int endmac_ordinal = 7;
static const int eval_ordinal = 8;
static const int file_ordinal = 9;
static const int foreach_ordinal = 10;
static const int if_ordinal = 11;
static const int ignore_linefeeds_ordinal = 12;
static const int include_ordinal = 13;
static const int let_ordinal = 14;
static const int map_ordinal = 15;
static const int notice_linefeeds_ordinal = 16;
static const int parse_ordinal = 17;
static const int pos_ordinal = 18;
static const int set_ordinal = 19;
static const int show_ordinal = 20;
static const int use_ordinal = 21;

static const String str_star("*");
static const String str_true("true");
static const String str_false("false");
static const String str_slash("/");
static const String str_quote("'");
static const String str_double_quote("\"");
static const String str_caps("CAPS");
static const String str_lows("LOWS");
static const String str_sing("SING");
static const String str_cpz("CPZ");
static const String str_unl("UNL");
static const String str_llc("LLC");
static const String str_nspc("NSPC");
static const String str_type("TYPE");
static const String str_iskindof("ISKINDOF");


//				    a b c d e f  g  h  i  j  k  l  m  n  o  p  q  r  s  t  u  v  w  x  y  z
static const int start_index[27] = {0,0,0,0,1,9,11,11,11,14,14,14,15,16,17,17,19,19,19,21,21,22,22,22,22,22,22};

static bool string_match_alpha(const char *s,const char *t) {
    while (*s && ((*s | 0x20) == (*t | 0x20))) {
	s++;
	t++;
	}
    return ((*s == 0) && (*t == 0));
    }

static int get_command_index(const char *text) {
    int index = (*text | 0x20) - 'a';
    if ((index < 0) || (index > 25))
        return -1;
    int start = start_index[index];
    int end = start_index[index + 1] -1;
    while (start <= end) {
	const char *t = text;
	const char *s = keywords[start];
    	while (*s && ((*t == *s) || ((*t | 0x20) == *s) && (*s != '_'))) {
	    t ++;
	    s ++;
	    }
	if ((*s == 0) && !isalpha(*t) && (*t != '_'))
	    return start;
	start ++;
	}
    return -1;
    }

class UseBlock {
	String text;
	int line_count;
    public:
	const String usename;
	int indent;
	UseBlock(const String &x) : text(200,200),line_count(0),usename(x),indent(0) { } 
	inline void push(char ch) {
	    text.push(ch);
	    if (ch == '\n')
		line_count ++;
	    }

	int get_line_count() {return line_count;}
	String &get_text() {
	    return text;
	    }
	int text_size() {return text.length();}
    };

class OutputFile
    {
	String current_text;
    public:
        const String filename;
	simple_stack<UseBlock *> blocks;
	simple_stack<UseBlock *> context;
	UseBlock *current;
        OutputFile(const String &name)
                : filename(name), current(NULL) {
		set_use_block("");
		}
        ~OutputFile();
	void set_use_block(const String &x);
	String get_text();
	int get_lines();
    };

OutputFile::~OutputFile() {
  while (blocks.len() > 0) {
    UseBlock *bl = blocks.pop();
    delete bl;
    }
}
/*
static int get_lines(const char *start)
    {
    int line = 1;
    while (*start)
        {
        if (*start == '\n')
            line ++;
        start++;
        }
    return line;
    }
*/

int OutputFile::get_lines() {
    int lines = 0;
    for (int i = 0;i < blocks.len();i++)
	lines += blocks[i]->get_line_count();
    return lines;
    }

String OutputFile::get_text() {
    String x;
    for (int i = 0;i < blocks.len(); i++)
	x += blocks[i]->get_text();
    return x;
    }

void OutputFile::set_use_block(const String &x) {
    if (x == str_star) {
	if (context.len() > 0) {
	    current = context.pop();
	    }
	else {
	    current = blocks[0];
	    }
	return;
	}
    context.push(current);
    for (int i =0;i<blocks.len();i++) {
	if (blocks[i]->usename == x) {
	    current = blocks[i];
	    return;
	    }
	}
    current = new UseBlock(x);
    blocks.push(current);
    }

static OutputFile *current;

class ParInfo
    {
    public:
	LString name;
	LString type;
	ParInfo(const LString &n,LString t) : name(n),type(t) {}
	ParInfo() {}
    };

class MacroDef
    {
    public:
	LString name;
	simple_stack<ParInfo>par_names;
	MacroDef *next_with_name;
	String text_of_def;
	const char *filename_of_def;
	const char *original_text;
	MacroDef(const LString &the_name) : 
	  name(the_name),par_names(3,3),next_with_name(NULL),
	  filename_of_def(NULL), original_text(NULL) {}
    };

MacroExpansion::MacroExpansion(int start,int expand)
            : 	expanded(false), par(0), text(0),
     		filename(0), start(0), text_len(0),
     		skipping_white_space(false),
     		debug_it(false), step(false), step_over(false) {
    }

void MacroExpansion::reset()
    {
    while (macro_defs.len() > 0)
	{
	MacroDef *def = macro_defs.pop();
	MacroDef *last;
	while (def != NULL)
	    {
	    last = def;
	    def = def->next_with_name;
	    delete last;
	    }
	}
    }

void MacroExpansion::push_body_text(const char *text,int len)
    {
    while ((len > 0) && (*text != 0))
	{
	// fprintf(stderr,"%c",*text);
	current->current->push(*text);
	assert(*text != 24);
	text++;
	len --;
	}
    }

void MacroExpansion::push_par_text(const char *text)
    {
    if(text) {
	// fprintf(stderr,text);
        while (*text != 0)
            {
            current->current->push(*text);
	    assert(*text != 24);
            text++;
            }
	}
    }

class foreach_stack_entry
    {
	LString name;
	MacroObjectPtr object;
	MacroIterPtr iter;
	const char *start_pos;	// start position in text being expanded
	bool is_copy;
    public:
	foreach_stack_entry(String the_name,MacroObjectPtr the_object,const char *t,bool is_iter) 
		: name(the_name),
		  object(the_object),
		  iter(0),
		  start_pos(t),is_copy(false) { 
	    if (is_iter)
                iter = to_MacroIter(object.get_ptr());
	    }

	foreach_stack_entry(String the_name,const char *t,foreach_stack_entry &x)
		: name(the_name),object(x.object),iter(x.iter),start_pos(t),is_copy(true)
	    {
	    }

	void set_is_copy()
	    {
	    is_copy = true;
	    }

	bool get_is_copy() {return is_copy;}

	bool is_iter() {return !iter.is_null();}

	~foreach_stack_entry()
	    {
	    }

	bool Advance()
	    {
	    if (iter.is_null())
		return false;
	    return iter->Next();
	    }

	bool isFirst() const
	    {
	    if (iter.is_null())
		return true;
	    return (iter->pos() == 0);
	    }

	bool isDone() const
	    {
	    if (iter.is_null())
		return true;
	    return iter->isDone();
	    }

	bool isLast() const
	    {
	    if(iter.is_null())
		return false;
	    return iter->isLast();
	    }

	const LString & get_name() {return name;}

	void set_name(const LString & n) {name = n;}

	MacroObjectPtr get_object() const
	    {
	    if (iter.is_null())
		return object;
	    if (isDone())
		return NULL;
	    return iter->Item();
	    }

	const char * get_start_pos() 
	    {
	    return start_pos;
	    }

	int pos()
	    {
	    if (iter.is_null())
		return 0;
	    return iter->pos();
	    }
	    
    };

MacroExpansion::~MacroExpansion()
    {
    while (files.len() > 0)
	delete files.pop();
    while (macro_defs.len() > 0) {
	MacroDef *def = macro_defs.pop();
	while (def) {
	    MacroDef *ndef = def->next_with_name;
	    delete def;
	    def = ndef;
	    }
	}
    while (foreach_stack.len() > 0) {
	delete foreach_stack.pop();
	}
    while (sources.len() > 0)
        delete sources.pop();
    }

void MacroExpansion::pop_foreach_stack()
    {
    delete  foreach_stack.pop();
    }



//	Find an object with the gen name in the ForEach stack
foreach_stack_entry *MacroExpansion::get_foreach_object(const LString &name)
    {
    int i = foreach_stack.len() - 1;
    while (i >= 0)
        {
        if (foreach_stack[i]->get_name() == name)
            {
            break;
            }
        i --;
        }

    if (i >= 0)
	return foreach_stack[i];
    else
	return NULL;
    }

//	Skip a keyword

bool MacroExpansion::skip_keyword(const char * keyword,const char *error_msg)
    {
    String par_name;
    int idlen = get_name(text,par_name);

    if ((idlen ==0) || !string_match_case((const char *)par_name,keyword,true))
        {
	error_message(error_msg);
	return false;
	}
    text = skip_space_and_comments(text + idlen);
    return true;
    }


//	Find an object from a name. The name may contain periods.
//	We shall be positioned at the first period (if any) as the
//	first name has alrady been read

static bool global_diagnose = true;  // need to do better than global flag
MacroObjectPtr MacroExpansion::get_named_object(const LString &param_name, bool diagnose, MacroObjectPtr add_object)
{
    LString name = param_name;
    MacroObjectPtr last_good_par = NULL;
    bool create = (add_object != NULL);
    LString full_name = name;

    if (create) {
	AbstractMacroListObject *mobj = to_AbstractMacroListObject(add_object.get_ptr());
    	if (mobj != NULL) {
	    	mobj->set_is_not_temp();
	    }
	}

    foreach_stack_entry *foreach = get_foreach_object(name);

    MacroObjectPtr next_par = par;

    if (foreach != NULL) {
	next_par = foreach->get_object();
	}
    else
	next_par = par->get_child(name);

    if ((next_par.is_null()) && create) {
        AbstractNamedList *nobj = to_AbstractNamedList(par);
		
        if ((*text == '.') || (*text == '[')) {
            next_par = new NamedList();
            }
        else {
            next_par = add_object;
            }

	if (nobj == NULL) {
	    String err = "name ";
	    err += full_name.c_str();
	    err += " cannot be set as it is not a named list - actual type is ";
	    err += par->object_type_name().c_str();
	    AbstractStringMacroObject * sobj = to_AbstractStringMacroObject(par);
	    if (sobj != NULL) {
		err += " value is ";
		err += sobj->get_text();
		}
	    error_message(err);
	    }
	else {
            nobj->AddObject(name,next_par);
	    }
	}

    text = skip_space_and_comments(text);
    while ((*text == '.') || (*text == '[')) {
	if (*text == '.') {
	    text ++;
	    if (*text == '?') {
	    	text ++;
	    	name = "?";
	    	}
	   else {
	        String local_name;
	       	int idlen = get_name(text,local_name);
		name = local_name;
	       	text = text +  idlen;
		}
	    }
	else {
	    text ++;
	    const char *start = text;
	    skip_brackets('[',']');
	    name = get_expanded_value(start,text-start-1);
	    }

	const char * cur_pos = text;
	text = skip_space_and_comments(text);
	if ((*text == '[') && (text != cur_pos))
	    	break;

	if (global_diagnose && diagnose && !next_par.is_null()) {
            String s_full_name(full_name);
	    s_full_name += ".";
	    s_full_name += name.c_str();
	    full_name = s_full_name;
	    }
	if (!next_par.is_null()) {
	    last_good_par = next_par;
	    static LString q("?");
	    if (name == q) {
		AbstractNamedList *nobj = to_AbstractNamedList(last_good_par);
		if (nobj != NULL) {
		    next_par = nobj->get_as_list();
		    }
	 	else {
		    AbstractMacroListObject *nobj = 
			to_AbstractMacroListObject(last_good_par.get_ptr());
		    if (nobj != NULL) {
			next_par = nobj;
			}
		    else {
		        error_message("Wildcard not applied to name list");
		        }
		    }
		if (create)
		    error_message("Wild card encountered while attempting to define name");
		    create = false;
		    }
	    	else {
	            next_par = next_par->get_child(name);
		    }
	    	if ((next_par.is_null()) && create) {
                    AbstractNamedList *nobj = to_AbstractNamedList(last_good_par);
		    if ((last_good_par != NULL) && (nobj == NULL)) {
            	    	MacroIter *iter = to_MacroIter(last_good_par.get_ptr());
		    	if (iter != NULL)
                	    nobj = to_AbstractNamedList(iter->Item());
		    	}
		if (nobj == NULL) {
		    String err = "cannot set value because ";
		    err += full_name.c_str();
		    err += " is not a name list";
		    error_message(err);
		    break;
		    }
		if ((*text == '.') || (*text == '[')) {
	   	    next_par = new NamedList();
		    }
	  	else {
		    next_par = add_object;
		    }
		nobj->AddObject(name,next_par);
		}
	    }
	}
    if ((next_par.is_null()) && diagnose && global_diagnose) {
		String error = "warning: name ";
		error += full_name.c_str();
		error += " not found";
		error_message((const char *)error);
		if (last_good_par != NULL) {
	    	next_par =  last_good_par->get_child(name);
	    	fprintf(stdout,"valid names are %s\n",(const char *)last_good_par->child_name_list());
		}
  	
	}
    return next_par;
}

//	As above except that the first name is not yet read
MacroObjectPtr MacroExpansion::get_named_object( bool diagnose, MacroObjectPtr add_object)
    {
    String name;
    text = skip_space_and_comments(text);
    int idlen =  get_name(text,name);
    text = skip_space_and_comments(text + idlen);
    return get_named_object(name,diagnose,add_object);
    }

void MacroExpansion::inherit_pars(MacroExpansion &y)
    {
    macro_defs = y.macro_defs;
    par = y.par;
    foreach_stack = y.foreach_stack;
    }

String MacroExpansion::get_expanded_value(const char *start,int length)
    {
    // fprintf(stderr,"expanding text %s\n",(const char *)text);
    String get_expanded("get_expanded_value call");
    MacroDef def(get_expanded);
    OutputFile *save_current = current;
    current = new OutputFile("@@@@@@");
    files.push(current);
    OutputFile *snails = current;
    def.text_of_def = String(start,length);
    def.filename_of_def = filename_positions.top();
    def.original_text = start;

    macro_calls.push(&def);
    expand_inner(def.filename_of_def, def.text_of_def);
    macro_calls.pop();
    String result = snails->get_text();
    if (files.top() == snails) {
    	delete files.pop();
	}
    current = save_current;
    return result;
    }

const char *skip_past_command(const char * text)
    {
    text = skip_past(text,'>');
    text = skip_space_and_comments(text);
    return text;
    }

bool MacroExpansion::define_name(const char *par_name) {
    if (*text == '"') {
	String buff;
    	const char *start = text;
	int len = get_string(text,buff);
	text = text + len;
	StringMacroObject * obj = new StringMacroObject(buff);
	foreach_stack.push(new foreach_stack_entry(par_name,obj,start,false));
	return true;
	}
    else if (*text == '[') {
    	text ++;
    	const char *start = text;
	skip_brackets('[',']');
	String val = get_expanded_value(start,text-start-1);
    	StringMacroObject * obj = new StringMacroObject(val);
    	foreach_stack.push(new foreach_stack_entry(par_name,obj,start,false));
	return true;
        }
    else {
	String name;
	text = skip_space_and_comments(text);
	int idlen =  get_name(text,name);
	text = skip_space_and_comments(text + idlen);
	foreach_stack_entry *object = NULL;
	if (*text != '.') {
	    foreach_stack_entry *foreach = get_foreach_object(name);
	    if ((foreach != NULL) && foreach->is_iter()) {
	       	object = new foreach_stack_entry(par_name,text,*foreach);
	    	}
	    }
	if (object == NULL) {
	    MacroObjectPtr obj = get_named_object(name,false,NULL);
	    if (obj.is_null())
	    	return false;
	    object = new foreach_stack_entry(par_name,obj,text,false);
	    }
	foreach_stack.push(object);
	return true;
    	}
    return false;
    }

bool MacroExpansion::expand_macro(String name) {

    // simple check for run-away scripts. Should be controllable from
    // the command line.

    if (macro_calls.len() > 10000) {
	fprintf(stderr,"macro calls more than 10000 levels deep\n");
	fprintf(stderr,"you are probably in a recursive loop\n");
	fprintf(stderr,"if not - you need to make this check command line controllable\n");
	fprintf(stderr,"macro name was %s\n",(const char *)name);
	push_par_text("/* macro expansion truncated*/");
	return true;
	}
    // make sure we complete the macro name - there can be [] expressions involved
    bool debug = false;
    const char *start = text;
    while (*text == '[') {
        debug = true;
        text ++;
        const char *start = text;
        skip_brackets('[',']');
	name = get_expanded_value(start,text-start-1);
        if (isalpha(*text) || (*text == '_')) {
            String par_name;
            int idlen =  get_name(text,par_name);
            text = text + idlen;
            name += par_name;
            }
        }

    // evaluate parameters. We don't know the names yet, so make them all empty

    int foreach_pos = foreach_stack.len();
    // push the parameters onto the foreach stack
    // parameters take the form name or "text".
    int par_no = 0;
    text = skip_space_and_comments(text);
    while ((*text != 0) && (*text != '>')) {
	if (*text == ',') {
	    text = skip_space_and_comments(text + 1);
	    StringMacroObject*  obj = new StringMacroObject("");
            foreach_stack.push(new foreach_stack_entry("",obj,start,false));
	    continue;
	    }
        else {
            define_name("");
            }

        text = skip_space_and_comments(text);
	if (*text == ',')
	    text = skip_space_and_comments(text + 1);
        par_no ++;
        }
    text = skip_past(text,'>');
    
    MacroDef *def = resolve_macro(name,foreach_stack,foreach_pos,
			foreach_stack.len()-foreach_pos);
    if (def != NULL) {

	// now we kinow the names of the parameters so set them

	for (int i = 0;i < foreach_stack.len()-foreach_pos; i++) {
	    foreach_stack[foreach_pos + i]->set_name(def->par_names[i].name);
	    // printf(" %s=",(const char *)def->par_names[i].name);
	    // foreach_stack[foreach_pos + i]->get_object()->Print(0);
	    }

    	macro_calls.push(def);
    	expand_inner(def->filename_of_def, (const char *)def->text_of_def);
    	macro_calls.pop();
	}
    else {
	text = start;
	}

    while (foreach_stack.len() > foreach_pos)
	{
	pop_foreach_stack();
	}
    return (def != NULL);
    }


static const char *structured_skip(const char *text,const char *start,const char *end,const char **closing_dir_pos = NULL)
    {
    int start_len = strlen(start);
    int end_len = strlen(end);
    const char * last_dir_pos = text;
    if (closing_dir_pos != NULL)
        *closing_dir_pos = last_dir_pos;

    while (1)
        {
        if (*text == 0)
            return text;
        if (*text == '<')
            {
            if (string_match_case(text,start,true))
                {
                text = structured_skip(text + start_len,start,end);
                }
            else if (string_match_case(text,end,true))
                {
		last_dir_pos = text;
                text += end_len;
                while ((*text != 0) && (*text != '>'))
                    text ++;
                text ++;
		break;
                }
            else
                {
                text ++;
                }
            }
        else
            text ++;
	text = skip_space_and_comments(text);
        }
    if (closing_dir_pos != NULL)
	*closing_dir_pos = last_dir_pos;
    return skip_space_and_comments(text);
    }

const char *skiplet(const char *text)
    {
    return structured_skip(text,"<let","<endlet");
    }

const char *skipforeach(const char *text)
    {
    return structured_skip(text,"<foreach","<endfor");
    }

const char *skipif(const char *text)
    {
    const char * start = "<if";
    const char * end = "<endif";
    const char * elsec = "<else";
    const char * elseif = "<elseif";
    int start_len = strlen(start);
    int end_len = strlen(end);
    int elsec_len = strlen(elsec);
    while (1)
        {
        if (*text == 0)
            return text;
        if (*text == '<')
            {
            if (string_match_case(text,start,true))
                {
                text = structured_skip(text + start_len,start,end);
                }
            else if (string_match_case(text,end,true))
                {
		text = skip_past(text + end_len,'>');
		return skip_space_and_comments(text);
                }
	    else if (string_match_case(text,elsec,true))
		{
		text = skip_past(text + elsec_len,'>');
		return skip_space_and_comments(text);
		}
	    else if (string_match_case(text,elseif,true))
		{
		return text;
		}
            else
                {
                text ++;
                }
            }
        else
            text = skip_space_and_comments(text+1);
        }
    return text;
    }

static bool is_numeric(const char *str) {
   const char *x = str;
   if ((*x == '+') || (*x == '-')) {
      x ++;
      }
   while ((*x >= '0') && (*x <= '9')) {
      x ++;
      }
   return ((x != str) && (*x == 0));
   }

static long get_long_val(const char *x) {
    bool neg = (*x == '-');
    long val = 0;
    if ((*x == '+') || (*x == '-')) {
        x ++;
        }
    while ((*x >= '0') && (*x <= '9')) {
        val = 10 * val - (*x - '0');
        x ++;
        }
    if (neg)
        return val;
    else
        return -val;
    }

class parameter
    {
    public:
	String par_value;

	parameter() {}
	parameter(const char *text) : par_value(text) {}

	parameter(const String x) : par_value(x) {}

	parameter(const parameter &x) : par_value(x.par_value) {}

	parameter(bool x) : par_value(x) {}

	parameter(int x) : par_value(x) {}

	operator bool() {
	    return string_match_alpha(par_value.c_str(),"true");
	    }

	operator int() {
	    int i;
	    sscanf(par_value.c_str(),"%d",&i);
	    return i;
	    }

 	bool get_bool_val()
	    {
	    return string_match_alpha(par_value.c_str(),"true");
	    }

	bool is_bool() {
	    return (string_match_alpha(par_value.c_str(),"true") ||
			string_match_alpha(par_value.c_str(),"false"));
	    }

	int compare(parameter &x) {
	    const char *str = par_value.c_str();
	    const char *xstr = x.par_value.c_str();

	    if (string_match_alpha(str,"true")) {
		if (string_match_alpha(xstr,"true"))
		    return 0;
                if (string_match_alpha(xstr,"false"))
                    return 1;
		}
	    else if (string_match_alpha(str,"false")) {
                if (string_match_alpha(xstr,"true"))
                    return -1;
                if (string_match_alpha(xstr,"false"))
                    return 0;
                }

	    if (is_numeric(str) && is_numeric(xstr)) {
		long val = get_long_val(str);
		long xval = get_long_val(xstr);
		return ((long)val - (long)xval);
		}
		
	    return (par_value.compare(x.par_value));
	    }

	bool operator == (parameter &x) {
	    return (compare(x) == 0);
	    }

        bool operator != (parameter &x) {
            return (compare(x) != 0);
            }

        bool operator <= (parameter &x) {
            return (compare(x) <= 0);
            }
        bool operator >= (parameter &x) {
            return (compare(x) >= 0);
            }
	bool operator < (parameter &x) {
            return (compare(x) < 0);
            }
        bool operator > (parameter &x) {
            return (compare(x) > 0);
            }

	bool operator && (parameter &x) {
	    return (get_bool_val() && x.get_bool_val());
	    }

        bool operator || (parameter &x) {
            return (get_bool_val() || x.get_bool_val());
            }

	const char *get_text() {
	    return par_value.c_str();
	    }
    };


//	evaluate an <if condition

simple_stack <parameter> par_stack(10,10);

parameter MacroExpansion::condition_true()
    {
    int this_call_bottom = par_stack.len();
    while (1)
	{
	text = skip_space_and_comments(text);
  	if ((*text == 0) || (*text == '>') || (*text == ')'))
	    {
	    break;
	    }

        if (isnumeric(*text))
            {
            String par_name;
            int idlen =  get_name(text,par_name);
            if (idlen == 0) {
		par_stack.set_len(this_call_bottom);
                return false;
                }
            text += idlen;
	    par_stack.push(par_name);
	    continue;
	    }

	if (isalpha(*text))
	    {
	    String par_name;
	    int idlen =  get_name(text,par_name);
	    if (idlen == 0) {
		par_stack.set_len(this_call_bottom);
                return false;
                }
	    text += idlen;
	    static LString first_string("first");
	    static LString last_string("last");
	    static LString exists_string("exists");
	    LString par_lname(par_name);
            if (par_lname == first_string)
                {
                text = skip_space_and_comments(text);
                idlen =  get_name(text,par_name);
                text += idlen;
		foreach_stack_entry *entry = get_foreach_object(par_name);
		par_stack.push((entry != NULL) && entry->isFirst());
                }
            else if (par_lname == last_string)
                {
                text = skip_space_and_comments(text);
                idlen =  get_name(text,par_name);
                text += idlen;
                foreach_stack_entry *entry = get_foreach_object(par_name);
                par_stack.push((entry != NULL) && entry->isLast());
                }
	    else if (par_lname == exists_string)
	   	{
		MacroObjectPtr p = get_named_object(false,NULL);
		par_stack.push(p != NULL);
		}
	    else
		{
	    	MacroObjectPtr p = get_named_object(par_lname,true,NULL);
	    	if (p.is_null())
	            par_stack.push("");
	    	else
	            par_stack.push(p->get_text());
		}
	    continue;
	    }
	switch (*text)
	    {
	    case '"':
		{
		text ++;
		String s;
		while (1)
		    {
		    if (*text == 0) {
			par_stack.set_len(this_call_bottom);
                        return false;
                        }

		    if (*text == '"')
			break;
		    if (*text == '\\')
		  	{
			text ++;
			}
		    s.push(*text);
		    text ++;
		    }
		par_stack.push(s);
		if (*text == '"')
		    text ++;
		}
		break;
	    case '=':
		{
		text ++;
		if (*text == '=')
		    text ++;
		if (par_stack.len() <= this_call_bottom) {
                    return false;
                    }

		parameter rt = condition_true();
		parameter lf = par_stack.pop();

		par_stack.push(lf == rt);
		}
		break;

	    case '+':
		{
		text ++;
	 	if (par_stack.len() <= this_call_bottom)
		    return 0;
	 	parameter rt = condition_true();
                parameter lf = par_stack.pop();
		// printf("plus: %d + %d\n",(int)lf,(int)rt);
		par_stack.push((int)lf + (int)rt);
		}
		break;
           case '-':
                {
                text ++;
                if (par_stack.len() <= this_call_bottom)
                    return 0;
                parameter rt = condition_true();
                parameter lf = par_stack.pop();
		// printf("minus: %d - %d\n",(int)lf,(int)rt);
                par_stack.push((int)lf - (int)rt);
                }
                break;

            case '*':
                {
                text ++;
                if (par_stack.len() <= this_call_bottom)
                    return 0;
                parameter rt = condition_true();
                parameter lf = par_stack.pop();
		// printf("times: %d * %d\n",(int)lf,(int)rt);
                par_stack.push((int)lf * (int)rt);
                }
                break;
           case '/':
                {
                text ++;
                if (par_stack.len() <= this_call_bottom)
                    return 0;
                parameter rt = condition_true();
                parameter lf = par_stack.pop();
		int divisor = (int)rt;
		if (divisor == 0) {
		    par_stack.set_len(this_call_bottom);
		    return false;
		    }
		// printf("divide: %d / %d\n",(int)lf,divisor);
                par_stack.push((int)lf / divisor);
                }
                break;



	    case '!':
		{
                text ++;
                if (*text == '=')
		    {
                    text ++;
                    if (par_stack.len() <= this_call_bottom) {
                        return false;
                        }


                    parameter rt = condition_true();
                    parameter lf = par_stack.pop();
                    par_stack.push(lf != rt);
		    }
		else
		    {
		    parameter rt = condition_true();
		    par_stack.push(!(bool)rt);
		    }
		}
		break;
	    case '>':
		{
                text ++;
		char ch = *text;
                if (*text == '=')
                    text ++;
                if (par_stack.len() <= this_call_bottom) {
                    return false;
                    }

                parameter rt = condition_true();
                parameter lf = par_stack.pop();
	 	if (ch == '=')
		    {
		    par_stack.push(lf >= rt);
		    }
		else
		    {
                    par_stack.push(lf > rt);
		    }
		}
		break;
	    case '<':
		{
                text ++;
		char ch = *text;
                if (*text == '=')
                    text ++;
                if (par_stack.len() <= this_call_bottom) {
                    return false;
                    }

                parameter rt = condition_true();
                parameter lf = par_stack.pop();
		// printf("comparing %s to %s\n",lf.get_text(),rt.get_text());
                if (ch == '=')
                    {
                    par_stack.push(lf <= rt);
                    }
                else
                    {
                    par_stack.push(lf < rt);
                    }
		}
		break;
	    case '&':
		{
                text ++;
                if (*text == '&')
                    text ++;
                if (par_stack.len() <= this_call_bottom) {
                    return false;
                    }

                parameter rt = condition_true();
                parameter lf = par_stack.pop();
                par_stack.push(lf && rt);
		}
		break;
	    case '[':
		{
		text ++;
            	const char *start = text;
            	skip_brackets('[',']');
            	String name = get_expanded_value(start,text-start-1);
            	text = skip_space_and_comments(text);
		par_stack.push(name);
		}
		break;

	    case '|':
		{
                text ++;
                if (*text == '|')
                    text ++;
                if (par_stack.len() <= this_call_bottom) {
                    return false;
		    }
                parameter rt = condition_true();
                parameter lf = par_stack.pop();
                par_stack.push(lf || rt);
		}
		break;
	    case '(':
		{
		text ++;
		par_stack.push(condition_true());
		if (*text != 0)
		    text ++;
		}
		break;
		
	    default:
		par_stack.set_len(this_call_bottom);
		return false;
	    }
	}

    if (par_stack.len() != (this_call_bottom + 1))
	{
	par_stack.set_len(this_call_bottom);
	return false;
	}
    return par_stack.pop();
    }

void MacroExpansion::reset_text()
    {
    while (files.len() > 0)
        {
        delete files.pop();
        }
    }


void MacroExpansion::Expand(const char *filename, const char *text,MacroObjectPtr param)
    {
    step = false;
    step_over = false;
    reset_text();
    files.push(new OutputFile(""));
    current = files[0];
    macro_calls.reset();
    filename_positions.reset();
    start_positions.reset();
    saved_positions.reset();
    expanded = false;
    foreach_stack.reset();
    par = param;
    start = text;
    text_len = strlen(text);
    sources.push(new source_files("INPUT",text));
    expand_inner(filename, text);
    return;
    }


void MacroExpansion::process_foreach_clause()
    {
    String par_name;
    int idlen = get_name(text,par_name);

    if (idlen == 0)
	{
	error_message("Badly formed foreach clause");
	text = skipforeach(text);
	return;
	}
    
    String foreach_name = par_name;
    
    text = skip_space_and_comments(text + idlen);
    
    if (!skip_keyword("in","'in' missing in foreach"))
	{
	text = skipforeach(text);
	return;
	}
    
    MacroObjectPtr obj = get_named_object(false,NULL);
    
    if (obj.is_null())
	{
	text = skipforeach(text);
	return;
	}
    MacroIterPtr iter = obj->get_iter();
    if ((!iter.is_null()) && (iter -> isDone()))
	{
	text = skipforeach(text);
	return;
	}
    text = skip_space_and_comments(text);
    while (*text != '>')
	{
	if (!skip_keyword("such","expected > or such that in foreach"))
	    {
	    text = skipforeach(text);
	    return;
	    }

	if (!skip_keyword("that","expected > or such that in foreach"))
	    {
	    text = skipforeach(text);
	    return;
	    }
	
	const char *start = text;
	const char * end_text;
	MacroListObject new_list;
	new_list.reset();
	global_diagnose = false;
	while (! iter ->isDone())
	    {
	    text = start;
	    foreach_stack.push(new foreach_stack_entry(foreach_name,iter->Item(),text,false));
	    if (bool(condition_true()))
		{
		new_list.AddObject(iter->Item());
		}
	    pop_foreach_stack();
	    end_text = text;
	    iter->Next();
	    }
	global_diagnose = true;
	iter = new_list.get_iter();
	}
    
    if ((iter.is_null()) || iter->isDone())
	{
	text = skipforeach(text);
	return;
	}
    text = skip_past_command(text);
    foreach_stack.push(new foreach_stack_entry(foreach_name,iter,text,true));
    }

void MacroExpansion::skip_brackets(char left,char right)
    {
    int i = 0;
    while ((*text != 0) && (i >= 0))
	{
	if (*text == '\\')
	    text ++;
	else if (*text == left)
	    i++;
	else if (*text == right)
	    i--;
	text ++;
	}
    }

void MacroExpansion::select_file(const String &name)
    {
    for (int i= 0;i<files.len();i++)
        {
        if (files[i]->filename == name)
            {
            current = files[i];
            return;
            }
        }
    current = new OutputFile(name);
    files.push(current);
    }

void MacroExpansion::parse_file(const String &name)
    {
    int i = 0;
    while ((i < files.len()) && (files[i]->filename != name))
	i++;
    if (i >= files.len())
	return;
    String x = files[i]->get_text();
    get_expanded_value((const char *)x,x.length());
    }

void MacroExpansion::process_let()
    {
    while (*text != '>')
	{
	String par_name;
    	int idlen = get_name(text,par_name);
    	if (idlen == 0)
            {
	    error_message("name missing in let");
	    text = skiplet(text);
            return;
            }
	
    	String let_name = par_name;

    	text = skip_space_and_comments(text + idlen);
	    
    	if (!skip_keyword("be","'be' missing in let statement"))
	    {
	    text = skiplet(text);
	    return;
	    }

        if (!define_name(par_name))
	    {
	    text = skiplet(text);
	    return;
	    }
	text = skip_space_and_comments(text);
	if (*text == ',')
	    text = skip_space_and_comments(text + 1);
	}
    text = skip_past_command(text);
    }


void MacroExpansion::expand_inner(const char *filename, const char *the_text)
    {
    // static int depth = 0;
    // fprintf(stderr,"expand inner %d file %s text %20s\n",depth,filename,the_text);
    // depth ++;
    String par_name;
    bool saved_step_over = step_over;
    step_over = false;

    bool notice_linefeeds = false;

    int idlen;
    // foreach_stack.reset();
    // expanded = false;
    // reset();

    saved_positions.push(text);
    text = the_text;
    start_positions.push(text);
    filename_positions.push(filename);
    text = skip_space_and_comments(text,true);
    const char *next_to_put = text;

    if ((start_positions.len() == 1) && debug_it)
	process_debug_commands();

    const char *top_of_loop_text = NULL;
    if (debug_it)
	top_of_loop_text = normalize_pos(skip_space_and_comments(text));
    while (*text) {
	if (debug_it)
	    {
	    const char *ntext = normalize_pos(text);
	    bool call_debug = false;
	    for (int i = 0;i < breakpoints.len(); i++)
		{
		if (breakpoints[i].filenbr > 0)
		    {
		    int filenbr = breakpoints[i].filenbr;
		    if (filenbr >= files.len())
			continue;
		    if (files[filenbr-1]->get_lines() >= breakpoints[i].lineno)
			{
			call_debug = true;
			breakpoints[i].filenbr = 10000;
			break;
			}
		    }
		}
	    if (is_in_range(top_of_loop_text) && is_in_range(ntext))
		{
	    	if (!iswhitespace(*text) && (top_of_loop_text <= ntext) && (step || step_over))
		    call_debug = true;
	    	for (int i = 0;i < breakpoints.len(); i++)
	            {
	            const char *bpos = breakpoints[i].text;
		    if (breakpoints[i].filenbr > 0)
			continue;
	            if ((top_of_loop_text < bpos) && (bpos <= ntext))
		        {
		        printf("hit breakpoint %d\n",i + 1);
		        call_debug = true;
		        }
	            }
		}
	    if (call_debug)
		process_debug_commands();
	    top_of_loop_text = normalize_pos(skip_space_and_comments(text));
	    }
	if (*text == '\\')
	    {
	    push_body_text(next_to_put,text-next_to_put);
	    next_to_put = text + 1; // skipping the quote character
	    text ++;
	    if (*text != 0)
		{
		text++;
		}
	    continue;
	    }
	else if (*text == '\n')
	    {
	    const char *trim = text - 1;
	    while ((trim > next_to_put) && iswhitespace(*trim))
		trim --;
	    push_body_text(next_to_put,trim-next_to_put + 1);
	    if (notice_linefeeds)
		{
	        push_body_text(text,1);
                for (int i=0;i<current->current->indent;i++)
                    push_par_text(" ");
	 	}
	    text = skip_space_and_comments(text + 1,true);
	    next_to_put = text;
	    continue;
	    }
	else if (*text != '<')
	    {
	    text ++;
	    continue;
	    }

	push_body_text(next_to_put,text-next_to_put);
	next_to_put = text;
	text ++;
	// error_message("next command is");

	// <> can be used to "continue" to next line

	if (*text == '>')
	    {
	    text = skip_space_and_comments(text + 1);
	    next_to_put = text;
	    continue;
	    }
	if (*text == '/')
	    {
	    text = skip_space_and_comments(text + 1);
	    if (*text == '=')
                {
                int val;
                text = skip_space_and_comments(text + 1);
                int numlen = get_value(text,val);
                text = skip_space_and_comments(text + numlen);
                current->current->indent = val;
		if(current->current->indent < 0)
                    current->current->indent = 0;
                }

	    if (*text == '+')
		{
		int val;
		text = skip_space_and_comments(text + 1);
		int numlen = get_value(text,val);
		text = skip_space_and_comments(text + numlen);
		if(current->current->indent < 0)
		    current->current->indent = 0;
		current->current->indent += val;
		}
	    else if(*text == '-')
                {
                int val;
                text = skip_space_and_comments(text + 1);
                int numlen = get_value(text,val);
                text = skip_space_and_comments(text + numlen);
                current->current->indent -= val;
		if (current->current->indent < 0)
		    current->current->indent = 0;
                }
	    else if(*text == '0')
		{
		/* </0> can appear on either side of something 
		    (like a pragma) that you do not want indented
		 */
		static int saved_indent = 0;
		text = skip_space_and_comments(text + 1); 
		if (saved_indent == 0)
			{
			saved_indent = current->current->indent;
			current->current->indent = 0;
			}
		else
		        {
			 current->current->indent = saved_indent;
			 saved_indent = 0;
			}
		}
	    else
		{
		push_par_text("\n");
		for (int i=0;i<current->current->indent;i++)
		    push_par_text(" ");
		}


	    text = skip_past(text,'>');
            next_to_put = text;
            continue;
	    }
	if (*text == '!')
	    {
	    String command;
	    text = skip_space_and_comments(text + 1);
	    if ((*text == '"') || (*text == '\''))
		{
		command.push(*text);
		text = skip_space_and_comments(text + 1);
		}
	    else
		{
            	idlen = get_name(text,command);
		text = skip_space_and_comments(text + idlen);
		}
	    MacroObjectPtr obj = get_named_object(false,NULL);
	    String extras;
	    while (*text != '>')
		{
		extras.push(*text);
		text ++;
		}
	    push_par_text_command(command,obj,extras);
            text = skip_past(text,'>');
	    next_to_put = text;
	    continue;
	    }


	if (*text == '?')   // optional text depending upon existence of a parameter
	    {
	    text ++;
            MacroObjectPtr obj = get_named_object(false,NULL);
            next_to_put = text;
            text = skip_past(text,'>');

	    if (obj != NULL)
		{
		push_body_text(next_to_put,text-next_to_put-1);
		}
	    next_to_put = text;
	    continue; 
	    }

 	if (*text == '[')
	    {
	    if (expand_macro(""))
                {
                next_to_put = text;
                continue;
		}
            }

	idlen = get_name(text,par_name);
	if (idlen == 0)
	    {
            continue;
            }

 	const char *the_name = (const char *)par_name;
	const char *end_of_name = text + idlen;
	text = skip_space_and_comments(text + idlen);

	switch (get_command_index(the_name)) {
          case notice_linefeeds_ordinal: {
	    notice_linefeeds = true;
            text = skip_past_command(text);
            next_to_put = text;
            continue;
            }

	  case ignore_linefeeds_ordinal: {
            notice_linefeeds = false;
            text = skip_past_command(text);
            next_to_put = text;
            continue;
            }

	  case endif_ordinal: {
	    text = skip_past_command(text);
            next_to_put = text;
            continue;
            }

	  case endmac_ordinal: {
	    text = skip_past_command(text);
            next_to_put = text;
	    goto finish;
	    }

	  case endfor_ordinal: {
            // backup on a foreach here
            if (foreach_stack.len() == 0) {
                text = skip_past_command(text);
                continue;
                }
            foreach_stack_entry * top = foreach_stack.top();
            if (!top->Advance()) {
		text = skip_past_command(text);
                pop_foreach_stack();
                next_to_put = text;
                continue;
                }
            text = top->get_start_pos();
            next_to_put = text;
	    if (debug_it) 
        	top_of_loop_text = normalize_pos(skip_space_and_comments(text));

            continue;
            }

          case endlet_ordinal: {
            // backup on a foreach here
	    text = skip_past_command(text);

            if (foreach_stack.len() > 0) {
		pop_foreach_stack();
		}
            next_to_put = text;
            continue;
            }

	  case def_ordinal: {
	    idlen = get_name(text,par_name);
            if (idlen == 0) {
		error_message("missing name in macro def");
                text = skip_past_command(text);
                next_to_put = text;
		if(debug_it)
		    top_of_loop_text = normalize_pos(skip_space_and_comments(text));
                continue;
                }
            text = text + idlen;

	    // see if already exists. If it does, then we are replacing the definition

	    MacroDef *def = new MacroDef(par_name);

	    bool error = false;
	    text = skip_space_and_comments(text);
	    while ((*text != 0) && (*text != '>')) {
		String par;
		String type;
		idlen = get_name(text,par);
            	if (idlen == 0)
                    {
		    String err = "crud in macro parameter list for ";
		    err += par_name;
		    error_message(err);
		    error = true;
		    break;
		    }
	  	text = skip_space_and_comments(text + idlen);
		if (*text == ':') {
		    text = text = skip_space_and_comments(text + 1);
		    idlen = get_name(text,type);
		    if (idlen == 0) {
                    	String err = "crud in macro parameter list for ";
                    	err += par_name;
                    	error_message(err);
                    	error = true;
                    	break;
                    	}
		     text = skip_space_and_comments(text + idlen);
		     }

		def->par_names.push(ParInfo(par,type));
		}
	    if (*text != '>') {
		String err = "missing > for macro ";
		err += par_name;
		error_message(err);
		error = true;
		}
	    if (error) {
		text = skip_past_command(text);
                next_to_put = text;
		delete def;
		top_of_loop_text = skip_space_and_comments(text);
                continue;
                }
	    const char *start = text + 1;
	    const char *endpos;
	    text = structured_skip(text,"<def","<enddef",&endpos);
	    def->filename_of_def = "unknown_in_macro.cpp";
	    def->text_of_def = String(start,endpos - start);
	    def->original_text = normalize_pos(start);
	    text = skip_space_and_comments(text);
	    next_to_put = text;
	    top_of_loop_text = skip_space_and_comments(text);

	    // now enter the macro

	    add_macro(def);
	    continue;
	    }

          case pos_ordinal: {
            idlen =  get_name(text,par_name);
            text += idlen;
            foreach_stack_entry *entry = get_foreach_object(par_name);
	    if (entry != NULL) {
                char text[20];
                sprintf(text,"%d",entry->pos());
                push_par_text(text);
                }
	    text = skip_past(text,'>');
            next_to_put = text;
	    continue;
	    }

	  case else_ordinal:
	  case elseif_ordinal: {
	    text = structured_skip(text,"<if","<endif");
	    next_to_put = text;
            if (debug_it)
                top_of_loop_text = normalize_pos(skip_space_and_comments(text));

	    continue;
	    }

	  case eval_ordinal: {
	    if (*text != '(') {
		text = skip_past_command(text);
		next_to_put = text;
		continue;
		}
	    text = skip_space_and_comments(text+1);
	    parameter par = condition_true();
	    push_par_text(par.get_text());
	    text = skip_past_command(text);
	    next_to_put = text;
            continue;
            }

	  case parse_ordinal: {
            const char * val_start = text;
            skip_brackets('<','>');
            String x = get_expanded_value(val_start,text - val_start - 1);
            parse_file(x);
            next_to_put = text;
            continue;
            }

          case include_ordinal: {
            const char * val_start = text;
            skip_brackets('<','>');
            String fname = get_expanded_value(val_start,text - val_start - 1);
	    String base_name = fname;
	    char * ftext = ::get_file_text((const char *)fname,false);
	    int i = 0;
	    while ((ftext == NULL) && (i < include_dirs.len())) {
		String fname = include_dirs[i] + str_slash + base_name;
		ftext = ::get_file_text((const char *)fname,false);
		i++;
		}
	    if (ftext == NULL) {
		String mess = String("Could not open file:") + base_name;
		error_message((const char *)mess);
		}
	    else {
		sources.push(new source_files(base_name,ftext));
            	get_expanded_value(ftext,strlen(ftext));
		}
            next_to_put = text;
            continue;
            }

	  case file_ordinal: {
	    const char * val_start = text;
            skip_brackets('<','>');
            String x = get_expanded_value(val_start,text - val_start - 1);
	    select_file(x);
	    text = skip_space_and_comments(text);
            next_to_put = text;
            continue;
            }

	  case use_ordinal: {
	    const char * val_start = text;
            skip_brackets('<','>');
            String x = get_expanded_value(val_start,text - val_start - 1);
	    current->set_use_block(x);
	    text = skip_space_and_comments(text);
            next_to_put = text;
            continue;
            }

	  case if_ordinal: {
	    if (*text != '(') {
		text = skipif(text);
		next_to_put = text;
		continue;
		}
	    text ++;
	    if (!((bool)condition_true())) {
		text = skipif(text);
		next_to_put = text;
		while (string_match_case(text,"<elseif",true)) {
		    text = skip_space_and_comments(text + 7);
            	    if (*text != '(')
                	{
                	text = skipif(text);
                	next_to_put = text;
			break;
			}
		    if (bool(condition_true())) {
			text = skip_past_command(text);
			next_to_put = text;
			break;
			}
		    text = skipif(text);
		    }
                if (debug_it)
                    top_of_loop_text = normalize_pos(skip_space_and_comments(text));

		next_to_put = text;
		continue;
		}
           if (*text != ')') {
                text = skipif(text);
                }
	    text = skip_space_and_comments(text + 1);
	    if (*text != '>')
                {
                text = skipif(text);
                }
	    text = skip_space_and_comments(text + 1);
	    next_to_put = text;
	    continue;
	    }

	  case foreach_ordinal: {
	    process_foreach_clause();
	    next_to_put = text;
	    continue;
	    }

          case map_ordinal: {
            MacroObjectPtr obj = get_named_object(true,NULL);
            if (!skip_keyword("to","to missing in set")) {
                skip_brackets('<','>');
                next_to_put = text;
                continue;
                }
	    get_named_object(true,obj);
	    text = skip_past_command(text);
            next_to_put = text;
            continue;
            }

	  case set_ordinal: {
	    MacroObjectPtr obj = get_named_object(true,new StringMacroObject(""));
	    if (!skip_keyword("to","to missing in set")) {
		skip_brackets('<','>');
		next_to_put = text;
            	continue;
            	}

            AbstractStringMacroObject* sobj = to_AbstractStringMacroObject(obj);
	    if (sobj == NULL) {
		error_message("only string objects can be set");
                skip_brackets('<','>');
                next_to_put = text;
                continue;
		}

	    const char * val_start = text;
	    skip_brackets('<','>');
	    String x = get_expanded_value(val_start,text - val_start - 1);
	    sobj->set_text(x);
            next_to_put = text;
            continue;
            }
	    
	  case let_ordinal: {
	    process_let();
            next_to_put = text;
            continue;
	    }

          case show_ordinal: {
            error_message("show called",true);
            MacroObjectPtr obj = get_named_object(true,NULL);
            if (obj.is_null())
                error_message("show called to display non-existant object");
            else
                obj->Print(0);
            text = skip_past_command(text);
	    next_to_put = text;
            continue;
            }


          case echo_ordinal: {
	    const char *val_start = text;
            skip_brackets('<','>');
            String x = get_expanded_value(val_start,text - val_start - 1);
	    error_message(x,true);
            next_to_put = text;
            continue;
            }
	  }

	text = end_of_name;
#if 0
	static int level = 0;
	{
	  for(int i = 0; i < level; i++) {
	    fprintf(stderr, " ");
	  }
	  fprintf(stderr, "Expanding %s\n", the_name);
	  level++;
	}
#endif
	bool exp_bad = expand_macro(the_name);
#if 0
	{
	  for(int i = 0; i < level; i++) {
	    fprintf(stderr, " ");
	  }
	  fprintf(stderr, "End %s\n", the_name);
	  level--;
	}
#endif
	if (exp_bad)
	    {
	    next_to_put = text;
	    continue;
	    }
	MacroObjectPtr value = get_named_object(par_name,true,NULL);
	if(value != NULL)
	    {
	    push_par_text(value->get_text());
	    expanded = true;
	    }
	text = skip_past(text,'>');
	next_to_put = text;
	}
  finish:
    push_body_text(next_to_put,text-next_to_put);
    if ((start_positions.len() == 1) && debug_it)
        process_debug_commands();

    text = saved_positions.pop();
    start_positions.pop();
    filename_positions.pop();
    step_over = step_over | saved_step_over;
    // depth --;
    // fprintf(stderr,"returning\n");
    }

void MacroExpansion::error_message(const char *message,bool use_stdout)
    {
    use_stdout = true;
    send_error(filename_positions.top(), 
	       start_positions.top(),text,message,use_stdout);
    int len = start_positions.len() -1;
    while (len > 0)
	{
	len --;
	String S = "called from:";
	if (macro_calls[len] == NULL)
	    S += "inline expansion";
	else
	    S += macro_calls[len]->name.c_str();
	send_error(filename_positions[len], 
		   start_positions[len],saved_positions[len+1],(const char *)S,use_stdout);
	}
    if (debug_it)
	process_debug_commands();
#if 0
    len = foreach_stack.len();
    printf("foreach stack contains %d items\n",len);
    FILE *out = stderr;
    if (use_stdout)
	out = stdout;
    while (len > 0)
	{
	len --;
	MacroObjectPtr obj = foreach_stack[len]->get_object();
	fprintf(out,"object : %s ",(const char *)foreach_stack[len]->get_name());
 	if (obj == NULL)
	    fprintf(out,"Object is null");
	else
	    obj->Print(0);
	fprintf(out,"\n");
	}
#endif
    }

NamedList * to_NamedList(MacroObjectPtr p)
    {
        if( p && p->get_instance_class_name() == NamedList::get_ClassName() )
            return (NamedList *)p.get_ptr();
        else
            return NULL;
    }

StringMacroObject *  to_StringMacroObject(MacroObjectPtr p)
    {
        if( p && p->get_instance_class_name() == StringMacroObject::get_ClassName() )
            return (StringMacroObject *)p.get_ptr();
        else
            return NULL;
    }

MacroListObject::~MacroListObject()
    {
    list.reset();
    }

void print_indent(int indent)
    {
    fprintf(stdout,"\n");
    for (int j = 0;j < indent;j++)
        fprintf(stdout," ");
    }

void print_start(int indent,const char *type,const char * dynamic_type)
    {
    print_indent(indent);

    fprintf(stdout,"{%s type = %s",type,dynamic_type);
    print_indent(indent);
    }

void print_end(int indent)
    {
    fprintf(stdout,"}");
    print_indent(indent);
    }


void MacroListObject::Print(int indent) const
    {
    print_start(indent,"MacroListObject",object_type_name());
    for (int i = 0;i < list.len();i++)
	{
	list[i]->Print(indent + 4);
	print_indent(indent);
	}
    print_end(indent);
    }


void NamedList::Print(int indent) const
    {
    print_start(indent,"NamedList",object_type_name());

    for (int i = 0;i < names.len();i++)
        {
	fprintf(stdout,"%s=>",(const char *)names[i]);
	get_item(i)->Print(indent + 4);
	print_indent(indent);
        }
    print_end(indent);
    }

class detabbed_text
    {
       	int indent;	// whitespace to ignore at start of lines
    	const char *text;
	int spaces_needed; // spaces needed to fill out tab being replaced
	int colno;
	bool starts_line;
	bool next_starts_line;

	bool strip_indent;

	simple_stack<int> indent_stack;

        char get_detabbed_ch()
            {
	    if (*text == 0)
		return *text;
            if (spaces_needed > 0)
                {
                spaces_needed --;
                colno ++;
                return ' ';
                }
            if (*text == 9)
                {
                spaces_needed = 8 - ((colno + 1) & 0x7);
                text ++;
		spaces_needed --;
                colno ++;
                return ' ';
		}
	    colno++;
	    return *text++;
	    }


    public:
	detabbed_text(const char *t) : indent(0),text(t),spaces_needed(0),colno(0),
			starts_line(true),next_starts_line(true),strip_indent(true),indent_stack(10,10) {}

	char getch() 
	    {
	    if (strip_indent)
		colno = 0;

	    char ch = get_detabbed_ch();
	    if (strip_indent)
		{
		strip_indent = false;
		while ((ch == ' ') && (colno < indent))
		    ch = get_detabbed_ch();
		next_starts_line = true;
		}
	    starts_line = next_starts_line;
	    if (ch == '\n')
		{
		strip_indent = true;
		}
	    else 
		{
		if (ch != ' ')
		    next_starts_line = false;
		}
	    return ch;
	    }

	bool is(const char * t)
	    {
	    if (spaces_needed)
		return false;
	    return prefix_match(text,t);
	    }

	bool is_start_line()
	    {
	    return starts_line;
	    }
	bool set_indent_for_directive();
	};
		
static const char *directives[] = {
	"if",
	"endif",
	"let",
	"endlet",
	"foreach",
	"endfor",
	"def",
	"enddef",
	NULL};

bool detabbed_text::set_indent_for_directive()
    {
    if (spaces_needed)
	return false;
    int i = 0;
    while (1)
	{
    	if (directives[i] == NULL)
	    return false;
    	if (prefix_match(text,directives[i]))
	    {
	    int j = strlen(directives[i]);
	    if (!isalpha(text[j]))
		{
		if ((i & 1) && (indent_stack.len() > 0))
		    {
		    indent = indent_stack.pop();
		    }
		else 
		    {
		    indent_stack.push(indent);
		    indent = colno;
		    }
	    	return true;
		}
	    }
	i++;
	}
    }

//      Execute a command ("function") on the text
//
//      this version can handle the following commands (calling push_par_text to push output)
//      !' enclose parameter in single quotes
//      !" enclose in double quotes
//      !CAPS output in all caps
//      !LOWS output in all lower case
//      !CPZ  capitalizes text. First character is captalized. Underlines are removed and the
//                              next character is capitalized
//      !UNL  lower cases text and puts an underline in front of capitals
//	!SING cuts an s off the end of a name
//
//      !TYPE returns (pushes) the results of calling "p->object_type_name()" 
//
//      NB = the shriek (!) is not passed as part of the command. It is used to invoke the command
void MacroExpansion::push_par_text_command(String command,const MacroObjectPtr p,String extras)
    {
    String val;
    if (p != NULL)
	val = p->get_text();
    if ((command == str_quote) || (command == str_double_quote))
	{
	push_par_text((const char *)command);
	push_par_text((const char *)val);
	push_par_text((const char *)command);
	return;
	}

    else if (command == str_caps) {
	for (int i = 0;i < val.length(); i++)
	    val[i] = UPCASE(val[i]);
	push_par_text((const char *)val);
	return;
	}
    else if (command == str_lows) {
        for (int i = 0;i < val.length(); i++)
            val[i] = LOWCASE(val[i]);
        push_par_text((const char *)val);
        return;
        }
    else if (command == str_sing) {
	int i = val.length();
	if (val.Right(3) == String("ies"))
	    {
	    val = val.Left(-3) + String("y");
	    }
        else if ((i > 0) && ((val[i - 1] == 's') || (val[i - 1] == 'S'))
            & (val[ i - 2] != 's') && (val[i - 2] != 'S'))
	    val.truncate_at_pos(i-1);
        push_par_text((const char *)val);
        return;
        }

    else if (command == str_cpz) {
	String v;
	bool cap = true;
	for (int i = 0;i < val.length(); i++)
	    {
	    char ch = val[i];
	    if (ch == '_')
		{
		cap = true;
		}
	    else
		{
	        if (cap)
		    v.push(UPCASE(ch));
	        else 
		    v.push(ch);
	        cap = false;
		}
	    }
	push_par_text((const char *)v);
        return;
        }

    else if (command == str_unl) {
        String v;
        for (int i = 0;i < val.length(); i++)
            {
            char ch = LOWCASE(val[i]);
	    if (ch != val[i])
		{
		ch = LOWCASE(ch);
		if (i > 0)	// don't put out a leading _
		    v.push('_');
		}
             v.push(ch);
            }
        push_par_text((const char *)v);
        return;
        }
    else if (command == str_llc) {
        val[0] = LOWCASE(val[0]);
        push_par_text((const char*)val);
        return;
    }
    else if (command == str_nspc) {
        String foo;
        int foo_cnt = 0;
        foo = p->get_text(); 
        for (int i = 0;i < val.length(); i++) {
		if (!iswhitespace(val[i]))
            	foo[foo_cnt++] = val[i];
		}
	foo[foo_cnt] = '\0';
	push_par_text((const char*)foo);
	}
    else if (command == str_type) {
       if( p )
            push_par_text(p->object_type_name().c_str());
       else
            push_par_text("");
       }
    else if (command == str_iskindof) {
       if (p && p->is_instance_of(extras))
           push_par_text("true");
       else
           push_par_text("false");
       }
    else {
	String errorMsg = "Non-extant command ";
	errorMsg += command;
	error_message(errorMsg, true);
    }
}

MacroIterPtr MacroIter::get_iter()
    {
    MacroObjectPtr item = Item();
    if (!item.is_null())
	return item->get_iter();
    else {
	return new SingleMacroIter(NULL);
	}
    }

MacroObjectPtr  MacroIter::get_child(const LString &name) const
    {
    if (Item().is_null())
	return NULL;
    return Item()->get_child(name);
    }

String MacroIter::child_name_list() const 
   {
    if (Item().is_null())
	return String();
    return Item()->child_name_list();
    }

void MacroIter::Print(int indent) const
    {
    print_start(indent,"MacroIter",object_type_name());

    if (Item().is_null())
	fprintf(stdout,"<NULL>");
    else
	Item()->Print(indent);
    }
    
const String MacroIter::get_text() const
    {
    if (Item().is_null())
	return "";
    return Item()->get_text();
    }    

MacroIter* to_MacroIter(MacroObject * p)
    {
        if( p && p->get_instance_class_name() == MacroIter::get_ClassName() )
            return (MacroIter*)p;
        else
            return NULL;
    }

MacroListObject * to_MacroListObject(MacroObject * p)
    {
        if( p && p->get_instance_class_name() == MacroListObject::get_ClassName() )
            return (MacroListObject *)p;
        else
            return NULL;
    }

static const String not_there("");
const String & MacroExpansion::get_file_name(int i)
    {
    if ((i < 0) || (i >= files.len()))
	return not_there;
    return files[i]->filename;
    }

String MacroExpansion::get_file_text(int i)
    {
    if ((i < 0) || (i >= files.len()))
        return String("");
    return files[i]->get_text();
    }

void MacroExpansion::add_break(int lineno)
    {
    const char *t = start;
    int i = lineno;
    while ((i > 1) && (*t))
	{
	if (*t == '\n')
	    i--;
	t++;
	}
    if (*t == 0)
	{
	printf("no such line number found - last line is %d\n",lineno-i);
	}
    else
	{
        breakpoints.push(Breakpoint(lineno,t));
	printf("added breakpoint %d at ",breakpoints.len());
	print_lines(start,lineno,lineno);
	}
    }

const char * MacroExpansion::normalize_pos(const char *text)
    {
    int l = macro_calls.len();
    while (l > 0)
	{
	l--;
        MacroDef *current = macro_calls[l];
        const char *s = (const char *)current->text_of_def;
        if ((text >= s) && (text < s + current->text_of_def.length()))
            {
            text = current->original_text + (text - s);
            }
	}
    return text;
    }


void MacroExpansion::get_lineno(const char *text,int &line_no,int &file_no)
    {
    const char *t = NULL;
    static const char *last_text = NULL;
    static int last_line = 0;

    text = normalize_pos(text);
    
    int i = 0;
    while ((i < sources.len()) && (!sources[i]->is_in(text)))
	i++;

    if (i >= sources.len())
	{
	i = 0;
	
	file_no = -1;
	line_no = -1;
	return;
	}

    file_no = i;

    t = sources[i]->get_text();
    int line = 1;

    // may be able to restart search from last position

    if ((t >= last_text) && sources[i]->is_in(last_text))
	{
	t = last_text;
	line = last_line;
	}

    while (t < text)
	{
	if (*t == 0)
	    break;
	if (*t == '\n')
	    line ++;
	t++;
	}
    last_text = text;
    last_line = line;
    line_no = line;
    }

void MacroExpansion::show_breaks()
    {
    for (int i = 0;i < breakpoints.len(); i++)
	printf("%d:line %d:%20s\n",i,breakpoints[i].lineno,breakpoints[i].text);
    }

void MacroExpansion::delete_break(int no)
    {
    if ((no > 0) && (no <= breakpoints.len()))
	breakpoints.remove(no - 1);
    }

void MacroExpansion::clear_breaks()
    {
    breakpoints.reset();
    }

void MacroExpansion::print_lines(const char *start,int first,int last)
    {
    const char *p = start;
    int line = 1;

    if (first < 0)
	{
	int line_count = 0;
	while (*p != 0)
	    {
	    if (*p == '\n')
		line_count ++;
	    p++;
	    }
	first = first + line_count;
	last = last + line_count + 1;
	p = start;
	}
    while (line < first)
	{
	if(*p == 0)
	    break;
	if (*p == '\n')
	    line ++;
	p ++;
	}
    if (last < first)
	last = first;

    while (line <= last)
	{
	char buff[81];
	int i = 0;
	printf("line:%3d:",line);
	while ( (*p != 0) && (*p != '\n'))
	    {
	    buff[i++] = *p++;
	    if (i > 78)
		{
		buff[i] = 0;
		printf("%s",buff);
		i = 0;
		}
	    }
	buff[i++] = 0;
	line ++;
	printf("%s\n",buff);
	if (*p == 0)
	    break;
	p++;
	}
    }

void MacroExpansion::show_pos(const char *stext)
    {
    const char *text = normalize_pos(stext);
    const char *p = text;
    String line;
    String pointer;

    while ((p > start) && (*p == '\n'))
	p--;
    while ((p >= start) && (*p != '\n'))
	{
	p --;
	}

    if (p > start)
	p++;
    while ((*p != 0) && (*p != '\n'))
	{
	line.push(*p);
	if (p == text)
	    pointer.push('^');
	else
	    pointer.push(' ');
	p++;
	}

    int file_no,line_no;
    get_lineno(text,line_no,file_no);
    if (file_no > 0)
	{
	printf("file %s:",(const char *)(sources[file_no]->get_name()));
	}
    printf("line:%3d:%s\n",line_no,(const char *)line);
    printf("         %s\n",(const char *)pointer);
    }

#include <signal.h>

static bool interrupted;

#if defined(IRIX)
static void handler()
{
	interrupted = true;
}
#else
static void handler(int)
    {
    interrupted = true;
    }
#endif


void MacroExpansion::process_debug_commands()
    {
#ifndef MSVC    /* NT chokes on sigaction structs */
    int pr_line,pr_file;
    get_lineno(normalize_pos(text),pr_line,pr_file);
    pr_line -= 5;
    if (pr_line < 1)
	pr_line = 1;
    show_pos(text);
    String command;
    step = false;
    step_over = false;


    while (1)
	{
	printf("debug : ");
#ifndef MSVC
	struct sigaction oldact;

	struct sigaction newact;
	newact.sa_handler = handler;
	sigemptyset(&newact.sa_mask);
	newact.sa_flags = 0;
	sigaction(SIGINT,&newact,&oldact);
#endif
	interrupted = false;
	int ch = getchar();
	command.make_empty();

	// read a command ignoring leadin blanks

	bool leading = true;
	while (ch != '\n')
	    {
	    if (!leading || (ch != ' '))
		{
		leading = false;
		command.push(ch);
		}
	    ch = getchar();
	    if (interrupted)
		{
		interrupted = false;
		printf("\ndebug : ");
		command = "";
		}
	    }

 	sigaction(SIGINT,&oldact,NULL);
	if (leading)
	    {
	    // repeat previous command
	    command = last_command;
	    fflush(stdout);
	    }
	else
	    {
	    last_command = command;
	    }

        switch (command[0])
	    {
	    case 'c':
	    case 'C':
		return;
	    case 'b':
	    case 'B':
		{
		int line;
		int filenbr;
		int len = sscanf(((const char *)command)+1,"%d %d",&line,&filenbr);
		if (len == 2)
		    breakpoints.push(Breakpoint(line,filenbr));
		else
		    add_break(line);
		break;
		}
	    case 'd':
	    case 'D':
                {
                int no;
                int i = sscanf(((const char *)command)+1,"%d",&no);
                if (i > 0)delete_break(no);
                break;
                }
	    case 'f':
	    case 'F':
		{
		int no,count;
		count = 10;
                int i = sscanf(((const char *)command)+1,"%d %d",&no,&count);
		if (i <= 0)
		    {
		    for (i= 0;i<files.len();i++)
        		{
        		printf("file %2d:%s\n",i+1,(const char *)files[i]->filename);
			}
	  	    }
		else
		    {
		    if ((no < 1) || (no > files.len()))
			{
			printf("invalid file number - number of files is %d\n",files.len());
			}
		    else
			{
			print_lines((const char *)(*files[no-1]->get_text()),-count,0);
			}
		    }
		break;    
	  	}
	    case 'l':
	    case 'L':
		{
		int no;
		int i = -1;
		if (!leading)
                    i = sscanf(((const char *)command)+1,"%d",&no);
                if (i > 0)
		    {
		    print_lines(start,no,no+10);
		    pr_line = no + 11;
		    }
		else
		    {
		    print_lines(start,pr_line,pr_line+10);
		    pr_line += 11;
		    }
		break;
		}
	    case 'n':
	    case 'N':
		step_over = true;
		return;
	    case 'p':
	    case 'P':
		{
		debug_it = false;
		String value = get_expanded_value(((const char *)command)+1,command.length() - 1);
		debug_it = true;
		printf("%s = %s\n",((const char *)command)+1,(const char *)value);
		break;
		}
            case 's':
	    case 'S':
                if ((command == String("sb")) || (command == String("SB")))
                    show_breaks();
                else
                    {
                    step = true;
                    return;
                    }
                break;
	    case 't':
	    case 'T':
		{
		const char *saved_text = text;
		text = ((const char *)command)+1;
		text = skip_space_and_comments(text);
		MacroObjectPtr obj = get_named_object(true,NULL);
            	if (obj.is_null())
                    printf("non-existant object\n");
                else
                    obj->Print(0);
		text = saved_text;
		break;
		}
	    case 'w':
	    case 'W':
		{
		int len = macro_calls.len();
		show_pos(text);
    		while (len > 0)
        	    {
        	    len --;
		    printf("called from:%s\n",(const char *)(macro_calls[len]->name));
		    show_pos(saved_positions[len + 1]);
        	    }
		
		break;
		}
	    case 'q':
	    case 'Q':
		while (1)
		    {
		    printf("really quit (y/n)? :");
		    ch = getchar();

		    if ((ch == 'y') || (ch == 'Y'))
			exit(0);
		    if ((ch == 'n') || (ch == 'N'))
			break;
		    while (ch != '\n')
			ch = getchar();
		    }
		break;
	 	
	    case 'h':
	    case 'H':
	    case '?':
		printf("Commands are:\n\n");
		printf("c - continue\n");
		printf("b <lineno> - break at line\n");
		printf("f - print list of files being generated\n");
		printf("f no [count] - print the last count (default 10) lines of file number no\n");
		printf("sb - show breakpoints\n");
		printf("sh - show name\n");
		printf("s - step\n");
		printf("d <no> - delete breakpoint\n");
		printf("l <lineno> - list source\n");
		printf("n - step over macro expansion");
		printf("p expression - print value of expression\n");
		printf("t name - look up the name an print the tree value\n");
	    	printf("w - print out where we are (call stack\n");
		printf("h or ? - this help text\n");
		printf("q - quit the debugger now\n");
		break;
	    default:
		printf("invalid command - use h or ? to see list\n");
		break;
	    }
	}
#endif
    }


bool MacroExpansion::is_in_range(const char *text)
    {
    return ((text >= start) && (text < (start + text_len)));
    }

static bool convert_to_int(const String &x,int &val)
    {
    const char *p = (const char *)x;
    val = 0;
    bool neg = false;
    while (*p == ' ')
	p++;
    if (*p == '+')
	p++;
    else if (*p == '-')
	{
	p++;
	neg = true;
	}
    while (*p == ' ')
        p++;

    while ((*p >= '0') && (*p <= '9'))
	{
	val = 10 * val - (*p - '0');
	p++;
	}
    if (!neg)
	val = -val;
    return (*p == 0);
    }

MacroObjectPtr  AbstractMacroListObject::get_child(const LString &name) const 
    {
    MacroObjectPtr result = NULL;
    AbstractMacroListObject *result_list = NULL;
    AbstractMacroListObject * cur = const_cast<AbstractMacroListObject *>(this);
    int state = 0;

    // special case.
    // if the name is numeric, we want a particular entry from the list

    int val;
    if (convert_to_int(name,val))
	{
	if (val < 0) 
	    return NULL;
	MacroIterPtr iter = cur->get_iter();
	while ((val > 0) && (!iter->isDone()))
	    {
	    val --;
	    iter->Next();
	    }

	if (!iter->isDone())
	    result = iter->Item();
	return result;    
	}

    //	We want to achieve the following:
    //
    //  Create a list of all children satisfying the name. There may be a number
    //  of these in the current list and subsidiary lists of lists, or there may be
    //  precisely one
    //
    //  If there is one, return it.
    //  If there are a number, return a list
    //  We use the following states:
    //
    //  0 - initial state
    //  1 - exactly 1 item has been found, and it is pointed at by result
    //  2 - a list of objects has been found which we must not add extra items to
    //  3 - a list of objects has been found which we may add objects to
    //      Either of these lists is pointed to by result_list
    MacroIterPtr iter = cur->get_iter();
    while (!iter->isDone())
	{
	MacroObjectPtr item = iter->Item()->get_child(name);
	AbstractMacroListObject *litem = to_AbstractMacroListObject(item.get_ptr());
	
	// check pathological case - empty list

	if (litem != NULL)
	    {
	    if (litem->length() == 0)
		item = NULL;
	    }
	if (item != NULL)
	    {
	    switch (state)
		{
		case 0:
		    if (litem != NULL)
			{
			result_list = litem;
			if (litem->length() == 1)
			    {
			    result = litem->get_item(0);
			    state = 1;
			    }
			else if (litem->get_is_temp())
			    state = 3;
			else
			    state = 2;
			}
		    else
			{
			result = item;
			state = 1;
			}
		    break;
		case 1:
		    if (litem != NULL)
			{
			if (litem->get_is_temp())
			    {
			    litem->AddObject(result);
			    result_list = litem;
			    state = 3;
			    }
			else
			    {
			    result_list = new MacroListObject();
                            result_list->set_is_temp();
                            result_list->AddObject(result);
                            result_list->AddObjectList(*litem);
			    state = 3;
			    }
			}
		    else
			{
                	result_list = new MacroListObject();
                	result_list->set_is_temp();
			result_list->AddObject(result);
			result_list->AddObject(item);
			state = 3;
			}
		    break;
		case 2:
			{
			AbstractMacroListObject *cur_list = result_list;
		        result_list = new MacroListObject();
                        result_list->set_is_temp();
			result_list->AddObjectList(*cur_list);
			if (litem != NULL)
			    result_list->AddObjectList(*litem);
			else
                            result_list->AddObject(item);
			state = 3;
			}
		    break;
		case 3:
                    if (litem != NULL)
                        result_list->AddObjectList(*litem);
                    else
                        result_list->AddObject(item);
		    break;
	        }
	    }
	iter->Next();
	}
    if (result_list != NULL)
	{
	return result_list;
	}
    else
	{
	return result;
	}
    }

void MacroListObject::AddObjectList(const AbstractMacroListObject &x)
    {
    AbstractMacroListObject * cur = const_cast<AbstractMacroListObject *>(&x);
    MacroIterPtr iter = cur->get_iter();
    while (!iter->isDone())
  	{
        AddObject(iter->Item());
	iter->Next();
	}
    }
 
MacroObjectPtr NamedList::get_child(const LString &name) const
    {
    int len = names.len();
    for (int i = 0;i < len;i++)
        {
        if (name == names[i])
            return get_item(i);
        }
    return NULL;
    }
 
String NamedList::child_name_list() const
    {
    String list;
    for (int i = 0;i < names.len();i++)
        {
        if (i > 0)
            list += ",";
        list += names[i].c_str();
        }
    return list;
    }
 
MacroObjectPtr AbstractNamedList::get_as_list()
    {
    MacroListObject *result_list = new MacroListObject();
    MacroObjectPtr result = result_list;
    result_list->set_is_temp();
    int len = length();
    for (int i = 0 ; i < len ; i ++)
        result_list->AddObject(get_item(i));
    return result;
    }

int MacroExpansion::find_name(const LString &name)
    {
    int i = macro_defs.len();
    while (i > 0)
        {
        i--;
        if (macro_defs[i]->name == name)
            return i;
        }
    return -1;
    }

void MacroExpansion::add_macro(MacroDef *def)
    {
    int index = find_name(def->name);
    if (index < 0)
	{
	macro_defs.push(def);
	return;
	}

    // this may replace an existing entry, or it may be an overload
    // to replace an entry it must be an exact match

    MacroDef *odef = macro_defs[index];
    MacroDef *last = NULL; 

    while (odef != NULL)
	{
	if (def->par_names.len() == odef->par_names.len())
	    {
	    int i = 0;
	    while (i < def->par_names.len())
		{
		if (def->par_names[i].type != odef->par_names[i].type)
		    break;
		i++;
		}
	    if (i >= def->par_names.len()) // match found
		{
		def->next_with_name = odef->next_with_name;
		if (last == NULL)
		    macro_defs[index] = def;
		else
		    last->next_with_name = def;
		delete odef;
		return;
		}
	    }
	last = odef;
	odef = odef->next_with_name;
	}
    
    // if we get here, it is not a replacement, so add it to start of list

    def->next_with_name = macro_defs[index];
    macro_defs[index] = def;
    }

int MacroExpansion::derivation_distance(const LString &required,const MacroObjectPtr found) {
    return -1;
    }


MacroDef *MacroExpansion::resolve_macro(const LString &name,
			simple_stack<foreach_stack_entry *>&pars,int par_offset,int par_len)
    {
    int index = find_name(name);
    if (index < 0)
        {
        return NULL;
        }
    MacroDef *def = macro_defs[index];
    MacroDef *best = NULL;  // best match found so far
    int best_score_derivations = 0;
    static int deepest_derivation = 100;

    while (def != NULL)
        {

	// to be a match the macro must have at least as many parameters as
	// are being passed (it may have less)
	//
	// A default parameter (no type spec) must always be treated as far away
        if (def->par_names.len() >= par_len)
            {
	    int derivations = 0;
	    int default_pars = 0;
            int i = 0;
            while (i < par_len)
                {
		  static LString zero_str("");
		if (def->par_names[i].type != zero_str)
		    {
	            LString par_type;
		    MacroObjectPtr object = pars[par_offset + i]->get_object();
		    if (!object.is_null())
			par_type = object->object_type_name();
                    if (def->par_names[i].type != par_type) {
                        int distance = derivation_distance(def->par_names[i].type,object);
			if (distance < 0)
			    break;
			derivations += distance;
			// if this is deeper than we expected, up the anti
			// on deepest_derivation and rerun the resolution
			if (distance > deepest_derivation) {
			    deepest_derivation = distance + 100;
			    return resolve_macro(name,pars,par_offset,par_len);
			    }
			}
		    }
	 	else {
		    default_pars ++;
		    }
                i++;
                }

	    if (i < par_len)
		{
		def = def->next_with_name;
		continue;
		}
	    
	    while (i < def->par_names.len())
		{
		  static LString zero_str("");
		if (def->par_names[i].type != zero_str)
		    break;
		default_pars ++;
		i++;
		}

	    derivations += default_pars * deepest_derivation;
	    if (i >= def->par_names.len()) {
		if ((best == NULL) || (best_score_derivations > derivations)){
		    best = def;
		    best_score_derivations = derivations;
		    }
		}
            }
        def = def->next_with_name;
        }
    return best;
    }

void MacroListObject::perform_final_cleanup() {
    if (_performing_cleanup)
	return;
    _performing_cleanup = true;
    while (list.len() > 0) {
	list.top()->perform_final_cleanup();
	list.pop();
	}
    _performing_cleanup = false;
    }

void MacroListObject::AddObject(MacroObjectPtr object) {
    list.push(object);
    }

