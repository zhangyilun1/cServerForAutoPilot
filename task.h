#ifndef _TASKS_H_
#define _TASKS_H_

#include <iostream>
#include <string>
#include "common.h"

class Tasks {

public:

    Tasks();
    ~Tasks();

    // Getter and SUBMISSIONid
    std::string getSubmissionID() const;
    void setSubmissionID(const std::string& submissionID);

    std::string getMissionType() const;
    void setMissionType(const std::string& missionType);

    // Getter and Setter for rtk
    int getRtk() const;
    void setRtk(const int& rtk);

    // Getter and Setter for flightType
    int getFlightType() const;
    void setFlightType(const int& flight);

    // Getter and Setter for timestamp
    int getTimestamp() const;
    void setTimestamp(const int& timestamp);

    // Getter and Setter for record
    int getWhetherRecord() const;
    void setWhetherRecord(const int& record);

    // Getter and Setter for imgwidth
    int getImgwidth() const;
    void setImgwidth(const int& imgwidth);

    // Getter and Setter for imgheigth
    int getImgheigth() const;
    void setImgheigth(const int& imgheigth);

    int getReturnCode()const;
    void setReturnCode(const int& returnCode);

    Gps getReturnGPS()const;   
    void setReturnGPS(const Gps& returnGPS);

    const std::list<WaypointInformation>& getWaypointList() const;
    void setWaypointList(const std::list<WaypointInformation>& waypointList);

private:
    std::string submissionID;
    int rtk;
    int flight;
    int timestamp;
    int record;
    int imgwidth;
    int imgheigth;
    int returnCode;
    Gps returnGPS;
    std::string missionType;
    std::list<WaypointInformation> waypointList;
};

#endif