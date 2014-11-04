/*

  This is a small C++ program to initialize the VHDL library with basic
   floating point cores. This program is automatically compiled and run
   from the install script and should not be run again.

*/

#include "sqlite3.h"

#include <string>
#include <iostream>
#include <sstream>

#include <cstdlib>
#include <cassert>

using namespace std ;

void createSQLTables(sqlite3* handle)
{
  sqlite3_stmt* returnHandle = NULL ;
  const char* remainder ;
  int currentValue ;
  
  assert(handle != NULL and "Must open database before creating!") ;
  //create the core table; if it already exists, this call does nothing
  std::string createTableQuery = "CREATE TABLE IF NOT EXISTS `CompileInfo` (\
            `id` INTEGER PRIMARY KEY AUTOINCREMENT DEFAULT NULL,\
            `configurationName` VARCHAR(255) DEFAULT NULL,\
            `timestamp` TIMESTAMP DEFAULT NULL,\
            `compilerVersion` VARCHAR(255) DEFAULT NULL,\
            `targetFlags` VARCHAR(255) DEFAULT NULL,\
            `highFlags` MEDIUMTEXT DEFAULT NULL,\
            `lowFlags` MEDIUMTEXT DEFAULT NULL,\
            `pipeliningFlags` MEDIUMTEXT DEFAULT NULL,\
            `streamFlags` MEDIUMTEXT DEFAULT NULL,\
            `isCompiled` BOOL DEFAULT false\
          );";
  sqlite3_prepare(handle,
		  createTableQuery.c_str(),
		  createTableQuery.size(),
		  &returnHandle,
		  &remainder) ;
  currentValue = sqlite3_step(returnHandle) ;
  if (currentValue == SQLITE_MISUSE or currentValue == SQLITE_ERROR)
  {
    cerr << "Could not create table \'CompileInfo\'; (error " << currentValue << ")" << sqlite3_errmsg(handle) << "\n";
  }
  sqlite3_finalize(returnHandle) ;

  // create the Database Version table.
  createTableQuery = "CREATE TABLE IF NOT EXISTS `DatabaseVersion` ("
    "`version` VARCHAR(255) DEFAULT NULL);" ;
  sqlite3_prepare(handle,
		  createTableQuery.c_str(),
		  createTableQuery.size(),
		  &returnHandle,
		  &remainder) ;
  currentValue = sqlite3_step(returnHandle) ;
  if (currentValue == SQLITE_MISUSE or currentValue == SQLITE_ERROR)
  {
    cerr << "Could not create table \'DatabaseVersion\'; (error " << currentValue << ")" << sqlite3_errmsg(handle) << "\n";
  }
  sqlite3_finalize(returnHandle) ;
  
  //create the component info table, if it already exists, this call does nothing
  createTableQuery = "CREATE TABLE IF NOT EXISTS `ComponentInfo` (\
            `componentName` VARCHAR(255) DEFAULT NULL,\
            `id` INTEGER REFERENCES CompileInfo(id) ON UPDATE CASCADE ON DELETE CASCADE,\
            `type` VARCHAR(255) DEFAULT NULL,\
            `area` INTEGER DEFAULT NULL,\
            `frequency` INTEGER DEFAULT NULL,\
            `portOrder` MEDIUMTEXT DEFAULT NULL,\
            `structName` VARCHAR(255) DEFAULT NULL,\
            `delay` INTEGER DEFAULT NULL,\
            `active` BOOL DEFAULT false,\
            `description` VARCHAR(255) DEFAULT NULL\
          );";
  sqlite3_prepare(handle,
		  createTableQuery.c_str(),
		  createTableQuery.size(),
		  &returnHandle,
		  &remainder) ;
  currentValue = sqlite3_step(returnHandle) ;
  if (currentValue == SQLITE_MISUSE or currentValue == SQLITE_ERROR)
  {
    cerr << "Could not create table \'ComponentInfo\'; (error " << currentValue << ")" << sqlite3_errmsg(handle) << "\n";
  }
  sqlite3_finalize(returnHandle) ;
  
  //create the port table; if it already exists, this call does nothing
  createTableQuery = "CREATE TABLE IF NOT EXISTS 'Ports' (\
                      `id` INTEGER REFERENCES CompileInfo(id) ON UPDATE CASCADE ON DELETE CASCADE,\
                      'readableName' VARCHAR(255) DEFAULT NULL ,\
                      'type' VARCHAR(255) DEFAULT NULL ,\
                      'portNum' INTEGER DEFAULT NULL ,\
                      'vhdlName' VARCHAR(255) DEFAULT NULL ,\
                      'direction' INTEGER DEFAULT NULL ,\
                      'bitwidth' INTEGER DEFAULT NULL ,\
                      'dataType' VARCHAR(255) DEFAULT NULL\
                      );";
  sqlite3_prepare(handle,
		  createTableQuery.c_str(),
		  createTableQuery.size(),
		  &returnHandle,
		  &remainder) ;
  currentValue = sqlite3_step(returnHandle) ;
  if (currentValue == SQLITE_MISUSE or currentValue == SQLITE_ERROR)
  {
    cerr << "Could not create table \'Ports\'; (error " << currentValue << ")" << sqlite3_errmsg(handle) << "\n";
  }
  sqlite3_finalize(returnHandle) ;
  
  //create the modules used table; if it already exists, this call does nothing
  createTableQuery = "CREATE TABLE IF NOT EXISTS `ResourcesUsed` (\
            `id` INTEGER REFERENCES CompileInfo(id) ON UPDATE CASCADE ON DELETE CASCADE,\
            `resourceID` INTEGER REFERENCES CompileInfo(id) ON UPDATE CASCADE ON DELETE CASCADE,\
            `resourceType` VARCHAR(255) DEFAULT NULL,\
            `numUsed` INTEGER DEFAULT NULL\
          );";
  sqlite3_prepare(handle,
		  createTableQuery.c_str(),
		  createTableQuery.size(),
		  &returnHandle,
		  &remainder) ;
  currentValue = sqlite3_step(returnHandle) ;
  if (currentValue == SQLITE_MISUSE or currentValue == SQLITE_ERROR)
  {
    cerr << "Could not create table \'ResourcesUsed\'; (error " << currentValue << ")" << sqlite3_errmsg(handle) << "\n";
  }
  sqlite3_finalize(returnHandle) ;
  
  //create the modules used table; if it already exists, this call does nothing
  createTableQuery = "CREATE TABLE IF NOT EXISTS `ResourcesCalled` (\
            `id` INTEGER REFERENCES CompileInfo(id) ON UPDATE CASCADE ON DELETE CASCADE,\
            `resourceID` INTEGER REFERENCES CompileInfo(id) ON UPDATE CASCADE ON DELETE CASCADE,\
            `resourceType` VARCHAR(255) DEFAULT NULL,\
            `numUsed` INTEGER DEFAULT NULL\
          );";
  sqlite3_prepare(handle,
		  createTableQuery.c_str(),
		  createTableQuery.size(),
		  &returnHandle,
		  &remainder) ;
  currentValue = sqlite3_step(returnHandle) ;
  if (currentValue == SQLITE_MISUSE or currentValue == SQLITE_ERROR)
  {
    cerr << "Could not create table \'ResourcesUsed\'; (error " << currentValue << ")" << sqlite3_errmsg(handle) << "\n";
  }
  sqlite3_finalize(returnHandle) ;
  
  //create the file info table; if it already exists, this call does nothing
  createTableQuery = "CREATE TABLE IF NOT EXISTS `FileInfo` (\
            `id` INTEGER REFERENCES CompileInfo(id) ON UPDATE CASCADE ON DELETE CASCADE,\
            `fileName` VARCHAR(255) DEFAULT NULL,\
            `fileType` VARCHAR(255) DEFAULT NULL,\
            `location` VARCHAR(255) DEFAULT NULL\
          );";
  sqlite3_prepare(handle,
		  createTableQuery.c_str(),
		  createTableQuery.size(),
		  &returnHandle,
		  &remainder) ;
  currentValue = sqlite3_step(returnHandle) ;
  if (currentValue == SQLITE_MISUSE or currentValue == SQLITE_ERROR)
  {
    cerr << "Could not create table \'FileInfo\'; (error " << currentValue << ")" << sqlite3_errmsg(handle) << "\n";
  }
  sqlite3_finalize(returnHandle) ;
}

int insertIntrinsicInfo(sqlite3* handle, std::string name, std::string intrinsic_type, int delay)
{
  sqlite3_stmt* returnHandle = NULL ;
  const char* remainder ;
  int currentValue ;
  stringstream ss;
  ss << "INSERT INTO CompileInfo (isCompiled) VALUES (\'true\');";
  sqlite3_prepare(handle,
		  ss.str().c_str(),
		  ss.str().size(),
		  &returnHandle,
		  &remainder) ;
  if (!returnHandle)
  {
	  cerr << ss.str() << "\n";
  }
  assert(returnHandle != NULL) ;
  currentValue = sqlite3_step(returnHandle) ;
  assert(currentValue == SQLITE_DONE ) ;
  sqlite3_finalize(returnHandle) ;
  int id = sqlite3_last_insert_rowid(handle);
  ss.str("");
  ss << "INSERT INTO ComponentInfo (componentName, id, type, delay, active) VALUES (\""
     << name << "\", " << id << ", \"" << intrinsic_type << "\", " << delay << ", 1);";
  sqlite3_prepare(handle,
		  ss.str().c_str(),
		  ss.str().size(),
		  &returnHandle,
		  &remainder) ;
  if (!returnHandle)
  {
	  cerr << ss.str() << "\n";
  }
  assert(returnHandle != NULL) ;
  currentValue = sqlite3_step(returnHandle) ;
  assert(currentValue == SQLITE_DONE ) ; 
  return id;
}

int insertDisabledIntrinsicInfo(sqlite3* handle, std::string name, std::string intrinsic_type, int delay)
{
  sqlite3_stmt* returnHandle = NULL ;
  const char* remainder ;
  int currentValue ;
  stringstream ss;
  ss << "INSERT INTO CompileInfo (isCompiled) VALUES (\'true\');";
  sqlite3_prepare(handle,
		  ss.str().c_str(),
		  ss.str().size(),
		  &returnHandle,
		  &remainder) ;
  if (!returnHandle)
  {
	  cerr << ss.str() << "\n";
  }
  assert(returnHandle != NULL) ;
  currentValue = sqlite3_step(returnHandle) ;
  assert(currentValue == SQLITE_DONE ) ;
  sqlite3_finalize(returnHandle) ;
  int id = sqlite3_last_insert_rowid(handle);
  ss.str("");
  ss << "INSERT INTO ComponentInfo (componentName, id, type, delay, active) VALUES (\""
     << name << "\", " << id << ", \"" << intrinsic_type << "\", " << delay << ", 0);";
  sqlite3_prepare(handle,
		  ss.str().c_str(),
		  ss.str().size(),
		  &returnHandle,
		  &remainder) ;
  if (!returnHandle)
  {
	  cerr << ss.str() << "\n";
  }
  assert(returnHandle != NULL) ;
  currentValue = sqlite3_step(returnHandle) ;
  assert(currentValue == SQLITE_DONE ) ; 
  return id;
}

void insertRegisterPort(sqlite3* handle, int id, int portNum, std::string portName, std::string portDir, int bitsize, std::string dataType)
{
  sqlite3_stmt* returnHandle = NULL ;
  const char* remainder ;
  int currentValue ;
  stringstream ss;
  ss << "INSERT INTO Ports (id, readableName, type, portNum, vhdlName, direction, bitwidth, dataType) VALUES"
     << "(" << id << ", '" << portName << "', 'REGISTER', " << portNum << ", '" << portName << "', '" << portDir << "', " << bitsize << ", '" << dataType << "');";
  sqlite3_prepare(handle,
		  ss.str().c_str(),
		  ss.str().size(),
		  &returnHandle,
		  &remainder) ;
  assert(returnHandle != NULL) ;
  currentValue = sqlite3_step(returnHandle) ;
  assert(currentValue != SQLITE_ERROR && currentValue != SQLITE_MISUSE) ;
  sqlite3_finalize(returnHandle) ;
}

void insertCurrentVersion(sqlite3* handle, std::string version) 
{
  sqlite3_stmt* returnHandle = NULL ;
  const char* remainder ;
  int currentValue ;
  stringstream ss;
  ss << "INSERT INTO DatabaseVersion (version) VALUES"
     << "('" << version << "');";
  sqlite3_prepare(handle,
		  ss.str().c_str(),
		  ss.str().size(),
		  &returnHandle,
		  &remainder) ;
  assert(returnHandle != NULL) ;
  currentValue = sqlite3_step(returnHandle) ;
  assert(currentValue != SQLITE_ERROR && currentValue != SQLITE_MISUSE) ;
  sqlite3_finalize(returnHandle) ;
}

int main(int argc, char* argv[])
{

  sqlite3* handle = NULL ;

  assert(argc == 2) ;

  string pathToLibrary = argv[1] ;

  sqlite3_open(pathToLibrary.c_str(),
	       &handle) ;

  assert(handle != NULL) ;
  
  createSQLTables(handle);
  insertCurrentVersion(handle, "0.7.6") ;
  
  int id;
  
  id = insertIntrinsicInfo(handle, "fp_mul", "FP_MUL", 8);
  insertRegisterPort(handle, id, 1, "a", "IN", 32, "float");
  insertRegisterPort(handle, id, 2, "b", "IN", 32, "float");
  insertRegisterPort(handle, id, 3, "result", "OUT", 32, "float");
  id = insertIntrinsicInfo(handle, "fp_add", "FP_ADD", 13);
  insertRegisterPort(handle, id, 1, "a", "IN", 32, "float");
  insertRegisterPort(handle, id, 2, "b", "IN", 32, "float");
  insertRegisterPort(handle, id, 3, "result", "OUT", 32, "float");
  id = insertIntrinsicInfo(handle, "fp_sub", "FP_SUB", 13);
  insertRegisterPort(handle, id, 1, "a", "IN", 32, "float");
  insertRegisterPort(handle, id, 2, "b", "IN", 32, "float");
  insertRegisterPort(handle, id, 3, "result", "OUT", 32, "float");
  id = insertIntrinsicInfo(handle, "fp_div", "FP_DIV", 13);
  insertRegisterPort(handle, id, 1, "a", "IN", 32, "float");
  insertRegisterPort(handle, id, 2, "b", "IN", 32, "float");
  insertRegisterPort(handle, id, 3, "result", "OUT", 32, "float");
  id = insertIntrinsicInfo(handle, "int_div8", "INT_DIV", 12);
  insertRegisterPort(handle, id, 1, "dividend", "IN", 8, "int");
  insertRegisterPort(handle, id, 2, "divisor", "IN", 8, "int");
  insertRegisterPort(handle, id, 3, "quotient", "OUT", 8, "int");
  id = insertIntrinsicInfo(handle, "int_div16", "INT_DIV", 20);
  insertRegisterPort(handle, id, 1, "dividend", "IN", 16, "int");
  insertRegisterPort(handle, id, 2, "divisor", "IN", 16, "int");
  insertRegisterPort(handle, id, 3, "quotient", "OUT", 16, "int");
  id = insertIntrinsicInfo(handle, "int_div32", "INT_DIV", 36);
  insertRegisterPort(handle, id, 1, "dividend", "IN", 32, "int");
  insertRegisterPort(handle, id, 2, "divisor", "IN", 32, "int");
  insertRegisterPort(handle, id, 3, "quotient", "OUT", 32, "int");
  id = insertIntrinsicInfo(handle, "int_mod8", "INT_MOD", 12);
  insertRegisterPort(handle, id, 1, "dividend", "IN", 8, "int");
  insertRegisterPort(handle, id, 2, "divisor", "IN", 8, "int");
  insertRegisterPort(handle, id, 3, "remainder", "OUT", 8, "int");
  id = insertIntrinsicInfo(handle, "int_mod16", "INT_MOD", 20);
  insertRegisterPort(handle, id, 1, "dividend", "IN", 16, "int");
  insertRegisterPort(handle, id, 2, "divisor", "IN", 16, "int");
  insertRegisterPort(handle, id, 3, "remainder", "OUT", 16, "int");
  id = insertIntrinsicInfo(handle, "int_mod32", "INT_MOD", 36);
  insertRegisterPort(handle, id, 1, "dividend", "IN", 32, "int");
  insertRegisterPort(handle, id, 2, "divisor", "IN", 32, "int");
  insertRegisterPort(handle, id, 3, "remainder", "OUT", 32, "int");

  //comparisons and conversions
  id = insertIntrinsicInfo(handle, "fp_greater_than", "FP_GREATER_THAN", 2) ;
  insertRegisterPort(handle, id, 1, "a", "IN", 32, "float") ;
  insertRegisterPort(handle, id, 2, "b", "IN", 32, "float") ;
  insertRegisterPort(handle, id, 3, "result", "OUT", 1, "int") ;
  id = insertIntrinsicInfo(handle, "fp_less_than", "FP_LESS_THAN", 2) ;
  insertRegisterPort(handle, id, 1, "a", "IN", 32, "float") ;
  insertRegisterPort(handle, id, 2, "b", "IN", 32, "float") ;
  insertRegisterPort(handle, id, 3, "result", "OUT", 1, "int") ;
  id = insertIntrinsicInfo(handle, "fp_equal", "FP_EQUAL", 2) ;
  insertRegisterPort(handle, id, 1, "a", "IN", 32, "float") ;
  insertRegisterPort(handle, id, 2, "b", "IN", 32, "float") ;
  insertRegisterPort(handle, id, 3, "result", "OUT", 1, "int") ;
  id = insertIntrinsicInfo(handle, "fp_greater_than_equal", "FP_GREATER_THAN_EQUAL", 2) ;
  insertRegisterPort(handle, id, 1, "a", "IN", 32, "float") ;
  insertRegisterPort(handle, id, 2, "b", "IN", 32, "float") ;
  insertRegisterPort(handle, id, 3, "result", "OUT", 1, "int") ;
  id = insertIntrinsicInfo(handle, "fp_less_than_equal", "FP_LESS_THAN_EQUAL", 2) ;
  insertRegisterPort(handle, id, 1, "a", "IN", 32, "float") ;
  insertRegisterPort(handle, id, 2, "b", "IN", 32, "float") ;
  insertRegisterPort(handle, id, 3, "result", "OUT", 1, "int") ;
  id = insertIntrinsicInfo(handle, "fp_not_equal", "FP_NOT_EQUAL", 2) ;
  insertRegisterPort(handle, id, 1, "a", "IN", 32, "float") ;
  insertRegisterPort(handle, id, 2, "b", "IN", 32, "float") ;
  insertRegisterPort(handle, id, 3, "result", "OUT", 1, "int") ;
  id = insertIntrinsicInfo(handle, "fp_to_int", "FP_TO_INT", 6) ;
  insertRegisterPort(handle, id, 1, "fpInput", "IN", 32, "float") ;
  insertRegisterPort(handle, id, 2, "intOutput", "OUT", 32, "int") ;
  id = insertIntrinsicInfo(handle, "fp_to_fp", "FP_TO_FP", 2) ;
  insertRegisterPort(handle, id, 1, "fpInput", "IN", 16, "float") ;
  insertRegisterPort(handle, id, 2, "fpOutput", "OUT", 32, "float") ;
  id = insertIntrinsicInfo(handle, "int_to_fp", "INT_TO_FP", 6) ;
  insertRegisterPort(handle, id, 1, "intInput", "IN", 32, "int") ;
  insertRegisterPort(handle, id, 2, "fpOutput", "OUT", 32, "float") ;
  
  //voters
  id = insertIntrinsicInfo(handle, "TripleWordVoter", "TRIPLE_VOTE", 1);
  insertRegisterPort(handle, id, 1, "error", "OUT", 1, "");
  insertRegisterPort(handle, id, 1, "val0_in", "IN", 32, "int");
  insertRegisterPort(handle, id, 1, "val1_in", "IN", 32, "int");
  insertRegisterPort(handle, id, 1, "val2_in", "IN", 32, "int");
  insertRegisterPort(handle, id, 1, "val0_out", "OUT", 32, "int");
  insertRegisterPort(handle, id, 1, "val1_out", "OUT", 32, "int");
  insertRegisterPort(handle, id, 1, "val2_out", "OUT", 32, "int");
  id = insertDisabledIntrinsicInfo(handle, "TripleSingleWordVoter", "TRIPLE_VOTE", 1);
  insertRegisterPort(handle, id, 1, "error", "OUT", 1, "");
  insertRegisterPort(handle, id, 1, "val0_in", "IN", 32, "int");
  insertRegisterPort(handle, id, 1, "val1_in", "IN", 32, "int");
  insertRegisterPort(handle, id, 1, "val2_in", "IN", 32, "int");
  insertRegisterPort(handle, id, 1, "val0_out", "OUT", 32, "int");
  insertRegisterPort(handle, id, 1, "val1_out", "OUT", 32, "int");
  insertRegisterPort(handle, id, 1, "val2_out", "OUT", 32, "int");
  id = insertIntrinsicInfo(handle, "DoubleWordVoter", "DOUBLE_VOTE", 1);
  insertRegisterPort(handle, id, 1, "error", "OUT", 1, "");
  insertRegisterPort(handle, id, 1, "val0_in", "IN", 32, "int");
  insertRegisterPort(handle, id, 1, "val1_in", "IN", 32, "int");
  insertRegisterPort(handle, id, 1, "val2_in", "IN", 32, "int");
  insertRegisterPort(handle, id, 1, "val0_out", "OUT", 32, "int");
  insertRegisterPort(handle, id, 1, "val1_out", "OUT", 32, "int");
  insertRegisterPort(handle, id, 1, "val2_out", "OUT", 32, "int");
  id = insertDisabledIntrinsicInfo(handle, "DoubleSingleWordVoter", "DOUBLE_VOTE", 1);
  insertRegisterPort(handle, id, 1, "error", "OUT", 1, "");
  insertRegisterPort(handle, id, 1, "val0_in", "IN", 32, "int");
  insertRegisterPort(handle, id, 1, "val1_in", "IN", 32, "int");
  insertRegisterPort(handle, id, 1, "val2_in", "IN", 32, "int");
  insertRegisterPort(handle, id, 1, "val0_out", "OUT", 32, "int");
  insertRegisterPort(handle, id, 1, "val1_out", "OUT", 32, "int");
  insertRegisterPort(handle, id, 1, "val2_out", "OUT", 32, "int");
   
  sqlite3_close(handle) ;
   
  return 0 ;
}
