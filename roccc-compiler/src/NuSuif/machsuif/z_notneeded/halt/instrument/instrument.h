/* file "instrument/instrument.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef INSTRUMENT_INSTRUMENT_H
#define INSTRUMENT_INSTRUMENT_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "instrument/instrument.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <bvd/bvd.h>

/*
 * This pass instruments an application using the HALT (Harvard
 * Atom-Like Tool) labels inserted by an earlier pass.
 */

class InstrumentPass {
  public:
    InstrumentPass() { };

    void initialize();
    void do_opt_unit(OptUnit*);
    void finalize() { }

    void set_main_name(const char* name) { main_name = name; }

  protected:
    void warm_catalog(OpndCatalog*);

    const char *main_name;

    // Per-unit variables
    Cfg			*cfg;
    RegSymCatalog	*opnd_catalog;
    DefUseAnalyzer	*du_analyzer;
    Liveness		*liveness;
};

#endif /* INSTRUMENT_INSTRUMENT_H */
