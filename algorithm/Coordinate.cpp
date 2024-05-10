#include "Coordinate.h"

void CDatumPoint::setDatumPoint(double x, double y, double z)
{
	m_OriginX2 = x;
	m_OriginY2 = y;
	m_OriginZ2 = z;
}

void CDatumPoint::setLongitudeAndLatitude(double longitude, double latitude, double altitude)
{
	m_GPSX2 = longitude;
	m_GPSY2 = latitude;
	m_GPSZ2 = altitude;
}

void CDatumPoint::setOrigin(double x, double y, double z, bool isGPS)
{
	if (!isGPS)
	{
		m_OriginX1 = x;
		m_OriginY1 = y;
		m_OriginZ1 = z;
	}
	else
	{
		m_GPSX1 = x;
		m_GPSY1 = y;
		m_GPSZ1 = z;
	}

}

LongitudeAndLatitude CDatumPoint::DatumPointToLongitudeAndLatitude(Coordinate temp)
{
	LongitudeAndLatitude buffer;
	buffer.stc_latitude = temp.stc_y / 111192.4 + m_GPSY1;
	buffer.stc_longitude = temp.stc_x / (111192.4 * cos(m_GPSY1 / 180 * 3.1415926)) + m_GPSX1;
	buffer.stc_altitude = temp.stc_z + m_GPSZ1;
	return buffer;
}

Coordinate CDatumPoint::longitudeAndLatitudeToDatumPoint(LongitudeAndLatitude temp)
{
	Coordinate buffer;
	buffer.stc_y = (temp.stc_latitude - m_GPSY1) * 111192.4;
	buffer.stc_x = (temp.stc_longitude - m_GPSX1) * (111192.4 * cos(temp.stc_latitude / 180 * 3.1415926));
	buffer.stc_z = temp.stc_altitude - m_GPSZ1;
	return buffer;
}

//¾àÀë²î×ª¾­Î³¶È²î
LongitudeAndLatitude CDatumPoint::distance_in_meter_to_LongitudeAndLatitude_dev(Coordinate temp, double m_GPSY1)
{
	LongitudeAndLatitude buffer;
	buffer.stc_latitude = temp.stc_y / 111192.4;
	buffer.stc_longitude = temp.stc_x / (111192.4 * cos(m_GPSY1 / 180 * 3.1415926));
	buffer.stc_altitude = temp.stc_z;
	return buffer;
}
//¾­Î³¶È²î×ª¾àÀë²î
Coordinate CDatumPoint::LongitudeAndLatitude_dev_to_distance_in_meter(LongitudeAndLatitude temp, double latitude)
{
	Coordinate buffer;
	buffer.stc_y = (temp.stc_latitude) * 111192.4;
	buffer.stc_x = (temp.stc_longitude) * (111192.4 * cos(latitude / 180 * 3.1415926));
	buffer.stc_z = temp.stc_altitude;
	return buffer;
}