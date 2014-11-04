// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef ARRAY_DISMANTLERS_H
#define ARRAY_DISMANTLERS_H
/**
    \file Contains passes that operate on arrays.
*/
#include "suifkernel/module_subsystem.h"
#include "suifkernel/suif_env.h"
#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "procedure_walker_utilities.h"

class ArrayReferenceDismantlerPass : public PipelinablePass {
  SuifEnv *_env;
public:
  ArrayReferenceDismantlerPass(SuifEnv *the_env,  const LString &name =
                               "dismantle_array_reference_expressions");
  Module *clone() const;
  void initialize();
  void do_procedure_definition(ProcedureDefinition *pd);
};

class MultiDimArrayDismantlerPass : public Pass {
public:
    Module* clone() const;
    void initialize();

    void do_file_set_block( FileSetBlock* file_set_block );

    MultiDimArrayDismantlerPass(SuifEnv *env,
				const LString &name =
				"dismantle_multi_dim_arrays");
};

class NonConstBoundDismantlerPass : public Pass {
public:
    Module* clone() const;
    void initialize();
    void do_file_set_block( FileSetBlock* file_set_block );

    NonConstBoundDismantlerPass(SuifEnv *env,
				const LString &name =
				"dismantle_non_const_bound_arrays");
};

/**
    This class provides some utility functions that help with 
    conversion of single-dimantional to multi-dimentional array 
    types.
*/
class OneDimArrayConverter {
public:
    OneDimArrayConverter(SuifEnv* the_suif_env);

    ~OneDimArrayConverter();

    /**
    Convert an array type to a multidimentional array type. 
    
    The value is first looked up in a cache to avoid creating
    multiple multidim. array types representing the same thing.

    All the types we encounter here get recorded and you can remove
    them from the IR completely by using remove_all_one_dim_array_types.
    \see remove_all_one_dim_array_types
    */
    MultiDimArrayType* array_type2multi_array_type(ArrayType* at);

    /**
        Convert ArrayReferenceExpression \a top_array to a 
        MultiDimArrayExpression.
    */
    void convert_array_expr2multi_array_expr(ArrayReferenceExpression* top_array);

    void remove_all_one_dim_array_types();
   
protected:
    suif_map<ArrayType*, MultiDimArrayType*>* type_map;
    suif_vector<ArrayType*>* all_array_types;

    SuifEnv* suif_env;
    TypeBuilder* tb;
private:
    OneDimArrayConverter &operator=(const OneDimArrayConverter &);
    OneDimArrayConverter(const OneDimArrayConverter &);
};

/**
    A pass to convert ArrayReferenceExpression to
    MultiDimArrayExpressions.
    
    If command-line option -preserve1dim is specified, then 
    single ArrayReferenceExpressions are preserved and not
    converted to corresponding multidimentional ones.
*/
class One2MultiArrayExpressionPass : public PipelinablePass {
public:
    One2MultiArrayExpressionPass(SuifEnv *the_env) :
      PipelinablePass(the_env, "build_multi_dim_arrays"),
      converter(new OneDimArrayConverter(the_env)){};

    ~One2MultiArrayExpressionPass(){delete converter;}

    void initialize();

    Module *clone() const { return (Module *)this;}

    void do_procedure_definition(ProcedureDefinition* proc_def);

protected:
    OptionLiteral *_preserve_one_dim;
    OneDimArrayConverter* converter;
};

#if 0
/**
    This pass eliminates converts that appear when passing an array
    into a procedure by value, i.e. just as a pointer. The front end
    just passes a pointer to the array and whenever this pointer appears
    inside of an ArrayReferenceExpression or a MultiDimArrayReferenceExpression,
    there must be a convert present to convert the poiter type to an array
    type.
*/
class EliminateArrayConvertsPass : public PipelinablePass {
public:
    EliminateArrayConvertsPass(SuifEnv *the_env) :
      PipelinablePass(the_env, "eliminate_array_converts"){};

    Module *clone() const {return (Module *)this;}

    void do_procedure_definition(ProcedureDefinition *proc_def);
};
#endif

#endif /* ARRAY_DISMANTLERS_H */
