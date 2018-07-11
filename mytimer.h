// mytimer.h
#ifndef MYTIMER_H_INCLUDED
#define MYTIMER_H_INCLUDED

#include <iostream>
#include <string>
#include <ctime>

// 2017-02-21 TimeToStr () function is added as a static member function.
// 2017-02-21 UnixTime () function is added as a static member function.
//            This function converts Time-string to time_t.

namespace mylibrary
{

class MyTimer
{
public:
	static time_t UnixTime (const std::string& timestr);
	static std::string TimeToStr (time_t t);

public:
	MyTimer();
	virtual ~MyTimer();
	time_t start() { return (m_stop_time = m_start_time = time(0)); }
	time_t stop() { return (m_stop_time = time(0)); }
	time_t start_time() const { return m_start_time; }
	time_t stop_time() const { return m_stop_time; }
	double difftime() const;
	std::ostream& out(std::ostream& os, char sepch) const;
	std::ostream& csvout(std::ostream& os) const { return (out (os, char(','))); }
	std::ostream& txtout(std::ostream& os) const { return (out (os, char(' '))); }

private:
	static const char* m_monstr [12];

protected:
	time_t m_start_time;
	time_t m_stop_time;
};
	
}	// namespace mylibrary

#endif	// MYTIMER_H_INCLUDED
