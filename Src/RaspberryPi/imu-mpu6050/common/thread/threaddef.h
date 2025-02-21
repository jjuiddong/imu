//
// 2016-06-04, jjuiddong
//
//
#pragma once


namespace common
{

	class cTask;

	// ������� �������� �ְ� �޴� �޼��� ����
	struct SExternalMsg
	{
		int rcvTaskId; // �޼����� ���� Task ID (0=tread, -1=�ܺο��� �޴� �޼���)
		int msg;
		WPARAM wParam;
		LPARAM lParam;
		LPARAM added;

		SExternalMsg() {}
		SExternalMsg(int rcvtaskId, int msgtype, WPARAM wparam, LPARAM lparam, LPARAM _added) :
			rcvTaskId(rcvtaskId), msg(msgtype), wParam(wparam), lParam(lparam), added(_added)
		{
		}
	};

	
	namespace threadmsg
	{
		enum MSG
		{
			TASK_MSG = 100,// task message
				 		   // wParam : taskId
			TERMINATE_TASK,
						  // wParam : taskId
			MSG_LAST
		};
	
	}
}
