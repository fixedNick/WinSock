// sock.cpp : Defines the initialization routines for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "sock.h"
#include <afxsock.h>
#include <fstream>
#include <string>
#include <vector>
#include <mutex>
#include <atlconv.h> // bstr

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CsockApp, CWinApp)
END_MESSAGE_MAP()

CsockApp::CsockApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

CsockApp theApp;


BOOL CsockApp::InitInstance()
{
	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	return TRUE;
}


using namespace std;
CSocket client;
SOCKET clientSockHandle;

inline string ReceiveString(CSocket& s)
{
	int nLength;
	s.Receive(&nLength, sizeof(int));
	if (nLength < 0) return "";
	vector<char> response(nLength);
	s.Receive(&response[0], nLength);

	string resp = "";
	for (int i = 0; i < response.size(); i++)
		resp += response[i];

	return resp;
}

inline void SendString(CSocket& s, const string& str)
{
	int nLength = str.length();
	s.Send(&nLength, sizeof(int));
	s.Send(str.c_str(), nLength);
}

BSTR ConvertStringToBStr(char* str)
{
	int wslen = MultiByteToWideChar(CP_ACP, 0, str, strlen(str), 0, 0);
	BSTR bstr = SysAllocStringLen(0, wslen);
	MultiByteToWideChar(CP_ACP, 0, str, strlen(str), bstr, wslen);
	// Use bstr here
	SysFreeString(bstr);
	return bstr;
}

/// ���� ������/�������������:
/// -111: ������ ����������� �� �������

/// -222: �� ������� ������������ � NamePipe'�
string SendMessageToServer(string message)
{
	SendString(client, message);
	return ReceiveString(client);
}

// ��� ����, ���������������� ��� �������� ����������� � ������ ConnectToNamedPipe

extern "C"
{
	_declspec(dllexport) void _stdcall SendStringToSocket(CSocket& s, const string& str)
	{
		SendString(s, str);
	}

	_declspec(dllexport) string _stdcall ReceiveStringFromSocket(CSocket& s)
	{
		return ReceiveString(s);
	}

	_declspec(dllexport) int _stdcall ConnectToServerViaWSockets()
	{
		AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0);
		AfxSocketInit();

		client.Create();
		clientSockHandle = client.m_hSocket;

		if (!client.Connect("127.0.0.1", 12345)) {
			client.Close();
			closesocket(client.m_hSocket);
			return -222;
		}

		return atoi(SendMessageToServer("get_active_threads_count").c_str());
	}

	_declspec(dllexport) void _stdcall ClientDisconnect()
	{
		SendString(client, "quit");
		client.Close();
		closesocket(clientSockHandle);
	}
	/*� �������� ������������� �������� ��������� BStr,
	��� ����� ���������� ���������� ����� ����������� �� char* � bstr
	�����: ConvertStringToBStr(char* str)*/
	_declspec(dllexport) void* _stdcall SendTextViaWSockets(char* fileText, int tidx)
	{
		// ��������� ������� ��� �������, �� ������������ ��������
		// ����� �������: <�������>:<��������1>:<��������N>
		// ������ ��� ������ �������: <�������>:<����� ������>:<����� ��� ������>
		string request = "send_message:" + to_string(tidx) + ":" + fileText;

		// �������� �������
		string response = SendMessageToServer(request);

		// ���������� ������� ���������������� � BStr �����.
		return ConvertStringToBStr((char*)response.c_str());
	}

	_declspec(dllexport) int _stdcall StartServerThread(int threadsCount)
	{
		string req = "threads_start:" + to_string(threadsCount);

		// ��������� N �������
		string resp = SendMessageToServer(req);
		//int activeThreadsCount = atoi(resp);

		return atoi((char*)resp.c_str());
	}

	_declspec(dllexport) int _stdcall StopServerThread(bool stopServer)
	{
		string req = "thread_stop";
		string resp = SendMessageToServer(req);
		//int activeThreadsCount = atoi(resp);

		return atoi((char*)resp.c_str());
	}
	std::mutex mtx;

	_declspec(dllexport) void __cdecl WriteToFile(int thread_idx, char* str)
	{
		mtx.lock();
		std::ofstream vmdelet_out;                    //������� ����� 
		vmdelet_out.open(std::to_string(thread_idx) + ".txt", std::ios::app);  // ��������� ���� ��� ������ � �����
		vmdelet_out << str;                        // ���� ������
		vmdelet_out.close();                          // ��������� ����
		mtx.unlock();
	}
}