#include "net.hpp"
#include "crypto.hpp"

#include <windows.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#define STR(x) #x
#define MYTEXT(x)  "[Client] %s [%d] -> " x, __FUNCTION__, __LINE__

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
        printf(MYTEXT("Getaddrinfo failed: %d\n"), iResult);
        return INVALID_SOCKET;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr = result; ptr != NULL; ptr = ptr->ai_next) 
    {
        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        
        if (ConnectSocket == INVALID_SOCKET) 
        {
            printf(MYTEXT("Socket failed with error: %d\n"), WSAGetLastError());
            WSACleanup();
            return INVALID_SOCKET;
        }

        printf(MYTEXT("Connecting to %s\n"), pcszServerName);
        
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

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) 
    {
        printf(MYTEXT("Failed to connect to \"%s\"!\n"), pcszServerName);
        return INVALID_SOCKET;
    }
    
    return ConnectSocket;
}

HCRYPTKEY exchangeKeys(SOCKET s, HCRYPTPROV hCryptoProv) 
{    
    std::vector<BYTE> pubKey;
    
    net::RecvMsg(s, pubKey);
    printf(MYTEXT("Received public key from server\n"));

    printf(MYTEXT("Public Key Blob size is %zu\n"), pubKey.size());
    printBytes("Public Key Blob:\n", pubKey.data(), pubKey.size());

    HCRYPTKEY hImpPubKey = crypto::importKey(hCryptoProv, 0, pubKey.data(), pubKey.size());

    HCRYPTKEY hSesKey = crypto::getSessionKey(hCryptoProv);
    BYTE* pbBlob = NULL;

    printf(MYTEXT("Encrypting session key\n"));
    DWORD dwBlobSize = crypto::exportKey(hSesKey, hImpPubKey, SIMPLEBLOB, &pbBlob);

    std::vector<BYTE> sesKey (pbBlob, pbBlob + dwBlobSize);

    printf(MYTEXT("Session Key Blob size is %zu\n"), sesKey.size());
    printBytes("Session Key Blob:\n", sesKey.data(), sesKey.size());
    
    printf(MYTEXT("Sending session key to server...\n"));
    net::SendMsg(s, sesKey);
    printf(MYTEXT("Sent session key\n"));

    crypto::destroyKey(hImpPubKey);

    return hSesKey;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Wrong parameters\n");
        fprintf(stderr, "Usage: client.exe <server address> <server port>\n");
        return -1;
    }

    printf(MYTEXT("Client program\n"));
    printf(MYTEXT("\tServer address: %s\n"), argv[1]);
    printf(MYTEXT("\tServer port: %d\n"), atoi(argv[2]));

    printf(MYTEXT("Initialising winsock library...\n"));
    net::initializeWinSock();
    
    printf(MYTEXT("Creating socket and connecting to server...\n"));
    SOCKET sock = createSocketAndConnect(argv[1], argv[2]);
    printf(MYTEXT("Connected to %s\n"), argv[1]);
    net::PrintSocketInfo(sock, false);

    printf(MYTEXT("Getting keys from CSP [\"Key Containter for Encoding/Decoding\"]...\n"));
    
    HCRYPTPROV hCryptoProv  = crypto::getCryptoProv(TEXT("Key Containter for Encoding/Decoding"), crypto::ProvType::RSA);
    
    printf(MYTEXT("Generating and sending session key to the server...\n"));
    HCRYPTKEY hSesKey = exchangeKeys(sock, hCryptoProv);
    printf(MYTEXT("Keys have been exchanged\n"));

    for(;;)
    {
        printf("Enter message: ");
        std::string data;
        std::getline(std::cin, data);
        if (data.size() == 0)
            continue;
        if (data == "exit")
            break;
        printf("\n");
        std::vector<BYTE> rawData;
        rawData.resize(data.size());
        memcpy(&rawData[0], &data[0], data.size());
        crypto::encryptData(hSesKey, &rawData[0], rawData.size());
        net::SendMsg(sock, rawData);
        printBytes("\nSent raw data: ", &rawData[0], rawData.size());

        printf("\nWaiting for message from server...\n\n");

        net::RecvMsg(sock, rawData);
        printBytes("\nReceived raw data: ", &rawData[0], rawData.size());
        crypto::decryptData(hSesKey, &rawData[0], rawData.size());
        data.resize(rawData.size());
        memcpy(&data[0], &rawData[0], rawData.size());
        printf(MYTEXT("\nReceived message: %s\n\n"), data.c_str());
    }

    crypto::destroyKey(hSesKey);
    crypto::releaseCryptoProv(hCryptoProv);

    shutdown(sock, SD_BOTH);
    closesocket(sock);
    net::cleanupWinSock();
    return 0;
}
