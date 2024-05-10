#include "phpnetwork.h"
#include "log.h"
#include "doorGuard.h"
#include "encode.h"
#include "ZIni.h"
#include "surroundings.h"
#include "ftp.h"
using namespace std;
#define PHP_THREAD_PORT 80

int gSocketListen = -1;
bool gPhpExit = false;

//#define _GUARD_SOCKET_NONBLOCKING //设为非阻塞之后，本地accept总是出现syn包之后返回rst,ack包，将select的timeout调长后，第一个超时之前正常，之后又不正常。暂时用回阻塞

int socketListen(unsigned short port)
{
	// 1、创建监听用的文件描述符
	gSocketListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (gSocketListen == -1) {
		LOG(ERROR) << "socket create error:" << WSAGetLastError();
		return -1;
	}

	// 2、将监听文件描述符和IP端口信息绑定
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY); // 表示任意可用IP
	addr.sin_port = htons(port);              // 转换成网络字节序（大端字节序）

	int ret = bind(gSocketListen, (struct sockaddr *)&addr, sizeof(addr));
	if (ret == -1) {
		LOG(ERROR) << "socket bind error:" << WSAGetLastError();
		return -1;
	}

	// 3、监听文件描述符
	if ((ret = listen(gSocketListen, 128)) == -1) {
		LOG(ERROR) << "socket listen error:" << WSAGetLastError();
		return -1;
	}

	//int recvTimeout = 3 * 1000;// 3 * 1000;   //3s

	/*struct timeval recvTimeout = { 3,0 };
	if (setsockopt(gSocketListen, SOL_SOCKET, SO_RCVTIMEO, (char *)&recvTimeout, sizeof(struct timeval)) != 0) {
		LOG(ERROR) << "set socket timeout error:" << WSAGetLastError();
	}*/

#ifdef _GUARD_SOCKET_NONBLOCKING
	ULONG NonBlock = 1;
	if (ioctlsocket(gSocketListen, FIONBIO, &NonBlock) == SOCKET_ERROR)
	{
		LOG(ERROR) << "set non blocking socket failed with error" << WSAGetLastError();
		return -1;
	}
#endif
	return gSocketListen;
}

void releasePhpnetworkData()
{
	if (gSocketListen >= 0) {
		closesocket(gSocketListen);
		gSocketListen = -1;
	}
}

int processCMD(short cmd, char *data = nullptr, int len = 0)
{
	std::string sURL, strUserJson;
	char tmpdata[1024];
	//char tmpdata2[1024];
	memset(tmpdata, 0, 1024);
	char *name = nullptr;
	char *password = nullptr;
	int id = 0;
	char *path = nullptr;
	char *args[2];
	char strrestart[] = " -restart";
	args[1] = strrestart;

	static time_t tmlast = 0;
	static time_t tmnow = 0;

	/*time_t tmt;
	tmt = time(NULL);
	char timedata[24];
	sprintf(timedata, "%lld", tmt);*/

	time(&tmnow);
	if (tmnow - tmlast < 3) {//3s内算重复点击
		return -1;
	} 
	tmlast = tmnow;

	switch (cmd)
	{
	case 0://增加人员
	{
		if (!data || len == 0) {
			LOG(INFO) << "cmd format error";
			break;
		}
		LOG(INFO) << "cmd: add user";
		memcpy(&id, data, 4);
		id += 10000;

		//int usernamelen = 0;
		//memcpy(&usernamelen, data+4, 4);

		memcpy(tmpdata, data + 4, len-4);
		tmpdata[len - 4] = 0;
		name = UTF8ToAnsi(tmpdata);
		//strcpy(tmpdata2, name);

		/*memcpy(tmpdata, data + 8 + usernamelen, len - usernamelen - 8);
		tmpdata[usernamelen] = 0;
		password = UTF8ToAnsi(tmpdata);*/

		AddOneUser(name, id);
	}
		break;
	case 1://删除人员
		if (!data || len != 4) {
			LOG(INFO) << "cmd format error";
			break;
		}
		LOG(INFO) << "cmd: del user";
		memcpy(&id, data, 4);
		id += 10000;
		DelOneUser(id); //delete
		break;
	case 2://增加人员头像
		if (!data || len == 0) {
			LOG(INFO) << "cmd format error";
			break;
		}
		LOG(INFO) << "cmd: add user face";
		memcpy(&id, data, 4);
		id += 10000; //前面的留给门禁自己设置的ID
		memcpy(tmpdata, data + 4, len - 4);
		tmpdata[len - 4] = 0;
		path = UTF8ToAnsi(tmpdata);
		SetOneUserFace(id, path);
		break;
	case 3://设置门禁ip 
	{
		//注意，如果门禁和动环的IP相同，且均connect失败的情况，会导致死锁，这里简单检查一下IP是否相同
		if (!data || len == 0) {
			LOG(INFO) << "cmd format error";
			break;
		}
		LOG(INFO) << "cmd: set doorguard ip";
		memcpy(&id, data, 4);
		ZIni ini("./config.ini");
		std::string guardid = std::to_string(id);
		int index = -1;
		for (int i = 0; i < MAX_DOORGUARD_NUM; i++)
		{
			std::string tmpid = "guardID";
			tmpid = tmpid + std::to_string(i + 1);
			std::string tmpguardid = ini.get("setting", tmpid.c_str(), "-1");
			if (tmpguardid == guardid) {
				index = i;
				break;
			}
		}
		if (index == -1) { //未找到同ID的设备
			for (int i = 0; i < MAX_DOORGUARD_NUM; i++)
			{
				std::string tmpid = "guardID";
				tmpid = tmpid + std::to_string(i + 1);
				std::string tmpguardid = ini.get("setting", tmpid.c_str(), "-1");
				if (tmpguardid == "-1") {
					ini.set("setting", tmpid.c_str(), guardid.c_str());
					index = i;
					break;
				}
			}
		}
		if (index == -1) {
			LOG(ERROR) << "max doorguard count, del someone in inifile and try angain";
			break;
		}
		memcpy(tmpdata, data + 4, len - 4);
		tmpdata[len] = 0;
		{
			std::string sip = getSurroundingsIP();
			if (sip == tmpdata) {
				LOG(ERROR) << "doorguard ip same with surroundings ip";
				break;
			}
		}
		setDvrIP(tmpdata, index);

		std::string tmp = "guard";
		tmp = tmp + std::to_string(index + 1);
		ini.set("setting", tmp.c_str(), tmpdata);

		restartGuard(index);
	}
		break;
	case 4://重启程序
		if (!data || len != 0) {
			LOG(INFO) << "cmd format error";
			break;
		}
		LOG(INFO) << "cmd: restart";
		//发送消息给main 这里暂时注释掉
		//releaseResource();
		//_execv(".\\guard.exe", args);
		//exit(-1);
		break;
	case 5://设置动环IP
		//注意，如果门禁和动环的IP相同，且均connect失败的情况，会导致死锁，这里简单检查一下IP是否相同
		if (!data || len == 0) {
			LOG(INFO) << "cmd format error";
			break;
		}
		memcpy(&id, data, 4);
		{
			std::string surroundingsid = std::to_string(id);
			ZIni ini("./config.ini");
			ini.set("setting", "surroundingsID", surroundingsid.c_str());
		}
		memcpy(tmpdata, data + 4, len - 4);
		tmpdata[len - 4] = 0;
		LOG(INFO) << "cmd: set surroundings: " << id << ", ip: " << tmpdata;
		tmpdata[len] = 0;
		{
			for (int i = 0; i < MAX_DOORGUARD_NUM; i++)
			{
				std::string dip = getDvrIP(i);
				if (dip == tmpdata) {
					LOG(ERROR) << "surroundings ip same with doorguard ip," << dip;
					return -1;
				}
			}
		}
		setSurroundingsData(tmpdata, id);
		{
			ZIni ini("./config.ini");
			ini.set("setting", "surroundings", tmpdata);
		}
		refreshSurroundingsData();
		break;
	case 6://停止动环报警
		LOG(INFO) << "cmd: stop surroundings warning";
		stopWarning();
		break;
	case 7://更新
		LOG(INFO) << "cmd: update";
		checkUpdate();
		break;
	case 8:
	{
		LOG(INFO) << "cmd: sync user";
		int cnt = 0;
		memcpy(&cnt, data, 4);
		if (cnt > 2000) {
			break;
		}
		int cursor = 4;
		int userid = -1;
		int usernamelen = 0;
		int faceid = -1;
		char username[200];
		char facepath[500];
		for (int i = 0; i < cnt; i++) {
			memcpy(&userid, data + cursor, 4);
			userid += 10000;
			cursor += 4;
			memcpy(&usernamelen, data + cursor, 4);
			cursor += 4;
			if (usernamelen >= 200) {
				LOG(ERROR) << "user name is too long!";
				break;
			}
			memcpy(username, data + cursor, usernamelen);
			cursor += usernamelen;
			username[usernamelen] = 0;
			AddOneUser(username, userid);

			memcpy(&faceid, data + cursor, 4);
			cursor += 4;
			if (faceid != -1 && faceid < 10000) {
				sprintf(facepath, "./ftproot/doorfaces/%d.jpg", faceid);
				SetOneUserFace(userid, facepath);
			}
		}
	}
		break;
	case 9://删除门禁
		break;
	case 10://删除动环
		break;
	default:
		break;
	}
	return 0;
}

void stopPhpThread()
{
	gPhpExit = true;
}

void phpThread()
{
	if (socketListen(PHP_THREAD_PORT) < 0) {
		LOG(ERROR) << "phpThread create error!" ;
		return;
	}

	fd_set fdread;

	struct timeval timeout;
	timeout.tv_sec = 100; //连接超时3秒
	timeout.tv_usec = 0;

	char data[1024];
	while (!gPhpExit) {
#ifdef _GUARD_SOCKET_NONBLOCKING
		FD_ZERO(&fdread);
		FD_SET(gSocketListen, &fdread);
		int sret = select(gSocketListen + 1, NULL, &fdread, NULL, &timeout);
		if (sret <= 0){ //超时或出错
			continue;
		}
#endif
		struct sockaddr_in clientAddr;                // 输入参数
		int clientAddrLen = sizeof(clientAddr); // 同时作为输入和输出参数
		int cfd = accept(gSocketListen, (struct sockaddr *)&clientAddr, &clientAddrLen);
		if (cfd == -1) {
			LOG(ERROR) << "accept error:" << WSAGetLastError();
			continue;
		}
		int recvTimeout = 4 * 1000;//4s
		/*struct timeval recvTimeout = { 3,0 };*/
		if (setsockopt(cfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&recvTimeout, sizeof(int)) != 0) {
			LOG(ERROR) << "set socket timeout error:" << WSAGetLastError();
		}

		char clientIP[16];
		memset(clientIP, 0x00, sizeof(clientIP));
		inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP)); // 将网络字节序的整数IP转换成主机字节序的点分十进制字符串
		int clientPort = ntohs(clientAddr.sin_port);                          // 将网络字节序转换成主机字节序
		LOG(INFO) << "Accept client:" << clientIP << ", " << clientPort;

		// 5、读写连接
		char buf[10];
		memset(buf, 0x00, sizeof(buf));
		int sz = 0;
		//while (1) {	//php单次访问 发送命令后就关闭
			// 读取客户端信息
		sz = recv(cfd, buf, 10, 0);
		if (sz == 0) { // zero indicates end of file
			LOG(INFO) << "The client is closed";
			closesocket(cfd);
			continue;
		}
		//char strerr[8] = "error:1";
		if (sz == -1 || sz != 10 || buf[0] != 't' || buf[1] != 'x' || buf[8] != 'z' || buf[9] != 'f') {
			LOG(ERROR) << "read format error";
			//send(cfd, strerr, sizeof(strerr), 0);
			closesocket(cfd);
			continue;
		}
		//printf("read: %s\n", buf);

		short cmd = 0;
		int len = 0;
		memcpy(&cmd, buf + 2, 2);
		if (cmd > 0x10)
		{
			LOG(ERROR) << "read format error";
			//send(cfd, strerr, sizeof(strerr), 0);
			closesocket(cfd);
			continue;
		}
		memcpy(&len, buf + 4, 4);
		if (len > 0) {
			sz = recv(cfd, data, len, 0);
			if (sz == 0) { // zero indicates end of file
				LOG(INFO) << "The client is closed";
				closesocket(cfd);
				continue;
			}
			if (sz == -1 || sz != len) {
				LOG(ERROR) << "read error";
				//send(cfd, strerr, sizeof(strerr), 0);
				closesocket(cfd);
				continue;
			}
			closesocket(cfd);
			processCMD(cmd, data, len);
			//do something
		}
		else {
			closesocket(cfd);
			processCMD(cmd);
		}

	}
	if (gSocketListen != -1) {
		closesocket(gSocketListen);
		gSocketListen = -1;
	}
}