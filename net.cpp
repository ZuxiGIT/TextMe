#include "net.hpp"

#include <windows.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>

namespace net
{
    const uint32_t MAX_PACKET_SIZE = 4096;

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

	int Send(SOCKET s, const void *data, uint32_t numberOfBytes, uint32_t &bytesSent)
	{
		bytesSent = send(s, (const char *)data, numberOfBytes, 0);

		if (bytesSent == SOCKET_ERROR)
		{
            printf("failed to send data with error: %d\n", WSAGetLastError());
			return 1;
		}

		return 0;
	}

	int Recv(SOCKET s, void *data, uint32_t numberOfBytes, uint32_t &bytesReceived)
	{
		bytesReceived = recv(s, (char *)data, numberOfBytes, 0);

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

    int SendAll(SOCKET s, const void *data, uint32_t numberOfBytes)
	{
		uint32_t totalBytesSent = 0;

		while (totalBytesSent < numberOfBytes)
		{
			uint32_t bytesRemaining = numberOfBytes - totalBytesSent;
			uint32_t bytesSent = 0;
			const char *bufferOffset = (const char *)data + totalBytesSent;
			uint32_t result = Send(s, (const char *)bufferOffset, bytesRemaining, bytesSent);
			if (result != 0)
			{
				return 1;
			}
			totalBytesSent += bytesSent;
		}

		return 0;
	}

	int RecvAll(SOCKET s, void *data, uint32_t numberOfBytes)
	{
		uint32_t totalBytesReceived = 0;

		while (totalBytesReceived < numberOfBytes)
		{
			uint32_t bytesRemaining = numberOfBytes - totalBytesReceived;
			uint32_t bytesReceived = 0;
			char *bufferOffset = (char *)data + totalBytesReceived;
			uint32_t result = Recv(s, (char *)bufferOffset, bytesRemaining, bytesReceived);
			if (result != 0)
			{
				return 1;
			}
			totalBytesReceived += bytesReceived;
		}

		return 0;
	}

    int SendMsg(SOCKET s, const void *data, uint32_t numberOfBytes)
	{
        uint32_t netNumberOfBytes = htonl(numberOfBytes); // convert host byte order to network byte order
        int retCode = SendAll(s, (const char *)&netNumberOfBytes, sizeof(uint32_t));
        if (retCode != 0)
            return retCode;

        return SendAll(s, (const char *)data, numberOfBytes);
    }

    int RecvMsg(SOCKET s) // , void *data)
	{
        uint32_t netNumberOfBytes = 0;
        int retCode = RecvAll(s, (char *)&netNumberOfBytes, sizeof(uint32_t));
        if (retCode != 0)
            return retCode;

        uint32_t numberOfBytes = ntohl(netNumberOfBytes); // convert network byte order to host byte order
        if (numberOfBytes > MAX_PACKET_SIZE) // sanity check of buffer size
            return -1;

        char* data = new char[numberOfBytes + 1];
        data[numberOfBytes] = '\0';
        retCode = RecvAll(s, (char *)data, numberOfBytes);
        printf("[%u bytes] %s\n", numberOfBytes, data);
        delete [] data;
        return retCode;
    }
}