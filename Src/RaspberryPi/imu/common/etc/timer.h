//
// 2017-11-01, jjuiddong
// QueryPerformanceCounter Timer
//
#pragma once


namespace common
{

	class cTimer
	{
	public:
		cTimer();
		virtual ~cTimer();

		bool Create();
		double GetSeconds();
		double GetMilliSeconds();
		double GetDeltaSeconds();


	public:
		double m_prevCounter;
		struct timespec m_counterStart;
	};

}
