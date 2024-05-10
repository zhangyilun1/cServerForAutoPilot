#ifndef _JSONDATA_H_
#define _JSONDATA_H_

#include <iostream>
#include "common.h"
#include "log1.h"
#include <nlohmann/json.hpp>



class JsonData
{

public:
    JsonData(/* args */);
    ~JsonData();
    bool waypoint2JSONSplitByTowerID(const  std::vector<WaypointInformation>& vec, std::string& wholejson,
        std::vector<int>& toweridvec, std::vector< std::string>& towerstrvec);

    std::vector<WaypointInformation> Converting_the_aircraft_waypoint_structure_into_a_structure(const std::string& jsonStr);


};


#endif