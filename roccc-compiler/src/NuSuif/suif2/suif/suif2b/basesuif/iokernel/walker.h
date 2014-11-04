#ifndef SUIFKERNEL__WALKER_H__
#define SUIFKERNEL__WALKER_H__

#include "iokernel_forwarders.h"

/**
  * @file
  * This is the base class for all Walkers.
  * @see suif_walker.h
  * @see SelectiveWalker
  * @see GroupWalker
  * @see group_walker.h
  */

/**
  * See also the SelectiveWalker and GroupWalker classes found in
  * suifkernel/group_walker.h
  * These allow you to visit only suif objects of a certain type, and groups
  * of type.
  */

/**
  * DL Moore July 21 1999
  * Added a set_parent and get_parent method. set_parent is called just before
  * walking an aggregate for a SuifObject. It is not called for lists. 
  * It should always contain a valid value (if not NULL) for a replace call
  */


/** \class Walker walker.h iokernel/walker.h
  * This is the base class for all Walkers.
  * This class is abstract. Generally, you will want to derive from SuifWalker
  * found in suifkernel/suif_walker.h.
  * The normal use of walkers is described in that file.
  * @see suif_walker.h
  */
class Walker {
  
 public:
  
  /** Status codes that can be returned by the operator ().
    * The internal status should not be modified by the operator code.
    */
  enum ApplyStatus {
    /** Continue iteration. */
    Continue,
    
    /** Stop iteration, normal termination */
    Stop,
    
    /** Stop with error condition */
    Abort,
    
    /** Do not walk sub-objects of this object (pre-order only) */
    Truncate,
    
    /** Object has been replaced with a new object.
      *	You must set the address of the new object by calling set_address
      * before returning.
      */
    Replaced
  }; // enum ApplyStatus
	  
  Walker(SuifEnv *the_env);

  /** In case subclasses want to delete something */
  virtual ~Walker() {};

  /** Called by the walker to perform action on the data in \a address.
    * Subclass must overwrite this method.
    * @param address  address of the data to be applied on.
    * @param _meta    meta class of the data in \a address.
    */
  virtual ApplyStatus operator () (Address address,
				   const MetaClass *_meta) = 0;

  /** Determine if the tree rooted at the address should be walked
    * Default implementation returns true iff \a is_owned is true.
    * You can, for example, return false if you know that a node
    * of interest cannot be contained inside a given node.
    *
    * Notice the difference to is_visitable. The latter determines
    * if the function operator should be called on a given node. This
    * function controls walking of an entire subtree.
    *
    * @param address (a pointer to) a pointer.
    * @param is_owned true if the pointer pointed to by \a address is owned
    *                 by this object.
    * @param _meta  the meta class of the pointer in \a address.  This meta
    *               class will be a subclass of PointerMetaClass.
    */
  virtual bool is_walkable(Address address,
			   bool is_owned,
			   const MetaClass *_meta);

  /** Determine if operator() should be called on the data in \a address.
    * Default implementation always true.
    * @param address points to the data.
    * @param _meta   the meta class of the data pointed to by \a address.
    */
  virtual bool is_visitable(Address address,
			    const MetaClass *_meta) const;

  /** Set the order of walking to post_order.
    * Default is pre_order.
    */
  void set_post_order();

  /** Set the order of walking to pre_order, which is the default.
    */
  void set_pre_order();
			   
  /** Get the order of walking.
    * @return true iff the order is pre-order.
    */
  bool get_is_pre_order() const;
  
  void set_address(Address new_address);

  Address get_address() const;
  
  SuifEnv *get_env() const;
  
  void set_parent(Address addr);
  
  Address get_parent() const;

  /**	It is possible for nodes to be replaced during a walk. 
   *    This is important when walking lists, as the contents of
   *	the list can change while the list is being walked. 
   *
   *	So, if changes can occur, we must copy the list before
   *	walking it. get_makes_changes returns true if a walker changes
   *	the tree.
   */
  virtual bool get_makes_changes() const;

  /**	Should the node be ignored because it has been deleted or changed?
   *	This exists so that we do not need to try to develop the real
   *    meta class for a deleted object. Doing that will cause a seg fault 
   */
  virtual bool is_changed(Address addr) const;

 private:
  SuifEnv *_env;
  bool _is_pre_order;
  Address _address;
  Address _parent;

  /* play it safe */
  Walker &operator=(const Walker &);
  Walker(const Walker &);
};

#endif

