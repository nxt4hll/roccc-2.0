
#ifndef __CREATE_HI_CIRRF_SCRIPT_DOT_H__
#define __CREATE_HI_CIRRF_SCRIPT_DOT_H__

#include <string>
#include <vector>
#include <fstream>
#include <list>

class ScriptGenerator
{
 private:
  std::string requirements ;
  std::vector<std::string> options ;

  std::string compilerVersion ;

  std::ifstream fin;

  std::string streamInfoFile ;

  std::string compileFile ;

  std::string loopUnrollStatements ;
  std::string loopInterchangeStatements ;
  std::string loopFusionStatements ;
  std::string redundancyStatements ;
  std::string multiplyEliminationStatements ;
  std::string divisionEliminationStatements ;
  std::string temporalCSEStatements ;
  std::string inliningStatements ;
  
  std::string composedStatements ;

  std::string systolicArrayLabel ;
  
  std::string suifdriverCommand ;

  void ProcessArgs(int argc, char* argv[]) ;
  void ProcessOptimizationFile() ;
  void OutputScriptFile() ;

  void InitializeFileStreams(const char* optimizationFile) ;
  void InitializeSuifInputFile(const char* cFileName) ;

  void CreateSystolicArrayCommand() ;

  bool hasString(std::string toMatch) ;

  void CreateSuifDriverCommand() ;
  void CreateStandardSuifDriverCommand() ;

  void Normalize() ;
  void ConstantPropagation() ;

  void ProcessPreferences(const char* preferenceFile) ;

  std::string debugFile ;
  std::string outputDirectory ;

 public:

  void Execute(int argc, char* argv[]) ;

  ScriptGenerator() ;
  ~ScriptGenerator() ;
} ;

#endif 
