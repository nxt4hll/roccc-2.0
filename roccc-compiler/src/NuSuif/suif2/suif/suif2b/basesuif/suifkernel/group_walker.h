#ifndef SUIFKERNEL__GROUP_WALKER
#define SUIFKERNEL__GROUP_WALKER

#include "suif_walker.h"
#include "suif_object.h"

#include "common/sparse_vector.h"
#include "common/suif_hash_map.h"

/**
 * \file group_walker.h
 * This file contains three useful walkers: 
 * SelectiveWalker, GroupWalker, and ReplacingWalker
 * It also contains the TypeBasedDispatch,
 */

 /**
  * \class SelectiveWalker group_walker.h suifkernel/group_walker.h
  *
  *	SelectiveWalker allows you to walk SuifObjects of a given type
  *	(including derived objects)
  *
  *	Derive a class from it and overide operator ().
  *	Instantiate it with the name of the class to be walked.
  */


class SelectiveWalker : public SuifWalker {
    private:
	bool is_visitable(Address address,const MetaClass *_meta) const;
    public:
  /**
   * Build a SelectiveWalker that only applies operator() to
   * objects of type \a the_type or its subclasses
   */
	SelectiveWalker(SuifEnv *the_env,const LString &the_type);

  /**
   * Subclasses must override this to perform their
   * actions
   */
	Walker::ApplyStatus operator () (SuifObject *x) = 0;

  /**
   * Accessor for the metaclass type name passed to the constructor
   */
	const LString & get_type() const;
    private:
	const LString type;
    };

/*
 * The typeless implementation for the TypeBasedDispatch template.
 */
class TypeBasedDispatchHelper {
    	SparseVector<void *> _vector;
        bool _locked;
    public:
	TypeBasedDispatchHelper();
  /**
   * Set a value for a metaclass
   * Once a lookup_value is called, using this function is
   * illegal.
   */
        void set_value(const LString &index, void *val);
  /**
   * find the appropriate T for this SuifObject
   * If the object does not have an entry, get the value
   * for it's metaclass.
   * Once this method is called, it is illegal to 
   * call put_value() again
   */
	void *lookup_value(SuifObject *x);
  /**
   * Get the value for this object, if there is
   * none, return NULL.
   */
	void *get_value(SuifObject *x) const;
    };

/**
 * Keep a table of values indexed by metaclass name.
 * The table must be completely initialized before looking up
 * values.
 * 
 * Used by the Group walker to choose the appropriate walker to call.
 * A GroupWalker allows any of a group of nodes to be walked, each with its own
 * apply operator. 
 */
template <class T>
class TypeBasedDispatch {
        TypeBasedDispatchHelper _helper;
    public:
	TypeBasedDispatch() {}
  /**
   * Set a value for a metaclass
   * Once a lookup_value is called, using this function is
   * illegal.
   */
  void set_value(const LString &index, T val) {
    _helper.set_value(index, (void*)val);
  }
	  
  /**
   * find the appropriate T for this SuifObject
   * If the object does not have an entry, get the value
   * for it's metaclass.
   * Once this method is called, it is illegal to 
   * call put_value() again
   */
  T lookup_value(SuifObject *x) {
    return((T)_helper.lookup_value(x));
  }
  /**
   * Get the value for this object, if there is
   * none, return NULL.
   */
  T get_value(SuifObject *x) const {
    return((T)_helper.get_value(index));
  }
};

/**
 *	\class GroupWalker group_walker.h suifkernel/group_walker.h
 *      A list of SelectiveWalkers that will
 *	be called for one pass of the tree.
 */

class GroupWalker : public SuifWalker {
	TypeBasedDispatch <SelectiveWalker*> *_vector;
    protected:
	Walker::ApplyStatus operator () (SuifObject *x);
    public:
        GroupWalker(SuifEnv *the_env);
        virtual ~GroupWalker();

        void append_walker(SelectiveWalker &x);

    };

/**
 *	\class ReplacingWalker group_walker.h suifkernel/group_walker.h
 *      Walk a tree and replace nodes with new ones
 *	
 *	It differs from a GroupWalker in the following ways:
 * <UL><LI>
 *		i) a suif_map is defined from object pointers to object pointers
 *		   This maps replaced objects into replacement objects
 * </LI><LI>
 *		ii) Objects are visited the first time they are seen, not when 
 *		    their owners are visited
 * </LI><LI>
 *		iii) The function operator (operator() ) does not actually
 *		    do the replace. It is done when you return.
 * </LI></UL>
 *
 * \par Example usage
 * ReplacingWalker walk;
 * walk.add_replacement(
 */
class ReplacingWalker : public GroupWalker {
    	suif_hash_map<Address,Object *> _map;
    public:
	ReplacingWalker(SuifEnv *the_env);
	virtual bool is_walkable(Address address,bool is_owned,
                                  const MetaClass *_meta);
        Walker::ApplyStatus operator () (SuifObject *x);

  /**	Add a replacement for an object.
   *    Note that if you enter a value 
   *    during a walk, it must be for a value that has not been visited yet
   */
	void add_replacement(Object *from,Object *to);

  /**	You can lookup replacement values here.
   */
	Object *get_replacement(Object *x) const;

  /**	This walker makes changes to the tree, so override get_makes_changes */
  	bool get_makes_changes() const;
	
  /**	Has a given node changed? If so, we do not walk it (it may even
   *    be deleted
   */
	bool is_changed(Address addr) const;

    /**	Export of the iterator for the map */
    suif_hash_map<Address,Object *>::iterator get_map_begin() {return _map.begin();}
    suif_hash_map<Address,Object *>::iterator get_map_end() {return _map.end();}
    suif_hash_map<Address,Object *>::const_iterator get_map_begin() const {return _map.begin();}
    suif_hash_map<Address,Object *>::const_iterator get_map_end() const {return _map.end();}



    };


#endif
