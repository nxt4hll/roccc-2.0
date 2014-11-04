/*
 * file : suif_gc_obj.h
 */

#include "suif_gc_obj.h"

class GCList : SuifGcObj {
private:
  const SuifGcObj* const _obj;
  GCList      *       _next;
public:
  GCList(const SuifGcObj* obj, GCList* lst) : _obj(obj), _next(lst) {};

  GCList(void) : _obj(0), _next(0) {
  }

  void deposit(const SuifGcObj* obj) {
    GCList* l = new GCList(obj, _next);
    _next = l;
  }

  void withdraw(const SuifGcObj* obj) {
    if (_next == 0) return;
    if (_next->_obj == 0)
      _next = _next->_next;
    if (_next->_obj == obj) {
      _next = _next->_next;
      return;
    }
    _next->withdraw(obj);
  }
};


static GCList suif_gc_bank;

void SuifGcObj::register_object(const SuifGcObj* obj)
{
  suif_gc_bank.deposit(obj);
}


void SuifGcObj::unregister_object(const SuifGcObj* obj)
{
  suif_gc_bank.withdraw(obj);
}

