#ifndef _SUIF_GC_OBJ_H_
#define _SUIF_GC_OBJ_H_

#ifdef SUIFGC
#include "gc/gc_cpp.h"
#endif


class SuifGcObj

#ifdef SUIFGC
  : public gc
#endif

{
 public:
#ifndef SUIFGC
    virtual ~SuifGcObj() {};
#endif
  static void register_object(const SuifGcObj*);
  static void unregister_object(const SuifGcObj*);
};


#endif // _SUIF_GC_OBJ_H_
