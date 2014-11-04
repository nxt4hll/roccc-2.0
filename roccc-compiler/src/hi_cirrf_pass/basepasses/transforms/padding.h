// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#ifndef PADDING_H
#define PADDING_H

#include "suifkernel/command_line_parsing.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/suif_env.h"
#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

//	Find multi value blocks and add padding
//	between fields

class PaddingPass : public Pass {
        OptionLiteral *_virs_flag_argument;
	OptionSelection *_virs_selector;
	bool virs;
public:
	PaddingPass(SuifEnv *pEnv, 
		    const LString &name =
		    "insert_struct_padding");
	Module* clone() const { return (Module*)this; }
	void do_file_set_block(FileSetBlock *pFSB);
	bool is_walkable(Address address, bool is_owned, MetaClass *_meta);
	virtual void initialize();
};

/* Add a field at the end of structures when there is space.
 * This is needed to prepare for conversion to C
 */
class StructPaddingPass : public Pass {
public:
  StructPaddingPass(SuifEnv *env, 
		    const LString &name = "insert_struct_final_padding");
  void do_file_set_block(FileSetBlock *fsb);
  Module* clone() const { return (Module*)this; }
};
    
#endif

