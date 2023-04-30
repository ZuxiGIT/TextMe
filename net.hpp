#pragma once

#include <WinSock2.h>
#include <tchar.h>
#include <cstdint>

namespace net
{
    int initializeWinSock();
    void cleanupWinSock();
    int PrintSocketInfo(SOCKET sock, bool local);
    int Send(SOCKET s, const void *data, uint32_t numberOfBytes, uint32_t &bytesSent);
	int Recv(SOCKET s, void *data, uint32_t numberOfBytes, uint32_t &bytesReceived);
    int SendAll(SOCKET s, const void *data, uint32_t numberOfBytes);
    int RecvAll(SOCKET s, void *data, uint32_t numberOfBytes);
    int SendMsg(SOCKET s, const void *data, uint32_t numberOfBytes);
    int RecvMsg(SOCKET s); // , void *data);

} /* net */