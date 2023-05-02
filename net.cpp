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
            printf("Failed to retrieve address from socket with error: %d\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        CHAR pchrIpAddress[INET_ADDRSTRLEN] = {};
        if(!inet_ntop(AF_INET, &Address.sin_addr, pchrIpAddress, INET_ADDRSTRLEN))
        {
            printf("Address translation failed\n");
            WSACleanup();
            return 1;
        }

        printf("Ip address: %s port: %d\n", 
               pchrIpAddress, 
               ntohs(Address.sin_port));

        return 0;
    }

	int Send(SOCKET s, const BYTE* data, DWORD numberOfBytes, DWORD &bytesSent)
	{
		bytesSent = send(s, reinterpret_cast<const CHAR*>(data), numberOfBytes, 0);

		if (bytesSent == static_cast<DWORD>(SOCKET_ERROR))
		{
            printf("Failed to send data with error: %d\n", WSAGetLastError());
			return 1;
		}

		return 0;
	}

	int Recv(SOCKET s, PBYTE data, DWORD numberOfBytes, DWORD& bytesReceived)
	{
		bytesReceived = recv(s, reinterpret_cast<CHAR*>(data), numberOfBytes, 0);

		if (bytesReceived == 0)
		{
            printf("Connection closed?\n");
			return 1;
		}

		if (bytesReceived == static_cast<DWORD>(SOCKET_ERROR))
		{
            printf("Failed to recv data with error: %d\n", WSAGetLastError());
			return 1;
		}

		return 0;
	}

    int SendAll(SOCKET s, const BYTE* data, DWORD numberOfBytes)
	{
		DWORD totalBytesSent = 0;

		while (totalBytesSent < numberOfBytes)
		{
			DWORD bytesRemaining = numberOfBytes - totalBytesSent;
			DWORD bytesSent = 0;
			const BYTE*  bufferOffset = data + totalBytesSent;
			DWORD result = Send(s, bufferOffset, bytesRemaining, bytesSent);
			if (result != 0)
			{
				return 1;
			}
			totalBytesSent += bytesSent;
		}

		return 0;
	}

	int RecvAll(SOCKET s, PBYTE data, DWORD numberOfBytes)
	{
		DWORD totalBytesReceived = 0;

		while (totalBytesReceived < numberOfBytes)
		{
			DWORD bytesRemaining = numberOfBytes - totalBytesReceived;
			DWORD bytesReceived = 0;
			BYTE* bufferOffset = data + totalBytesReceived;
			int result = Recv(s, bufferOffset, bytesRemaining, bytesReceived);
			if (result != 0)
			{
				return 1;
			}
			totalBytesReceived += bytesReceived;
		}

		return 0;
	}

    int SendMsg(SOCKET s, const std::vector<BYTE> &data)
	{
        DWORD netNumberOfBytes = htonl(data.size()); // convert host byte order to network byte order
        int retCode = SendAll(s, reinterpret_cast<PBYTE>(&netNumberOfBytes), sizeof(DWORD));
        if (retCode != 0)
            return retCode;

        return SendAll(s, data.data(), data.size());
    }

    int RecvMsg(SOCKET s, std::vector<BYTE> &data)
	{
        data.clear();

        DWORD netNumberOfBytes = 0;
        int retCode = RecvAll(s, reinterpret_cast<PBYTE>(&netNumberOfBytes), sizeof(DWORD));
        if (retCode != 0)
            return retCode;

        DWORD numberOfBytes = ntohl(netNumberOfBytes); // convert network byte order to host byte order
        if (numberOfBytes > MAX_PACKET_SIZE) // sanity check of buffer size
            return -1;

        data.resize(numberOfBytes);
        retCode = RecvAll(s, data.data(), numberOfBytes);
        return retCode;
    }
}