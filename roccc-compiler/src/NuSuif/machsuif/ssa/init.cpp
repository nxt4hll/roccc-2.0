/* file "ssa/init.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "ssa/init.h"
#endif

#include <iokernel/object_factory.h>
#include <iokernel/pointer_meta_class.h>
#include <iokernel/aggregate_meta_class.h>

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <cfa/cfa.h>
#include <bvd/bvd.h>

#include <ssa/ssa_ir.h>
#include <ssa/ssa_core.h>
#include <ssa/init.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

IdString k_phi_nodes;			// key for attaching phi-nodes to blocks

enum { IS_SSA      = true };
enum { IS_DEFLATED = true };

class SsaCfgObjectAggregateMetaClass : public ObjectAggregateMetaClass {
  public:
    SsaCfgObjectAggregateMetaClass(LString metaClassName = LString())
	: ObjectAggregateMetaClass(metaClassName) { }

    virtual void read(const ObjectWrapper &obj,
		      InputStream* inputStream) const
    {
	//std::cerr << "Starting to read  " << get_instance_name().c_str() << std::endl;
	ObjectAggregateMetaClass::read(obj, inputStream);

	SsaCfg *ssa = to<SsaCfg>(static_cast<Object*>(obj.get_address()));
	ssa->core.set_underlying_ptr(new SsaCore(ssa, IS_SSA, IS_DEFLATED));
    }    

    virtual void write(const ObjectWrapper &obj,
		       OutputStream* outputStream) const
    {
	//std::cerr << "Starting to write " << get_instance_name().c_str() << std::endl;
	SsaCfg  *ssa = to<SsaCfg>(static_cast<Object*>(obj.get_address()));
	SsaCore *core = ssa->core.get_underlying_ptr();
	if (core != NULL)
	    core->deflate();
	ObjectAggregateMetaClass::write(obj, outputStream);
    }
};

void
picklify_ssa_cfg_metaclass(SuifEnv *suif_env)
{
    ObjectFactory *mcof = suif_env->get_object_factory();

    AggregateMetaClass *meta_AnyBody
	= mcof->find_aggregate_meta_class(AnyBody::get_class_name());
    AggregateMetaClass *meta_SsaCfg
	= mcof->find_aggregate_meta_class(SsaCfg::get_class_name());
    AggregateMetaClass *new_meta_SsaCfg =
	new SsaCfgObjectAggregateMetaClass(SsaCfg::get_class_name());

    AggregateMetaClass *ssa_cfg_meta_class_mc =
	to<AggregateMetaClass>(meta_SsaCfg->Object::get_meta_class());
    kernel_assert(ssa_cfg_meta_class_mc);
    new_meta_SsaCfg->set_meta_class(ssa_cfg_meta_class_mc);
    new_meta_SsaCfg->set_constructor_function(SsaCfg::constructor_function);
    new_meta_SsaCfg->set_size(sizeof(SsaCfg));
    new_meta_SsaCfg->inherits_from( meta_AnyBody );
    ssa_cfg_meta_class_mc->
	set_alignment(meta_AnyBody->get_alignment_of_instance());
    mcof->enter_meta_class(new_meta_SsaCfg);

    delete meta_SsaCfg;

    AggregateMetaClass *meta_Cfg =
	mcof->find_aggregate_meta_class(Cfg::get_class_name());
    PointerMetaClass *meta_Cfg_owner =
	mcof->get_pointer_meta_class(meta_Cfg,true,false,true);

    AggregateMetaClass *meta_ProcedureDefinition =
	mcof->find_aggregate_meta_class(ProcedureDefinition::get_class_name());
    PointerMetaClass *meta_ProcedureDefinition_ref =
	mcof->get_pointer_meta_class
	    (meta_ProcedureDefinition,false,false,false);

    AggregateMetaClass *meta_IrOpnd =
	mcof->find_aggregate_meta_class(IrOpnd::get_class_name());
    PointerMetaClass *meta_IrOpnd_ref =
	mcof->get_pointer_meta_class(meta_IrOpnd,false,false,false);
    STLMetaClass *meta_IrOpnd_ref_vector =
	mcof->get_stl_meta_class
	    (String("LIST:vector<") +
	     meta_IrOpnd_ref->get_instance_name() +
	     String(">"),
	     new STLDescriptor<suif_vector<IrOpnd*> >(meta_IrOpnd_ref));

    MetaClass *meta_int = mcof->find_meta_class("int");
    STLMetaClass *meta_int_vector =
	mcof->get_stl_meta_class
	    (String("LIST:vector<") +
	     meta_int->get_instance_name() + String(">"),
	     new STLDescriptor<suif_vector<int> >(meta_int));

    MetaClass* meta_bool = mcof->find_meta_class("bool");

    new_meta_SsaCfg->add_field_description
	("_cfg", meta_Cfg_owner,
	 OFFSETOF(SsaCfg,_cfg));
    new_meta_SsaCfg->add_field_description
	("_unit",meta_ProcedureDefinition_ref,
	 OFFSETOF(SsaCfg,_unit));
    new_meta_SsaCfg->add_field_description
	("_old_names", meta_IrOpnd_ref_vector,
	 OFFSETOF(SsaCfg,_old_names));
    new_meta_SsaCfg->add_field_description
	("_new_names", meta_IrOpnd_ref_vector,
	 OFFSETOF(SsaCfg,_new_names));
    new_meta_SsaCfg->add_field_description
	("_new_to_old", meta_int_vector,
	 OFFSETOF(SsaCfg,_new_to_old));
    new_meta_SsaCfg->add_field_description
	("_formal_value_ids", meta_int_vector,
	 OFFSETOF(SsaCfg,_formal_value_ids));
    new_meta_SsaCfg->add_field_description
	("_build_minimal_form", meta_bool,
	 OFFSETOF(SsaCfg,_build_minimal_form));
    new_meta_SsaCfg->add_field_description
	("_build_semi_pruned_form", meta_bool,
	 OFFSETOF(SsaCfg,_build_semi_pruned_form));
    new_meta_SsaCfg->add_field_description
	("_build_pruned_form", meta_bool,
	 OFFSETOF(SsaCfg,_build_pruned_form));
    new_meta_SsaCfg->add_field_description
	("_build_def_use_chains", meta_bool,
	 OFFSETOF(SsaCfg,_build_def_use_chains));
    new_meta_SsaCfg->add_field_description
	("_fold_copies", meta_bool,
	 OFFSETOF(SsaCfg,_fold_copies));
    new_meta_SsaCfg->add_field_description
	("_omit_useless_phi_nodes", meta_bool,
	 OFFSETOF(SsaCfg,_omit_useless_phi_nodes));
    new_meta_SsaCfg->add_field_description
	("_report_undefined_locs", meta_bool,
	 OFFSETOF(SsaCfg,_report_undefined_locs));
    new_meta_SsaCfg->add_field_description
	("_keep_live_in_info", meta_bool,
	 OFFSETOF(SsaCfg,_keep_live_in_info));
    new_meta_SsaCfg->add_field_description
	("_keep_live_out_info", meta_bool,
	 OFFSETOF(SsaCfg,_keep_live_out_info));
}

extern "C" void
init_ssa(SuifEnv *suif_env)
{
    static bool init_done = false;

    if (init_done)
        return;
    init_done = true;

    init_machine(suif_env);
    init_cfg(suif_env);
    init_cfa(suif_env);
    init_bvd(suif_env);
    init_ssa_irnodes(suif_env);

    k_phi_nodes = "phi_nodes";

    Operation::one_null_instr.push_front(NULL);
    Operation::one_null_phi  .push_front(NULL);

    picklify_ssa_cfg_metaclass(suif_env);
}
