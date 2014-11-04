// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include "iokernel/cast.h"
#include "suifkernel/group_walker.h"
#include "suifkernel/suif_object.h"
#include "utils/symbol_utils.h"
#include "utils/type_utils.h"
#include "utils/print_utils.h"
#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "suifkernel/utilities.h"

#include "symbol_walkers.h"

#include <stdio.h>
#include <string.h>

//extern int printobj(SuifObject *obj);

class SymbolNamer : public SelectiveWalker {
	LString source_file_name;		
    public:
	SymbolNamer(SuifEnv *env, LString  name) 
		: SelectiveWalker(env,Symbol::get_class_name()) ,
					source_file_name(name) {}

	ApplyStatus operator () (SuifObject *x);

	};

static String get_orig_sym_name(Symbol *sym) 
{
  static LString k_orig_name = "orig_name";
  Annote *an = sym->peek_annote(k_orig_name);
  if (!is_kind_of<BrickAnnote>(an)) return(emptyString);
  BrickAnnote *name_an = to<BrickAnnote>(an);
  if (name_an->get_brick_count() != 1) return(emptyString);
  SuifBrick *br = name_an->get_brick(0);
  if (!is_kind_of<StringBrick>(br)) return(emptyString);
  String base_name = to<StringBrick>(br)->get_value();
  return(base_name);

}

static char legal_var_name_chars[] =
  "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789";

static String condition_file_name(LString * file_name)
    {
    int len_name = (int) strcspn(file_name->c_str(), ".");
    if ((len_name > 10) || ( len_name <= 0))
        len_name = 10;
    char *conditioned_name = new char[len_name + 1];
    strncpy(conditioned_name, file_name->c_str(), len_name);
    conditioned_name[len_name] = (char)NULL;

    int illegal_char_off;
    while ((illegal_char_off = strspn(conditioned_name,legal_var_name_chars)) < len_name)
        {
        conditioned_name[illegal_char_off] = '_';
        }
    String cn(conditioned_name);
    delete [] conditioned_name;
    return(cn);
    }

static void name_variable(Symbol *symbol, LString * file_name) {
    static LString k_orig_name = "orig_name";

    if (symbol->get_name() != emptyLString)
      return;

    String base_name;
    String orig_name = get_orig_sym_name(symbol);
    if (orig_name != emptyString) {
      base_name = orig_name;
    } else {
      base_name = "_";

      String source_file_name = condition_file_name(file_name);
      // add up to 10 characters at the end of the source file name
      // name Are you KIDDING!
      base_name += source_file_name.Right(10);
      base_name += "Tmp";
    } 

    // guaranteed to complete because of the IInteger.
    // Of course it would be faster if we randomized.
    for (IInteger ii = 0; ; ii++) {
      String next_name = base_name + ii.to_String();
      if (!LString::exists(next_name)) {
	SymbolTable *st = symbol->get_symbol_table();
	suif_assert_message(st != NULL,
			    ("attempt to name unattached symbol"));
	st->change_name(symbol,next_name);
	return;
      }
    }
    suif_assert_message(0, ("Could not form a unique name"));
}
 


Walker::ApplyStatus SymbolNamer::operator () (SuifObject *x) {
    Symbol *symbol = to<Symbol>(x);
    name_variable(symbol, & source_file_name);
    return Walker::Continue;
    }

//	Table of keywords sorted by length then alphabetically
static const int len_start[]={0,0,0,2,4,12,18,26,28,32};
static const char *keywords[] = {
	"do",	//	2->0
	"if",
	"for",	//	3->2
	"int",
	"auto", //	4->4
	"case",
	"char",
	"else",
	"enum",
	"goto",
	"long",
	"void",
	"break", //	5->12
	"const",
	"float",
	"short",
	"union",
	"while",
	"double",//	6->18
	"extern",
	"return",
	"signed",
	"sizeof",
	"static",
	"struct",
	"switch",
	"default",//	7->26
	"typedef",
	"continue",//	8->28
	"register",
	"unsigned",
	"volatile"};
static bool is_keyword(const LString &name) {
    const char *str = name.c_str();
    int i = 0;
    while ((i <= 8) && str[i])
	i++;
    if (i > 8)
	return false;
    int start = len_start[i];
    int end = len_start[i+1];
    while ((start < end) && (str[0] > keywords[start][0]))
	start ++;
    if ((start >= end) || (str[0] != keywords[start][0]))
	return false;
    while ((start < end) && (str[0] == keywords[start][0])){
	const char *str1 = str + 1;
	const char *key1 = keywords[start] + 1;
	while ((*key1 == *str1) && *str1) {
	    key1++;
	    str1++;
	    }
	if (*str1 == *key1)
	    return true;
	start ++;
	}
    return false;
    }



/* return true if the symbol has a name crash with another symbol in
 * its parent (a symbol table).
 * This implementation just consider name crashes and ignore the symbol
 * type.
 */
static bool is_var_name_crashd_locally(const Symbol* symbol)
{
  LString sname = symbol->get_name();
  if (sname == emptyLString) return false;
  SymbolTable* symtab = to<SymbolTable>(symbol->get_parent());
  if (!is_kind_of<VariableSymbol>(symbol)) return false;
  for (Iter<SymbolTableObject*> iter = 
	 symtab->get_symbol_table_object_iterator();
       iter.is_valid();
       iter.next()) {
    if (symbol == const_cast<const SymbolTableObject*>(iter.current()))
      continue;
    if (!is_kind_of<VariableSymbol>(iter.current())) continue;
    if (iter.current()->get_name() == sname) {
      return true;
    }
  }
  return false;
}


Walker::ApplyStatus CollisionAvoider::operator () (SuifObject *x) {
    Symbol *symbol = to<Symbol>(x);

    // delete names which mask externals. If we end up with a name that does
    // not, just return, otherwise create a new name

    // printf("next symbol is ");printobj(symbol);
    LString sname = symbol->get_name();
    //    bool is_key = is_keyword(sname);

    if (sname == emptyLString) {
        if (name_all_symbols)
            name_variable(symbol,& source_file_name);
        return Walker::Continue;
        }

    SymbolTable *scope = symbol->get_symbol_table();
    if (!is_keyword(sname)) {
      bool conflicts = is_var_name_crashd_locally(symbol);
      if (scope != external_symbol_table) {
	// if it doesn't conflict in the external symbol table
	if (external_symbol_table->has_lookup_table_member(sname)) {
	  conflicts = true;
	} else {
	  if (file_scope_symbol_tables) {
	    // or the file scope tables
	    for (list<SymbolTable*>::iterator iter = 
		   file_scope_symbol_tables->begin();
		 iter != file_scope_symbol_tables->end();  iter++)
	        {
		SymbolTable *symtab = *iter;
		if (symtab == scope) continue;
		if (symtab->has_lookup_table_member(sname))
		    {
		    conflicts = true;
		    break;
		    }
	        }
	  }
   
	  if (!conflicts)
	      {
              SymbolTable *curr_scope =
                             symbol->get_symbol_table()->get_explicit_super_scope();
              while (curr_scope &&
                     curr_scope->get_explicit_super_scope() != external_symbol_table)
                  {
                  if (curr_scope->has_lookup_table_member(sname ))
                      {
                      conflicts = true;
                      break;
                      }
                  curr_scope = curr_scope->get_explicit_super_scope();
                  }
              }
         }
      }

      if (!conflicts)
	return Walker::Continue;
    }

    // TBD:
    // Actually, we may have to check 
    // for ANY conflict.  I.e. for machsuif,
    // labels conflict with procedures

    // Added July 29 for Fortran
    // Make sure names are not common "reserved words" from C
    // This is lsightly bogus - should be configurable somehow
    // We only need this because we are generating C out. If we
    // are going to a code gen, or outputting some other language
    // this is not needed or needs changing

    //	int no = symbol->num_name_with_key(sname);
    //	while (no > 0) {
    // printf("conflict found\n");
    //    	    symbol->remove_name(sname,--no);
    //	    }
    //}
    rename_symbol(symbol, emptyLString);
    //    if (name_all_symbols)
    name_variable(symbol,& source_file_name);
    return Walker::Continue;
    }


void NameAllSymbolsPass::do_file_set_block( FileSetBlock* file_set_block ) {
    SymbolNamer walker(get_suif_env(),(file_set_block->get_file_block(0))->
					get_source_file_name());
    file_set_block->walk(walker);
    }

void AvoidExternCollisions::do_file_set_block( FileSetBlock* file_set_block ) {
    CollisionAvoider walker(get_suif_env(),
			    file_set_block->get_external_symbol_table(),
			    NULL,
			    (file_set_block->get_file_block(0))->
							get_source_file_name(),
			    false);
    file_set_block->walk(walker);
    }

void AvoidFileScopeCollisions::do_file_set_block( FileSetBlock* file_set_block ) {
    list<SymbolTable*> file_scope_tables;
    for (Iter<FileBlock*> iter = file_set_block->get_file_block_iterator();
	 iter.is_valid(); iter.next()) 
      {
	file_scope_tables.push_back(iter.current()->get_symbol_table());
      }
    file_scope_tables.push_back(file_set_block->get_file_set_symbol_table());
    
    CollisionAvoider walker(get_suif_env(),
			    file_set_block->get_external_symbol_table(),
			    &file_scope_tables,
			    (file_set_block->get_file_block(0))->
				get_source_file_name(),
			    false);
    file_set_block->walk(walker);
    }


void AvoidLabelCollisions::do_procedure_definition( ProcedureDefinition *proc_def ) {
  // get a list of all of the Labels in the procedure.
  list<CodeLabelSymbol*>* l = collect_objects<CodeLabelSymbol>(proc_def);

  suif_hash_map<LString, int> label_defined;
  // It's an n^2 algorithm.  If this ever becomes
  // a problem, we can use a hash map.
  while (!l->empty()) {
    CodeLabelSymbol *lab = l->front();
    l->pop_front();
    LString name = lab->get_name();
    if (name == emptyLString) continue;
    if (label_defined.find(name) != label_defined.end()) {
      rename_symbol(lab, emptyLString);
      continue;
    }
    label_defined.enter_value(name, 1);
  }
  delete l;
}

void CombinedPass::do_file_set_block( FileSetBlock* file_set_block ) {
    list<SymbolTable*> file_scope_tables;

    for (Iter<FileBlock*> iter = file_set_block->get_file_block_iterator();
	 iter.is_valid(); iter.next()) 
      {
	file_scope_tables.push_back(iter.current()->get_symbol_table());
      }
    file_scope_tables.push_back(file_set_block->get_file_set_symbol_table());
    
    CollisionAvoider walker(get_suif_env(),
			    file_set_block->get_external_symbol_table(),
			    &file_scope_tables,
                            (file_set_block->get_file_block(0))->
                                get_source_file_name(),
			    true);
    file_set_block->walk(walker);
    }

NameAllSymbolsPass::NameAllSymbolsPass(SuifEnv *env, const LString &name) 
	: Pass(env, name) {}

AvoidExternCollisions::AvoidExternCollisions(SuifEnv *env, const LString &name) 
        : Pass(env, name) {}

AvoidFileScopeCollisions::AvoidFileScopeCollisions(SuifEnv *env,
						   const LString &name) 
        : Pass(env, name) {}

AvoidLabelCollisions::AvoidLabelCollisions(SuifEnv *env, const LString &name) 
	: PipelinablePass(env, name) {}

CombinedPass::CombinedPass(SuifEnv *env, const LString &name) 
        : Pass(env, name) {}

