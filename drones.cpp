#include "drones.h"
using namespace std;
Drones::Drones()
{
}

Drones::~Drones()
{
}

string Drones::getSnCode()
{
    return this->snCode;
}

void Drones::setSnCode(string snCode)
{
    this->snCode = snCode;
}

string Drones::getDroneType()
{
    return this->droneType;
}

void Drones::setDroneType(string droneType)
{
    this->droneType = droneType;
}

string Drones::getMaxSpeed()
{
    return this->maxSpeed;
}

void Drones::setMaxSpeed(string maxSpeed)
{
    this->maxSpeed = maxSpeed;
}

string Drones::getSystemVersion()
{
    return this->systemVersion;
}

void Drones::setSystemVersion(string systemVersion)
{
    this->systemVersion = systemVersion;
}


string Drones::getLen()
{
    return this->len;
}

void Drones::setLen(string len)
{
    this->len = len;
}


HeartbeatInfo Drones::getHeartbeatInfo(){
    return this->infor;
}

void Drones::setSystemVersion(HeartbeatInfo infor){
    this->infor = infor;

}
