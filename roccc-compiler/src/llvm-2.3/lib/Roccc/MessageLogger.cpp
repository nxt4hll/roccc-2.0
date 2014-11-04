#include "rocccLibrary/MessageLogger.h"
#include "rocccLibrary/InternalWarning.h"
#include <map>
#include <sstream>

namespace ROCCC {

class MessageLoggerImpl {
  class DataNode {
    std::string name;
    DataNode* parent;
    std::vector<DataNode*> children;
    std::vector<std::string> data;
    int getLevel()
    {
      if( parent == NULL )
        return 0;
      else
        return parent->getLevel() + 1;
    }
  public:
    DataNode(std::string n, DataNode* p) : name(n), parent(p) {}
    void add(std::vector<std::string> d)
    {
      for(std::vector<std::string>::iterator SVI = d.begin(); SVI != d.end(); ++SVI)
        data.push_back(*SVI);
    }
    DataNode* getChild(std::string n)
    {
      for(std::vector<DataNode*>::iterator DNI = children.begin(); DNI != children.end(); ++DNI)
      {
        if( (*DNI)->name == n )
          return *DNI;
      }
      DataNode* ret = new DataNode(n, this);
      children.push_back(ret);
      return ret;
    }
    std::string getName(){return name;}
    std::vector<std::string> getData(){return data;}
    std::string print()
    {
      std::stringstream ss;
      if( this->getLevel() != 0 )
        ss << "<h" << this->getLevel()+2 << ">" << this->getName() << "</h" << this->getLevel()+2 << ">\n";
      ss << "<ul>\n";
      for(std::vector<std::string>::iterator DI = data.begin(); DI != data.end(); ++DI)
      {
        std::string temp = *DI;
        std::string newLine = "\n";
        std::string br = "<br>" + newLine;
        size_t position = temp.find(newLine);
        while( position != std::string::npos )
        {
          temp.replace(position, newLine.length(), br);
          position = temp.find(newLine, position + br.length());
        }
        ss << "<li";
        if( temp.find("Error:") != std::string::npos )
          ss << " style=\"color:red\">";
        else if( temp.find("Warning:") != std::string::npos )
          ss << " style=\"color:green\">";
        else
          ss << ">";
        ss << temp;
        ss <<"</li>\n";
      }
      for(std::vector<DataNode*>::iterator CI = children.begin(); CI != children.end(); ++CI)
      {
        ss << "<li>";
        ss << (*CI)->print();
        ss << "</li>\n";
      }
      ss << "</ul>\n";
      return ss.str();
    }
  };
public:
  MessageLoggerImpl(){}
  std::map< std::vector<std::string>,std::vector<std::string> > data;
  std::string print()
  {
    DataNode global("", NULL);
    for( std::map< std::vector<std::string>,std::vector<std::string> >::iterator DI = data.begin(); DI != data.end(); ++DI )
    {
      DataNode* curNode = &global;
      for( std::vector<std::string>::const_iterator HVI = DI->first.begin(); HVI != DI->first.end(); ++HVI )
      {
        curNode = curNode->getChild(*HVI);
      }
      curNode->add(DI->second);
    }
    std::stringstream ss;
    ss << "<html>\n"
       << "<head>\n"
       << "<title>Report</title>\n"
       << "<body>\n";
    ss << global.print();
    ss << "</body>\n"
       << "</html>\n";
    return ss.str();
  }
};

MessageLogger* MessageLogger::instance = NULL;

MessageLogger::MessageLogger() : data(new MessageLoggerImpl())
{
}
MessageLogger* MessageLogger::getInstance()
{
  if( instance == NULL )
    instance = new MessageLogger();
  return instance;
}
void MessageLogger::log(std::vector<std::string> section, std::string info)
{
  INTERNAL_MESSAGE(info);
  data->data[section].push_back(info);
}
std::string MessageLogger::printLog()
{
  return data->print();
}

//helper function to aid in creating a message with a global section
void logMessage(std::string info)
{
  std::vector<std::string> section;
  MessageLogger::getInstance()->log(section, info);
}

//helper function to aid in creating a message with a single section
void logMessage(std::string section1, std::string info)
{
  std::vector<std::string> section;
  section.push_back(section1);
  MessageLogger::getInstance()->log(section, info);
}

//helper function to aid in creating a message with a section and a subsection
void logMessage(std::string section1, std::string subsection1, std::string info)
{
  std::vector<std::string> section;
  section.push_back(section1);
  section.push_back(subsection1);
  MessageLogger::getInstance()->log(section, info);
}

}
