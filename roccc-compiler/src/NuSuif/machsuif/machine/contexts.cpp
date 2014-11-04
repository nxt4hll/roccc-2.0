/* file "machine/contexts.cpp" */

/*
   Copyright (c) 2000 The President and Fellows of Harvard College

   All rights reserved.

   This software is provided under the terms described in
   the "machine/copyright.h" include file.
   */

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "machine/contexts.h"
#endif

#include <machine/substrate.h>
#include <machine/init.h>
#include <machine/problems.h>
#include <machine/note.h>
#include <machine/contexts.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

class ContextCache : public Map<IdString, Context*> {
  public:
    ~ContextCache();
};

typedef Map<IdString, Context*>::iterator ContextCacheHandle;

ContextCache::~ContextCache()
{
  for (ContextCacheHandle h = begin(); h != end(); ++h)
    delete (*h).second;    
}

ContextCache the_context_cache;


Context *the_context;

MachineContext::MachineContext()
{
  cached_printer = NULL;
  cached_c_printer = NULL;
  cached_code_fin = NULL;
}

MachineContext::~MachineContext()
{
  delete cached_printer;
  delete cached_c_printer;
  delete cached_code_fin;
}

/* --------------------------  context creation  ---------------------------- */

typedef Map<IdString, Context*(*)()>::iterator ContextCreatorHandle;

Map<IdString,Context*(*)()> the_context_creator_registry;

  Context*
target_context(FileBlock* file_block)
{
  OneNote<IdString> libnote = get_note(file_block, k_target_lib);
  claim(!is_null(libnote), "expected target_lib annotation on file block");
  IdString libname = libnote.get_value();

  return find_context(libname);
}

  Context*
find_context(IdString libname)
{
  ContextCacheHandle h = the_context_cache.find(libname);

  if (h != the_context_cache.end())
    return (*h).second;

  ContextCreatorHandle c = the_context_creator_registry.find(libname);

  if (c == the_context_creator_registry.end()) {	// library needs loading
#ifdef NCI_1999
    the_suif_env->get_dll_subsystem()->loadAndInitializeDLL(libname);
#else
    the_suif_env->require_module(libname);
#endif

    c = the_context_creator_registry.find(libname);
    claim(c != the_context_creator_registry.end(),
        "Library `%s' didn't register a context creator", libname.chars());
  }

  Context *result = ((*c).second)();			// create target context
  the_context_cache[libname] = result;
  return result;
}
