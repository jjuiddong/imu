
#include "../../stdafx.h"
#include "thread1.h"
#include "task.h"
#include <boost/bind.hpp>
#include <chrono>
// #include <MMSystem.h>


namespace common
{
	unsigned ThreadProcess(void *pThreadPtr )
	{
		// CoInitialize(NULL); // for COM interface

		cThread *pThread = (cThread*)pThreadPtr;
		pThread->Run();
		pThread->Exit();
		return 0;
	}

	template<class T>
	static bool IsSameId(T *p, int id)
	{
		if (!p) return false;
		return p->m_id == id;
	}
}
using namespace common;


cThread::cThread(const StrId &name
	, const int maxTask //= -1
	, iMemoryPool3Destructor *memPool //= NULL
	, const uint maxProcTaskSize //= 5
	, const int sleepMillis //= 1
) 
	: m_state(eState::WAIT)
	, m_name(name)
	, m_procTaskIndex(0)
	, m_maxTask(maxTask)
	, m_memPoolTask(memPool)
	, m_sortObj(NULL)
	, m_maxProcTaskSize(maxProcTaskSize)
	, m_sleepMillis(sleepMillis)
{
	m_tasks.reserve(32);
}

cThread::~cThread()
{
	Clear();
}


//------------------------------------------------------------------------
//  ������ ����
//------------------------------------------------------------------------
void cThread::Start()
{
	if ((eState::WAIT == m_state) || (eState::END == m_state))
	{
		if (eState::END != m_state)
		{
			Terminate(INFINITE);
		}
		else
		{
			if (m_thread.joinable())
				m_thread.join();
		}

		m_state = eState::RUN;
		m_thread = std::thread(ThreadProcess, this);
	}
}


// �����带 �Ͻ� ���� ��Ų��.
bool cThread::Pause()
{
	if (eState::RUN == m_state)
	{
		m_taskCS.Lock();
		m_state = eState::PAUSE;
		return true;
	}
	return false;
}


// �����带 �� �����Ѵ�. Pause �����϶��� �����ϴ�.
bool cThread::Resume()
{
	if ((eState::PAUSE == m_state) || (eState::IDLE == m_state))
	{
		m_taskCS.Unlock();
		m_state = eState::RUN;
		return true;
	}
	return false;
}


//------------------------------------------------------------------------
// ������ ����
//------------------------------------------------------------------------
bool cThread::Terminate(const int milliSeconds) //milliSeconds = -1
{
	if (!m_thread.joinable())
	{
		// Already Terminate
		m_state = eState::END;
		return false;
	}

	m_state = eState::END;
	if (m_thread.joinable())
		m_thread.join();
	return true;
}


//------------------------------------------------------------------------
// �����忡�� �޼����� ������.
// rcvTaskId : ���� �½�ũ ���̵� ('0' �̶�� �����尡 �޴´�.)
//			   -1 : �ܺη� ���� �޼����� ����
//------------------------------------------------------------------------
void cThread::Send2ThreadMessage( threadmsg::MSG msg, WPARAM wParam, LPARAM lParam, LPARAM added)
{
	AutoCSLock cs(m_msgCS);
	m_threadMsgs.push_back( SExternalMsg(-1, (int)msg, wParam, lParam, added) );
}


//------------------------------------------------------------------------
// 
//------------------------------------------------------------------------
void cThread::Send2ExternalMessage( int msg, WPARAM wParam, LPARAM lParam, LPARAM added )
{
	AutoCSLock cs(m_msgCS);
	m_externalMsgs.push_back( SExternalMsg(-1, msg, wParam, lParam, added) );
}


//------------------------------------------------------------------------
// �����尡 ���� �޼����� �����Ѵ�.
// �޼����� ���ٸ� false�� �����Ѵ�.
//------------------------------------------------------------------------
bool cThread::GetThreadMsg( OUT SExternalMsg *out
	, eMessageOption::Enum opt  // = eMessageOption::REMOVE
)
{
	if (!out)
		return false;

	bool reval;
	{
		AutoCSLock cs(m_msgCS);
		if (m_threadMsgs.empty())
		{
			reval = false;
		}
		else
		{
			*out = m_threadMsgs.front();
			if (eMessageOption::REMOVE == opt)
				m_threadMsgs.pop_front();
			reval = true;
		}
	}
	return reval;
}


//------------------------------------------------------------------------
// �����忡�� �ܺη� ���ϴ� �޼����� �����Ѵ�.
// �޼����� ���ٸ� false�� �����Ѵ�.
//------------------------------------------------------------------------
bool cThread::GetExternalMsg( OUT SExternalMsg *out
	, eMessageOption::Enum opt // = eMessageOption::REMOVE
) 
{
	if (!out)
		return false;

	bool reval;
	{
		AutoCSLock cs(m_msgCS);
		if (m_externalMsgs.empty())
		{
			reval = false;
		}
		else
		{
			*out = m_externalMsgs.front();
			if (eMessageOption::REMOVE == opt)
				m_externalMsgs.pop_front();
			reval = true;
		}
	}
	return reval;
}


// m_tasks, m_addTasks sorting
// ���ڷ� �Ѿ�� compareFn �� �������� ������ (new) ��ü���� �Ѵ�.
// ������ ������ compareFn ��ü�� ���ŵȴ�. (delete)
// ��Ƽ ���������� �����ϱ� ������, ����ȭ�� ���� �̷��� ó���ߴ�.
void cThread::SortTasks(iSortingTasks *sortObj)
{	
	AutoCSLock cs(m_containerCS);

	SAFE_DELETE(m_sortObj);
	m_sortObj = sortObj;
}


//------------------------------------------------------------------------
// 
//------------------------------------------------------------------------
bool cThread::AddTask(cTask *task)
{
	RETV(!task, false);

	AutoCSLock cs(m_containerCS);
	task->m_pThread = this;
	m_addTasks.push_back( task );
	return true;
}


//------------------------------------------------------------------------
// 
//------------------------------------------------------------------------
bool cThread::RemoveTask(const int id)
{		
	AutoCSLock cs(m_containerCS);
	m_removeTasks.push_back(id);
	return true;
}


int cThread::GetTaskCount()
{
	return m_tasks.size();
}


/**
@brief  taskId�� �ش��ϴ� task�� ����.
*/
cTask*	cThread::GetTask(const int taskId)
{
	//AutoCSLock cs(m_taskCS); ����ȭ ���� �ʴ´�.
	auto it = find_if(m_tasks.begin(), m_tasks.end(), IsTask(taskId));
	if (m_tasks.end() == it)
		return NULL; // ���ٸ� ����
	return *it;
}


//------------------------------------------------------------------------
// ������ ����
// Task�� �����Ų��.
//------------------------------------------------------------------------
int cThread::Run()
{
	// using namespace std::chrono_literals;

	// cTimer timer;
	// timer.Create();

	// int delayTerminateCount = 0;

	// while ((eState::RUN == m_state)
	// 	|| (eState::PAUSE == m_state))
	// {
	// 	m_taskCS.Lock();

	// 	//1. Add & Remove Task
	// 	UpdateTask();

	// 	if (m_tasks.empty()) // break no task
	// 	{
	// 		// 5�� ���� ��ٸ��ٰ� �����带 �����Ѵ�.
	// 		if (delayTerminateCount++ > 5)
	// 			break;
	// 		std::this_thread::sleep_for(1000ms);
	// 	}

	// 	//2. Task Process
	// 	if ((eState::RUN == m_state) && !m_tasks.empty())
	// 	{
	// 		delayTerminateCount = 0;

	// 		if ((int)m_tasks.size() <= m_procTaskIndex)
	// 			m_procTaskIndex = 0;

	// 		const double dt = (float)timer.GetDeltaSeconds();
	// 		int taskProcCnt = 0;

	// 		do
	// 		{
	// 			++taskProcCnt;
	// 			cTask *task = m_tasks[m_procTaskIndex];
	// 			if (cTask::eRunResult::End == task->Run(dt))
	// 			{
	// 				// finish task , remove taks
	// 				//common::rotatepopvector(m_tasks, m_procTaskIndex);
	// 				common::removevector2(m_tasks, m_procTaskIndex);
	// 				ReleaseTask(task);
	// 			}
	// 			else
	// 			{
	// 				++m_procTaskIndex;
	// 			}

	// 		} while (((eState::RUN == m_state)
	// 			//|| (eState::PAUSE == m_state)
	// 			)
	// 			&& (m_procTaskIndex < (int)m_tasks.size())
	// 			&& ((uint)taskProcCnt < m_maxProcTaskSize)
	// 			);
	// 	}

	// 	//3. Message Process
	// 	DispatchMessage();

	// 	m_taskCS.Unlock();

	// 	if (m_sleepMillis >= 0)
	// 		std::this_thread::sleep_for(std::chrono::milliseconds(m_sleepMillis));
	// }

	// m_taskCS.Unlock();

	// // ���������� �� �޼����� ���������� ó���Ѵ�.
	// DispatchMessage();

	return 1;
}


//------------------------------------------------------------------------
// call exit thread
//------------------------------------------------------------------------
void cThread::Exit()
{
	m_state = eState::END;
}


void cThread::UpdateTask()
{
	AutoCSLock cs1(m_containerCS);

	// Sorting
	if (m_sortObj)
	{
		m_sortObj->Sorting(m_tasks);
		m_sortObj->Sorting(m_addTasks);
		SAFE_DELETE(m_sortObj);
	}

	for (auto &p : m_addTasks)
	{
		auto it = find_if(m_tasks.begin(), m_tasks.end(), IsTask(p->m_id));
		if (m_tasks.end() == it) // not exist
		{
			if (m_maxTask < 0) // infinity
			{
				if (p->m_isTopPriority)
				{
					m_tasks.push_back(p);
					common::rotateright(m_tasks); // ���������� ȸ������, �߰��Ѱ��� ���� ������ �����Ѵ�.
				}
				else
				{
					m_tasks.push_back(p);
				}
			}
			else
			{
				if ((int)m_tasks.size() >= m_maxTask)
				{
					dbg::Logp("Err Over Maximize Task Count\n");

					// remove most old task
					auto &task = m_tasks.back();
					ReleaseTask(task);
					m_tasks.pop_back();
				}

				if (p->m_isTopPriority)
				{
					m_tasks.push_back(p);
					common::rotateright(m_tasks); // ���������� ȸ������, �߰��Ѱ��� ���� ������ �����Ѵ�.
				}
				else
				{
					m_tasks.push_back(p);
				}
			}
		}
		else
		{
			assert(0); // already exist
		}
	}
	m_addTasks.clear();

	for (auto &id : m_removeTasks)
	{
		auto it = find_if(m_tasks.begin(), m_tasks.end(), IsTask(id));
		if (m_tasks.end() != it)
		{
			cTask *p = *it;
			m_tasks.erase(it);
			ReleaseTask(p);
		}
	}
	m_removeTasks.clear();
}


void cThread::ReleaseTask(cTask *task)
{
	RET(!task);

	if (m_memPoolTask)
	{
		task->Clear();
		m_memPoolTask->Free(task);
	}
	else
	{
		delete task;
	}
}


//------------------------------------------------------------------------
// ����� �޼������� �½�ũ�� ������.
//------------------------------------------------------------------------
void cThread::DispatchMessage()
{
	AutoCSLock cs(m_msgCS);
	auto it = m_threadMsgs.begin();
	while (m_threadMsgs.end() != it)
	{
		if (threadmsg::TASK_MSG == it->msg) // task message
		{
			auto t = find_if(m_tasks.begin(), m_tasks.end(), 
				boost::bind( &IsSameId<cTask>, _1, it->wParam) );
			if (m_tasks.end() != t)
			{
				(*t)->MessageProc((threadmsg::MSG)it->msg, it->wParam, it->lParam, it->added);
			}
			else
			{
				dbg::ErrLog("cThread::DispatchMessage Not Find Target Task\n");
			}
		}
		else // Thread���� �� �޼���
		{
			MessageProc((threadmsg::MSG)it->msg, it->wParam, it->lParam, it->added);
		}
		++it;
	}
	m_threadMsgs.clear();
}


//------------------------------------------------------------------------
// Message Process
//------------------------------------------------------------------------
void cThread::MessageProc( threadmsg::MSG msg, WPARAM wParam, LPARAM lParam, LPARAM added )
{
	switch (msg)
	{
	case threadmsg::TERMINATE_TASK:
		{
			// terminate task of id wParam
			auto it = std::find_if( m_tasks.begin(), m_tasks.end(), 
				bind( &IsSameId<common::cTask>, _1, (int)wParam) );
			if (m_tasks.end() != it)
			{
				ReleaseTask(*it);
				m_tasks.erase(it);
			}
		}
		break;
	}
}


bool cThread::IsRun()
{
	switch (m_state)
	{
	case eState::RUN:
	case eState::PAUSE:
	case eState::IDLE:
		return true;
	}
	return false;
}


void cThread::Join()
{
	if (IsRun() && m_thread.joinable())
		m_thread.join();
}


//------------------------------------------------------------------------
// ������ ����, �������� ������ ���� ����
//------------------------------------------------------------------------
void cThread::Clear()
{
	if (!Terminate(INFINITE))
		return; // already terminate

	{
		AutoCSLock cs(m_containerCS);
		for (auto &p : m_addTasks)
			ReleaseTask(p);
		SAFE_DELETE(m_sortObj);
		m_addTasks.clear();
	}

	auto it = m_tasks.begin();
	while (m_tasks.end() != it)
	{
		cTask *p = *it++;
		ReleaseTask(p);
	}
	m_tasks.clear();

	{
		AutoCSLock cs2(m_msgCS);
		m_threadMsgs.clear();
		m_externalMsgs.clear();
	}
}
