// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

This pass is responsible.  

*/

#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/MessageLogger.h"
#include "rocccLibrary/FileInfo.h"
#include "rocccLibrary/DatabaseHelpers.h"
#include "rocccLibrary/FunctionType.h"

#include <assert.h>
#include <fstream>

namespace llvm
{
  class WriteLogMessagePass : public FunctionPass
  {
  private:
  public:
    static char ID ;
    WriteLogMessagePass() ;
    ~WriteLogMessagePass() ;
    virtual bool runOnFunction(Function& b) ;
  } ;
}

using namespace llvm ;

char WriteLogMessagePass::ID = 0 ;

static RegisterPass<WriteLogMessagePass> X ("writeLog", 
					"Writes the log.");

WriteLogMessagePass::WriteLogMessagePass() : FunctionPass((intptr_t)&ID) 
{
  ; // Nothing in here
}

WriteLogMessagePass::~WriteLogMessagePass()
{
  ; // Nothing to delete either
}

bool WriteLogMessagePass::runOnFunction(Function& f)
{
  CurrentFile::set(__FILE__);
  bool changed = false ;
  if ( f.isDeclaration() || (f.getDFFunction() == NULL and ROCCC::ROCCCFunctionType(&f) != ROCCC::SYSTEM) )
  {
    return changed ;
  }

  std::string outputFileName = f.getName() + ".html";
  char buff[1024];
  assert( getcwd(buff, 1024) and "Path too long!" ); //getcwd returns 0 if the path is too long
  llvm::cout << "Writing report to \'" << buff << "/" << outputFileName << "\'\n";
  Database::FileInfoInterface::addFileInfo(Database::getCurrentID(), Database::FileInfo(outputFileName, Database::FileInfo::REPORT, std::string(buff)+"/"));
  std::ofstream f1(outputFileName.c_str());
  LOG_MESSAGE0("Compilation report for " << f.getName() << ".\n");
  f1 << ROCCC::MessageLogger::getInstance()->printLog();
  
  return changed ;
}

