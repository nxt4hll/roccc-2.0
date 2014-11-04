
#include "time_utils.h"
#include <time.h>

Time::Time(void) :
  _time(time(0))
{}


Time::Time(const Time& t) :
  _time(t._time)
{}


String Time::to_string(void)
{
  String res(ctime(&_time));
  res.truncate_at_pos(res.length()-1);
  return res;
}


int Time::seconds_from(Time t)
{
  return (_time - t._time);
}


int Time::get_seconds(void)
{
  struct tm * tmp = gmtime(&_time);
  return tmp->tm_sec;
}

int Time::get_minutes(void)
{
  struct tm * tmp = gmtime(&_time);
  return tmp->tm_min;
}

/*
 * On Solaris 2.6, this returns the day of the month.  Gdb shows that the
 * struct tm returns contains identicle values at tm_hour and tm_mday.
 * Probably a bug in gmtime.
 */
int Time::get_hours(void)
{
  struct tm * tmp = gmtime(&_time);
  return tmp->tm_hour;
}
