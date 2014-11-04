#ifndef _ROCCC_INTERNAL_WARNING_DOT_H__
#define _ROCCC_INTERNAL_WARNING_DOT_H__

//change this, or remove it, to add in different levels of debug
//#define ROCCC_DEBUG ALL_WARNINGS


#define ALL_WARNINGS MESSAGE_WARNINGS
//message warnings are similar to debug-level warnings
#define MESSAGE_WARNINGS 4
//source warnings are generally comments embedded directly in generated sources
#define SOURCE_WARNINGS 3
//serious warnings should be output to the user, but do not halt execution
#define SERIOUS_WARNINGS 2
//error warnings are those that abort execution
#define ERROR_WARNINGS 1
#define NO_WARNINGS 0

//if we havent defined our own warning level, set it to the default
#ifndef ROCCC_DEBUG
#define ROCCC_DEBUG SERIOUS_WARNINGS
#endif

#if ROCCC_DEBUG >= MESSAGE_WARNINGS
 #ifndef LLVM_VALUE_H
  #include <iostream>
  #define INTERNAL_MESSAGE(x) std::cout << x
  #define INTERNAL_WARNING(x) std::cout << "INTERNAL WARNING - " << CurrentFile::get() <<  ": " << x
 #else
  #define INTERNAL_MESSAGE(x) llvm::cout << x
  #define INTERNAL_WARNING(x) llvm::cout << "INTERNAL WARNING - " << CurrentFile::get() <<  ": " << x
 #endif
#else
 #define INTERNAL_MESSAGE(x)
 #define INTERNAL_WARNING(x)
#endif
#if ROCCC_DEBUG >= ERROR_WARNINGS
 #ifndef LLVM_VALUE_H
  #include <iostream>
  #define INTERNAL_ERROR(x) std::cerr << "INTERNAL ERROR - " << CurrentFile::get() <<  ": " << x
 #else
  #define INTERNAL_ERROR(x) llvm::cerr << "INTERNAL ERROR - " << CurrentFile::get() <<  ": " << x
 #endif
#else
 #define INTERNAL_ERROR(x)
#endif
#if ROCCC_DEBUG >= SERIOUS_WARNINGS
 #ifndef LLVM_VALUE_H
  #include <iostream>
  #define INTERNAL_SERIOUS_WARNING(x) std::cerr << "INTERNAL WARNING - " << CurrentFile::get() <<  ": " << x
 #else
  #define INTERNAL_SERIOUS_WARNING(x) llvm::cerr << "INTERNAL WARNING - " << CurrentFile::get() <<  ": " << x
 #endif
#else
 #define INTERNAL_SERIOUS_WARNING(x)
#endif

#include <string>

//basic singleton to hold the current top-level file being executed
class CurrentFile {
  static std::string file_name;
  CurrentFile();//DO NOT IMPLEMENT
public:
  static std::string get(){return file_name;}
  static void set(std::string fn){file_name = fn;INTERNAL_MESSAGE("Starting " << fn << std::endl);}
};

#endif
