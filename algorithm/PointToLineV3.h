//#pragma once
#ifndef POINTLINE
#define POINTLINE
#include <vector>
#include "../common.h"
#include "Coordinate.h"
#include <cmath>
//using namespace std;

class PointToLineV3
{
	public:
		PointToLineV3();
		~PointToLineV3();
		//std::vector<RouteIndexInfo> pointToLine(RecPoint input);
		std::vector<WaypointInformation> pointToLine_OU(RecPoint input);
		std::vector<WaypointInformation> pointToLine_U(RecPoint input);
		std::vector<WaypointInformation> route_to_osdkroute_V3(CommissionType2Post tempcommissionType2Post);
		std::vector<WaypointInformation> pointToLine_minor(RecPoint input, uint32_t photoflag);
		//距离差转经纬度差
		LongitudeAndLatitude distance_in_meter_to_LongitudeAndLatitude_dev(Coordinate temp, double m_GPSY1);
		//经纬度差转距离差
		DatumPoint LongitudeAndLatitude_dev_to_distance_in_meter(LongitudeAndLatitude temp, double latitude);

	private:
		double ang2rad(double in);
		float angplus(float angle_1, float angle_2);
		int rad2ang(double in);
		std::vector<double> matrixrotate2d(std::vector<double> in_point, double theta);
		std::vector<double> get_unit_directon2d(double x1, double y1, double x2, double y2);
		std::vector<double> get_unit_directon3d(double x1, double y1, double z1, double x2, double y2, double z2);
		double get_distance2d(double x1, double y1, double x2, double y2);
		double get_distance3d(double x1, double y1, double z1, double x2, double y2, double z2);
		int get_angle_tonorth(double  x2, double  y2);



};
#endif // POINT
