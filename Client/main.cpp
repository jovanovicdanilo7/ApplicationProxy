#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define PORT "1080"
#define BUFFER 512
#define SERVER_PORT 27015
#define SET_BYTE(p,data) (*(char*)p = data)

int __cdecl main()
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo* p = NULL;
    struct addrinfo x;

    char buffer[BUFFER] = { 0 };
    char receive_buf[BUFFER];
    char* pBuff;
    const char* serverName = "localhost";

    int iResult;
    int recvbuflen = BUFFER;
    short int portNum = SERVER_PORT;

    pBuff = buffer;

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

    iResult = getaddrinfo(serverName, PORT, &x, &result);
    if (iResult != 0)
    {
        printf("Function getaddrinfo() failed with error: %d.\n", iResult);
        WSACleanup();
        return 1;
    }

    for (p = result; p != NULL; p = p->ai_next)
    {
        ConnectSocket = socket(p->ai_family, p->ai_socktype,
                               p->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
        {
            printf("Socket creation failed with error: %ld.\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        iResult = connect(ConnectSocket, p->ai_addr, (int)p->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    SET_BYTE(pBuff++, 5);
    SET_BYTE(pBuff++, 2);
    SET_BYTE(pBuff++, 0x00);
    SET_BYTE(pBuff++, 0x02);

    printf("+----+----------+----------+\n");
    printf("|VER | NMETHODS | METHODS  |\n");
    printf("+----+----------+----------+\n");
    printf("| %-2x |    %-2x    | %-8s |\n", buffer[0], buffer[1], "1 to 2");
    printf("+----+----------+----------+\n");

    iResult = send(ConnectSocket, buffer, 4, 0);
    if (iResult == SOCKET_ERROR)
    {
        printf("Function send() failed with error: %d.\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    printf("Number of bytes sent: %ld.\n", iResult);

    iResult = recv(ConnectSocket, receive_buf, 2, 0);
    if (iResult == SOCKET_ERROR)
    {
        printf("Function recv() failed with error: %d.\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    if ((receive_buf[0] != buffer[0]) || (receive_buf[1] == 0xFF))
    {
        printf("+----+----------+\n");
        printf("|VER | METHODS  |\n");
        printf("+----+----------+\n");
        printf("| %-2x  | %-2x  |\n", receive_buf[0], receive_buf[1]);
        printf("+----+----------+\n");
        printf("Wrong version of SOCKS protocol or unsupported vrefication method!\n");
        iResult = shutdown(ConnectSocket, SD_SEND);
        if (iResult == SOCKET_ERROR)
        {
            printf("Function shutdown() failed with error: %d.\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }
        return 1;
    }
    else
    {
        pBuff = buffer;
        char username[] = "Djura";
        char password[] = "foklinda";
        char usernameLen = strlen(username);
        char passwordLen = strlen(password);

        printf("Username: %s\n", username);
        printf("Password: %s\n", password);
        printf("Length of username is: %d\n", usernameLen);
        printf("Length of password is: %d\n", passwordLen);

        SET_BYTE(pBuff++, 0x01);
        SET_BYTE(pBuff++, usernameLen);

        for (int i = 0; i < usernameLen; i++)
        {
            SET_BYTE(pBuff++, username[i]);
        }

        SET_BYTE(pBuff++, passwordLen);
        for (int i = 0; i < passwordLen; i++)
        {
            SET_BYTE(pBuff++, password[i]);
        }

        int packetLen = 3 + usernameLen + passwordLen;
        iResult = send(ConnectSocket, buffer, packetLen, 0);
        if (iResult == SOCKET_ERROR)
        {
            printf("Function send() failed with error: %d.\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }
        printf("Number of bytes sent: %ld.\n", iResult);
    }


    iResult = recv(ConnectSocket, receive_buf, 2, 0);
    if (iResult == SOCKET_ERROR)
    {
        printf("Function recv() failed with error: %d.\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    if (receive_buf[0] == buffer[0] && receive_buf[1] == 0x00)
    {
        printf("Authentication successful!\n");

    }else
    {
        printf("Authentication failed! Wrong username or password.\n");
        iResult = shutdown(ConnectSocket, SD_SEND);
        if (iResult == SOCKET_ERROR)
        {
            printf("Function shutdown() failed with error: %d.\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }
        return 1;
    }

    pBuff = buffer;
    SET_BYTE(pBuff++, 0x05);
    SET_BYTE(pBuff++, 0x01);
    SET_BYTE(pBuff++, 0x00);
    SET_BYTE(pBuff++, 0x01);
    SET_BYTE(pBuff++, 127);
    SET_BYTE(pBuff++, 0);
    SET_BYTE(pBuff++, 0);
    SET_BYTE(pBuff++, 1);
    SET_BYTE(pBuff++, (portNum >> 8));
    SET_BYTE(pBuff++, (char)(portNum & 0xff));

    printf("FIRST_PART: %x \n", buffer[8]);
    printf("SECOND_PART: %x\n", (buffer[9]));

    printf("+----+-----+-------+------+----------+----------+\n");
    printf("|VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |\n");
    printf("+----+-----+-------+------+----------+----------+\n");
    printf("| %-2x | %-2x  | X'00' |  %-2x  |%-7s |  %-5s   |\n", buffer[0], buffer[1], buffer[3], "127.0.0.1", "27015");
    printf("+----+-----+-------+------+----------+----------+\n");

    iResult = send(ConnectSocket, buffer, 10, 0);
    if (iResult == SOCKET_ERROR)
    {
        printf("Function send() failed with error: %d.\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
    printf("Number of bytes sent: %ld.\n", iResult);

    iResult = recv(ConnectSocket, receive_buf, 10, 0);
    if (iResult == SOCKET_ERROR)
    {
        printf("Function recv() failed with error: %d.\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    if (receive_buf[1] == 0x00)
    {
        printf("Client is conected to server and ready to send a file!\n");

        FILE* file = fopen("The\\Path\\Of\\The\\File\\You\\Want\\To\\Send", "rb");
        size_t bytesRead = 0;

        if (file != NULL)
        {
            fseek(file, 0, SEEK_END);
            int len = ftell(file);
            rewind(file);
            printf("Size of file is: %d.\n", len);
            buffer[0] = (len >> 24) & 0xff;
            buffer[1] = (len >> 16) & 0xff;
            buffer[2] = (len >> 8) & 0xff;
            buffer[3] = len & 0xff;

            iResult = send(ConnectSocket, buffer, 4, 0);
            if (iResult == SOCKET_ERROR)
            {
                printf("Function send() failed with error: %d.\n", WSAGetLastError());
                closesocket(ConnectSocket);
                WSACleanup();
                return 1;
            }

            while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0)
            {
                iResult = send(ConnectSocket, buffer, sizeof(buffer), 0);
                if (iResult == SOCKET_ERROR)
                {
                    printf("Function send() failed with error: %d.\n", WSAGetLastError());
                    closesocket(ConnectSocket);
                    WSACleanup();
                    return 1;
                }

                printf("Number of bytes sent: %ld.\n", iResult);

                iResult = recv(ConnectSocket, receive_buf, 1, 0);
                if (iResult == SOCKET_ERROR)
                {
                    printf("Function recv() failed with error: %d.\n", WSAGetLastError());
                    closesocket(ConnectSocket);
                    WSACleanup();
                    return 1;
                }
                printf("Acknowledgement received: %ld\n", iResult);
            }
        }else
        {
            printf("An error occurred while sending file.\n");
        }

    }else
    {
        printf("Connection to server failed.\n");
        iResult = shutdown(ConnectSocket, SD_SEND);
        if (iResult == SOCKET_ERROR)
        {
            printf("Function shutdown() failed with error: %d.\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }
    }
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}
