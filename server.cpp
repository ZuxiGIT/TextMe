#include "net.hpp"
#include "crypto.hpp"

#include <windows.h>
#include <stdio.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#define STR(x) #x
#define MYTEXT(x)  "[Server] %s [%d] " x, __FUNCTION__, __LINE__

DWORD HandleConnection(SOCKET ClientSocket);

SOCKET createSocketAndListen(PCSTR pcszServiceName)
{
    int iResult;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, pcszServiceName, &hints, &result);
    if ( iResult != 0 ) {
        printf(MYTEXT("getaddrinfo failed with error: %d\n"), iResult);
        
        return INVALID_SOCKET;
    }

    // Create a SOCKET for the server to listen for client connections.
    SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf(MYTEXT("socket failed with error: %d\n"), WSAGetLastError());
        freeaddrinfo(result);
        return INVALID_SOCKET;
    }

    // Setup the TCP listening socket
    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf(MYTEXT("bind failed with error: %d\n"), WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        return INVALID_SOCKET;
    }

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf(MYTEXT("listen failed with error: %d\n"), WSAGetLastError());
        closesocket(ListenSocket);
        return INVALID_SOCKET;
    }

    freeaddrinfo(result);
    return ListenSocket;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf(MYTEXT("failed to create socket. Please, specify port number as command line argument\n"));
        return -1;
    }

    std::ios::sync_with_stdio(true);

    net::initializeWinSock();
    SOCKET sock = createSocketAndListen(argv[1]);
    printf(MYTEXT("check\n"));

    for(;;)
    {
        // Accept a client socket
        SOCKET ClientSocket = accept(sock, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf(MYTEXT("accept failed with error: %d\n"), WSAGetLastError());
            continue;
        }

        printf(MYTEXT("->new client has been connected\n"));
        if(net::PrintSocketInfo(ClientSocket, false))
        {
            printf(MYTEXT("failed to get socket info\n"));
            continue;
        }

        HandleConnection(ClientSocket);
    }

    net::cleanupWinSock();
    return 0;
}

HCRYPTKEY exchangeKeys(SOCKET s, HCRYPTPROV hCryptoProv) {
    HCRYPTKEY hExchKey = crypto::getKeyPair(crypto::KeyPairType::Exchange,hCryptoProv);

    BYTE* pbBlob = NULL;
    DWORD dwBlobSize = crypto::exportKey(hExchKey, 0, PUBLICKEYBLOB, &pbBlob);

    std::vector<BYTE> pubKey;
    pubKey.resize(dwBlobSize);
    memcpy(&pubKey[0], pbBlob, dwBlobSize);
    delete [] pbBlob;

    printf("\nPublic Key Blob size is %zu\n", pubKey.size());
    printBytes("Public Key Blob:\n", &pubKey[0], pubKey.size());

    printf("Sending public key to client...\n");
    net::SendMsg(s, pubKey);

    std::vector<BYTE> sesKey;
    net::RecvMsg(s, sesKey);
    printf("Session key received...\n");

    printf("\nSession Key Blob size is %zu\n", sesKey.size());
    printBytes("Session Key Blob:\n", &sesKey[0], sesKey.size());

    crypto::destroyKey(hExchKey);

    return crypto::importKey(hCryptoProv, 0, &sesKey[0], sesKey.size());
}

DWORD HandleConnection(SOCKET ClientSocket)
{
    HCRYPTPROV hCryptoProv  = crypto::getCryptoProv(TEXT("Key Containter for Encoding/Decoding"), crypto::ProvType::RSA);
    HCRYPTKEY hImpSesKey = exchangeKeys(ClientSocket, hCryptoProv);

    while (true)
    {
        printf("Enter message: ");
        std::string data;
        std::getline(std::cin, data);
        if (data.size() == 0)
            continue;
        printf("\n");
        std::vector<BYTE> rawData;
        rawData.resize(data.size());
        memcpy(&rawData[0], &data[0], data.size());
        crypto::encryptData(hImpSesKey, &rawData[0], rawData.size());
        net::SendMsg(ClientSocket, rawData);
        printBytes("\nSent raw data: ", &rawData[0], rawData.size());

        printf("\nWaiting for message from client...\n\n");

        net::RecvMsg(ClientSocket, rawData);
        printBytes("\nReceived raw data: ", &rawData[0], rawData.size());
        crypto::decryptData(hImpSesKey, &rawData[0], rawData.size());
        data.resize(rawData.size());
        memcpy(&data[0], &rawData[0], rawData.size());
        printf("\nReceived message: %s\n\n", data.c_str());
    }

    crypto::destroyKey(hImpSesKey);
    crypto::releaseCryptoProv(hCryptoProv);

    shutdown(ClientSocket, SD_BOTH);
    closesocket(ClientSocket);
    return 0;
}
