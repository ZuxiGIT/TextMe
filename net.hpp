#pragma once

#include <WinSock2.h>
#include <tchar.h>

namespace net
{
    int initializeWinSock();
    void cleanupWinSock();
    int PrintSocketInfo(SOCKET sock, bool local);
    int Send(SOCKET s, const char *data, int numberOfBytes, int &bytesSent);
	int Recv(SOCKET s, char *data, int numberOfBytes, int &bytesReceived);
    int SendAll(SOCKET s, const char *data, int numberOfBytes);
    int RecvAll(SOCKET s, char *data, int numberOfBytes);

} /* net */