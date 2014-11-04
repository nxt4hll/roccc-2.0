
#ifndef SYSTEM_GENERATOR_DOT_H
#define SYSTEM_GENERATOR_DOT_H

#include <string>
#include <map>
#include <list>

#include "baseOutput.h"

class SystemGenerator : public HiCirrfGenerator
{
 protected:
  std::list<VariableSymbol*> inputArrays ;
  std::list<VariableSymbol*> outputArrays ;
  std::list<VariableSymbol*> feedbackScalars ;

 private:
  std::map<std::string, int> streamAddressChannels ;
  std::map<std::string, int> streamDataChannels ;
  String streamFileName ;

  std::list< std::pair<VariableSymbol*, int> > stepSizes ;

  void CollectSteps() ;

  int fifoCounter ;

 protected:

  virtual void CollectInputs() ;
  virtual void CollectOutputs() ;
  virtual void CollectFeedbacks() ;

 public:

  SystemGenerator(SuifEnv* e, ProcedureDefinition* p, String s) ;
  ~SystemGenerator() ;

  virtual void Setup() ;
  virtual LString CleanOutputName(LString n) ;
  virtual void Output() ;
  virtual void OutputDotH() ;
  virtual void OutputFunctionType() ;
  virtual void OutputFakes() ;
  virtual void CollectVariables() ;

  virtual void PrintForStatement(CForStatement* s) ;

  void InitializeChannels() ;
  void OutputNumChannels() ;
  void OutputNumChannelsWorkhorse(VariableSymbol* v, bool isInput) ;
  void OutputSteps() ;

  void OutputInputArrays() ;
  void OutputOutputArrays() ;
  
  LString StripSuffix(LString& original, const char* suffix) ;

  void OutputSystolicInitialization() ;

  void PrintLoadPrevious() ;
  void PrintStoreToNext() ;

  void PrintTemporalLoads() ; // Load previous
  void PrintTemporalStores() ; // Store next

  void PrintInputLoopFifos() ; 
  void PrintOutputLoopFifos() ;

  void PrintInputLoopFifo(VariableSymbol* v) ;
  void PrintOutputLoopFifo(VariableSymbol* v) ;
 
} ;

#endif
