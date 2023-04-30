#include "net.hpp"

#include <windows.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>

namespace net
{
    int initializeWinSock()
    {
        WSADATA wsaData;
        int iResult;

        // Initialize Winsock
        iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
        if (iResult != 0) {
            printf("WSAStartup failed: %d\n", iResult);
            return 1;
        }
        return iResult;
    }

    void cleanupWinSock()
    {
        WSACleanup();
    }

    int PrintSocketInfo(SOCKET sock, bool local)
    {
        sockaddr_in Address;
        int iAddressSize = sizeof(Address);

        int iResult = local ? getsockname(sock, (sockaddr*)&Address, &iAddressSize) : 
                            getpeername(sock, (sockaddr*)&Address, &iAddressSize);

        if(iResult == SOCKET_ERROR)
        {
            printf("failed to retrieve address from socket with error: %d\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        CHAR pchrIpAddress[INET_ADDRSTRLEN] = {};
        if(!inet_ntop(AF_INET, &Address.sin_addr, pchrIpAddress, INET_ADDRSTRLEN))
        {
            printf("->address translation failed\n");
            WSACleanup();
            return 1;
        }

        printf("->ip address: %s port: %d\n", 
            pchrIpAddress, 
            ntohs(Address.sin_port));
        
        return 0;
    }
}