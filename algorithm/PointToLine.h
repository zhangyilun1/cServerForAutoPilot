#pragma once
#include <vector>
#include "../common.h"
#include "Coordinate.h"
class CPointToLine
{
public:
	CPointToLine();
	~CPointToLine();
	//std::vector<RouteIndexInfo> pointToLine(RecPoint input);
	std::vector<RouteIndexInfo> pointToLine_U_2(RecPoint input);
	std::vector<RouteIndexInfo> pointToLine_OU_1(RecPoint input);
	std::vector<RouteIndexInfo> pointToLine_U_S2(RecPoint input);

	std::vector<WaypointInformation> route_to_osdkroute(CommissionType2Post tempcommissionType2Post);
};

