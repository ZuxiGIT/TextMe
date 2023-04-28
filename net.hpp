#pragma once

#include <WinSock2.h>
#include <tchar.h>

namespace net
{
    int initializeWinSock();
    void cleanupWinSock();
    int PrintSocketInfo(SOCKET sock, bool local);

} /* net */