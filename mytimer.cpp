// mytimer.cpp

#include "mytimer.h"

namespace mylibrary
{

// static member
const char* 
MyTimer::m_monstr[] =
{
	"Jan","Feb","Mar","Apr","May","Jun",
	"Jul","Aug","Sep","Oct","Nov","Dec"
};

// static function
time_t
MyTimer::UnixTime (const std::string& timestr)
{
	if (timestr.size() < 24)
		return (time_t)(-1);

	struct tm tmtmp;

	// Month
	int m = -1;
	for (int i = 0; i < 12; i++)
	{
		if (timestr.substr(4,3) == m_monstr[i])
		{
			m = i;
			break;
		}
	}
	if (m < 0)
		return (time_t)(-1);
	tmtmp.tm_mon = m;

	int ibuf[4];

	// Day
	if (timestr[ 8] == ' ')
		ibuf[0] = 0;
	else
		ibuf[0] = (int)(timestr[ 8] - '0');
	ibuf[1] = (int)(timestr[ 9] - '0');
	tmtmp.tm_mday = ibuf[0] * 10 + ibuf[1];

	// Hour
	if (timestr[11] == ' ')
		ibuf[0] = 0;
	else
		ibuf[0] = (int)(timestr[11] - '0');
	ibuf[1] = (int)(timestr[12] - '0');
	tmtmp.tm_hour = ibuf[0] * 10 + ibuf[1];

	// Minuit
	if (timestr[14] == ' ')
		ibuf[0] = 0;
	else
		ibuf[0] = (int)(timestr[14] - '0');
	ibuf[1] = (int)(timestr[15] - '0');
	tmtmp.tm_min = ibuf[0] * 10 + ibuf[1];

	// Sec
	if (timestr[17] == ' ')
		ibuf[0] = 0;
	else
		ibuf[0] = (int)(timestr[17] - '0');
	ibuf[1] = (int)(timestr[18] - '0');
	tmtmp.tm_sec = ibuf[0] * 10 + ibuf[1];
	
	// Year
	ibuf[0] = (int)(timestr[20] - '0');
	ibuf[1] = (int)(timestr[21] - '0');
	ibuf[2] = (int)(timestr[22] - '0');
	ibuf[3] = (int)(timestr[23] - '0');
	tmtmp.tm_year = ibuf[0] * 1000 + ibuf[1] * 100 + ibuf[2] * 10 + ibuf[3] - 1900;

	// Other members
	tmtmp.tm_wday = 0;	// will be set by mktime
	tmtmp.tm_yday = 0;	// will be set by mktime
	tmtmp.tm_isdst = 0;	// will be set by mktime

	return mktime (&tmtmp);
}

std::string
MyTimer::TimeToStr (time_t t)
{
	time_t tin = t;
	char tstr[64];
	char* ptstr = asctime(localtime(&tin));
	for (int i = 0; i < 64; i++)
	{
		tstr[i] = 0;
		if ((*ptstr == '\n') || (*ptstr == 0))
			break;
		tstr[i] = *ptstr++;
	}
	std::string timstr(tstr);
	return timstr;
}

MyTimer::MyTimer()
{
	m_stop_time = m_start_time = time(0);
}

MyTimer::~MyTimer()
{
}

double
MyTimer::difftime() const
{
	return (std::difftime(m_stop_time, m_start_time));
}

std::ostream&
MyTimer::out(std::ostream& os, char sepch) const
{
	double dt = std::difftime(m_stop_time, m_start_time);

	char tstrs[64];
	char tstre[64];
	char* ptstr = asctime(localtime(&m_start_time));
	for (int i = 0; i < 64; i++)
	{
		tstrs[i] = 0;
		if ((*ptstr == '\n') || (*ptstr == 0))
			break;
		tstrs[i] = *ptstr++;
	}
	ptstr = asctime(localtime(&m_stop_time));
	for (int i = 0; i < 64; i++)
	{
		tstre[i] = 0;
		if ((*ptstr == '\n') || (*ptstr == 0))
			break;
		tstre[i] = *ptstr++;
	}

	os
		<< "localtime"
		<< sepch << "start" << sepch << tstrs
		<< sepch << "stop" << sepch << tstre
		<< sepch << "difftime(sec)" << sepch << dt;

	return os;
}

}	// namespace mylibrary
