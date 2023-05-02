#pragma once

#include <WinSock2.h>
#include <tchar.h>
#include <cstdint>
#include <vector>

namespace net
{
    int initializeWinSock();
    void cleanupWinSock();
    int PrintSocketInfo(SOCKET sock, bool local);
    int Send(SOCKET s, const BYTE* data, DWORD numberOfBytes, INT &bytesSent);
	int Recv(SOCKET s, PBYTE data, DWORD numberOfBytes, INT& bytesReceived);
    int SendAll(SOCKET s, const BYTE* data, DWORD numberOfBytes);
    int RecvAll(SOCKET s, PBYTE data, DWORD numberOfBytes);
    int SendMsg(SOCKET s, const std::vector<BYTE> &data);
    int RecvMsg(SOCKET s, std::vector<BYTE> &data);    
} /* net */