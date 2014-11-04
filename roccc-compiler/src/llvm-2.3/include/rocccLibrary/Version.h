// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*  --*- C++ -*--
 
 File: Version.h
 
 This contains the version number, both major and minor, along with any other versioning information.
 
 */

#ifndef _VERSION_H__
#define _VERSION_H__

/* source code version information */
#define ROCCC_VERSION_MAJOR    "0"
#define ROCCC_VERSION_MINOR    "7"
#define ROCCC_VERSION_BUGFIX   "6"
#define ROCCC_VERSION_INTERNAL "0"
#define ROCCC_VERSION_DATE    "Tuesday, February 19, 2013"
  
#define ROCCC_VERSION_NUM     ROCCC_VERSION_MAJOR "." ROCCC_VERSION_MINOR "." ROCCC_VERSION_BUGFIX "." ROCCC_VERSION_INTERNAL
#define ROCCC_VERSION         "ROCCC -- version " ROCCC_VERSION_NUM \
                              ", released on " ROCCC_VERSION_DATE
  
#endif
