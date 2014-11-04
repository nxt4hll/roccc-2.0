
#include "grammar.h"
#include "common/text_util.h"
#include <stdio.h>

NonTerminal * get_if_nonterminal(GrammarSymbol *s)
    {
    if (s->is_terminal())
	return NULL;
    return (NonTerminal *)s;
    }

Terminal * get_if_terminal(GrammarSymbol *s)
    {
    if (!s->is_terminal())
        return NULL;
    return (Terminal *)s;
    }

void Grammar::send_error(const char *text,const char *message)
    {
    ::send_error(file_of_definition, text_of_definition,text,message);
    }

const char *special_terminals[] = {"identifier","number","verbatim",0};

GrammarSymbol *Grammar::read_nonterminal(const char *&text,bool must_be_new)
    {
    text = skip_space_and_comments(text);
    if (*text != '<')
        {
	send_error(text,"Expected a lhs symbol");
        return NULL;
        }

    text = skip_space_and_comments(text + 1);
    String name;
    int idlen = get_name(text,name);
    text = skip_space_and_comments(text + idlen);

    if (*text != '>')
        {
	send_error(text,"missing > ");
        return NULL;
	}
    text = skip_space_and_comments(text + 1);
    int i = 0;
    while ((special_terminals[i] != NULL) && (name != String(special_terminals[i])))
	i++;

    if (special_terminals[i] != NULL)
	{
	return get_symbol(name,true);
	}
    
    NonTerminal *s = get_if_nonterminal(get_symbol(name,false));
    if (must_be_new && s->production_count() > 0)
	{
	send_error(text,"LHS is already defined");
	return NULL;
	}
    return s;
    }

bool Grammar::read_rhsides(const char * &text,NonTerminal *lhs)
    {
    while (1)
	{
	// process 1 rhs

	Production *p = new Production(lhs);

	while ((*text != '|') && (*text != 0))
	    {
	    switch (*text)
		{
		case '<':
	 	    {
		    const char * ftext = skip_past(text,'>');
		    ftext = skip_space_and_comments(ftext);
		    if (prefix_match(ftext,"::="))
		    	return true;

		    // its just a lhs

		    GrammarSymbol *nt = read_nonterminal(text,false);
		    if (nt == NULL)
		 	return false;
		    p->AddRhs(nt);
		    break;
	 	    }

		case '"':
		    {
		    String name;
		    int idlen = get_string(text,name);
		    text = skip_space_and_comments(text + idlen);
                    p->AddRhs(get_symbol(name,true));
		    break;
                    }

#ifdef REPEAT_CONSTRUCTS
		case '}':
		    text = skip_space_and_comments(text + 1);
		    return true;

		case '{':
		    {
		    char name[20];
		    sprintf(name,"T_%1d",SyntheticCount);
		    SyntheticCount++;
		    text = skip_space_and_comments(text + 1);
		    NonTerminal *replhs = get_if_nonterminal(get_symbol(name,false));
		    if (replhs->production_count() > 0)
		        {
			send_error(text,"synthetic non-terminal already exists");
		        return false;
		        }
		    if (!read_rhsides(text,replhs))
		        return false;
		    p->AddRhs(replhs);
		    replhs->set_repeating();
		    }
#endif
		default:
		    if (isalpha(*text))
			{
			String name;
			int idlen = get_name(text,name);
    			text = skip_space_and_comments(text + idlen);
			p->AddRhs(get_symbol(name,true));
			}
		    else
		 	{
			String name;
                        int idlen = get_mark(text,name);
                        text = skip_space_and_comments(text + idlen);
                        p->AddRhs(get_symbol(name,true));
			}
		}
	    }

	if (*text != '|')
	    break;
	text = skip_space_and_comments(text + 1);
	}
    return true;
    }


bool Grammar::read_grammar(char *filename)
    {
    const char *text = get_file_text(filename);
    text_of_definition = text;
    distinguished = NULL;
    if (text == NULL)
	return false;
    //    Production *current = NULL;

    text = skip_space_and_comments(text,true);

    while (*text != 0)
        {
	text = skip_space_and_comments(text);

	// we should be about to read a lhs. Current symbol should be <

	NonTerminal *lhs = get_if_nonterminal(read_nonterminal(text,true));

	if (lhs == NULL)
	    {
	    send_error(text,"no left hand side found");
	    return false;
	    }

	if (distinguished == NULL)
	    distinguished = lhs;

	if (!prefix_match(text,"::="))
	    {
	    send_error(text,"missing separator ::=");
	    return false;
	    }
   	
 	text = skip_space_and_comments(text + 3);

	// now read in zero or more right hand sides. A RHS can contain optional repeating patterns
	// 

	if (!read_rhsides(text,lhs))
	    return false;
	}

    // sort the productions into descending length

    for (int i = 0;i < symbols.len();i++)
	{
	NonTerminal *s = get_if_nonterminal(symbols[i]);
	if (s != NULL)
	    s->sort();
	}
    return true;
    }


bool Grammar::Check()
    {
 
    // Check all non-terminals have productions and
    // All terminals have none. Turn non-terminals
    // with no productions into terminals

    for (int i = 0;i < symbols.len(); i++)
        {
        GrammarSymbol *sym = symbols[i];
        if (!sym->is_terminal())
	    {
	    NonTerminal *nsym = (NonTerminal *)sym;
	    if (nsym->production_count() == 0)
		{
		fprintf(stderr,"Nonterminal %s has no productions\n",
			(const char *)sym->get_name());
		}
	    }
        }
    return true;
    }

bool SymbolList::exists(GrammarSymbol *x)
    {
    for (int i = 0;i < len(); i++)
    	{
	if ((*this)[i] == x)
    	    return true;
    	}
    return false;
    }

GrammarSymbol * SymbolList::get_symbol(const String name,const bool terminal)
    {
    String key(name);

    for (int i = 0; i < len(); i++)
	{
	GrammarSymbol *t = (*this)[i];
        if ((t->get_name() == key) && (t->is_terminal() == terminal))
            return t;
        }
    return NULL;
    }

GrammarSymbol * Grammar::get_symbol(const String name,const bool terminal)
    {
    GrammarSymbol *s = symbols.get_symbol(name,terminal);
    if (s != NULL)
	return s;
    if (terminal)
	{
	s = new Terminal(name);
	}
    else 
	{
	s = new NonTerminal(name);
	}
    symbols.push(s);
    return s;
    }

void Grammar::Print()
    {
    for (int i = 0;i < symbols.len();i++)
	{
	NonTerminal *s = get_if_nonterminal(symbols[i]);
	if (s != NULL)
	    {
	    if (s->is_repeating())
		printf("{");

	    for (int j = 0;j < s->production_count();j++)
		s->get_production(j)->Print();
	    if (s->is_repeating())
                printf("}");
	    printf("\n");
	    }
	}
    }

void Production::Print()
    {
    lhs_sym->Print();
    printf("::=");
    for (int i = 0;i < rhs.len();i++)
	{
	rhs[i]->Print();
	printf(" ");
	}
    printf("\n");
    }

void Terminal::Print()
    {
    printf("%s",(const char *)get_name());
    }

void NonTerminal::Print()
    {
    printf("<%s>",(const char *)get_name());
    }

void NonTerminal::sort()
    {
    int len = productions.len() - 1;
    bool sorted = false;
    while (!sorted)
	{
	int i = 0;
	sorted = true;
	while (i < len)
	    {
	    Production *p1 = productions[i];
	    Production *p2 = productions[i+1];
	    if (p1->get_rhs_len() < p2->get_rhs_len())
	 	{
		productions[i] = p2;
		productions[i+1] = p1;
		sorted = false;
		}
	    i++;
	    }
	len --;
	}
    }

void NonTerminal::AddProduction(Production *prod)
    {
    productions.push(prod);
    prod->set_prod_no(productions.len());
    }

