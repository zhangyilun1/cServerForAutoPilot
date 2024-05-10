#include "task.h"
using namespace std;
Tasks::Tasks(){

}

Tasks::~Tasks(){

}

string Tasks::getSubmissionID() const {
    return submissionID;
}

void Tasks::setSubmissionID(const string& submissionID) {
    this->submissionID = submissionID;
}

int Tasks::getRtk() const {
    return rtk;
}

void Tasks::setRtk(const int& rtk) {
    this->rtk = rtk;
}

int Tasks::getFlightType() const {
    return flight;
}

void Tasks::setFlightType(const int& flight) {
    this->flight = flight;
}

int Tasks::getTimestamp() const {
    return timestamp;
}

void Tasks::setTimestamp(const int& timestamp) {
    this->timestamp = timestamp;
}

int Tasks::getWhetherRecord() const {
    return record;
}

void Tasks::setWhetherRecord(const int& record) {
    this->record = record;
}

int Tasks::getImgwidth() const {
    return imgwidth;
}

void Tasks::setImgwidth(const int& imgwidth) {
    this->imgwidth = imgwidth;
}

int Tasks::getImgheigth() const {
    return imgheigth;
}

void Tasks::setImgheigth(const int& imgheigth) {
    this->imgheigth = imgheigth;
}



int Tasks::getReturnCode() const {
    return returnCode;
}

void Tasks::setReturnCode(const int& returnCode) {
    this->returnCode = returnCode;
}


Gps Tasks::getReturnGPS() const {
    return returnGPS;
}

void Tasks::setReturnGPS(const Gps& returnGPS) {
    this->returnGPS = returnGPS;
}



const std::list<WaypointInformation>& Tasks::getWaypointList() const {
    return waypointList;
}

void Tasks::setWaypointList(const std::list<WaypointInformation>& waypointList) {
    this->waypointList = waypointList;
}


string Tasks::getMissionType() const {
    return missionType;
}

void Tasks::setMissionType(const string& missionType) {
    this->missionType = missionType;
}
