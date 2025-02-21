//
// 2019-02-06, jjuiddong
// Thread Pool Semaphore (TPSemaphore)
//
// book: IT EXPERT book example upgrade
//	- Chapter 2.2.2
//		- TPSemaphore
//
#pragma once


namespace common
{

	class cTPSemaphore
	{
	public:
		cTPSemaphore();
		virtual ~cTPSemaphore();

		bool Init(const int threadCount = -1);
		bool PushTask(cTask *task);
		cTask* PopTask();
		bool RemoveTask(const StrId &taskName);
		bool RemoveTask(const int taskId);
		void Wait();
		bool IsInit();
		void Terminate();
		void Clear();

		static int ThreadFunction(cTPSemaphore *tpSemaphore);


	public:
		CriticalSection m_cs;
		cSemaphore m_sema;
		std::deque<cTask*> m_tasks;
		vector<std::thread*> m_threads;
		bool m_isThreadLoop;
	};

}
