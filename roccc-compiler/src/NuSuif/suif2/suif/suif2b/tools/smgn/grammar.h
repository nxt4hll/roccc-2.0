#ifndef __GRAMMAR__
#define __GRAMMAR__

#include "common/simple_stack.h"
#include "common/MString.h"

#ifndef NULL
#define NULL 0
#endif

class Production;

class GrammarSymbol
    {
	String name;
    public:
	GrammarSymbol(String the_name) : name(the_name) {}
	virtual ~GrammarSymbol() {}
	String get_name() {return name;}
	virtual bool is_terminal() = 0;
	virtual void Print() = 0;
	};

class Terminal : public GrammarSymbol
    {
    public:
	Terminal(String the_name) : GrammarSymbol(the_name) {}
	bool is_terminal() {return true;}
	void Print();
	};

class SymbolList: public simple_stack<GrammarSymbol *>
    {
    public:
	bool exists(GrammarSymbol *x);
        GrammarSymbol * get_symbol(const String name,const bool terminal);
    };

typedef simple_stack<Production *> ProductionList;

class NonTerminal : public GrammarSymbol
    {
    	ProductionList productions;
	bool repeating;	// can repeat. Only for generated non terminals
    public:
	NonTerminal(String the_name) : GrammarSymbol(the_name),productions(3,5),repeating(false) {}
            
        void AddProduction(Production *prod);

	bool is_terminal() { return false; }

	void set_repeating() {repeating = true;}

	bool is_repeating() {return repeating;}

	int production_count()
	    {
	    return productions.len();
	    }

	Production *get_production(int n)
	    {
	    return productions[n];
	    }
	void Print();

	void sort();
	};


class Production 
    {
        SymbolList rhs;
        NonTerminal *lhs_sym;
	int prod_no;
    public:
        Production(NonTerminal *lhs) : lhs_sym(lhs), prod_no(0)
            {
            lhs->AddProduction(this);
            }

        NonTerminal * get_lhs() { return lhs_sym; }

        void AddRhs(GrammarSymbol *symbol)
            {
            rhs.push(symbol);
            }
	int get_rhs_len()
	    {
	    return rhs.len();
	    }

	GrammarSymbol *get_rhs(int n)
	    {
	    return rhs[n];
	    } 

	void Print();

	int get_prod_no() {return prod_no;}

	void set_prod_no(int i) {prod_no = i;}
    };


class Grammar 
    {
        SymbolList symbols;
        NonTerminal *distinguished;
        int SymbolCount;
        int SyntheticCount;
        bool get_rhs(const char * &text,Production *prod);

	bool read_rhsides(const char * &text,NonTerminal *lhs);

        GrammarSymbol ** SaveSymPtr;
        int terminals;
        int nonterminals;

	const char *text_of_definition;
	const char *file_of_definition;

	void send_error(const char *text,const char *message);

	GrammarSymbol *read_nonterminal(const char *&text,bool must_be_new);
    public:
    
        Grammar() : 
	  symbols(),distinguished(NULL),SymbolCount(0),
	  SyntheticCount(0),SaveSymPtr(NULL),terminals(0),
	  nonterminals(0), text_of_definition(NULL),
	  file_of_definition(NULL)
        {}
        bool read_grammar(char * filename);        
        
        // Find the distinguished symbol
        NonTerminal *get_distinguished_symbol() { return distinguished;}
        
        void Print();

        // Perform various checks on the grammar
        bool Check();

	void PrintProductions(GrammarSymbol *sym); 

	GrammarSymbol * get_symbol(const String name,const bool terminal);
	};   

void print_symbol_list(SymbolList &Symbols);     

NonTerminal * get_if_nonterminal(GrammarSymbol *s);

Terminal * get_if_terminal(GrammarSymbol *s);

#endif
    
