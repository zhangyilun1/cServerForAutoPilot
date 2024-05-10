#include "server.h"
#include "sqliteConnection.h"
#include "config.h"
#include "log.h"
#include "memoryTrack.h"
#include <functional>
using namespace std;
//#include "/opt/homebrew/Cellar/nlohmann-json/3.11.2/include/nlohmann/json.hpp" 

 

#define DRONE_SERVER_PORT "DRONE_SERVER_PORT"
#define PHP_SERVER_PORT "PHP_SERVER_PORT"
#define UPGRADE_SERVER_PORT "UPGRADE_SERVER_PORT"
#define SQLITE_PATH "SQLITE_PATH"
#define DB_PATH_LOCAL  "C:/Users/123/Desktop/origin_php/myphp/public/feixing.db"
#define DB_PATH "C:/Users/Administrator/Desktop/newversion/myphp/myphp/public/feixing.db"
// std::mutex sessionMutex;



// void processReceivedData(const char* data, int length) {
//     // Convert the received data to a string for processing
//     std::string receivedData(data, length);
//     // Print the received data
//     std::cout << "Received data: " << receivedData << std::endl;
//     try {
//         nlohmann::json jsonData = nlohmann::json::parse(receivedData);
        
//         // Now you can access JSON data and perform operations
//         if (jsonData.contains("controller")) {
//             std::string value = jsonData["controller"];
//             std::cout << "Value of key: " << value << std::endl;
//             if(!value.empty())
//             {
//                 if(value == "get_drone")
//                 {
//                     //TRACE("value : " + value);
//                     // SQLiteConnector.executeQuery("select * from drone ;");
//                 }
//             }

//         }
//     } catch (const std::exception& e) {
//         // JSON parsing failed
//         std::cerr << "Error parsing JSON: " << e.what() << std::endl;
//     }
// }

// void start(Session* session) {

//     //std::unique_lock<std::mutex> lock(sessionMutex);
//     //TRACE("start");
//     char buff[1024];
//     int bytesRead = session->read(buff, sizeof(buff));
//     pthread_t tid = pthread_self();
//     std::stringstream ss;
//     ss << tid;
//     //TRACE("bytesRead : " + to_string(bytesRead));
//     if (bytesRead > 0 && !(strncmp(buff, "* PING", 6) == 0)) {
//         //TRACE("Received data from client : " + std::string(buff) + " tid = " + ss.str());
//         processReceivedData(buff, bytesRead);
//         session->write("success !!!");

//     }
//     else 
//     {
//         session->write("fail !!!");
//         //TRACE("fail !! ");  
//     }
//     memset(buff, 0, sizeof(buff));

// }


//vector<thread>& sessionThreads
void handleClient(SessionManager* m_sessionManager) {
    // 接收和发送数据的缓冲区
    //std::unique_lock<std::mutex> lock(sessionMutex);
    //TRACE( "sission map size in handleClient: " + to_string(m_sessionManager->CheckSessionMapSize()));
    if(m_sessionManager->CheckSessionMapSize() == 0)
    {
        //TRACE( "sission map is null ");
    }
    else if(m_sessionManager->CheckSessionMapSize() > 0)
    {
        //TRACE( "sission map size : " + to_string(m_sessionManager->CheckSessionMapSize()));
        // for(auto sessionPair : m_sessionManager->getSessionMap())
        // {
        //     Session* session = sessionPair.second;
        //     sessionThreads.emplace_back(start, session);
        // }
    }
}   







int main(){
    initLog();
    LOG(ERROR) << "begin"  ;
    //SessionManager *m_sessionManager = SessionManager::GetInstance();
    // read config content  
    // ConfigFile* mConfig = new ConfigFile("./config.txt");
    int dronePort = 32321;
    int phpServerPort = 2345;
    int upgradeServerPort = 32330;
    int pictureAndLogPort = 32322;
    //string SQLitePath = DB_PATH_LOCAL;
    string SQLitePath = DB_PATH;
   
    // mConfig->Read(SQLITE_PATH, SQLitePath);
    // int dronePort = mConfig->Read(DRONE_SERVER_PORT, dronePort);
    // int phpServerPort = mConfig->Read(PHP_SERVER_PORT, phpServerPort);
    // int upgradeServerPort = mConfig->Read(UPGRADE_SERVER_PORT, upgradeServerPort);
    // string SQLitePath = mConfig->Read(SQLITE_PATH, SQLitePath);
     LOG(ERROR) << "drone port : " << dronePort;
     LOG(ERROR) << "phpServer port : " << phpServerPort;
     LOG(ERROR) << "upgradeServer port : " << upgradeServerPort;
     LOG(ERROR) << "pictureAndLog port : " << pictureAndLogPort;


    // create sqlite connect
    SQLiteConnection& SQLiteConnector = SQLiteConnection::getInstance();
    cout << "get SQLiteConnector "<< SQLitePath << endl;
    bool connection = SQLiteConnector.open(SQLitePath);
    // TRACE( " connection is  : " +  to_string(connection));

    // port: 32321 begin listen
    // Server* serverForDrone  = new Server(dronePort);
    std::shared_ptr<Server> serverForDrone = std::make_shared<Server>(dronePort);
    thread th1(&Server::startWithListen, serverForDrone.get()); 
    th1.detach();
    //th1.join();

    //port: 2345 begin listen

    //std::shared_ptr<Server> serverForPHP = std::make_shared<Server>(phpServerPort, PhpHandler);
    std::shared_ptr<Server> serverForPHP = std::make_shared<Server>(phpServerPort);
    thread th2(&Server::startWithListen, serverForPHP.get());
    th2.detach();


    //port: 32330 begin listen
    //Server* serverForUpgrade = new Server(upgradeServerPort);
    std::shared_ptr<Server> serverForUpgrade = std::make_shared<Server>(upgradeServerPort);
    thread th3(&Server::startWithListen, serverForUpgrade.get());
    th3.detach();
    //th3.join();


    //port: 32322 begin listen
    //Server* pictureAndLog = new Server(pictureAndLogPort);
    std::shared_ptr<Server> pictureAndLog = std::make_shared<Server>(pictureAndLogPort);
    thread th4(&Server::startWithListen, pictureAndLog.get());
    th4.detach();
    //th4.join();

    while(true)
    {    
        Sleep(10000);
    }
    
    //delete mConfig;

    //SQLiteConnector.close();
    //delete m_sessionManager;
    //delete serverForDrone;
    //delete serverForPHP;
    //delete serverForUpgrade;
    //delete pictureAndLog;
}

