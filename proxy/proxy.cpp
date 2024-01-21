#include "proxy.h"
#include <iostream>

AppProxy::AppProxy(QObject * parent):QObject(parent)
{
    buffer = (char* ) malloc(sizeof(char*) * 2048);
    response = (char* ) malloc(sizeof(char*) * 2048);

    clients["Danilo"] = "mrkirm1";
    clients["Sajmon"] = "cemusveovo";
    clients["Djura"]  = "foklinda";
    clients["Nile"]   = "mekijanovic";
    clients["Roske"]  = "sisavcad";

    connect(this,    SIGNAL(startSignalArrived()),  this, SLOT(startProxy()));
    connect(&socket, SIGNAL(newConnection()),       this, SLOT(acceptConnection()));
    connect(this,    SIGNAL(firstMessageArived()),  this, SLOT(readFirstMessage()));
    connect(this,    SIGNAL(autheticationArived()), this, SLOT(autheticationConfirmed()));
    connect(this,    SIGNAL(serverInfoRecieved()),  this, SLOT(connectToServer()));
    connect(this,    SIGNAL(filePartArrived()),     this, SLOT(sendToServer()));
}

AppProxy::~AppProxy()
{
    socket.close();
}

void AppProxy::start()
{
    emit startSignalArrived();
}

void AppProxy::startProxy()
{
    socket.listen(QHostAddress::Any, 1080);
    cout << "Proxy is listening for incoming connections..." << endl << endl;
}

void AppProxy::acceptConnection()
{
    clientSocket = socket.nextPendingConnection();
    cout << "Connection accepted!" << endl << endl;

    if(clientSocket->waitForReadyRead() == true)
    {
        emit firstMessageArived();
        clientSocket->waitForBytesWritten();
    }
}

void AppProxy::readFirstMessage()
{
    int ret = 0;
    ret = clientSocket->read(buffer, clientSocket->bytesAvailable());
    cout << "Proxy has readed selection message."<< endl;
    if(ret != 0)
    {
        buffer[ret] = '\0';
        cout << "Number of readed bytes: ";
        cout << ret << endl << endl;
        cout << "Clients selection message: "<< endl << endl;
        cout << "Protocol version: ";
        cout << hex <<(int)buffer[0] << endl;
        cout << "Number of methods: ";
        cout << hex <<(int)buffer[1] << endl;
        cout << "Method number: ";
        cout << hex <<(int)buffer[2] << endl;
        cout << "Method number: ";
        cout << hex <<(int)buffer[3] << endl << endl;
    }

    if(buffer[0] != 5)
    {
        response[0] = 5;
        response[1] = 0xff;
        clientSocket->write(response, 2);
        socket.close();
        emit startSignalArrived();
    }else if (((buffer[1] == 1) && ((buffer[2] == 0) || (buffer[2] == 2))) || ((buffer[1] == 2) && ((buffer[2] == 0) && (buffer[3] == 2))))
    {
        cout << "Sending resposne..." << endl;
        response[0] = 5;
        response[1] = 0x02;
        clientSocket->write(response, 2);
        if(clientSocket->waitForReadyRead() == true)
        {
            emit autheticationArived();
            clientSocket->waitForBytesWritten();
        }

    }else
    {
        response[0] = 5;
        response[1] = 0xff;
        clientSocket->write(response, 2);
        socket.close();
        emit startSignalArrived();
    }
}

void AppProxy::autheticationConfirmed()
{
    int ret = clientSocket->read(buffer, clientSocket->bytesAvailable());
    cout << "The authentication process has started..." << endl;
    int usernameLen = buffer[1];
    int passwordLen = buffer[2+usernameLen];
    string username ="";
    string password ="";
    if(ret != 0)
    {
        buffer[ret] = '\0';
        cout << "Number of readed bytes: ";
        cout << ret << endl << endl;
        cout << "Authentication message arrived: "<< endl;
        std::cout.copyfmt(std::ios(NULL));
        for(int i = 0; i<usernameLen; i++)
        {
            username += buffer[2+i];
        }
        for(int i = 0; i<passwordLen; i++)
        {
            password += buffer[3+usernameLen+i];
        }

        cout << "Username: ";
        cout <<username << endl;
        cout << "Password: ";
        cout << password << endl;

        cout << "Checking username and password..." << endl;
    }
    map<string, string>::iterator it;
    it = clients.find(username);
    if(it != clients.end())
    {
        if(clients[username] == password)
        {
            response[0] = 0x01;
            response[1] = 0x00;
            clientSocket->write(response, 2);

            cout << "Username and password are correct!" << endl;

            if(clientSocket->waitForReadyRead() == true)
            {
                emit serverInfoRecieved();
                clientSocket->waitForBytesWritten();
            }

        }else
        {
            response[0] = 0x01;
            response[1] = 0xff;
            clientSocket->write(response, 2);
            socket.close();
            emit startSignalArrived();
        }
    }else
    {
        response[0] = 0x01;
        response[1] = 0xff;
        clientSocket->write(response, 2);
        socket.close();
        emit startSignalArrived();
    }

}

void AppProxy::connectToServer()
{
    int ret = clientSocket->read(buffer, clientSocket->bytesAvailable());
    cout << "Reading SOCKS request message..." << endl;
    cout << "Number of readed bytes: ";
    cout << ret << endl << endl;
    cout << "Connecting to server..." << endl;
    qint16 portNum;
    qint32 firstOctet = (int)buffer[4];
    qint32 secondOctet = (int)buffer[5];
    qint32 thirdOctet = (int)buffer[6];
    qint32 fourthOctet = (int)buffer[7];
    qint32 ipv4 = (firstOctet * 16777216)
            + (secondOctet * 65536)
            + (thirdOctet * 256)
            + (fourthOctet);
    unsigned char lowerPart = (buffer[9]);
    unsigned char upperPart = (buffer[8]);
    portNum = (upperPart << 8) + lowerPart;
    cout << "Number of port: ";
    cout << portNum << endl;
    QHostAddress addr(ipv4);
    serverSocket.connectToHost(addr, portNum);
    serverSocket.waitForConnected();
    if(serverSocket.waitForConnected() == false)
    {
        response[0] = 5;
        response[1] = 0xff;
        response[2] = 0x00;
        response[3]= 0x01; //IPv4
        for(int i=0;i<4;i++)
        {
            response[4+i] = buffer[4+i];
        }
        response[8] = buffer[8]; //first part of port
        response[9] = buffer[9]; //second part of port
        clientSocket->write(response, 10);
        socket.close();
        emit startSignalArrived();

    }else
    {
        cout << "Successfully connected to the server!" <<endl;
        response[0] = 5;
        response[1] = 0x00;
        response[2] = 0x00;
        response[3]= 0x01; //IPv4
        for(int i=0;i<4;i++)
        {
            response[4+i] = buffer[4+i];
        }
        response[8] = buffer[8]; //first part of port
        response[9] = buffer[9]; //second part of port
        clientSocket->write(response, 10);

        if(clientSocket->waitForReadyRead() == true)
        {
            emit filePartArrived();
            serverSocket.waitForBytesWritten();
        }

    }

}

void AppProxy::sendToServer()
{
    cout << "Receiving the file from the client and forwarding it to the server." << endl;
    int ret = 0;
    int ret1 = 0;
    int fileSize = 0;
    //Recieve size of file
    ret = clientSocket->read(buffer, 4);
    unsigned char firstOctet = (int)buffer[0];
    unsigned char secondOctet = (int)buffer[1];
    unsigned char thirdOctet = (int)buffer[2];
    unsigned char fourthOctet = (int)buffer[3];

    fileSize = (firstOctet << 24) + (secondOctet << 16) + (thirdOctet << 8) + fourthOctet;
    cout << "Size of file is: ";
    cout << fileSize << endl;

    while(fileSize > 0)
    {
        clientSocket->waitForReadyRead();
        ret = clientSocket->read(buffer, 512);
        response = buffer;
        serverSocket.waitForBytesWritten();
        ret1 = serverSocket.write(response, 512);
        clientSocket->waitForBytesWritten();
        clientSocket->write(buffer, 1);
        fileSize-=ret1;
    }
    socket.close();
    serverSocket.close();
    emit startSignalArrived();

}

