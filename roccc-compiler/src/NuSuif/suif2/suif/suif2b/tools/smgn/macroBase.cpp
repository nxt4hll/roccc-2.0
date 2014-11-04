#include "macroBase.h"
const LString AbstractMacroListObject::ClassName("AbstractMacroListObject");
const LString AbstractNamedList::ClassName("AbstractNamedList");
const LString AbstractStringMacroObject::ClassName("AbstractStringMacroObject");

AbstractNamedList::~AbstractNamedList() {}

AbstractNamedList * to_AbstractNamedList(MacroObject *p)
    {
	if (p == NULL)
	     return NULL;
        if( p->isKindOf(AbstractNamedList::get_ClassName() ))
            return (AbstractNamedList *)p;
        else
            return NULL;
    }

AbstractMacroListObject * to_AbstractMacroListObject(MacroObject *p)
    {
        if (p == NULL)
             return NULL;

        if( p->isKindOf( AbstractMacroListObject::get_ClassName() ))
            return (AbstractMacroListObject *)p;
        else
            return NULL;
    }

AbstractStringMacroObject * to_AbstractStringMacroObject(MacroObject *p)
    {
        if (p == NULL)
             return NULL;

        if( p->isKindOf( AbstractStringMacroObject::get_ClassName() ))
            return (AbstractStringMacroObject *)p;
        else
            return NULL;
    }
