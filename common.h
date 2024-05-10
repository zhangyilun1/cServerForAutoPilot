#ifndef _COMMOM___H_
#define _COMMOM___H_

#include <string>
#include <vector>
#include <list>
#include "nlohmann/json.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/string.hpp"



#define TASKTYPENOTUPLOADED					0						//踩点任务
#define TASKTYPEROUTESGENERATE				1						//精细航线生成任务
#define TASKTYPEROUTESPATROL				2						//精细航线巡视任务
#define TASKTYPEDISTRIBUTIONDOT				3						//配网踩点任务
#define TASKTYPEDISTRIBUTIONROUTE			4						//配网巡视任务
#define TASKTYPEIMPORTROUTE					5						//导入航线任务


struct detection_bb
{
    uint32_t index = 0;
    int obj_class;
    int xmin;
    int ymin;
    int xmax;
    int ymax;
    int width;
    int height;
    int xcent;
    int ycent;
    float prob;
    float v_width = 0;
    float v_height = 0;
    float v_xcent = 0;
    float v_ycent = 0;

    float last_v_width = 0;
    float last_v_height = 0;
    float last_v_xcent = 0;
    float last_v_ycent = 0;

    std::vector<float> class_prob;
    int frame_count = 0;

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(class_prob,frame_count, height, index, last_v_height, last_v_width, last_v_xcent, last_v_ycent, obj_class, prob, v_height,
        v_width, v_xcent, v_ycent, width, xcent, xmax, xmin, ycent, ymax, ymin);
        // // Serialize the vector of floats using a loop
        // size_t class_prob_size = class_prob.size();
        // ar(class_prob_size); // Serialize the size of the vector
        // for (float &prob : class_prob)
        // {
        //     ar(prob); // Serialize each float value
        // }
    }
    
};
#pragma pack(push, 1)
struct Gps
{
    double stc_lat; // 纬度
    double stc_lon; // 经度
    double stc_alt; // 绝对海拔

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(stc_lon, stc_lat, stc_alt);
    }
};

struct WaypointInformation
{
    double stc_x = 0;          // lon//degree
    double stc_y = 0;          // lat//degree
    float stc_z = 0;           // alt//m
    uint16_t stc_index = 0;    // 路点序号
    float stc_yaw = 0;         // 云台yaw角度 [-180 180]
    float stc_gimbalPitch = 0; // 云台pitch角度 [-90 0]
    bool stc_shootPhoto = 0;   // 是否为拍照点，1拍照，0不拍
    int camera_type = 0;       // 0-->bianjiao    1-->guangjiao    2-->vedio mode    4-->shotbothside
    int ISO = 0;               //
    std::vector<uint8_t> target_class_id;
    int Number_of_layers_of_current_waypoint = 999; // 当前路点层数 //return point --> 999
    std::string Photo_time;                         // 拍照时间
    uint32_t task_id = 0;                           // 任务ID
    uint32_t tower_id = 0;                          // 杆塔id
     std::vector<Gps> Datum_point;                   // 三个基准点的坐标
    float zoom_Magnification = 0;                   // zoom
    int expo_comp = 0;                              // 暴光 compensation
    int shutter_speed = 0;                          // shutter speed
    bool necessary = 0;                             // safe pass, 0-->not necessary,   1-->necessary
    int orient_to_tower = 0;                        // 0-->tower center     ,1-->right up,  2-->left up,   3-->left down,    4-->right down, 9-->other
    detection_bb detection_box;                     //
    float maxFlightSpeed = 0;                       // max speed to next point
    int waypointType = 0;                           // mission type
    int back_Type = 0;                              // back type  0-->no safe back     1-->direct back
    int CONFIG_NO = 4;                              // 1--->ZHU,4--->PEI
    float POI_X = 0;
    float POI_Y = 0;
    float POI_Z = 0;
    float gimbal_roll = 0;
    int Reserve1 = 0;
    int Reserve2 = 0;

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(stc_x, stc_y, stc_z, stc_index, stc_yaw, stc_gimbalPitch, stc_shootPhoto, camera_type,
           ISO, target_class_id, Number_of_layers_of_current_waypoint, Photo_time, task_id, tower_id, Datum_point,
           zoom_Magnification, expo_comp, shutter_speed, necessary, orient_to_tower, detection_box, maxFlightSpeed, waypointType, back_Type, CONFIG_NO, POI_X, POI_Y, POI_Z, gimbal_roll, Reserve1, Reserve2);
    }
};
#pragma pack(pop)

struct DatumPoint {
	double stc_x;
	double stc_y;
	double stc_z;
};

struct TowerListTrans {
    std::list<int> towerList;
    int bRTK; 
    int submissionID;
};

struct RecPoint {
	std::vector<DatumPoint> rec_pt = { DatumPoint(),DatumPoint(), DatumPoint() };  //3个基准点，第一个是塔正中心上方10米，第二个是塔右最高绝缘子，第三个是塔左最低绝缘子
	DatumPoint prev_pt;                                                           //上座塔第一个基准点
	DatumPoint next_pt;                                                           //下座塔第一个基准点
	int layer_num = 0;                                                            //绝缘子层数
	double yaw_rotate_angle_selected = 15;                                           //侧扫参数，默认30度
	double longlook_whole_tower_distance = 60;                                    //第一个点距离杆塔中心距离
	double left_angle = 30;															//左夹角度数   配电用作侧拍角度
	double right_angle = 30;														//配电用作正拍角度
	double height = 50;																//塔高 配电用作拍摄高度
	int ChannelInspection = 0;
	int zoom = 1;																	//放大倍率
};

struct RouteIndexInfo {
	double stc_x;
	double stc_y;
	double stc_z;
	int	stc_index;									//路点序号
	double	stc_yaw;								//云台yaw角度 [-180 180]
	double	stc_gimbalPitch;						//云台pitch角度 [-90 0]
	int	stc_shootPhoto;								//是否为拍照点，1拍照，0不拍
	double stc_opticalZoomFocalLength;				//光学变焦
	std::vector<int> target_class_id;
};

struct AirlineTower {
	int stc_towerId = 0;								//杆塔id
	int stc_insulator_layer = 0;						//绝缘子层数
	double stc_leftAngle = 0;
	double stc_rightAngle = 0;
	double stc_offCenterAngle = 0;					//偏心角度
	double stc_height = 0;
	int stc_ChannelInspection = -1;
	double stc_x;
	double stc_y;
	double stc_z;
	int zoom = 1;
	int pass = 0;							//0 正常塔 1 路过塔 只过中心点，不拍照
	int lineID = -1;
	int towerNumber = 0;
	int mode = 0;
	int height = 0;					//飞向下一座塔的提升高度 直飞就是0
	int towerShapeID = -1;
	int towerVirtual = 0;			//正常塔 0 虚拟塔 1 定位塔 2（虚拟塔在任务最后和任务最前，或最前塔的前一塔和最后塔的后一塔是虚拟塔）
    int localClearPhoto = 0;
    int channelPhotography = 0;
	std::string stc_path;								//数据库中基准点数据存储路径
};

struct TowerList {
	int towerID; 									//杆塔ID
	int lineID; 									//所属线路ID
	int basicID;									//基准点ID
	int AccurateLineID;								//精细航线ID
	int towerShapeID;								//杆塔形状ID
	int insulatorNum;								//绝缘子数量  配网暂用作放大倍数*10
	double tower_length;							//塔长
	double tower_width;								//塔宽
	double tower_height;							//塔高
	double longitude;								//经度
	double latitude;								//纬度
	double altitude;								//海拔
	 std::string createdTime;							//创建日期
	 std::string comment;								//备注
	 std::string towerName;								//杆塔名称
	 std::string towerShapeName;							//杆塔形状名称  现在改为航线类型
	 std::string towerType;								//杆塔类型名称
	 std::string lineName; 								//所属线路名称
	int createManID;								//创建人ID
	int tower_number;								//杆塔序号
	double init_longitude = -1;						//初始经度
	double init_latitude = -1;						//初始纬度
	double init_altitude = -1;						//初始海拔
	int basicType = -1;								//基准点类型，0为GPS，1为RTK
	int photoPosition = 0;							//拍照点：前后左右中
	int mode = 0;									//前往下一塔的飞行方式 
	//低2位为 00 直飞 01 提升高度 10 按线路飞 ；倒数第三位 0  顺序 1 倒序  ；倒数第四位 相邻 0 不相邻 1；倒数第五位 0 同线路 1 不同线路
	int height = 0;									//mode为提升高度 此值有效
};

struct TowerShapeList {
	int tower_shape_id;								//杆塔形状ID
	int insulator_layer; 							//绝缘子层数
	int initialroute_id;							//初始航线ID
	int lineTypeID;									//航线类型 对应现在的杆塔类型
	int ChannelInspection;							//是否通道巡检
	double tower_length;							//长
	double tower_width;								//宽
	double tower_height;							//高
	double towerTypeLeftAngle;						//左侧夹角度数
	double towerTypeRightAngle;						//右侧夹角度数
	double towerTypeOffCenterAngle;					//偏心角度
	 std::string created_time;							//创建日期
	 std::string comment;								//备注
	 std::string tower_shape_name; 						//杆塔形状名称
	 std::string lineTypeName;							//航线类型名称
	int createManID;								//创建人ID
};

struct BasicResult {
	 std::string stc_routeId;
	std::vector<Gps> stc_vecGps;
};


struct FlightRecord {
    std::string submissionName;
    std::string planDate;
    std::string excuteDate;
    std::string missionID;
};





struct BasicResultGet {
	bool stc_isSuccess;								//是否成功
	int stc_routeType;								//任务类型
	BasicResult stc_basicResult;
	int stc_errCode;								//异常编码
	 std::string stc_errMsg;								//异常信息

	 std::string stc_createDate;							//创建日期
};




#pragma pack(push, 1)
struct DroneData
{
     std::string snCode;
     std::string version;
     float maxSpeed;
     std::string model;
     std::string lens;
     //std::string wifiName;
     //std::string wifiPassword;
    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(snCode);
        ar(version);
        ar(maxSpeed);
        ar(model);
        ar(lens);
        //ar(wifiName);
        //ar(wifiPassword);

    }
};
#pragma pack(pop)

// struct Record {
//     string missionID;
//     string flight;
//     float timestamp;
//     string model;
//     string lens;
// } 

#pragma pack(push, 1)
struct HeartbeatInfo
{
    double longitude;
    double latitude;
    double altitude;
    int missionID;
    uint8_t status;
    uint32_t index;
    uint32_t count;
    bool isInitialized = false; 
    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(longitude);
        ar(latitude);
        ar(altitude);
        ar(missionID);
        ar(status);
        ar(index);
        ar(count);
    }
};
#pragma pack(pop)
#pragma pack(push, 1)
struct DroneINFO
{

     std::string snCode;
     std::string droneType;
     std::string lensType;
     float maxSpeed;
     std::string systemVersion;

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(snCode);
        ar(systemVersion);
        ar(maxSpeed);
        ar(droneType);
        ar(lensType);
       
       
    }
};
#pragma pack(pop)
#pragma pack(push, 1)
struct TaskInfo
{
    uint32_t taskID;
    uint8_t rtk;
    uint8_t flight;
    uint32_t timestamp;
    uint8_t record;
    uint32_t imgwidth;
    uint32_t imgheight;
    uint8_t returnCode;
    Gps Gps;
    std::list<WaypointInformation> waypoints;

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(rtk);
        ar(flight);
        ar(timestamp);
        ar(record);
        ar(imgwidth);
        ar(imgheight);
        ar(returnCode);
        ar(Gps);
        //ar(waypoints);
    //     size_t waypoints_size = waypoints.size();
    //     ar(waypoints_size); // Serialize the size of the vector
    //     for (WaypointInformation &waypoint : waypoints)
    //     {
    //         ar(waypoint); // Serialize each float value
    //     }
    }
};
#pragma pack(pop)
//#pragma pack(push, 1)
//struct LogInfo {
//    uint8_t answer;
//    char
//};
//#pragma pack(pop)


struct InspectionPoint {
	Gps	stc_gps;									//截止路点GPS
	int	stc_index;								//路点序号
	double	stc_yaw;								//云台yaw角度 [-180 180]
	double	stc_gimbalPitch;						//云台pitch角度 [-90 0]
	int	stc_shootPhoto;								//是否为拍照点，1拍照，0不拍
	double stc_opticalZoomFocalLength;				//光学变焦
	std::vector<int> target_class_id;
};

struct CommissionType2Post {
	std::string stc_routeID;							//杆塔ID
	int stc_routeType;								//任务类型
	Gps stc_startPoint;								//起始点
	Gps stc_endPoint;								//结束点
	std::vector<InspectionPoint> stc_fplist;		//路点信息
};

struct LongitudeAndLatitude {
	double stc_longitude;
	double stc_latitude;
	double stc_altitude;
};

struct Coordinate {
	double stc_x;
	double stc_y;
	double stc_z;
};

struct UpgradePacket{
   char filePath[256];
   int fileSize;
   char md5[32];
};



#endif



