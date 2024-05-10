#include "session.h"
#include <log4cplus/loggingmacros.h>
#include "log.h"
#include <ctime>
using namespace std;
std::mutex mt;
// std::mutex sendMutex;
// std::mutex receiveMutex;

uint64_t getTimestamp()
{
    std::time_t timestamp = std::time(nullptr);

    //LOG(ERROR) << "Timestamp: " << timestamp ;
    unsigned long ts = timestamp;

    SYSTEMTIME curtime;
     // GetSystemTime(&curtime);
     // LOG(ERROR) << "system hour : " << curtime.wHour << "minute : " << curtime.wMinute;
    GetLocalTime(&curtime);
    //LOG(ERROR) << "local hour : " << curtime.wHour << "minute : " << curtime.wMinute;


    return ts;

}

// Session::Session(SOCKET aSocket, uint64_t tmo)
// {
//   mSocket = aSocket;
//   mTimeout = tmo;
//   Update();
//   fcntl(mSocket, F_SETFL, fcntl(mSocket, F_GETFL) | O_NONBLOCK);//Set to non-blocking
// }

Session::Session(SOCKET aSocket, uint64_t tmo, int listenPort)
{
  mSocket = aSocket;
  mTimeout = tmo;
  mlistenPort = listenPort;
  setWhetherCheckHearbeat(true);
  Update();
  LOG(ERROR) << " new session " << aSocket;
  unsigned long ul = 1;
  int ret = ioctlsocket(mSocket, FIONBIO, (unsigned long*)&ul);
  //fcntl(mSocket, F_SETFL, fcntl(mSocket, F_GETFL) | O_NONBLOCK);//Set to non-blocking
}

Session::~Session()
{
  //shutdown(mSocket, SHUT_RDWR);
  shutdown(mSocket, SD_BOTH);
  LOG(ERROR) << " close session : " << mSocket;
  closesocket(mSocket);
}


void Session::setTask(Tasks* aTask) {
    task = aTask;
}

Tasks* Session::getTask() {
    return task;
}

int Session::write(const char *aString)
{
  std::lock_guard<std::mutex> lock(mt);
  string sendInfor = aString;
  ////TRACE( "send to client : " +  sendInfor);
  int res;
  try {
    res = send(mSocket, aString, (int) strlen(aString), 0);//MSG_DONTWAIT
    }
  catch(...) {
    res = -1;
  }
  return res;
}

int Session::read(char *aBuffer, int aLen)
{
  std::lock_guard<std::mutex> lock(mt);
  int len = recv(mSocket, aBuffer, aLen,0); //MSG_DONTWAIT
  ////TRACE("len : "  + to_string(len) + " aBuffer : " + string(aBuffer));
  if (len > 0) {
    aBuffer[len] = '\0'; // 添加字符串结束符
  } 
  // else 
  //   perror("recv error");
  
  return len;
}


void Session::Update()
{
    mLastActivetime = getTimestamp();
    LOG(ERROR) << "when session last active time : " << mLastActivetime;
}

bool Session::Validate()
{
    //LOG(ERROR) << "session LastActivetime : " << mLastActivetime;
    //LOG(ERROR) << "now : " << getTimestamp();
   /* if (mLastActivetime + 14 < getTimestamp()) {
       LOG(ERROR) << "validate is : " << true;
    }
    else {
        LOG(ERROR) << "validate is : " << false;
    }*/
    //LOG(ERROR) << mLastActivetime + 7000 * 2 < getTimestamp();
    //TRACE( "lastTime : " <<  mLastActivetime);
  // //TRACE( "time : " << mLastActivetime +  7000 * 2 );
  // //TRACE( "current time : " << getTimestamp() );
  // //TRACE( "session is validate : " << to_string(mLastActivetime +  7000 * 2 <= getTimestamp()) );
  return mLastActivetime +  14 < getTimestamp();
}

void  Session::setWhetherCheckHearbeat(bool checkHearbeat) {
    this->checkHearbeat = checkHearbeat;
}
bool  Session::getCheckHearbeat() {
    return checkHearbeat;
}


void Session::setSncode(std::string snCode) {
    this->snCode = snCode;
}

std::string Session::getSncode() {
    return snCode;
}