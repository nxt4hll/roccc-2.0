/* file "summarize/summarize.h" */

/*
    Copyright (c) 1995-99 The President and Fellows of Harvard University

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef SUMMARIZE_SUMMARIZE_H
#define SUMMARIZE_SUMMARIZE_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "summarize/summarize.h"
#endif

#include <machine/machine.h>

/*
 * This pass summarizes the instrumentation points in a machsuif
 * intermediate file.  For the time being, it only recognizes the following
 * kinds of instrumentation: ENTRY, EXIT, CBR, MBR.
 */

class Summarize {
  public:
    Summarize();
    ~Summarize() { }
    
    void initialize();
    void do_opt_unit(OptUnit*);
//  void finalize() { }

    FILE* get_output_file() const	{ return out; }
    void  set_output_file(FILE *out)	{ this->out = out; }

  protected:
    FILE *out;			// output file
    int max_event_id;		// largest event id plus 1
};

#endif /* SUMMARIZE_SUMMARIZE_H */
