#include "PointToLineV3.h"
#include "../log.h"
using namespace std;

PointToLineV3::PointToLineV3()
{

}

PointToLineV3::~PointToLineV3()
{
}

double PointToLineV3::ang2rad(double in) {
	double radius = in / 180 * 3.1415926;
	return radius;
}

float PointToLineV3::angplus(float angle_1, float angle_2) {
	float angle_out = angle_1 + angle_2;
	if (angle_out > 180) {
		angle_out = angle_out - 360;
	}
	if (angle_out < -180) {
		angle_out = angle_out + 360;
	}
	return angle_out;
}
int PointToLineV3::rad2ang(double in) {
	double degree = in / 3.1415926 * 180;
	return degree;
}

vector<double> PointToLineV3::matrixrotate2d(vector<double> in_point, double theta)
{
	double x = in_point[0];
	double y = in_point[1];
	double angle_rad = ang2rad(theta);
	double new_x = x * cos(angle_rad) - y * sin(angle_rad);
	double new_y = x * sin(angle_rad) + y * cos(angle_rad);
	vector<double> result;
	result.push_back(new_x);
	result.push_back(new_y);
	return result;
}

vector<double> PointToLineV3::get_unit_directon2d(double x1, double y1, double x2, double y2)
{
	double dx = x2 - x1;
	double dy = y2 - y1;
	double square = dx * dx + dy * dy;
	double dis_of_two = sqrt(square);
	double new_x = dx / dis_of_two;
	double new_y = dy / dis_of_two;
	vector<double> result;
	result.push_back(new_x);
	result.push_back(new_y);
	return result;
}

vector<double> PointToLineV3::get_unit_directon3d(double x1, double y1, double z1, double x2, double y2, double z2)
{
	double dx = x2 - x1;
	double dy = y2 - y1;
	double dz = z2 - z1;
	double square = dx * dx + dy * dy + dz * dz;
	double dis_of_two = sqrt(square);
	double new_x = dx / dis_of_two;
	double new_y = dy / dis_of_two;
	double new_z = dz / dis_of_two;
	vector<double> result;
	result.push_back(new_x);
	result.push_back(new_y);
	result.push_back(new_z);
	return result;
}
double PointToLineV3::get_distance2d(double x1, double y1, double x2, double y2)
{
	double dx = x2 - x1;
	double dy = y2 - y1;
	double square = dx * dx + dy * dy;
	double dis_of_two = sqrt(square);
	return dis_of_two;
}
double PointToLineV3::get_distance3d(double x1, double y1, double z1, double x2, double y2, double z2)
{
	double dx = x2 - x1;
	double dy = y2 - y1;
	double dz = z2 - z1;
	double square = dx * dx + dy * dy + dz * dz;
	double dis_of_two = sqrt(square);
	return dis_of_two;
}
int PointToLineV3::get_angle_tonorth(double  x2, double  y2)
{
	double z1 = 0;
	double z2 = 0;
	double x1 = 0;
	double y1 = 1;
	if (x1 == 0 && y1 == 0 && z1 == 0) return 0;
	if (x2 == 0 && y2 == 0 && z2 == 0) return 0;
	double a_dot_b = x1 * x2 + y1 * y2 + z1 * z2;
	double a_lenth = sqrt(x1 * x1 + y1 * y1 + z1 * z1);
	double b_lenth = sqrt(x2 * x2 + y2 * y2 + z2 * z2);
	double cos_angle = a_dot_b / a_lenth / b_lenth;
	double angle_rad = acos(cos_angle);
	int angle_deg = rad2ang(angle_rad);
	if (x2 < 0) {
		angle_deg = -angle_deg;
	}
	return angle_deg;
}

/////////////////////////////U型精简两次
std::vector<WaypointInformation> PointToLineV3::pointToLine_U(RecPoint input)
{
	double safe_height_increament = 10;
	vector<WaypointInformation> line_out;
	line_out.clear();
	DatumPoint initial_line_up_point = input.rec_pt[0];
	DatumPoint initial_line_right_point = input.rec_pt[1];
	DatumPoint initial_line_left_point = input.rec_pt[2];
	DatumPoint initial_line_prev_point = input.prev_pt;
	DatumPoint initial_line_next_point = input.next_pt;
	double tower_center_height = (initial_line_right_point.stc_z + initial_line_left_point.stc_z) / 2;
	double left_yaw_angle = input.left_angle;
	double right_yaw_angle = input.right_angle;
	double longlook_whole_tower_distance = input.height;
	int ChannelInspection = input.ChannelInspection;
	float peer_2_yaw_center = input.yaw_rotate_angle_selected;
	int layer_number = input.layer_num;

	double dis_right = get_distance2d(initial_line_right_point.stc_x, initial_line_right_point.stc_y, initial_line_up_point.stc_x, initial_line_up_point.stc_y);
	double dis_left = get_distance2d(initial_line_left_point.stc_x, initial_line_left_point.stc_y, initial_line_up_point.stc_x, initial_line_up_point.stc_y);
	vector<double> unit_rightpoint_dirction = get_unit_directon2d(initial_line_right_point.stc_x, initial_line_right_point.stc_y, initial_line_up_point.stc_x, initial_line_up_point.stc_y);
	vector<double> unit_leftpoint_dirction = get_unit_directon2d(initial_line_left_point.stc_x, initial_line_left_point.stc_y, initial_line_up_point.stc_x, initial_line_up_point.stc_y);
	vector<double> d_right_forward = matrixrotate2d(unit_rightpoint_dirction, -right_yaw_angle);
	vector<double> d_right_backward = matrixrotate2d(unit_rightpoint_dirction, right_yaw_angle);
	vector<double> d_right_prev_perpendicular = matrixrotate2d(unit_rightpoint_dirction, 90);
	vector<double> d_left_forward = matrixrotate2d(unit_leftpoint_dirction, left_yaw_angle);
	vector<double> d_left_backward = matrixrotate2d(unit_leftpoint_dirction, -left_yaw_angle);
	vector<double> d_left_next_perpendicular = matrixrotate2d(unit_leftpoint_dirction, 90);

	d_right_forward[0] = -d_right_forward[0] * (dis_right / cos(ang2rad(right_yaw_angle)));
	d_right_backward[0] = -d_right_backward[0] * (dis_right / cos(ang2rad(right_yaw_angle)));
	d_left_forward[0] = -d_left_forward[0] * (dis_left/ cos(ang2rad(left_yaw_angle)));
	d_left_backward[0] = -d_left_backward[0] * (dis_left / cos(ang2rad(left_yaw_angle)));
	d_right_forward[1] = -d_right_forward[1] * (dis_right / cos(ang2rad(right_yaw_angle)));
	d_right_backward[1] = -d_right_backward[1] * (dis_right / cos(ang2rad(right_yaw_angle)));
	d_left_forward[1] = -d_left_forward[1] * (dis_left / cos(ang2rad(left_yaw_angle)));
	d_left_backward[1] = -d_left_backward[1] * (dis_left / cos(ang2rad(left_yaw_angle)));

	//前置判断3个点符不符合标准，，return 什么你自己定义一下，以后我再慢慢补充规则
	if (fabs(initial_line_up_point.stc_z - initial_line_right_point.stc_z) < 1) {
		//目前暂定 当第一个点的index为-1时，则为不符合标准---第一第二个点高度差不合理
		vector<WaypointInformation> buffer;
		WaypointInformation buffer_first_data;
		buffer_first_data.stc_index = -1;
		buffer.push_back(buffer_first_data);
		return buffer;
	}
	if (fabs(initial_line_right_point.stc_z - initial_line_left_point.stc_z) < 3) {
		//目前暂定 当第一个点的index为-1时，则为不符合标准----第二第三个点高度差不合理
		vector<WaypointInformation> buffer;
		WaypointInformation buffer_first_data;
		buffer_first_data.stc_index = -1;
		buffer.push_back(buffer_first_data);
		return buffer;
	}

	WaypointInformation point_0;
	if (pow(initial_line_prev_point.stc_x, 2) + pow(initial_line_prev_point.stc_y, 2) <= 1)
	{
		point_0.stc_x = initial_line_up_point.stc_x + longlook_whole_tower_distance * d_right_prev_perpendicular[0];
		point_0.stc_y = initial_line_up_point.stc_y + longlook_whole_tower_distance * d_right_prev_perpendicular[1];
		point_0.stc_z = initial_line_up_point.stc_z + safe_height_increament;
		point_0.stc_index = line_out.size() + 1;
		point_0.stc_yaw = get_angle_tonorth(initial_line_up_point.stc_x - point_0.stc_x, initial_line_up_point.stc_y - point_0.stc_y);
		point_0.stc_gimbalPitch = atan((tower_center_height - point_0.stc_z) / longlook_whole_tower_distance) * 180 / 3.1415926;
		point_0.stc_shootPhoto = 1;
		point_0.zoom_Magnification = 2;
		std::vector<uint8_t> temp_target_class_id;
		temp_target_class_id.push_back(1);
		//temp_target_class_id.push_back(3);
		point_0.target_class_id = temp_target_class_id;
		point_0.orient_to_tower = 0;
		point_0.Number_of_layers_of_current_waypoint = -1;
		point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
		point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
		point_0.POI_Z = tower_center_height - point_0.stc_z;
		point_0.waypointType = 1;
		line_out.push_back(point_0);
	}
	else
	{
		vector<double> unit_prev_directon = get_unit_directon3d(initial_line_up_point.stc_x, initial_line_up_point.stc_y, initial_line_up_point.stc_z, initial_line_prev_point.stc_x, initial_line_prev_point.stc_y, initial_line_prev_point.stc_z);
		point_0.stc_x = initial_line_up_point.stc_x + longlook_whole_tower_distance * unit_prev_directon[0];
		point_0.stc_y = initial_line_up_point.stc_y + longlook_whole_tower_distance * unit_prev_directon[1];
		point_0.stc_z = initial_line_up_point.stc_z + longlook_whole_tower_distance * unit_prev_directon[2];
		point_0.stc_index = line_out.size() + 1;
		point_0.stc_yaw = get_angle_tonorth(initial_line_up_point.stc_x - initial_line_prev_point.stc_x, initial_line_up_point.stc_y - initial_line_prev_point.stc_y);
		point_0.stc_gimbalPitch = atan((tower_center_height - point_0.stc_z) / sqrt(pow(point_0.stc_x, 2) + pow(point_0.stc_y, 2))) * 180 / 3.1415926;
		point_0.stc_shootPhoto = 1;
		point_0.zoom_Magnification = 2;
		std::vector<uint8_t> temp_target_class_id;
		temp_target_class_id.push_back(1);
		point_0.target_class_id = temp_target_class_id;
		point_0.orient_to_tower = 0;
		
		point_0.Number_of_layers_of_current_waypoint = -1;
		point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
		point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
		point_0.POI_Z = tower_center_height - point_0.stc_z;
		point_0.waypointType = 1;
		line_out.push_back(point_0);
	}
	//正上方路点
	point_0.stc_x = initial_line_up_point.stc_x;
	point_0.stc_y = initial_line_up_point.stc_y;
	point_0.stc_z = initial_line_up_point.stc_z;
	point_0.stc_index = line_out.size() + 1;
	point_0.stc_yaw = get_angle_tonorth(d_left_next_perpendicular[0], d_left_next_perpendicular[1]);
	point_0.stc_gimbalPitch = -90;
	point_0.stc_shootPhoto = 1;
	point_0.zoom_Magnification = 2;
	std::vector<uint8_t> temp_target_class_id;
	temp_target_class_id.push_back(2);
	point_0.target_class_id = temp_target_class_id;
	point_0.orient_to_tower = 0;
	
	point_0.Number_of_layers_of_current_waypoint = -1;
	point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
	point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
	point_0.POI_Z = tower_center_height - safe_height_increament;
	point_0.waypointType = 1;
	line_out.push_back(point_0);
	//右后塔尖路点
	point_0.stc_x = d_right_backward[0];
	point_0.stc_y = d_right_backward[1];
	point_0.stc_z = initial_line_up_point.stc_z;
	point_0.stc_index = line_out.size() + 1;
	point_0.stc_yaw = get_angle_tonorth(initial_line_up_point.stc_x - d_right_backward[0], initial_line_up_point.stc_y - d_right_backward[1]);
	point_0.stc_gimbalPitch = atan((initial_line_right_point.stc_z) / sqrt(pow(point_0.stc_x, 2) + pow(point_0.stc_y, 2))) * 180 / 3.1415926;
	point_0.stc_shootPhoto = 1;
	point_0.zoom_Magnification = 2;
	temp_target_class_id.clear();
	temp_target_class_id.push_back(4);
	point_0.target_class_id = temp_target_class_id;
	point_0.orient_to_tower = 0;
	
	point_0.Number_of_layers_of_current_waypoint = -1;
	point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
	point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
	point_0.POI_Z = initial_line_right_point.stc_z;
	point_0.waypointType = 1;
	line_out.push_back(point_0);
	//右后绝缘子路店
	point_0.stc_x = d_right_backward[0];
	point_0.stc_y = d_right_backward[1];
	point_0.stc_z = initial_line_right_point.stc_z;
	point_0.stc_index = line_out.size() + 1;
	float yaw_2_center = get_angle_tonorth(initial_line_up_point.stc_x - d_right_backward[0], initial_line_up_point.stc_y - d_right_backward[1]);
	point_0.stc_yaw = angplus(yaw_2_center, peer_2_yaw_center);
	point_0.stc_gimbalPitch = 5;
	point_0.stc_shootPhoto = 1;
	point_0.zoom_Magnification = 2;
	temp_target_class_id.clear();
	temp_target_class_id.push_back(0);
	point_0.target_class_id = temp_target_class_id;
	point_0.orient_to_tower = 0;
	
	point_0.Number_of_layers_of_current_waypoint = 1;
	point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
	point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
	point_0.POI_Z = 0;
	point_0.waypointType = 1;
	line_out.push_back(point_0);
	//右后绝缘子底部路点
	point_0.stc_x = d_right_backward[0];
	point_0.stc_y = d_right_backward[1];
	point_0.stc_z = initial_line_left_point.stc_z;
	point_0.stc_index = line_out.size() + 1;
	yaw_2_center = get_angle_tonorth(initial_line_up_point.stc_x - d_right_backward[0], initial_line_up_point.stc_y - d_right_backward[1]);
	point_0.stc_yaw = angplus(yaw_2_center, peer_2_yaw_center);
	point_0.stc_gimbalPitch = 0;
	point_0.stc_shootPhoto = 0;
	point_0.zoom_Magnification = 2;
	temp_target_class_id.clear();
	temp_target_class_id.push_back(0);
	point_0.target_class_id = temp_target_class_id;
	point_0.orient_to_tower = 0;
	
	point_0.Number_of_layers_of_current_waypoint = layer_number;
	point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
	point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
	point_0.POI_Z = 0;
	point_0.waypointType = 1;
	line_out.push_back(point_0);
	//通道路点
	if (ChannelInspection == 1) {
		point_0.stc_x = initial_line_right_point.stc_x;
		point_0.stc_y = initial_line_right_point.stc_y;
		point_0.stc_z = initial_line_left_point.stc_z;
		point_0.stc_index = line_out.size() + 1;
		if (pow(initial_line_prev_point.stc_x, 2) + pow(initial_line_prev_point.stc_y, 2) <= 1)
		{
			float yaw_2_another = get_angle_tonorth(d_right_prev_perpendicular[0], d_right_prev_perpendicular[1]);
			point_0.stc_yaw = angplus(yaw_2_another, 8);
			//point_0.stc_yaw = get_angle_tonorth(d_right_prev_perpendicular[0], d_right_prev_perpendicular[1]);
			point_0.POI_X = d_right_prev_perpendicular[0] * longlook_whole_tower_distance + point_0.stc_x;
			point_0.POI_Y = d_right_prev_perpendicular[1] * longlook_whole_tower_distance + point_0.stc_y;
			point_0.POI_Z = 0 + point_0.stc_z;
		}
		else {
			//float yaw_2_another = get_angle_tonorth(initial_line_prev_point.stc_x - initial_line_up_point.stc_x, initial_line_prev_point.stc_y - initial_line_up_point.stc_y);
			//point_0.stc_yaw = angplus(yaw_2_another, 8);
			point_0.stc_yaw = get_angle_tonorth(initial_line_prev_point.stc_x - initial_line_up_point.stc_x, initial_line_prev_point.stc_y - initial_line_up_point.stc_y);
			point_0.POI_X = initial_line_prev_point.stc_x + point_0.stc_x;
			point_0.POI_Y = initial_line_prev_point.stc_y + point_0.stc_y;
			point_0.POI_Z = initial_line_prev_point.stc_z - 20 + point_0.stc_z;
		}
		point_0.stc_gimbalPitch = 3;
		point_0.stc_shootPhoto = 1;
		point_0.zoom_Magnification = 2;
		temp_target_class_id.clear();
		temp_target_class_id.push_back(98);
		point_0.target_class_id = temp_target_class_id;
		point_0.orient_to_tower = 0;
		
		point_0.Number_of_layers_of_current_waypoint = 3;

		point_0.waypointType = 1;
		line_out.push_back(point_0);
	}
	//右前绝缘子底部路店
	point_0.stc_x = d_right_forward[0];
	point_0.stc_y = d_right_forward[1];
	point_0.stc_z = initial_line_left_point.stc_z;
	point_0.stc_index = line_out.size() + 1;
	yaw_2_center = get_angle_tonorth(initial_line_up_point.stc_x - d_right_forward[0], initial_line_up_point.stc_y - d_right_forward[1]);
	point_0.stc_yaw = angplus(yaw_2_center, -peer_2_yaw_center);
	point_0.stc_gimbalPitch = 5;
	point_0.stc_shootPhoto = 1;
	point_0.zoom_Magnification = 2;
	temp_target_class_id.clear();
	temp_target_class_id.push_back(0);
	point_0.target_class_id = temp_target_class_id;
	point_0.orient_to_tower = 0;
	
	point_0.Number_of_layers_of_current_waypoint = layer_number;
	point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
	point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
	point_0.POI_Z = 0;
	point_0.waypointType = 1;
	line_out.push_back(point_0);
	//右前绝缘子路店
	point_0.stc_x = d_right_forward[0];
	point_0.stc_y = d_right_forward[1];
	point_0.stc_z = initial_line_right_point.stc_z;
	point_0.stc_index = line_out.size() + 1;
	yaw_2_center = get_angle_tonorth(initial_line_up_point.stc_x - d_right_forward[0], initial_line_up_point.stc_y - d_right_forward[1]);
	point_0.stc_yaw = angplus(yaw_2_center, -peer_2_yaw_center);
	point_0.stc_gimbalPitch = 0;
	point_0.stc_shootPhoto = 0;
	point_0.zoom_Magnification = 2;
	temp_target_class_id.clear();
	temp_target_class_id.push_back(0);
	point_0.target_class_id = temp_target_class_id;
	point_0.orient_to_tower = 0;
	
	point_0.Number_of_layers_of_current_waypoint = 1;
	point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
	point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
	point_0.POI_Z = 0;
	point_0.waypointType = 1;
	line_out.push_back(point_0);
	//右前塔尖路点
	point_0.stc_x = d_right_forward[0];
	point_0.stc_y = d_right_forward[1];
	point_0.stc_z = initial_line_up_point.stc_z;
	point_0.stc_index = line_out.size() + 1;
	point_0.stc_yaw = get_angle_tonorth(initial_line_up_point.stc_x - d_right_forward[0], initial_line_up_point.stc_y - d_right_forward[1]);
	point_0.stc_gimbalPitch = atan((initial_line_right_point.stc_z) / sqrt(pow(point_0.stc_x, 2) + pow(point_0.stc_y, 2))) * 180 / 3.1415926;
	point_0.stc_shootPhoto = 0;
	point_0.zoom_Magnification = 2;
	temp_target_class_id.clear();
	temp_target_class_id.push_back(4);
	point_0.target_class_id = temp_target_class_id;
	point_0.orient_to_tower = 0;
	
	point_0.Number_of_layers_of_current_waypoint = -1;
	point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
	point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
	point_0.POI_Z = initial_line_right_point.stc_z;
	point_0.waypointType = 1;
	line_out.push_back(point_0);
	//左前塔尖路点
	point_0.stc_x = d_left_forward[0];
	point_0.stc_y = d_left_forward[1];
	point_0.stc_z = initial_line_up_point.stc_z;
	point_0.stc_index = line_out.size() + 1;
	point_0.stc_yaw = get_angle_tonorth(initial_line_up_point.stc_x - d_left_forward[0], initial_line_up_point.stc_y - d_left_forward[1]);
	point_0.stc_gimbalPitch = atan((initial_line_right_point.stc_z) / sqrt(pow(point_0.stc_x, 2) + pow(point_0.stc_y, 2))) * 180 / 3.1415926;
	point_0.stc_shootPhoto = 1;
	point_0.zoom_Magnification = 2;
	temp_target_class_id.clear();
	temp_target_class_id.push_back(4);
	point_0.target_class_id = temp_target_class_id;
	point_0.orient_to_tower = 0;
	
	point_0.Number_of_layers_of_current_waypoint = -1;
	point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
	point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
	point_0.POI_Z = initial_line_right_point.stc_z;
	point_0.waypointType = 1;
	line_out.push_back(point_0);
	//左前绝缘子路点
	point_0.stc_x = d_left_forward[0];
	point_0.stc_y = d_left_forward[1];
	point_0.stc_z = initial_line_right_point.stc_z;
	point_0.stc_index = line_out.size() + 1;
	yaw_2_center = get_angle_tonorth(initial_line_up_point.stc_x - d_left_forward[0], initial_line_up_point.stc_y - d_left_forward[1]);
	point_0.stc_yaw = angplus(yaw_2_center, peer_2_yaw_center);
	point_0.stc_gimbalPitch = 5;
	point_0.stc_shootPhoto = 1;
	point_0.zoom_Magnification = 2;
	temp_target_class_id.clear();
	temp_target_class_id.push_back(0);
	point_0.target_class_id = temp_target_class_id;
	point_0.orient_to_tower = 0;
	
	point_0.Number_of_layers_of_current_waypoint = 1;
	point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
	point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
	point_0.POI_Z = 0;
	point_0.waypointType = 1;
	line_out.push_back(point_0);
	//左前绝缘子底部路点
	point_0.stc_x = d_left_forward[0];
	point_0.stc_y = d_left_forward[1];
	point_0.stc_z = initial_line_left_point.stc_z;
	point_0.stc_index = line_out.size() + 1;
	yaw_2_center = get_angle_tonorth(initial_line_up_point.stc_x - d_left_forward[0], initial_line_up_point.stc_y - d_left_forward[1]);
	point_0.stc_yaw = angplus(yaw_2_center, peer_2_yaw_center);
	point_0.stc_gimbalPitch = 0;
	point_0.stc_shootPhoto = 0;
	point_0.zoom_Magnification = 2;
	temp_target_class_id.clear();
	temp_target_class_id.push_back(0);
	point_0.target_class_id = temp_target_class_id;
	point_0.orient_to_tower = 0;
	
	point_0.Number_of_layers_of_current_waypoint = layer_number;
	point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
	point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
	point_0.POI_Z = 0;
	point_0.waypointType = 1;
	line_out.push_back(point_0);
	//左侧通道路点
	if (ChannelInspection == 1) {
		point_0.stc_x = initial_line_left_point.stc_x;
		point_0.stc_y = initial_line_left_point.stc_y;
		point_0.stc_z = initial_line_left_point.stc_z;
		point_0.stc_index = line_out.size() + 1;
		if (pow(initial_line_next_point.stc_x, 2) + pow(initial_line_next_point.stc_y, 2) <= 1)
		{
			float yaw_2_another = get_angle_tonorth(d_left_next_perpendicular[0], d_left_next_perpendicular[1]);
			point_0.stc_yaw = angplus(yaw_2_another, 8);
			point_0.POI_X = d_left_next_perpendicular[0] * longlook_whole_tower_distance + point_0.stc_x;
			point_0.POI_Y = d_left_next_perpendicular[1] * longlook_whole_tower_distance + point_0.stc_y;
			point_0.POI_Z = 0 + point_0.stc_z;
		}
		else {
			point_0.stc_yaw = get_angle_tonorth(initial_line_next_point.stc_x - initial_line_up_point.stc_x, initial_line_next_point.stc_y - initial_line_up_point.stc_y);
			point_0.POI_X = initial_line_next_point.stc_x + point_0.stc_x;
			point_0.POI_Y = initial_line_next_point.stc_y + point_0.stc_y;
			point_0.POI_Z = initial_line_next_point.stc_z - 20 + point_0.stc_z;
		}
		point_0.stc_gimbalPitch = 4;
		point_0.stc_shootPhoto = 1;
		point_0.zoom_Magnification = 2;
		temp_target_class_id.clear();
		temp_target_class_id.push_back(98);
		point_0.target_class_id = temp_target_class_id;
		point_0.orient_to_tower = 0;
		
		point_0.Number_of_layers_of_current_waypoint = 3;
		point_0.waypointType = 1;
		line_out.push_back(point_0);
	}
	//左后绝缘子底部路点
	point_0.stc_x = d_left_backward[0];
	point_0.stc_y = d_left_backward[1];
	point_0.stc_z = initial_line_left_point.stc_z;
	point_0.stc_index = line_out.size() + 1;
	yaw_2_center = get_angle_tonorth(initial_line_up_point.stc_x - d_left_backward[0], initial_line_up_point.stc_y - d_left_backward[1]);
	point_0.stc_yaw = angplus(yaw_2_center, -peer_2_yaw_center);
	point_0.stc_gimbalPitch = 5;
	point_0.stc_shootPhoto = 1;
	point_0.zoom_Magnification = 2;
	temp_target_class_id.clear();
	temp_target_class_id.push_back(0);
	point_0.target_class_id = temp_target_class_id;
	point_0.orient_to_tower = 0;
	
	point_0.Number_of_layers_of_current_waypoint = layer_number;
	point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
	point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
	point_0.POI_Z = 0;
	point_0.waypointType = 1;
	line_out.push_back(point_0);
	//左后绝缘子路点
	point_0.stc_x = d_left_backward[0];
	point_0.stc_y = d_left_backward[1];
	point_0.stc_z = initial_line_right_point.stc_z;
	point_0.stc_index = line_out.size() + 1;
	yaw_2_center = get_angle_tonorth(initial_line_up_point.stc_x - d_left_backward[0], initial_line_up_point.stc_y - d_left_backward[1]);
	point_0.stc_yaw = angplus(yaw_2_center, -peer_2_yaw_center);
	point_0.stc_gimbalPitch = 0;
	point_0.stc_shootPhoto = 0;
	point_0.zoom_Magnification = 2;
	temp_target_class_id.clear();
	temp_target_class_id.push_back(0);
	point_0.target_class_id = temp_target_class_id;
	point_0.orient_to_tower = 0;
	
	point_0.Number_of_layers_of_current_waypoint = 1;
	point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
	point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
	point_0.POI_Z = 0;
	point_0.waypointType = 1;
	line_out.push_back(point_0);
	
	//左后塔尖路点
	point_0.stc_x = d_left_backward[0];
	point_0.stc_y = d_left_backward[1];
	point_0.stc_z = initial_line_up_point.stc_z;
	point_0.stc_index = line_out.size() + 1;
	point_0.stc_yaw = get_angle_tonorth(initial_line_up_point.stc_x - d_left_backward[0], initial_line_up_point.stc_y - d_left_backward[1]);
	point_0.stc_gimbalPitch = atan((initial_line_right_point.stc_z) / sqrt(pow(point_0.stc_x, 2) + pow(point_0.stc_y, 2))) * 180 / 3.1415926;
	point_0.stc_shootPhoto = 0;
	point_0.zoom_Magnification = 2;
	temp_target_class_id.clear();
	temp_target_class_id.push_back(4);
	point_0.target_class_id = temp_target_class_id;
	point_0.orient_to_tower = 0;
	
	point_0.Number_of_layers_of_current_waypoint = -1;
	point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
	point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
	point_0.POI_Z = initial_line_right_point.stc_z;
	point_0.waypointType = 1;
	line_out.push_back(point_0);
	//回头整体路点
	if (pow(initial_line_next_point.stc_x, 2) + pow(initial_line_next_point.stc_y, 2) <= 1)
	{
		point_0.stc_x = initial_line_up_point.stc_x + longlook_whole_tower_distance * d_left_next_perpendicular[0];
		point_0.stc_y = initial_line_up_point.stc_y + longlook_whole_tower_distance * d_left_next_perpendicular[1];
		point_0.stc_z = initial_line_up_point.stc_z + safe_height_increament;
		point_0.stc_index = line_out.size() + 1;
		point_0.stc_yaw = get_angle_tonorth(initial_line_up_point.stc_x - point_0.stc_x, initial_line_up_point.stc_y - point_0.stc_y);
		point_0.stc_gimbalPitch = atan((tower_center_height - point_0.stc_z) / longlook_whole_tower_distance) * 180 / 3.1415926;
		point_0.stc_shootPhoto = 1;
		point_0.zoom_Magnification = 2;
		temp_target_class_id.clear();
		temp_target_class_id.push_back(1);
		//temp_target_class_id.push_back(5);
		point_0.target_class_id = temp_target_class_id;
		point_0.orient_to_tower = 0;
		
		point_0.Number_of_layers_of_current_waypoint = -1;
		point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
		point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
		point_0.POI_Z = tower_center_height - point_0.stc_z;
		point_0.waypointType = 1;
		line_out.push_back(point_0);

		//////////////////////调整机头至下一座塔
		vector<double> unit_next_directon = get_unit_directon3d(initial_line_up_point.stc_x, initial_line_up_point.stc_y, initial_line_up_point.stc_z, initial_line_next_point.stc_x, initial_line_next_point.stc_y, initial_line_next_point.stc_z);
		point_0.stc_x = initial_line_up_point.stc_x + (longlook_whole_tower_distance + 5) * d_left_next_perpendicular[0];
		point_0.stc_y = initial_line_up_point.stc_y + (longlook_whole_tower_distance + 5) * d_left_next_perpendicular[1];
		point_0.stc_z = initial_line_up_point.stc_z + safe_height_increament;
		point_0.stc_index = line_out.size() + 1;
		point_0.stc_yaw = get_angle_tonorth(point_0.stc_x - initial_line_up_point.stc_x, point_0.stc_y - initial_line_up_point.stc_y);
		point_0.stc_gimbalPitch = atan((tower_center_height - point_0.stc_z) / sqrt(pow(point_0.stc_x, 2) + pow(point_0.stc_y, 2))) * 180 / 3.1415926;
		point_0.stc_shootPhoto = 0;
		point_0.zoom_Magnification = 2;
		temp_target_class_id.clear();
		temp_target_class_id.push_back(1);
		point_0.target_class_id = temp_target_class_id;
		point_0.orient_to_tower = 0;
		
		point_0.Number_of_layers_of_current_waypoint = -1;
		point_0.POI_X = point_0.stc_x - initial_line_up_point.stc_x;
		point_0.POI_Y = point_0.stc_y - initial_line_up_point.stc_y;
		point_0.POI_Z = tower_center_height - point_0.stc_z;
		point_0.waypointType = 1;
		line_out.push_back(point_0);

	}else
	{
		vector<double> unit_next_directon = get_unit_directon3d(initial_line_up_point.stc_x, initial_line_up_point.stc_y, initial_line_up_point.stc_z, initial_line_next_point.stc_x, initial_line_next_point.stc_y, initial_line_next_point.stc_z);
		point_0.stc_x = initial_line_up_point.stc_x + longlook_whole_tower_distance * unit_next_directon[0];
		point_0.stc_y = initial_line_up_point.stc_y + longlook_whole_tower_distance * unit_next_directon[1];
		point_0.stc_z = initial_line_up_point.stc_z + longlook_whole_tower_distance * unit_next_directon[2];
		point_0.stc_index = line_out.size() + 1;
		point_0.stc_yaw = get_angle_tonorth(initial_line_up_point.stc_x - initial_line_next_point.stc_x, initial_line_up_point.stc_y - initial_line_next_point.stc_y);
		point_0.stc_gimbalPitch = atan((tower_center_height - point_0.stc_z) / sqrt(pow(point_0.stc_x, 2) + pow(point_0.stc_y, 2))) * 180 / 3.1415926;
		point_0.stc_shootPhoto = 1;
		point_0.zoom_Magnification = 2;
		temp_target_class_id.clear();
		temp_target_class_id.push_back(1);
		point_0.target_class_id = temp_target_class_id;
		point_0.orient_to_tower = 0;
		
		point_0.Number_of_layers_of_current_waypoint = -1;
		point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
		point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
		point_0.POI_Z = tower_center_height - point_0.stc_z;
		point_0.waypointType = 1;
		line_out.push_back(point_0);

		//////////////////////调整机头至下一座塔
		unit_next_directon = get_unit_directon3d(initial_line_up_point.stc_x, initial_line_up_point.stc_y, initial_line_up_point.stc_z, initial_line_next_point.stc_x, initial_line_next_point.stc_y, initial_line_next_point.stc_z);
		point_0.stc_x = initial_line_up_point.stc_x + (longlook_whole_tower_distance + 5) * unit_next_directon[0];
		point_0.stc_y = initial_line_up_point.stc_y + (longlook_whole_tower_distance + 5) * unit_next_directon[1];
		point_0.stc_z = initial_line_up_point.stc_z + (longlook_whole_tower_distance + 5) * unit_next_directon[2];
		point_0.stc_index = line_out.size() + 1;
		point_0.stc_yaw = get_angle_tonorth(initial_line_next_point.stc_x - initial_line_up_point.stc_x, initial_line_next_point.stc_y - initial_line_up_point.stc_y);
		point_0.stc_gimbalPitch = atan((tower_center_height - point_0.stc_z) / sqrt(pow(point_0.stc_x, 2) + pow(point_0.stc_y, 2))) * 180 / 3.1415926;
		point_0.stc_shootPhoto = 0;
		point_0.zoom_Magnification = 2;
		temp_target_class_id.clear();
		temp_target_class_id.push_back(1);
		point_0.target_class_id = temp_target_class_id;
		point_0.orient_to_tower = 0;
		
		point_0.Number_of_layers_of_current_waypoint = -1;
		point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
		point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
		point_0.POI_Z = tower_center_height - point_0.stc_z;
		point_0.waypointType = 1;
		line_out.push_back(point_0);
	}
	return line_out;
}


/////////////////////////////倒U型航线
std::vector<WaypointInformation> PointToLineV3::pointToLine_OU(RecPoint input)
{
	double safe_height_increament = 10;
	vector<WaypointInformation> line_out;
	line_out.clear();
	DatumPoint initial_line_up_point = input.rec_pt[0];
	DatumPoint initial_line_right_point = input.rec_pt[1];
	DatumPoint initial_line_left_point = input.rec_pt[2];
	DatumPoint initial_line_prev_point = input.prev_pt;
	DatumPoint initial_line_next_point = input.next_pt;
	double tower_center_height = (initial_line_right_point.stc_z + initial_line_left_point.stc_z) / 2;
	double left_yaw_angle = input.left_angle;
	double right_yaw_angle = input.right_angle;
	double longlook_whole_tower_distance = input.height;
	int ChannelInspection = input.ChannelInspection;
	float peer_2_yaw_center = input.yaw_rotate_angle_selected;
	int layer_number = input.layer_num;

	double dis_right = get_distance2d(initial_line_right_point.stc_x, initial_line_right_point.stc_y, initial_line_up_point.stc_x, initial_line_up_point.stc_y);
	double dis_left = get_distance2d(initial_line_left_point.stc_x, initial_line_left_point.stc_y, initial_line_up_point.stc_x, initial_line_up_point.stc_y);
	vector<double> unit_rightpoint_dirction = get_unit_directon2d(initial_line_right_point.stc_x, initial_line_right_point.stc_y, initial_line_up_point.stc_x, initial_line_up_point.stc_y);
	vector<double> unit_leftpoint_dirction = get_unit_directon2d(initial_line_left_point.stc_x, initial_line_left_point.stc_y, initial_line_up_point.stc_x, initial_line_up_point.stc_y);
	vector<double> d_right_forward = matrixrotate2d(unit_rightpoint_dirction, -right_yaw_angle);
	vector<double> d_right_backward = matrixrotate2d(unit_rightpoint_dirction, right_yaw_angle);
	vector<double> d_right_prev_perpendicular = matrixrotate2d(unit_rightpoint_dirction, 90);
	vector<double> d_left_forward = matrixrotate2d(unit_leftpoint_dirction, left_yaw_angle);
	vector<double> d_left_backward = matrixrotate2d(unit_leftpoint_dirction, -left_yaw_angle);
	vector<double> d_left_next_perpendicular = matrixrotate2d(unit_leftpoint_dirction, 90);

	d_right_forward[0] = -d_right_forward[0] * (dis_right / cos(ang2rad(right_yaw_angle)));
	d_right_backward[0] = -d_right_backward[0] * (dis_right / cos(ang2rad(right_yaw_angle)));
	d_left_forward[0] = -d_left_forward[0] * (dis_left / cos(ang2rad(left_yaw_angle)));
	d_left_backward[0] = -d_left_backward[0] * (dis_left / cos(ang2rad(left_yaw_angle)));
	d_right_forward[1] = -d_right_forward[1] * (dis_right / cos(ang2rad(right_yaw_angle)));
	d_right_backward[1] = -d_right_backward[1] * (dis_right / cos(ang2rad(right_yaw_angle)));
	d_left_forward[1] = -d_left_forward[1] * (dis_left / cos(ang2rad(left_yaw_angle)));
	d_left_backward[1] = -d_left_backward[1] * (dis_left / cos(ang2rad(left_yaw_angle)));

	//前置判断3个点符不符合标准，，return 什么你自己定义一下，以后我再慢慢补充规则
	if (fabs(initial_line_up_point.stc_z - initial_line_right_point.stc_z) < 1) {
		//目前暂定 当第一个点的index为-1时，则为不符合标准---第一第二个点高度差不合理
		vector<WaypointInformation> buffer;
		WaypointInformation buffer_first_data;
		buffer_first_data.stc_index = -1;
		buffer.push_back(buffer_first_data);
		return buffer;
	}
	if (fabs(initial_line_right_point.stc_z - initial_line_left_point.stc_z) < 3) {
		//目前暂定 当第一个点的index为-1时，则为不符合标准----第二第三个点高度差不合理
		vector<WaypointInformation> buffer;
		WaypointInformation buffer_first_data;
		buffer_first_data.stc_index = -1;
		buffer.push_back(buffer_first_data);
		return buffer;
	}

	WaypointInformation point_0;
	if (pow(initial_line_prev_point.stc_x, 2) + pow(initial_line_prev_point.stc_y, 2) <= 1)
	{
		point_0.stc_x = initial_line_up_point.stc_x + longlook_whole_tower_distance * d_right_prev_perpendicular[0];
		point_0.stc_y = initial_line_up_point.stc_y + longlook_whole_tower_distance * d_right_prev_perpendicular[1];
		point_0.stc_z = initial_line_up_point.stc_z + safe_height_increament;
		point_0.stc_index = line_out.size() + 1;
		point_0.stc_yaw = get_angle_tonorth(initial_line_up_point.stc_x - point_0.stc_x, initial_line_up_point.stc_y - point_0.stc_y);
		point_0.stc_gimbalPitch = atan((tower_center_height - point_0.stc_z) / longlook_whole_tower_distance) * 180 / 3.1415926;
		point_0.stc_shootPhoto = 1;
		point_0.zoom_Magnification = 2;
		std::vector<uint8_t> temp_target_class_id;
		temp_target_class_id.push_back(1);
		//temp_target_class_id.push_back(3);
		point_0.target_class_id = temp_target_class_id;
		point_0.orient_to_tower = 0;
		
		point_0.Number_of_layers_of_current_waypoint = -1;
		point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
		point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
		point_0.POI_Z = tower_center_height - point_0.stc_z;
		point_0.waypointType = 1;
		line_out.push_back(point_0);
	}
	else
	{
		vector<double> unit_prev_directon = get_unit_directon3d(initial_line_up_point.stc_x, initial_line_up_point.stc_y, initial_line_up_point.stc_z, initial_line_prev_point.stc_x, initial_line_prev_point.stc_y, initial_line_prev_point.stc_z);
		point_0.stc_x = initial_line_up_point.stc_x + longlook_whole_tower_distance * unit_prev_directon[0];
		point_0.stc_y = initial_line_up_point.stc_y + longlook_whole_tower_distance * unit_prev_directon[1];
		point_0.stc_z = initial_line_up_point.stc_z + longlook_whole_tower_distance * unit_prev_directon[2];
		point_0.stc_index = line_out.size() + 1;
		point_0.stc_yaw = get_angle_tonorth(initial_line_up_point.stc_x - initial_line_prev_point.stc_x, initial_line_up_point.stc_y - initial_line_prev_point.stc_y);
		point_0.stc_gimbalPitch = atan((tower_center_height - point_0.stc_z) / sqrt(pow(point_0.stc_x, 2) + pow(point_0.stc_y, 2))) * 180 / 3.1415926;
		point_0.stc_shootPhoto = 1;
		point_0.zoom_Magnification = 2;
		std::vector<uint8_t> temp_target_class_id;
		temp_target_class_id.push_back(1);
		point_0.target_class_id = temp_target_class_id;
		point_0.orient_to_tower = 0;
		
		point_0.Number_of_layers_of_current_waypoint = -1;
		point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
		point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
		point_0.POI_Z = tower_center_height - point_0.stc_z;
		point_0.waypointType = 1;
		line_out.push_back(point_0);
	}
	//正上方路点
	point_0.stc_x = initial_line_up_point.stc_x;
	point_0.stc_y = initial_line_up_point.stc_y;
	point_0.stc_z = initial_line_up_point.stc_z;
	point_0.stc_index = line_out.size() + 1;
	point_0.stc_yaw = get_angle_tonorth(d_left_next_perpendicular[0], d_left_next_perpendicular[1]);
	point_0.stc_gimbalPitch = -90;
	point_0.stc_shootPhoto = 1;
	point_0.zoom_Magnification = 2;
	std::vector<uint8_t> temp_target_class_id;
	temp_target_class_id.push_back(2);
	point_0.target_class_id = temp_target_class_id;
	point_0.orient_to_tower = 0;
	
	point_0.Number_of_layers_of_current_waypoint = -1;
	point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
	point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
	point_0.POI_Z = tower_center_height - safe_height_increament;
	point_0.waypointType = 1;
	line_out.push_back(point_0);
	//右后塔尖路点
	point_0.stc_x = d_right_backward[0];
	point_0.stc_y = d_right_backward[1];
	point_0.stc_z = initial_line_up_point.stc_z;
	point_0.stc_index = line_out.size() + 1;
	point_0.stc_yaw = get_angle_tonorth(initial_line_up_point.stc_x - d_right_backward[0], initial_line_up_point.stc_y - d_right_backward[1]);
	point_0.stc_gimbalPitch = atan((initial_line_right_point.stc_z) / sqrt(pow(point_0.stc_x, 2) + pow(point_0.stc_y, 2))) * 180 / 3.1415926;
	point_0.stc_shootPhoto = 1;
	point_0.zoom_Magnification = 2;
	temp_target_class_id.clear();
	temp_target_class_id.push_back(4);
	point_0.target_class_id = temp_target_class_id;
	point_0.orient_to_tower = 0;
	
	point_0.Number_of_layers_of_current_waypoint = -1;
	point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
	point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
	point_0.POI_Z = initial_line_right_point.stc_z;
	point_0.waypointType = 1;
	line_out.push_back(point_0);
	//右后绝缘子路点
	point_0.stc_x = d_right_backward[0];
	point_0.stc_y = d_right_backward[1];
	point_0.stc_z = initial_line_right_point.stc_z;
	point_0.stc_index = line_out.size() + 1;
	float yaw_2_center = get_angle_tonorth(initial_line_up_point.stc_x - d_right_backward[0], initial_line_up_point.stc_y - d_right_backward[1]);
	point_0.stc_yaw = angplus(yaw_2_center, peer_2_yaw_center);
	point_0.stc_gimbalPitch = 5;
	point_0.stc_shootPhoto = 1;
	point_0.zoom_Magnification = 2;
	temp_target_class_id.clear();
	temp_target_class_id.push_back(0);
	point_0.target_class_id = temp_target_class_id;
	point_0.orient_to_tower = 0;
	
	point_0.Number_of_layers_of_current_waypoint = 1;
	point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
	point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
	point_0.POI_Z = 0;
	point_0.waypointType = 1;
	line_out.push_back(point_0);
	//右后绝缘子底部路点
	point_0.stc_x = d_right_backward[0];
	point_0.stc_y = d_right_backward[1];
	point_0.stc_z = initial_line_left_point.stc_z;
	point_0.stc_index = line_out.size() + 1;
	yaw_2_center = get_angle_tonorth(initial_line_up_point.stc_x - d_right_backward[0], initial_line_up_point.stc_y - d_right_backward[1]);
	point_0.stc_yaw = angplus(yaw_2_center, peer_2_yaw_center);
	point_0.stc_gimbalPitch = 0;
	point_0.stc_shootPhoto = 0;
	point_0.zoom_Magnification = 2;
	temp_target_class_id.clear();
	temp_target_class_id.push_back(0);
	point_0.target_class_id = temp_target_class_id;
	point_0.orient_to_tower = 0;
	
	point_0.Number_of_layers_of_current_waypoint = layer_number;
	point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
	point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
	point_0.POI_Z = 0;
	point_0.waypointType = 1;
	line_out.push_back(point_0);
	//通道路点
	if (ChannelInspection == 1) {
		point_0.stc_x = initial_line_right_point.stc_x;
		point_0.stc_y = initial_line_right_point.stc_y;
		point_0.stc_z = initial_line_left_point.stc_z;
		point_0.stc_index = line_out.size() + 1;
		if (pow(initial_line_prev_point.stc_x, 2) + pow(initial_line_prev_point.stc_y, 2) <= 1)
		{
			float yaw_2_another = get_angle_tonorth(d_right_prev_perpendicular[0], d_right_prev_perpendicular[1]);
			point_0.stc_yaw = angplus(yaw_2_another, 8);
			//point_0.stc_yaw = get_angle_tonorth(d_right_prev_perpendicular[0], d_right_prev_perpendicular[1]);
			point_0.POI_X = d_right_prev_perpendicular[0] * longlook_whole_tower_distance + point_0.stc_x;
			point_0.POI_Y = d_right_prev_perpendicular[1] * longlook_whole_tower_distance + point_0.stc_y;
			point_0.POI_Z = 0 + point_0.stc_z;
		}
		else {
			//float yaw_2_another = get_angle_tonorth(initial_line_prev_point.stc_x - initial_line_up_point.stc_x, initial_line_prev_point.stc_y - initial_line_up_point.stc_y);
			//point_0.stc_yaw = angplus(yaw_2_another, 8);
			point_0.stc_yaw = get_angle_tonorth(initial_line_prev_point.stc_x - initial_line_up_point.stc_x, initial_line_prev_point.stc_y - initial_line_up_point.stc_y);
			point_0.POI_X = initial_line_prev_point.stc_x + point_0.stc_x;
			point_0.POI_Y = initial_line_prev_point.stc_y + point_0.stc_y;
			point_0.POI_Z = initial_line_prev_point.stc_z + 20 + point_0.stc_z;
		}
		point_0.stc_gimbalPitch = 3;
		point_0.stc_shootPhoto = 1;
		point_0.zoom_Magnification = 2;
		temp_target_class_id.clear();
		temp_target_class_id.push_back(98);
		point_0.target_class_id = temp_target_class_id;
		point_0.orient_to_tower = 0;
		
		point_0.Number_of_layers_of_current_waypoint = 3;

		point_0.waypointType = 1;
		line_out.push_back(point_0);
	}

	//右前塔尖路点
	point_0.stc_x = d_right_forward[0];
	point_0.stc_y = d_right_forward[1];
	point_0.stc_z = initial_line_up_point.stc_z;
	point_0.stc_index = line_out.size() + 1;
	point_0.stc_yaw = get_angle_tonorth(initial_line_up_point.stc_x - d_right_forward[0], initial_line_up_point.stc_y - d_right_forward[1]);
	point_0.stc_gimbalPitch = atan((initial_line_right_point.stc_z) / sqrt(pow(point_0.stc_x, 2) + pow(point_0.stc_y, 2))) * 180 / 3.1415926;
	point_0.stc_shootPhoto = 0;
	point_0.zoom_Magnification = 2;
	temp_target_class_id.clear();
	temp_target_class_id.push_back(4);
	point_0.target_class_id = temp_target_class_id;
	point_0.orient_to_tower = 0;
	
	point_0.Number_of_layers_of_current_waypoint = -1;
	point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
	point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
	point_0.POI_Z = initial_line_right_point.stc_z;
	point_0.waypointType = 1;
	line_out.push_back(point_0);
	//左前塔尖路点
	point_0.stc_x = d_left_forward[0];
	point_0.stc_y = d_left_forward[1];
	point_0.stc_z = initial_line_up_point.stc_z;
	point_0.stc_index = line_out.size() + 1;
	point_0.stc_yaw = get_angle_tonorth(initial_line_up_point.stc_x - d_left_forward[0], initial_line_up_point.stc_y - d_left_forward[1]);
	point_0.stc_gimbalPitch = atan((initial_line_right_point.stc_z) / sqrt(pow(point_0.stc_x, 2) + pow(point_0.stc_y, 2))) * 180 / 3.1415926;
	point_0.stc_shootPhoto = 1;
	point_0.zoom_Magnification = 2;
	temp_target_class_id.clear();
	temp_target_class_id.push_back(4);
	point_0.target_class_id = temp_target_class_id;
	point_0.orient_to_tower = 0;
	
	point_0.Number_of_layers_of_current_waypoint = -1;
	point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
	point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
	point_0.POI_Z = initial_line_right_point.stc_z;
	point_0.waypointType = 1;
	line_out.push_back(point_0);
	//左前绝缘子路点
	point_0.stc_x = d_left_forward[0];
	point_0.stc_y = d_left_forward[1];
	point_0.stc_z = initial_line_right_point.stc_z;
	point_0.stc_index = line_out.size() + 1;
	yaw_2_center = get_angle_tonorth(initial_line_up_point.stc_x - d_left_forward[0], initial_line_up_point.stc_y - d_left_forward[1]);
	point_0.stc_yaw = angplus(yaw_2_center, peer_2_yaw_center);
	point_0.stc_gimbalPitch = 5;
	point_0.stc_shootPhoto = 1;
	point_0.zoom_Magnification = 2;
	temp_target_class_id.clear();
	temp_target_class_id.push_back(0);
	point_0.target_class_id = temp_target_class_id;
	point_0.orient_to_tower = 0;
	
	point_0.Number_of_layers_of_current_waypoint = 1;
	point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
	point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
	point_0.POI_Z = 0;
	point_0.waypointType = 1;
	line_out.push_back(point_0);
	//左前绝缘子底部路点
	point_0.stc_x = d_left_forward[0];
	point_0.stc_y = d_left_forward[1];
	point_0.stc_z = initial_line_left_point.stc_z;
	point_0.stc_index = line_out.size() + 1;
	yaw_2_center = get_angle_tonorth(initial_line_up_point.stc_x - d_left_forward[0], initial_line_up_point.stc_y - d_left_forward[1]);
	point_0.stc_yaw = angplus(yaw_2_center, peer_2_yaw_center);
	point_0.stc_gimbalPitch = 0;
	point_0.stc_shootPhoto = 0;
	point_0.zoom_Magnification = 2;
	temp_target_class_id.clear();
	temp_target_class_id.push_back(0);
	point_0.target_class_id = temp_target_class_id;
	point_0.orient_to_tower = 0;
	
	point_0.Number_of_layers_of_current_waypoint = layer_number;
	point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
	point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
	point_0.POI_Z = 0;
	point_0.waypointType = 1;
	line_out.push_back(point_0);
	//左侧通道路点
	if (ChannelInspection == 1) {
		point_0.stc_x = initial_line_left_point.stc_x;
		point_0.stc_y = initial_line_left_point.stc_y;
		point_0.stc_z = initial_line_left_point.stc_z;
		point_0.stc_index = line_out.size() + 1;
		if (pow(initial_line_next_point.stc_x, 2) + pow(initial_line_next_point.stc_y, 2) <= 1)
		{
			float yaw_2_another = get_angle_tonorth(d_left_next_perpendicular[0], d_left_next_perpendicular[1]);
			point_0.stc_yaw = angplus(yaw_2_another, 8);
			point_0.POI_X = d_left_next_perpendicular[0] * longlook_whole_tower_distance + point_0.stc_x;
			point_0.POI_Y = d_left_next_perpendicular[1] * longlook_whole_tower_distance + point_0.stc_y;
			point_0.POI_Z = 0 + point_0.stc_z;
		}
		else {
			point_0.stc_yaw = get_angle_tonorth(initial_line_next_point.stc_x - initial_line_up_point.stc_x, initial_line_next_point.stc_y - initial_line_up_point.stc_y);
			point_0.POI_X = initial_line_next_point.stc_x + point_0.stc_x;
			point_0.POI_Y = initial_line_next_point.stc_y + point_0.stc_y;
			point_0.POI_Z = initial_line_next_point.stc_z - 20 + point_0.stc_z;
		}
		point_0.stc_gimbalPitch = 4;
		point_0.stc_shootPhoto = 1;
		point_0.zoom_Magnification = 2;
		temp_target_class_id.clear();
		temp_target_class_id.push_back(98);
		point_0.target_class_id = temp_target_class_id;
		point_0.orient_to_tower = 0;
		
		point_0.Number_of_layers_of_current_waypoint = 3;
		point_0.waypointType = 1;
		line_out.push_back(point_0);
	}
	//左后塔尖路点
	point_0.stc_x = d_left_backward[0];
	point_0.stc_y = d_left_backward[1];
	point_0.stc_z = initial_line_up_point.stc_z;
	point_0.stc_index = line_out.size() + 1;
	point_0.stc_yaw = get_angle_tonorth(initial_line_up_point.stc_x - d_left_backward[0], initial_line_up_point.stc_y - d_left_backward[1]);
	point_0.stc_gimbalPitch = atan((initial_line_right_point.stc_z) / sqrt(pow(point_0.stc_x, 2) + pow(point_0.stc_y, 2))) * 180 / 3.1415926;
	point_0.stc_shootPhoto = 0;
	point_0.zoom_Magnification = 2;
	temp_target_class_id.clear();
	temp_target_class_id.push_back(4);
	point_0.target_class_id = temp_target_class_id;
	point_0.orient_to_tower = 0;
	
	point_0.Number_of_layers_of_current_waypoint = -1;
	point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
	point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
	point_0.POI_Z = initial_line_right_point.stc_z;
	point_0.waypointType = 1;
	line_out.push_back(point_0);
	//回头整体路点
	if (pow(initial_line_next_point.stc_x, 2) + pow(initial_line_next_point.stc_y, 2) <= 1)
	{
		point_0.stc_x = initial_line_up_point.stc_x + longlook_whole_tower_distance * d_left_next_perpendicular[0];
		point_0.stc_y = initial_line_up_point.stc_y + longlook_whole_tower_distance * d_left_next_perpendicular[1];
		point_0.stc_z = initial_line_up_point.stc_z + safe_height_increament;
		point_0.stc_index = line_out.size() + 1;
		point_0.stc_yaw = get_angle_tonorth(initial_line_up_point.stc_x - point_0.stc_x, initial_line_up_point.stc_y - point_0.stc_y);
		point_0.stc_gimbalPitch = atan((tower_center_height - point_0.stc_z) / longlook_whole_tower_distance) * 180 / 3.1415926;
		point_0.stc_shootPhoto = 1;
		point_0.zoom_Magnification = 2;
		temp_target_class_id.clear();
		temp_target_class_id.push_back(1);
		//temp_target_class_id.push_back(5);
		point_0.target_class_id = temp_target_class_id;
		point_0.orient_to_tower = 0;
		
		point_0.Number_of_layers_of_current_waypoint = -1;
		point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
		point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
		point_0.POI_Z = tower_center_height - point_0.stc_z;
		point_0.waypointType = 1;
		line_out.push_back(point_0);

		//////////////////////调整机头至下一座塔
		vector<double> unit_next_directon = get_unit_directon3d(initial_line_up_point.stc_x, initial_line_up_point.stc_y, initial_line_up_point.stc_z, initial_line_next_point.stc_x, initial_line_next_point.stc_y, initial_line_next_point.stc_z);
		point_0.stc_x = initial_line_up_point.stc_x + (longlook_whole_tower_distance + 5) * d_left_next_perpendicular[0];
		point_0.stc_y = initial_line_up_point.stc_y + (longlook_whole_tower_distance + 5) * d_left_next_perpendicular[1];
		point_0.stc_z = initial_line_up_point.stc_z + safe_height_increament;
		point_0.stc_index = line_out.size() + 1;
		point_0.stc_yaw = get_angle_tonorth(point_0.stc_x - initial_line_up_point.stc_x, point_0.stc_y - initial_line_up_point.stc_y);
		point_0.stc_gimbalPitch = atan((tower_center_height - point_0.stc_z) / sqrt(pow(point_0.stc_x, 2) + pow(point_0.stc_y, 2))) * 180 / 3.1415926;
		point_0.stc_shootPhoto = 0;
		point_0.zoom_Magnification = 2;
		temp_target_class_id.clear();
		temp_target_class_id.push_back(1);
		point_0.target_class_id = temp_target_class_id;
		point_0.orient_to_tower = 0;
		
		point_0.Number_of_layers_of_current_waypoint = -1;
		point_0.POI_X = point_0.stc_x - initial_line_up_point.stc_x;
		point_0.POI_Y = point_0.stc_y - initial_line_up_point.stc_y;
		point_0.POI_Z = tower_center_height - point_0.stc_z;
		point_0.waypointType = 1;
		line_out.push_back(point_0);

	}
	else
	{
		vector<double> unit_next_directon = get_unit_directon3d(initial_line_up_point.stc_x, initial_line_up_point.stc_y, initial_line_up_point.stc_z, initial_line_next_point.stc_x, initial_line_next_point.stc_y, initial_line_next_point.stc_z);
		point_0.stc_x = initial_line_up_point.stc_x + longlook_whole_tower_distance * unit_next_directon[0];
		point_0.stc_y = initial_line_up_point.stc_y + longlook_whole_tower_distance * unit_next_directon[1];
		point_0.stc_z = initial_line_up_point.stc_z + longlook_whole_tower_distance * unit_next_directon[2];
		point_0.stc_index = line_out.size() + 1;
		point_0.stc_yaw = get_angle_tonorth(initial_line_up_point.stc_x - initial_line_next_point.stc_x, initial_line_up_point.stc_y - initial_line_next_point.stc_y);
		point_0.stc_gimbalPitch = atan((tower_center_height - point_0.stc_z) / sqrt(pow(point_0.stc_x, 2) + pow(point_0.stc_y, 2))) * 180 / 3.1415926;
		point_0.stc_shootPhoto = 1;
		point_0.zoom_Magnification = 2;
		temp_target_class_id.clear();
		temp_target_class_id.push_back(1);
		point_0.target_class_id = temp_target_class_id;
		point_0.orient_to_tower = 0;
		
		point_0.Number_of_layers_of_current_waypoint = -1;
		point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
		point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
		point_0.POI_Z = tower_center_height - point_0.stc_z;
		point_0.waypointType = 1;
		line_out.push_back(point_0);

		//////////////////////调整机头至下一座塔
		unit_next_directon = get_unit_directon3d(initial_line_up_point.stc_x, initial_line_up_point.stc_y, initial_line_up_point.stc_z, initial_line_next_point.stc_x, initial_line_next_point.stc_y, initial_line_next_point.stc_z);
		point_0.stc_x = initial_line_up_point.stc_x + (longlook_whole_tower_distance + 5) * unit_next_directon[0];
		point_0.stc_y = initial_line_up_point.stc_y + (longlook_whole_tower_distance + 5) * unit_next_directon[1];
		point_0.stc_z = initial_line_up_point.stc_z + (longlook_whole_tower_distance + 5) * unit_next_directon[2];
		point_0.stc_index = line_out.size() + 1;
		point_0.stc_yaw = get_angle_tonorth(initial_line_next_point.stc_x - initial_line_up_point.stc_x, initial_line_next_point.stc_y - initial_line_up_point.stc_y);
		point_0.stc_gimbalPitch = atan((tower_center_height - point_0.stc_z) / sqrt(pow(point_0.stc_x, 2) + pow(point_0.stc_y, 2))) * 180 / 3.1415926;
		point_0.stc_shootPhoto = 0;
		point_0.zoom_Magnification = 2;
		temp_target_class_id.clear();
		temp_target_class_id.push_back(1);
		point_0.target_class_id = temp_target_class_id;
		point_0.orient_to_tower = 0;
		
		point_0.Number_of_layers_of_current_waypoint = -1;
		point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
		point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
		point_0.POI_Z = tower_center_height - point_0.stc_z;
		point_0.waypointType = 1;
		line_out.push_back(point_0);
	}
	return line_out;
}



std::vector<WaypointInformation> PointToLineV3::route_to_osdkroute_V3(CommissionType2Post tempcommissionType2Post) {
	std::vector<WaypointInformation> vector_commute_task;
	string temp_poi = "";
	for (int i = 0; i < tempcommissionType2Post.stc_fplist.size(); i++)
	{
		WaypointInformation temp_commuteTask;
		CDatumPoint m_CDatumPoint;

		//转化存在问题
		for (int j = 0; j < tempcommissionType2Post.stc_fplist[i].target_class_id.size(); j++)
		{
			temp_commuteTask.target_class_id.push_back(tempcommissionType2Post.stc_fplist[i].target_class_id[j]);
			//temp_poi += string::number(temp_commuteTask.targets[j]);
			//temp_poi += "  ";
		}

		// CruiseInstance temp_CruiseInstance;
		// string url = temp_CruiseInstance.read_url_xml("MAXSPEED");
		string url = "10";
		temp_commuteTask.maxFlightSpeed = stof(url);
		//绝缘子路点减速
		if (tempcommissionType2Post.stc_fplist[i].target_class_id[0] == 0) {
			url = 2;
			temp_commuteTask.maxFlightSpeed = stof(url);
		}
		temp_commuteTask.stc_x = tempcommissionType2Post.stc_fplist[i].stc_gps.stc_lon;
		temp_commuteTask.stc_y = tempcommissionType2Post.stc_fplist[i].stc_gps.stc_lat;
		temp_commuteTask.stc_z = tempcommissionType2Post.stc_fplist[i].stc_gps.stc_alt;

		temp_commuteTask.stc_yaw = tempcommissionType2Post.stc_fplist[i].stc_yaw;
		temp_commuteTask.stc_gimbalPitch = tempcommissionType2Post.stc_fplist[i].stc_gimbalPitch;







		//////////////////////////////////////////////////////////////////////kevin///////////////////////////////////

		//其他点
		m_CDatumPoint.setOrigin(tempcommissionType2Post.stc_fplist[i].stc_gps.stc_lat, tempcommissionType2Post.stc_fplist[i].stc_gps.stc_lon, tempcommissionType2Post.stc_fplist[i].stc_gps.stc_alt, true);
		Coordinate aaaa;
		aaaa.stc_x = 30 * sin(ang2rad(temp_commuteTask.stc_yaw));
		aaaa.stc_y = 30 * cos(ang2rad(temp_commuteTask.stc_yaw));
		aaaa.stc_z = 30 * tan(ang2rad(temp_commuteTask.stc_gimbalPitch));

		LongitudeAndLatitude daaaa = m_CDatumPoint.DatumPointToLongitudeAndLatitude(aaaa);
		double calculate_interest_point_lon = daaaa.stc_longitude;
		double calculate_interest_point_lat = daaaa.stc_latitude;

		//double calculate_interest_point_lon = tempcommissionType2Post.stc_fplist[1].stc_gps.stc_lon;
		//double calculate_interest_point_lat = tempcommissionType2Post.stc_fplist[1].stc_gps.stc_lat;
		double calculate_interest_point_height;
		if (tempcommissionType2Post.stc_fplist[i].stc_gimbalPitch == 0) {
			calculate_interest_point_height = tempcommissionType2Post.stc_fplist[i].stc_gps.stc_alt;
		}
		else {
			calculate_interest_point_height = tempcommissionType2Post.stc_fplist[i].stc_gps.stc_alt + aaaa.stc_z;
		}

		//通道巡检，右侧以最后的点作为interest point
		if (tempcommissionType2Post.stc_fplist[i].stc_gimbalPitch == 3) {
			//calculate_interest_point_lon = tempcommissionType2Post.stc_fplist[1].stc_gps.stc_lon + (tempcommissionType2Post.stc_fplist[0].stc_gps.stc_lon - tempcommissionType2Post.stc_fplist[1].stc_gps.stc_lon) * 3;
			//calculate_interest_point_lat = tempcommissionType2Post.stc_fplist[1].stc_gps.stc_lat + (tempcommissionType2Post.stc_fplist[0].stc_gps.stc_lat - tempcommissionType2Post.stc_fplist[1].stc_gps.stc_lat) * 3;
			aaaa.stc_x = 300 * sin(ang2rad(temp_commuteTask.stc_yaw));
			aaaa.stc_y = 300 * cos(ang2rad(temp_commuteTask.stc_yaw));
			aaaa.stc_z = 300 * sin(ang2rad(temp_commuteTask.stc_gimbalPitch));

			daaaa = m_CDatumPoint.DatumPointToLongitudeAndLatitude(aaaa);
			calculate_interest_point_lon = daaaa.stc_longitude;
			calculate_interest_point_lat = daaaa.stc_latitude;
			calculate_interest_point_height = tempcommissionType2Post.stc_fplist[i].stc_gps.stc_alt;
		}
		//通道巡检，左侧以第一个点作为interest point
		if (tempcommissionType2Post.stc_fplist[i].stc_gimbalPitch == 4) {
			//calculate_interest_point_lon = tempcommissionType2Post.stc_fplist[1].stc_gps.stc_lon + (tempcommissionType2Post.stc_fplist[tempcommissionType2Post.stc_fplist.size() - 1].stc_gps.stc_lon - tempcommissionType2Post.stc_fplist[1].stc_gps.stc_lon) * 3;
			//calculate_interest_point_lat = tempcommissionType2Post.stc_fplist[1].stc_gps.stc_lat + (tempcommissionType2Post.stc_fplist[tempcommissionType2Post.stc_fplist.size() - 1].stc_gps.stc_lat - tempcommissionType2Post.stc_fplist[1].stc_gps.stc_lat) * 3;

			aaaa.stc_x = 300 * sin(ang2rad(temp_commuteTask.stc_yaw));
			aaaa.stc_y = 300 * cos(ang2rad(temp_commuteTask.stc_yaw));
			aaaa.stc_z = 300 * sin(ang2rad(temp_commuteTask.stc_gimbalPitch));

			daaaa = m_CDatumPoint.DatumPointToLongitudeAndLatitude(aaaa);
			calculate_interest_point_lon = daaaa.stc_longitude;
			calculate_interest_point_lat = daaaa.stc_latitude;
			calculate_interest_point_height = tempcommissionType2Post.stc_fplist[i].stc_gps.stc_alt;
		}
		//正上方，最后一个点的方向为interest point
		if (tempcommissionType2Post.stc_fplist[i].stc_gimbalPitch == -90) {
			//calculate_interest_point_lon = tempcommissionType2Post.stc_fplist[1].stc_gps.stc_lon + (tempcommissionType2Post.stc_fplist[tempcommissionType2Post.stc_fplist.size() - 1].stc_gps.stc_lon - tempcommissionType2Post.stc_fplist[1].stc_gps.stc_lon) * 50;
			//calculate_interest_point_lat = tempcommissionType2Post.stc_fplist[1].stc_gps.stc_lat + (tempcommissionType2Post.stc_fplist[tempcommissionType2Post.stc_fplist.size() - 1].stc_gps.stc_lat - tempcommissionType2Post.stc_fplist[1].stc_gps.stc_lat) * 50;

			aaaa.stc_x = 3000 * sin(ang2rad(temp_commuteTask.stc_yaw));
			aaaa.stc_y = 3000 * cos(ang2rad(temp_commuteTask.stc_yaw));
			aaaa.stc_z = sin(ang2rad(temp_commuteTask.stc_gimbalPitch));

			daaaa = m_CDatumPoint.DatumPointToLongitudeAndLatitude(aaaa);
			calculate_interest_point_lon = daaaa.stc_longitude;
			calculate_interest_point_lat = daaaa.stc_latitude;

			calculate_interest_point_height = tempcommissionType2Post.stc_fplist[i].stc_gps.stc_alt - 30;
		}


		LongitudeAndLatitude calculate_interest_point;
		calculate_interest_point.stc_altitude = calculate_interest_point_height;
		calculate_interest_point.stc_latitude = calculate_interest_point_lat;
		calculate_interest_point.stc_longitude = calculate_interest_point_lon;
		m_CDatumPoint.setOrigin(tempcommissionType2Post.stc_fplist[i].stc_gps.stc_lat, tempcommissionType2Post.stc_fplist[i].stc_gps.stc_lon, tempcommissionType2Post.stc_fplist[i].stc_gps.stc_alt, true);
		////latitude can calculate y
		temp_commuteTask.POI_Y = m_CDatumPoint.longitudeAndLatitudeToDatumPoint(calculate_interest_point).stc_y;
		////longitude can calculate x
		temp_commuteTask.POI_X = m_CDatumPoint.longitudeAndLatitudeToDatumPoint(calculate_interest_point).stc_x;
		////relativeHeight can calculate z
		temp_commuteTask.POI_Z = m_CDatumPoint.longitudeAndLatitudeToDatumPoint(calculate_interest_point).stc_z;
		if (tempcommissionType2Post.stc_fplist[i].stc_shootPhoto == 0) {
			temp_commuteTask.POI_Z = 1;
		}
		vector_commute_task.push_back(temp_commuteTask);
	}

	return vector_commute_task;
}


std::vector<WaypointInformation> PointToLineV3::pointToLine_minor(RecPoint input, uint32_t photoflag)
{
	float front_angle = 90-input.left_angle;
	float left_anlge = 90-input.right_angle;

	DatumPoint delta_lon_lat_up;
	delta_lon_lat_up.stc_x = 0.01;
	delta_lon_lat_up.stc_y = 0.01;
	delta_lon_lat_up.stc_z = 0.01;
	LongitudeAndLatitude temp;
	temp.stc_longitude = input.prev_pt.stc_x - input.rec_pt[0].stc_x;
	temp.stc_latitude = input.prev_pt.stc_y - input.rec_pt[0].stc_y;
	temp.stc_altitude = input.prev_pt.stc_z - input.rec_pt[0].stc_z;
	if (input.prev_pt.stc_x == 0) {
		temp.stc_longitude = 0;
		temp.stc_latitude = 0;
		temp.stc_altitude = 0;
	}
	DatumPoint delta_lon_lat_pre =  LongitudeAndLatitude_dev_to_distance_in_meter(temp, input.rec_pt[0].stc_y);
	temp.stc_longitude = input.next_pt.stc_x - input.rec_pt[0].stc_x;
	temp.stc_latitude = input.next_pt.stc_y - input.rec_pt[0].stc_y;
	temp.stc_altitude = input.next_pt.stc_z - input.rec_pt[0].stc_z;
	if (input.next_pt.stc_x == 0) {
		temp.stc_longitude = 0;
		temp.stc_latitude = 0;
		temp.stc_altitude = 0;
	}
	DatumPoint delta_lon_lat_next = LongitudeAndLatitude_dev_to_distance_in_meter(temp, input.rec_pt[0].stc_y);

	vector<WaypointInformation> line_out;
	line_out.clear();
	float front_distance = 30;
	float basic_point_height = 30;
	float distance = 15;

	basic_point_height = input.height;
	if (basic_point_height > 5) {

		LOG(ERROR) << "front_angle : " << front_angle;
		LOG(ERROR) << "left_angle : " << left_anlge;

		front_distance = tan(front_angle *3.1415926 / 180) * input.height;
		distance = tan(left_anlge *3.1415926 / 180) * input.height;
		LOG(ERROR) << "front_distance : " << front_distance;
		LOG(ERROR) << "distance : " << distance;
	}

	DatumPoint initial_line_up_point = delta_lon_lat_up;
	DatumPoint initial_line_prev_point = delta_lon_lat_pre;
	DatumPoint initial_line_next_point = delta_lon_lat_next;
	bool photo_shot_position_status_1 = false;
	bool photo_shot_position_status_2 = false;
	bool photo_shot_position_status_3 = false;
	bool photo_shot_position_status_4 = false;
	bool photo_shot_position_status_5 = false;
	//上
	if (photoflag & 0x1)
	{
		photo_shot_position_status_1 = true;
		LOG(ERROR) << "behind flag ";
	}
	//下
	if (photoflag & 0x2)
	{
		photo_shot_position_status_2 = true;
		LOG(ERROR) << "front flag ";
	}
	//左
	if (photoflag & 0x4)
	{
		photo_shot_position_status_3 = true;
		LOG(ERROR) << "left flag ";
	}
	//右
	if (photoflag & 0x8)
	{
		photo_shot_position_status_4 = true;
		LOG(ERROR) << "right flag ";
	}
	//中
	if (photoflag & 0x10)
	{
		photo_shot_position_status_5 = true;
		LOG(ERROR) << "up flag ";
	}


	if (pow(initial_line_prev_point.stc_x, 2) + pow(initial_line_prev_point.stc_y, 2) <= 1 && pow(initial_line_next_point.stc_x, 2) + pow(initial_line_next_point.stc_y, 2) <= 1) {
		//目前暂定 当第一个点的index为-1时，则为不符合标准----第二第三个点高度差不合理
		vector<WaypointInformation> buffer;
		WaypointInformation buffer_first_data;
		buffer_first_data.stc_index = -1;
		buffer.push_back(buffer_first_data);
		return buffer;
	}

	vector<double> d_forward = get_unit_directon2d(initial_line_next_point.stc_x, initial_line_next_point.stc_y, initial_line_up_point.stc_x, initial_line_up_point.stc_y);
	vector<double> d_backward = get_unit_directon2d(initial_line_prev_point.stc_x, initial_line_prev_point.stc_y, initial_line_up_point.stc_x, initial_line_up_point.stc_y);
	vector<double> d_right = matrixrotate2d(d_forward, 90);
	vector<double> d_left = matrixrotate2d(d_forward, -90);
	if (pow(initial_line_prev_point.stc_x, 2) + pow(initial_line_prev_point.stc_y, 2) <= 1) {
		d_forward = get_unit_directon2d(initial_line_next_point.stc_x, initial_line_next_point.stc_y, initial_line_up_point.stc_x, initial_line_up_point.stc_y);
		d_backward = matrixrotate2d(d_forward, 180);
		d_right = matrixrotate2d(d_forward, 90);
		d_left = matrixrotate2d(d_forward, -90);
	}
	if (pow(initial_line_next_point.stc_x, 2) + pow(initial_line_next_point.stc_y, 2) <= 1) {
		d_backward = get_unit_directon2d(initial_line_prev_point.stc_x, initial_line_prev_point.stc_y, initial_line_up_point.stc_x, initial_line_up_point.stc_y);
		d_forward = matrixrotate2d(d_backward, 180);
		d_right = matrixrotate2d(d_backward, 90);
		d_left = matrixrotate2d(d_backward, -90);
	}
	if (pow(initial_line_prev_point.stc_x, 2) + pow(initial_line_prev_point.stc_y, 2) > 1 && pow(initial_line_next_point.stc_x, 2) + pow(initial_line_next_point.stc_y, 2) > 1) {
		d_forward = get_unit_directon2d(initial_line_next_point.stc_x, initial_line_next_point.stc_y, initial_line_up_point.stc_x, initial_line_up_point.stc_y);
		d_backward = get_unit_directon2d(initial_line_prev_point.stc_x, initial_line_prev_point.stc_y, initial_line_up_point.stc_x, initial_line_up_point.stc_y);
		d_right = matrixrotate2d(d_forward, 90);
		d_left = matrixrotate2d(d_backward, 90);
	}


	WaypointInformation point_0;
	if (photo_shot_position_status_2 == true && photo_shot_position_status_5 == true) {
		//正前方路点
		point_0.stc_x = -d_backward[0] * front_distance;
		point_0.stc_y = -d_backward[1] * front_distance;
		point_0.stc_z = initial_line_up_point.stc_z + basic_point_height;
		point_0.stc_index = line_out.size() + 1;
		LOG(ERROR) << "d_backward[0] : " << d_backward[0];
		LOG(ERROR) << "d_backward[1] : " << d_backward[1];
		point_0.stc_yaw = get_angle_tonorth(d_backward[0], d_backward[1]);
		LOG(ERROR) << "point_0.stc_yaw : " << point_0.stc_yaw;
		point_0.stc_gimbalPitch = atan(-basic_point_height / sqrt(pow(point_0.stc_x, 2) + pow(point_0.stc_y, 2))) * 180 / 3.1415926;
		LOG(ERROR) << "point_0.stc_gimbalPitch : " << point_0.stc_gimbalPitch;
		point_0.stc_shootPhoto = 1;
		point_0.zoom_Magnification = 2;
		std::vector<uint8_t> temp_target_class_id;
		temp_target_class_id.push_back(21);
		point_0.target_class_id = temp_target_class_id;
		point_0.orient_to_tower = 0;
		
		point_0.Number_of_layers_of_current_waypoint = -1;
		point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
		point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
		point_0.POI_Z = -10;
		point_0.waypointType = TASKTYPEDISTRIBUTIONROUTE;
		point_0.necessary = false;
		point_0.zoom_Magnification = input.zoom;
		line_out.push_back(point_0);
	}

	if (photo_shot_position_status_3 == true && photo_shot_position_status_5 == true) {
		//左方路点
		point_0.stc_x = d_left[0] * distance;
		point_0.stc_y = d_left[1] * distance;
		point_0.stc_z = initial_line_up_point.stc_z + basic_point_height;
		point_0.stc_index = line_out.size() + 1;
		point_0.stc_yaw = get_angle_tonorth(-d_left[0], -d_left[1]);
		point_0.stc_gimbalPitch = atan(-basic_point_height / sqrt(pow(point_0.stc_x, 2) + pow(point_0.stc_y, 2))) * 180 / 3.1415926;
		point_0.stc_shootPhoto = 1;
		point_0.zoom_Magnification = 2;
		std::vector<uint8_t> temp_target_class_id;
		temp_target_class_id.push_back(21);
		point_0.target_class_id = temp_target_class_id;
		point_0.orient_to_tower = 1;
		
		point_0.Number_of_layers_of_current_waypoint = -1;
		point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
		point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
		point_0.POI_Z = -10;
		point_0.waypointType = TASKTYPEDISTRIBUTIONROUTE;
		point_0.necessary = false;
		point_0.zoom_Magnification = input.zoom;
		line_out.push_back(point_0);
	}


	//正上方路点
	point_0.stc_x = initial_line_up_point.stc_x;
	point_0.stc_y = initial_line_up_point.stc_y;
	point_0.stc_z = initial_line_up_point.stc_z + basic_point_height;
	point_0.stc_index = line_out.size() + 1;
	point_0.stc_yaw = get_angle_tonorth(-d_forward[0], -d_forward[1]);
	point_0.stc_gimbalPitch = -90;
	if (photo_shot_position_status_5) {
		point_0.stc_shootPhoto = 1;
	}
	else {
		point_0.stc_shootPhoto = 0;
	}
	point_0.zoom_Magnification = 2;
	point_0.necessary = true;
	std::vector<uint8_t> temp_target_class_id;
	temp_target_class_id.push_back(21);
	point_0.target_class_id = temp_target_class_id;
	point_0.orient_to_tower = 2;
	
	point_0.Number_of_layers_of_current_waypoint = -1;
	point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
	point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
	point_0.POI_Z = -10;
	point_0.waypointType = TASKTYPEDISTRIBUTIONROUTE;
	point_0.zoom_Magnification = input.zoom;
	line_out.push_back(point_0);


	if (photo_shot_position_status_4 == true && photo_shot_position_status_5 == true) {
		//右方路点
		point_0.stc_x = d_right[0] * distance;
		point_0.stc_y = d_right[1] * distance;
		point_0.stc_z = initial_line_up_point.stc_z + basic_point_height;
		point_0.stc_index = line_out.size() + 1;
		point_0.stc_yaw = get_angle_tonorth(-d_right[0], -d_right[1]);
		point_0.stc_gimbalPitch = atan(-basic_point_height / sqrt(pow(point_0.stc_x, 2) + pow(point_0.stc_y, 2))) * 180 / 3.1415926;
		point_0.stc_shootPhoto = 1;
		point_0.zoom_Magnification = 2;
		std::vector<uint8_t> temp_target_class_id;
		temp_target_class_id.push_back(21);
		point_0.target_class_id = temp_target_class_id;
		point_0.orient_to_tower = 3;
		
		point_0.Number_of_layers_of_current_waypoint = -1;
		point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
		point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
		point_0.POI_Z = -10;
		point_0.waypointType = TASKTYPEDISTRIBUTIONROUTE;
		point_0.necessary = false;
		point_0.zoom_Magnification = input.zoom;
		line_out.push_back(point_0);
	}

	if (photo_shot_position_status_1 == true && photo_shot_position_status_5 == true) {
		//后方路点
		
		point_0.stc_x = -d_forward[0] * front_distance;
		point_0.stc_y = -d_forward[1] * front_distance;
		point_0.stc_z = initial_line_up_point.stc_z + basic_point_height;
		point_0.stc_index = line_out.size() + 1;
		point_0.stc_yaw = get_angle_tonorth(d_forward[0], d_forward[1]);
		point_0.stc_gimbalPitch = atan(-basic_point_height / sqrt(pow(point_0.stc_x, 2) + pow(point_0.stc_y, 2))) * 180 / 3.1415926;
		point_0.stc_shootPhoto = 1;
		point_0.zoom_Magnification = 2;
		std::vector<uint8_t> temp_target_class_id;
		temp_target_class_id.push_back(21);
		point_0.target_class_id = temp_target_class_id;
		point_0.orient_to_tower = 4;
		point_0.Number_of_layers_of_current_waypoint = -1;
		point_0.POI_X = initial_line_up_point.stc_x - point_0.stc_x;
		point_0.POI_Y = initial_line_up_point.stc_y - point_0.stc_y;
		point_0.POI_Z = -10;
		point_0.waypointType = TASKTYPEDISTRIBUTIONROUTE;
		point_0.necessary = false;
		point_0.zoom_Magnification = input.zoom;
		line_out.push_back(point_0);
	}

	for (auto iter = line_out.begin(); iter != line_out.end(); iter++) {
		Coordinate line_out_inxyz;
		line_out_inxyz.stc_x = iter->stc_x;
		line_out_inxyz.stc_y = iter->stc_y;
		line_out_inxyz.stc_z = iter->stc_z;
		LongitudeAndLatitude line_out_lonlatalt = distance_in_meter_to_LongitudeAndLatitude_dev(line_out_inxyz, input.rec_pt[0].stc_y);
		iter->stc_x = line_out_lonlatalt.stc_longitude + input.rec_pt[0].stc_x;
		iter->stc_y = line_out_lonlatalt.stc_latitude + input.rec_pt[0].stc_y;
		iter->stc_z = line_out_lonlatalt.stc_altitude + input.rec_pt[0].stc_z;
	}

	return line_out;
}


//距离差转经纬度差
LongitudeAndLatitude PointToLineV3::distance_in_meter_to_LongitudeAndLatitude_dev(Coordinate temp, double m_GPSY1)
{
	LongitudeAndLatitude buffer;
	buffer.stc_latitude = temp.stc_y / 111192.4;
	buffer.stc_longitude = temp.stc_x / (111192.4 * cos(m_GPSY1 / 180 * 3.1415926));
	buffer.stc_altitude = temp.stc_z;
	return buffer;
}
//经纬度差转距离差
DatumPoint PointToLineV3::LongitudeAndLatitude_dev_to_distance_in_meter(LongitudeAndLatitude temp, double latitude)
{
	DatumPoint buffer;
	buffer.stc_y = (temp.stc_latitude) * 111192.4;
	buffer.stc_x = (temp.stc_longitude) * (111192.4 * cos(latitude / 180 * 3.1415926));
	buffer.stc_z = temp.stc_altitude;
	return buffer;
}