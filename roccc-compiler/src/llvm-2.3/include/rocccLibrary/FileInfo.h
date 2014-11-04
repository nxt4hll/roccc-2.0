// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

  

*/
#ifndef _FILE_INFO_DOT_H__
#define _FILE_INFO_DOT_H__

#include <string>
#include <vector>

namespace Database {

class FileInfo {
public:
  enum FILE_TYPE {C_SOURCE, VHDL_SOURCE, VHDL_LOOPCONTROLLER, VHDL_INPUTCONTROLLER, VHDL_OUTPUTCONTROLLER, REPORT, DATAPATH_GRAPH, INFERRED_FIFO};
  FileInfo(std::string n, FILE_TYPE t, std::string loc);
  std::string getName();
  FILE_TYPE getType();
  std::string getLocation();
private:
  std::string name;
  FILE_TYPE type;
  std::string location;
};

class FileInfoInterface {
  FileInfoInterface(); //DO NOT IMPLEMENT
public:
  static std::vector<FileInfo> getFileInfo(int id);
  static void addFileInfo(int id, FileInfo fi);  
};

}
#endif
