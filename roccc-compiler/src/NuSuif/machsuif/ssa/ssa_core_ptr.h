/* file "ssa/ssa_core_ptr.h" */

/*
    Copyright (c) 2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef SSA_SSA_CORE_PTR_H
#define SSA_SSA_CORE_PTR_H

#include <machine/copyright.h>

class SsaCore;

/*
 * A smart pointer class used because Hoof won't let us extend the
 * destructor of a hoof-generated class.  In SsaCfg we use the following
 * class in lieu of SsaCore*, pure so that we can define its destructor to
 * invoke delete on the underlying SsaCore pointer.
 */
class SsaCorePtr
{
  public:
    SsaCorePtr() : underlying_ptr(0) { }
    ~SsaCorePtr();
    SsaCore* operator->() { return underlying_ptr; }

    SsaCore* get_underlying_ptr() const { return underlying_ptr; }
    void set_underlying_ptr(SsaCore *up) { underlying_ptr = up; }

  protected:
    SsaCore *underlying_ptr;
};

#endif /* SSA_SSA_CORE_PTR_H */
