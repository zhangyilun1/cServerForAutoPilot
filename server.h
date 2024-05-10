#ifndef _SERVER_H_
#define _SERVER_H_

#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <winsock2.h> 
//#include <winsock.h> 
#include <ws2tcpip.h>
#include <fcntl.h>
#include <thread>
#include <mutex>
#include <iostream>
#include <string>
#include "sessionManager.h"
#include "fileSessionProcessor.h"
#include "updateSessionProcessor.h"
#include "log1.h"

#include "ServerHandler.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "wsock32.lib")
//typedef struct sockaddr_in SOCKADDR_IN;
//typedef struct sockaddr SOCKADDR;
//#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define SOCKET int
//#define closesocket close
//#define closesocket closesocket
class Server {
    
public:
    // Server(int port, SERVER_HANDLER server_handler);
    Server(int port);
    ~Server();

public:
    void serverStart();
    static void startWithListen(Server *server);
    
private:
    void beginListen();
    void nonblockAccecpt(SOCKET mScoket);
    //void sessionConnect(SOCKET mScoket,int lifeTime);
    void sessionConnect(SOCKET mScoket,int lifeTime, int mPort ,int client);
    
protected:
    int mPort;
    SOCKET mScoket;
    //std::shared_ptr<ServerHandler> server_handler_;
 /*   SERVER_HANDLER server_handler_;*/


};

#endif