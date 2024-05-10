#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

// 处理客户端连接的函数
void handleClient(int clientSocket) {
    // 接收和发送数据的缓冲区
    char buffer[1024];
    while (true) {
        // 接收客户端发送的消息
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            // 客户端断开连接
            break;
        }
        // 处理客户端发送的消息
        // ...
        
        // 发送响应给客户端
        send(clientSocket, buffer, bytesRead, 0);
    }
    
    // 关闭客户端连接
    close(clientSocket);
}

int main() {
    // 创建服务器套接字
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Failed to create server socket." );
        return 1;
    }
    
    // 绑定服务器地址和端口
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8000);
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Failed to bind server socket." );
        close(serverSocket);
        return 1;
    }
    
    // 监听连接请求
    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Failed to listen on server socket." );
        close(serverSocket);
        return 1;
    }
    
    std:://TRACE( "Server started. Listening on port 8000..." );
    
    std::vector<std::thread> clientThreads;
    
    while (true) {
        // 接受客户端连接
        sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if (clientSocket == -1) {
            std::cerr << "Failed to accept client connection." );
            continue;
        }
        
        // 创建一个新线程处理客户端连接
        std::thread clientThread(handleClient, clientSocket);
        clientThread.detach();
    }
    
    // 关闭服务器套接字
    close(serverSocket);
    
    return 0;
}
*/