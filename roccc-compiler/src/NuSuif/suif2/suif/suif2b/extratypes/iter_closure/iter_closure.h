#ifndef __ITER_CLOSURE__
#define __ITER_CLOSURE__
//#include <gc_cpp.h>

template<class T>
class IterClosure/*: public gc_cleanup */{
public:
  IterClosure<T>(){};
  virtual void apply(T *t) = 0;
};

template<class T>
class DeletingIterClosure : public IterClosure<T> {
public:
  DeletingIterClosure<T>() : IterClosure<T>() {};

  virtual void apply(T *t) {
    delete t->get_elt();
    delete t;
  }
};

#endif
