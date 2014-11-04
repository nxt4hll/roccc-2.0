// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#include <iostream>
#include "warning_utils.h"
#include "warning_level.h"

void OutputWarning(const char* toPrint)
{
#if WARNING_LEVEL > 0
  std::cerr << toPrint << std::endl ;
#endif

}

void OutputInformation(const char* toPrint)
{
#if WARNING_LEVEL > 1
  std::cout << toPrint << std::endl ;
#endif
}

// This should go out no matter what
void OutputError(const char* toPrint)
{
  std::cerr << toPrint << std::endl ;
}
