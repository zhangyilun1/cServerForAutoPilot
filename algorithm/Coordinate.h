#pragma once
#include <math.h>
#include "../common.h"


class CDatumPoint
{
public:

	void setDatumPoint(double x = 0.0, double y = 0.0, double z = 0.0);


	void setLongitudeAndLatitude(double longitude , double latitude, double altitude);


	void setOrigin(double x = 0.0, double y = 0.0, double z = 0.0, bool isGPS = false);			


	LongitudeAndLatitude DatumPointToLongitudeAndLatitude(Coordinate temp);


	Coordinate longitudeAndLatitudeToDatumPoint(LongitudeAndLatitude temp);

	LongitudeAndLatitude distance_in_meter_to_LongitudeAndLatitude_dev(Coordinate temp, double m_GPSY1);


	Coordinate LongitudeAndLatitude_dev_to_distance_in_meter(LongitudeAndLatitude temp, double latitude);

private:
	double m_OriginX1;				
	double m_OriginY1;
	double m_OriginZ1;

	double m_GPSX1;					
	double m_GPSY1;					
	double m_GPSZ1;					

	double m_OriginX2;				
	double m_OriginY2;
	double m_OriginZ2;

	double m_GPSX2;					
	double m_GPSY2;					
	double m_GPSZ2;					
};

