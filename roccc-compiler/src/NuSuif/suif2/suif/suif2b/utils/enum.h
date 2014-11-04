#ifndef _ENUM_H_
#define _ENUM_H_


template<class T>
class EnumHelper {
 public:
  virtual bool is_valid(void) = 0;
  virtual T    current(void) =0;
  virtual void next(void) = 0;
};


template<class T>
class Enum {
 private:
  EnumHelper<T>* const _enum_helper;
  bool                 _is_owner;
 public:
  Enum(EnumHelper<T>* e) : _enum_helper(e), _is_owner(true) {};

  Enum(const Enum& that) :
    _enum_helper(that._enum_helper), _is_owner(that._is_owner) {
    const_cast<Enum&>(that)._is_owner = false; // transfer ownership to this
  };

  ~Enum(void) { if (_is_owner) delete _enum_helper; };

  bool is_valid(void) { return _enum_helper->is_valid(); };

  T    current(void) { return _enum_helper->current(); };

  void next(void) { _enum_helper->next(); };

};




#endif /* _ENUM_H_ */
