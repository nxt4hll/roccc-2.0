#include "parser.h"
#include "common/text_util.h"
#include "common/MString.h"

/*
static const char *trim(const char * text)
    {
    static char buff[22];
    for (int i = 0;i < 20;i++)
	buff[i] = text[i];
    buff[20] = 0;
    return buff;
    }
*/

const char *last_failure;
String last_failure_message;

MacroObjectPtr ParseProduction(const char *&text,Grammar &g,Production *p,bool top_level);


static bool lexical_match(const char *text,const String &name)
    {
    bool keyword = true;
    for (int i =0;i < name.length();i++)
	{
	char ch = name[i];
	if (!isalpha(ch))
	    keyword = false;
	if (text[i] != ch)
	    return false;
	}
    if (keyword)
	{
	char ch = text[name.length()];
	if (isalpha(ch) || (ch == '_'))
	    return false;
	}
    return true;
    }

static int get_verbatim(const char **text, const String &term, 
			String *value)

{
    const char *res = strstr(*text, term.c_str());
    int  idx = (res) ? ((res - *text)) : -1;

    if (idx != -1) {
		*value = String(*text, idx);
		*text += idx;
		}

    return (idx);
}


MacroObjectPtr ListParser(const char *&text,Grammar &g,NonTerminal *start_symbol,bool top_level)
    {

    // first see if this is a list producing production. It is if it takes the form 
    // <something> <string> <start_symbol> | <something> or
    // <start_symbol> <string> <something> | <something>

    // printf(" checking for list ");
    // start_symbol->Print();
    // printf("\n");

    int productions = start_symbol->production_count();

    if (productions != 2)
	return NULL;

    Production *p2 = start_symbol->get_production(0);
    Production *p1 = start_symbol->get_production(1);

    // the second will be shorter and must be a prefix or suffix of the second

    int p1_len = p1->get_rhs_len();
    int p2_len = p2->get_rhs_len();
    if (p2_len < 2)
	return NULL;
    if (p1_len < 1)
	return NULL;

    if (p1_len > p2_len - 1)
	return NULL;

    //int offset;

    int start_terminal_fill;
    int end_terminal_fill;
    if (p2->get_rhs(0) == start_symbol) 
	{

	// The forn is left recursive, so match from the end of p1

	for (int i = 0;i < p1_len;i++)
            {
            if (p1->get_rhs(i) != p2->get_rhs(p2_len - p1_len + i))
                return NULL;
	    }

	start_terminal_fill = 1;
	end_terminal_fill = p2_len - p1_len;
        }
    else if (p2->get_rhs(p2_len - 1) == start_symbol)
	{

	// The form is right recursive, so match start against p1,
	// followed by terminals

	for (int i = 0;i < p1_len;i++)
            {
            if (p1->get_rhs(i) != p2->get_rhs(i))
                return NULL;
            }
	start_terminal_fill = p1_len;
	end_terminal_fill = p2_len - 1;
	}

    else
	return NULL;

    // Check the fill really does only contain terminals
    // really should be non=parameterized terminals 
    for (int i = start_terminal_fill;i < end_terminal_fill;i++)
        {
        Terminal *s = get_if_terminal(p2->get_rhs(i));
        if (s == NULL)
            return NULL;
        }


    // ok, its a list production. Find as many as possible in a row, making sure that we stop if empty is
    // matched

    MacroListObject *list = new MacroListObject(true);
    list->set_type_name(p1->get_lhs()->get_name());
    bool found = false;
    while (1)
	{
	text = skip_space(text);
	const char *start_pos = text;
	MacroObjectPtr m = ParseProduction(text,g,p1,false);
	
	if (m != NULL)
	    {
	    if (start_pos == text) // special case - empty
		{
		if (found)
		    {
		    m.make_null();
		    return list;
		    }
		else
		    {
		    delete list;
		    return m;
		    }
	    	}
	    found = true;
	    list->AddObject(m);
	    }
  	else
	    {
	    text = start_pos;
	    if (found)
		return list;
	    delete list;
	    return NULL;
	    }
	const char *fill_start = text;
 	for (int i = start_terminal_fill;i < end_terminal_fill; i++)
	    {
	    Terminal *t = get_if_terminal(p2->get_rhs(i));
            const String &name = t->get_name();
	    text = skip_space(text);
            if (lexical_match(text,name))
                {
                text += name.length();
                }
	    else
		{
		text = fill_start;
		if (found)
		    return list;
		delete list;
		return NULL;
	 	}
	    }
	}
    }

static MacroObjectPtr Parser(const char *&text,Grammar &g,NonTerminal *start_symbol,
		    bool top_level,int &production)
    {
      //bool nullable = false;

    const char * start_pos = text;

    int i = production;
    int productions = start_symbol->production_count();

    // is this a list generating production? These are handled as a special case

    // printf("attempting to parse ");
    // start_symbol->Print();
    // printf("\n");

    MacroObjectPtr  list_result = NULL;
    if (i == 0)
	list_result = ListParser(text,g,start_symbol,top_level);
    if (list_result != NULL)
	return list_result;
    while (i < productions)
	{
	if (*text == 0)
	    {
	    text = start_pos;
	    return NULL;
	    }
	Production *p = start_symbol->get_production(i);
	MacroObjectPtr  return_object = ParseProduction(text,g,p,top_level);
	if (return_object != NULL)
	    {
	    production = i;
	    return return_object;
	    }
	text = start_pos;
	i++;
	}
    text = start_pos;
    return NULL;
    }

class int_buffer
    {
    public:
        simple_stack<int *> buffer;
    };

class int_buff_manager
    {
        simple_stack<int_buffer *>by_len;
    public:
	int * get_buffer(int len)
	    {
   	
	    while (len >= by_len.len())
	        by_len.push(NULL);
	    if (by_len[len] == NULL)
	        by_len[len] = new int_buffer;
	    if (by_len[len]->buffer.len() == 0)
	        return new int[len];
	    return by_len[len]->buffer.pop();
	    }
	void free_buffer(int *buff,int len)
	    {
            while (len >= by_len.len())
                by_len.push(NULL);
            if (by_len[len] == NULL)
                by_len[len] = new int_buffer;
	    by_len[len]->buffer.push(buff);
	    }
	};

int_buff_manager buff_manager;



MacroObjectPtr ParseProduction(const char *&text,Grammar &g,Production *p,bool top_level)
    {
    // printf("attempting to parse ");
    // p->Print();
    // printf("\n");
    NamedList * return_object = new NamedList(true);
    return_object->set_type_name(p->get_lhs()->get_name());
    if (p->get_rhs_len() == 0)
	{
	MacroObjectPtr smo = new StringMacroObject("true");
	smo->set_type_name("empty");
	return_object->AddObject("empty",smo);
        return return_object;
	}

    int j;
    int len = p->get_rhs_len();
    const char *start_pos = text;
    int *match_pos = buff_manager.get_buffer(len);

    j = 0;
    while (j < len)
        {
	while (j < len) // succeed loop
	    {
	    GrammarSymbol *s = p->get_rhs(j);
	    text = skip_space(text);
	    if (s->is_terminal())
	    	{
	    	const String &name = s->get_name();
	    	if (name == String("identifier"))
	    	    {
	    	    String value;
	    	    int idlen = get_name(text,value);
	    	    if (idlen == 0)
	    		{
	    		if (last_failure < text)
	                    {
	    		    last_failure_message = "missing identifier";
	    		    }
			delete return_object;
	    		return NULL;
	    		}
	    	    text += idlen;
		    MacroObjectPtr smo = new StringMacroObject(value);
		    smo->set_type_name("identifier");
	    	    return_object->AddObject("identifier",smo);
	    	    }
	        else if (name == String("verbatim")) 
	            {
	            String value;
		    GrammarSymbol *next= p->get_rhs(j+1);
		
		    if (!next) 
		        {
		        last_failure_message = "verbatim needs to be followed by a non-terminal";
		        delete return_object;
		        return NULL;
		        }

		    if (!next->is_terminal()) 
		        {
		        last_failure_message = "verbatim followed by non-terminal";
		        delete return_object;
		        return NULL;
		        }
		
	            // figure out the right way to do errors here
		    int idlen = get_verbatim(&text, next->get_name(), &value);

		    if (idlen < 0) 
		        {
		        if (last_failure < text)
		            {
			    last_failure_message = "couldn't find terminator on verbatim";
		            }

		        delete return_object;
		        return NULL;
		        }

		    MacroObjectPtr smo = new StringMacroObject(value);
	  	    smo->set_type_name("verbatim");
	            return_object->AddObject("verbatim", smo);
	            }
	        else if (lexical_match(text,name))
	    	    {
	    	    text += name.length();
	    	    }
	    	else
	    	    {
		    // printf(" no prefix match of %s and %s\n",(const char *)name,trim(text));
	    	    if (last_failure < text)
	    		{
	    		String reason;
	    		last_failure = text;
			reason += "wanted token '";
			reason += name;
			reason += "'";
			//sprintf(reason,"wanted token %s",(const char *)name);
	    		last_failure_message = reason;
	    		}
	    	    else if (last_failure == text)
	    		{
			String reason;
	                last_failure = text;
			reason += "token '";
			reason += name;
			reason += "'";
			//sprintf(reason,"token %s",(const char *)name);
	    		if (last_failure_message.find(reason) < 0)
	    		    {
	                    last_failure_message += " or ";
	    		    last_failure_message += reason;
	    		    }
	    		}
		    break;
	    	    }
	        }
	    else
	    	{
	    	NonTerminal *n = get_if_nonterminal(s);
		match_pos[j] = 0;
	    	MacroObjectPtr m = Parser(text,g,n,false,match_pos[j]);
	    	if (m.is_null())
	    	    {
	    	    if (last_failure < text)
	                {
			String reason;
	                last_failure = text;
			reason += "could not parse a ";
			reason += n->get_name();
	    		//sprintf(reason,"could not parse a %s",(const char *)n->get_name());
	    		last_failure_message = reason;
	    	 	}
		    break;
	    	    }
		m->set_type_name(n->get_name());
	    	return_object->AddObject(n->get_name(),m);
	        }
	    j++;
	    }
	if (j >= len)	// matched entire production
	    break;

	// failed to match enture production, so back up
	while (j > 0) // fail loop
	    {
	    j--; // try a preceding production 
            GrammarSymbol *s = p->get_rhs(j);
            text = skip_space(text);
            if (s->is_terminal())  // no alternatives are available, so just back up over
	        continue;

	    NonTerminal *n = get_if_nonterminal(s);
	    match_pos[j] ++;
	    MacroObjectPtr m = Parser(text,g,n,false,match_pos[j]);
	    if (m != NULL)
		{
		return_object->CutBack(j);
		m->set_type_name(n->get_name());
		return_object->AddObject(n->get_name(),m);
		j ++;
		break;
		}
	    }

	if (j == 0) // failed to find a restart point
	    {
	    delete return_object;
	    return NULL;
	    }
 	}
    MacroObjectPtr smo = new StringMacroObject(String(start_pos,text-start_pos));
    smo->set_type_name("text");
    return_object->AddObject("text",smo);
    if (j >= len)
        {
        if (top_level)
    	    {
    	    text = skip_space(text);
    	    if (*text == 0)
		return return_object;
    	    if (last_failure < text)
    	        {
                last_failure = text;
    	        last_failure_message = "extra text at end of input";
    	        }
      	    }
        else
    	    {
    	    return return_object;
	    }
    	}
    delete return_object;
    return NULL;
    }

MacroObjectPtr Parse(const char *filename,
		   const char *text,Grammar &g)
    {
    const char *start = text;
    last_failure_message = "could not parse input";
    last_failure = text;
    int prod_no = 0;
    MacroObjectPtr m = Parser(text,g,g.get_distinguished_symbol(),true,prod_no);
    if (m.is_null())
	send_error(filename, start,last_failure,(const char *)last_failure_message);
    return m;
    }
