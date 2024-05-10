#include "server.h"
#include "log.h"
#include <log4cplus/loggingmacros.h>
#include <sstream>
using namespace std;
std::mutex mtx;

SessionManager* m_sessionManager = SessionManager::GetInstance();

//Server::Server(int port, SERVER_HANDLER server_handler) : server_handler_(server_handler){
//	assert(server_handler_);
//	LOG(ERROR) << "server begin ";
//	this->mPort = port;
//	this->mScoket = 0;
//	//SessionManager* m_sessionManager = SessionManager::GetInstance();
//}

Server::Server(int port){
	LOG(ERROR) << "server begin ";
	this->mPort = port;
	this->mScoket = 0;
	//SessionManager* m_sessionManager = SessionManager::GetInstance();
}

Server::~Server() {
	LOG(ERROR) << "server delete ";
}

void Server::serverStart() {
	thread th1(startWithListen, this);
	th1.detach();
}


void Server::startWithListen(Server* server) {
	thread::id tid = this_thread::get_id();
	std::stringstream ss;
	ss << tid;
	LOG(ERROR) << "tid" << ss.str();
	server->beginListen();
}

void Server::beginListen() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		LOG(ERROR) << "Failed to initialize Winsock.";
		return;
	}
	LOG(ERROR) << "WSAStartup success.";
	int mOne = 1;
	SOCKADDR_IN t;

	// 创建套接字
	//mScoket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	mScoket = socket(AF_INET, SOCK_STREAM, 0);
	LOG(ERROR) << "mScoket " << mScoket;
	if (mScoket == INVALID_SOCKET) {
		LOG(ERROR) << "Failed to create socket.";
		WSACleanup();
		return;
	}
	LOG(ERROR) << "create socket success.";
	// 设置 SO_REUSEADDR 选项
	//int recvTimeout = 3 * 1000;// 3 * 1000; //3s
	//setsockopt(mScoket, SOL_SOCKET, SO_RCVTIMEO, (char*)&recvTimeout, sizeof(int));
	if (setsockopt(mScoket, SOL_SOCKET, SO_REUSEADDR, (char*)&mOne, sizeof(int)) == SOCKET_ERROR) {
		int error = WSAGetLastError();
		LOG(ERROR) << "Failed to set SO_REUSEADDR option." << error;
		closesocket(mScoket);
		WSACleanup();
		return;
	}

	//LOG(ERROR) << "begin listening";
	//int mOne = 1;
	//SOCKADDR_IN t;
	////signal(SIGPIPE, SIG_IGN);
	//mScoket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//setsockopt(mScoket, SOL_SOCKET, SO_REUSEADDR, (char *)&mOne, sizeof(int));
	//LOG(ERROR) << "mScoket " << mScoket;
	//if(mScoket == INVALID_SOCKET)
	//{
	//    return;
	//}

	LOG(ERROR) << "setsockopt";
	t.sin_family = AF_INET;
	t.sin_port = htons(this->mPort);
	t.sin_addr.s_addr = htonl(INADDR_ANY);
	if (::bind(mScoket, (SOCKADDR*)&t, sizeof(t)) == SOCKET_ERROR)
	{
		return;
	}
	LOG(ERROR) << "bind";
	if (listen(mScoket, 4) == SOCKET_ERROR)
	{
		return;
	}
	LOG(ERROR) << "Server started. Listening on port" << to_string(this->mPort);
	//TRACE( "Server started. Listening on port : " + to_string(this->mPort));
	while (1)
	{
		nonblockAccecpt(mScoket);
		Sleep(100);
	}
}


void Server::nonblockAccecpt(SOCKET mSocket)
{
	fd_set rset;
	FD_ZERO(&rset);
	FD_SET(mSocket, &rset);
	int nfds = mSocket + 1;
	struct timeval timeout;
	timeout.tv_sec = 1;  // time out 1 second
	timeout.tv_usec = 0;
	::memset(&timeout, 0, sizeof(timeout));
	if (::select(nfds, &rset, 0, 0, &timeout) > 0)
	{
		if (FD_ISSET(mSocket, &rset))
		{
			SOCKADDR_IN addr;
			socklen_t len = sizeof(addr);
			memset(&addr, 0, sizeof(addr));

			SOCKET socket = ::accept(mSocket, (SOCKADDR*)&addr, &len);

			if (socket == INVALID_SOCKET)
			{
				return;
			}
			char clientIP[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(addr.sin_addr), clientIP, INET_ADDRSTRLEN);
			int clientPort = ntohs(addr.sin_port);
			string str(clientIP);
			LOG(ERROR) << "Client IP " << str;
			LOG(ERROR) << "Client Port " << to_string(clientPort);
			LOG(ERROR) << "socket " << socket;
			FD_CLR(mSocket, &rset);
			/*server_handler_(socket, clientPort);*/
			//server_handler_->Handler(socket, clientPort);
			sessionConnect(socket, 140, this->mPort , clientPort);
			
		}
	}
}
//void Server::sessionConnect(SOCKET socket, int lifeTime)
//void Server::sessionConnect(SOCKET socket, int lifeTime, int port)
//{
//	SessionManager* m_sessionManager = SessionManager::GetInstance();
//	//FileSessionProcessor* fileSessionProcessor = m_sessionManager->createFileSessionProcessor();
//	//FileSessionProcessor* fileSessionProcessor = new FileSessionProcessor();
//	//FileSessionProcessor* fileSessionProcessor = FileSessionProcessor ::GetInstance(port, socket);
//	//if (port == 32322) {
//	//	LOG(ERROR) << "connect  32322 ";
//	//	fileSessionProcessor->PfnProcessFileThread(socket);
//	//}
//	//else {
//		LOG(ERROR) << "== RegistSession socket ==  ";
//		m_sessionManager->RegistSession(socket, lifeTime, port);
//		
//	//}
//	
//}


void Server::sessionConnect(SOCKET socket, int lifeTime, int port, int client)
{
	if (port == 32322) {
		LOG(ERROR) << "connect  32322 " << client;
		//FileSessionProcessor* fileSessionProcessor = new FileSessionProcessor();
		std::shared_ptr <FileSessionProcessor> fileSessionProcessor = std::make_shared <FileSessionProcessor> ();
		fileSessionProcessor->PfnProcessFileThread(socket);
		//delete fileSessionProcessor;
	}
	else if (port == 32330) {
		//UpdateSessionProcessor* updateSessionProcessor = new UpdateSessionProcessor();
		std::shared_ptr <UpdateSessionProcessor> updateSessionProcessor = std::make_shared <UpdateSessionProcessor>();
		LOG(ERROR) << "connect  32330 " << client;
		updateSessionProcessor->PfnProcessUpdateThread(socket);
		//delete updateSessionProcessor;
	}
	else {
		LOG(ERROR) << "== RegistSession socket ==  ";
		
		m_sessionManager->RegistSession(socket, lifeTime, port);

	}
}



