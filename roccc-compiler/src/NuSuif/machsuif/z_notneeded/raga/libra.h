/* file "raga/libra.h" */

/*
    Copyright (c) 1995-99 The President and Fellows of Harvard University

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef RAGA_LIBRA_H
#define RAGA_LIBRA_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "raga/libra.h"
#endif

#include <machine/machine.h>


/* Entry type for the table of register candidates, which
 * include the hard (machine) registers (HR), plus virtual
 * registers (VR) and local variables (LV) */

class RaCell {
  public:
    RaCell();

    unsigned id;
    float frequency;		// weighted occurrence count
    int squeeze;		// sum of weights of interfering candidates
    int color;			// -1 or index in "color" map
    Opnd opnd;			// Corresponding HR, VR, or LV operand
    RaCell *temp;		// Point-life VR: holds this value if spilled
};


// Spill counters
extern int spill_load_count;	// number of loads  added by spillage
extern int spill_store_count;	// number of stores added by spillage


/* Scan code of one procedure, updating operands and adding spill code */

class Raga;

typedef OpndFilter::InOrOut InOrOut;
typedef enum { FWD, BWD } scan_edit_direction_t; // FIXME: use bool
typedef void (Raga::*scan_edit_instr_f)(Instr*);
typedef void (Raga::*scan_edit_memory_f)(VarSym*);
typedef bool (Raga::*scan_edit_screen_f)(Opnd);
typedef Opnd (Raga::*scan_edit_reg_cand_f)(Opnd, bool, InstrHandle);
typedef void (Raga::*scan_edit_block_f)(CfgNode*, scan_edit_direction_t);

extern void scan_edit_code(Raga*, Cfg*, scan_edit_instr_f, scan_edit_instr_f,
			   scan_edit_screen_f, scan_edit_reg_cand_f,
			   scan_edit_reg_cand_f, scan_edit_reg_cand_f,
			   scan_edit_memory_f, scan_edit_direction_t = FWD,
			   scan_edit_block_f = NULL);
extern void scan_edit_opnds(Raga*, InstrHandle, scan_edit_reg_cand_f);
extern Instr* load_or_store(bool do_load, Opnd, VarSym*);
extern bool note_spills;

/*
 * Symmetric, irreflexive matrix class
 *
 * Used for interference-graph adjacency matrices
 *
 */
class AdjacencyMatrix {
  public:
    AdjacencyMatrix()
    {
	bytes = NULL;
	node_count = allocated = 0;
    }
    ~AdjacencyMatrix()
        { delete bytes; }
    void init(size_t node_count);
    void insert(size_t r, size_t c)
    {
	claim((bytes != NULL) && (r < node_count) && (c < node_count));
	size_t i = index(r, c);
        bytes[i >> 3] |= (1 << (i & 7));
    }
    void remove(size_t r, size_t c)
    {
	claim((bytes != NULL) && (r < node_count) && (c < node_count));
	size_t i = index(r, c);
        bytes[i >> 3] &= ~(1 << (i & 7));
    }
    bool operator()(size_t r, size_t c)
    {
	claim((bytes != NULL) && (r < node_count) && (c < node_count));
	size_t i = index(r, c);
        return (bytes[i >> 3] & (1 << (i & 7))) != 0;
    }

  protected:
    unsigned char *bytes;
    size_t node_count;
    size_t allocated;

    size_t index(size_t r, size_t c)
    {
	claim(r != c);
	size_t k = r, l = c;
	if (r < c)
	    { k = c; l = r; }
	return (k * (k - 1))/2 + l;
    }
};

extern int find_maximal_regs(int bank, int conv, NatSet *result = NULL);

extern Vector<Vector<int> >* class_displacements();

extern Vector<Vector<NatSetDense> >* aliases_modulo_class();

#endif /* RAGA_LIBRA_H */
