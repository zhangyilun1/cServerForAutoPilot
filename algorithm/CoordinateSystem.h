#pragma once
class CoordinateSystem
{
public:
	CoordinateSystem();
	~CoordinateSystem();

	static CoordinateSystem* GetCoordinateSystem()
	{
		static CoordinateSystem cs;
		return &cs;
	}
	bool outOfChina(double lat, double lon);
	int wgs2bd(double lat, double lon, double* pLat, double* pLon); // WGS84=>BD09 地球坐标系=>百度坐标系 
	int gcj2bd(double lat, double lon, double* pLat, double* pLon);	// GCJ02=>BD09 火星坐标系=>百度坐标系 
	int bd2gcj(double lat, double lon, double* pLat, double* pLon);	// BD09=>GCJ02 百度坐标系=>火星坐标系 
	int wgs2gcj(double lat, double lon, double* pLat, double* pLon);// WGS84=>GCJ02 地球坐标系=>火星坐标系
	int gcj2wgs(double lat, double lon, double* pLat, double* pLon);// GCJ02=>WGS84 火星坐标系=>地球坐标系(粗略)
	int bd2wgs(double lat, double lon, double* pLat, double* pLon);	// BD09=>WGS84 百度坐标系=>地球坐标系(粗略)
	int gcj2wgs_Exactly(double lat, double lon, double* wgs_Lat, double* wgs_lon);// GCJ02=>WGS84 火星坐标系=>地球坐标系（精确）
	int bd2wgs_Exactly(double lat, double lon, double* pLat, double* pLon);// BD09=>WGS84 百度坐标系=>地球坐标系(精确)
	double *OffSet(double lat, double lon);	// 偏移量
	double transformLat(double x, double y);// 纬度偏移量
	double transformLon(double x, double y);// 经度偏移量

private:
	/*
	 pi: 圆周率。
	 a: 卫星椭球坐标投影到平面地图坐标系的投影因子。
	 ee: 椭球的偏心率。
	 x_pi: 圆周率转换量。
	*/
	double pi = 3.14159265358979324;
	double a = 6378245.0;
	double ee = 0.00669342162296594323;
	double x_pi = 3.14159265358979324 * 3000.0 / 180.0;
};

