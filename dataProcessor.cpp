#include "dataProcessor.h"
#include <list>
#include <chrono>
#include <filesystem>
#include <sys/stat.h>
#include <ctime>
#include <cstring>
#include "log.h"
// #include "./algorithm/PointToLine.h"
// #include "./algorithm/PointToLineV2.h"
// #include "./algorithm/PointToLineV3.h"
using namespace std;
#define BUFFER_SIZE 10240
//#define IMPORT_ROUTE_FILE_LOCAL_PATH  "C:/Users/123/Desktop/origin_php/myphp/public/importRoute/";
//"C:/Users/Administrator/Desktop/newversion/myphp/myphp/public/importRoute/";
#define IMPORT_ROUTE_FILE_PATH "C:/Users/Administrator/Desktop/newversion/myphp/myphp/public/importRoute/";

static SQLiteConnection* SQLiteConnector;

template<class T>
void Serialization(std::stringstream& ss, const T& t){
 cereal::BinaryOutputArchive oarchive(ss); 
 oarchive(t);
}

template<class T>
void UnSerialization(std::stringstream& ss, T& t){
 cereal::BinaryInputArchive iarchive(ss); 
 iarchive(t);
}

template <class Archive>
void save(Archive &ar, const std::list<WaypointInformation> &list) {
    ar(list.size()); // Serialize the size of the list
    for (const auto &item : list) {
        ar(item); // Serialize each element of the list
    }
}

template <class Archive>
void load(Archive &ar, std::list<WaypointInformation> &list) {
    size_t size;
    ar(size); // Deserialize the size of the list
    list.clear();
    for (size_t i = 0; i < size; ++i) {
        WaypointInformation item;
        ar(item); // Deserialize each element of the list
        list.push_back(item);
    }
}

uint8_t* beforeSerialization(const uint8_t* data, int length)
{
    int newLength = length - 2;
    uint8_t* newData = new uint8_t[newLength];
    for (int i = 3; i < length; i++) {
        newData[i - 3] = data[i];
    }
    return newData;
}

DataProcessor::DataProcessor()
{
   
}

DataProcessor::~DataProcessor()
{
    
}

void DataProcessor::processReceivedData(const char* data, int length, Session* session) {
    LOG(ERROR) << "==== processReceivedData ==== " ;
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
    std::string receivedData(data, length);
    LOG(ERROR) << "receivedData : " << receivedData;
    try {
        nlohmann::json jsonData = nlohmann::json::parse(receivedData);
        if (jsonData.contains("controller")) {
            std::string controller = jsonData["controller"];
            LOG(ERROR) << "controller : " << controller;
            if(!controller.empty())
            {
                if(controller == "get_drone")
                {
                    //TRACE("controller : " + controller);
                    uint8_t cmd = 0x0b;
                    //TRACE("cmd : " + to_string(cmd));
                    DroneINFO droneInfo;
                    droneInfo.snCode = jsonData["data"]["droneSncode"];
                    LOG(ERROR) << "droneInfo.snCode" << droneInfo.snCode;
                    droneInfo.droneType = jsonData["data"]["droneType"];
                    LOG(ERROR) << "droneInfo.droneType" << droneInfo.droneType;
                    droneInfo.lensType = jsonData["data"]["lensType"];
                    LOG(ERROR) << "droneInfo.lensType" << droneInfo.lensType;
                    string maxSpeed = jsonData["data"]["maxSpeed"];
                    //droneInfo.maxSpeed = atof(maxSpeed.c_str());
                    droneInfo.maxSpeed = stof(maxSpeed);
                    LOG(ERROR) << "droneInfo.maxSpeed" << droneInfo.maxSpeed;
                   // LOG(ERROR) << "droneInfo.maxSpeed" << droneInfo.maxSpeed;
                    droneInfo.systemVersion = jsonData["data"]["systemVersion"];
                    //TRACE("droneInfo.systemVersion  : " + droneInfo.systemVersion);

                    stringstream ss;
                    {
                        cereal::BinaryOutputArchive oarchive(ss);
                        oarchive(droneInfo);
                    }

                    //TRACE("begin insert to info");
                    std::vector<uint8_t> info;
                    info.push_back(cmd);
                    auto serializedData = ss.str();
                    size_t dataLength = serializedData.size();
                    //TRACE("serialData: " + to_string(serializedData.size()));
                    uint8_t highByte = (dataLength >> 8) & 0xFF;
                    uint8_t lowByte = dataLength & 0xFF;
                    info.push_back(highByte);
                    info.push_back(lowByte);
                    LOG(ERROR) << "finished insert to info";
                    info.insert(info.end(), serializedData.begin(), serializedData.end());

                    SessionManager *m_sessionManager = SessionManager::GetInstance(); 
                    Session* targetSession = m_sessionManager->getSessionBySnCode(droneInfo.snCode);
                    if (targetSession != nullptr) {
                        send(targetSession->socket(), reinterpret_cast<const char*>(info.data()), info.size(), 0);
                    } else {
    
                        LOG(ERROR) << "session unexisted";
                    }
                    for (const uint8_t byte : info) {
                        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
                    }
                    std::cout << std::dec << std::endl;
                    ss.str("");
                    ss.clear();
                }
                else if(controller == "index"){
                    SessionManager* m_sessionManager = SessionManager::GetInstance();
                    LOG(ERROR) << "upload task to drone : " << controller; 
                    string snCode = jsonData["data"]["snCode"];
                    string submissionID = jsonData["data"]["submissionID"];
                    int submissionID_int = stoi(submissionID);
                    LOG(ERROR) << "missionID : " << submissionID_int;
                    string missionType = jsonData["data"]["missionType"];
                    int missionType_int = stoi(missionType);
                    LOG(ERROR) << "missionType_int : " << missionType_int;
                    if (missionType_int == TASKTYPENOTUPLOADED || missionType_int == TASKTYPEDISTRIBUTIONDOT ) {
                        LOG(ERROR) << " === point task === ";
                        uint8_t level = 1;
                     /*   if (missionType_int == TASKTYPENOTUPLOADED) {
                            level = 0;
                        }else if (missionType_int == TASKTYPEDISTRIBUTIONDOT) {
                            level = 1;
                        }*/
                        string realData;
                        uint8_t cmd = 0x04;
                        realData = cmd;
                        uint8_t bRTK = 1;
                        list<int> towerList = getTowerList(submissionID_int);
                        LOG(ERROR) <<"towerList size :" << towerList.size();
                        stringstream ss;
                        Serialization(ss, towerList);
                        Tasks* task = new Tasks();
                        task->setSubmissionID(jsonData["data"]["submissionID"]);
                        task->setMissionType(jsonData["data"]["missionType"]);
                     /* stringstream missionID;
                        Serialization(missionID, submissionID_int);
                        string serializedData = missionID.str();
                        vector<uint8_t> info;
                        for (size_t i = 0; i < 4; ++i) {
                            uint8_t byte = static_cast<uint8_t>(serializedData[i]);
                            info.push_back(byte);
                            LOG(ERROR) << "id";
                            LOG(ERROR) << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
                        }*/

                        auto serial_data = ss.str();
                        LOG(ERROR) << "serial_data : " << serial_data.size();
                        uint16_t data_len = serial_data.size() + sizeof(bRTK) + sizeof(submissionID_int) + sizeof(level);
                        LOG(ERROR) << "data_len : " << data_len << " size of data_len ：" << sizeof(data_len);
                        LOG(ERROR) << "before resize realData : " << realData.size();
                        realData.resize(realData.size() + sizeof(data_len) + sizeof(bRTK) + sizeof(submissionID_int) + sizeof(level));
                        LOG(ERROR) << "after resize realData : " << realData.size();
                        uint16_t bigEndianValue = ((data_len & 0xFF00) >> 8) | ((data_len & 0x00FF) << 8);

                        int len = sizeof(cmd);
                        memcpy((char*)realData.data() + len , &bigEndianValue, sizeof(bigEndianValue));
                        len += sizeof(bigEndianValue);
                        memcpy((char*)realData.data() + len, (char*)&level, sizeof(level));
                        len += sizeof(level);
                        memcpy((char*)realData.data() + len, (char*)&bRTK, sizeof(bRTK));
                        len += sizeof(bRTK);
                        memcpy((char*)realData.data() + len, (char*)&submissionID_int, sizeof(submissionID_int));
                        LOG(ERROR) << "sizeof(submissionID_int) :" << sizeof(submissionID_int);
                        len += sizeof(submissionID_int);
       
                        std::copy(serial_data.begin(), serial_data.end(), std::back_inserter(realData));
                        LOG(ERROR) << "realData :" << realData.size();
                        Session* targetSession = m_sessionManager->getSessionBySnCode(snCode);
                        if (targetSession != nullptr) {
                            SSIZE_T bytesSent = send(targetSession->socket(), realData.data(), realData.size(), 0);
                            if (bytesSent == realData.size()) {
                                targetSession->setTask(task);
                                LOG(ERROR) << "point task upload successfully " << bytesSent;
                            }
                            else if (bytesSent == -1) {
                                LOG(ERROR) << "Failed to send data. Error code: " << errno;
                            }
                            else {
                                LOG(ERROR) << "Partial data sent. Sent bytes:  " << bytesSent;
                            }
                        }
                        for (const uint8_t byte : realData) {
                            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
                        }

                    }
                    else  {
                        uint8_t cmd = 0x06;
                        size_t dataLength = 0;
                        uint8_t highByte = (dataLength >> 8) & 0xFF;
                        uint8_t lowByte = dataLength & 0xFF;
                        Tasks* task = new Tasks();
                        task->setSubmissionID(jsonData["data"]["submissionID"]);
                        task->setMissionType(jsonData["data"]["missionType"]);
                        task->setRtk(jsonData["data"]["rtk"]);
                        task->setFlightType(jsonData["data"]["flight"]);
                        task->setTimestamp(jsonData["data"]["timestamp"]);
                        task->setWhetherRecord(jsonData["data"]["record"]);
                        task->setImgwidth(jsonData["data"]["imgwidth"]);
                        task->setImgheigth(jsonData["data"]["imgheight"]);
                        LOG(ERROR) << jsonData["data"]["return"];
                        if (jsonData["data"]["return"] == 254) {
                            double landingLongtitude = jsonData["data"]["landingLongtitude"];
                            double landingLatitude = jsonData["data"]["landingLatitude"];
                            double landingAltitude = jsonData["data"]["landingAltitude"];
                            if (!isnan(landingLongtitude) && !isnan(landingLatitude) && !isnan(landingAltitude))
                            {
                                Gps returnGps;
                                returnGps.stc_alt = landingAltitude;
                                returnGps.stc_lat = landingLatitude;
                                returnGps.stc_lon = landingLongtitude;
                                task->setReturnGPS(returnGps);
                            }
                        }
                     
                        task->setReturnCode(jsonData["data"]["return"]);
                        Session* targetSession = m_sessionManager->getSessionBySnCode(snCode);
                        vector<uint8_t> info;
                        info.push_back(cmd);
                        info.push_back(highByte);
                        info.push_back(lowByte);
                        LOG(ERROR) << "info size : " << info.size();
                        for (const uint8_t byte : info) {
                            LOG(ERROR) << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
                        }
                        if (targetSession != nullptr) {
                            targetSession->setTask(task);
                            SSIZE_T bytesSent = send(targetSession->socket(), reinterpret_cast<const char*>(info.data()), info.size(), 0);
                            if (bytesSent == static_cast<SSIZE_T>(info.size())) {
                                LOG(ERROR) << "task upload successfully " << bytesSent;
                            }
                            else if (bytesSent == -1) {
                                LOG(ERROR) << "Failed to send data. Error code: " << errno;

                            }
                            else {
                                LOG(ERROR) << "Partial data sent. Sent bytes:  " << bytesSent;
                            }
                        }
                        else {
                            LOG(ERROR) << "session unexisted ";
                        }
                        for (const uint8_t byte : info) {
                            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
                        }
                    }
                }
                else if(controller == "takeoffLink"){
                    LOG(ERROR) << "takeoffLink controller : " << controller;
                    uint8_t cmd = 0x01;
                    size_t dataLength = 0;
                    uint8_t highByte = (dataLength >> 8) & 0xFF;
                    uint8_t lowByte = dataLength & 0xFF;
                    string snCode = jsonData["data"]["snCode"];
                    LOG(ERROR) << "snCode : " << snCode;
                    SessionManager *m_sessionManager = SessionManager::GetInstance(); 
                    Session* targetSession = m_sessionManager->getSessionBySnCode(snCode);
                    vector<uint8_t> info;
                    info.push_back(cmd); 
                    info.push_back(highByte); 
                    info.push_back(lowByte);
                    if (targetSession != nullptr) {
                        SSIZE_T bytesSent = send(targetSession->socket(), reinterpret_cast<const char*>(info.data()), info.size(), 0);
                        if (bytesSent == static_cast<SSIZE_T>(info.size())) {
                            LOG(ERROR) << "task takeoffLink successfully " << bytesSent;
                        } else if (bytesSent == -1) {
                            LOG(ERROR) << "Failed to send takeoffLink data. Error code: " << errno;
                        } else {
                            LOG(ERROR) << "Partial data sent. Sent bytes: " << bytesSent;
                        }
                    } else {
                        LOG(ERROR) << "session unexisted" ;
                    }
                }
                else if (controller == "sync_file" || controller == "clear_log" || controller == "picture_download") {
                    logInfoSendToDrone(jsonData);
                }
                else if (controller == "update_version") {
                    updateSendToDrone(jsonData);
                }
                else if (controller == "kill_parent") {
                    killParentSendToDrone(jsonData);
                }
                else if (controller == "running_download") {
                    runningStatusDownloadSendToDrone(jsonData);
                }
                else if (controller == "photograph_test") {
                    photographTestSendToDrone(jsonData);
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
    }
}

void DataProcessor::runningStatusDownloadSendToDrone(nlohmann::json jsonData) {
    LOG(ERROR) << " === runningStatusDownloadSendToDrone ===";
    string controller = jsonData["controller"];
    uint8_t cmd = 0x12;
    string snCode = jsonData["data"]["snCode"];
    LOG(ERROR) << " snCode :" << snCode;
    uint16_t data_len = 2;
    uint16_t bigEndianValue = ((data_len & 0xFF00) >> 8) | ((data_len & 0x00FF) << 8);
    uint8_t magic1 = 0xda;
    uint8_t magic2 = 0xda;
    SessionManager* m_sessionManager = SessionManager::GetInstance();
    Session* targetSession = m_sessionManager->getSessionBySnCode(snCode);
    string realData;
    realData = cmd;
    realData.resize(realData.size() + sizeof(bigEndianValue) + sizeof(magic1) + sizeof(magic2));
    int len = sizeof(cmd);
    memcpy((char*)realData.data() + len, &bigEndianValue, sizeof(bigEndianValue));
    len += sizeof(bigEndianValue);
    memcpy((char*)realData.data() + len, &magic1, sizeof(magic1));
    len += sizeof(magic1);
    memcpy((char*)realData.data() + len, &magic2, sizeof(magic2));
    len += sizeof(magic2);
    if (targetSession != nullptr) {
        SSIZE_T bytesSent = send(targetSession->socket(), reinterpret_cast<const char*>(realData.data()), realData.size(), 0);
        if (bytesSent == static_cast<SSIZE_T>(realData.size())) {
            LOG(ERROR) << "send kill parent successfully " << bytesSent;
        }
        else if (bytesSent == -1) {
            LOG(ERROR) << "Failed to send  kill parent. Error code: " << errno;
        }
        else {
            LOG(ERROR) << "Partial data sent. Sent bytes: " << bytesSent;
        }

    }
    else {
        LOG(ERROR) << "session unexisted";
    }
    for (const uint8_t byte : realData) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
    }
}

void DataProcessor::photographTestSendToDrone(nlohmann::json jsonData) {
    LOG(ERROR) << " === photographTestSendToDrone  ===";
    string controller = jsonData["controller"];
    uint8_t cmd = 0x13;
    string snCode = jsonData["data"]["snCode"];
    LOG(ERROR) << " snCode :" << snCode;
    uint16_t data_len = 2;
    uint16_t bigEndianValue = ((data_len & 0xFF00) >> 8) | ((data_len & 0x00FF) << 8);
    uint8_t magic1 = 0xdb;
    uint8_t magic2 = 0xdb;
    SessionManager* m_sessionManager = SessionManager::GetInstance();
    Session* targetSession = m_sessionManager->getSessionBySnCode(snCode);
    string realData;
    realData = cmd;
    realData.resize(realData.size() + sizeof(bigEndianValue) + sizeof(magic1) + sizeof(magic2));
    int len = sizeof(cmd);
    memcpy((char*)realData.data() + len, &bigEndianValue, sizeof(bigEndianValue));
    len += sizeof(bigEndianValue);
    memcpy((char*)realData.data() + len, &magic1, sizeof(magic1));
    len += sizeof(magic1);
    memcpy((char*)realData.data() + len, &magic2, sizeof(magic2));
    len += sizeof(magic2);
    if (targetSession != nullptr) {
        SSIZE_T bytesSent = send(targetSession->socket(), reinterpret_cast<const char*>(realData.data()), realData.size(), 0);
        if (bytesSent == static_cast<SSIZE_T>(realData.size())) {
            LOG(ERROR) << "send kill parent successfully " << bytesSent;
        }
        else if (bytesSent == -1) {
            LOG(ERROR) << "Failed to send  kill parent. Error code: " << errno;
        }
        else {
            LOG(ERROR) << "Partial data sent. Sent bytes: " << bytesSent;
        }

    }
    else {
        LOG(ERROR) << "session unexisted";
    }
    for (const uint8_t byte : realData) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
    }
}

void DataProcessor::updateSendToDrone(nlohmann::json jsonData) {
    LOG(ERROR) << " === updateSendToDrone ===";
    string controller = jsonData["controller"];
    uint8_t cmd = 0x10;
    string snCode = jsonData["data"]["droneSncode"];
    LOG(ERROR) << " snCode :" << snCode;
    uint16_t data_len = 2;
    uint16_t bigEndianValue = ((data_len & 0xFF00) >> 8) | ((data_len & 0x00FF) << 8);
    uint8_t magic1 = 0xce;
    uint8_t magic2 = 0xce;
    SessionManager* m_sessionManager = SessionManager::GetInstance();
    Session* targetSession = m_sessionManager->getSessionBySnCode(snCode);
    string realData;
    realData = cmd;
    realData.resize(realData.size() + sizeof(bigEndianValue) + sizeof(magic1) + sizeof(magic2));
    int len = sizeof(cmd);
    memcpy((char*)realData.data() + len, &bigEndianValue, sizeof(bigEndianValue));
    len += sizeof(bigEndianValue);
    memcpy((char*)realData.data() + len, &magic1, sizeof(magic1));
    len += sizeof(magic1);
    memcpy((char*)realData.data() + len, &magic2, sizeof(magic2));
    len += sizeof(magic2);
    if (targetSession != nullptr) {
        SSIZE_T bytesSent = send(targetSession->socket(), reinterpret_cast<const char*>(realData.data()), realData.size(), 0);
        if (bytesSent == static_cast<SSIZE_T>(realData.size())) {
            LOG(ERROR) << "send get log successfully " << bytesSent;
        }
        else if (bytesSent == -1) {
            LOG(ERROR) << "Failed to send get log. Error code: " << errno;
        }
        else {
            LOG(ERROR) << "Partial data sent. Sent bytes: " << bytesSent;
        }

    }
    else {
        LOG(ERROR) << "session unexisted";
    }
    for (const uint8_t byte : realData) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
    }
}

void DataProcessor::killParentSendToDrone(nlohmann::json jsonData) {
    LOG(ERROR) << " === killParentSendToDrone ===";
    string controller = jsonData["controller"];
    uint8_t cmd = 0x11;
    string snCode = jsonData["data"]["droneSncode"];
    LOG(ERROR) << " snCode :" << snCode;
    uint16_t data_len = 2;
    uint16_t bigEndianValue = ((data_len & 0xFF00) >> 8) | ((data_len & 0x00FF) << 8);
    uint8_t magic1 = 0xcf;
    uint8_t magic2 = 0xcf;
    SessionManager* m_sessionManager = SessionManager::GetInstance();
    Session* targetSession = m_sessionManager->getSessionBySnCode(snCode);
    string realData;
    realData = cmd;
    realData.resize(realData.size() + sizeof(bigEndianValue) + sizeof(magic1) + sizeof(magic2));
    int len = sizeof(cmd);
    memcpy((char*)realData.data() + len, &bigEndianValue, sizeof(bigEndianValue));
    len += sizeof(bigEndianValue);
    memcpy((char*)realData.data() + len, &magic1, sizeof(magic1));
    len += sizeof(magic1);
    memcpy((char*)realData.data() + len, &magic2, sizeof(magic2));
    len += sizeof(magic2);
    if (targetSession != nullptr) {
        SSIZE_T bytesSent = send(targetSession->socket(), reinterpret_cast<const char*>(realData.data()), realData.size(), 0);
        if (bytesSent == static_cast<SSIZE_T>(realData.size())) {
            LOG(ERROR) << "send kill parent successfully " << bytesSent;
        }
        else if (bytesSent == -1) {
            LOG(ERROR) << "Failed to send  kill parent. Error code: " << errno;
        }
        else {
            LOG(ERROR) << "Partial data sent. Sent bytes: " << bytesSent;
        }

    }
    else {
        LOG(ERROR) << "session unexisted";
    }
    for (const uint8_t byte : realData) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
    }
}

void DataProcessor::logInfoSendToDrone(nlohmann::json jsonData) {
    LOG(ERROR) << " === getLogSendToDrone ===";
    string controller = jsonData["controller"];
    uint8_t cmd = 0;
    if (controller == "sync_file") {
        cmd = 0x0D;
    }
    else if (controller == "clear_log") {
        cmd = 0x0E;
    }
    else if (controller == "picture_download") {
        cmd = 0x0F;
    }
    string snCode = jsonData["data"]["snCode"];
    uint32_t beginDate = jsonData["data"]["beginDate"];
    uint32_t finishDate = jsonData["data"]["finishDate"];
    uint16_t data_len = sizeof(beginDate) + sizeof(finishDate);
    uint16_t bigEndianValue = ((data_len & 0xFF00) >> 8) | ((data_len & 0x00FF) << 8);
    SessionManager* m_sessionManager = SessionManager::GetInstance();
    Session* targetSession = m_sessionManager->getSessionBySnCode(snCode);
    string realData;
    realData = cmd;
    realData.resize(realData.size() + sizeof(bigEndianValue) + sizeof(beginDate) + sizeof(finishDate));
    int len = sizeof(cmd);
    memcpy((char*)realData.data() + len, &bigEndianValue, sizeof(bigEndianValue));
    len += sizeof(bigEndianValue);
    memcpy((char*)realData.data() + len, (char*)&beginDate, sizeof(beginDate));
    len += sizeof(beginDate);
    memcpy((char*)realData.data() + len, (char*)&finishDate, sizeof(finishDate));
    len += sizeof(finishDate);
    if (targetSession != nullptr) {
        SSIZE_T bytesSent = send(targetSession->socket(), reinterpret_cast<const char*>(realData.data()), realData.size(), 0);
        if (bytesSent == static_cast<SSIZE_T>(realData.size())) {
            LOG(ERROR) << "send get log successfully " << bytesSent;
        }
        else if (bytesSent == -1) {
            LOG(ERROR) << "Failed to send get log. Error code: " << errno;
        }
        else {
            LOG(ERROR) << "Partial data sent. Sent bytes: " << bytesSent;
        }

    }
    else {
        LOG(ERROR) << "session unexisted";
    }
    for (const uint8_t byte : realData) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
    }

}

void DataProcessor::processDronesData(const uint8_t* data, int length, Session* session) {
    LOG(ERROR) << "processDronesData" ;
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
    if(length == 0)
    {
        LOG(ERROR) << "socket error in process data";
        return;
    }
    int cmd = data[0];
    Drones* drone = new Drones();
    uint8_t* serializationData = beforeSerialization(data,length);
    if(cmd == 0x0a){
       processSNCode(serializationData, length, SQLiteConnector, drone, session);
    } else if(cmd == 0x00){
        getHeartbeatInfo(serializationData, length, drone, session);
    } else if (cmd == 0x0d) {
        //session->setWhetherCheckHearbeat(false);
        processLog(serializationData, length, session);
    }

    delete[] serializationData;
    delete drone;
}

void DataProcessor::processUpgrade(const uint8_t* data, int length, Session* session){
    LOG(ERROR) << " ==== processUpgrade ====";
    LOG(ERROR) << "data : " << data;
    char snCode[24] = {0};
    memcpy(snCode, data, 24);
    LOG(ERROR) << "snCode :" << snCode;
    char versionNumber[12] = {0};
    memcpy(versionNumber, data + 24, 12);
    LOG(ERROR) << "versionNumber :" << versionNumber;

    stringstream ss;
    int seekg = 0;
    ss << string((const char*)data + 36, sizeof(seekg));
    UnSerialization(ss, seekg);
    LOG(ERROR) << "seekg : " << seekg;


    char confirm = 0;
    vector<vector<string>> versionAndFilePath = querySystemVersionAndFilePath();
    if(!versionAndFilePath.empty()){
        string dbVersionNumber = versionAndFilePath[0][0];
        LOG(ERROR) << "dbVersionNumber :" << dbVersionNumber;
        string dbFilePath = versionAndFilePath[0][1];
        // int result = strncmp(versionNumber, dbVersionNumber.c_str(), dbVersionNumber.size());

        int versionNumberFromDrone;
        int a, b, c;
        if (!str2Version(versionNumber, a, b, c))
            versionNumberFromDrone = 0;
        else {
            versionNumberFromDrone = a * 1000000 + b * 1000 + c;
        }
        int versionNumberFromDB;
        int one,two, three;
        if (!str2Version(dbVersionNumber, one, two, three))
            versionNumberFromDB = 0;
        else {
            versionNumberFromDB = one * 1000000 + two * 1000 + three;
        }

        LOG(ERROR) << "versionNumberFromDrone :" << versionNumberFromDrone;
        LOG(ERROR) << "versionNumberFromDB :" << versionNumberFromDB;

        int result;
        if (versionNumberFromDrone > versionNumberFromDB) {
            result = -1;
        }
        else if (versionNumberFromDrone < versionNumberFromDB) {
            result = 1;
        }
        else if (versionNumberFromDrone == versionNumberFromDB) {
            result = 0;
        }

        //int compareResult = versionNumberCompareWithDB(versionNumber, dbVersionNumber);
        ConfigFile* mConfig = new ConfigFile("./config.txt");
        int rollBack = whetherRoolBack(mConfig);
        LOG(ERROR) << "rollBack : " << rollBack;
        if(rollBack == 1){
            LOG(ERROR) << "need roll back ";
            confirm = 2;
            send(session->socket(), &confirm, 1, 0);
        } else if(result == 0 || result == -1){
            LOG(ERROR) << "cancel update : " ;
            /* else if (result == 0 && compareResult == 1) { */
            send(session->socket(), &confirm, 1, 0);
        } else if (result == 1){
            LOG(ERROR) << "need update  " ;
            string md5Hash = calculateMD5byOpenSSL(dbFilePath);
            LOG(ERROR) << "md5Hash in string :" << md5Hash;
            sendFile(session->socket(), dbFilePath, md5Hash, seekg);
        }
        delete mConfig;
    }
   
}

bool DataProcessor::str2Version(std::string str, int& ione, int& itwo, int& ithree)
{
    std::string::size_type onepos = 0;
    if ((onepos = str.find(".", 0)) == std::string::npos) {
        return false;
    }
    int versionone = atoi(str.substr(0, onepos).c_str());

    std::string twostr = str.substr(onepos + 1, str.length() - onepos - 1);
    std::string::size_type twopos = 0;
    if ((twopos = twostr.find(".", 0)) == std::string::npos) {
        return false;
    }
    int versiontwo = atoi(twostr.substr(0, twostr.length() - twopos - 1).c_str());


    std::string threestr = twostr.substr(twopos + 1, twostr.length() - twopos - 1);
    int versionthree = atoi(threestr.c_str());
    ione = versionone;
    itwo = versiontwo;
    ithree = versionthree;
    return true;
}

int DataProcessor::whetherRoolBack(ConfigFile* mConfig){
    LOG(ERROR) << " ==== rollBack ====";
    int rollBack = mConfig->Read("ROLLBACK", rollBack);
    return rollBack;
}

vector<vector<string>> DataProcessor::querySystemVersionAndFilePath(){
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance(); 
    const char* query = "SELECT Version,FilePath FROM UpgradeManager;";
    vector<vector<string>> results;
    if (SQLiteConnector->executeQueryWithPlaceholder(query, {}, results) && results.size() > 0) {
        return results;
    } else {
        return results;
    }
}

bool DataProcessor::versionNumberCompareWithDB(const char* versionNumber, const string dbVersionNumber){
    int dbLength = dbVersionNumber.size() + 1;
    //TRACE("dbLength :" + to_string(dbLength));
    int versionLength = strlen(versionNumber);
    //TRACE("versionLength :" + to_string(versionLength));
    for(int i = dbLength; i < versionLength; i++){
        if(versionNumber[i] != '0')
        {
            return false;
        }
    }
    return true;
}

void DataProcessor::sendFile(int clientSocket, const std::string& filePath, string md5Hash, int seekg) {

    ifstream file(filePath, std::ios::binary | std::ios::ate);
    //ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        //TRACE( "Error opening file : " + filePath);
        return;
    }
    const char * path = filePath.c_str();
    struct stat fileInfo; 
    if (stat(path, &fileInfo) != 0) { 
        //TRACE( "Error opening file : " ); 
    } 
    //size_t fileSize = fileInfo.st_size;

    size_t totalFileSize = fileInfo.st_size;
    int remainingFileSize = totalFileSize - seekg;
    if (remainingFileSize < 0) {
        LOG(ERROR) << "remainingFileSize < 0 : " << remainingFileSize;
        return;
    }
    LOG(ERROR) << "totalFileSize : " << totalFileSize;
    LOG(ERROR) << "seekg : " << seekg;
    LOG(ERROR) << "remainingFileSize : " << remainingFileSize;
    //TRACE("fileSize: " + std::to_string(fileSize));
    //TRACE("good1: " + std::to_string(file.good()));

    /*
        comfirm :           1 for update 
                            2 for fail

        UpgradePacket :     char filePath[256];
                            uint32_t fileSize;
                            char md5[32];
    */
    
    char comfirm = 1;
    int a = send(clientSocket, &comfirm, 1, 0);

    UpgradePacket packet;
    //packet.fileSize = file.tellg();
    packet.fileSize = remainingFileSize;
    
    LOG(ERROR) << "fileSize: " << packet.fileSize;

    //file.seekg(0, std::ios::beg);
    file.seekg(seekg);
    //TRACE("fileSize: " + std::to_string(packet.fileSize));

    char* buffer = new char[packet.fileSize];
    LOG(ERROR) << "packet.fileSize " << packet.fileSize;
    file.read(buffer, packet.fileSize);
    if (file.good()) {
        LOG(ERROR) << "file is good !!!" ;
        string realFilePath = findDestinationFilePathFromDB();
        LOG(ERROR) << "realFilePath : " << realFilePath;
        strcpy(packet.filePath, realFilePath.c_str());
        memcpy(packet.md5, md5Hash.c_str(), 32);
        LOG(ERROR) << " sizeof(UpgradePacket) : " << sizeof(UpgradePacket);
        int upgradePacketSize = sizeof(UpgradePacket);
        LOG(ERROR) << "filePath " << packet.filePath;
        LOG(ERROR) << "fileSize " << packet.fileSize;
        LOG(ERROR) << "md5 " << sizeof(packet.md5) << " " << md5Hash;
        LOG(ERROR) << "struct size " << upgradePacketSize;
        send(clientSocket, reinterpret_cast<const char*>(&packet), upgradePacketSize, 0);// &packet
    } else {
        LOG(ERROR) << "file is bad !!!" ;
    }
    sendFileBy10KB(buffer, packet.fileSize, clientSocket);
    delete[] buffer;
    file.close();
}

string DataProcessor::findDestinationFilePathFromDB() {
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
    string destinationFilePath = "";
    string selectQuery = "SELECT destinationFilePath FROM UpgradeManager";
    vector<vector<string>> results;
    if (SQLiteConnector->executeQueryWithPlaceholder(selectQuery, {}, results) && results.size() > 0) {
        destinationFilePath = results[0][0];
    }
    else {
        LOG(ERROR) <<"Error executing query or no results.";
    }
    return destinationFilePath;
}

void DataProcessor::sendFileBy10KB(char* buffer, int bufferSize, int clientSocket) {

    int totalBytesSent = 0;
    int i = 1;
    //int count = 1000;
    //int scount = 0;
    LOG(ERROR) << "bufferSize :" << bufferSize;
    size_t bytesToSend = min(1024U * 10, static_cast<unsigned int>(bufferSize));
    LOG(ERROR) << "bytes To Send :" << bytesToSend;
    u_long mode = 0;
    ioctlsocket(clientSocket, FIONBIO, &mode);
    while (totalBytesSent < bufferSize) {
        int bytesSent = send(clientSocket, buffer + totalBytesSent, bytesToSend, 0);// MSG_DONTWAIT
        LOG(ERROR) << "i :" << i <<  " bytes Sent : " << bytesSent;
        if (bytesSent == -1) {

            if (bytesSent == SOCKET_ERROR) {
                LOG(ERROR) << "SOCKET ERROR" ;
                int error = WSAGetLastError();
                LOG(ERROR) << "ERROR" << error;
            }
            LOG(ERROR) << errno;

            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                
                LOG(ERROR) << "retry send data ";
                this_thread::sleep_for(chrono::seconds(3));
                continue;
            }
            else {
                LOG(ERROR) << "Error sending data: :" << strerror(errno);
            }
         
            break;
        } else {
            LOG(ERROR) << "Bytes sent:" << bytesSent;
            totalBytesSent += bytesSent;
            LOG(ERROR) << "totalBytesSent" << totalBytesSent;
        }
        bytesToSend = min(1024U * 10, static_cast<unsigned int>(bufferSize - totalBytesSent));
     /* scount++; 
        if(scount == count){
            closesocket(clientSocket);
            scount = 0;
        }*/
        i++;
        Sleep(3);
    }
    LOG(ERROR) << "=== finish ==== ";
    // //TRACE("===  send file by 10 kb === ");
    // char buffer[BUFFER_SIZE];
    // //TRACE("buffer: " + to_string(sizeof(buffer)));
    // //TRACE("file good : " + std::to_string(file.good()));
    // //TRACE("eof : " + to_string(file.eof()));
    // if (!file) {
    //     //TRACE("Error opening or reading the file.");
    //     return;
    // }
    // file.seekg(0, std::ios::beg);
    // file.read(buffer, BUFFER_SIZE);
    // if (file.peek() == ifstream::traits_type::eof()) {
    //     //TRACE("File is empty.");
    //     return;
    // }   
    // //TRACE("eof : " + to_string(file.eof()));
    // //TRACE("gcount 1: " + to_string(file.gcount()));
    // while (file.read(buffer, BUFFER_SIZE)) {
    //     int bytesRead = file.gcount();
    //     //TRACE("gcount: " + std::to_string(bytesRead));
    //     if (bytesRead > 0) {
    //         int bytesSent = send(clientSocket, buffer, bytesRead, 0);
    //         if (bytesSent == -1) {
    //             //TRACE(" Error sending data:" + to_string(bytesSent));
    //             break;
    //         }
    //         else 
    //             //TRACE("bytesSent :" + to_string(bytesSent));
    //         for (int i = 0; i < 10; i++) {
    //             std::cout << std::hex << (uint)(buffer[i]) << " ";
    //         }
    //         std::cout << std::endl;
    //     }
    // }

}

void DataProcessor::getTaskInfo(const uint8_t* data, int length, Session* session)
{

   LOG(ERROR) << "====  get TaskInfo ====";
   Gps gps;
   //Tasks* task= new Tasks();
    try{
        if(length > 3){
            uint8_t* data1 =  beforeSerialization(data,length);
            Tasks* task = session->getTask();
            vector<WaypointInformation> waypointVec;
            int submissionID = stoi(task->getSubmissionID());
            //int missionType = stoi(task->getMissionType());
            //if (missionType == TASKTYPENOTUPLOADED || missionType == TASKTYPEDISTRIBUTIONDOT || missionType == TASKTYPEIMPORTROUTE) {
            //    string realData;
            //    uint8_t cmd = 0x04;
            //    //realData = cmd;
            //    int bRTK = 1;
            //    list<int> towerList = getTowerList(submissionID);
            //    stringstream ss;
            //    Serialization(ss, towerList);
            //    auto serial_data = ss.str();
            //    uint16_t data_len = serial_data.size() + sizeof(bRTK) + sizeof(submissionID);
            //    realData.resize(realData.size() + sizeof(data_len) + sizeof(bRTK) + sizeof(submissionID));
            //    uint16_t bigEndianValue = ((data_len & 0xFF00) >> 8) | ((data_len & 0x00FF) << 8);
            //    memcpy((void*)(realData.data() + sizeof(cmd)), &data_len, sizeof(data_len));
            //    memcpy((char*)(realData.data() + sizeof(cmd) + sizeof(data_len)), (char*)&bRTK, sizeof(bRTK));
            //    memcpy((char*)(realData.data() + sizeof(submissionID) + sizeof(data_len) + sizeof(bRTK)), (char*)&submissionID, sizeof(submissionID));
            //    std::copy(serial_data.begin(), serial_data.end(), std::back_inserter(realData));
            //    SSIZE_T bytesSent = send(session->socket(), realData.data(), realData.size(), 0);
            //}
            //TRACE("submissionID in getTaskInfo: " + to_string(submissionID));
            LOG(ERROR) << "submissionID in getTaskInfo : " << submissionID;
            std::stringstream gpsInfo;
            gpsInfo.write((const char*)data1, length);
            UnSerialization(gpsInfo,gps);
            LOG(ERROR) << std::fixed << std::setprecision(10) << "gps.stc_lon : " << gps.stc_lon;
            LOG(ERROR) << std::fixed << std::setprecision(10) << "gps.stc_lat : " << gps.stc_lat;
            LOG(ERROR) << std::fixed << std::setprecision(10) << "gps.stc_alt : " << gps.stc_alt;

            //gps.stc_lon = 105.8937155;
            //gps.stc_lat = 29.267501;
            //gps.stc_alt = 289.872;

            WaypointInformation m_rtkhome_point;
            m_rtkhome_point.stc_x = gps.stc_lon;
	        m_rtkhome_point.stc_y = gps.stc_lat;
	        m_rtkhome_point.stc_z = gps.stc_alt;

            LOG(ERROR) << "set frist tower " ;
            TowerList firstTower = getFirstTower(submissionID);
            LOG(ERROR) << "firstTower towerID : " << firstTower.towerID;
            LOG(ERROR) << "firstTower towerName : " << firstTower.towerName;

            //LOG(ERROR) << "firstTower towerID : " << firstTower.towerID;
            //TRACE("towerID : " + to_string(firstTower.towerID));
            //TRACE("towerName : " + firstTower.towerName);
            //TRACE("dz : " + to_string(dz));

            LongitudeAndLatitude current_drone_position;
	        current_drone_position.stc_altitude = m_rtkhome_point.stc_z;
	        current_drone_position.stc_longitude = m_rtkhome_point.stc_x;
	        current_drone_position.stc_latitude = m_rtkhome_point.stc_y;

            CDatumPoint datumPoint;
	        datumPoint.setOrigin(firstTower.init_longitude, firstTower.init_latitude, firstTower.init_altitude, true);

            double dx = datumPoint.longitudeAndLatitudeToDatumPoint(current_drone_position).stc_x;
	        double dy = datumPoint.longitudeAndLatitudeToDatumPoint(current_drone_position).stc_y;
	        double dz = datumPoint.longitudeAndLatitudeToDatumPoint(current_drone_position).stc_z;

            LOG(ERROR) << "dx : " << dx;
            LOG(ERROR) << "dy : " << dy;
            LOG(ERROR) << "dz : " << dz;



            int missionType = stoi(task->getMissionType());
            LOG(ERROR) << "missionType : " << missionType;
	        switch (missionType)
            {
 
                case TASKTYPENOTUPLOADED:
                case TASKTYPEDISTRIBUTIONDOT:
         
                break;
          
                // case TASKTYPEROUTESGENERATE:
                //     m_data::Getm_data()->Get_Generate_fine_route_task_UploadToAircraft(m_vecDrone[index].submissionID, m_rtkhome_point);
                //     break;
                case TASKTYPEDISTRIBUTIONROUTE:
                    waypointVec = generateMinorAirline(submissionID, m_rtkhome_point, task->getFlightType());
                    break;

                case TASKTYPEIMPORTROUTE:
                     waypointVec = readFileType(submissionID);
                     waypointVec = addFristWayPoint(waypointVec, m_rtkhome_point, submissionID);
                     break;

                default:
                    break;
            }

            LOG(ERROR) << "waypointVec" << waypointVec.size();
            if (waypointVec.size() < 2)
            {
                LOG(ERROR) << "create line fail: waypointVec size < 2 " ;
                return;
            }

            vector<WaypointInformation> commute_task = waypointVec;
            for (auto item : waypointVec) {
                
                LOG(ERROR) << "task id " << item.task_id << " tower id " << item.tower_id << " FROM_PC_Hightprecision_latitude = " << item.stc_y <<" FROM_PC_Hightprecision_longitude = "
                    << item.stc_x << " FROM_PC_Hightprecision_relativeHeight = "<< item.stc_z ;
                LOG(ERROR) << " item.stc_yaw = " << item.stc_yaw;

                LOG(ERROR) << " item.zoom_Magnification : " << item.zoom_Magnification;
                for (auto target : item.target_class_id) {
                    LOG(ERROR) << " item.target : " << target;
                }
              
            }

            double d_longitude = 0;
	        double d_latitude = 0;
	        float d_height = 0;
            int doublephoto = GetDoublePhotoBySubmissionID(submissionID);
            for (int j = 0; j < commute_task.size(); j++) {
                /*if (basicType == 1)
                {
                    commute_task[j].stc_x -= d_longitude;
                    commute_task[j].stc_y -= d_latitude;
                }*/
                commute_task[j].stc_z -= d_height;
                if (doublephoto == 1)
                {
                    commute_task[j].Photo_time = "1";
                }
                else if (doublephoto == 0)
                {
                    commute_task[j].Photo_time = "0";
                }
	        }

            list<WaypointInformation> commute_task_list;
	        for (int i = 0; i < commute_task.size(); i++)
	        {
		        commute_task_list.push_back(commute_task[i]);
                LOG(ERROR) <<" commute_task[i].stc_gimbalPitch : " << commute_task[i].stc_gimbalPitch;
	        }
            LOG(ERROR) << "commute_task_list size : " << commute_task_list.size();
            vector<uint8_t> info;
            string realData;
            uint8_t cmd = 0x02;
            realData = cmd;
            info.push_back(cmd);

            TaskInfo taskInfo;
            taskInfo.taskID = stoi(task->getSubmissionID());
            LOG(ERROR) << " taskInfo.taskID : " << taskInfo.taskID;
            taskInfo.flight = task->getFlightType();
            taskInfo.rtk = task->getRtk();
            taskInfo.record = task->getWhetherRecord();
            taskInfo.timestamp = task->getTimestamp();
            taskInfo.imgwidth = task->getImgwidth();
            taskInfo.imgheight = task->getImgheigth();
            int extralen = sizeof(taskInfo.taskID) + sizeof(taskInfo.rtk) + sizeof(taskInfo.flight) + sizeof(taskInfo.timestamp) + sizeof(taskInfo.record) + sizeof(taskInfo.imgwidth) + sizeof(taskInfo.imgheight);
            taskInfo.returnCode = task->getReturnCode();
            LOG(ERROR) << "returnCode : " << taskInfo.returnCode;
            if(task->getReturnCode() == 254){
               
                taskInfo.Gps = task->getReturnGPS();
                extralen += sizeof(taskInfo.returnCode) + sizeof(taskInfo.Gps);
                LOG(ERROR) << "return code == 254 : " << extralen;
            } else {
                extralen += sizeof(taskInfo.returnCode);
                LOG(ERROR) << "return code != 254 : " << extralen;
            }

            std::stringstream ss;
            Serialization(ss, commute_task_list);
            std::string serializedData = ss.str();
            uint16_t data_len = serializedData.size() + extralen;
            realData.resize(realData.size() + sizeof(data_len) + extralen);

            uint16_t bigEndianValue =  ((data_len & 0xFF00) >> 8) | ((data_len & 0x00FF) << 8);
            int len = sizeof(cmd);
	        memcpy((char*)realData.data() + len, &bigEndianValue, sizeof(bigEndianValue));
            len += sizeof(bigEndianValue);
            memcpy((char*)realData.data() + len, (char*)&taskInfo.taskID, sizeof(taskInfo.taskID));
            len += sizeof(taskInfo.taskID);
	        memcpy((char*)realData.data() + len, (char*)&taskInfo.rtk, sizeof(taskInfo.rtk));
	        len += sizeof(taskInfo.rtk);
	        memcpy((char*)realData.data() + len, (char*)&taskInfo.flight, sizeof(taskInfo.flight));
	        len += sizeof(taskInfo.flight);
	        memcpy((char*)realData.data() + len, (char*)&taskInfo.timestamp, sizeof(taskInfo.timestamp));
	        len += sizeof(taskInfo.timestamp);
	        memcpy((char*)realData.data() + len, (char*)&taskInfo.record, sizeof(taskInfo.record));
	        len += sizeof(taskInfo.record);
	        memcpy((char*)realData.data() + len, (char*)&taskInfo.imgwidth, sizeof(taskInfo.imgwidth));
	        len += sizeof(taskInfo.imgwidth);
	        memcpy((char*)realData.data() + len, (char*)&taskInfo.imgheight, sizeof(taskInfo.imgheight));
            len += sizeof(taskInfo.imgheight);
            memcpy((char*)realData.data() + len, (char*)&taskInfo.returnCode, sizeof(taskInfo.returnCode));
            len += sizeof(taskInfo.returnCode);
            if(taskInfo.returnCode == 254)
            {
                memcpy((char*)realData.data() + len, (char*)&taskInfo.Gps, sizeof(taskInfo.Gps));
            }
            copy(serializedData.begin(), serializedData.end(), std::back_inserter(realData));
            SSIZE_T bytesSent =  send(session->socket(), realData.data(), realData.size(), 0);
            LOG(ERROR) << "send task info to drone "  << bytesSent;
            delete task;
            if (bytesSent == realData.size()) {
                LOG(ERROR) << "Data sent successfully. ";

            } else if (bytesSent == -1) {
                LOG(ERROR) << "Failed to send data. Error code. " << errno;
            } else {
                LOG(ERROR) << "Partial data sent. Sent bytes:. " << bytesSent;
            }
            ss.str("");
            ss.clear();
            for (const uint8_t byte : realData) {
               std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
            }

        } else {
            LOG(ERROR) << "data format error";
        }
    }
    catch(exception& e){
        LOG(ERROR) << "getTaskInfo error";
    }
}

void DataProcessor::processLog(const uint8_t* data, int length, Session* session)
{
    LOG(ERROR) << " === processLog ===";
    try {
        if (length > 3) {
            LOG(ERROR) << "length :" << length;
            std::stringstream sAnswer;
            uint8_t answer = 0;
            sAnswer << string((const char*)data, sizeof(answer));
            UnSerialization(sAnswer, answer);
            LOG(ERROR) << "answer : " << (int)answer;
            if (answer == 0) {
                std::stringstream sName;
                char name[64] = { 0 };
                sName << string((const char*)data + sizeof(answer), sizeof(name));
                UnSerialization(sName, name);
                LOG(ERROR) << "log file name : " << name;

                std::stringstream slen;
                uint32_t len = 0;
                slen << string((const char*)data + sizeof(answer) + sizeof(name), sizeof(len));
                UnSerialization(slen, len);
                LOG(ERROR) << "log file len : " << len;

                /*const char* fileContent = reinterpret_cast<const char*>(data + sizeof(answer) + sizeof(name) + sizeof(len));
                LOG(ERROR) << "fileContent : " << sizeof(fileContent);*/

                //string filePath = "C:/Users/Administrator/Desktop/console_socket/console_socket/x64/Debug/droneLog/" + (string)name;
                ////string filePath = "C:/Users/123/Desktop/origin_php/myphp/public/droneLog/" + (string)name;
                //LOG(ERROR) << "filePath : " << filePath;
                //if (std::filesystem::exists(filePath)) {
                //    deleteFile(filePath);
                //}

                //recvLogFile(filePath, name, fileContent, len);
                   
                

            }else if (answer == 1) {
                LOG(ERROR) << "fail to get log ";
            }
            else if (answer == 2) {
                LOG(ERROR) << "log file unexist ";
            }
            else if (answer == 3) {
                LOG(ERROR) << "log too huge";
            }
        }
    }
    catch (exception& e) {
        LOG(ERROR) << "get log error";
    }
}

bool  DataProcessor::recvLogFile(string filePath, string fileName, const char *data, uint32_t len) {
    std::ofstream outputFile(filePath, std::ios::binary);
    if (outputFile.is_open()) {
        outputFile.write(data, len);
        outputFile.close();
        bool dbResult = insertLogToDB(fileName);
        if (dbResult) {
            LOG(ERROR) << "Binary file saved successfully.";
            return true;
        }
        else {
            return false;
        }
    }
    else {
        LOG(ERROR) << "Error saving binary file.";
        return false;
    }
}

bool DataProcessor::deleteFile(const std::string& filePath) {
    try {
        std::filesystem::remove(filePath);
        return true;
    }
    catch (const std::exception& e) {
        LOG(ERROR) << "Error deleting file: " << e.what();
        return false;
    }
}

bool DataProcessor::insertLogToDB(string filePath) {
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
    string result;
    string selectQuery = "SELECT COUNT(*) FROM droneLog WHERE logName = '" + filePath + "';";
    if (SQLiteConnector->executeScalar(selectQuery, result)) {
        int count = stoi(result);
        if (count == 0) {
            string insertQuery = "INSERT INTO droneLog (logName) VALUES (?);";
            vector<string> placeholders = {
                        filePath
            };
            if (SQLiteConnector->executeDroneInsertQuery(insertQuery, placeholders)) {
                LOG(ERROR) << "Insertion successful";
                return true;
            }
            else {
                LOG(ERROR) << "Insertion failed";
                return false;
            }
        }
        else {
            LOG(ERROR) << "log already exists";
            return false;
        }
    }
}

list<int> DataProcessor::getTowerList(int submissionID)
{
    std::vector<int> vectowerID = querySubmissionTower(submissionID);
    std::list<int> toweridlist;
    for (int j = 0; j < vectowerID.size(); j++)
    {
        toweridlist.push_back(vectowerID[j]);
    }
    return toweridlist;
}

vector<int> DataProcessor::querySubmissionTower(int submissionID)
{
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
    vector<int> vecSubmissionTower;
    string query = "SELECT towerID FROM submissiontowerList WHERE submissionID = ?";
    vector<string> placeholders;
    placeholders.push_back(to_string(submissionID));
    vector<vector<string>> queryResults;
    if (SQLiteConnector->executeQueryWithPlaceholder(query, placeholders, queryResults)) {
        for (const auto& row : queryResults) {
            if (!row.empty()) {
                int towerID = stoi(row[0]);
                vecSubmissionTower.push_back(towerID);
            }
        }
    }

    return vecSubmissionTower;
}

void DataProcessor::processSNCode(const uint8_t* data, int length, SQLiteConnection* SQLiteConnector, Drones* drone, Session* session){
    
    DroneData droneData = getDroneSNCode(data,length);
    string snCode = droneData.snCode;
    LOG(ERROR) << "snCode : " << snCode;
    string version = droneData.version;
    LOG(ERROR) << "version : " << version;
    float maxSpeed = droneData.maxSpeed;
    LOG(ERROR) << "maxSpeed : " << maxSpeed;
    string model = droneData.model;
    LOG(ERROR) << "model : " << model;
    string lens = droneData.lens;
    LOG(ERROR) << "lens : " << lens;
    /*string wifiName = droneData.wifiName;
    LOG(ERROR) << "wifiName : " << wifiName;
    string wifiPassword = droneData.wifiPassword;
    LOG(ERROR) << "wifiPassword : " << wifiPassword;*/

    std::time_t timestamp = std::time(nullptr);
    LOG(ERROR) << "Timestamp: " << timestamp ;
    unsigned long ts = timestamp;
    string lastTime = to_string(ts);
    LOG(ERROR) << "lastTime: " << lastTime;


    drone->setSnCode(snCode);
    drone->setDroneType(model);
    drone->setMaxSpeed(to_string(maxSpeed));
    drone->setSystemVersion(version);
    drone->setLen(lens);

    string whetherOnline = "1";
    SessionManager *m_sessionManager = SessionManager::GetInstance(); 
    m_sessionManager->insertToDroneSessionMap(snCode, session);
    session->setSncode(snCode);
    string result;
    string selectQuery = "SELECT COUNT(*) FROM drone WHERE snCode = '" + drone->getSnCode() + "';";
    if (SQLiteConnector->executeScalar(selectQuery, result)) {
        int count = stoi(result);
        if (count == 0) {
            string insertQuery = "INSERT INTO drone (snCode, systemVersion, maxSpeed, droneType, lensType, whetherOnline) VALUES (?, ?, ?, ?, ? ,?);";
            vector<string> placeholders = {
                drone->getSnCode(),
                version,
                to_string(maxSpeed),
                model,
                lens,
                whetherOnline
            };
            if (SQLiteConnector->executeDroneInsertQuery(insertQuery, placeholders)) {
                LOG(ERROR) << "Insertion drone successful and update wifiRecord";
           /*     string wifiRecordINSERTSQL = "INSERT INTO wifiRecord (snCode, wifiName, wifiPassword,lastTime) VALUES (?, ?, ?, ?);";
                vector<string> recordPlaceholders = {
                    drone->getSnCode(),
                    wifiName,
                    wifiPassword,
                    lastTime
                };
                if (SQLiteConnector->executeDroneInsertQuery(wifiRecordINSERTSQL, recordPlaceholders)) {
                    LOG(ERROR) << "Insertion successful,update WiFiRecord";
                } else {
                    LOG(ERROR) << "Insert to wifiRecord failed";
                    return;
                }*/
            } else {
                LOG(ERROR) << "Insertion failed";
            }
        } else {
            LOG(ERROR) << "Duplicate snCode found, insertion skipped";
            vector<std::string> snCodeVec;
            std::string updateQuery = "UPDATE drone SET systemVersion = ? ,maxSpeed = ?, droneType= ?, lensType = ? , whetherOnline = ? WHERE snCode = ?";
            snCodeVec.push_back(version);
            snCodeVec.push_back(to_string(maxSpeed));
            snCodeVec.push_back(model);
            snCodeVec.push_back(lens);
            snCodeVec.push_back(whetherOnline);
            snCodeVec.push_back(drone->getSnCode());
            if (SQLiteConnector->executeDroneInsertQuery(updateQuery, snCodeVec)) {
                LOG(ERROR) << "update executed drone successfully and update wifiRecord";
                //string wifiRecordUpdateSQL = "UPDATE wifiRecord set wifiName = ?, wifiPassword = ?, lastTime = ? WHERE snCode = ?";
                //vector<string> recordUpdate = {
                //    wifiName,
                //    wifiPassword,
                //    lastTime,
                //    drone->getSnCode()
                //};
                //if (SQLiteConnector->executeDroneInsertQuery(wifiRecordUpdateSQL, recordUpdate)) {
                //    LOG(ERROR) << "update wifiRecord successfully";
                //}
                //else {
                //    LOG(ERROR) << "Insert to wifiRecord failed";
                //    return;
                //}
            }
            else {
                LOG(ERROR) << "Error executing drone update query.";
            }
        }
    } else {
        LOG(ERROR) << "Database query COUNT(*) failed";
    }
}   

DroneData DataProcessor::getDroneSNCode(const uint8_t* data, int length){
    LOG(ERROR) << "get snCode";
    DroneData droneData;
    try{
        if(length > 3){
            std::stringstream ss;
            ss.write((const char*)data, length);
            UnSerialization(ss,droneData);
            string snCode = droneData.snCode;
            string version = droneData.version;
            float maxSpeed = droneData.maxSpeed;
            string model = droneData.model;
            string lens = droneData.lens;

            ss.str("");
            ss.clear();

        } else {
            LOG(ERROR) << "DroneSNCode data format error";
            //throw runtime_error("Data format error");
        }
    }
    catch(exception& e){
       LOG(ERROR) << "UnSerialization snCode error";
       //throw;
    }
     return droneData;
}

HeartbeatInfo DataProcessor::getHeartbeatInfo(const uint8_t* data, int length, Drones* drone, Session* session){
    //TRACE(" ==== getHeartbeatInfo ===");
    LOG(ERROR) << "==== getHeartbeatInfo ===";
    HeartbeatInfo heartbeatInfo;
    //TRACE("drone->getSnCode() : " + drone->getSnCode());
    //TRACE("session in getHeartbeatInfo:" + to_string(session->socket()));
    try{
        if(length > 3){
            std::stringstream ss;
            ss.write((const char*)data, length);
            UnSerialization(ss,heartbeatInfo);
            //TRACE("FINISHED");
            double longitude = heartbeatInfo.longitude;
            double latitude = heartbeatInfo.latitude;
            double altitude = heartbeatInfo.altitude;
            int missionID = heartbeatInfo.missionID;
            LOG(ERROR) << "missionID : " << missionID;
            uint8_t status = heartbeatInfo.status;
            LOG(ERROR) << "status : " << (int)status;
            uint32_t index = heartbeatInfo.index;
            LOG(ERROR) << "index : " << index;
            uint32_t count = heartbeatInfo.count;
            LOG(ERROR) << "count : " << count;
            heartbeatInfo.isInitialized = true; 
            SessionManager *m_sessionManager = SessionManager::GetInstance(); 
            m_sessionManager->insertToHeartbeatInfoSessionMap(session, heartbeatInfo);
            vector<uint8_t> info;
            info.push_back(0x00);
            info.push_back(0x00);
            info.push_back(0x00);
    
            SSIZE_T bytesSent = send(session->socket(), reinterpret_cast<const char*>(info.data()), info.size(), 0);
            if (bytesSent == info.size()) {
                LOG(ERROR) << "heartbeat sent successfully";
            } else if (bytesSent == -1) {
                LOG(ERROR) << "Failed to send data. Error code :" << errno;
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    LOG(ERROR) << "errno is EAGAIN and EWOULDBLOCK";
                }
                else {
                    closesocket(session->socket());
                }
            } else {
                LOG(ERROR) << "Partial data sent. Sent bytes";
               
            }
            ss.str("");
            ss.clear();
        } else {
            LOG(ERROR) << "HeartbeatInfo format error" ;
        }
    }
    catch(exception& e){
        LOG(ERROR) << "UnSerialization HeartbeatInfo error";
    }
    return heartbeatInfo;
}


// bool DataProcessor::updateTaskStatus(HeartbeatInfo heartbeatInfo){
//     SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
//     if(heartbeatInfo.missionID == -1){
//         string query = "SELECT towerID, lineID, height, longitude, latitude, altitude, insulatorNum, towerShapeID, towerName, basicID FROM submission_tower_missiontype WHERE submissionID = ?";
//         vector<string> placeholders;
//         placeholders.push_back(to_string(submission_id));
//     } else {
//         string query = "SELECT towerID, lineID, height, longitude, latitude, altitude, insulatorNum, towerShapeID, towerName, basicID FROM submission_tower_missiontype WHERE submissionID = ?";
//     }
// }


vector<WaypointInformation> DataProcessor::GetSubmissionWaypoints(int submissionID)
{
    LOG(ERROR) << "=== GetSubmissionWaypoints === ";
	std::vector<BasicResultGet> tower_data;
	std::vector<AirlineTower> m_towerInfoForInitialRoute = Query_submission_towerinfo(submissionID);
	if (m_towerInfoForInitialRoute.size() < 3)
	{
		return std::vector<WaypointInformation>();
	}
    for (int i = 0; i < m_towerInfoForInitialRoute.size(); i++) {
        LOG(ERROR) << " m_towerInfoForInitialRoute id  ： " << m_towerInfoForInitialRoute[i].stc_towerId;
    }
	int missionType = Query_submisson_missionID(submissionID);
	std::vector<int> flagVec;
	for (int i = 0; i < m_towerInfoForInitialRoute.size(); i++)
	{
		if (m_towerInfoForInitialRoute[i].stc_towerId == -1)
		{
			BasicResultGet temp;
			Gps temp_gps;
			temp_gps.stc_alt = 0;
			temp_gps.stc_lat = 0;
			temp_gps.stc_lon = 0;
			temp.stc_basicResult.stc_vecGps.push_back(temp_gps);
			temp.stc_basicResult.stc_vecGps.push_back(temp_gps);
			temp.stc_basicResult.stc_vecGps.push_back(temp_gps);
			tower_data.push_back(temp);
			flagVec.push_back(0);
			continue;
		}

		if (i % 3 == 0 && (missionType == TASKTYPEROUTESGENERATE || missionType == TASKTYPEROUTESPATROL) && m_towerInfoForInitialRoute[i].stc_path.empty()) {

            ////TRACE("tower point error");
		    return std::vector<WaypointInformation>();
		}

		BasicResultGet p_tower_data;
		if (i % 3 == 0 && (missionType == TASKTYPEROUTESGENERATE || missionType == TASKTYPEROUTESPATROL)) {
           // //TRACE("read file ");
			// string data = "";
			// m_cruiseInterface.Cruise_Read_File(m_towerInfoForInitialRoute[i].stc_path, data);
			// p_tower_data = m_cruiseJson.GetBasicResult(data.toUtf8());
		}
		else {
           // LOG(ERROR) <<  " mission type ";
			p_tower_data.stc_isSuccess = true;
			Gps tempGps;
			tempGps.stc_lon = m_towerInfoForInitialRoute[i].stc_x;
			tempGps.stc_lat = m_towerInfoForInitialRoute[i].stc_y;
			tempGps.stc_alt = m_towerInfoForInitialRoute[i].stc_z;
			p_tower_data.stc_basicResult.stc_vecGps.push_back(tempGps);
			p_tower_data.stc_basicResult.stc_vecGps.push_back(tempGps);
			p_tower_data.stc_basicResult.stc_vecGps.push_back(tempGps);
		}
		p_tower_data.stc_errCode = m_towerInfoForInitialRoute[i].height;
        LOG(ERROR) << "p_tower_data.stc_errCode : " << p_tower_data.stc_errCode;
		if (m_towerInfoForInitialRoute[i].towerVirtual == 2) {
			p_tower_data.stc_errMsg = "2";
		}
		else if (m_towerInfoForInitialRoute[i].towerVirtual == 1) {
			p_tower_data.stc_errMsg = "1";
		}
		else {
			p_tower_data.stc_errMsg = "0";
		}
		tower_data.push_back(p_tower_data);
		int photoflag = get_photoPosition(m_towerInfoForInitialRoute[i].stc_towerId);
		if (m_towerInfoForInitialRoute[i].pass == 0) {
			photoflag |= 0b10000;
		}
		flagVec.push_back(photoflag);
	}
    //TRACE("tower_data size : " + to_string(tower_data.size()));
    LOG(ERROR) <<  " tower_data size : " << tower_data.size();
	int increasehigh = 0;
	vector<WaypointInformation> vecWaypoint;
	for (int i = 0; i < tower_data.size(); ) {
        ////TRACE(" i :" + to_string(i) + " , towerVirtual : " + to_string(m_towerInfoForInitialRoute[i].towerVirtual));
		if (m_towerInfoForInitialRoute[i].towerVirtual == 0) { 
			if (i + 2 >= tower_data.size()) {
				return std::vector<WaypointInformation>();
		    }

			RecPoint temprecPoint;
			DatumPoint tmpdp;
			tmpdp.stc_x = tower_data[i].stc_basicResult.stc_vecGps[0].stc_lon;
			tmpdp.stc_y = tower_data[i].stc_basicResult.stc_vecGps[0].stc_lat;
			tmpdp.stc_z = tower_data[i].stc_basicResult.stc_vecGps[0].stc_alt;
			temprecPoint.rec_pt[0] = tmpdp;
			if (missionType == TASKTYPEROUTESGENERATE || missionType == TASKTYPEROUTESPATROL) {
				tmpdp.stc_x = tower_data[i].stc_basicResult.stc_vecGps[1].stc_lon;
				tmpdp.stc_y = tower_data[i].stc_basicResult.stc_vecGps[1].stc_lat;
				tmpdp.stc_z = tower_data[i].stc_basicResult.stc_vecGps[1].stc_alt;
				temprecPoint.rec_pt[1] = tmpdp;
				tmpdp.stc_x = tower_data[i].stc_basicResult.stc_vecGps[2].stc_lon;
				tmpdp.stc_y = tower_data[i].stc_basicResult.stc_vecGps[2].stc_lat;
				tmpdp.stc_z = tower_data[i].stc_basicResult.stc_vecGps[2].stc_alt;
				temprecPoint.rec_pt[2] = tmpdp;
			}
			else {
				temprecPoint.rec_pt[1] = tmpdp;
				temprecPoint.rec_pt[2] = tmpdp;
			}

			DatumPoint prev_temp;
			if (m_towerInfoForInitialRoute[i + 1].stc_towerId != -1) {
				prev_temp.stc_x = tower_data[i + 1].stc_basicResult.stc_vecGps[0].stc_lon;
				prev_temp.stc_y = tower_data[i + 1].stc_basicResult.stc_vecGps[0].stc_lat;
				prev_temp.stc_z = tower_data[i + 1].stc_basicResult.stc_vecGps[0].stc_alt;
			}
			else
			{
				prev_temp.stc_x = 0.0;
				prev_temp.stc_y = 0.0;
				prev_temp.stc_z = 0.0;
			}
			temprecPoint.prev_pt = prev_temp;

			DatumPoint next_temp;
			if (m_towerInfoForInitialRoute[i + 2].stc_towerId != -1) {
				next_temp.stc_x = tower_data[i + 2].stc_basicResult.stc_vecGps[0].stc_lon;
				next_temp.stc_y = tower_data[i + 2].stc_basicResult.stc_vecGps[0].stc_lat;
				next_temp.stc_z = tower_data[i + 2].stc_basicResult.stc_vecGps[0].stc_alt;
			}
			else
			{
				next_temp.stc_x = 0.0;
				next_temp.stc_y = 0.0;
				next_temp.stc_z = 0.0;
			}
			temprecPoint.next_pt = next_temp;

			temprecPoint.layer_num = m_towerInfoForInitialRoute[i].stc_insulator_layer;
			temprecPoint.left_angle = m_towerInfoForInitialRoute[i].stc_leftAngle;
            LOG(ERROR) << "temprecPoint.left_angle : " << temprecPoint.left_angle;
			temprecPoint.right_angle = m_towerInfoForInitialRoute[i].stc_rightAngle;
            LOG(ERROR) << "temprecPoint.right_angle : " << temprecPoint.right_angle;
			temprecPoint.yaw_rotate_angle_selected = m_towerInfoForInitialRoute[i].stc_offCenterAngle;
			temprecPoint.height = m_towerInfoForInitialRoute[i].stc_height;
			temprecPoint.ChannelInspection = m_towerInfoForInitialRoute[i].stc_ChannelInspection;
			temprecPoint.zoom = m_towerInfoForInitialRoute[i].zoom;
			int m_lineTypeID = Query_lintTypeID(m_towerInfoForInitialRoute[i].stc_towerId);
			//std::vector<RouteIndexInfo> temp_vec_routeIndexInfo;
			std::vector<WaypointInformation> temp_vec_routeIndexInfo_WaypointInformation;
            temp_vec_routeIndexInfo_WaypointInformation.size();
			//= m_pointToLine.pointToLine_u1(temprecPoint);
            // //TRACE("missionType :  " + to_string(missionType));
			if (missionType == TASKTYPEDISTRIBUTIONROUTE) {
                ////TRACE("missionType :  " + to_string(missionType));
				temp_vec_routeIndexInfo_WaypointInformation = m_point_to_line_v3.pointToLine_minor(temprecPoint, flagVec[i]);
			}
			else if (m_lineTypeID == 0)
			{
               // //TRACE(" m_lineTypeID == 0  ");
				temp_vec_routeIndexInfo_WaypointInformation = m_point_to_line_v3.pointToLine_U(temprecPoint);
			}
			else if (m_lineTypeID == 1)
			{
                ////TRACE(" m_lineTypeID == 1 ");
				temp_vec_routeIndexInfo_WaypointInformation = m_point_to_line_v3.pointToLine_OU(temprecPoint);
			}
            ////TRACE("temp_vec_routeIndexInfo_WaypointInformation :  " + to_string(temp_vec_routeIndexInfo_WaypointInformation.size()));
			if (temp_vec_routeIndexInfo_WaypointInformation.size() == 1 && temp_vec_routeIndexInfo_WaypointInformation[0].stc_index == 0xFFFF)//stc_index为-1，因为是无符号数，改下判断条件
			{
                //TRACE("1 benchmark data does not meet standards");
				return std::vector<WaypointInformation>();
			}
			if (temp_vec_routeIndexInfo_WaypointInformation.size() == 0)
			{
                ////TRACE("2 benchmark data does not meet standards");
				//m_cruiseInterface.Cruise_Write_File(QDir::currentPath() + "/temp_initial_route.pcd", write_error_pcl_file());
			
				return std::vector<WaypointInformation>();
			}
            ////TRACE("temp_vec_routeIndexInfo_WaypointInformation size :" + to_string(temp_vec_routeIndexInfo_WaypointInformation.size()));
			for (int a = 0; a < temp_vec_routeIndexInfo_WaypointInformation.size(); a++)
			{
				temp_vec_routeIndexInfo_WaypointInformation[a].tower_id = m_towerInfoForInitialRoute[i].stc_towerId;
				temp_vec_routeIndexInfo_WaypointInformation[a].Datum_point = tower_data[i].stc_basicResult.stc_vecGps;
				temp_vec_routeIndexInfo_WaypointInformation[a].task_id = submissionID;
                temp_vec_routeIndexInfo_WaypointInformation[a].Reserve1 = m_towerInfoForInitialRoute[i].channelPhotography;
                temp_vec_routeIndexInfo_WaypointInformation[a].Reserve2 = m_towerInfoForInitialRoute[i].localClearPhoto;
			}
			if (increasehigh != 0) {
				WaypointInformation wptmp = temp_vec_routeIndexInfo_WaypointInformation[0];
				wptmp.stc_shootPhoto = 0;
				wptmp.stc_z += increasehigh;
				wptmp.tower_id = -1;
				wptmp.task_id = submissionID;
				wptmp.stc_gimbalPitch = -90;
				wptmp.necessary = true;
				temp_vec_routeIndexInfo_WaypointInformation.insert(temp_vec_routeIndexInfo_WaypointInformation.begin(), wptmp);
				increasehigh = 0;
			}
            ////TRACE("increasehigh != 0 temp_vec_routeIndexInfo_WaypointInformation size :" + to_string(temp_vec_routeIndexInfo_WaypointInformation.size()));
			//LOG(ERROR )
            if (tower_data[i].stc_errCode != 0)
			{
				int inx = temp_vec_routeIndexInfo_WaypointInformation.size() - 1;
				WaypointInformation wptmp = temp_vec_routeIndexInfo_WaypointInformation[inx];
				wptmp.stc_shootPhoto = 0;
				wptmp.stc_z += tower_data[i].stc_errCode;
				for (int a = 0; a < temp_vec_routeIndexInfo_WaypointInformation.size(); a++) {
					if (temp_vec_routeIndexInfo_WaypointInformation[a].necessary == true) {
						wptmp.stc_yaw = temp_vec_routeIndexInfo_WaypointInformation[a].stc_yaw;
						break;
					}
				}
				wptmp.stc_gimbalPitch = -90;
				wptmp.tower_id = -1;
				wptmp.task_id = submissionID;
				wptmp.necessary = true;
				temp_vec_routeIndexInfo_WaypointInformation.push_back(wptmp);
				increasehigh = tower_data[i].stc_errCode;
			}
            ////TRACE("tower_data[i].stc_errCode != 0 temp_vec_routeIndexInfo_WaypointInformation size :" + to_string(temp_vec_routeIndexInfo_WaypointInformation.size()));
			vecWaypoint.insert(vecWaypoint.end(), temp_vec_routeIndexInfo_WaypointInformation.begin(), temp_vec_routeIndexInfo_WaypointInformation.end());
            ////TRACE("in process vecWaypoint.size  : " + to_string(vecWaypoint.size()));
			i += 3;
		}
		else if (m_towerInfoForInitialRoute[i].towerVirtual == 1) {//虚拟塔
			WaypointInformation virtualwaypoint;
			virtualwaypoint.stc_x = m_towerInfoForInitialRoute[i].stc_x;
			virtualwaypoint.stc_y = m_towerInfoForInitialRoute[i].stc_y;
			virtualwaypoint.stc_z = m_towerInfoForInitialRoute[i].stc_z;
			virtualwaypoint.stc_shootPhoto = 0;
			virtualwaypoint.tower_id = m_towerInfoForInitialRoute[i].stc_towerId;
			virtualwaypoint.task_id = submissionID;
			virtualwaypoint.target_class_id.push_back(12);
			virtualwaypoint.necessary = true;
            virtualwaypoint.Reserve1 = m_towerInfoForInitialRoute[i].channelPhotography;
            virtualwaypoint.Reserve2 = m_towerInfoForInitialRoute[i].localClearPhoto;
			if (increasehigh != 0) {
				WaypointInformation wptmp = virtualwaypoint;
				wptmp.tower_id = -1;
				wptmp.stc_z += increasehigh;
				wptmp.stc_gimbalPitch = -90;
				vecWaypoint.push_back(wptmp);
				increasehigh = 0;
			}
			vecWaypoint.push_back(virtualwaypoint);
			if (tower_data[i].stc_errCode != 0)
			{
				WaypointInformation wptmp = virtualwaypoint;
				wptmp.tower_id = -1;
				wptmp.stc_z += tower_data[i].stc_errCode;
				wptmp.stc_gimbalPitch = -90;
				vecWaypoint.push_back(wptmp);
				increasehigh = tower_data[i].stc_errCode;
			}
			i++;
		}
		else
		{
			i++;
		}
	}
    ////TRACE("vecWaypoint.size  : " + to_string(vecWaypoint.size()));
    //vector<WaypointInformation> vecWaypoint;
	return vecWaypoint;
}

vector<AirlineTower> DataProcessor::Query_submission_towerinfo(int submission_id)
{
    LOG(ERROR) << "Query_submission_towerinfo";
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
    std::vector<AirlineTower> vecTowerInfoForInitialRoute;
    //db submission_tower_missiontype without mode字段
    std::string query = "SELECT towerID, lineID, height, longitude, latitude, altitude, insulatorNum, towerShapeID, towerName, basicID, mode, localClearPhoto, channelPhotography FROM submission_tower_missiontype WHERE submissionID = ?";
    std::vector<std::string> placeholders;
    placeholders.push_back(std::to_string(submission_id));
    std::vector<std::vector<std::string>> queryResults;
    if (SQLiteConnector->executeQueryWithPlaceholder(query, placeholders, queryResults)) {
        for (const auto& row : queryResults) {
            AirlineTower temp; 
            //TRACE("ROW SIZE : " + to_string(row.size()));    
            //towerID 0, lineID 1, height 2,
            //longitude 3 , latitude 4, altitude 5, 
            //insulatorNum 6,  towerShapeID 7, towerName 8, basicID 9
            temp.stc_towerId = std::stoi(row[0]);
            //TRACE("temp.stc_towerId : " + to_string(temp.stc_towerId));
            temp.lineID = std::stoi(row[1]);
            //TRACE("temp.lineID : " +  to_string(temp.lineID));
            // temp.mode = std::stoi(row[2]);
            // //TRACE("Query_submission_towerinfo");
            // temp.height = std::stoi(row[2]);
            //TRACE("height : " + row[2]);
            temp.height = 0;
            //TRACE("temp.height : " + to_string(temp.height));  
            temp.stc_x = std::stod(row[3]);
            //TRACE("temp.stc_x : " + to_string(temp.stc_x));
            temp.stc_y = std::stod(row[4]);
            //TRACE("temp.stc_x : " + to_string(temp.stc_y));
            temp.stc_z = std::stod(row[5]);
            //TRACE("temp.stc_z  : " + to_string(temp.stc_z));
            temp.zoom = std::stoi(row[6]);
            //TRACE("temp.zoom  : " + to_string(temp.zoom));
            temp.towerShapeID = std::stoi(row[7]);
            //TRACE("temp.towerShapeID  : " + to_string( temp.towerShapeID));

            string towerName = row[8];
            //TRACE("towerName  : " + row[8]);
            if (towerName.front() == '@' || temp.towerShapeID == 36) {

			    temp.towerVirtual = 1;
		    }
            //TRACE("temp.towerVirtual  : " + to_string(temp.towerVirtual));
            int basicid = stoi(row[9]);
            //TRACE("basicid  : " + row[9]);
            if (basicid > 0) {
                std::string basicQuery = "SELECT savePath FROM basicList WHERE basicID = ?";
                std::vector<std::vector<std::string>> basicQueryResults;
                std::vector<std::string> basicPlaceholders = { std::to_string(basicid) };
                if (SQLiteConnector->executeQueryWithPlaceholder(basicQuery, basicPlaceholders, basicQueryResults)) {
                    if (!basicQueryResults.empty() && !basicQueryResults[0].empty()) {
                        temp.stc_path = string (basicQueryResults[0][0]);
                        //TRACE("temp.stc_path  : " + temp.stc_path);
                    }
                }
            }
            //TRACE("temp.stc_path  : " + temp.stc_path);
            if (row[10].empty())
            {
                temp.mode = 0;
            }
            else {
                temp.mode = stoi(row[10]);
            }
            if (row[2].empty())
            {
                temp.height = 0;
            }else {
                temp.height = stoi(row[2]);
            }


            if (row[11].empty())
            {
                temp.localClearPhoto = 0;
            }
            else {
                temp.localClearPhoto = stoi(row[11]);
            }

            if (row[12].empty())
            {
                temp.channelPhotography = 0;
            }
            else {
                temp.channelPhotography = stoi(row[12]);
            }


            LOG(ERROR) << "temp.mode " << temp.mode << " temp.height :" << temp.height;

            vecTowerInfoForInitialRoute.push_back(temp);
		}

    } else {
        LOG(ERROR) << "Query submission_tower_missiontype failed " ;
    }
	map<int, std::vector<TowerList>> towermap;
	AirlineTower prev;
	AirlineTower next;
	prev.stc_towerId = -1;
	next.stc_towerId = -1;
	std::vector<AirlineTower> returntower;
	std::vector<AirlineTower> alltower;
    LOG(ERROR) << "vecTowerInfoForInitialRoute : " << vecTowerInfoForInitialRoute.size();
	//计算出所有杆塔（按线路飞需要计算）
	for (int i = 0; i < vecTowerInfoForInitialRoute.size();i++) {
		std::vector<TowerList> towers;
		auto iter = towermap.find(vecTowerInfoForInitialRoute[i].lineID);
		if (iter == towermap.end()) {
			towermap[vecTowerInfoForInitialRoute[i].lineID] = QueryTowersByLineID(vecTowerInfoForInitialRoute[i].lineID, true);
		}
		towers = towermap[vecTowerInfoForInitialRoute[i].lineID];
		//低2位为 00 直飞 01 提升高度 10 按线路飞 ；倒数第三位 0  顺序 1 倒序  ；倒数第四位 相邻 0 不相邻 1；倒数第五位 0 同线路 1 不同线路

        LOG(ERROR) << "vecTowerInfoForInitialRoute[i].mode : "  << vecTowerInfoForInitialRoute[i].mode;
        LOG(ERROR) << "(vecTowerInfoForInitialRoute[i].mode & 0b11) : " << (vecTowerInfoForInitialRoute[i].mode & 0b11);

		if (((vecTowerInfoForInitialRoute[i].mode & 0b11) == 0b0) || ((vecTowerInfoForInitialRoute[i].mode & 0b11) == 0b1)|| (vecTowerInfoForInitialRoute[i].mode == 3)) {//直飞或提升高度;
            LOG(ERROR) << "not sequence flight";
			auto ts = GetTowerShapeInformationByID(vecTowerInfoForInitialRoute[i].towerShapeID);
			vecTowerInfoForInitialRoute[i].stc_insulator_layer = ts.insulator_layer;
            //LOG(ERROR) << "1111";
			vecTowerInfoForInitialRoute[i].stc_leftAngle = ts.towerTypeLeftAngle;
            LOG(ERROR) << "vecTowerInfoForInitialRoute[i].stc_leftAngle : " << vecTowerInfoForInitialRoute[i].stc_leftAngle;
           // LOG(ERROR) << "2";
			vecTowerInfoForInitialRoute[i].stc_rightAngle = ts.towerTypeRightAngle;
            LOG(ERROR) << "vecTowerInfoForInitialRoute[i].stc_rightAngle : " << vecTowerInfoForInitialRoute[i].stc_rightAngle;
           //LOG(ERROR) << "3";
			vecTowerInfoForInitialRoute[i].stc_height = ts.tower_height; 
           // LOG(ERROR) << "4";
			vecTowerInfoForInitialRoute[i].stc_ChannelInspection = ts.ChannelInspection;
            //LOG(ERROR) << "5";
			vecTowerInfoForInitialRoute[i].stc_offCenterAngle = ts.towerTypeOffCenterAngle;
            //LOG(ERROR) << "6";
			alltower.push_back(vecTowerInfoForInitialRoute[i]);
		}
		else if ((vecTowerInfoForInitialRoute[i].mode & 0b11) == 0b10) {
    
            LOG(ERROR) << "begin sequence flight";
			if (i == vecTowerInfoForInitialRoute.size() - 1) {
                LOG(ERROR) << "vecTowerInfoForInitialRoute.size() error ";
				break;
			}
			if (vecTowerInfoForInitialRoute[i].mode & 0b100) {
                LOG(ERROR) << "vecTowerInfoForInitialRoute[i].mode & 0b100 ";
				std::vector<AirlineTower> tmptowers;
				bool bfind = false;
				bool bendfind = false;
				int findmode = 0;
				for (int j = towers.size() - 1; j > 0; j--) {
					if (towers[j].towerID == vecTowerInfoForInitialRoute[i].stc_towerId) {
						AirlineTower tmptw;
						tmptw.stc_towerId = towers[j].towerID;
						tmptw.lineID = towers[j].lineID;
						tmptw.stc_x = towers[j].longitude;
						tmptw.stc_y = towers[j].latitude;
						tmptw.stc_z = towers[j].altitude;
						tmptw.zoom = towers[j].insulatorNum;
						auto ts = GetTowerShapeInformationByID(vecTowerInfoForInitialRoute[i].towerShapeID);
						tmptw.stc_insulator_layer = ts.insulator_layer;
						tmptw.stc_leftAngle = ts.towerTypeLeftAngle;
						tmptw.stc_rightAngle = ts.towerTypeRightAngle;
						tmptw.stc_height = ts.tower_height;
						tmptw.stc_ChannelInspection = ts.ChannelInspection;
						tmptw.stc_offCenterAngle = ts.towerTypeOffCenterAngle;
						tmptw.mode = vecTowerInfoForInitialRoute[i].mode;
						findmode = vecTowerInfoForInitialRoute[i].mode;
						if (towers[j].basicID > 0) {
                            string select_sql = "SELECT savePath FROM basicList WHERE basicID = ?";
                            vector<vector<string>> queryResults;
                            vector<string> placeholders = {to_string(towers[j].basicID) };
                            if (SQLiteConnector->executeQueryWithPlaceholder(select_sql, placeholders, queryResults)) {
                                if (!queryResults.empty()) {
                                    const auto& row = queryResults[0];
                                    tmptw.stc_path = row[0];
                                }
                            }
						}
						tmptowers.push_back(tmptw);
						bfind = true;
						continue;
					}
					else if (bfind) {
						if (towers[j].towerID == vecTowerInfoForInitialRoute[i + 1].stc_towerId) {
							bendfind = true;
						}
						else {
							AirlineTower tmptw;
							tmptw.stc_towerId = towers[j].towerID;
							tmptw.lineID = towers[j].lineID;
							tmptw.stc_x = towers[j].longitude;
							tmptw.stc_y = towers[j].latitude;
							auto ts = GetTowerShapeInformationByID(towers[j].towerShapeID);
							tmptw.stc_z = towers[j].altitude + ts.tower_height;
							tmptw.zoom = towers[j].insulatorNum;
							tmptw.pass = 1;
							tmptw.mode = findmode;
                            string select_sql = "SELECT savePath FROM basicList WHERE basicID = ?";
                            vector<vector<string>> queryResults;
                            vector<string> placeholders = {to_string(towers[j].basicID) };
                            if (SQLiteConnector->executeQueryWithPlaceholder(select_sql, placeholders, queryResults)) {
                                if (!queryResults.empty()) {
                                    const auto& row = queryResults[0];
                                    tmptw.stc_path = row[0];
                                }
                            }
							tmptowers.push_back(tmptw);
						}
						if (bendfind) {
							alltower.insert(alltower.end(), tmptowers.begin(), tmptowers.end());
							break;
						}
					}
				}
				if (!bendfind) {
                    LOG(ERROR) << "not sequence flight";
					break;
				}
			}
			else {
                LOG(ERROR) << "sequence flight";
				std::vector<AirlineTower> tmptowers;
				bool bfind = false;
				bool bendfind = false;
				int findmode = 0;
				for (int j = 0; j < towers.size(); j++) {
					if (towers[j].towerID == vecTowerInfoForInitialRoute[i].stc_towerId) {
						AirlineTower tmptw;
						tmptw.stc_towerId = towers[j].towerID;
						tmptw.lineID = towers[j].lineID;
						tmptw.stc_x = towers[j].longitude;
						tmptw.stc_y = towers[j].latitude;
						tmptw.stc_z = towers[j].altitude;
						tmptw.zoom = towers[j].insulatorNum;
						auto ts = GetTowerShapeInformationByID(vecTowerInfoForInitialRoute[i].towerShapeID);
						tmptw.stc_insulator_layer = ts.insulator_layer;
						tmptw.stc_leftAngle = ts.towerTypeLeftAngle;
						tmptw.stc_rightAngle = ts.towerTypeRightAngle;
						tmptw.stc_height = ts.tower_height;
						tmptw.stc_ChannelInspection = ts.ChannelInspection;
						tmptw.stc_offCenterAngle = ts.towerTypeOffCenterAngle;

						tmptw.mode = vecTowerInfoForInitialRoute[i].mode;
						findmode = vecTowerInfoForInitialRoute[i].mode;

						if (towers[j].basicID > 0) {

                            string select_sql = "SELECT savePath FROM basicList WHERE basicID = ?";
                            vector<vector<string>> queryResults;
                            vector<string> placeholders = {to_string(towers[j].basicID) };
                            if (SQLiteConnector->executeQueryWithPlaceholder(select_sql, placeholders, queryResults)) {
                                if (!queryResults.empty()) {
                                    const auto& row = queryResults[0];
                                    tmptw.stc_path = row[0];
                                }
                            }
						}

						tmptowers.push_back(tmptw);
						bfind = true;
						continue;
					}
					else if (bfind) {
						if (towers[j].towerID == vecTowerInfoForInitialRoute[i + 1].stc_towerId) {
							bendfind = true;
						}
						else {
							AirlineTower tmptw;
							tmptw.stc_towerId = towers[j].towerID;
							tmptw.lineID = towers[j].lineID;
							tmptw.stc_x = towers[j].longitude;
							tmptw.stc_y = towers[j].latitude;
							auto ts = GetTowerShapeInformationByID(towers[j].towerShapeID);
							tmptw.stc_z = towers[j].altitude + ts.tower_height;
							tmptw.zoom = towers[j].insulatorNum;
							tmptw.pass = 1;
							tmptw.mode = findmode;
							if (towers[j].basicID > 0) {
                                string select_sql = "SELECT savePath FROM basicList WHERE basicID = ?";
                                vector<vector<string>> queryResults;
                                vector<string> placeholders = {to_string(towers[j].basicID) };
                                if (SQLiteConnector->executeQueryWithPlaceholder(select_sql, placeholders, queryResults)) {
                                    if (!queryResults.empty()) {
                                        const auto& row = queryResults[0];
                                        tmptw.stc_path = row[0];
                                    }
                                }
							}
							tmptowers.push_back(tmptw);
						}
						if (bendfind) {
							alltower.insert(alltower.end(), tmptowers.begin(), tmptowers.end());
							break;
						}
				}
	 			}
				if (!bendfind) {
                    LOG(ERROR) << " mode error";
					break;
				}
			}
		}
		
	}
    LOG(ERROR) << "CONTINUE FLY : " << alltower.size();
	for (int i = 0; i < alltower.size(); i++) {
		if (alltower[i].towerVirtual == 1) {
			returntower.push_back(alltower[i]);
		}
		else {
			std::vector<TowerList> towers;
			auto iter = towermap.find(alltower[i].lineID);
			if (iter == towermap.end()) {
				towermap[alltower[i].lineID] = QueryTowersByLineID(alltower[i].lineID, true);
			}
			towers = towermap[alltower[i].lineID];
            LOG(ERROR) << "CURRENT TOWER ID: " << alltower[i].stc_towerId;
			returntower.push_back(alltower[i]);
			prev = findNextTrueTower(towers, alltower[i].stc_towerId, !(alltower[i].mode & 0b100));
            LOG(ERROR) << "PREV TOWER ID: " << prev.stc_towerId;
			prev.towerVirtual = 2;
			returntower.push_back(prev);
			next = findNextTrueTower(towers, alltower[i].stc_towerId, (alltower[i].mode & 0b100));
            LOG(ERROR) << "next TOWER ID: " << next.stc_towerId;
			next.towerVirtual = 2;
			returntower.push_back(next);
		}
	}
    //TRACE(" returntower size : " + to_string(returntower.size()));
    //vector<AirlineTower> returntower;
    LOG(ERROR) << "returntower size : " << returntower.size();
	return returntower;
}

vector<TowerList> DataProcessor::QueryTowersByLineID(int lineid, bool asc)
{
    LOG(ERROR) << " === QueryTowersByLineID ===";
    //TRACE(" === QueryTowersByLineID ===");
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
    vector<TowerList> towers;
    string ascordesc = asc ? "ASC" : "DESC";
    string select_sql = "SELECT * FROM towerList WHERE lineID = ? ORDER BY towerNumber " + ascordesc;
    vector<vector<string>> queryResults;
    vector<string> placeholders = { to_string(lineid) };
    if (SQLiteConnector->executeQueryWithPlaceholder(select_sql, placeholders, queryResults)) {
        for (const auto& row : queryResults) {
            // //TRACE(" row : " + to_string(row.size()));
            TowerList temp;
            temp.towerID = stoi(row[0]);
            temp.towerName = row[1];
            temp.lineID = stoi(row[2]);
            temp.insulatorNum = stoi(row[3]);
            temp.tower_length = stod(row[4]);
            temp.tower_width = stod(row[5]);
            temp.tower_height = stod(row[6]);
            temp.towerType = row[7];
            temp.towerShapeID = stoi(row[8]);
            temp.longitude = stod(row[9]);
            temp.latitude = stod(row[10]);
            temp.altitude = stod(row[11]);
            temp.init_longitude = stod(row[12]);
            temp.init_latitude = stod(row[13]);
            temp.init_altitude = stod(row[14]);
            temp.basicID = stoi(row[15]);
            temp.AccurateLineID = stoi(row[16]);
            temp.createdTime = row[17];
            temp.createManID = stoi(row[18]);
            temp.tower_number = stoi(row[19]);
            temp.comment = row[20];
            temp.photoPosition = stoi(row[21]);


            towers.push_back(temp);
        }
    }

    return towers;
}

TowerShapeList DataProcessor::GetTowerShapeInformationByID(int towershapeid)
{
    LOG(ERROR) << " === GetTowerShapeInformationByID ===";
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
    TowerShapeList temp;
    string select_sql = "SELECT * FROM towerShapeList WHERE towerShapeNameID = ?";
    vector<vector<string>> queryResults;
    vector<string> placeholders = { to_string(towershapeid) };

    if (SQLiteConnector->executeQueryWithPlaceholder(select_sql, placeholders, queryResults)) {
        if (!queryResults.empty()) {
            const auto& row = queryResults[0];
     
            temp.tower_shape_id = stoi(row[0]);
            temp.tower_shape_name = row[1];
            temp.insulator_layer = stoi(row[2]);
            temp.initialroute_id = std::stoi(row[3]);
            temp.tower_length = stod(row[4]);
            temp.tower_width = stod(row[5]);
            temp.tower_height = stod(row[6]);
            temp.created_time = row[7];
            temp.createManID = std::stoi(row[8]);
            temp.lineTypeID = std::stoi(row[9]);
            temp.towerTypeLeftAngle = stod(row[10]);
            LOG(ERROR) << " temp.towerTypeLeftAngle" << temp.towerTypeLeftAngle;

            temp.towerTypeRightAngle = stod(row[11]);
            LOG(ERROR) <<"temp.towerTypeRightAngle : " << temp.towerTypeRightAngle;

            temp.comment = row[12];
            temp.ChannelInspection = std::stoi(row[13]);
            temp.towerTypeOffCenterAngle = stod(row[14]);
        }
    }
    LOG(ERROR) << " FINISHED";
    return temp;
}

AirlineTower DataProcessor::findNextTrueTower(std::vector<TowerList> &towers, int towerid, bool desc)
{
	AirlineTower tw;
	tw.stc_towerId = -1;
	int increase = 1;
	int start = 0;
	int end = towers.size();
	if (desc) {
		increase = -1;
		end = -1;
		start = towers.size() - 1;
	}
	bool bfind = false;
	for (int i=start; i != end; i+=increase )
	{
		if (!bfind) {
			if (towers[i].towerID == towerid) {
				bfind = true;
			}
		}
		else {
			if (towers[i].towerName.front() == '@' || towers[i].towerShapeID == 36) {
				continue;
			}
			else {
				tw.stc_towerId = towers[i].towerID;
				tw.lineID = towers[i].lineID;
				tw.stc_x = towers[i].longitude;
				tw.stc_y = towers[i].latitude;
				tw.stc_z = towers[i].altitude;
				tw.zoom = towers[i].insulatorNum;
				tw.towerShapeID = towers[i].towerShapeID;
				return tw;
			}
		}
	}
	return tw;
}

int DataProcessor::Query_submisson_missionID(int submission_id){   
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
	int missionID = -1;
    string select_sql = "SELECT missionID FROM submissionList WHERE submissionID = ?";
    vector<vector<string>> queryResults;
    vector<string> placeholders = {to_string(submission_id)};
    if (SQLiteConnector->executeQueryWithPlaceholder(select_sql, placeholders, queryResults)) {
        if (!queryResults.empty()) {
            const auto& row = queryResults[0];
            missionID = stoi(row[0]);
        }
    }
    LOG(ERROR) << missionID;
	return missionID;
}

int DataProcessor::get_photoPosition(int towerid){
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
	int photopos = 0;
    string select_sql = "SELECT photoPosition FROM towerList WHERE towerID = ? ";
    vector<vector<string>> queryResults;
    vector<string> placeholders = {to_string(towerid)};
    if (SQLiteConnector->executeQueryWithPlaceholder(select_sql, placeholders, queryResults)) {
        if (!queryResults.empty()) {
            const auto& row = queryResults[0];
            photopos = stoi(row[0]);
        }
    }
    // //TRACE("photopos : " + to_string(photopos));
	return photopos;
}

int DataProcessor::Query_lintTypeID(int towerid){
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
	int lint_type_id = 0;
    string select_sql = "SELECT towerShapeList.lineTypeID FROM towerShapeList WHERE towerShapeNameID IN( \
			SELECT towerList.towerShapeID FROM towerList WHERE towerID = ? ); ";
    vector<vector<string>> queryResults;
    vector<string> placeholders = {to_string(towerid)};
    if (SQLiteConnector->executeQueryWithPlaceholder(select_sql, placeholders, queryResults)) {
        if (!queryResults.empty()) {
            const auto& row = queryResults[0];
            lint_type_id = stoi(row[0]);
        }
    }
    LOG(ERROR) << "lint_type_id " << lint_type_id;
    return lint_type_id;
}

TowerList DataProcessor::getFirstTower(int submission_id){
    LOG(ERROR) << "=== getFirstTower === ";
    TowerList temp;
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
	int lint_type_id;
    string query = "SELECT * FROM towerList WHERE towerID in (select towerID from submissiontowerList WHERE submissionID = ? ORDER BY towerNumber limit 1);";
    vector<string> placeholders;
    placeholders.push_back(to_string(submission_id));
    vector<vector<string>> queryResults;
    LOG(ERROR) << " === query db ===";
    if (SQLiteConnector->executeQueryWithPlaceholder(query, placeholders, queryResults)) {
        for (const auto& row : queryResults) {
            temp.towerID = stoi(row[0]);
            temp.towerName = row[1]; 
            temp.lineID = stoi(row[2]); 
            temp.insulatorNum = stoi(row[3]);
            temp.tower_length = stod(row[4]); 
            temp.tower_width = stod(row[5]); 
            temp.tower_height = stod(row[6]);
            temp.towerType = row[7];
            temp.towerShapeID = stoi(row[8]);
            temp.longitude =  stod(row[9]);
            temp.latitude =  stod(row[10]);
            temp.altitude =  stod(row[11]);
            temp.init_longitude =  stod(row[12]);
            temp.init_latitude = stod(row[13]);
            temp.init_altitude =  stod(row[14]);
            temp.basicID = stoi(row[15]);
            temp.AccurateLineID = stoi(row[16]);
            temp.createdTime = row[17];
            temp.createManID = stoi(row[18]);
            temp.tower_number = stoi(row[19]);
            temp.comment = row[20];
            temp.photoPosition = stoi(row[21]);
       
	    }
    }
    LOG(ERROR) << " === return result ===";
	return temp;
}

vector<WaypointInformation> DataProcessor::generateMinorAirline(int submissionID, WaypointInformation current_point, int flighttasktype){
    //存装杆塔信息
    LOG(ERROR) << "==== generateMinorAirline ===="  ;
	std::vector<BasicResultGet> tower_data;

	string data;

	// CommissionType2Post tempcommissionType2Post;

	// FlyingCommission1Post tempflyingCommission1Post;

	std::vector<WaypointInformation> vec_to_zj;
	if (flighttasktype == 2) {//续飞
        //TRACE("flighttasktype == 2");
        FileSystem* fileInterface = new FileSystem();
		std::vector<WaypointInformation> vecComplete = readRoute(submissionID, fileInterface);
		vec_to_zj = getContinueWaypoints(submissionID, vecComplete);
        delete fileInterface;
	} else {
		vec_to_zj = GetSubmissionWaypoints(submissionID);
	}

    if (vec_to_zj.size() < 3) {
        LOG(ERROR) << "data error in GenerateMinorAirline : " << vec_to_zj.size();
        return std::vector<WaypointInformation>();
    }
    LOG(ERROR) << "continue getMinorAirline ";
	WaypointInformation point_end = vec_to_zj[0];
    LOG(ERROR) << "point_end POI_X : " << point_end.POI_X;
    LOG(ERROR) << "point_end  POI_Y : " << point_end.POI_Y;
    LOG(ERROR) << "point_end  POI_Z : " << point_end.POI_Z;

	/////////////////////////////////////////把降落点的gps = landing_interest_point/////////////////////////////////////
	/////////////////////////////////////////如果没有降落点,最后//////////////////////////////////
    int returnPolicy = getReturnPolicy(submissionID);
    LOG(ERROR) << " returnPolicy : " << returnPolicy;
    //if (returnPolicy == 0 || returnPolicy == 1) { 
    //    LOG(ERROR) << " returnPolicy : " << returnPolicy;
    //    // follow tower return  AND direct return 
    //    LongitudeAndLatitude landing_interest_point;
    //    landing_interest_point = Get_LongitudeAndLatitude(submissionID);
    //    landing_interest_point.stc_altitude = 0;
    //    CDatumPoint* m_CDatumPoint = new CDatumPoint();
    //    m_CDatumPoint->setOrigin(vec_to_zj[vec_to_zj.size() - 1].stc_y, vec_to_zj[vec_to_zj.size() - 1].stc_x, vec_to_zj[vec_to_zj.size() - 1].stc_z, true);
    //    LongitudeAndLatitude landing_interest_point;
    //    landing_interest_point = Get_LongitudeAndLatitude(submissionID);
    //    landing_interest_point.stc_altitude = 0;
    //    CDatumPoint* m_CDatumPoint = new CDatumPoint();
    //    m_CDatumPoint->setOrigin(vec_to_zj[vec_to_zj.size() - 1].stc_y, vec_to_zj[vec_to_zj.size() - 1].stc_x, vec_to_zj[vec_to_zj.size() - 1].stc_z, true);
    //    point_end.POI_Y = 100 * m_CDatumPoint->longitudeAndLatitudeToDatumPoint(landing_interest_point).stc_y;
    //    point_end.POI_X = 100 * m_CDatumPoint->longitudeAndLatitudeToDatumPoint(landing_interest_point).stc_x;
    //    point_end.POI_Z = -45;
    //    point_end.stc_x = landing_interest_point.stc_longitude;
    //    point_end.stc_y = landing_interest_point.stc_latitude;
    //    point_end.stc_z = landing_interest_point.stc_altitude;
    //    if (landing_interest_point.stc_latitude == 0 && landing_interest_point.stc_longitude == 0) {
    //        point_end.POI_Z = -1;
    //        //当前点复值过来
    //        point_end.stc_x = current_point.stc_x;
    //        point_end.stc_y = current_point.stc_y;
    //        point_end.stc_z = current_point.stc_z;
    //    }
    //    point_end.stc_index = vec_to_zj.size();
    //    point_end.stc_shootPhoto = true;
    //    point_end.target_class_id[0] = 7;
    //    point_end.necessary = false;
    //    vec_to_zj.push_back(point_end);
    //    WaypointInformation point_start = vec_to_zj[0];
    //    int flightHeight = Query_submisson_flightHeight(submissionID);
    //    point_start.stc_shootPhoto = false;
    //    point_start.necessary = true;
    //    point_start.stc_gimbalPitch = -90;
    //    point_start.tower_id = -1;
    //    vec_to_zj.insert(vec_to_zj.begin(), point_start);
    //}else if (returnPolicy == 2) {
    //    //Off-site landing
    //    LongitudeAndLatitude landing_interest_point;
    //    landing_interest_point = getOffSiteReturnAddr(submissionID);
    //    CDatumPoint* m_CDatumPoint = new CDatumPoint();
    //    m_CDatumPoint->setOrigin(vec_to_zj[vec_to_zj.size() - 1].stc_y, vec_to_zj[vec_to_zj.size() - 1].stc_x, vec_to_zj[vec_to_zj.size() - 1].stc_z, true);
    //    point_end.POI_Y = 100 * m_CDatumPoint->longitudeAndLatitudeToDatumPoint(landing_interest_point).stc_y;
    //    point_end.POI_X = 100 * m_CDatumPoint->longitudeAndLatitudeToDatumPoint(landing_interest_point).stc_x;
    //    point_end.POI_Z = -45;
    //    point_end.stc_x = landing_interest_point.stc_longitude;
    //    point_end.stc_y = landing_interest_point.stc_latitude;
    //    point_end.stc_z = landing_interest_point.stc_altitude;
    //    if (landing_interest_point.stc_latitude == 0 && landing_interest_point.stc_longitude == 0) {
    //        point_end.POI_Z = -1;
    //        //当前点复值过来
    //        point_end.stc_x = current_point.stc_x;
    //        point_end.stc_y = current_point.stc_y;
    //        point_end.stc_z = current_point.stc_z;
    //    }
    //    point_end.stc_index = vec_to_zj.size();
    //    point_end.stc_shootPhoto = true;
    //    point_end.target_class_id[0] = 7;
    //    point_end.necessary = false;
    //    vec_to_zj.push_back(point_end);
    //    WaypointInformation point_start = vec_to_zj[0];
    //    double flightHeight = landing_interest_point.stc_altitude;
    //    point_start.stc_z = flightHeight + current_point.stc_z;
    //    point_start.stc_shootPhoto = false;
    //    point_start.necessary = true;
    //    point_start.stc_gimbalPitch = -90;
    //    point_start.tower_id = -1;
    //    vec_to_zj.insert(vec_to_zj.begin(), point_start);
    //}
    //


	LongitudeAndLatitude landing_interest_point;
	landing_interest_point = Get_LongitudeAndLatitude(submissionID);
	landing_interest_point.stc_altitude = 0;
    //TRACE("stc_y :"+ to_string(vec_to_zj[vec_to_zj.size() - 1].stc_y));
    LOG(ERROR) << "stc_y: " << vec_to_zj[vec_to_zj.size() - 1].stc_y;
    //TRACE("stc_x :"+ to_string( vec_to_zj[vec_to_zj.size() - 1].stc_x));
    LOG(ERROR) << "stc_x : " << vec_to_zj[vec_to_zj.size() - 1].stc_x;
    //TRACE("stc_z :"+ to_string(vec_to_zj[vec_to_zj.size() - 1].stc_z));
    LOG(ERROR) << "stc_z : " << vec_to_zj[vec_to_zj.size() - 1].stc_z;
    CDatumPoint* m_CDatumPoint = new CDatumPoint();
	m_CDatumPoint->setOrigin(vec_to_zj[vec_to_zj.size() - 1].stc_y, vec_to_zj[vec_to_zj.size() - 1].stc_x, vec_to_zj[vec_to_zj.size() - 1].stc_z, true);
	//latitude can calculate y
	point_end.POI_Y = 100 * m_CDatumPoint->longitudeAndLatitudeToDatumPoint(landing_interest_point).stc_y;
	//TRACE("point_end.POI_Y :" + to_string(point_end.POI_Y));
    ////longitude can calculate x
	point_end.POI_X = 100 * m_CDatumPoint->longitudeAndLatitudeToDatumPoint(landing_interest_point).stc_x;
	//TRACE("point_end.POI_X :" + to_string(point_end.POI_X));
    ////relativeHeight can calculate z
	point_end.POI_Z = -45;
    //TRACE("point_end.POI_Z :" + to_string(point_end.POI_Z));
	point_end.stc_x = landing_interest_point.stc_longitude;
	point_end.stc_y = landing_interest_point.stc_latitude;
	point_end.stc_z = landing_interest_point.stc_altitude;
	if (landing_interest_point.stc_latitude == 0 && landing_interest_point.stc_longitude == 0) {
		point_end.POI_Z = -1;
		//当前点复值过来
		point_end.stc_x = current_point.stc_x;
		point_end.stc_y = current_point.stc_y;
		point_end.stc_z = current_point.stc_z;
	}
	point_end.stc_index = vec_to_zj.size();
	point_end.stc_shootPhoto = true;
	point_end.target_class_id[0] = 7;
	point_end.necessary = false;
	//vec_to_zj.push_back(point_end);
	WaypointInformation point_start = vec_to_zj[0];
	int flightHeight = Query_submisson_flightHeight(submissionID);
	point_start.stc_z = flightHeight+ current_point.stc_z;
	point_start.stc_shootPhoto = false;
	point_start.necessary = true;
	point_start.stc_gimbalPitch = -90;
	point_start.tower_id = -1;
	vec_to_zj.insert(vec_to_zj.begin(), point_start);

	/////////////////////////////////////////然后把vec_to_zj直接传给大神/////////////////////////////////
	////////////////////////更改速度参数//////////////////////////////////
	auto pre_iter = vec_to_zj.begin();
	for (std::vector<WaypointInformation>::iterator next_iter = ++vec_to_zj.begin(); next_iter != vec_to_zj.end() && pre_iter != vec_to_zj.end(); pre_iter++, next_iter++) {
		float dis = CalcPointDis(*pre_iter, *next_iter);
		if (dis >= 50.0) {
			pre_iter->maxFlightSpeed = 10.0;
		}
		else {
			pre_iter->maxFlightSpeed = 5.0;
		}
		for (auto item : pre_iter->target_class_id)
		{
			if (item == 0 && pre_iter->stc_shootPhoto == 1)
			{
				pre_iter->maxFlightSpeed = 1.2;
				break;
			}
		}
	}

	////////////////////////更改速度参数//////////////////////////////////

    delete m_CDatumPoint;
	// //更改子任务状态
	Change_subtask_state(submissionID);
    //TRACE("vec_to_zj size :" + to_string(vec_to_zj.size()));
	return vec_to_zj;
}

int DataProcessor::getReturnPolicy(int submission_id) {
    int returnPolicy = -1;
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
    string query = "SELECT policyID FROM submissionList WHERE submissionID = ? ";
    vector<string> placeholders;
    placeholders.push_back(to_string(submission_id));
    vector<vector<string>> queryResults;
    if (SQLiteConnector->executeQueryWithPlaceholder(query, placeholders, queryResults)) {
        if (!queryResults.empty() && !queryResults[0].empty()) {
            // 将结果从字符串转换为整数
            returnPolicy = std::stoi(queryResults[0][0]);
        }
    }

    return returnPolicy;

}

LongitudeAndLatitude DataProcessor::getOffSiteReturnAddr(int submission_id) {
    LOG(ERROR) << " === getOffSiteReturnAddr ===";
    LongitudeAndLatitude temp;
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
    int lint_type_id;
    string query = "SELECT landingLongtitude,landingLatitude,landingAltitude FROM submissionList WHERE submissionID = ? ";
    vector<string> placeholders;
    placeholders.push_back(to_string(submission_id));
    vector<vector<string>> queryResults;
    if (SQLiteConnector->executeQueryWithPlaceholder(query, placeholders, queryResults)) {
        for (const auto& row : queryResults) {
            temp.stc_longitude = stod(row[0]);
            temp.stc_latitude = stod(row[1]);
            temp.stc_altitude = stod(row[2]);
        }
    }
    return temp;
}

LongitudeAndLatitude DataProcessor::Get_LongitudeAndLatitude(int submission_id){
    LOG(ERROR) << " === Get_LongitudeAndLatitude ===";
    LongitudeAndLatitude temp;
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
	int lint_type_id;
    string query = "SELECT landingLongtitude,landingLatitude FROM submissionList WHERE submissionID = ? ";
    vector<string> placeholders;
    placeholders.push_back(to_string(submission_id));
    vector<vector<string>> queryResults;
    if (SQLiteConnector->executeQueryWithPlaceholder(query, placeholders, queryResults)) {
        for (const auto& row : queryResults) {
            if (row[0].empty())
            {
                temp.stc_longitude = 0;
            }
            else {
                temp.stc_longitude = stod(row[0]);
            }
            if (row[1].empty())
            {
                temp.stc_latitude = 0;
            }
            else {
                temp.stc_latitude = stod(row[1]);
            }
           
            //temp.stc_latitude = stod(row[1]);
        }
    }
    return temp;
}

int DataProcessor::GetDoublePhotoBySubmissionID(int submission_id){
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
    string query =  "SELECT doublePhoto FROM submissionList WHERE submissionID = ? ";;
    vector<string> placeholders;
    placeholders.push_back(to_string(submission_id));
    vector<vector<string>> queryResults;
    if (SQLiteConnector->executeQueryWithPlaceholder(query, placeholders, queryResults)) {
        for (const auto& row : queryResults) {
            int doublePhoto = 0;
            if (row[0].empty())
            {
                return doublePhoto;
            }
            else {
                doublePhoto = stoi(row[0]);
                return doublePhoto;
            }
           
        }
    }
    return -1;

}

double DataProcessor::CalcPointDis(const WaypointInformation & l, const WaypointInformation & r)
{
	const double pi = 3.1415926535897923846;
	const float earth_radius = 6378137.0;
	double a = l.stc_y - r.stc_y;
	double b = l.stc_x - r.stc_x;
	a = a * pi / 180;
	b = b * pi / 180;
	return sqrt(pow(a * earth_radius, 2) + pow(cos(l.stc_y * pi / 180) * b * earth_radius, 2));
}

int DataProcessor::Query_submisson_flightHeight(int submission_id)
{
    int height = 0;
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
    string query =  "SELECT flightHeight FROM submissionList WHERE submissionID =  ? ";
    vector<string> placeholders;
    placeholders.push_back(to_string(submission_id));
    vector<vector<string>> queryResults;
    if (SQLiteConnector->executeQueryWithPlaceholder(query, placeholders, queryResults)) {
        for (const auto& row : queryResults) {
            if (row[0].empty()) {
                height = 0;
            }
            else{
                height = stoi(row[0]);
            }
           
        }
    }
    return height;
}

void DataProcessor::Change_subtask_state(int submission_id)
{
    LOG(ERROR) << "change subtask state : " << submission_id;
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
    std::string update_sql = "UPDATE submissionList SET completeStatus = '1' WHERE submissionID = " + std::to_string(submission_id) + ";";
    
    if (SQLiteConnector->executeNonQuery(update_sql)) {
        LOG(ERROR) << "Subtask state changed successfully.";
    } else {
        LOG(ERROR) << "Failed to change subtask state.";
    }
}

void DataProcessor::changeTowerGPT(const uint8_t* data, int length, Session* session) {
    LOG(ERROR) << "=== changeTowerGPT === " ;
    /*
        int submissionid = stoi(session->getTask()->getSubmissionID());
        LOG(ERROR) << "submissionid : " << submissionid;
    */
    int missionid = 0;
    try {
        if (length > 3) {
            uint8_t* data1 = beforeSerialization(data, length);
            int newDataLen = length - 3;
            LOG(ERROR) << "newDataLen : " << newDataLen;
            list<Gps> GpsList;
            stringstream ssid;
            stringstream ss;
            ssid << string((const char*)data1, sizeof(missionid));
            UnSerialization(ssid, missionid);
            LOG(ERROR) << "missionid : " << missionid;
            int len = sizeof(missionid);

            ss << string((const char*)data1 + len, newDataLen - len);
            UnSerialization(ss, GpsList);
            LOG(ERROR) << "GpsList : " << GpsList.size();
            list<int> towerList = getTowerList(missionid);
            list<Gps> newPoints;
            auto it = GpsList.begin(); 
            ++it; 
            while (it != GpsList.end()) {
                LOG(ERROR) << std::fixed << std::setprecision(10) << "gps.stc_lon in GpsList: " << it->stc_lon;
                LOG(ERROR) << std::fixed << std::setprecision(10) << "gps.stc_lat in GpsList : " << it->stc_lat;
                LOG(ERROR) << std::fixed << std::setprecision(10) << "gps.stc_alt in GpsList : " << it->stc_alt;
                newPoints.push_back(*it);
                ++it;
            }

            auto itt = newPoints.begin();
            while (itt != newPoints.end()) {
                LOG(ERROR) << std::fixed << std::setprecision(10) << "gps.stc_lon in newPoints: " << itt->stc_lon;
                LOG(ERROR) << std::fixed << std::setprecision(10) << "gps.stc_lat in newPoints : " << itt->stc_lat;
                LOG(ERROR) << std::fixed << std::setprecision(10) << "gps.stc_alt in newPoints : " << itt->stc_alt;
                ++itt;
            }

            LOG(ERROR) << "newPoints : " << newPoints.size();

            if (updateTowerGPS(towerList, newPoints))
            {
                uint8_t answer = 0x01; //success
                vector<uint8_t> infoPackeage = makePointAnswerPackage(missionid, answer);
                SSIZE_T bytesSent = send(session->socket(), reinterpret_cast<const char*>(infoPackeage.data()), infoPackeage.size(), 0);
            }
            else {
                throw std::runtime_error("update error.");
            }

        }
        else {
            LOG(ERROR) << "data format error !" ;
            throw std::runtime_error("Data format error: length is too small.");
        }
    }catch(exception& e){
        uint8_t answer = 0x00; //fail
        vector<uint8_t> infoPackeage = makePointAnswerPackage(missionid, answer);
      /*  for (const uint8_t byte : infoPackeage) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
        }*/
        SSIZE_T bytesSent = send(session->socket(), reinterpret_cast<const char*>(infoPackeage.data()), infoPackeage.size(), 0);
        LOG(ERROR) << "Exception caught: " << e.what();
    }
}



void DataProcessor::getWifiRecord(const uint8_t* data, int length, Session* session) {
    try {
        LOG(ERROR) << " === getWifiRecord ===";
        if (length > 3) {
            uint8_t* data1 = beforeSerialization(data, length);
            int newDataLen = length - 3;
            stringstream ssid;
            stringstream ss;

            char fields1[32] = { 0 };
            LOG(ERROR) << "size of fields1 : " << sizeof(fields1);
            ssid << string((const char*)data1, sizeof(fields1));
            UnSerialization(ssid, fields1);
            LOG(ERROR) << "fields1 : " << fields1;

            char value1[64] = { 0 };
            LOG(ERROR) << "size of value1 : " << sizeof(value1);
            ssid << string((const char*)data1 + sizeof(fields1), sizeof(value1));
            UnSerialization(ssid, value1);
            LOG(ERROR) << "value1 : " << value1;

            char fields2[32] = { 0 };
            LOG(ERROR) << "size of fields2 : " << sizeof(fields2);
            ssid << string((const char*)data1 + sizeof(fields1) + sizeof(value1), sizeof(fields2));
            UnSerialization(ssid, fields2);
            LOG(ERROR) << "fields2 : " << fields2;

            char value2[64] = { 0 };
            LOG(ERROR) << "size of value2 : " << sizeof(value2);
            ssid << string((const char*)data1 + sizeof(fields1) + sizeof(value1) + sizeof(fields2), sizeof(value2));
            UnSerialization(ssid, value2);
            LOG(ERROR) << "value2 : " << value2;

            char fields3[32] = { 0 };
            LOG(ERROR) << "size of fields3 : " << sizeof(fields3);
            ssid << string((const char*)data1 + sizeof(fields1) + sizeof(value1) + sizeof(fields2) + sizeof(value2), sizeof(fields3));
            UnSerialization(ssid, fields3);
            LOG(ERROR) << "fields3 : " << fields3;

            char value3[64] = { 0 };
            LOG(ERROR) << "size of value3 : " << sizeof(value3);
            ssid << string((const char*)data1 + sizeof(fields1) + sizeof(value1) + sizeof(fields2) + sizeof(value2) + sizeof(fields3), sizeof(value3));
            UnSerialization(ssid, value3);
            LOG(ERROR) << "value3 : " << value3;


            LOG(ERROR) << "sncode : " << session->getSncode();
            if (!storeWifiInfor2DB(value1, value2, value3)){
                LOG(ERROR) << "store wifi infor to DB fail !! ";
                return; 
            }

            uint8_t cmd = 0x14;
            uint8_t answer = 0x01;
            vector<uint8_t> info;
            info.push_back(cmd);
            int answerLen = 1;
            uint16_t u16 = static_cast<uint16_t>(answerLen);
            info.push_back(static_cast<uint8_t>((u16 >> 8) & 0xFF)); // 高字节
            info.push_back(static_cast<uint8_t>(u16 & 0xFF));        // 低字节
            info.push_back(answer);
            for (const uint8_t byte : info) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
            }
            SSIZE_T bytesSent = send(session->socket(), reinterpret_cast<const char*>(info.data()), info.size(), 0);
            LOG(ERROR) << "info size : " << info.size();
            LOG(ERROR) << "bytesSent : " << bytesSent;
        }

    }
    catch (exception& e) {
        uint8_t answer = 0x00; //fail
        vector<uint8_t> info;
        uint8_t cmd = 0x14;
        info.push_back(cmd);
        int answerLen = 1;
        uint16_t u16 = static_cast<uint16_t>(answerLen);
        info.push_back(static_cast<uint8_t>((u16 >> 8) & 0xFF)); // 高字节
        info.push_back(static_cast<uint8_t>(u16 & 0xFF));        // 低字节
        info.push_back(answer);
        for (const uint8_t byte : info) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
        }
        SSIZE_T bytesSent = send(session->socket(), reinterpret_cast<const char*>(info.data()), info.size(), 0);
        LOG(ERROR) << "info size : " << info.size();
        LOG(ERROR) << "bytesSent : " << bytesSent;
    }
}



bool DataProcessor::storeWifiInfor2DB(std::string wifiName, std::string wifiPassword, std::string cpuID) {
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();

    std::time_t timestamp = std::time(nullptr);
    LOG(ERROR) << "Timestamp: " << timestamp;
    unsigned long ts = timestamp;
    string lastTime = to_string(ts);
    LOG(ERROR) << "lastTime: " << lastTime;
    string result{};
    string selectQuery = "SELECT COUNT(*) FROM wifiRecord WHERE cpuID = " + cpuID + "; ";
    if (SQLiteConnector->executeScalar(selectQuery, result)) {
        int count = stoi(result);
        if (count == 0) {
            string wifiRecordINSERTSQL = "INSERT INTO wifiRecord (wifiName, wifiPassword, cpuID, lastTime) VALUES (?, ?, ?, ?);";
            vector<string> recordPlaceholders = {
                wifiName,
                wifiPassword,
                cpuID,
                lastTime
            };

            if (SQLiteConnector->executeDroneInsertQuery(wifiRecordINSERTSQL, recordPlaceholders)) {
                LOG(ERROR) << "Insertion successful,update WiFiRecord";
            }
            else {
                LOG(ERROR) << "Insert to wifiRecord failed";
                return false;
            }
        }
        else {
            LOG(ERROR) << "Duplicate cpuID found ";
            vector<std::string> snCodeVec;
            std::string updateQuery = "UPDATE wifiRecord SET wifiName = ? ,wifiPassword = ?, lastTime= ? WHERE cpuID = ?";
            snCodeVec.push_back(wifiName);
            snCodeVec.push_back(wifiPassword);
            snCodeVec.push_back(lastTime);
            snCodeVec.push_back(cpuID);
            if (SQLiteConnector->executeDroneInsertQuery(updateQuery, snCodeVec)) {
                LOG(ERROR) << "update executed drone successfully and update wifiRecord";
            }
            else {
                LOG(ERROR) << "Error executing drone update query.";
                return false;
            }
        }
    }
    else {
        LOG(ERROR) << "Database query COUNT(*) failed";
        return false;
    }




    //string wifiRecordINSERTSQL = "INSERT INTO wifiRecord (snCode, wifiName, wifiPassword, lastTime) VALUES (?, ?, ?, ?);";
    //vector<string> recordPlaceholders = {
    //    snCode,
    //    wifiName,
    //    wifiPassword,
    //    lastTime
    //};

    //if (SQLiteConnector->executeDroneInsertQuery(wifiRecordINSERTSQL, recordPlaceholders)) {
    //    LOG(ERROR) << "Insertion successful,update WiFiRecord";
    //}else {
    //    LOG(ERROR) << "Insert to wifiRecord failed";
    //    return false;
    //}
    return true;
}


void DataProcessor::getRecord(const uint8_t* data, int length, Session* session){
    int missionid = 0;
    try{
        FileSystem* fileInterface = new FileSystem();
        LOG(ERROR) << " === getRecord ===";
        if(length > 3){
            uint8_t* data1 =  beforeSerialization(data,length);
            int newDataLen = length - 3;
            LOG(ERROR) << "newDataLen : " << newDataLen;
            list<WaypointInformation> way_point;
            stringstream ssid;
            stringstream ss;



            char snCode[24] = { 0 };
            LOG(ERROR) << "size of snCode : " << sizeof(snCode);
            ssid << string((const char*)data1, sizeof(snCode));
            UnSerialization(ssid, snCode);
            LOG(ERROR) << "snCode : " << snCode;

            int missionid = 0;
            ssid << string((const char*)data1 + sizeof(snCode), sizeof(missionid));
            UnSerialization(ssid, missionid);
            LOG(ERROR) << "missionid : " << missionid;

            char flighttype = 0;
            ssid << string((const char*)data1 + sizeof(snCode) + sizeof(missionid), sizeof(flighttype));
            UnSerialization(ssid, flighttype);
            LOG(ERROR) << "flighttype : " << (int)flighttype;

            unsigned int timestamp = 0;
            ssid << string((const char*)data1 + sizeof(snCode) + sizeof(missionid)+ sizeof(flighttype), sizeof(timestamp));
            UnSerialization(ssid, timestamp);
            LOG(ERROR) << "timestamp : " << timestamp;



            int len = sizeof(snCode)+ sizeof(missionid) + sizeof(flighttype) + sizeof(timestamp);
            if (newDataLen <= len) {
                LOG(ERROR) << "error format " ;
		        return;
	        }
            LOG(ERROR) << "way point : " << newDataLen - len;
            ss << string((const char*)data1 + len, newDataLen - len);
            UnSerialization(ss, way_point);
          

            vector<WaypointInformation> wpvec;
            for (auto listiter = way_point.begin(); listiter != way_point.end(); listiter++) {
                wpvec.push_back(*listiter);
            }
            if (wpvec.size() == 0) {
                LOG(ERROR) << "return empty waypoints";
                return;
            }
           
            if (wpvec[wpvec.size()-1].waypointType == 9  && flighttype == 2) {
                //缺电返回
                fileInterface->writeRoute(wpvec, missionid, timestamp);
            } else {
                //正常返回
                LOG(ERROR) << "==== nomal return ===";
                uint32_t tmptimestamp = 0;
                if (fileInterface->checkBreakTask(missionid, tmptimestamp)) {
                       if (!fileInterface->mergeContinueWaypoints(missionid, wpvec)) {
                        LOG(ERROR) << "MergeContinueWaypoints error!";
                        return; 
                    }
                   updateCompleteMissionStatus(missionid);

                } else {
                    fileInterface->writeRoute(wpvec, missionid);
                    if (updateCompleteMissionStatus(missionid)) {
                        insertToRecordList(missionid, snCode);
                    }
                

                }
            }
            //TRACE(" === send info === ");
            uint8_t answer = 0x01;
            vector<uint8_t> infoPackeage = makeInfoPackage(missionid, answer);
           // vector<uint8_t> infoPackeage = makeInfoPackage(submissionid, answer);
            for (const uint8_t byte : infoPackeage) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
            }
            SSIZE_T bytesSent = send(session->socket(), reinterpret_cast<const char*>(infoPackeage.data()), infoPackeage.size(), 0);
            delete fileInterface;
        } 
    } catch(exception& e){
        uint8_t answer = 0x00; //fail
        vector<uint8_t> infoPackeage = makeInfoPackage(missionid, answer);
        //vector<uint8_t> infoPackeage = makeInfoPackage(submissionid, answer);
        for (const uint8_t byte : infoPackeage) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
        }
        SSIZE_T bytesSent = send(session->socket(), reinterpret_cast<const char*>(infoPackeage.data()), infoPackeage.size(), 0);
        //TRACE("getRecord UnSerialization error");
    }

}

vector<uint8_t> DataProcessor::makeInfoPackage(int missionid, uint8_t answer){
    vector<uint8_t> info;
    uint8_t cmd = 0x08;
    info.push_back(cmd);
    int answerLen = 5;
    uint16_t u16 = static_cast<uint16_t>(answerLen);
    info.push_back(static_cast<uint8_t>((u16 >> 8) & 0xFF)); // 高字节
    info.push_back(static_cast<uint8_t>(u16 & 0xFF));        // 低字节
        
    stringstream missionID;
    Serialization(missionID, missionid);
    string serializedData = missionID.str();
    //TRACE(" serializedData :" + serializedData);

    for (size_t i = 0; i < 4; ++i) {
        uint8_t byte = static_cast<uint8_t>(serializedData[i]);
        info.push_back(byte);
    }

    info.push_back(answer);
    return info;
}


vector<uint8_t> DataProcessor::makePointAnswerPackage(int missionid, uint8_t answer) {
    vector<uint8_t> info;
    uint8_t cmd = 0x07;
    info.push_back(cmd);
    int answerLen = 5;
    uint16_t u16 = static_cast<uint16_t>(answerLen);
    info.push_back(static_cast<uint8_t>((u16 >> 8) & 0xFF)); // 高字节
    info.push_back(static_cast<uint8_t>(u16 & 0xFF));        // 低字节

    stringstream missionID;
    Serialization(missionID, missionid);
    string serializedData = missionID.str();
    //TRACE(" serializedData :" + serializedData);

    for (size_t i = 0; i < 4; ++i) {
        uint8_t byte = static_cast<uint8_t>(serializedData[i]);
        info.push_back(byte);
    }

    info.push_back(answer);
    return info;
}



bool DataProcessor::updateTowerGPS(list<int> towerList, list<Gps> newPoints) {
    if (towerList.size() != newPoints.size()) {

        return false;
    }
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
    vector<std::string> placeholders;
    auto towerIt = towerList.begin();
    auto gpsIt = newPoints.begin();
    while (towerIt != towerList.end() && gpsIt != newPoints.end()) {
        placeholders.clear();

        /* 
        LOG(ERROR) << "stc_lon : " << gpsIt->stc_lon;
        LOG(ERROR) << "longitude : " << to_string(gpsIt->stc_lon);

        LOG(ERROR) << "stc_lat : " << gpsIt->stc_lat;
        LOG(ERROR) << "latitude : " << to_string(gpsIt->stc_lat);

        LOG(ERROR) << "stc_alt : " << gpsIt->stc_alt;
        LOG(ERROR) << "altitude : " << to_string(gpsIt->stc_alt);
        
        LOG(ERROR) << "towerIt" << *towerIt;*/


        std::stringstream sslon;
        sslon << std::fixed << std::setprecision(10) << gpsIt->stc_lon;
        LOG(ERROR) << std::fixed << std::setprecision(10) << "gpsIt stc_lon : " << gpsIt->stc_lon;
        string stc_lon = sslon.str();
        LOG(ERROR) << "stc_lon : " << stc_lon;
        std::stringstream sslat;
        sslat << std::fixed << std::setprecision(10) << gpsIt->stc_lat;
        LOG(ERROR) << std::fixed << std::setprecision(10) << "gpsIt stc_lat : " << gpsIt->stc_lat;
        string stc_lat = sslat.str();
        LOG(ERROR) << "stc_lat : " << stc_lat;
        std::stringstream ssalt;
        ssalt << std::fixed << std::setprecision(10) << gpsIt->stc_alt - 10;
        LOG(ERROR) << std::fixed << std::setprecision(10) << "gpsIt stc_alt  : " << gpsIt->stc_alt - 10;
        string stc_alt = ssalt.str();
        LOG(ERROR) << "stc_alt : " << stc_alt;
        placeholders.push_back(stc_lon);
        placeholders.push_back(stc_lat);
        placeholders.push_back(stc_alt);
        placeholders.push_back(to_string(*towerIt));

        string query = "update towerList set longitude = ?, latitude = ?, altitude = ? where towerID = ?;";
        if (!SQLiteConnector->executeDroneInsertQuery(query, placeholders)) {
            return false;
        }

        ++towerIt;
        ++gpsIt;
    }

    return true;
}

//bool DataProcessor::updateCompleteMissionStatus(int submissionID){
//    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance(); 
//    std::string updateQuery = "UPDATE submissionList SET completeStatus = '2', droneSN = '' WHERE submissionID = '" + std::to_string(submissionID) + "';";
//    bool result = SQLiteConnector->executeNonQuery(updateQuery);    
//    return result;
//}

//uint64_t getTimestamp()
//{
//    std::time_t timestamp = std::time(nullptr);
//
//    LOG(ERROR) << "Timestamp: " << timestamp ;
//    unsigned long ts = timestamp;
//
//    SYSTEMTIME curtime;
//     GetSystemTime(&curtime);
//     LOG(ERROR) << "system hour : " << curtime.wHour << "minute : " << curtime.wMinute;
//    GetLocalTime(&curtime);
//    LOG(ERROR) << "local hour : " << curtime.wHour << "minute : " << curtime.wMinute;
//
//
//    return ts;
//
//}

bool DataProcessor::updateCompleteMissionStatus(int submissionID) {
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
   /* std::string updateQuery = "UPDATE submissionList SET completeStatus = 3, droneSN = NULL WHERE submissionID = ? ";
    bool result = SQLiteConnector->executeNonQuery(updateQuery);
    return r(esult;*/
    vector<std::string> holders;

    std::time_t timestamp = std::time(nullptr);
    struct tm timeInfo;
    char buffer[20];
    localtime_s(&timeInfo, &timestamp);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeInfo);
    LOG(ERROR) << " buffer : " << buffer;
    string status = "3";
    //string droneID = NULL;
    string planDate = buffer;
    std::string updateQuery = "UPDATE submissionList SET completeStatus = ?, droneID = NULL, planDate = ?  WHERE submissionID = ? ";
    holders.push_back(status);
    //holders.push_back(droneID);
    holders.push_back(planDate);
    holders.push_back(to_string(submissionID));
    if (SQLiteConnector->executeDroneInsertQuery(updateQuery, holders)) {
        LOG(ERROR) << "update executed submissionList successfully.";
       
        return true;
    }
    else {
        LOG(ERROR) << "Error executing submissionList update query.";
        return false;
    }
}


bool DataProcessor::insertToRecordList(int submissionID, string snCode) {
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
    FlightRecord flightRecord = getSubmissionInfo(submissionID);
    LOG(ERROR) << "flightRecord submissionName: " << flightRecord.submissionName;
    LOG(ERROR) << "flightRecord planDate: " << flightRecord.planDate;
    LOG(ERROR) << "flightRecord excuteDate: " << flightRecord.excuteDate;
    LOG(ERROR) << "flightRecord missionID: " << flightRecord.missionID;

    vector<std::string> holders;
    std::string insertQuery = "INSERT INTO flightRecords(submissionID, submissionName, DepartureTime, LangdingTime,droneSncode ) VALUE(?,?,?,?,?) ";
    holders.push_back(to_string(submissionID));
    holders.push_back(flightRecord.excuteDate);
    holders.push_back(flightRecord.planDate);
    holders.push_back(flightRecord.missionID);
    holders.push_back(snCode);

    if (SQLiteConnector->executeDroneInsertQuery(insertQuery, holders)) {
        LOG(ERROR) << "Insertion successful";
        return true;
    }
    else {
        LOG(ERROR) << "Insertion failed";
        return false;
    }
}
  

FlightRecord DataProcessor::getSubmissionInfo(int submissionID) {
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
    vector<std::string> holders;
    std::string selectQuery = "SELECT submissionName,planDate,excuteDate,missionID from submissionList where submissionID = ?";
    std::vector<std::string> placeholders;
    FlightRecord flightRecord;
    placeholders.push_back(std::to_string(submissionID));
    std::vector<std::vector<std::string>> queryResults;
    if (SQLiteConnector->executeQueryWithPlaceholder(selectQuery, placeholders, queryResults)) {
        for (const auto& row : queryResults) {

            flightRecord.submissionName = row[0];
            flightRecord.planDate = row[1];
            flightRecord.excuteDate = row[2];
            flightRecord.missionID = row[3];
        }
    }
    return flightRecord;
}




vector<WaypointInformation> DataProcessor::getContinueWaypoints(int submissionID, std::vector<WaypointInformation> &completePoints){
	int completesz = completePoints.size();
	if (completesz < 1){
		return std::vector<WaypointInformation>();
	}
	std::vector<WaypointInformation> allwp = GetSubmissionWaypoints(submissionID);
	std::vector<WaypointInformation> continuewp;
	bool bfind = false;
	for (int i=0; i<allwp.size(); i++)
	{
		if (!bfind) {
			if (allwp[i].tower_id == completePoints[completesz - 1].tower_id) {
				bfind = true;
				for (int j = i; j < allwp.size(); j++) {
					if (allwp[j].necessary == true) {
						allwp[j].stc_shootPhoto = false;
						continuewp.push_back(allwp[j]);
						break;
					}
				}

				int curnum = 0;
				for (int k = completesz - 1; k > 0; k--) {
					if (completePoints[k].tower_id == allwp[i].tower_id) {
						curnum++;
					}
					else {
						break;
					}
				}
				i = i + curnum - 1;
			}
			else if (allwp[i].necessary == true) {
				allwp[i].stc_shootPhoto = false;
				continuewp.push_back(allwp[i]);
			}
		}
		else {
			continuewp.push_back(allwp[i]);
		}
	}
	return continuewp;
}

vector<WaypointInformation> DataProcessor::readRoute(int task_id, FileSystem* fileInterface)
{
	uint32_t timestamp = 0;
	string name;
   
	if (fileInterface->checkBreakTask(task_id, timestamp)) {
		name = to_string(task_id) + "_" + to_string(timestamp);
	}
	else {
		name = to_string(task_id);
	}
    return fileInterface->readDataFromFile(name);
}


bool DataProcessor::deleteDroneFromDB(string snCode) {
    LOG(ERROR) << " === deleteDroneFromDB === ";
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
    vector<std::string> placeholders;
    std::string query = "select submissionID FROM submissionList where droneID = (SELECT droneID from drone where snCode = ?);";
    placeholders.push_back(snCode);
    vector<vector<string>> queryResults;
    LOG(ERROR) << query ;
    if (SQLiteConnector->executeQueryWithPlaceholder(query, placeholders, queryResults)) {
        if (!queryResults.empty() && !queryResults[0].empty())
        {
            string submissionID = queryResults[0][0];
            LOG(ERROR) << "submissionID : " << submissionID;
            vector<std::string> holders;
            std::string updateQuery = "UPDATE submissionList SET droneID = NULL WHERE submissionID = ?";
            holders.push_back(submissionID);
            if (SQLiteConnector->executeDroneInsertQuery(updateQuery, holders)) {
                LOG(ERROR) << "update executed submissionList successfully.";
            }
            else {
                LOG(ERROR) << "Error executing submissionList update query.";
                return false;
            }
        }
        else {
            LOG(ERROR) << "submission unexist";
        }
        vector<std::string> snCodeVec;
        std::string updateQuery = "UPDATE drone SET whetherOnline = '0' WHERE snCode = ?";
        snCodeVec.push_back(snCode);
        if (SQLiteConnector->executeDroneInsertQuery(updateQuery, snCodeVec)) {
            LOG(ERROR) << "update executed submissionList successfully.";
        }
        else {
            LOG(ERROR) << "Error executing submissionList update query.";
            return false;
        }
     /*   std::string deleteQuery = "DELETE FROM drone WHERE snCode = ?";
        LOG(ERROR) << deleteQuery;
        snCodeVec.push_back(snCode);
        if (SQLiteConnector->executeDroneInsertQuery(deleteQuery, snCodeVec)) {
            LOG(ERROR) << "DELETE query executed successfully.";
            return true;
        }
        else {
            LOG(ERROR) << "Error executing DELETE query.";
            return false;
        }*/
    }
 
}


vector<WaypointInformation> DataProcessor::readFileType(int submissionID) {
    
    string filePath = getSubmissionFile(submissionID);
    vector<WaypointInformation> waypointList;
    filesystem::path path(filePath);
    string extension = path.extension().string();
    LOG(ERROR) << "extentsion : " << extension;

    if (extension == ".kml") {
        waypointList = processKMLFile(filePath, submissionID);
    }
    else if (extension == ".json") {
        waypointList = processJsonFile(filePath, submissionID);
    }
    LOG(ERROR) << "waypointList size  : " << waypointList.size();
    
    return waypointList;
}


vector<WaypointInformation> DataProcessor::processKMLFile(std::string KMLPath, int submissionID) {
    LOG(ERROR) << "=== ProcessKML ===";
    std::vector <WaypointInformation> pointvec;
    //string Path = "C:/Users/123/Desktop/origin_php/myphp/public/importRoute/";
    //string Path = IMPORT_ROUTE_FILE_LOCAL_PATH;
    string networkType = getNetWorkType();

    string Path = IMPORT_ROUTE_FILE_PATH;
    LOG(ERROR) << "KMLPath : " << KMLPath;
    string KMLfile = Path + KMLPath;
    LOG(ERROR) << "KMLfile : " << KMLfile;
    std::ifstream file(KMLfile);
    if (!file.is_open()) {
        LOG(ERROR) << "file open fail";
        return pointvec;
    }
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(KMLfile.c_str()) == tinyxml2::XML_SUCCESS) {
        tinyxml2::XMLElement* root = doc.RootElement();
        tinyxml2::XMLElement* document = root->FirstChildElement("Document");
        if (document) {
            LOG(ERROR) << "entry document";
            tinyxml2::XMLElement* folder = document->FirstChildElement("Folder");
            if (folder) {    
                LOG(ERROR) << "entry folder";
                int pointIndex = 0;
                for (tinyxml2::XMLElement* placemark = folder->FirstChildElement("Placemark"); placemark; placemark = placemark->NextSiblingElement("Placemark")) {

                    WaypointInformation waypoint;

                    std::vector<uint8_t> target_class_id;
                    target_class_id.push_back(55);
                    waypoint.target_class_id = target_class_id;
          


                    tinyxml2::XMLElement* coordinatesElement = placemark->FirstChildElement("Point")->FirstChildElement("coordinates");
                    if (coordinatesElement) {
                        const char* coordinatesText = coordinatesElement->GetText();
                        std::vector<std::string> list = splitString(coordinatesText, ',');
                        if (list.size() == 2) {
                            waypoint.stc_x = std::stod(list.at(0));
                            waypoint.stc_y = std::stod(list.at(1));
                            LOG(ERROR) << "waypoint.stc_x: " << std::fixed << std::setprecision(10) << waypoint.stc_x;
                            LOG(ERROR) << "waypoint.stc_y: " << std::fixed << std::setprecision(10) << waypoint.stc_y;
                        }
                    }



                    tinyxml2::XMLElement* txzftypeElement = placemark->FirstChildElement("txzftype");
                    if (txzftypeElement) {
                        const char* txzfType = txzftypeElement->GetText();
                        LOG(ERROR) << "txzfType: " << txzfType;
                        if (txzfType) {
                            if (strcmp(txzfType, "jj") == 0) {
                                std::vector<uint8_t> target_class_id_hdgd;
                                target_class_id_hdgd.push_back(3);
                                waypoint.target_class_id = target_class_id_hdgd;
                                LOG(ERROR) << "target_class_id_hdgd: " << (int)waypoint.target_class_id[0];
                            }
                            else if (strcmp(txzfType, "tj") == 0) {
                                std::vector<uint8_t> target_class_id_tj;
                                target_class_id_tj.push_back(0);//tj
                                waypoint.target_class_id = target_class_id_tj;
                                LOG(ERROR) << "target_class_id_tj: " << (int)waypoint.target_class_id[0];
                            }
                            else if (strcmp(txzfType, "bp") == 0) {
                                std::vector<uint8_t> target_class_id_bp;
                                target_class_id_bp.push_back(4); //bp
                                waypoint.target_class_id = target_class_id_bp;
                                LOG(ERROR) << "target_class_id_bp: " << (int)waypoint.target_class_id[0];
                            }
                            else if (strcmp(txzfType, "ts") == 0) {
                                std::vector<uint8_t> target_class_id_ts;
                                target_class_id_ts.push_back(1);//ts
                                waypoint.target_class_id = target_class_id_ts;
                                LOG(ERROR) << "target_class_id_ts: " << (int)waypoint.target_class_id[0];
                            }
                            else if (strcmp(txzfType, "jyz") == 0) {
                                std::vector<uint8_t> target_class_id_jyz;
                                target_class_id_jyz.push_back(2);//ts
                                waypoint.target_class_id = target_class_id_jyz;
                                LOG(ERROR) << "target_class_id_jyz: " << (int)waypoint.target_class_id[0];
                            }
                        }
                    }



                    tinyxml2::XMLElement* ellipsoidHeightElement = placemark->FirstChildElement("wpml:ellipsoidHeight");
                    if (coordinatesElement) {
                        const char* ellipsoidHeightText = ellipsoidHeightElement->GetText();
                        float ellipsoidHeigh = strtof(ellipsoidHeightText, nullptr);
                        if (ellipsoidHeigh) {
                            waypoint.stc_z = ellipsoidHeigh;
                            LOG(ERROR) << "waypoint.stc_z: " << std::fixed << std::setprecision(10) << waypoint.stc_z - 10;
                        }
                    }
                    tinyxml2::XMLElement* actionGroupElement = placemark->FirstChildElement("wpml:actionGroup");
                    if (actionGroupElement) {
                        tinyxml2::XMLElement* actionElement = actionGroupElement->FirstChildElement("wpml:action");
                        if (actionElement) {
                            tinyxml2::XMLElement* actionActuatorFuncParamElement = actionElement->FirstChildElement("wpml:actionActuatorFuncParam");
                            if (actionActuatorFuncParamElement) {
                                tinyxml2::XMLElement* focalLengthElement = actionActuatorFuncParamElement->FirstChildElement("wpml:focalLength");
                                if (focalLengthElement) {
                                    const char* focalLength = focalLengthElement->GetText();
                                    float zoom = strtof(focalLength, nullptr);
                                    waypoint.zoom_Magnification = zoom / 24 * 10;
                                    waypoint.stc_shootPhoto = 1;
                                    LOG(ERROR) << "waypoint.zoom_Magnification: " << waypoint.zoom_Magnification;
                                }

                                tinyxml2::XMLElement* gimbalYawRotateAngleElement = actionActuatorFuncParamElement->FirstChildElement("wpml:gimbalYawRotateAngle");
                                if (gimbalYawRotateAngleElement) {
                                    const char* gimbalYawRotateAngle = gimbalYawRotateAngleElement->GetText();
                                    float yaw = strtof(gimbalYawRotateAngle, nullptr);
                                    if (yaw) {
                                        waypoint.stc_yaw = yaw;
                                        LOG(ERROR) << " waypoint.stc_yaw: " << waypoint.stc_yaw;
                                    }
                                      
                                }

                                tinyxml2::XMLElement* gimbalPitchAngleElement = actionActuatorFuncParamElement->FirstChildElement("wpml:gimbalPitchRotateAngle");
                                if (gimbalPitchAngleElement) {
                                    const char* gimbalPitchAngleText = gimbalPitchAngleElement->GetText();
                                    waypoint.stc_gimbalPitch = strtof(gimbalPitchAngleText, nullptr);
                                    LOG(ERROR) << "waypoint.stc_gimbalPitch: " << waypoint.stc_gimbalPitch;
                                }
                            }
                        }
                    }

                  
                    waypoint.stc_index = pointIndex++;
                    LOG(ERROR) << "waypoint.stc_index: " << waypoint.stc_index;
                    waypoint.task_id = submissionID;
                    waypoint.necessary = 1;
                   
                    //if (networkType == "main") {
                    //    waypoint.waypointType = 1; //1 is main network, 4 is distribution network
                    //}
                    //else if (networkType == "distribution") {
                    //    waypoint.waypointType = 4;
                    //}
                    LOG(ERROR) << "waypoint.target_class_id: " <<(int)waypoint.target_class_id[0];

                    waypoint.waypointType = 1;
                    pointvec.push_back(waypoint);
                }
            }
        }
    }
    else {
        LOG(ERROR) << "Failed to load the KML file." ;
    }

    LOG(ERROR) <<"pointvec size : " << pointvec.size();
    return pointvec;
}

std::vector<std::string> DataProcessor::splitString(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream tokenStream(s);
    std::string token;
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

vector<WaypointInformation> DataProcessor::processJsonFile(std::string& filePath, int submissionID) {
    LOG(ERROR) << " ==== processJsonFile ====";
    std::vector <WaypointInformation> pointvec;
    //string Path = IMPORT_ROUTE_FILE_LOCAL_PATH;
    string Path = IMPORT_ROUTE_FILE_PATH;
    LOG(ERROR) << "jsonFile : " << filePath;
    string jsonFile = Path + filePath;
    LOG(ERROR) << "jsonFile : " << jsonFile;
    string networkType = getNetWorkType();
    std::ifstream file(jsonFile);
    if (!file.is_open()) {
        LOG(ERROR) << "Failed to open JSON file.";
        return pointvec;
    }
    nlohmann::json jsonData;

    file >> jsonData;
    file.close();

    std::string taskname = jsonData["taskname"];
    LOG(ERROR) << "Taskname: " << taskname;

    nlohmann::json points = jsonData["points"];
    LOG(ERROR) << "points size: " << points.size();
    int pointIndex = 0;
    for (const auto& point : points) {
        WaypointInformation waypoint;
        LOG(ERROR) << "Point Info:";
        double aircraftYaw = point["aircraftYaw"];
        waypoint.stc_yaw = aircraftYaw;
        double height = point["height"] - 10;
        waypoint.stc_z = height;
        double lat = point["lat"];
        waypoint.stc_y = lat;
        double lng = point["lng"];
        waypoint.stc_x = lng;
        LOG(ERROR) << "waypoint.stc_x: " << std::fixed << std::setprecision(10) << waypoint.stc_x;
        LOG(ERROR) << "waypoint.stc_y: " << std::fixed << std::setprecision(10) << waypoint.stc_y;
        LOG(ERROR) << "waypoint.stc_z: " << std::fixed << std::setprecision(10) << waypoint.stc_z;
        LOG(ERROR) << "waypoint.stc_yaw:  " << waypoint.stc_yaw;
    

        std::vector<uint8_t> target_class_id;
        target_class_id.push_back(55);
        waypoint.target_class_id = target_class_id;
        LOG(ERROR) << "target_class_id_nomal: " << waypoint.target_class_id[0];

        if (point.count("yawPitchArray") && !point["yawPitchArray"].empty()) {

            int keyID = point["keyID"];
            LOG(ERROR) << "keyID:" << keyID;
            nlohmann::json keyPoints = jsonData["powerline"]["keyPoint"];
            LOG(ERROR) << "keyPoints size: " << keyPoints.size();

            for (const auto& keyPoint : keyPoints) {
                int id = keyPoint["ID"];
                if (keyID == id) {
                    LOG(ERROR) << "id equel to keyID : " << id;
                    string hardwareType = keyPoint["hardwareType"];
                    string partName = keyPoint["partName"];
                    // string jj = "\xe6\x8c\x82\xe7\x82\xb9";//挂点
                    string jj = u8"挂点";
                    // string tj = "\xe5\x9c\xb0\xe7\xba\xbf";//地线
                    string tj = u8"地线";
                    // string bp = "\xe6\x9d\x86\xe5\x8f\xb7\xe7\x89\x8c";//杆号牌
                    string bp = u8"杆号牌";
                    // string ts = "\xe5\xa1\x94\xe5\x85\xa8\xe8\xb2\x8c";//塔全貌
                    string ts = u8"塔全貌";
                    string hdgd = u8"横担端挂点";
                    string dxgd = u8"导线端挂点";

                    if (partName.find(hdgd) != string::npos) {
                        std::vector<uint8_t> target_class_id_hdgd;
                        target_class_id_hdgd.push_back(3);
                        waypoint.target_class_id = target_class_id_hdgd;
                        LOG(ERROR) << "target_class_id_hdgd: " << (int)waypoint.target_class_id[0];
                    }
                    else if (partName.find(dxgd) != string::npos) {
                        std::vector<uint8_t> target_class_id_dxgd;
                        target_class_id_dxgd.push_back(3); //jj
                        waypoint.target_class_id = target_class_id_dxgd;
                        LOG(ERROR) << "target_class_id_dxgd: " << (int)waypoint.target_class_id[0];
                    }
                    else if (partName.find(tj) != string::npos) {
                        std::vector<uint8_t> target_class_id_tj;
                        target_class_id_tj.push_back(0);//tj
                        waypoint.target_class_id = target_class_id_tj;
                        LOG(ERROR) << "target_class_id_tj: " << (int)waypoint.target_class_id[0];
                    }
                    else if (partName.find(bp) != string::npos) {
                        std::vector<uint8_t> target_class_id_bp;
                        target_class_id_bp.push_back(4); //bp
                        waypoint.target_class_id = target_class_id_bp;
                        LOG(ERROR) << "target_class_id_bp: " << (int)waypoint.target_class_id[0];
                    }
                    else if (partName.find(ts) != string::npos) {
                        std::vector<uint8_t> target_class_id_ts;
                        target_class_id_ts.push_back(1);//ts
                        waypoint.target_class_id = target_class_id_ts;
                        LOG(ERROR) << "target_class_id_ts: " << (int)waypoint.target_class_id[0];
                    }
                }
            }
            double gimbalPitch = point["yawPitchArray"][0]["gimbalPitch"];
            waypoint.stc_gimbalPitch = gimbalPitch;
            std::string keyName = point["yawPitchArray"][0]["keyName"];
            //LOG(ERROR) << "KeyName: " << keyName;
            waypoint.stc_shootPhoto = 1;
            waypoint.zoom_Magnification = 20;

        }
        else {
            LOG(ERROR) << " don't need take photo ";
        }

        LOG(ERROR) << "waypoint.stc_gimbalPitch: " << waypoint.stc_gimbalPitch;
        waypoint.stc_index = pointIndex++;
        LOG(ERROR) << "waypoint.stc_index: " << waypoint.stc_index;
        LOG(ERROR) << "waypoint target id : " << (int)waypoint.target_class_id[0];
        waypoint.task_id = submissionID;
        waypoint.necessary = 1;
        //if (networkType == "main") {
        //    waypoint.waypointType = 1; //1 is main network, 4 is distribution network
        //}
        //else if (networkType == "distribution") {
        //    waypoint.waypointType = 4;
        //}

        waypoint.waypointType = 1;
        pointvec.push_back(waypoint);

    }
    LOG(ERROR) << "pointvec size : " << pointvec.size();
    return pointvec;
}

string DataProcessor::getSubmissionFile(int submissionID) {
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
    vector<std::string> holders;
    std::string selectQuery = "SELECT filePath from uploadRouteSubmissionList where submissionID = ?";
    std::vector<std::string> placeholders;
    string fileName;
    placeholders.push_back(std::to_string(submissionID));
    std::vector<std::vector<std::string>> queryResults;
    if (SQLiteConnector->executeQueryWithPlaceholder(selectQuery, placeholders, queryResults)) {
        for (const auto& row : queryResults) {
            fileName = row[0];
        }
    }
    LOG(ERROR) << "fileName : " << fileName;
    return fileName;
}


vector < WaypointInformation>  DataProcessor::addFristWayPoint(std::vector< WaypointInformation>& waypointVec, WaypointInformation current_point, int submissionID) {
    WaypointInformation point_start = waypointVec[0];
    int flightHeight = Query_submisson_flightHeight(submissionID);
    point_start.stc_z = flightHeight + current_point.stc_z;
    point_start.stc_shootPhoto = false;
    point_start.necessary = true;
    point_start.stc_gimbalPitch = -90;
    point_start.tower_id = -1;
    waypointVec.insert(waypointVec.begin(), point_start);
    auto pre_iter = waypointVec.begin();
    for (std::vector<WaypointInformation>::iterator next_iter = ++waypointVec.begin(); next_iter != waypointVec.end() && pre_iter != waypointVec.end(); pre_iter++, next_iter++) {
        float dis = CalcPointDis(*pre_iter, *next_iter);
        if (dis >= 50.0) {
            pre_iter->maxFlightSpeed = 10.0;
        }
        else {
            pre_iter->maxFlightSpeed = 5.0;
        }
        for (auto item : pre_iter->target_class_id)
        {
            if (item == 0 && pre_iter->stc_shootPhoto == 1)
            {
                pre_iter->maxFlightSpeed = 1.2;
                break;
            }
        }
    }
    return waypointVec;
}


string DataProcessor::getNetWorkType() {
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
    vector<std::string> holders;
    std::string selectQuery = "SELECT networkType from networkTypes where isValid = ?";
    std::vector<std::string> placeholders;
    string networkType;
    placeholders.push_back(std::to_string(1));
    std::vector<std::vector<std::string>> queryResults;
    if (SQLiteConnector->executeQueryWithPlaceholder(selectQuery, placeholders, queryResults)) {
        for (const auto& row : queryResults) {
            networkType = row[0];
        }
    }
    LOG(ERROR) << "networkType : " << networkType;
    return networkType;
}