// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/*

  This class is soley responsible for verifying that the suif file coming in
   is in a format appropriate for ROCCC 2.0.  If necessary, this class
   also adds in an appropriate annotation as to the specifics of the
   code (if the arrays should be treated as scalars, if we are calling other 
   hardware from this block, etc).

*/

#ifndef VERIFY_PASS_DOT_H
#define VERIFY_PASS_DOT_H

#include <suifkernel/group_walker.h>
#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>
#include <suifkernel/suif_env.h>

class VerifyPass : public PipelinablePass 
{

 private:

  SuifEnv* env ;

  // Once determined, the type is stored here
  int myType ;

  // Returns the type of the function.  Whether the function is
  //  a simple hardware block or a complete system.  Also sets the
  //  internal register
  int DetermineType() ;

  // Top level checking function.  If this function returns, then
  //  the function passed all the checks.
  void VerifyRocccStatus() ;

  // Check that every variable has a size > 0, error out if not
  void VerifySizes() ;
  void VerifySizesWorkhorse(SymbolTable* symTab) ;

  // Check that every shift operation is shifting by a constant, otherwise
  //  we assert out
  void VerifyShifts() ;

  // Make sure that we are not using any operator that can perform short
  //  circuit evaluation, which leads to errors on the lo-cirrf side
  void VerifyShortCircuit() ;

  // Make sure that there is only one write per lookup table
  void VerifyLUTWrites() ;

  // Check that a C loop exists, error out if not
  void VerifyCLoop() ;

  // Check that every array access is a function of a loop index
  void VerifyArrayAccesses() ;

  // Helper variables and functions for verifying all of the array accesses.
  list<VariableSymbol*> allIndicies ;
  void ClearIndicies() ;
  void CollectIndicies() ;
  bool IsIndex(VariableSymbol* v) ;
  
  // Make sure module code has a struct with both inputs and outputs as
  //  the parameter.  If a variable isn't used it won't be declared.
  void VerifyModuleParameters() ;

  // This makes sure that all writes to field accesses are not inputs
  void VerifyModuleDestinations() ;

  // This function makes sure that all reads are not from outputs
  void VerifyModuleSources() ;

  // This function makes sure that modules do not contain any loops
  //  that are not unrolled.
  void VerifyModuleLoop() ;

  // This function makes sure that no two names are used in both
  //  the main function and the struct that we declare.
  void VerifyModuleNames() ;

  // These functions make sure that outputs are only written once
  void VerifyModuleOutputWrites() ;
  void VerifySystemOutputWrites() ;

  void VerifyModuleFeedback() ;

  // This function makes sure that the variables used in the input fifo
  //  are different from the variables used in the output fifo (which
  //  might happen in weird circumstances involving systolic array
  //  generation).
  void VerifyInputOutputUniqueness() ;

  // These functions make sure that system code for loops conform to 
  //  the restrictions of ROCCC.
  bool HasCForLoop() ;
  bool OnlyOneCForLoopNest() ;
  bool ComparisonCorrect() ;
  bool PerfectlyNestedLoops() ;

  // Helper functions
  bool InList(list<LString>& names, LString toCheck) ;
  bool InList(list<VariableSymbol*>& vars, VariableSymbol* v) ;

  ProcedureDefinition* procDef ;
  
 public:
  VerifyPass(SuifEnv *pEnv);
  ~VerifyPass() ;
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
