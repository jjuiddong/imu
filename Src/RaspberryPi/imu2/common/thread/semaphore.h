//
// 2019-02-06, jjuiddong
// Semaphore
//
// Reference
//	- https://github.com/preshing/cpp11-on-multicore/blob/master/common/sema.h
//
#pragma once


namespace common
{

	class cSemaphore
	{
	public:
		cSemaphore();
		virtual ~cSemaphore();
		void Wait();
		void Signal();
		void Clear();
	public:
		sem_t m_sem;
	};

	inline cSemaphore::cSemaphore() {
	}

	inline cSemaphore::~cSemaphore() {
	}

	inline void cSemaphore::Wait() {
		sem_wait(&m_sem);
	}

	inline void cSemaphore::Signal() {
		sem_post(&m_sem);
	}

	inline void cSemaphore::Clear() {
	}

}
