// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

  

*/
#ifndef _DATABASE_INTERFACE_DOT_H__
#define _DATABASE_INTERFACE_DOT_H__

#include <string> 
#include <list>
#include <fstream>

class sqlite3;

namespace Database 
{

  class Port
  {
  public:
    std::string name ;
    bool isInput ;
    int bitSize ;
    std::string type;
    std::string readable_name;
    std::string dataType;
  private:
    Port() { ; } // Do not implement
  public:
  Port(std::string n, bool i, int b, std::string t, std::string rn, std::string dt) : name(n), isInput(i), bitSize(b), type(t), readable_name(rn), dataType(dt) { ; } 
    ~Port() { ; }
    
    inline std::string getName() const { return name ; }
    inline bool isOutput() const { return !isInput ; }
    inline int getBitSize() const { return bitSize ; } 
    inline bool isDebug() const { return (type == "DEBUG"); }
    inline std::string getType() const { return type; }
    inline std::string getReadableName() const { return readable_name; }
    inline std::string getDataType() const { return dataType; }
    
  } ;
  
  class Stream {
    Port* cross_clk;
    Port* stop_access;
    Port* enable_access;
    Port* address_clk;
    Port* address_rdy;
    Port* address_stall; //may be null, if output stream
    std::list<Port*> address_channels_base;
    std::list<Port*> address_channels_count;
    std::list<Port*> data_channels;
  public:
    Stream(std::list<Port*> ports); //takes a list of ports all related to the stream, and puts them in the correct place
    std::string getName() const;
    bool isOutput() const;
    bool isInput() const;
    Port* getCrossClk() const;
    Port* getStopAccess() const;
    Port* getEnableAccess() const;
    Port* getAddressClk() const;
    Port* getAddressReady() const;
    Port* getAddressStall() const;
    const std::list<Port*>& getDataChannels() const;
    const std::list<Port*>& getAddressChannelsBase() const;
    const std::list<Port*>& getAddressChannelsCount() const;
  };
  
  //TODO - implement
  class LookupTable {
    Port* enable;
  public:
    LookupTable(std::list<Port*> ports);
  };
  
  // This class keeps track of one library component.  Each instance
  //  contains all the information necessary to describe a module as a 
  //  component or an entity, but does not contain any implementation
  //  information.  This class contains the functionality to output
  //  either the simple library or the VHDL package file.
  
  class LibraryEntry
  {
  public:
    enum TYPE {MODULE, SYSTEM, 
               INT_DIV, INT_MOD, 
               FP_ADD, FP_SUB, FP_MUL, FP_DIV,
               FP_EQ, FP_NEQ, FP_GT, FP_LT, FP_GTE, FP_LTE,
               FP_TO_INT, INT_TO_FP, FP_TO_FP, INT_TO_INT, DOUBLE_VOTE, TRIPLE_VOTE, MONSTER_VOTE, STREAM_SPLITTER, STREAM_DOUBLE_VOTE, STREAM_TRIPLE_VOTE};
  private:
    std::string name ;
    int delay ;
    std::list<Port*> allPorts ;
    TYPE type;
    int id;
  public:
    LibraryEntry() ;
    ~LibraryEntry() ;
    
    // This constructor is used when we create a Library Entry for the
    //  component we are compiling
    LibraryEntry(std::string n, int d, std::list<Port*> p, TYPE t, int _id) ;
    LibraryEntry(std::string n, int d, std::list<Port*> p, std::string t, int _id) ;
    
    // This function returns true if the value happens to be already
    //  listed as a port for this library entry
    bool IsPort(std::string name) ;

    // Determines if a particular port is an output/input
    bool IsOutputPort(int portNum) ;    
    bool IsInputPort(std::string name);
    bool IsOutputPort(std::string name);
    
    
    std::list<Port*> getAllPorts();
    std::list<Port*> getNonStreamPorts();
    std::list<Port*> getNonDebugScalarPorts();
    std::list<Stream> getStreams();
    std::string getName() const { return name ; }
    int getDelay() const { return delay ; }
    TYPE getType() const { return type; }
    static std::string convTypeToString(TYPE t);
    int getID() const { return id; }
  } ;
  
  class DatabaseInterface
  {
  private:
    sqlite3* handle ;
    
    std::list<LibraryEntry*> allCores ;
    
    void CreateCoreTables() ;
    
    static DatabaseInterface* instance;
    DatabaseInterface() ;
    ~DatabaseInterface() ;
    void ReadAllEntries() ;
  public:
  
    static DatabaseInterface* getInstance();
    sqlite3* getHandle(){return handle;}

    bool EntryExists(std::string coreName) ;
    
    LibraryEntry LookupEntry(std::string coreName) ;
    void CreateEntry(LibraryEntry* toAdd) ;

    std::list<LibraryEntry*>& getCores() { if(allCores.empty())ReadAllEntries(); return allCores ; }
    
  } ;
  
}

#endif 
