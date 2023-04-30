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

	int Send(SOCKET s, const char *data, int numberOfBytes, int &bytesSent)
	{
		bytesSent = send(s, data, numberOfBytes, 0);

		if (bytesSent == SOCKET_ERROR)
		{
            printf("failed to send data with error: %d\n", WSAGetLastError());
			return 1;
		}

		return 0;
	}

	int Recv(SOCKET s, char *data, int numberOfBytes, int &bytesReceived)
	{
		bytesReceived = recv(s, data, numberOfBytes, 0);

		if (bytesReceived == 0)
		{
            printf("Connection closed?\n");
			return 1;
		}

		if (bytesReceived == SOCKET_ERROR)
		{
            printf("failed to recv data with error: %d\n", WSAGetLastError());
			return 1;
		}

		return 0;
	}

    int SendAll(SOCKET s, const char *data, int numberOfBytes)
	{
		int totalBytesSent = 0;

		while (totalBytesSent < numberOfBytes)
		{
			int bytesRemaining = numberOfBytes - totalBytesSent;
			int bytesSent = 0;
			const char *bufferOffset = data + totalBytesSent;
			int result = Send(s, bufferOffset, bytesRemaining, bytesSent);
			if (result != 0)
			{
				return 1;
			}
			totalBytesSent += bytesSent;
		}

		return 0;
	}

	int RecvAll(SOCKET s, char *data, int numberOfBytes)
	{
		int totalBytesReceived = 0;

		while (totalBytesReceived < numberOfBytes)
		{
			int bytesRemaining = numberOfBytes - totalBytesReceived;
			int bytesReceived = 0;
			char *bufferOffset = data + totalBytesReceived;
			int result = Recv(s, bufferOffset, bytesRemaining, bytesReceived);
			if (result != 0)
			{
				return 1;
			}
			totalBytesReceived += bytesReceived;
		}

		return 0;
	}
}