#include "rocccLibrary/FileInfo.h"
#include "rocccLibrary/DatabaseInterface.h"
#include "rocccLibrary/sqlite3.h"
#include "rocccLibrary/InternalWarning.h"

#include <cassert>
#include <sstream>

using namespace Database;
  
FileInfo::FileInfo(std::string n, FileInfo::FILE_TYPE t, std::string loc) : name(n), type(t), location(loc)
{
}
std::string FileInfo::getName()
{
  return name;
}
FileInfo::FILE_TYPE FileInfo::getType()
{
  return type;
}
std::string FileInfo::getLocation()
{
  return location;
}
FileInfo::FILE_TYPE stringToType(std::string t)
{
  if( t == "C_SOURCE" )
    return FileInfo::C_SOURCE;
  if( t == "VHDL_SOURCE" )
    return FileInfo::VHDL_SOURCE;
  if( t == "VHDL_LOOPCONTROLLER" )
    return FileInfo::VHDL_LOOPCONTROLLER;
  if( t == "VHDL_INPUTCONTROLLER" )
    return FileInfo::VHDL_INPUTCONTROLLER;
  if( t == "VHDL_OUTPUTCONTROLLER" )
    return FileInfo::VHDL_OUTPUTCONTROLLER;
  if( t == "REPORT" )
    return FileInfo::REPORT;
  if( t == "DATAPATH_GRAPH" )
    return FileInfo::DATAPATH_GRAPH;
  if( t == "INFERRED_FIFO" )
    return FileInfo::INFERRED_FIFO;
  assert(0 and "Unknown type!");
}
std::string typeToString(FileInfo::FILE_TYPE t)
{
  if( t == FileInfo::C_SOURCE )
    return "C_SOURCE";
  if( t == FileInfo::VHDL_SOURCE )
    return "VHDL_SOURCE";
  if( t == FileInfo::VHDL_LOOPCONTROLLER )
    return "VHDL_LOOPCONTROLLER";
  if( t == FileInfo::VHDL_INPUTCONTROLLER )
    return "VHDL_INPUTCONTROLLER";
  if( t == FileInfo::VHDL_OUTPUTCONTROLLER )
    return "VHDL_OUTPUTCONTROLLER";
  if( t == FileInfo::REPORT )
    return "REPORT";
  if( t == FileInfo::DATAPATH_GRAPH )
    return "DATAPATH_GRAPH";
  if( t == FileInfo::INFERRED_FIFO )
    return "INFERRED_FIFO";
  return "";  
}

std::vector<FileInfo> FileInfoInterface::getFileInfo(int id)
{
 return std::vector<FileInfo>();
}
void FileInfoInterface::addFileInfo(int id, FileInfo fi)
{
  sqlite3* handle = DatabaseInterface::getInstance()->getHandle();
  sqlite3_stmt* returnHandle = NULL;
  const char* remainder = NULL;
  int currentValue = 0;
  
  std::stringstream query;
  query << "INSERT INTO FileInfo(id, fileName, fileType, location) VALUES(" << id << ", \'" << fi.getName() << "\', \'" << typeToString(fi.getType()) << "\', \'" << fi.getLocation() << "\')";
  sqlite3_prepare(handle,
                  query.str().c_str(),
                  query.str().size(),
                  &returnHandle,
                  &remainder) ;
  currentValue = sqlite3_step(returnHandle) ;
  if (currentValue == SQLITE_MISUSE || currentValue == SQLITE_ERROR)
  {
    INTERNAL_WARNING("Problem adding file info for file of type " << typeToString(fi.getType()) << " to database; " << sqlite3_errmsg(handle) << "\n");
  }
  sqlite3_finalize(returnHandle) ;
}
