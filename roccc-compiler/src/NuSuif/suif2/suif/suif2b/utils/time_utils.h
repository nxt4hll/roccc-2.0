#ifndef _TIME_UTILS_H_
#define _TIME_UTILS_H_

/** Defines Time class, objects representing time instance.
  */

#include <sys/types.h>
#include "common/MString.h"


class Time {
 private:
  time_t _time;

 public:

  /** Constructor that returns a time that represents now.
    */
  Time(void);
  
  /** Copy constructor.
    */
  Time(const Time&);

  /** Returns a human readable time string, similar to the one printed
    * by 'date'.
    */
  String to_string(void);

  /** Returns number of seconds between this and the other time instance.
    */
  int seconds_from(Time);


  /** Returns seconds after the minute.
    */
  int get_seconds(void);

  /** Returns minutes after the hour.
    */
  int get_minutes(void);
  
  /** Returns hours after midnight.
    */
  int get_hours(void);

};

#endif //_TIME_UTILS_H_
