//
// 2019-02-06, jjuiddong
// Wait Queue Semaphore
//
// book: IT EXPERT bool example upgrade
//	- Chapter 2.2.2
//		- WQSemaphore
// 
#pragma once


namespace common
{

	class cWQSemaphore
	{
	public:
		cWQSemaphore();
		virtual ~cWQSemaphore();

		bool PushTask(cTask *task);
		cTask* PopTask();
		void Wait();
		void Terminate();
		void Clear();

		static int ThreadFunction(cWQSemaphore *wqSemaphore);


	public:
		CriticalSection m_cs;
		cSemaphore m_sema;
		queue<cTask*> m_tasks;
		std::thread m_thread;
		bool m_isThreadLoop;
	};

}
