#include "net.hpp"
#include "crypto.hpp"

#include <windows.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <iostream>

SOCKET createSocketAndConnect(PCSTR pcszServerName, PCSTR pcszServiceName)
{
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    
    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    SOCKET ConnectSocket = INVALID_SOCKET;

    int iResult = 0;
    // Resolve the server address and port
    iResult = getaddrinfo(pcszServerName, pcszServiceName, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        return INVALID_SOCKET;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) 
    {
        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) 
        {
            printf("socket failed with error: %d\n", WSAGetLastError());
            WSACleanup();
            return INVALID_SOCKET;
        }

        printf("connecting to %s\n", pcszServerName);
        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) 
        {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }

        break;
    }
    printf("conneced to %s\n", pcszServerName);


    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) 
    {
        printf("Unable to connect to \"%s\"!\n", pcszServerName);
        return INVALID_SOCKET;
    }

    return ConnectSocket;
}

int main(void)
{
    std::ios::sync_with_stdio(true);

    printf("Enter address: ");
    std::string address;
    std::cin >> address;
    printf("Enter port: ");
    std::string port;
    std::cin >> port;
    net::initializeWinSock();
    SOCKET sock = createSocketAndConnect(address.c_str(), port.c_str());
    const char data[16] = "Please, work...";
    net::SendMsg(sock, data, 16);
    shutdown(sock, SD_BOTH);
    closesocket(sock);
    net::cleanupWinSock();
    return 0;
}
