//	This include file is provided as a place to put system specific
//	declarations, etc, that cannot be handled simply as -D's in
//	the Makefile.
//
//	You should always use the makefile in preference to this file as
//	a source of system dependent modification
//
//	Anything you put in here should be ifdefed by controlling variables
//
//	All Suif system implementation modules (cpp files) include this as
//	the first thing (before any other includes)
#ifndef __SYSTEM_SPECIFIC_H__
#define __SYSTEM_SPECIFIC_H_

#if (defined(_MBCS) && !defined(MSVC))
#error "If you're compiling the system under Microsoft \
    Visual C++, MSVC must be defined"
#endif 

#ifdef LONG_LONG_ALLOWED
#   ifdef MSVC
#       undef LONGLONG
#   else
#       define LONGLONG long long
#   endif
#endif


//	If you turn on TRACK_ALLOC and use suif_malloc.cpp you can
//	look for memory leaks. You need to put NOTE_FILE_AND_LINE
//	in front of every new you are interested in.

// #define TRACK_ALLOC
// void set_file_and_line(const char *file,int line);
// #define NOTE_FILE_AND_LINE set_file_and_line(__FILE__,__LINE__)

#define NOTE_FILE_AND_LINE

//	MSC requires some extra non-standard declarations on
//	variables.
#ifdef MSVC
#undef LONGLONG
#define DLLEXPORT __declspec( dllexport )
#define DLLIMPORT __declspec( dllimport )
#pragma warning( disable : 4018 )
#pragma warning( disable : 4390 )
#pragma warning( disable : 4068 )       // complains "unknown pragma"
#ifndef NO_COVARIANCE
#define NO_COVARIANCE
#endif
#ifndef SUPPRESS_PRAGMA_INTERFACE
#define SUPPRESS_PRAGMA_INTERFACE
#endif
#else
#define DLLEXPORT
#define DLLIMPORT
#endif /* MSVC */

#ifdef MONTANA
#define EXPORT _Export
#else
#define EXPORT
#endif


//	The following definition conceivably belongs elsewhere but this
//	is a convenient place for it.

#define BITSPERBYTE 8



#endif
