#ifndef SUIFKERNEL__ITER_H
#define SUIFKERNEL__ITER_H

#include "iokernel/meta_class.h"
#include "iokernel/cast.h"
#include "suifkernel/suifkernel_messages.h"

/** 
 *   \file iter.h
 *   An iterator interface
 * 
 */ 

/**
 * \class IterHelper iter.h suifkernel/iter.h
 * Used by the Iter Template class
 */

class IterHelper {
  void clone_iter();
public:
  /** returns the current address/metaclass pair */
  ObjectWrapper current_object() const;

  const MetaClass* current_meta_class() const;

  const LString& current_name() const;

  /** Returns true if more elements are left in the iterator */
  bool is_valid() const;

  /** Returns the next element in the iterator */
  void next();

  /** Returns the previous element in the iterator */
  void previous();

  /** Sets the current position to a particular value */ 
  void set_to( size_t index );

  /** Returns the number of elements in the whole collection */
  size_t length() const;

  void *current() const;

  IterHelper( Iterator* iter );
  ~IterHelper();

  IterHelper(const IterHelper &x);

  IterHelper & operator =(const IterHelper &x);

private:
  Iterator* _iter;
  bool _owned;
};

/**
 * \class Iter iter.h suifkernel/iter.h
 * A template for iterators (using the iterhelper)
 *
 * Common usage:
 * \code
 * for (Iter<Expression*> iter = st->get_expression_iterator(); 
 *      iter.is_valid(); iter.next()) {
 *     Expression *e = iter.current();
 * }
 * \endcode
 * An iterator is an ordered collection of objects.
 * It has a concept of a current element position 
 * which is initialized to the first element of the collection.
 * One can advance the position by using <tt>next</tt> or set it back
 * by using <tt>previous</tt>. 
 * 
 * The current element can be retrieved with <tt>current</tt>.
 * Other useful methods: 
 * <ul><li>   
 * is_valid.  Returns T if more elements exist beyond the current index.</li>
 * <li>  
 * set_to.  Set the current index to a particular value. </li>   
 * <li>
 * length. Returns the number of elements in the whole collection. </li>
 * </ul>  
 */

template<class T>
class Iter : public IterHelper {
  // the current type
  public:

  virtual T& current() const {
    suif_assert( is_valid() );
    void *v = IterHelper::current();
    T& val = *(T*)v;
    return val;
  }

  Iter( Iterator* iter ) : IterHelper( iter ) {}
  Iter( const Iter &other ) : IterHelper( other ) {}
  virtual ~Iter(void) {};

};


#endif
