#ifndef SUIFKERNEL__SUIF_OBJECT_H
#define SUIFKERNEL__SUIF_OBJECT_H

#include "iokernel/object.h"
#include "iokernel/walker.h"

#include "suifkernel_forwarders.h"

/**
 * \class SuifObject suif_object.h suifkernel/suif_object.h
 * The base object for all IR nodes in the SUIF
 * representation.
 * 
 */

class SuifObject : public Object {
  friend class SuifEnv;
public:

  /**
   * use the default system cloner to clone this object
   * and all objects in the OWNERSHIP tree rooted at this
   * object.
   */
  virtual SuifObject* deep_clone( SuifEnv* suif_env = 0 ) const;
  /**
   * use the default system cloner to clone this object only.
   * all references that this object has to other SUIFObjects
   * will be copied to the new clone
   */
  virtual SuifObject* shallow_clone( SuifEnv* suif_env = 0 ) const;

  /**
   * Get the object's parent in the OWNERSHIP tree.
   * 
   * it MUST be the case that the PARENT also has a reference
   * to this object.  Any object removed from the tree
   * must set its parent to zero and remove the reference to
   * it from the parent.
   */
  virtual SuifObject* get_parent() const;

  /**
   * Set the object's parent.
   * It is an error to set the parent to a new object
   * if the parent is NOT NULL.
   * \par WARNING
   * using this method leaves the system in an
   * invalid state until the parent object
   * pointer to this object is removed.
   * Use this method with EXTREME caution
   */
  virtual void set_parent( SuifObject* object );

  /**
   * Get the object factory that built this object.
   */
  virtual ObjectFactory* get_object_factory() const;

  /**
   * Get the suif enviroment that built this object
   */
  virtual SuifEnv* get_suif_env() const;
  /**
   * output this object using the default system
   * printer to the default system output
   */
  virtual void print_to_default() const; // for debugging

  /**
   * output this object using the default system
   * printer to the output stream.
   */
  virtual void print( std::ostream& output ) const;

  /**
   * output this object using the default system
   * printer to a string
   */
  virtual String print_to_string() const;

  /**
   * replace this object with the new object.
   * This method will replace the parent pointer in this
   * object and set the reference from it's parent to the
   * new object.
   * The new object may be NULL
   */
  virtual int replace( SuifObject* original,
		       SuifObject* new_object,
                       bool fuse_if_possible = false );

  /**
   * A placeholder for a method that will
   * verify various interal invariants on an object.
   * Invariants are those like
   * <UL>
   * <LI> All of the SuifObjects that is owns
   * have parent pointers to this object,
   * <LI> Some objects have cross references that 
   *  are invariants. i.e. a ProcedureSymbol has
   *  a reference to its ProcedureDefinition.
   *  The ProcedureDefinition also has a
   *  reference back to the ProcedureSymbol.
   * </UL>
   * Currently, this method is not implemented
   */
  virtual void verify_invariants( ErrorSubSystem* message_destination );

  /**
   * This is an old print method to be used when
   * no system printer is installed
   */
  virtual void print(FormattedText &x) const;


  static const LString &get_class_name();

  virtual Walker::ApplyStatus walk(Walker &walk);


protected:
  SuifObject();
  static void constructorFunction( Address instance );

private:
  SuifObject* parent;
private:
  SuifObject(const SuifObject &);
  SuifObject& operator=(const SuifObject &);

};

/**	A useful routine for printing objects
 */

int printobj(SuifObject *obj);

// return true if an object with this meta class is a subclass
// of SuifObject
bool is_kind_of_suif_object_meta_class(const MetaClass *mc);

#endif
