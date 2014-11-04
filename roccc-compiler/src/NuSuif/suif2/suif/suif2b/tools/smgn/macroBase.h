#ifndef __MACROBASE__
#define __MACROBASE__

#include <stddef.h>
#include <stdio.h>
#include "common/simple_stack.h"
#include "common/MString.h"
#include "common/ref_counter.h"

#ifndef NULL
#define NULL 0
#endif

//	A macro exapnder
//
//

class MacroIter;

class MacroObject;

typedef RCPointer<MacroObject> MacroObjectPtr;
typedef DerivedRCPointer<MacroIter,MacroObjectPtr> MacroIterPtr;

class MacroObject : public RefCountedClass
    {
    public:

	static const LString ClassName;

	MacroObject() {}

        virtual ~MacroObject() {}
 
        // get the text associated with the object
        virtual const String get_text() const = 0;
 
        // return an iterator associated with this object
        // If this object is not iterated, just return the object using
        // SingleMacroIter. You can also return NULL if there are no objects
        // to iterate over (or you can return an iterator with isDone true)
 
        virtual MacroIterPtr get_iter() = 0;
 
        // get a child object. Return NULL if does not exist.
 
        virtual MacroObjectPtr  get_child(const LString &name) const {return NULL;}
 
        virtual String child_name_list() const {return String("<none>");}
 
        virtual void Print(int indent = 0) const
                { fprintf(stdout,"%s",(const char *)get_text()); }
 
        virtual bool IS_SIMPLE_list() {return false;}
	virtual void set_type_name(const LString &)  {}
	virtual LString get_instance_class_name() const = 0;
        virtual LString object_type_name() const {
            return get_instance_class_name();
        }

	// is_instance_of checks for a kind in the application domain/ This
        // is in contradistinction to isKindOf which checks the type
        // of the representation (ie, the latter is what kind of macro object)
  	//
	virtual bool is_instance_of(const LString &kind ) const
	    { return kind == object_type_name();}
        static const LString & get_ClassName() {return ClassName;}
        virtual bool isKindOf( const LString &kind ) const
            { return (kind == ClassName);
            }
	virtual void perform_final_cleanup() {}
    };


//	All classes which are passed to the macro processor must be MacroObjects

//	For forall, a MacroIter is required. 

class MacroIter : public MacroObject
    {
        static const LString ClassName;
    public:

	// get the current object
	virtual MacroObjectPtr Item() const = 0;

	// move to next, return true if successful
	virtual bool Next() = 0;

	// are we at end

	virtual bool isDone() = 0;
	virtual bool isLast() = 0;
	virtual int pos() = 0;

	virtual const String get_text() const;

	// return an iterator associated with this object
	// If this object is not iterated, just return the object using
	// SingleMacroIter. You can also return NULL if there are no objects
	// to iterate over (or you can return an iterator with isDone true)

	virtual MacroIterPtr get_iter();

	// get a child object. Return NULL if does not exist.

	virtual MacroObjectPtr  get_child(const LString &name) const;

	virtual String child_name_list() const;

	virtual void Print(int indent = 0) const;

	virtual LString get_instance_class_name() const {
        return ClassName;
    }
        static const LString & get_ClassName() {return ClassName;}
        virtual bool isKindOf( const LString &kind ) const
            { return ((kind == ClassName) || MacroObject::isKindOf(kind));
            }
    };

MacroIter * to_MacroIter(MacroObject * );

//	Often, simple_stack objects are iterated over. 
//	Here is a template of simple_stack base macro iterator

template <class x> class simple_stack_MacroIter : public MacroIter
    {
	int iter_pos;
	const ref_stack<x> list;
    public:
	simple_stack_MacroIter(const ref_stack<x> &the_list) : iter_pos(0), list(the_list) {}
	
	// note that this is an x not an x *. which means this template can only
	// be instantiated with pointer types!!!!!
	MacroObjectPtr  Item() const
	    {
	    if (iter_pos >= list.len()) 
		return NULL;
	    return list[iter_pos];
	    }

	bool Next() 
	    {
	    iter_pos ++; 
	    if (iter_pos >= list.len()) 
		{
		iter_pos = list.len();
		return false;
		}
	    else 
		return true;
	    }

	bool isDone() 
	    {
	    return (iter_pos >= list.len());
	    }

	bool isLast()
	    {
	    return (iter_pos == (list.len() - 1));
	    }

	int pos() {return iter_pos;}
    };

// You also often want to pass in a null iterator or an iterator with
// exactly one object in it. When a forall is done on an object which is
// not a list, just the object itself should be returned. 

class SingleMacroIter : public MacroIter
    {
        MacroObjectPtr object;
    public:
	SingleMacroIter(MacroObjectPtr the_object = NULL) 
		: object(the_object) {}

        // note that this is an x not an x *. which means this template can only
        // be instantiated with pointer types!!!!!
        MacroObjectPtr  Item() const {return object;}

        bool Next() {object.make_null();return false;}
        bool isDone() {return object.is_null();}
	bool isLast() {return true;}

	int pos() {return 0;}

    };


class AbstractMacroListObject : public MacroObject
    {
        static const LString ClassName;

    protected:
        bool is_temp;

    public:

        AbstractMacroListObject() : is_temp(false) {}

	virtual ~AbstractMacroListObject() {}

        const String get_text() const    {return "";}

        virtual void AddObject(MacroObjectPtr object) = 0;

        virtual void CutBack(int new_len)=0;

        virtual MacroIterPtr get_iter()= 0;

	virtual int length() const = 0;

        virtual MacroObjectPtr  get_item(int i) const = 0;

	virtual void Print(int indent = 0) const = 0;

        bool get_is_temp()    {return is_temp;}

        void set_is_temp()    {is_temp = true;}

        void set_is_not_temp() {is_temp = false;}

	virtual void AddObjectList(const AbstractMacroListObject &x) = 0;

	virtual void reset()=0;

	virtual bool is_simple_list()      {return true;}

	virtual LString get_instance_class_name() const {
        return ClassName;
    }

        static const LString & get_ClassName() {return ClassName;}
        virtual bool isKindOf( const LString &kind ) const
            { return ((kind == ClassName) || MacroObject::isKindOf(kind));
            }


	MacroObjectPtr  get_child(const LString &name) const;

    };

AbstractMacroListObject * to_AbstractMacroListObject(MacroObject* p);

//	Only one parameter gets passed in. This has no name. To pass in real parameters,
//	you generally want to pass an instance of the following class. The top level name resolution
// 	will then be done in the class object


class AbstractNamedList : public MacroObject
    {
        static const LString ClassName;

    public:
        virtual ~AbstractNamedList();

	virtual String child_name_list() const = 0;

	virtual void AddObject(const LString &name, MacroObjectPtr object)= 0;

	virtual void CutBack(int new_len)=0;

        virtual MacroIterPtr get_iter() = 0;

	virtual MacroObjectPtr  get_item(int i) const = 0;

	virtual void Print(int indent = 0) const = 0;

	virtual bool is_simple_list()             {return false;}

	virtual  MacroObjectPtr  get_child(const LString &name) const = 0;

	virtual int length()const  = 0;

	virtual LString get_instance_class_name() const {
        return ClassName;
    }
        static const LString & get_ClassName() {return ClassName;}
        virtual bool isKindOf( const LString &kind ) const
            { return ((kind == ClassName) || MacroObject::isKindOf(kind));
            }


	const String get_text() const    {return "";}

	virtual MacroObjectPtr get_as_list();


    };

AbstractNamedList * to_AbstractNamedList(MacroObject * );


//	Implementation of the simple string valued macro object


class AbstractStringMacroObject : public MacroObject
    {
        static const LString ClassName;

    public:

        virtual MacroIterPtr get_iter() = 0;

        // get a child object. Return NULL if does not exist.

	virtual void set_text(const String &the_text) = 0;

	virtual const String get_text() const = 0;

	virtual LString get_instance_class_name() const {
        return ClassName;
    }
        static const LString & get_ClassName() {return ClassName;}
        virtual bool isKindOf( const LString &kind ) const
            { return ((kind == ClassName) || MacroObject::isKindOf(kind));
            }

    };

AbstractStringMacroObject*  to_AbstractStringMacroObject(MacroObject * p);

#endif
