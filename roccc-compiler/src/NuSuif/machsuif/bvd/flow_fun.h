/* file "bvd/flow_fun.h" -- Flow functions */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef BVD_FLOW_FUN_H
#define BVD_FLOW_FUN_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "bvd/flow_fun.h"
#endif

#include <machine/machine.h>

class FlowFun {
  public:
    FlowFun(int num_slots_hint = 0);

    FlowFun(const FlowFun&);
    void operator=(const FlowFun&);
    
    void set_to_top();
    void set_to_top(int slot);
    void set_to_bottom();
    void set_to_bottom(int slot);
    void set_to_id();
    void set_to_id(int slot);
    
    void apply(NatSetDense &updated);
    
    void print(int bound, FILE* = stdout);
    
  private:
    NatSetDense _id;
    NatSetDense _cs;
};

inline void
FlowFun::set_to_top()
{
    // set function to "constant 1" on all bits
    _id.remove_all();
    _cs.insert_all();
}

inline void
FlowFun::set_to_top(int slot)
{
    claim(slot >= 0, "FlowFun::set_to_top - negative slot %d", slot);
    _id.remove(slot);
    _cs.insert(slot);
}

inline void
FlowFun::set_to_bottom()
{
    // set function to "constant 0" on all bits
    _id.remove_all();
    _cs.remove_all();
}

inline void
FlowFun::set_to_bottom(int slot)
{
    claim(slot >= 0, "FlowFun::set_to_bottom - negative slot %d", slot);
    _id.remove(slot);
    _cs.remove(slot);
}

inline void
FlowFun::set_to_id()
{
    // set function to "identity" on all bits
    _id.insert_all();
    _cs.remove_all();
}

inline void
FlowFun::set_to_id(int slot)
{
    _id.insert(slot);
    _cs.remove(slot);
}

inline void
FlowFun::apply(NatSetDense &updated)
{
    updated *= _id;
    updated += _cs;
}

#endif /* BVD_FLOW_FUN_H */
