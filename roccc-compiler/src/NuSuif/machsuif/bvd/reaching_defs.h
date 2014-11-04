/* file "bvd/reaching_defs.h" -- reaching-definitions solver */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef BVD_REACHING_DEFS_H
#define BVD_REACHING_DEFS_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "bvd/reaching_defs.h"
#endif

#include <machine/machine.h>
#include <bvd/liveness.h>	// RegPartition

/*
 * A `DefTeller' maps an instruction to a set of zero or more small
 * integers, each uniquely identifying a quantity that the instruction
 * defines.  It suffices to use a set instead of a list.  If an instruction
 * defines the same item twice, say, the two definitions can be treated as
 * one.
 *
 * (teller: One who counts or keeps tally.)
 *
 * The abstract class doesn't specify what kinds of quantities an
 * instruction may define.  That's left to concrete subclasses.
 *
 * Methods: if `mi' is a machine instruction, then:
 *
 * insert_definees(mi, s) inserts the integer identifiers for the items defined
 *			  by `mi' into set `s'.
 * num_definees()	  returns the number of distinct definees seen by
 *			  definees() since this DefTeller instance was
 *			  constructed.
 */

class DefTeller {
public:
    DefTeller() { }
    virtual ~DefTeller() { }

    virtual void insert_definees(Instr*, NatSet *definees) const = 0;
    virtual int num_definees() const = 0;
};

/*
 * A `VarDefTeller' identifies two kinds of definees in a machine
 * instruction: virtual registers and local variables whose address is
 * never taken.
 */

class VarDefTeller : public DefTeller {
public:
    VarDefTeller(OptUnit *opt_unit, OpndCatalog *catalog) {
        the_opt_unit = opt_unit;
        the_catalog = catalog;
    }
    virtual ~VarDefTeller() { }

    void insert_definees(Instr*, NatSet *definees) const;
    int num_definees() const;

protected:
    OptUnit *the_opt_unit;
    OpndCatalog *the_catalog;
};

/*
 * A `RegDefTeller' identifies register-like definees in a machine
 * instruction: hard registers, virtual registers, and local variables
 * whose address is never taken.
 */
class RegDefTeller : public DefTeller {
public:
    RegDefTeller(OptUnit *opt_unit, OpndCatalog *catalog)
	: the_opt_unit(opt_unit), the_catalog(catalog),
	  the_partition(new RegPartition(catalog))
    { }
    virtual ~RegDefTeller() { delete the_partition; }

    void insert_definees(Instr*, NatSet *definees) const;
    int num_definees() const;

protected:
    OptUnit *the_opt_unit;
    OpndCatalog *the_catalog;
    const RegPartition *the_partition;
};

/*
 * Class DefMap
 *
 * Map between an instruction and its "definition point identifiers".
 *
 * A definition point is a point in the program at which some quantity
 * (typically a variable) is defined.  Since an instruction may define
 * zero, one, or many such things, it may have multiple definition
 * points.
 *
 * Class `DefMap' can be used to number the definition points in a
 * procedure being compiled, assigning a sequence of consecutive small
 * integers to each definition instruction.  Such integer identifiers can
 * then be used to index bit vectors in data-flow analysis.
 *
 * If `mi' is a machine-instruction pointer, `def_point' is a definition
 * point identifier, and `count' is the number of definition points
 * represented by `mi', then:
 *
 * num_def_points()	returns the number of definition points in the map
 * 			so far.
 * enter(mi, count)	enters `mi' in the map, associated with a sequence
 * 			of `count' definition point id's.  Returns the
 * 			first definition point id of the sequence, or
 * 			else -1 if `count' is 0.
 * lookup(def_point)	returns (a pointer to) the instruction containing
 * 			the definition point whose id is `def_point'.
 * 			Returns NULL if no such instruction is in the map.
 * first_point(mi)	returns the first definition point id for `mi',
 * 			or else 0 if `mi' has no definition points or
 * 			isn't mapped.
 * point_count(mi)	returns the number of definition points in `mi', or
 * 			else 0 if `mi' has no definition points or
 * 			isn't mapped.
 */
class DefMap {
  public:
    DefMap();
    ~DefMap() { }

    int num_def_points() const;
    int enter(Instr *mi, int count);
    Instr *lookup(int def_point) const;
    int first_point(Instr *mi) const;
    int point_count(Instr *mi) const;

  private:
    int next_def_point;
    Vector<Instr*> table;
    HashMap<Instr*, int> def_point_first;
    HashMap<Instr*, int> def_point_count;
};

/*
 * class ReachingDefs
 *
 * Use this class to initiate reaching-definitions analysis (by
 * constructing an instance of ReachingDefs) and to access the results
 * (by using the public methods of the base class Bvd).
 *
 * In addition to those inherited methods, class ReachingDefs gives you
 * two other useful methods:
 *
 * o  map() returns a pointer to a new DefMap covering the instructions
 *    in the program under analysis.
 *
 * o  def_points_for(definee) gives you all of the definition points for a
 *    particular "definee", be it a variable, a register, or whatever.
 *    Its argument is the integer id of the definee (as assigned by a
 *    DefTeller).  Its result is a pointer to a set of definition-point
 *    identifiers (also small integers).
 */

class ReachingDefs : public Bvd {
  public:
    ReachingDefs(Cfg *graph, DefTeller *teller);

    virtual ~ReachingDefs() { delete def_map; }

    virtual void find_kill_and_gen(Instr*);

    virtual const NatSet* kill_set() const  { return &instr_kill_set; }
    virtual const NatSet* gen_set()  const  { return &instr_gen_set; }

    const DefMap* map() const		    { return def_map; }

    const NatSet* def_points_for(int definee) const;

  protected:
    DefMap *def_map;		// map between instr and its def-point range
    DefTeller *def_teller;	// passed in via ReachingDefs constructor
    NatSetSparse instr_kill_set;// def-points killed by current instruction
    NatSetSparse instr_gen_set;	// def-points genned by current instruction
    Vector<NatSetSparse>	// map from definee id to its def-points
	def_point_sets;
    NatSetSparse empty_set;	// for use when a "definee" is undefined

    virtual int num_slots() const
	{ return def_map->num_def_points(); }
};

#endif /* BVD_REACHING_DEFS_H */
