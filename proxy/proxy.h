#ifndef PROXY_H
#define PROXY_H
#include<map>
#include<string>
#include <QtNetwork>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtDebug>
using namespace std;

class AppProxy : public QObject
{
    Q_OBJECT
public:
    AppProxy(QObject * parent = 0);
    ~AppProxy();

    char* buffer;
    char* response;
    void start();

public slots:
    void startProxy();
    void acceptConnection();
    void readFirstMessage();
    void autheticationConfirmed();
    void connectToServer();
    void sendToServer();
    //void writeB(qint64);

private:

    QTcpServer socket;
    QTcpSocket *clientSocket;
    QTcpSocket serverSocket;
    map <string, string> clients;

signals:
    void startSignalArrived();
    void autheticationArived();
    void firstMessageArived();
    void serverInfoRecieved();
    void filePartArrived();

};
#endif // PROXY_H
