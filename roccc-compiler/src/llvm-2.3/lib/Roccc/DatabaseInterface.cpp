#include "rocccLibrary/DFBasicBlock.h"

#include "llvm/Constants.h"
#include "llvm/Support/CFG.h"

#include <sstream>

#include "rocccLibrary/sqlite3.h"

#include "rocccLibrary/DatabaseInterface.h"
#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/ROCCCNames.h"
#include "rocccLibrary/Version.h"

using namespace Database;

Stream::Stream(std::list<Port*> ports) : cross_clk(NULL), stop_access(NULL), enable_access(NULL), address_clk(NULL), address_rdy(NULL), address_stall(NULL)
{
  assert(ports.begin() != ports.end() and "Cannot have empty port list when creating stream!");
  std::string readableName = (*ports.begin())->getReadableName();
  for(std::list<Port*>::iterator PI = ports.begin(); PI != ports.end(); ++PI)
  {
    assert( (*PI)->getReadableName() == readableName and "All stream ports must have same readable name!");
    std::string type = (*PI)->getType();
    if( type == "STREAM_CROSS_CLK" )
    {
      if( cross_clk != NULL )
      {
        INTERNAL_ERROR("Cross_clk port " << (*PI)->getName() << " found for stream " << readableName << ", but cross_clk port " << cross_clk->getName() << " already found!\n");
      }
      assert( cross_clk == NULL and "More than one cross_clk port found for stream!" );
      cross_clk = (*PI);
    }
    else if( type == "STREAM_STOP_ACCESS" )
    {
      if( stop_access != NULL )
      {
        INTERNAL_ERROR("Stop_access port " << (*PI)->getName() << " found for stream " << readableName << ", but stop_access port " << stop_access->getName() << " already found!\n");
      }
      assert( stop_access == NULL and "More than one stop_access port found for stream!" );
      stop_access = (*PI);
    }
    else if( type == "STREAM_ENABLE_ACCESS" )
    {
      if( enable_access != NULL )
      {
        INTERNAL_ERROR("Enable_access port " << (*PI)->getName() << " found for stream " << readableName << ", but enable_access port " << enable_access->getName() << " already found!\n");
      }
      assert( enable_access == NULL and "More than one enable_access port found for stream!" );
      enable_access = (*PI);
    }
    else if( type == "STREAM_ADDRESS_CLK" )
    {
      if( address_clk != NULL )
      {
        INTERNAL_ERROR("Address clk port " << (*PI)->getName() << " found for stream " << readableName << ", but address clk port " << address_stall->getName() << " already found!\n");
      }
      assert( address_clk == NULL and "More than one address clk port found for stream!" );
      address_clk = (*PI);
    }
    else if( type == "STREAM_ADDRESS_STALL" )
    {
      if( address_stall != NULL )
      {
        INTERNAL_ERROR("Address stall port " << (*PI)->getName() << " found for stream " << readableName << ", but address stall port " << address_stall->getName() << " already found!\n");
      }
      assert( address_stall == NULL and "More than one address stall port found for stream!" );
      address_stall = (*PI);
    }
    else if( type == "STREAM_ADDRESS_RDY" )
    {
      if( address_rdy != NULL )
      {
        INTERNAL_ERROR("Address_rdy port " << (*PI)->getName() << " found for stream " << readableName << ", but address_rdy port " << address_rdy->getName() << " already found!\n");
      }
      assert( address_rdy == NULL and "More than one address_rdy port found for stream!" );
      address_rdy = (*PI);
    }
    else if( type == "STREAM_ADDRESS_BASE" )
    {
      address_channels_base.push_back(*PI);
    }
    else if( type == "STREAM_ADDRESS_COUNT" )
    {
      address_channels_count.push_back(*PI);
    }
    else if( type == "STREAM_CHANNEL" )
    {
      data_channels.push_back(*PI);
    }
    else
    {
      INTERNAL_ERROR("Type " << type << " of port " << (*PI)->getName() << " is unknown!\n");
      assert(0 and "Unknown database port type!");
    }
  }
  assert(cross_clk and "Cannot have stream without cross_clk port!");
  assert(stop_access and "Cannot have stream without stop_access port!");
  assert(enable_access and "Cannot have stream without enable_access port!");
  assert(data_channels.size() != 0 and "Cannot have stream with no data channels!");
  assert(address_channels_base.size() != 0 and "Cannot have stream with no address channels!");
  assert(address_channels_base.size() == address_channels_count.size() and "Number of address channel's base an count ports must match!");
  assert(address_clk and "Cannot have stream with no address clk port!");
  assert(address_stall and "Cannot have stream with no address stall port!");
  assert(address_rdy and "Cannot have stream with no address_rdy port!");
}
std::string Stream::getName() const
{
  assert(cross_clk);
  return cross_clk->getReadableName();
}
bool Stream::isOutput() const
{
  assert(getDataChannels().size() > 0);
  return (*getDataChannels().begin())->isOutput();
}
bool Stream::isInput() const
{
  return !isOutput();
}
Port* Stream::getCrossClk() const
{
  return cross_clk;
}
Port* Stream::getStopAccess() const
{
  return stop_access;
}
Port* Stream::getEnableAccess() const
{
  return enable_access;
}
Port* Stream::getAddressClk() const
{
  return address_clk;
}
Port* Stream::getAddressStall() const
{
  return address_stall;
}
Port* Stream::getAddressReady() const
{
  return address_rdy;
}
const std::list<Port*>& Stream::getDataChannels() const
{
  return data_channels;
}
const std::list<Port*>& Stream::getAddressChannelsBase() const
{
  return address_channels_base;
}
const std::list<Port*>& Stream::getAddressChannelsCount() const
{
  return address_channels_count;
}

LibraryEntry::LibraryEntry()
{
  delay = -1 ;
  name = "" ;
}
LibraryEntry::~LibraryEntry()
{
  /*
  std::list<Port*>::iterator delIter ;
  delIter = allPorts.begin() ;
  while (delIter != allPorts.end())
  {
    delete (*delIter) ;
    ++delIter ;
  }
  */
}
LibraryEntry::LibraryEntry(std::string n, int d, std::list<Port*> p, TYPE t, int _id) :
name(n), delay(d), allPorts(p), type(t), id(_id)
{
  ; // Nothing else to define
}
LibraryEntry::LibraryEntry(std::string n, int d, std::list<Port*> p, std::string st_type, int _id) :
name(n), delay(d), allPorts(p), type(MODULE), id(_id)
{
  if( st_type == "MODULE" )
    type = LibraryEntry::MODULE;
  else if( st_type == "SYSTEM" )
    type = LibraryEntry::SYSTEM;
  else if( st_type == "INT_DIV" )
    type = LibraryEntry::INT_DIV;
  else if( st_type == "INT_MOD" )
    type = LibraryEntry::INT_MOD;
  else if( st_type == "FP_ADD" )
    type = LibraryEntry::FP_ADD;
  else if( st_type == "FP_SUB" )
    type = LibraryEntry::FP_SUB;
  else if( st_type == "FP_MUL" )
    type = LibraryEntry::FP_MUL;
  else if( st_type == "FP_DIV" )
    type = LibraryEntry::FP_DIV;
  else if( st_type == "FP_EQUAL" )
    type = LibraryEntry::FP_EQ;
  else if( st_type == "FP_NOT_EQUAL" )
    type = LibraryEntry::FP_NEQ;
  else if( st_type == "FP_LESS_THAN" )
    type = LibraryEntry::FP_LT;
  else if( st_type == "FP_GREATER_THAN" )
    type = LibraryEntry::FP_GT;
  else if( st_type == "FP_LESS_THAN_EQUAL" )
    type = LibraryEntry::FP_LTE;
  else if( st_type == "FP_GREATER_THAN_EQUAL" )
    type = LibraryEntry::FP_GTE;
  else if( st_type == "FP_TO_INT" )
    type = LibraryEntry::FP_TO_INT;
  else if( st_type == "FP_TO_FP" )
    type = LibraryEntry::FP_TO_FP;
  else if( st_type == "INT_TO_FP" )
    type = LibraryEntry::INT_TO_FP;
  else if( st_type == "INT_TO_INT" )
    type = LibraryEntry::INT_TO_INT;
  else if( st_type == "DOUBLE_VOTE" )
    type = LibraryEntry::DOUBLE_VOTE;
  else if( st_type == "TRIPLE_VOTE" )
    type = LibraryEntry::TRIPLE_VOTE;
  else if( st_type == "STREAM_SPLITTER" )
    type = LibraryEntry::STREAM_SPLITTER;
  else if( st_type == "STREAM_DOUBLE_VOTE" )
    type = LibraryEntry::STREAM_DOUBLE_VOTE;
  else if( st_type == "STREAM_TRIPLE_VOTE" )
    type = LibraryEntry::STREAM_TRIPLE_VOTE;
}

bool LibraryEntry::IsPort(std::string name)
{
  std::list<Port*>::iterator portIter = allPorts.begin() ;
  while (portIter != allPorts.end())
  {
    if ((*portIter)->getName() == name)
    {
      return true ;
    }
    ++portIter ;
  }
  return false ;
}
bool LibraryEntry::IsInputPort( std::string name )
{
  std::list<Port*>::iterator portIter = allPorts.begin() ;
  while (portIter != allPorts.end())
  {
    if ( (*portIter)->getName() == name && !(*portIter)->isOutput() )
    {
      return true ;
    }
    ++portIter ;
  }
  return false ;
}
bool LibraryEntry::IsOutputPort( std::string name )
{
  return (IsPort(name) && !IsInputPort(name));
}
bool LibraryEntry::IsOutputPort(int portNum)
{
  assert(static_cast<unsigned>(portNum) < allPorts.size() and "portNum is greater than total number of ports!") ;
  assert(portNum >= 0 and "portNum must be non-negative!") ;
  assert(allPorts.size() > 0 and "Searching empty port list!") ;
  std::list<Port*>::iterator portIter = allPorts.begin() ;
  for (int i = 0 ; i < portNum ; ++i)
    ++portIter ;
  return (*portIter)->isOutput() ;
}
std::list<Port*> LibraryEntry::getAllPorts()
{
  return allPorts;
}
std::list<Port*> LibraryEntry::getNonStreamPorts()
{
  std::list<Port*> non_stream;
  for(std::list<Port*>::iterator PI = allPorts.begin(); PI != allPorts.end(); ++PI)
  {
    if( (*PI)->getType() == "REGISTER" or
        (*PI)->getType() == "DEBUG" )
    {
      non_stream.push_back(*PI);
    }
  }
  return non_stream;
}
std::list<Port*> LibraryEntry::getNonDebugScalarPorts()
{
  std::list<Port*> scalars;
  for(std::list<Port*>::iterator PI = allPorts.begin(); PI != allPorts.end(); ++PI)
  {
    if( (*PI)->getType() == "REGISTER" )
    {
      scalars.push_back(*PI);
    }
  }
  return scalars;
}
std::list<Stream> LibraryEntry::getStreams()
{
  std::list<Stream> streams;
  for(std::list<Port*>::iterator PI = allPorts.begin(); PI != allPorts.end(); ++PI)
  {
    if( (*PI)->getType() == "STREAM_CROSS_CLK" )
    {
      std::string readableName = (*PI)->getReadableName();
      //get all of the ports that have the same readableName
      std::list<Port*> streamPorts;
      for(std::list<Port*>::iterator PI2 = allPorts.begin(); PI2 != allPorts.end(); ++PI2)
      {
        if( (*PI2)->getReadableName() == readableName )
        {
          streamPorts.push_back(*PI2);
        }
      }
      streams.push_back(Stream(streamPorts));
    }
  }
  return streams;
}
std::string LibraryEntry::convTypeToString(TYPE t)
{
  switch ( t )
  {
    case LibraryEntry::MODULE:             return "MODULE";
    case LibraryEntry::SYSTEM:             return "SYSTEM";
    case LibraryEntry::INT_DIV:            return "INT_DIV";
    case LibraryEntry::INT_MOD:            return "INT_MOD";
    case LibraryEntry::FP_ADD:             return "FP_ADD";
    case LibraryEntry::FP_SUB:             return "FP_SUB";
    case LibraryEntry::FP_MUL:             return "FP_MUL";
    case LibraryEntry::FP_DIV:             return "FP_DIV";
    case LibraryEntry::FP_EQ:              return "FP_EQUAL";
    case LibraryEntry::FP_NEQ:             return "FP_NOT_EQUAL";
    case LibraryEntry::FP_LT:              return "FP_LESS_THAN";
    case LibraryEntry::FP_GT:              return "FP_GREATER_THAN";
    case LibraryEntry::FP_LTE:             return "FP_LESS_THAN_EQUAL";
    case LibraryEntry::FP_GTE:             return "FP_GREATER_THAN_EQUAL";
    case LibraryEntry::FP_TO_INT:          return "FP_TO_INT";
    case LibraryEntry::FP_TO_FP:           return "FP_TO_FP";
    case LibraryEntry::INT_TO_FP:          return "INT_TO_FP";
    case LibraryEntry::INT_TO_INT:         return "INT_TO_INT";
    case LibraryEntry::DOUBLE_VOTE:        return "DOUBLE_VOTE";
    case LibraryEntry::TRIPLE_VOTE:        return "TRIPLE_VOTE";
    case LibraryEntry::STREAM_SPLITTER:    return "STREAM_SPLITTER";
    case LibraryEntry::STREAM_DOUBLE_VOTE: return "STREAM_DOUBLE_VOTE";
    case LibraryEntry::STREAM_TRIPLE_VOTE: return "STREAM_TRIPLE_VOTE";
    default:
      INTERNAL_ERROR("Unknown type " << t << "!\n");
      assert(0 and "Unknown type! Database corruption!");
  }
}

DatabaseInterface* DatabaseInterface::instance = NULL;
DatabaseInterface* DatabaseInterface::getInstance()
{
  if( instance == NULL )
    instance = new DatabaseInterface();
  return instance;
}
DatabaseInterface::DatabaseInterface()
{
  handle = NULL ;
  std::string pathToDBFile = "" ;
  assert( getenv("DATABASE_PATH") and "DATABASE_PATH must be set!" );
  pathToDBFile += getenv("DATABASE_PATH") ;
  assert(pathToDBFile != "" && "DATABASE_PATH not set") ;
  pathToDBFile += "/vhdlLibrary.sql3" ;
  if( SQLITE_OK != sqlite3_open_v2(pathToDBFile.c_str(), &handle, SQLITE_OPEN_READWRITE, NULL) )
  {
    INTERNAL_ERROR("Could not open database; " << sqlite3_errmsg(handle) << "\n");
    assert(0 and "Could not open database." );
  }
  assert(handle != NULL and "Error opening sql database!") ;
  CreateCoreTables() ;
  ReadAllEntries();
}
DatabaseInterface::~DatabaseInterface()
{
  sqlite3_close(handle) ;
  handle = NULL ;
  if( instance == this )
    instance = NULL;
  
  // Delete all of the library cores that we might have created
  std::list<LibraryEntry*>::iterator coreIter = allCores.begin() ;
  while (coreIter != allCores.end())
  {
    delete (*coreIter) ;
    ++coreIter ;
  }
  
}
// If this fails, then the core table must already exist.  In either case,
//  this is a safe call to perform.
void DatabaseInterface::CreateCoreTables()
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
    INTERNAL_SERIOUS_WARNING("Could not create table \'CompileInfo\'; (error " << currentValue << ")" << sqlite3_errmsg(handle) << "\n");
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
    INTERNAL_SERIOUS_WARNING("Could not create table \'ComponentInfo\'; (error " << currentValue << ")" << sqlite3_errmsg(handle) << "\n");
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
    INTERNAL_SERIOUS_WARNING("Could not create table \'Ports\'; (error " << currentValue << ")" << sqlite3_errmsg(handle) << "\n");
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
    INTERNAL_SERIOUS_WARNING("Could not create table \'ResourcesUsed\'; (error " << currentValue << ")" << sqlite3_errmsg(handle) << "\n");
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
    INTERNAL_SERIOUS_WARNING("Could not create table \'FileInfo\'; (error " << currentValue << ")" << sqlite3_errmsg(handle) << "\n");
  }
  sqlite3_finalize(returnHandle) ;
  
  //create the stream info table; if it already exists, this call does nothing
  createTableQuery = "CREATE TABLE IF NOT EXISTS `StreamInfo` (\
            `id` INTEGER REFERENCES CompileInfo(id) ON UPDATE CASCADE ON DELETE CASCADE,\
            'readableName' VARCHAR(255) DEFAULT NULL ,\
            `NumElementsCalculationFormula` VARCHAR(2550) DEFAULT NULL,\
            'NumTotalWindowElements' INTEGER DEFAULT NULL,\
            'NumAccessedWindowElements' INTEGER DEFAULT NULL\
          );";
  sqlite3_prepare(handle,
		  createTableQuery.c_str(),
		  createTableQuery.size(),
		  &returnHandle,
		  &remainder) ;
  currentValue = sqlite3_step(returnHandle) ;
  if (currentValue == SQLITE_MISUSE or currentValue == SQLITE_ERROR)
  {
    INTERNAL_SERIOUS_WARNING("Could not create table \'StreamInfo\'; (error " << currentValue << ")" << sqlite3_errmsg(handle) << "\n");
  }
  sqlite3_finalize(returnHandle) ;
}
bool DatabaseInterface::EntryExists(std::string coreName)
{
  for(std::list<LibraryEntry*>::iterator CI = allCores.begin(); CI != allCores.end(); ++CI)
  {
    if( (*CI)->getName() == coreName )
    {
      return true;
    }
  }
  assert(handle != NULL and "Must open database before searching!") ;
  
  std::string lookupQuery = "SELECT * from ComponentInfo WHERE ComponentInfo.componentName = \'" ;
  lookupQuery += coreName ;
  lookupQuery += "\' AND ComponentInfo.active = 1" ;
  
  sqlite3_stmt* returnHandle ;
  const char* remainder ;
  int currentValue ;
  
  sqlite3_prepare(handle,
                  lookupQuery.c_str(),
                  lookupQuery.size(),
                  &returnHandle,
                  &remainder) ;
  currentValue = sqlite3_step(returnHandle) ;
  if( currentValue == SQLITE_ERROR or currentValue == SQLITE_MISUSE )
  {
    INTERNAL_SERIOUS_WARNING("Database error while checking whether component exists; (error " << currentValue << ")" << sqlite3_errmsg(handle) << "\n");
    return false;
  }
  const unsigned char* firstColumn = sqlite3_column_text(returnHandle, 0) ;
  sqlite3_finalize(returnHandle) ;
  return (firstColumn != NULL) ;
}
LibraryEntry DatabaseInterface::LookupEntry(std::string coreName)
{
  for(std::list<LibraryEntry*>::iterator CI = allCores.begin(); CI != allCores.end(); ++CI)
  {
    if( (*CI)->getName() == coreName )
    {
      return **CI;
    }
  }
  //it doesnt exist in the allCores list, so find it in the database
  assert(handle != NULL and "Must open database before searching!") ;
  assert( EntryExists(coreName) );
  
  std::string lookupQuery = "SELECT ComponentInfo.delay,ComponentInfo.type, ComponentInfo.id from ComponentInfo WHERE ComponentInfo.componentName = \'" ;
  lookupQuery += coreName ;
  lookupQuery += "\' AND ComponentInfo.active = 1" ;
  
  sqlite3_stmt* returnHandle ;
  const char* remainder ;
  int currentValue ;
  
  sqlite3_prepare(handle,
                  lookupQuery.c_str(),
                  lookupQuery.size(),
                  &returnHandle,
                  &remainder) ;
  currentValue = sqlite3_step(returnHandle) ;
  if( currentValue == SQLITE_ERROR or currentValue == SQLITE_MISUSE )
  {
    INTERNAL_ERROR("Database error while looking up component; (error " << currentValue << ")" << sqlite3_errmsg(handle) << "\n");
    assert(0 and "Database error.\n");
  }
  int delay = sqlite3_column_int(returnHandle, 0) ;
  std::string type = reinterpret_cast<const char*>(sqlite3_column_text(returnHandle, 1)) ;
  int id = sqlite3_column_int(returnHandle, 2) ;
  sqlite3_finalize(returnHandle) ;
  
  std::list<Port*> allPorts ;
  
  // Now, find all the ports
  std::string portQuery = "SELECT Ports.vhdlName, Ports.direction, Ports.bitwidth, Ports.type, Ports.readableName, Ports.dataType FROM Ports INNER JOIN ComponentInfo ON Ports.id=ComponentInfo.id WHERE ComponentInfo.componentName='" + coreName + "' AND ComponentInfo.active = 1 ORDER BY Ports.portNum";
  sqlite3_prepare(handle,
                  portQuery.c_str(),
                  portQuery.size(),
                  &returnHandle,
                  &remainder) ;
  currentValue = sqlite3_step(returnHandle) ;
  if (currentValue == SQLITE_ERROR or currentValue == SQLITE_MISUSE)
  {
    INTERNAL_ERROR(coreName << " not found in database (Did you remove a built in core?); " << sqlite3_errmsg(handle) << "\n") ;
    assert(0 and "Could not find component!") ;
  } 
  while (currentValue != SQLITE_DONE &&
         currentValue != SQLITE_ERROR &&
         currentValue != SQLITE_MISUSE)
  {
    std::string isInputString = reinterpret_cast<const char*>(sqlite3_column_text(returnHandle, 1)) ;
    std::string type = reinterpret_cast<const char*>(sqlite3_column_text(returnHandle, 3)) ;
    
    std::string readableName;
    if( reinterpret_cast<const char*>(sqlite3_column_text(returnHandle, 4)) )
      readableName = reinterpret_cast<const char*>(sqlite3_column_text(returnHandle, 4));
    std::string dataType;
    if( reinterpret_cast<const char*>(sqlite3_column_text(returnHandle, 5)) )
      dataType = reinterpret_cast<const char*>(sqlite3_column_text(returnHandle, 5));
    Port* nextPort = new Port(reinterpret_cast<const char*>(sqlite3_column_text(returnHandle, 0)),
                              (isInputString == "IN"),
                              sqlite3_column_int(returnHandle, 2),
                              type,
                              readableName,
                              dataType ) ;
    allPorts.push_back(nextPort) ;
    currentValue = sqlite3_step(returnHandle) ;
  }
  sqlite3_finalize(returnHandle) ;
  
  LibraryEntry toReturn(coreName, delay, allPorts, type, id) ;
  
  // Return by value...makes a copy
  return toReturn ;
}
// This function should be called when we have created a library entry
//  for the function being compiled and it did not previously exist in
//  the database.
void DatabaseInterface::CreateEntry(LibraryEntry* toAdd)
{
  assert(handle != NULL) ;
  sqlite3_stmt* returnHandle ;
  const char* remainder ;
  int currentValue ;
  std::string checkQuery;
  
  //we need to remove the component info last, as the rest of the deletes are
  // based off of it's name
  
  //remove the previous entry from the compile info table
  checkQuery = "DELETE FROM CompileInfo WHERE CompileInfo.id IN (\
                  SELECT ComponentInfo.id FROM ComponentInfo WHERE ComponentInfo.componentName = \'" + toAdd->getName() + "\'\
                ); ";
  sqlite3_prepare(handle,
                  checkQuery.c_str(),
                  checkQuery.size(),
                  &returnHandle,
                  &remainder) ;
  currentValue = sqlite3_step(returnHandle) ;
  if (currentValue == SQLITE_ERROR or currentValue == SQLITE_MISUSE)
  {
    INTERNAL_WARNING("Problem deleting compile info of " << toAdd->getName() << " from database; (error " << currentValue << ")" << sqlite3_errmsg(handle) << "\n");
  }
  sqlite3_finalize(returnHandle) ;
  //also delete the ports of the previous entry
  checkQuery = "DELETE FROM Ports WHERE Ports.id IN (\
                  SELECT ComponentInfo.id FROM ComponentInfo WHERE ComponentInfo.componentName = \'" + toAdd->getName() + "\'\
                ); ";
  sqlite3_prepare(handle,
                  checkQuery.c_str(),
                  checkQuery.size(),
                  &returnHandle,
                  &remainder) ;
  currentValue = sqlite3_step(returnHandle) ;
  if (currentValue == SQLITE_ERROR or currentValue == SQLITE_MISUSE)
  {
    INTERNAL_WARNING("Problem deleting port of " << toAdd->getName() << " from database; (error " << currentValue << ")" << sqlite3_errmsg(handle) << "\n");
  }
  sqlite3_finalize(returnHandle) ;
  //also delete the files of the previous entry
  checkQuery = "DELETE FROM FileInfo WHERE FileInfo.id IN (\
                  SELECT ComponentInfo.id FROM ComponentInfo WHERE ComponentInfo.componentName = \'" + toAdd->getName() + "\'\
                ); ";
  sqlite3_prepare(handle,
                  checkQuery.c_str(),
                  checkQuery.size(),
                  &returnHandle,
                  &remainder) ;
  currentValue = sqlite3_step(returnHandle) ;
  if (currentValue == SQLITE_ERROR or currentValue == SQLITE_MISUSE)
  {
    INTERNAL_WARNING("Problem deleting file info of " << toAdd->getName() << " from database; (error " << currentValue << ")" << sqlite3_errmsg(handle) << "\n");
  }
  sqlite3_finalize(returnHandle) ;
  //also delete the resources of the previous entry
  checkQuery = "DELETE FROM ResourcesUsed WHERE ResourcesUsed.id IN (\
                  SELECT ComponentInfo.id FROM ComponentInfo WHERE ComponentInfo.componentName = \'" + toAdd->getName() + "\'\
                ); ";
  sqlite3_prepare(handle,
                  checkQuery.c_str(),
                  checkQuery.size(),
                  &returnHandle,
                  &remainder) ;
  currentValue = sqlite3_step(returnHandle) ;
  if (currentValue == SQLITE_ERROR or currentValue == SQLITE_MISUSE)
  {
    INTERNAL_WARNING("Problem deleting resource info of " << toAdd->getName() << " from database; (error " << currentValue << ")" << sqlite3_errmsg(handle) << "\n");
  }
  sqlite3_finalize(returnHandle) ;
  //if its an intrinsic (ie, not a system or module), then deactivate all other intrinsics with same type
  if( toAdd->getType() != LibraryEntry::MODULE and toAdd->getType() != LibraryEntry::SYSTEM )
  {
    checkQuery = "UPDATE ComponentInfo SET ComponentInfo.active=0 WHERE ComponentInfo.type = \'" + LibraryEntry::convTypeToString(toAdd->getType()) + "\'";
    sqlite3_prepare(handle,
                    checkQuery.c_str(),
                    checkQuery.size(),
                    &returnHandle,
                    &remainder) ;
    currentValue = sqlite3_step(returnHandle) ;
    if (currentValue == SQLITE_ERROR or currentValue == SQLITE_MISUSE)
    {
      INTERNAL_WARNING("Problem setting other intrinsics of type " << LibraryEntry::convTypeToString(toAdd->getType()) << " to inactive in database; (error " << currentValue << ")" << sqlite3_errmsg(handle) << "\n");
    }
    sqlite3_finalize(returnHandle) ;
  }
  //remove the previous entry from the component info table
	checkQuery = "DELETE FROM ComponentInfo WHERE ComponentInfo.componentName = \'" + toAdd->getName() + "\'" ;
  sqlite3_prepare(handle,
                  checkQuery.c_str(),
                  checkQuery.size(),
                  &returnHandle,
                  &remainder) ;
  currentValue = sqlite3_step(returnHandle) ;
  if (currentValue == SQLITE_ERROR or currentValue == SQLITE_MISUSE)
  {
    INTERNAL_WARNING("Problem deleting component info of " << toAdd->getName() << " from database; (error " << currentValue << ")" << sqlite3_errmsg(handle) << "\n");
  }
  sqlite3_finalize(returnHandle) ;
  //add the new entry's component info
  {
    std::stringstream ss;
    ss << "INSERT INTO ComponentInfo (componentName, id, type, delay, active) VALUES "
       << "('" << toAdd->getName() << "', " << toAdd->getID() << ", '" << LibraryEntry::convTypeToString(toAdd->getType()) << "', " << toAdd->getDelay() << ", 1);";
    sqlite3_prepare(handle,
  		  ss.str().c_str(),
  		  ss.str().size(),
  		  &returnHandle,
  		  &remainder) ;
    assert(returnHandle != NULL) ;
    currentValue = sqlite3_step(returnHandle) ;
    if (currentValue == SQLITE_MISUSE or currentValue == SQLITE_ERROR)
    {
      INTERNAL_WARNING("Cannot add component info for " << toAdd->getName() << " to database; (error " << currentValue << ")" << sqlite3_errmsg(handle) << "\n");
    }
    sqlite3_finalize(returnHandle) ;
  }
  //also set the CompileInfo.compilerVersion to the current compiler version
  {
    std::stringstream ss;
    ss << "UPDATE CompileInfo SET compilerVersion = \'" << ROCCC_VERSION_NUM << "\' WHERE CompileInfo.id = " << toAdd->getID();
    sqlite3_prepare(handle,
                    ss.str().c_str(),
                    ss.str().size(),
                    &returnHandle,
                    &remainder) ;
    currentValue = sqlite3_step(returnHandle) ;
    if (currentValue == SQLITE_MISUSE or currentValue == SQLITE_ERROR)
    {
      INTERNAL_WARNING("Problem setting compiler version number of " << toAdd->getName() << " in database; (error " << currentValue << ")" << sqlite3_errmsg(handle) << "\n");
    }
    sqlite3_finalize(returnHandle) ;
  }
  //add all of the ports  
  const std::list<Port*> allPorts = toAdd->getAllPorts() ;
  std::list<Port*>::const_iterator portIter = allPorts.begin() ;
  int portNum = 1;
  while(portIter != allPorts.end())
  {
    std::stringstream ss;
    ss << "INSERT INTO Ports (id, readableName, type, portNum, vhdlName, direction, bitwidth, dataType) VALUES"
       << "(" << toAdd->getID() << ", '" << (*portIter)->getReadableName() << "', '" << (*portIter)->getType() << "', " << portNum << ", '" << (*portIter)->getName() << "', '" << ((*portIter)->isOutput() ? "OUT" : "IN") << "', " << (*portIter)->getBitSize() << ", '" << (*portIter)->getDataType() << "');";
    sqlite3_prepare(handle,
  		  ss.str().c_str(),
  		  ss.str().size(),
  		  &returnHandle,
  		  &remainder) ;
    currentValue = sqlite3_step(returnHandle) ;
    if (currentValue == SQLITE_MISUSE or currentValue == SQLITE_ERROR)
    {
      INTERNAL_WARNING("Cannot insert port " << (*portIter)->getName() << " from component " << toAdd->getName() << " into the database; (error " << currentValue << ")" << sqlite3_errmsg(handle) << "\n");
    }
    sqlite3_finalize(returnHandle) ;
    
    ss.clear() ;
    ++portIter ;
    ++portNum;
  }
  
}
void DatabaseInterface::ReadAllEntries()
{
  // Select all cores in the core table.
  std::string selectQuery = "SELECT componentName FROM ComponentInfo WHERE active=1" ;
  
  sqlite3_stmt* returnHandle ;
  const char* remainder ;
  int currentCore ;
  
  sqlite3_prepare(handle,
                  selectQuery.c_str(),
                  selectQuery.size(),
                  &returnHandle,
                  &remainder) ;
  currentCore = sqlite3_step(returnHandle) ;
  
  allCores.clear();
  // If the library is empty, we don't add any cores.  That's a.o.k.
  
  while (currentCore != SQLITE_DONE &&
         currentCore != SQLITE_ERROR &&
         currentCore != SQLITE_MISUSE)
  {
    std::string nextName ;
    nextName += reinterpret_cast<const char*>(sqlite3_column_text(returnHandle, 0)) ;
    allCores.push_back(new LibraryEntry(this->LookupEntry(nextName)));
    currentCore = sqlite3_step(returnHandle) ;
  }
}
