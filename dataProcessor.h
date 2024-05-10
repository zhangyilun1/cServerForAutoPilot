#ifndef _DATA_PROCESSOR_H_
#define _DATA_PROCESSOR_H_

#include "drones.h"
#include "sqliteConnection.h"
#include "log1.h"
#include "session.h"
#include "common.h"
#include "task.h"
#include "file.h"
#include "md5.h"
#include "./algorithm/PointToLineV3.h"
#include <cereal/archives/binary.hpp>
#include <cereal/types/list.hpp>
#include <algorithm>
#include <cereal/types/string.hpp>
#include "config.h"
#include <nlohmann/json.hpp>
#include <locale>
#include <codecvt>
#include "tinyxml2.h"

using std::min;
class Session;
// std::map<std::string, Session*> droneSessionMap;

class DataProcessor {

public:
    DataProcessor();
    ~DataProcessor();
 
    void processReceivedData(const char* data, int length, Session* session);  
    void processDronesData(const uint8_t* data, int length, Session* session);
    //void processTask(const uint8_t* data, int length, SQLiteConnection* SQLiteConnector, Drones* drone, Session* session);
    void getTaskInfo(const uint8_t* data, int length, Session* session);
    void processUpgrade(const uint8_t* data, int length, Session* session);
    HeartbeatInfo getHeartbeatInfo(const uint8_t* data, int length, Drones* drone, Session* session);
    void getRecord(const uint8_t* data, int length, Session* session);
    void getWifiRecord(const uint8_t* data, int length, Session* session);
    void changeTowerGPT(const uint8_t* data, int length, Session* session);
    bool deleteDroneFromDB(std::string snCode);
    std::vector<std::vector<std::string>> querySystemVersionAndFilePath();
    int whetherRoolBack(ConfigFile* mConfig);
    bool str2Version(std::string str, int& ione, int& itwo, int& ithree);
    void sendFile(int clientSocket, const std::string& filePath, std::string md5Hash, int seekg);
private:
    void processSNCode(const uint8_t* data, int length, SQLiteConnection* SQLiteConnector, Drones* drone, Session* session);
    DroneData getDroneSNCode(const uint8_t* data, int length);
    std::vector<WaypointInformation> GetSubmissionWaypoints(int submissionID);
    std::vector<AirlineTower> Query_submission_towerinfo(int submissionID);
    std::vector<TowerList> QueryTowersByLineID(int lineid, bool asc);
    TowerShapeList GetTowerShapeInformationByID(int towershapeid);
    AirlineTower findNextTrueTower(std::vector<TowerList> &towers, int towerid, bool desc);
    std::list<int> getTowerList(int submissionID);
    std::vector<int> querySubmissionTower(int submissionID);
    LongitudeAndLatitude getOffSiteReturnAddr(int submission_id);
    int getReturnPolicy(int submission_id);
    int Query_submisson_missionID(int submission_id);
    int get_photoPosition(int towerid);
    int Query_lintTypeID(int towerid);
    TowerList getFirstTower(int submission_id);
    
    std::vector<WaypointInformation> generateMinorAirline(int submissionID, WaypointInformation current_point, int flighttasktype);
    LongitudeAndLatitude Get_LongitudeAndLatitude(int submission_id);
    int GetDoublePhotoBySubmissionID(int submission_id);
    double CalcPointDis(const WaypointInformation & l, const WaypointInformation & r);
    int Query_submisson_flightHeight(int submission_id);
    void Change_subtask_state(int submission_id);
    bool updateCompleteMissionStatus(int submissionID);
    //std::vector<std::vector<std::string>> querySystemVersionAndFilePath();
   // bool compareVersionnNumberSQL(const char* data);
    //char* fileProcess(std::string filePath);
    std::string findDestinationFilePathFromDB();
  /*  void sendFile(int clientSocket, const std::string& filePath, std::string md5Hash,int seekg);*/
   // void sendUpgradePacket(int clientSocket, const UpgradePacket& packet);
    bool versionNumberCompareWithDB(const char* snCode, const std::string dbVersionNumber);
   /* int whetherRoolBack(ConfigFile* mConfig);*/
    void sendFileBy10KB(char* buffer, int bufferSize, int clientSocket);
    std::vector<uint8_t> makeInfoPackage(int missionid, uint8_t answer);
    std::vector<WaypointInformation> getContinueWaypoints(int submissionID, std::vector<WaypointInformation> &completePoints);
    std::vector<WaypointInformation> readRoute(int task_id, FileSystem* fileInterface);
    std::vector<uint8_t> makePointAnswerPackage(int missionid, uint8_t answer);
    bool updateTowerGPS(std::list<int> towerList, std::list<Gps> newPoints);

    void logInfoSendToDrone(nlohmann::json jsonData);
    void processLog(const uint8_t* data, int length, Session* session);
    bool insertLogToDB(std::string filePath);
    bool recvLogFile( std::string filePath,std::string fileName, const char* data, uint32_t len);
    bool deleteFile(const std::string& filePath);
  /*  bool str2Version(std::string str, int& ione, int& itwo, int& ithree);*/
    bool insertToRecordList(int submissionID, std::string snCode);
    FlightRecord getSubmissionInfo(int submissionID);
    void killParentSendToDrone(nlohmann::json jsonData);
    void updateSendToDrone(nlohmann::json jsonData);
    void runningStatusDownloadSendToDrone(nlohmann::json jsonData);
    void photographTestSendToDrone(nlohmann::json jsonData);


    std::vector<WaypointInformation> readFileType(int submissionID);
    std::vector <WaypointInformation> addFristWayPoint(std::vector< WaypointInformation>& waypointVec, WaypointInformation current_point, int submissionID);
    std::string getSubmissionFile(int submissionID);
    std::vector<WaypointInformation> processKMLFile(std::string KMLPath, int submissionID);
    std::vector<WaypointInformation> processJsonFile(std::string& filePath, int submissionID);
    std::vector<std::string> splitString(const std::string& s, char delimiter);
    std::string getNetWorkType();


    bool storeWifiInfor2DB(std::string wifiName, std::string wifiPassword, std::string cpuID);
private:
    Drones* drone;
    Tasks* task;
    SQLiteConnection* SQLiteConnector;
    Session* session;

public:
    TaskInfo taskInfo;
    PointToLineV3 m_point_to_line_v3;
    CDatumPoint m_CDatumPoint;
};

#endif
