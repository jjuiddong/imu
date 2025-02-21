//
// criticalsection auto call destructor
// - 2022-06-03
//		- refactoring
//
#pragma once


class cAutoCS
{
public:
	cAutoCS(
		pthread_mutex_t &cs
		) : 
		 m_isLeave(false) 
		, m_cs(cs)
	{ 
		pthread_mutex_lock(&cs);
	}

	virtual ~cAutoCS() 
	{ 
		if (!m_isLeave)
			pthread_mutex_unlock(&m_cs);
		m_isLeave = true;
	}

	void Enter()
	{
		if (m_isLeave)
			pthread_mutex_lock(&m_cs);
		m_isLeave = false;
	}

	void Leave() 
	{ 
		pthread_mutex_unlock(&m_cs);
		m_isLeave = true;  
	}

	pthread_mutex_t &m_cs;
	bool m_isLeave;
};


namespace common
{
	/// Auto Lock, Unlock
	template<class T>
	class AutoLock
	{
	public:
		AutoLock(T& t) : m_t(t) { m_t.Lock(); }
		~AutoLock() { m_t.Unlock(); }
		T &m_t;
	};



	/// Critical Section auto initial and delete
	class CriticalSection
	{
	public:
		CriticalSection();
		~CriticalSection();
		void Lock();
		void Unlock();
	protected:
		pthread_mutex_t m_cs;
	};

	inline CriticalSection::CriticalSection() {
		m_cs = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
	}
	inline CriticalSection::~CriticalSection() {
	}
	inline void CriticalSection::Lock() {
		pthread_mutex_lock(&m_cs);
	}
	inline void CriticalSection::Unlock() {
		pthread_mutex_unlock(&m_cs);
	}


	/// auto critical section lock, unlock
	class AutoCSLock : public AutoLock<CriticalSection>
	{
	public:
		AutoCSLock(CriticalSection &cs) : AutoLock(cs) { }
	};

}
