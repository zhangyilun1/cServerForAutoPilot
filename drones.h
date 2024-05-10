#ifndef _DRONES_H_
#define _DRONES_H_

#include <iostream>
#include <string>
#include "common.h"

class Drones {

public:

    Drones();
    ~Drones();

     std::string getSnCode();
    void setSnCode( std::string snCode);

     std::string getDroneType();
    void setDroneType( std::string droneType);

     std::string getMaxSpeed();
    void setMaxSpeed( std::string maxSpeed);

     std::string getSystemVersion();
    void setSystemVersion( std::string systemVersion);

     std::string getLen();
    void setLen( std::string len);

    HeartbeatInfo getHeartbeatInfo();
    void setSystemVersion(HeartbeatInfo infor);

private:
     std::string snCode;
     std::string droneType;
     std::string maxSpeed;
     std::string systemVersion;
    // std::string mode;
     std::string len;
    HeartbeatInfo infor;



};

#endif