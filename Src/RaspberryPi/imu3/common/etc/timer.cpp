
#include "../../stdafx.h"
#include "timer.h"
#include <time.h>

using namespace common;

cTimer::cTimer()
	: m_prevCounter(0.f)
{
}

cTimer::~cTimer()
{
}


bool cTimer::Create()
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);	
	m_counterStart = ts;
	return true;
}


// return seconds 
double cTimer::GetSeconds()
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);	
	int64 sec = ts.tv_sec - m_counterStart.tv_sec;
	int64 nsec = ts.tv_nsec - m_counterStart.tv_nsec;
	return double(sec) + double(nsec) / 1000000000.0;
}


// return milliseconds
double cTimer::GetMilliSeconds()
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);	
	int64 sec = ts.tv_sec - m_counterStart.tv_sec;
	int64 nsec = ts.tv_nsec - m_counterStart.tv_nsec;
	return double(sec) * 1000.0 + double(nsec) / 1000000.0;
}


// return seconds 
double cTimer::GetDeltaSeconds()
{
	const double t = GetSeconds();
	if (0.f == m_prevCounter)
	{
		m_prevCounter = t;
		return 0.0;
	}
	else
	{
		const double dt = t - m_prevCounter;
		m_prevCounter = t;
		return dt;
	}
}
