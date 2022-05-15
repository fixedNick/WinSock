#pragma once
#include <afxsock.h>
#include <string>
#include <vector>
using namespace std;

inline string ReceiveString(CSocket& s)
{
	int nLength;
	s.Receive(&nLength, sizeof(int));
	vector<char> response(nLength);
	s.Receive(&response[0], nLength);
	return string(&response[0], nLength);
}

inline void SendString(CSocket& s, const string& str)
{
	int nLength = str.length();
	s.Send(&nLength, sizeof(int));
	s.Send(str.c_str(), nLength);
}