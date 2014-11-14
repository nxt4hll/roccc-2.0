// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/*
  This program is designed to create the compile script for the
   hi cirrf on the basis of the optimizations and the order 
   specified by the user in the GUI.
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <cassert>
#include <cstdlib>

#include "scriptGenerator.h"

// These match the position in the vector, so make sure they don't change!
const int SystolicArrayGeneration    = 0 ;
const int TemporalCSE                = 1 ;
const int LoopUnrolling              = 2 ;
const int LoopInterchange            = 3 ;
const int LoopFusion                 = 4 ;
const int MultiplyByConstElimination = 5 ;
const int DivisionByConstElimination = 6 ;

// More optimizations
const int FullyUnroll                = 7 ;

// The Redundancy optimization
const int Redundancy                 = 8 ; 

// The inlining optimizations
const int InlineModule               = 9 ;

// For now, keep noticing, but ignore Export
const int Export                     = 10 ;

// When compiling a composed system
const int ComposedSystem             = 11 ;

// Keep inlining everything for a certain depth
const int InlineAllModules           = 12 ;

// Redundancy Label DOUBLE/TRIPLE

ScriptGenerator::ScriptGenerator() 
{
  // Initalize all of the parts that will not change

  requirements  = "require basicnodes suifnodes cfenodes transforms ; " ;
  requirements += "require control_flow_analysis jasonOutputPass ; " ;
  requirements += "require global_transforms utility_transforms ; " ;
  requirements += "require array_transforms loop_transforms ; " ;
  requirements += "require bit_vector_dataflow_analysis verifyRoccc ; " ;
  requirements += "require gcc_preprocessing_transforms ; " ;
  requirements += "require preprocessing_transforms ; " ;
  requirements += "require data_dependence_analysis  ; " ;
  requirements += "require fifoIdentification ; " ;
  requirements += "require libraryOutputPass ; " ;

  // Add optimizations as necessary
  options.push_back("SystolicArrayGeneration") ;
  options.push_back("TemporalCommonSubExpressionElimination") ;
  options.push_back("LoopUnrolling") ;
  options.push_back("LoopInterchange") ;
  options.push_back("LoopFusion") ;
  options.push_back("MultiplyByConstElimination") ;
  options.push_back("DivisionByConstElimination") ;

  // More optimizations
  options.push_back("FullyUnroll") ;
  options.push_back("Redundancy") ;
  options.push_back("InlineModule") ;
  
  // Ignore
  options.push_back("Export") ;

  options.push_back("ComposedSystem") ;

  options.push_back("InlineAllModules") ;
}

ScriptGenerator::~ScriptGenerator()
{
  fin.close() ;
}

void ScriptGenerator::ProcessArgs(int argc, char* argv[])
{
  // The arguments should be:
  //  1) The name of the file we are compiling
  //  2) The name of the optimization file
  //  3) The stream info file
  //  4) The preferences file
  //  5) The debug file
  //  6) The local directory to store the output script file in
  assert(argc == 7 && "Incorrect arguments passed to scriptGenerator!") ;

  InitializeSuifInputFile(argv[1]) ;
  InitializeFileStreams(argv[2]) ;
  streamInfoFile = std::string(argv[3]) ;
  ProcessPreferences(argv[4]) ;
  debugFile = std::string(argv[5]) ;
  outputDirectory = std::string(argv[6]) ;
}

void ScriptGenerator::ProcessPreferences(const char* preferenceFile)
{
  std::ifstream prefIn ;
  prefIn.open(preferenceFile) ;
  if (!prefIn)
  {
    std::cerr << "Could not open the preference file!" << std::endl ;
    return ;
  }
  
  prefIn >> std::ws ;
  while (!prefIn.eof())
  {
    // Each preferece has an integer value as well.
    std::string nextPreference ;
    prefIn >> nextPreference >> std::ws ;

    if (nextPreference == "COMPILER_VERSION")
    {
      prefIn >> compilerVersion >> std::ws ;
    }
    else
    {
      assert(0 && "Incorrect preference file") ;
    }
  }
  
  prefIn.close() ;
}

void ScriptGenerator::ProcessOptimizationFile()
{
  // Initialize all of the statements
  loopUnrollStatements = "" ;
  loopInterchangeStatements = "" ;
  loopFusionStatements = "" ;
  redundancyStatements = "" ;
  multiplyEliminationStatements = "" ;
  divisionEliminationStatements = "" ;
  temporalCSEStatements = "" ;
  inliningStatements = "" ;
  composedStatements = "" ;
  
  //intrinsicStatements = "" ;
  
  fin.clear() ;
  fin.seekg(std::ios_base::beg) ;

  std::string nextOptimization ;

  fin >> std::ws ;

  while(!fin.eof())
  {
    fin >> nextOptimization >> std::ws ;
    int nextIndex = 0 ;
    std::vector<std::string>::iterator optionIter = options.begin() ;
    while (optionIter != options.end())
    {
      if ((*optionIter) == nextOptimization)
      {
	break ;
      }
      ++nextIndex ;
      ++optionIter ;
    }
    if (optionIter == options.end())
    {
      std::cerr << "Unknown optimization detected!" << std::endl ;
      std::cerr << nextOptimization << std::endl ;
      assert(0) ;
    }

    switch (nextIndex)
    {
    case SystolicArrayGeneration:
      {
	// We should have caught this before!
	assert(0) ;
      }
      break ;
    case TemporalCSE:
      {
	temporalCSEStatements += "TemporalCSEPass ; " ;
      }
      break ;
    case LoopUnrolling:
      {
	std::string loopLabel ;
	std::string amount ;
	fin >> loopLabel >> std::ws >> amount >> std::ws ;

	loopUnrollStatements+= "UnrollPass2 " ;
	loopUnrollStatements += loopLabel ;
	
	if (amount == "FULLY")
	{
	  loopUnrollStatements += " -1" ;
	}
	else
	{
	  loopUnrollStatements += " " ;
	  loopUnrollStatements += amount ;
	}
	loopUnrollStatements += " ; " ;	
	loopUnrollStatements += "FlattenStatementListsPass ; " ;
	loopUnrollStatements += "NormalizeStatementListsPass ; " ;

	loopFusionStatements += "FlattenStatementListsPass ; " ;
	loopFusionStatements += "FusePass ; " ;
      }
      break ;
    case LoopInterchange:
      {
	std::string loopLabel1 ;
	std::string loopLabel2 ;
	fin >> loopLabel1 >> std::ws >> loopLabel2 >> std::ws ;

	loopInterchangeStatements += "InterchangePass " ;
	loopInterchangeStatements += loopLabel1 ; 
	loopInterchangeStatements += " " ;
	loopInterchangeStatements += loopLabel2 ;
	loopInterchangeStatements += " ; " ; 
      }
      break ;
    case LoopFusion:
      {
	loopFusionStatements += "FlattenStatementListsPass ; " ;
	loopFusionStatements += "FusePass ; " ;
      }
      break ;
    case MultiplyByConstElimination:
      {
	multiplyEliminationStatements += "MultiplyByConstEliminationPass ; " ;
      }
      break ;
    case DivisionByConstElimination:
      {
	divisionEliminationStatements += "DivByConstEliminationPass2 ; " ;
      }
      break ;
    case FullyUnroll:
      {
	loopUnrollStatements += "FullUnrollPass ; " ;

	loopFusionStatements += "FlattenStatementListsPass ; " ;
	loopFusionStatements += "FusePass ; " ;
      }
      break ;
    case Redundancy:
      {
	std::string label ;
	std::string doubleOrTriple ;
	fin >> label >> std::ws >> doubleOrTriple >> std::ws ;

	redundancyStatements += "MarkRedundantPass " ;
	redundancyStatements += label ;
	redundancyStatements += " " ;
	if (doubleOrTriple == "DOUBLE")
	{
	  redundancyStatements += "0 ; " ;
	}
	else
	{
	  redundancyStatements += "1 ; " ;
	}
      }
      break ;
    case InlineModule:
      {
	std::string functionName ;
	fin >> std::ws >> functionName >> std::ws ;
	inliningStatements += "InliningPass " ;
	inliningStatements += functionName ;
	inliningStatements += " " ;
	inliningStatements += outputDirectory ;
	inliningStatements += "/tmp/.preprocessInfo" ;
	inliningStatements += " ; " ;
      }
      break ;
    case Export:
      {
	; // Ignore
      }
      break ;
    case ComposedSystem:
      {
	composedStatements += "MarkSystemToSystemPass ; " ;
      }
      break ;
    case InlineAllModules:
      {
	std::string depth ;
	fin >> std::ws >> depth >> std::ws ;
	inliningStatements += "InlineAllModulesPass " ;
	inliningStatements += depth ;
	inliningStatements += " " ;
	inliningStatements += outputDirectory ;
	inliningStatements += "/tmp/.preprocessInfo" ;
	inliningStatements += " ; " ;
      }
      break ;      
    default:
      {
	std::cerr << "Unknown error!" << std::endl ;
	assert(0) ;
      }
    }    
  }
}

void ScriptGenerator::CreateSystolicArrayCommand()
{
  suifdriverCommand  = "suifdriver -e \"" ;
  suifdriverCommand += requirements ;
  suifdriverCommand += "load " + compileFile ;
  suifdriverCommand += ".suif ; " ;
  
  // All of the systolic array commands
  suifdriverCommand += "EvalTransformPass ;" ;
  suifdriverCommand += "ForLoopPreprocessingPass ;" ;
  suifdriverCommand += "FlattenStatementListsPass ;" ;
  suifdriverCommand += "NormalizeStatementListsPass ;" ;

  suifdriverCommand += "PointerConversionPass ; " ;
  suifdriverCommand += "CleanupStoreStatementsPass ; " ;

  suifdriverCommand += "IdentificationPass ; " ;
  suifdriverCommand += "VerifySystolicPass ; " ;

  suifdriverCommand += "UnrollPass2 " ;
  suifdriverCommand += systolicArrayLabel ;  
  suifdriverCommand += " -1 ; " ;
  // No longer necessary...
  //  suifdriverCommand += "RemoveLoopLabelLocStmtsPass ; " ;
  suifdriverCommand += "FlattenStatementListsPass ; " ;
  suifdriverCommand += "FusePass ; " ;
  suifdriverCommand += "ControlFlowSolvePass ; " ;
  suifdriverCommand += "DataFlowSolvePass2 ; " ;
  suifdriverCommand += "UD_DU_ChainBuilderPass2 ; " ;
  suifdriverCommand += "ConstantPropagationAndFoldingPass2 ; " ;
  //suifdriverCommand += "ConstantQualedArrayPropagationPass ; " ;
  suifdriverCommand += "ConstantArrayPropagationPass ; " ;
  suifdriverCommand += "ConstantPropagationAndFoldingPass2 ; " ;

  suifdriverCommand += "ControlFlowSolvePass ;" ;
  suifdriverCommand += "DataFlowSolvePass2 ;" ;
  suifdriverCommand += "UD_DU_ChainBuilderPass2 ;" ;
  suifdriverCommand += "FlattenStatementListsPass ;" ;

  //  suifdriverCommand += "PseudoSSA ; " ;
  suifdriverCommand += "HandleCopyStatements ; " ;
  suifdriverCommand += "HandleCallStatements ;" ;
  
  suifdriverCommand += "LoopInfoPass ; " ;
  suifdriverCommand += "PreprocessingPass ; " ;
  suifdriverCommand += "MEM_DP_BoundaryMarkPass ; " ;

  suifdriverCommand += "RawEliminationPass ; " ;
  suifdriverCommand += "ScalarReplacementPass ; " ;

  suifdriverCommand += "ControlFlowSolvePass ; " ;
  suifdriverCommand += "DataFlowSolvePass2 ; " ;
  suifdriverCommand += "UD_DU_ChainBuilderPass2 ; " ;
  suifdriverCommand += "CopyPropagationPass2 ; " ;

  suifdriverCommand += "FlattenStatementListsPass ; " ;
  suifdriverCommand += "IfConversionPass2 ; " ;
  suifdriverCommand += "FeedbackLoadEliminationPass ; " ;
  suifdriverCommand += "SolveFeedbackVariablesPass2 ; " ;
  
  suifdriverCommand += "ControlFlowSolvePass ; " ;
  suifdriverCommand += "DataFlowSolvePass2 ; " ;
  suifdriverCommand += "UD_DU_ChainBuilderPass2 ; " ;
  suifdriverCommand += "FifoIdentificationSystolicArray ; " ;

  suifdriverCommand += "RemoveUnusedVariables ;" ;

  suifdriverCommand += "RemoveNonPrintablePass ; " ;
  suifdriverCommand += "FifoIdentification ; " ;
  suifdriverCommand += "LeftoverRemovalPass ; " ;
  
  suifdriverCommand += "IdentificationPass ;" ;
  suifdriverCommand += "VerifyPass ; " ;
  suifdriverCommand += "OutputPass " ;
  suifdriverCommand += streamInfoFile ;
  suifdriverCommand += " ; " ;
  suifdriverCommand += "StripAnnotesPass ; " ;
  suifdriverCommand += "\"\n" ;
}

void ScriptGenerator::Execute(int argc, char* argv[])
{
  ProcessArgs(argc, argv) ;
  CreateSuifDriverCommand() ;
  OutputScriptFile() ;
}

// This function is responsible for taking the full cFilename
//  and stripping off the directory prefix and the .c suffix
void ScriptGenerator::InitializeSuifInputFile(const char* cFileName)
{
  assert(cFileName != NULL) ;
  std::string strippedName = cFileName ;

  // Remove the directories
  int position = strippedName.find_last_of("/") ;
  assert(position >= 0) ;
  strippedName = strippedName.substr(position + 1) ;

  // Remove the .c
  position = strippedName.find_last_of(".") ;
  assert(position >= 0) ;
  
  compileFile = strippedName.substr(0, position) ;
}

void ScriptGenerator::InitializeFileStreams(const char* optimizationFile)
{
  fin.open(optimizationFile) ;
  assert(!(!fin)) ;

  // A check to see if somehow we have opened an empty file.
  if (fin.eof())
  {
    assert(0) ;
  }
}

void ScriptGenerator::OutputScriptFile()
{
  std::string pathToOutputScript = outputDirectory ;
  assert(pathToOutputScript != "") ;
  pathToOutputScript += "/tmp/compile_suif2hicirrf.sh" ;

  std::ofstream fout(pathToOutputScript.c_str()) ;
  
  if (!fout)
  {
    assert(0) ;
  }

  fout << "#! /bin/bash" << std::endl ;
  fout << std::endl ;
  
  fout << "gcc2suif " << outputDirectory << " " << compileFile << ".c "
       << std::endl ;
  fout << std::endl ;

  fout << suifdriverCommand ;
  fout << std::endl ;
}

void ScriptGenerator::CreateSuifDriverCommand()
{
  if (hasString("SystolicArrayGeneration"))
  {
    CreateSystolicArrayCommand() ;
    return ;
  }

  ProcessOptimizationFile() ;

  CreateStandardSuifDriverCommand() ;
}

void ScriptGenerator::Normalize()
{
  // Change Call Expressions into Call Statements
  suifdriverCommand += "EvalTransformPass ; " ;

  // Remove empty StatementLists
  suifdriverCommand += "FlattenStatementListsPass ; " ;

  // Make sure all ifs and loops contain statement lists
  suifdriverCommand += "NormalizeStatementListsPass ; " ;

  // Convert stores to load variable expressions into store variable statements
  suifdriverCommand += "CleanupStoreStatementsPass ; " ;

  // Convert loads of symbol address expressions into load variable expressions
  suifdriverCommand += "CleanupLoadPass ; " ;
}

void ScriptGenerator::ConstantPropagation()
{
  suifdriverCommand += "ControlFlowSolvePass ; " ;
  suifdriverCommand += "DataFlowSolvePass2 ; " ;
  suifdriverCommand += "UD_DU_ChainBuilderPass2 ; " ;
  suifdriverCommand += "ConstantPropagationAndFoldingPass2 ; " ;
  suifdriverCommand += "ConstantArrayPropagationPass ; " ;
  suifdriverCommand += "ConstantPropagationAndFoldingPass2 ; " ;
  suifdriverCommand += "StripAnnotesPass ; " ;
}

void ScriptGenerator::CreateStandardSuifDriverCommand()
{
  suifdriverCommand  = "suifdriver -e \"" ;
  suifdriverCommand += requirements ;
  suifdriverCommand += "load " + compileFile ;
  suifdriverCommand += ".suif ; " ;

  // -------------------- Section 0 ----------------------
  //  Identify system to system code
  //suifdriverCommand += composedStatements ;
  suifdriverCommand += "MarkSystemToSystemPass ; " ;

  // -------------------- Section 1 ----------------------
  //    High Level Transformations (non hardware based)

  // Subsection 1.0 -> Handling initialization of variables
  suifdriverCommand += "InitializationPass ; " ;  

  // Subsection 1.1 -> Preprocess and preverify
  Normalize() ;
  suifdriverCommand += "ForLoopPreprocessingPass ; " ;
  suifdriverCommand += "PreVerifyPass ; " ;

  // Subsection 1.2 -> Inline
  suifdriverCommand += inliningStatements ; 
  Normalize() ;

  // Subsection 1.3 -> Fix module instantiations to output parameters
  suifdriverCommand += "IdentifyParametersPass ; " ;
  suifdriverCommand += "ReferenceCleanupPass ; " ;

  // Subsection 1.4 Pointer conversion into arrays
  suifdriverCommand += "PointerConversionPass ; " ;
  Normalize() ;

  // Subsection 1.5 -> Redundancy
  suifdriverCommand += redundancyStatements ;
  suifdriverCommand += "RedundancyPass ;" ;   
  Normalize() ;
  suifdriverCommand += "ControlFlowSolvePass ; " ;
  suifdriverCommand += "DataFlowSolvePass2 ; " ;
  suifdriverCommand += "UD_DU_ChainBuilderPass2 ; " ;
  suifdriverCommand += "RedundantToRedundantPass ; " ;
  suifdriverCommand += "StripAnnotesPass ; " ;
  Normalize() ;

  // Subsection 1.6 -> Loop interchange
  suifdriverCommand += loopInterchangeStatements ;

  // Subsection 1.7 -> Loop unrolling  
  suifdriverCommand += loopUnrollStatements ;
  suifdriverCommand += loopFusionStatements ;
  Normalize() ;
  ConstantPropagation() ;
  Normalize() ;

  // Subsection 1.8 -> Cleanup after loop unrolling
  suifdriverCommand += "CleanupRedundantVotes ; " ;
  suifdriverCommand += "CleanupUnrolledCalls ; " ;
  suifdriverCommand += "TransformUnrolledArraysPass ; " ;
  suifdriverCommand += "RemoveLoopLabelLocStmtsPass ; " ;
  Normalize() ;

  // Subsection 1.9 -> Cleanup all unnecessary statements
  suifdriverCommand += "RemoveUnsupportedStatements ; " ;
  Normalize() ;

  // Subsection 1.10 -> Determine what variables are lookup tables
  suifdriverCommand += "LookupTableIdentification ; " ;
  Normalize() ;

  // -------------------- Section 2 ----------------------
  //      High Level Transformations (hardware based)

  // Subsection 2.1 -> Predication and if conversion
  suifdriverCommand += "MiniScalarReplacementPass ; " ;
  Normalize() ;
  suifdriverCommand += "IfConversionPass2 ; " ;
  suifdriverCommand += "PredicationPass ; " ;
  suifdriverCommand += "FlattenStatementListsPass ; " ;
  suifdriverCommand += "IfConversionPass2 ; " ;  
  Normalize() ;
  ConstantPropagation() ;
  Normalize() ;

  // Subsection 2.2 -> Mult/Div constant elimination
  suifdriverCommand += multiplyEliminationStatements ;
  suifdriverCommand += divisionEliminationStatements ;

  // Subsection 2.3 -> Available code elimination
  suifdriverCommand += "AvailableCodeEliminationPass ; " ;
  Normalize() ;

  // Subsection 2.4 -> Summation pass
  suifdriverCommand += "CombineSummationPass ; " ;
  Normalize() ;
  suifdriverCommand += "SummationPass ; " ;
  Normalize() ;

  // Subsection 2.5 -> Temporal CSE
  suifdriverCommand += temporalCSEStatements ;
  Normalize() ;

  // Subsection 2.6 -> SSA
  suifdriverCommand += "PseudoSSA ; " ;
  Normalize() ;
  ConstantPropagation() ;
  Normalize() ;

  // Subsection 2.7 -> Copy Propagation
  suifdriverCommand += "ControlFlowSolvePass ; " ;
  suifdriverCommand += "DataFlowSolvePass2 ; " ;
  suifdriverCommand += "UD_DU_ChainBuilderPass2 ; " ;
  suifdriverCommand += "CopyPropagationPass2 ; " ;
  suifdriverCommand += "StripAnnotesPass ; " ;

  // -------------------- Section 3 ----------------------
  //         Analysis and Information Collection

  // Subsection 3.0 -> Lookup table Transformation
  suifdriverCommand += "LookupTableTransformation ; " ;
  Normalize() ;

  // Subsection 3.1 -> Loop information
  suifdriverCommand += "ControlFlowSolvePass ; " ;
  suifdriverCommand += "DataFlowSolvePass2 ; " ;
  suifdriverCommand += "UD_DU_ChainBuilderPass2 ; " ;
  suifdriverCommand += "LoopInfoPass ; " ;
  suifdriverCommand += "PreprocessingPass ; " ;

  // Subsection 3.2 -> Scalar replacement
  suifdriverCommand += "ScalarReplacementPass2 ; " ;
  Normalize() ;
  ConstantPropagation() ;
  Normalize() ;

  // Subsection 3.3 -> Output identification
  suifdriverCommand += "OutputIdentificationPass ; " ;
  suifdriverCommand += "FifoIdentification ; " ;

  // Subsection 3.4 -> Creating a module out of a system if necessary
  suifdriverCommand += "TransformSystemsToModules ; " ;

  // Subsection 3.5 -> Identification of module, system, or composed system
  suifdriverCommand += "IdentificationPass ;" ;

  // Subsection 3.6 -> Final cleanup
  suifdriverCommand += "RemoveUnusedVariables ;" ;
  suifdriverCommand += "CleanupStoreStatementsPass ; " ;
  suifdriverCommand += "CleanupBoolSelsPass ;" ;

  // -------------------- Section 4 ----------------------
  //            Output and Library Maintaince
  
  // Subsection 4.1 -> Handle preferences from the GUI
  suifdriverCommand += "PreferencePass " ;
  suifdriverCommand += compilerVersion ;
  suifdriverCommand += " ; " ;

  // Subsection 4.2 -> Make casting explicit
  suifdriverCommand += "CastPass ; " ;

  // Subsection 4.3 -> Label debug variables listed in the GUI
  suifdriverCommand += "IdentifyDebugPass " ;
  suifdriverCommand += debugFile ;
  suifdriverCommand += " ;" ; 

  // Subsection 4.4 -> Verify that everything went well
  suifdriverCommand += "VerifyPass ; " ;

  // Subsection 4.5 -> Output the hi-cirrf file
  suifdriverCommand += "OutputPass " ;
  suifdriverCommand += streamInfoFile ;
  suifdriverCommand += " ; " ;
  suifdriverCommand += "StripAnnotesPass ; " ;
  
  // Subsection 4.6 -> Maintain the repository of modules
  suifdriverCommand += "ExportPass " ;
  suifdriverCommand += outputDirectory ;
  suifdriverCommand += "/LocalFiles/ ; " ;
  suifdriverCommand += "CleanRepositoryPass " ;
  suifdriverCommand += outputDirectory ; 
  suifdriverCommand += "/LocalFiles/ ; " ;
  suifdriverCommand += "DumpHeaderPass " ;
  suifdriverCommand += outputDirectory ;
  suifdriverCommand += "/LocalFiles/ ; " ;

  // Subsection 4.7 -> Finalize the command
  suifdriverCommand += "\"" ;
}

bool ScriptGenerator::hasString(std::string toMatch)
{
  fin.clear() ;
  fin.seekg(std::ios_base::beg) ;

  std::string nextItem ;

  fin >> std::ws ;

  while (!fin.eof())
  {
    fin >> nextItem >> std::ws ;
    if (nextItem == toMatch)
    {
      // One quick check that really doesn't belong here...
      if (toMatch == "SystolicArrayGeneration")
      {
	fin >> systolicArrayLabel ;
      }
      return true ;
    }
  }
  
  return false ;
}
