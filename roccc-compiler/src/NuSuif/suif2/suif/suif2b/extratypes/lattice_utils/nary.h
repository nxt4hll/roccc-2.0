#ifndef NARY_H
#define NARY_H

#include "bit_vector/bit_vector.h"
#include "common/suif_vector.h"


template <class Rel>
class NaryL {
  suif_vector<Rel> *_values;
  Rel _default_value;
 public:
  NaryL(); /* default is top */
  NaryL(const NaryL &other);
  NaryL(const Rel &default_value);
  NaryL operator~() const;
  static NaryL do_negate(const NaryL &val);
  NaryL &operator =(const NaryL &other);
  bool operator ==(const NaryL &other) const;
  bool operator !=(const NaryL &other) const;

  Rel& operator[](size_t i); /* expand */
  Rel operator[](size_t i) const;

  Rel get_value(size_t i) const;
  void set_value(size_t i, const Rel &val);

  bool is_empty() const;
  size_t size() const; /* for this dimension */
  void set_default_value(const Rel &default_value);
  Rel get_default_value() const;
  size_t significant_bit_count() const;
  bool do_meet_with_test(const NaryL &other);
  static NaryL do_meet(const NaryL &val1, const NaryL &val2);
protected:
  void expand_to(size_t);
};

#include <common/suif_vector.h>

template <class Rel>
NaryL<Rel>::NaryL() :
  _values(new suif_vector<Rel>),
  _default_value()
{}
  
    
/* default is top */
template <class Rel>
NaryL<Rel>::NaryL(const NaryL &other) :
  _values(new suif_vector<Rel>),
  _default_value(other._default_value)
{
  for (typename suif_vector<Rel>::iterator iter = other._values->begin();
       iter != other._values->end(); iter++) {
    _values->push_back(*iter);
  }
}

template <class Rel>
NaryL<Rel>::NaryL(const Rel &default_value) :
  _values(new suif_vector<Rel>),
  _default_value(default_value)
{
}

template <class Rel>
NaryL<Rel> NaryL<Rel>::do_negate(const NaryL &val) {
  assert(val.is_empty());
  return(NaryL<Rel>(do_negate(val._default_value)));
}

template <class Rel>
NaryL<Rel> &NaryL<Rel>::operator =(const NaryL &other) {
  _values->clear();
  for (typename suif_vector<Rel>::iterator iter = other._values->begin();
       iter != other._values->end(); iter++) {
    _values->push_back(*iter);
  }
  
  _default_value = other._default_value;
  return(*this);
}

template <class Rel>
bool NaryL<Rel>::operator !=(const NaryL &other) const {
  return(!(*this == other));
}

template <class Rel>
bool NaryL<Rel>::operator ==(const NaryL &other) const {
  if (_default_value != other._default_value) return false;
  if (size() != other.size()) return false;
  size_t m = size();
  for (size_t i = 0; i < m ; i++) {
    if ((*_values)[i] != (*other._values)[i])
      return(false);
  }
  return(true);
}

template <class Rel>
bool NaryL<Rel>::is_empty() const {
  return(size() == 0);
}

template <class Rel>
size_t NaryL<Rel>::size() const {
  return(_values->size());
}

template <class Rel>
void NaryL<Rel>::expand_to(size_t i) {
  while(i >= size())
    _values->push_back(_default_value);
}

template <class Rel>
Rel& NaryL<Rel>::operator[](size_t i) {
  expand_to(i);
  return((*_values)[i]);
}

template <class Rel>
Rel NaryL<Rel>::operator[](size_t i) const {
  if ( i < size())
    return((*_values)[i]);
  return(_default_value);
}



template <class Rel>
Rel NaryL<Rel>::get_value(size_t i) const {
  if (i >= size())
    return(_default_value);
  return((*_values)[i]);
}

template <class Rel>
void NaryL<Rel>::set_value(size_t i, const Rel &val) {
  if (i >= size() && val == _default_value ) return;
  expand_to(i);
  (*_values)[i] = val;
}


template <class Rel>
void NaryL<Rel>::set_default_value(const Rel &default_value) {
  assert(is_empty());
  _default_value = default_value;
}

template <class Rel>
Rel NaryL<Rel>::get_default_value() const {
  return(_default_value);
}

/* clearly, this could be cached as long as all modification
 * is done through this 
 */
template <class Rel>
size_t NaryL<Rel>::significant_bit_count() const {
  size_t m = size();
  for (typename suif_vector<Rel>::iterator it = _values->begin();
       it != _values->end(); it++) {
    size_t n = (*it).significant_bit_count();
    if (n > m) m = n;
  }
  return(m);
}

template <class Rel>
bool NaryL<Rel>::do_meet_with_test(const NaryL &other) {
  NaryL val = do_meet(*this, other);
  if (val == *this) return false;
  *this = val;
  return(true);
}

template <class Rel>
NaryL<Rel> NaryL<Rel>::do_meet(const NaryL &val1, const NaryL &val2) {
  Rel new_default = val1._default_value.do_meet(val1._default_value,
						val2._default_value);
  NaryL new_nary(new_default);
  size_t m = val1.size();
  size_t n = val2.size();
  if (n > m) m = n;
  for (size_t i = 0; i < m; i++)
    {
      Rel rel_val1 = val1[i];
      Rel rel_val2 = val2[i];
      new_nary.set_value(i, rel_val1.do_meet(rel_val1, rel_val2));
    }
  return(new_nary);
}



#endif /* NARY_H */
