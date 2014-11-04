#include "rocccLibrary/DatabaseHelpers.h"

#include <assert.h>
#include <fstream>

int Database::getIDOfComponent(std::string n)
{
  assert(0 and "not supported yet");
}

int Database::getCurrentID()
{
  std::ifstream f1(".ROCCC/.compileInfo");
  assert( f1.good() );
  int curID = -1;
  f1 >> curID;
  assert( curID != -1 );
  return curID;
}
