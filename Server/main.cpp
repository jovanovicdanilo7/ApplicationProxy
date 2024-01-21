#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")

#define BUFFER 512
#define PORT "27015"

int __cdecl main(void)
{
    WSADATA wsaData;
    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo x;

    int iResult;
    int recvbuflen = BUFFER;
    char receive_buf[BUFFER];

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("Function WSAStartup() failed with error: %d.\n", iResult);
        return 1;
    }

    ZeroMemory(&x, sizeof(x));
    x.ai_family = AF_INET;
    x.ai_socktype = SOCK_STREAM;
    x.ai_protocol = IPPROTO_TCP;
    x.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(NULL, PORT, &x, &result);
    if (iResult != 0)
    {
        printf("Function getaddrinfo() failed with error: %d.\n", iResult);
        WSACleanup();
        return 1;
    }

    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET)
    {
        printf("Socket creation failed with error: %ld.\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("Function bind() failed with error: %d.\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("Function listen() failed with error: %d.\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET)
    {
        printf("Function accept() failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }


    printf("Client socket accepted and client is connected!\n");
    printf("Server is ready for receiving...\n");

    closesocket(ListenSocket);

    FILE* file = fopen("D:\\Danilo\\FTN\\MRKiMR\\Project\\Server\\file.txt", "wb");
    size_t bytesRead = 0;

    while ((iResult = recv(ClientSocket, receive_buf, recvbuflen, 0))>0)
    {
        fwrite(receive_buf, sizeof(char), iResult, file);
        bytesRead += recvbuflen;
    }

    fclose(file);

    printf("Number of bytes received: %d.\n", bytesRead);
    printf("File is received and saved on the given location!\n");


    do
    {
        if (iResult == 0)
            printf("Connection closing...\n");
        else
        {
            printf("Function recv() failed with error: %d.\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

    } while (iResult > 0);

    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("Function shutdown() failed with error: %d.\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}
