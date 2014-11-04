#include "common/system_specific.h"
#include "symbol_utils.h"
#include "type_utils.h"

#include "iokernel/cast.h"
#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif_factory.h"
#include "suifnodes/suif.h"
#include "iokernel/object_factory.h"
#include "iokernel/meta_class.h"
#include "iokernel/pointer_meta_class.h"
#include "suifkernel/utilities.h"
#include "dcast.h"


static void add_local_var_matches(SymbolTable *scope, String name,
                                  list<VariableSymbol *> *lst);
static void add_local_proc_matches(SymbolTable *scope, String name,
                                   list<ProcedureSymbol *> *lst);
static void add_local_code_label_matches(SymbolTable *scope, String name,
                                         list<CodeLabelSymbol *> *lst);

bool is_external_symbol_table(SymbolTable *the_table) {
  if (the_table == 0) return false;
  return (is_kind_of<FileSetBlock>(the_table->get_parent()) &&
	  to<FileSetBlock>(the_table->get_parent())
	  ->get_external_symbol_table() == the_table);
}


bool is_statically_allocated(Symbol *the_symbol)
  {
    suif_assert(the_symbol != 0);
    if (is_kind_of<ProcedureSymbol>(the_symbol))
        return true;
    if (is_kind_of<CodeLabelSymbol>(the_symbol))
        return true;
    if (!is_kind_of<VariableSymbol>(the_symbol))
        return false;
    VariableSymbol *the_var = to<VariableSymbol>(the_symbol);
    SymbolTable *tab = the_var->get_symbol_table();
    if (is_external_symbol_table(tab))
      return(true);
    VariableDefinition *vardef = 
      the_var->get_definition();
    return(vardef &&
	   vardef->get_is_static());
  }

SymbolTable *find_scoped_object_scope(const ScopedObject *the_obj)
{
  // There are 3 special cases:
  // FileBlock, Scope Statement, and Procedure Def
  // contain pointers to their "scope"
  // For all others, finding the scope is just looking
  // up the scope of the parent scoped object
  // The special cases are taken from the SUIF2 IR nodes
  // that implemented child_scoping_this()
  //
  if (is_kind_of<FileBlock>(the_obj)) {
    return(to<FileBlock>(the_obj)->get_symbol_table());
  }
  if (is_kind_of<ProcedureDefinition>(the_obj)) {
    return(to<ProcedureDefinition>(the_obj)->get_symbol_table());
  }

  if (is_kind_of<ScopeStatement>(the_obj)) {
    return(to<ScopeStatement>(the_obj)->get_symbol_table());
  }
  // The default case comes from code in
  // the SUIF2 szot
  SuifObject *par = the_obj->get_parent();
  if (par == NULL) return NULL;
  if (is_kind_of<ScopedObject>(par)) {
    return(find_scoped_object_scope(to<ScopedObject>(the_obj->get_parent())));
  }
  if (is_kind_of<SymbolTableObject>(par)) {
    return(to<SymbolTableObject>(par)->get_symbol_table());
  }
  if (is_kind_of<SymbolTable>(par)) {
    return(to<SymbolTable>(par));
  }
  return(NULL);
}

/*
 * This SHOULD be a dispatched method on all ScopedObjects
 * @@@
 */
SymbolTable *find_child_scoping_this(const ScopedObject *the_szot) {
  if (is_kind_of<FileBlock>(the_szot)) {
    return(to<FileBlock>(the_szot)->get_symbol_table());
  }
  if (is_kind_of<ProcedureDefinition>(the_szot)) {
    return(to<ProcedureDefinition>(the_szot)->get_symbol_table());
  }
  if (is_kind_of<ScopeStatement>(the_szot)) {
    return(to<ScopeStatement>(the_szot)->get_symbol_table());
  }
  return(NULL);

}

// follow up the parents until we find a symbol table,
// or an object that has a
//    symbol_table associated with it.
// ScopeStatement/FileBlock/ProcedureDefinition
//
SymbolTable *find_implicit_follow_up(const ScopedObject *the_szot) {
  SuifObject *par = the_szot->get_parent();
  if (par == NULL) return NULL;

  if (is_kind_of<SymbolTable>(par)) {
    return to<SymbolTable>(par);
  }

  if (is_kind_of<FileSetBlock>(par)) {
    return(to<FileSetBlock>(par)->get_file_set_symbol_table());
  }

  if (!is_kind_of<ScopedObject>(par)) {
    return NULL;
  }
  ScopedObject *parent_szot = to<ScopedObject>(par);
  SymbolTable *result = find_child_scoping_this(parent_szot);
  if (result != NULL) { return(result); }

  return(find_implicit_follow_up(parent_szot));
  //  if (result != NULL)
  //    the_szot->_has_symbol_table_links = TRUE;
  //  return result;
}

SymbolTable *find_implicit_super_scope(const SymbolTable *table)
{
  SuifObject *par = table->get_parent();
  if (par == NULL) return(NULL);
  if (is_kind_of<ScopedObject>(par)) {
    SymbolTable *par_tab = find_implicit_follow_up(to<ScopedObject>(par));
    return(par_tab);
  }
  if (is_kind_of<SymbolTable>(par)) {
    return(to<SymbolTable>(par));
  }
  if (is_kind_of<SymbolTableObject>(par)) {
    return(to<SymbolTableObject>(par)->get_symbol_table());
  }
  if (is_kind_of<FileSetBlock>(par)) {
    FileSetBlock* fsb = to<FileSetBlock>(par);
    if (table == fsb->get_external_symbol_table())
      return 0;
    else if (table == fsb->get_file_set_symbol_table())
      return fsb->get_external_symbol_table();
    else
      return fsb->get_file_set_symbol_table();
  }
  return(NULL);
}



SymbolTable *find_super_scope(const SymbolTable *scope)
{
  if (scope->get_explicit_super_scope() != NULL) {
    return(scope->get_explicit_super_scope());
  }
  return(find_implicit_super_scope(scope));
}

bool is_in_scope(const VariableSymbol* var, const SuifObject* obj)
{
  SymbolTable* vartab = DCAST(SymbolTable*, var->get_parent());
  for (SymbolTable* objtab = find_scope(obj);
       objtab != 0;
       objtab = find_super_scope(objtab)) {
    if (vartab == objtab) return true;
  }
  return false;
}
  

SymbolTable *find_scope(const SuifObject *the_zot)
  {
    if (the_zot == NULL)
        return NULL;
    if (is_kind_of<ScopedObject>(the_zot))
        return find_scoped_object_scope(to<ScopedObject>(the_zot));
    const SuifObject *follow = the_zot;
    while (follow != NULL)
      {
        if (is_kind_of<ScopedObject>(follow))
            return find_scoped_object_scope(to<ScopedObject>(follow));
        follow = follow->get_parent();
      }
    return NULL;
  }

VariableSymbol *lookup_var(SymbolTable *scope, String name)
  {
    suif_assert(scope != NULL);
    VariableSymbol *local = lookup_var_locally(scope, name);
    if (local != NULL)
        return local;
    //    SymbolTable *enclosing_scope = scope->super_scope();
    SymbolTable *enclosing_scope = find_super_scope(scope);
    if (enclosing_scope != NULL)
        return lookup_var(enclosing_scope, name);
    return NULL;
  }

VariableSymbol *lookup_var_locally(SymbolTable *scope, String name)
  {
    suif_assert(scope != NULL);
    // @@@ BUGBUG
    // currently we can only iterate over all
    LString n = name;
    // @@@ an INT??
    int nstos = scope->num_lookup_table_with_key(n);
    for (int i = 0; i < nstos; i++) {
      SymbolTableObject *sto = scope->lookup_lookup_table(n, i);
      if (is_kind_of<VariableSymbol>(sto)) {
	return(to<VariableSymbol>(sto));
      }
    }
    return NULL;
  }

list<VariableSymbol *> *multi_lookup_var(SymbolTable *scope,
                                                String name)
  {
    suif_assert(scope != NULL);
    list<VariableSymbol *> *result = new list<VariableSymbol *>;
    SymbolTable *follow = scope;
    while (follow != NULL)
      {
        add_local_var_matches(follow, name, result);
	follow = find_scope(follow);
      }
    return result;
  }

list<VariableSymbol *> *multi_lookup_var_locally(SymbolTable *scope,
                                                        String name)
  {
    suif_assert(scope != NULL);
    list<VariableSymbol *> *result = new list<VariableSymbol *>;
    add_local_var_matches(scope, name, result);
    return result;
  }

ProcedureSymbol *lookup_proc(SymbolTable *scope, String name)
  {
    suif_assert(scope != NULL);
    ProcedureSymbol *local = lookup_proc_locally(scope, name);
    if (local != NULL)
        return local;
    SymbolTable *enclosing_scope = find_super_scope(scope);
    if (enclosing_scope != NULL)
        return lookup_proc(enclosing_scope, name);
    return NULL;
  }

ProcedureSymbol *lookup_proc_locally(SymbolTable *scope, String name)
  {
    LString n(name);
    suif_assert(scope != NULL);
    size_t sym_count = scope->num_lookup_table_with_key(n);
    for (size_t sym_num = 0; sym_num < sym_count; ++sym_num)
      {
        SymbolTableObject *this_symbol =
	  scope->lookup_lookup_table(n, sym_num);
        if (is_kind_of<ProcedureSymbol>(this_symbol))
            return to<ProcedureSymbol>(this_symbol);
      }
    return NULL;
  }

list<ProcedureSymbol *> *multi_lookup_proc(SymbolTable *scope,
                                                  String name)
  {
    suif_assert(scope != NULL);
    list<ProcedureSymbol *> *result = new list<ProcedureSymbol *>;
    SymbolTable *follow = scope;
    while (follow != NULL)
      {
        add_local_proc_matches(follow, name, result);
        follow = find_super_scope(follow);
      }
    return result;
  }

list<ProcedureSymbol *> *multi_lookup_proc_locally(SymbolTable *scope,
                                                          String name)
  {
    suif_assert(scope != NULL);
    list<ProcedureSymbol *> *result = new list<ProcedureSymbol *>;
    add_local_proc_matches(scope, name, result);
    return result;
  }
CodeLabelSymbol *lookup_code_label(SymbolTable *scope, String name)
  {
    suif_assert(scope != NULL);
    CodeLabelSymbol *local = lookup_code_label_locally(scope, name);
    if (local != NULL)
        return local;
    SymbolTable *enclosing_scope = find_super_scope(scope);
    if (enclosing_scope != NULL)
        return lookup_code_label(enclosing_scope, name);
    return NULL;
  }

CodeLabelSymbol *lookup_code_label_locally(SymbolTable *scope,
                                                    String name)
  {
    LString n(name);
    suif_assert(scope != NULL);
    size_t sym_count = scope->num_lookup_table_with_key(n);
    for (size_t sym_num = 0; sym_num < sym_count; ++sym_num)
      {
        SymbolTableObject *this_symbol =
	  scope->lookup_lookup_table(name, sym_num);
        if (is_kind_of<CodeLabelSymbol>(this_symbol))
            return to<CodeLabelSymbol>(this_symbol);
      }
    return NULL;
  }

list<CodeLabelSymbol *> *multi_lookup_code_label(SymbolTable *scope,
                                                         String name)
  {
    suif_assert(scope != NULL);
    list<CodeLabelSymbol *> *result = new list<CodeLabelSymbol *>;
    SymbolTable *follow = scope;
    while (follow != NULL)
      {
        add_local_code_label_matches(follow, name, result);
        follow = find_super_scope(follow);
      }
    return result;
  }

list<CodeLabelSymbol *> *multi_lookup_code_label_locally(
        SymbolTable *scope, String name)
  {
    suif_assert(scope != NULL);
    list<CodeLabelSymbol *> *result = new list<CodeLabelSymbol *>;
    add_local_code_label_matches(scope, name, result);
    return result;
  }

static void add_local_var_matches(SymbolTable *scope, String name,
                                  list<VariableSymbol *> *lst)
  {
    assert(scope != NULL);
    size_t sym_count = scope->num_lookup_table_with_key(name);
    for (size_t sym_num = 0; sym_num < sym_count; ++sym_num)
      {
        SymbolTableObject *this_symbol = scope->lookup_lookup_table(name, sym_num);
        if (is_kind_of<VariableSymbol>(this_symbol))
            lst->push_back(to<VariableSymbol>(this_symbol));
      }
  }

static void add_local_proc_matches(SymbolTable *scope, String name,
                                   list<ProcedureSymbol *> *lst)
  {
    assert(scope != NULL);
    size_t sym_count = scope->num_lookup_table_with_key(name);
    for (size_t sym_num = 0; sym_num < sym_count; ++sym_num)
      {
        SymbolTableObject *this_symbol = scope->lookup_lookup_table(name, sym_num);
        if (is_kind_of<ProcedureSymbol>(this_symbol))
            lst->push_back(to<ProcedureSymbol>(this_symbol));
      }
  }

static void add_local_code_label_matches(SymbolTable *scope, String name,
                                         list<CodeLabelSymbol *> *lst)
  {
    assert(scope != NULL);
    size_t sym_count = scope->num_lookup_table_with_key(name);
    for (size_t sym_num = 0; sym_num < sym_count; ++sym_num)
      {
        SymbolTableObject *this_symbol = scope->lookup_lookup_table(name, sym_num);
        if (is_kind_of<CodeLabelSymbol>(this_symbol))
            lst->push_back(to<CodeLabelSymbol>(this_symbol));
      }
  }

VariableSymbol *new_unique_variable(SuifEnv *s,
				    SuifObject *scope_finder, 
				    QualifiedType *the_type) {
  return(new_unique_variable(s, scope_finder, the_type, "suif_tmp"));
}
VariableSymbol *new_unique_variable(SuifEnv *s,
					   SuifObject *scope_finder,
					   QualifiedType *the_type,
					   String base_name) {
  SymbolTable *st = find_scope(scope_finder);
  if (st == 0) return 0;
  for (IInteger ii=0; ;ii++) {
    String istr;
    ii.write(istr, 10);
    LString varname(base_name + istr);
    if (st->num_lookup_table_with_key(varname) == 0) {
      // Found one
      VariableSymbol *var = create_variable_symbol(s, the_type, varname);
      st->add_symbol(var);
      return(var);
    }
  }
  return(0); // it will never get here because we're using iintegers
}

VariableSymbol *new_anonymous_variable(SuifEnv *s,
                                           SuifObject *scope_finder,
                                           QualifiedType *the_type) {
    SymbolTable *st = find_scope(scope_finder);
    suif_assert(st);
    VariableSymbol *var = create_variable_symbol(s, the_type);
    st->append_symbol_table_object(var);
    return var;
    }

VariableSymbol *new_anonymous_variable(SuifEnv *s,
                                           SymbolTable *st,
                                           QualifiedType *the_type) {
    VariableSymbol *var = create_variable_symbol(s, the_type);
    st->append_symbol_table_object(var);
    return var;
    }

CodeLabelSymbol *new_unique_label(SuifEnv *s,
				  SuifObject *scope_finder) {
  return(new_unique_label(s, scope_finder, "suif_tmp"));
}

CodeLabelSymbol *new_unique_label(SuifEnv *s,
					 SuifObject *scope_finder,
					 String base_name) {
  SymbolTable *st = find_scope(scope_finder);
  if (st == 0) return 0;
  for (IInteger ii=0; ;ii++) {
    String istr;
    ii.write(istr, 10);
    LString varname(base_name + istr);
    if (st->num_lookup_table_with_key(varname) == 0) {
      // Found one
      CodeLabelSymbol *label = create_code_label_symbol(s, 
							create_label_type(s),
							varname
							);
      st->add_symbol(label);
      // std::cout << "[JUL] symbol_utils.cpp::new_unique_label : " << label->get_name() << std::endl;
//       FILE *f = fopen("jul.out", "a");
//       char buf[256];
//       strcpy(buf, label->get_name());
//       fprintf(f, "%s\n", buf);
//       //fwrite(buf,sizeof(char),strlen(buf),f);
//       fclose(f);
      return(label);
    }
  }
  return(0); // it will never get here because we're using iintegers
}

CodeLabelSymbol *new_anonymous_label(SymbolTable *scope) {
  SuifEnv *s = scope->get_suif_env();
  CodeLabelSymbol *label = 
    create_code_label_symbol(s, retrieve_label_type(s));
  scope->add_symbol(label);
  return(label);
}

CodeLabelSymbol *new_anonymous_label(SuifEnv *s,
					    SuifObject *scope_finder) {
  SymbolTable *st = find_scope(scope_finder);
  return(new_anonymous_label(s, st));
}


LoadVariableExpression *create_var_use(VariableSymbol *var) {
  return(create_load_variable_expression(var->get_suif_env(),
				 unqualify_data_type(var->get_type()),
					 var));
}

void rename_symbol(SymbolTableObject *sym, const LString &name) {
  suif_assert(sym != NULL);
  static LString k_orig_name = "orig_name";
  SymbolTable *st = sym->get_symbol_table();
  LString orig_name = sym->get_name();

  if (orig_name == name) return;

  // Add an annotation with the original name
  // if it doesn't exist yet.
  if (orig_name != emptyLString) {
    if (sym->peek_annote(k_orig_name) == NULL) {
      SuifEnv *s = sym->get_suif_env();
      BrickAnnote *an = create_brick_annote(s,
					    k_orig_name);
      an->append_brick(create_string_brick(s, orig_name));
      sym->append_annote(an);
    }
  }
  
  sym->set_name(name);
  if (st == NULL) return;

  st->remove_all_from_lookup_table(sym);
  if (name != emptyLString)
    st->add_lookup_table(name, sym);
}



// Return true if sym is a definition
//
bool is_definition_symbol(Symbol* sym)
{
  if (is_kind_of<ProcedureSymbol>(sym) &&
      to<ProcedureSymbol>(sym)->get_definition() != NULL) // proc declaration
    return true;
  if (is_kind_of<VariableSymbol>(sym) &&
      to<VariableSymbol>(sym)->get_definition() != NULL) // var declaration
    return true;
  return false;
}

DefinitionBlock *get_corresponding_definition_block(SymbolTable *st) {

    SuifObject *parent = st->get_parent();
    // start with some known easy cases

    if (is_kind_of<ProcedureDefinition>(parent)) {
	return to<ProcedureDefinition>(parent)->get_definition_block();
	}

    if (is_kind_of<FileBlock>(parent)) {
	return to<FileBlock>(parent)->get_definition_block();
	}

    if (is_kind_of<ScopeStatement>(parent)) {
	return to<ScopeStatement>(parent)->get_definition_block();
	}
    if (is_kind_of<FileSetBlock>(parent)) {
	FileSetBlock *fsb = to<FileSetBlock>(parent);
	FileBlock *fb = fsb->get_file_block(0);
	return fb->get_definition_block();
	}

    // we don't know so we guess

    ObjectFactory* of = st->get_object_factory();

    // get the 'basetype' of the objects that are investigated for replacement
    // in reality we are actually iterating over the pointers
    MetaClass* mc = of->find_meta_class( DefinitionBlock::get_class_name() );


    PointerMetaClass* what = of->get_pointer_meta_class( mc, true );

    // obtain an iterator iterating over all owned pointers
    Iterator* it = object_iterator_ut( ObjectWrapper(parent), mc, what );
    if ( it->is_valid() ) {
      DefinitionBlock *defblock = *(DefinitionBlock**)it->current();
      delete it;
      return(defblock);
      }
    suif_assert_message(false, ("Could not find DefinitionBlock for "
				"target symbol table\n"));
    return 0;
    }
