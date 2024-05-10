#ifndef _SESSION_H_
#define _SESSION_H_

#include "server.h"
#include "task.h"
//#include <sys/time.h>

class Session
{
public:
    unsigned int mLastHeartbeat;

public:
   // Session(SOCKET aSocket, uint64_t tmo);
    Session(SOCKET aSocket, uint64_t tmo, int listenPort);
    ~Session();
    int write(const char *aString);
    int read(char *aBuffer, int aLen);
    void Update();
    bool Validate();

    void setTask(Tasks* task);
    Tasks* getTask();
    
    void setSncode(std::string snCode);
    std::string getSncode();
    void setWhetherCheckHearbeat(bool CheckHearbeat);
    bool getCheckHearbeat();
    SOCKET socket() { 
        return mSocket; 
    }

    std::mutex& GetMutex() { 
        return mMutex; 
    } 
public: 
    int mlistenPort;
private:
    uint64_t mLastActivetime;
    SOCKET mSocket = INVALID_SOCKET;
    uint64_t mTimeout;
    bool checkHearbeat;
    std::mutex mMutex;
    Tasks* task; 
    std::string snCode;
};
#endif