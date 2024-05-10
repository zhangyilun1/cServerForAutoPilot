#ifndef _SESSION_MANAGER_H_
#define _SESSION_MANAGER_H_

#include "session.h"
#include "dataProcessor.h"
#include "fileSessionProcessor.h"
#include <map>


class Session;
class DataProcessor;
class SessionManager{
  public:
    static SessionManager* GetInstance();
    
  private:
    SessionManager();

  public:
    ~SessionManager();
    //void RegistSession(int64_t socket, uint64_t tmo);
    void RegistSession(int64_t socket, uint64_t tmo,int port);
    Session* DetachSession(int64_t sock);

    static void PfnHeartBeats(SessionManager* sessionManager);
    void HeartBeats();

    static void PfnCheckValidate(SessionManager* sessionManager);
    void CheckValidate();

    //void PfnProcessFileThread(int64_t socket);
    //static void HandleConnectionOnPort32322(SessionManager* sessionManager, int64_t socket);
    ////static void HandleConnectionOnPort32322(SessionManager* sessionManager, int64_t socket);
    //void processFile(int64_t socket);

    std::map<int64_t, Session*> ValidateSession();

    void SendFileInfo(std::string fileName, std::string pictureUrl);
    
    int CheckSessionMapSize();

    std::map<int64_t, Session*> getSessionMap(); 

    void insertToDroneSessionMap(const std::string& snCode, Session* session);
    
    Session* getSessionBySnCode(const std::string& snCode);

    void insertToHeartbeatInfoSessionMap(Session* session, HeartbeatInfo heartbeatInfo);
    
    //Session* getSessionBySnCode(const std::string& snCode);
    std::vector<std::vector<uint8_t>> dataSetPacket(uint8_t buffer[], int ret);


  public:
    std::map<std::string, Session*> snCodeToSessionMap;
    std::map<Session*, HeartbeatInfo> dronesMap;

  private:
    std::map<int64_t, Session*> m_sessMap;
    // std::map<int64_t, std::shared_ptr<Session>> m_sessMap;
    std::mutex mt;
    volatile uint64_t m_tmo = 2000; 
    //DataProcessor* dataProcessor;
    std::unique_ptr <DataProcessor> dataProcessor;
    std::vector<uint8_t> updateBuff;
    std::vector<uint8_t> packetBuff;
    int bufferSize = 0;

    //int prevSize;
    //int prevRet;
};

#endif