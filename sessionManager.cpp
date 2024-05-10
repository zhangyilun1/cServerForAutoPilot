#include "sessionManager.h"
#include "log.h"
#include <sstream>
using namespace std;
static SessionManager *sessionManager;

SessionManager *SessionManager::GetInstance()
{
    if (sessionManager == nullptr)
        sessionManager = new SessionManager();

    return sessionManager;
}

SessionManager::SessionManager():dataProcessor(make_unique<DataProcessor>())
{
    //DataProcessor* dataProcessor = new DataProcessor();

    std::thread t(PfnHeartBeats, this);
    t.detach();

    //std::thread t2(PfnCheckValidate, this);
    //t2.detach();
}

//SessionManager::checkHeartbeat() {
//    std::thread t2(PfnCheckValidate, this);
//    t2.detach();
//}

SessionManager::~SessionManager()
{
    //delete dataProcessor;  
    LOG(ERROR) << "release session manager ";
    for (auto it = m_sessMap.begin(); it != m_sessMap.end(); it++)
    {
        delete it->second;
    }
}

void SessionManager::RegistSession(int64_t socket, uint64_t tmo, int port)
{
    {
        std::lock_guard<std::mutex> lock(mt);
        //if session unexisted insert to map
        auto it = m_sessMap.find(socket);
        if (it != m_sessMap.end()) {
            LOG(ERROR) << "SAME Socket in m_sessMap";
            //Session* sessionTodelete = it->second.get();
            Session* sessionTodelete = it->second;
            closesocket(socket);
            m_sessMap.erase(socket);
            auto snCodeIter = snCodeToSessionMap.begin();
            while (snCodeIter != snCodeToSessionMap.end())
            {
                LOG(ERROR) << "snCode to delete first : " << snCodeIter->first;
                LOG(ERROR) << "snCode to delete second : " << snCodeIter->second;
                if (snCodeIter->second == sessionTodelete)
                {
                    LOG(ERROR) << "snCode to delete : " << snCodeIter->first;
                    dataProcessor->deleteDroneFromDB(snCodeIter->first);
                    snCodeToSessionMap.erase(snCodeIter++);
                    LOG(ERROR) << "snCodeToSessionMap already delete in snCodeToSessionMap ";
                }
                else
                {
                    ++snCodeIter;
                }
            }
            auto dronesIter = dronesMap.find(sessionTodelete);
            if (dronesIter != dronesMap.end())
            {
                dronesMap.erase(dronesIter);
                LOG(ERROR) << "snCodeToSessionMap already delete in dronesMap ";
            }
            delete sessionTodelete;
        }
        if (tmo < m_tmo)
        {
            m_tmo = tmo;
        }
        Session* a = new Session(socket, tmo, port);
        m_sessMap.insert(pair<int64_t, Session*>(socket, a));
         /* auto sessionPtr = std::make_shared<Session>(socket, tmo, port);
            m_sessMap.insert(make_pair(socket, sessionPtr));*/
        LOG(ERROR) << " INSERT SUCCESS " << m_sessMap.size() << " port : " << port;
    }
        if (port == 2345) {
            LOG(ERROR) << "SEND TO WEB";
            for (auto droneSession = snCodeToSessionMap.begin(); droneSession != snCodeToSessionMap.end(); droneSession++) {
                //TRACE("drone snCode :" + droneSession->first);
                //HeartbeatInfo infor = dronesMap.find(droneSession->second);
                auto droneInfo = dronesMap.find(droneSession->second);
                LOG(ERROR) << "drones Map size : " << dronesMap.size();
                if (droneInfo != dronesMap.end()) {
                    HeartbeatInfo infor = droneInfo->second;
                    nlohmann::json jsonData;
                    jsonData["snCode"] = droneSession->first;
                    LOG(ERROR) << "snCode: " << droneSession->first;
                    jsonData["longitude"] = to_string(infor.longitude);
                    LOG(ERROR) << "longitude : " << infor.longitude;
                    jsonData["latitude"] = to_string(infor.latitude);
                    LOG(ERROR) << "latitude :" << infor.latitude;
                    jsonData["altitude"] = to_string(infor.altitude);
                    LOG(ERROR) << "altitude :" << infor.altitude;
                    jsonData["missionID"] = to_string(infor.missionID);
                    LOG(ERROR) << "missionID :" << infor.missionID;
                    jsonData["status"] = to_string(infor.status);
                    LOG(ERROR) << "status :" << (int)infor.status;
                    jsonData["index"] = to_string(infor.index);
                    LOG(ERROR) << "index :" << infor.index;
                    jsonData["count"] = to_string(infor.count);
                    LOG(ERROR) << "count :" << infor.count;
                    string jsonString = jsonData.dump();
                    const char* buffer1 = jsonString.c_str();

                    SSIZE_T bytesSent = send(socket, buffer1, strlen(buffer1), 0);
                    //TRACE("SEND TO PHP SIZE :" + to_string(bytesSent));
                }
            }
        }
    
    // m_sessMap.insert(pair<int64_t, Session*>(socket, a));

}

map<int64_t, Session *> SessionManager::getSessionMap()
{
    map<int64_t, Session*> m_sessMap1;
    return m_sessMap1;
}

void SessionManager::PfnHeartBeats(SessionManager *sessionManager)
{
    sessionManager->HeartBeats();
}

//void SessionManager::PfnProcessFileThread(int64_t socket)
//{
//    LOG(ERROR) << "Create thread for process File ";
//    std::thread processFileThread(HandleConnectionOnPort32322, this, socket);
//    processFileThread.detach();
//}


//void SessionManager::HandleConnectionOnPort32322(SessionManager* sessionManager, int64_t socket)
//{
//    LOG(ERROR) << "HandleConnectionOnPort32322 ";
//   sessionManager->processFile(socket);
//}
//
//void  SessionManager::processFile(int64_t socket) {
//    LOG(ERROR) << "process file thread ";
//}
//FileSessionProcessor* SessionManager::createFileSessionProcessor() {
//    FileSessionProcessor* fileSessionProcessor = new FileSessionProcessor();
//    return fileSessionProcessor;
//}


void SessionManager::HeartBeats()
{
    LOG(ERROR) << " CHECK DATA ";
    //vector<uint8_t> updateBuff;
    while (true)
    {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        int max_fd = -1;
        {
            std::lock_guard<std::mutex> lock(mt);
            if (!m_sessMap.empty()) {
                for (auto it = m_sessMap.begin(); it != m_sessMap.end();)
                {
                    int current_fd = it->second->socket();
                    //int a = it->first;
                    FD_SET(current_fd, &read_fds);
                    max_fd = max(max_fd, current_fd);
                    //if (it->second->getCheckHearbeat()) {
                        if (it->second->Validate()) {
                            LOG(ERROR) << "Connection timeout - Removing session: " + to_string(current_fd);
                           // Session* sessionTodelete = it->second.get();
                            Session* sessionTodelete = it->second;
                            closesocket(it->second->socket());
                            it = m_sessMap.erase(it);
                            LOG(ERROR) << "close socket";
                            LOG(ERROR) << "session already delete in m_sessMap";
                            auto snCodeIter = snCodeToSessionMap.begin();
                            while (snCodeIter != snCodeToSessionMap.end())
                            {
                                LOG(ERROR) << "snCode to delete first : " << snCodeIter->first;
                                LOG(ERROR) << "snCode to delete second : " << snCodeIter->second;
                                if (snCodeIter->second == sessionTodelete)
                                {
                                    LOG(ERROR) << "snCode to delete : " << snCodeIter->first;
                                    dataProcessor->deleteDroneFromDB(snCodeIter->first);
                                    snCodeToSessionMap.erase(snCodeIter++);
                                    LOG(ERROR) << "snCodeToSessionMap already delete in snCodeToSessionMap ";
                                }
                                else
                                {
                                    ++snCodeIter;
                                }
                            }
                            auto dronesIter = dronesMap.find(sessionTodelete);
                            if (dronesIter != dronesMap.end())
                            {
                                dronesMap.erase(dronesIter);
                                LOG(ERROR) << "snCodeToSessionMap already delete in dronesMap ";
                            }
                            delete sessionTodelete;
                        }
                        else {
                            it++;
                        }
                    // }
                    //else {
                    //    it++;
                    //}
                  
                }
            }
        }

        struct timeval timeout;
        timeout.tv_sec = 14;
        timeout.tv_usec = 0;
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
        char errorBuffer[256];
        if (activity < 0)
        {
            continue;
        }
        else if (activity == 0)
        {
            continue;
        }
        else
        {
            {
                std::lock_guard<std::mutex> lock(mt);
                if (!m_sessMap.empty()) {
                    for (auto it = m_sessMap.begin(); it != m_sessMap.end();)
                    {
                        int current_fd = it->second->socket();
                        if (FD_ISSET(current_fd, &read_fds)) {
                            //uint8_t buffer[1024 * 128];
                            uint8_t* buffer = new uint8_t[1024 * 128];
                            int ret = it->second->read(reinterpret_cast<char*>(buffer), 1024 * 128);
                            if (ret == 0)
                            {
                                LOG(ERROR) << "Connection closed by the remote host.";
                                LOG(ERROR) << "m_sessMap size : " << m_sessMap.size();
                                LOG(ERROR) << "snCodeToSessionMap size : " << snCodeToSessionMap.size();
                                LOG(ERROR) << "dronesMap size : " << dronesMap.size();
                                //Session* sessionTodelete = it->second.get();
                                Session* sessionTodelete = it->second;
                                closesocket(it->second->socket());
                                it = m_sessMap.erase(it);
                                LOG(ERROR) << "close socket";
                                LOG(ERROR) << "session already delete in m_sessMap " << m_sessMap.size() << m_sessMap.empty();
                                LOG(ERROR) << "sessionTodelete : " << sessionTodelete;
                                auto snCodeIter = snCodeToSessionMap.begin();
                                //LOG(ERROR) << "snCode to delete first : " << snCodeIter->first;
                                //LOG(ERROR) << "snCode to delete second : " << snCodeIter->second;
                                while (snCodeIter != snCodeToSessionMap.end())
                                {
                                    if (snCodeIter->second == sessionTodelete)
                                    {
                                        LOG(ERROR) << "snCode to delete : " << snCodeIter->first;
                                        dataProcessor->deleteDroneFromDB(snCodeIter->first);
                                        snCodeToSessionMap.erase(snCodeIter++);
                                        LOG(ERROR) << "snCodeToSessionMap already delete in snCodeToSessionMap ";

                                    }
                                    else
                                    {
                                        ++snCodeIter;
                                    }
                                }
                                auto dronesIter = dronesMap.find(sessionTodelete);

                                if (dronesIter != dronesMap.end())
                                {
                                    dronesMap.erase(dronesIter);
                                    LOG(ERROR) << "snCodeToSessionMap already delete in dronesMap ";
                                }
                                LOG(ERROR) << "continue ";
                                delete sessionTodelete;
                                delete[] buffer;
                                continue;
                            }
                            else if (ret < 0)
                            {
                                it++;
                                continue;
                            }
                            else if (ret > 0) {
                                LOG(ERROR) << "ret > 0 and size is : " << ret;
                                vector<vector<uint8_t>> packets;
                                int result;

                                /* if (it->second->mlistenPort == 32330) {
                                    LOG(ERROR) << "Port 32330 for update software ";
                                    updateBuff.insert(updateBuff.end(), buffer, buffer +ret);
                                    LOG(ERROR) << "updateBuff : " << updateBuff.size();
                                    if (updateBuff.size() == 40) {
                                        dataProcessor->processUpgrade(updateBuff.data(), updateBuff.size(), it->second);
                                        updateBuff.clear();
                                    }
                                }
                                else */
                   
                                if (it->second->mlistenPort == 32321) {
                                    packets = dataSetPacket(buffer, ret);
                                    for (auto& packet : packets) {
                                        const uint8_t* packetData = packet.data();
                                        size_t packetSize = packet.size();
                                        Tasks* task = it->second->getTask();
                                        LOG(ERROR) <<"packetdata: " << (int)packetData[0];
                                        if (task && packetData[0] == 0x06) {
                                            //dataProcessor->getTaskInfo(packetData, packetSize, it->second.get());
                                            dataProcessor->getTaskInfo(packetData, packetSize, it->second);
                                            //delete task;
                                        }
                                        else if (packetData[0] == 0x07) {
                                           // dataProcessor->changeTowerGPT(packetData, packetSize, it->second.get());
                                            dataProcessor->changeTowerGPT(packetData, packetSize, it->second);
                                        }
                                        else if (packetData[0] == 0x08) {
                                            //dataProcessor->getRecord(packetData, packetSize, it->second.get());
                                            dataProcessor->getRecord(packetData, packetSize, it->second);
                                        }
                                        else if (packetData[0] == 0x14) {
                                            dataProcessor->getWifiRecord(packetData, packetSize, it->second);
                                        }
                                        else
                                           // dataProcessor->processDronesData(packetData, packetSize, it->second.get());
                                            dataProcessor->processDronesData(packetData, packetSize, it->second);
                                    }
                                    
                                   // it->second.get()->Update();
                                    it->second->Update();
                                }
                                else if (it->second->mlistenPort == 2345)
                                {
                                    LOG(ERROR) << "Port 2345 from web : ";
                                    dataProcessor->processReceivedData(reinterpret_cast<char*>(buffer), ret, it->second);
                                    //dataProcessor->processReceivedData(reinterpret_cast<char*>(buffer), ret, it->second.get());
                                }
                             
                                it++;
                            }
                            delete[] buffer;
                        }
                        else {
                            it++;
                        }
                        
                    }
                }


            }
            
        }
        Sleep(20);
    }
}

vector<vector<uint8_t>> SessionManager::dataSetPacket(uint8_t buffer[], int ret) {
    vector<vector<uint8_t>> packets;

    for (size_t i = 0; i < ret;) {
        uint16_t cmd = buffer[i];
        LOG(ERROR) << "cmd: " << cmd;
        uint16_t dataLength = (static_cast<uint16_t>(buffer[i + 1]) << 8) | buffer[i + 2];
        LOG(ERROR) << "dataLength : " << dataLength;
        int intValue = static_cast<int>(dataLength);
        LOG(ERROR) << "intValue : " << intValue;
        if (bufferSize == 0) {
            bufferSize = i + intValue + 3;
        }
        LOG(ERROR) << "bufferSize : " << bufferSize;
        LOG(ERROR) << "ret : " << ret;
        if (i + 3 + intValue <= ret) {
            LOG(ERROR) << "receive data more than or equal to ret ";
            vector<uint8_t> packet(buffer + i, buffer + i + 3 + dataLength);
            packets.push_back(packet);
            bufferSize = 0;
            i += 3 + dataLength;
        }
        else if (packetBuff.size() + ret <= bufferSize) {
            LOG(ERROR) << "receive data bigger than ret ";
            packetBuff.insert(packetBuff.end(), buffer, buffer + ret);
            LOG(ERROR) << "packetBuff : " << packetBuff.size();
            if (packetBuff.size() == bufferSize) {
                LOG(ERROR) << "packetBuff.size() == bufferSize " ;
                packets.push_back(packetBuff);
                packetBuff.clear();
            }
            i += ret;
        }
        else {
            break;
        }
        //return packets;
    }
    return packets;
}

//void SessionManager::HeartBeats()
//{
//    while (true)
//    {
//        LOG(ERROR) << "==== check hearbeat ====";
//
//        uint8_t buffer[100000];  // Use uint8_t buffer for receiving data
//        LOG(ERROR) << "m_sessMap size :" << m_sessMap.size();
//        LOG(ERROR) << "snCodeToSessionMap size :" << snCodeToSessionMap.size();
//        LOG(ERROR) << "dronesMap size :" << dronesMap.size();
//        for (auto it = m_sessMap.begin(); it != m_sessMap.end(); it++)
//        {
//            int ret = it->second->read(reinterpret_cast<char*>(buffer), sizeof(buffer));
//            if (ret > 0){
//                vector<vector<uint8_t>> packets;
//                if(it->second->mlistenPort == 32330){
//                    LOG(ERROR) << "Port 32330 for update software ";
//                    dataProcessor->processUpgrade(buffer, ret, it->second);  
//                }else if(it->second->mlistenPort == 32321) {   
//                    LOG(ERROR) << "Port 32321 for process data form drone "; 
//                    for (size_t i = 0; i < ret;) {
//                        uint16_t cmd = buffer[i];
//                        LOG(ERROR) << "cmd" << cmd;
//                        uint16_t dataLength = (static_cast<uint16_t>(buffer[i + 1]) << 8) | buffer[i + 2];
//                        int intValue = static_cast<int>(dataLength);
//                        if (i + 3 + intValue <= ret) {
//                            vector<uint8_t> packet(buffer + i, buffer + i + 3 + dataLength);
//                            packets.push_back(packet);
//                            i += 3 + dataLength;
//                        }
//                        else
//                            break;
//                    }
//                    for (auto & packet : packets) {
//                        const uint8_t *packetData = packet.data();
//                        size_t packetSize = packet.size();
//                        Tasks* task = it->second->getTask();
//                        if(task && packetData[0]== 0x06){
//                            dataProcessor->getTaskInfo(packetData, packetSize, it->second);
//                        }else if (task && packetData[0] == 0x07) {
//                            dataProcessor->changeTowerGPT(packetData, packetSize, it->second);
//                        }
//                        else if(task && packetData[0]== 0x08){
//                            dataProcessor->getRecord(packetData, packetSize, it->second);
//                        } 
//                        else 
//                            dataProcessor->processDronesData(packetData, packetSize, it->second);
//                    }
//                    it->second->Update();
//                }else if(it->second->mlistenPort == 2345)
//                {
//                    LOG(ERROR) << "Port 2345 from web" ;
//                    dataProcessor->processReceivedData(reinterpret_cast<char*>(buffer), ret, it->second);
//                }   
//            } 
//            memset(buffer, 0, sizeof(buffer));
//        }
//        Sleep(2000);
//    }
//}
// void SessionManager::HeartBeats()
// {
//     while (true)
//     {
//         //TRACE("check hearbeat");
//         // std::lock_guard<std::mutex> lock(mt);
//         const char mPong[32] = {"* PONG \n"};
//         char buffer[1024];
//         // uint8_t buffer[1024];
//         for (auto it = m_sessMap.begin(); it != m_sessMap.end(); it++)
//         {
//             int ret = it->second->read(buffer, 1024);
//             uint8_t* uint8_buffer = reinterpret_cast<uint8_t*>(buffer);
//             // int ret = it->second->read(buffer, 1024);
//             if (6 == ret && strncmp(buffer, "* PING", 6) == 0){
//                 string str = buffer;
//                 //TRACE("check hearbeat : " + to_string(ret) + " info : " + str);
//                 it->second->write(mPong);
//                 it->second->Update();
//             }
//             else if (ret > 0 && !(strncmp(buffer, "* PING", 6) == 0)){
//                 //TRACE("bytesRead : " + to_string(ret));
//                 // //TRACE("Received data from client : " + std::string(buffer));
//                 // DataProcessor* dataProcessor = new DataProcessor();
//                 dataProcessor->processDronesData(buffer, ret);
//                 // delete it->second;
//                 // processReceivedData(buff, ret);
//                 //it->second->write("success !!!");
//             }
//             *(uint64_t *)buffer = 0;
//         }
//         // usleep(m_tmo/10*1000);//Hang for a while
//         sleep(2);
//     }
// }

void SessionManager::PfnCheckValidate(SessionManager *sessionManager)
{
    //sessionManager->CheckValidate();
}

void SessionManager::CheckValidate()
{
    // std::lock_guard<std::mutex> lock(mt);
    //while (true)
    //{
          //usleep(50 * 100000);
         // Sleep(50 * 100);
        // 14000000
       // ValidateSession();
   // }
}


map<int64_t, Session*> SessionManager::ValidateSession()
{
    //LOG(ERROR) << "ValidateSession";
    //std::vector<Session*> delVec;
    //if (!m_sessMap.empty()) {
    //    auto iter = m_sessMap.begin();
    //    while (iter != m_sessMap.end())
    //    {
    //        if (iter->second->Validate())
    //        {
    //            LOG(ERROR) << "== session is invalidate == ";
    //            delVec.push_back(iter->second);
    //        }
    //        iter++;
    //    }
    //}
    //LOG(ERROR) <<" number of sission need to delete:" << delVec.size();
    //for (size_t i = 0; i < delVec.size(); i++)
    //{
    //    Session* sessionToDelete = delVec[i];
    //    m_sessMap.erase(sessionToDelete->socket());
    //    auto snCodeIter = snCodeToSessionMap.begin();
    //    while (snCodeIter != snCodeToSessionMap.end())
    //    {
    //        if (snCodeIter->second == sessionToDelete)
    //        {
    //            LOG(ERROR) << "snCode to delete : " << snCodeIter->first;
    //            dataProcessor->deleteDroneFromDB(snCodeIter->first);
    //            snCodeToSessionMap.erase(snCodeIter++);
    //           
    //        }
    //        else
    //        {
    //            ++snCodeIter;
    //        }
    //    }
    //    auto dronesIter = dronesMap.find(sessionToDelete);
    //    if (dronesIter != dronesMap.end())
    //    {
    //        dronesMap.erase(dronesIter);
    //    }
    //    // 删除 Session 对象
    //    delete sessionToDelete;
    //}
    map<int64_t, Session*> m_sessMap1;

    return m_sessMap1;
}


// map<int64_t, Session *> SessionManager::ValidateSession()
// {
//     // std::lock_guard<std::mutex> lock(mt);
//     //TRACE("ValidateSession");
//     std::vector<int64_t> delVec;
//     Session* session;
//     auto iter = m_sessMap.begin();
//     for (; iter != m_sessMap.end(); iter++)
//     {
//         if (iter->second->Validate())
//         {
//             delVec.push_back(iter->first);
//             session = iter->second;
//         }          
//     }   
//     for (size_t i = 0; i < delVec.size(); i++)
//     {
//         m_sessMap.erase(delVec[i]);
//         // string snCodeToDelete = delVec[i]->getSnCode();
//         // snCodeToSessionMap.erase(delVec[i]);
//         //dronesMap.erase(delVec[i]);
//     }
//     return m_sessMap;
// }

void SessionManager::SendFileInfo(string key, string value)
{
}

int SessionManager::CheckSessionMapSize()
{
    return (int)m_sessMap.size();
}

void SessionManager::insertToDroneSessionMap(const string& snCode, Session* session) {
    //lock_guard<mutex> lock(mt);
    snCodeToSessionMap.insert(std::make_pair(snCode, session));
}

Session* SessionManager::getSessionBySnCode(const std::string& snCode) {
    auto it = snCodeToSessionMap.find(snCode);
    if (it != snCodeToSessionMap.end()) {
        return it->second;
    }
    return nullptr;
}



void SessionManager::insertToHeartbeatInfoSessionMap(Session* session, HeartbeatInfo heartbeatInfo) {
    //lock_guard<mutex> lock(mt);
    auto it = dronesMap.find(session);
    if(it != dronesMap.end())
    {
        dronesMap.erase(it);
    }
    dronesMap.insert(std::make_pair(session, heartbeatInfo));
    LOG(ERROR) << "dronesMap size : " << dronesMap.size();
}

// Session* SessionManager::getSessionBySnCode(const std::string& snCode) {
//     auto it = snCodeToSessionMap.find(snCode);
//     if (it != snCodeToSessionMap.end()) {
//         return it->second;
//     }
//     return nullptr;
// }