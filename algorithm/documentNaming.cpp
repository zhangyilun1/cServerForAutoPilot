/*#include "documentNaming.h"

DocumentNaming::DocumentNaming()
{
}

DocumentNaming::~DocumentNaming()
{
}
QString DocumentNaming::get_photoname_according_route(WaypointInformation waypoint_info,float left_right_zoom_threshold_in) {
	QString tower_chinese_name;
	QString error = QString::fromLocal8Bit("请输入中间倍率");
	float left_right_zoom_threshold = left_right_zoom_threshold_in;
	if (left_right_zoom_threshold < 2 || left_right_zoom_threshold>20) {
		return error;
	}
	float tj_left_right_zoom_threshold = left_right_zoom_threshold * 1.5;
	float left_right_zoom_threshold_jj = left_right_zoom_threshold * 2;
	int target_class_id = waypoint_info.detection_box.obj_class;
	//如果targetclassid = 杆塔正视
	if (target_class_id == 1) {
		tower_chinese_name = get_photoname_tszs( waypoint_info);
	}
	//如果targetclassid = 塔尖横条
	else if (target_class_id == 4) {
		tower_chinese_name = get_photoname_jdht(waypoint_info, tj_left_right_zoom_threshold);
	}
	//如果targetclassid = 绝缘子
	else if (target_class_id == 0) {
		tower_chinese_name = get_photoname_jyz(waypoint_info, left_right_zoom_threshold);
	}
	//如果targetclassid = 99绝缘子金具
	else if (target_class_id == 99) {
		tower_chinese_name = get_photoname_jyz(waypoint_info, left_right_zoom_threshold_jj);
		tower_chinese_name += QString::fromLocal8Bit("金具");
	}
	//如果targetclassid = 塔身俯视
	else if (target_class_id == 2) {
		tower_chinese_name = QString::fromLocal8Bit("俯视图-杆塔顶部");
	}
	///////////////////////98侧面通道
	else if (target_class_id == 98) {
		tower_chinese_name = get_photoname_td(waypoint_info);
	}
	//其他
	else {
		tower_chinese_name = QString::fromLocal8Bit("未知");
	}
	return tower_chinese_name;
}


QString DocumentNaming::get_photoname_tszs(WaypointInformation waypoint_info) {
	QString tower_chinese_name;
	if (waypoint_info.orient_to_tower == 0) {
		tower_chinese_name = QString::fromLocal8Bit("大号侧-杆塔整体");
	}
	if (waypoint_info.orient_to_tower == 0) {
		tower_chinese_name = QString::fromLocal8Bit("小号侧-杆塔整体");
	}
	return tower_chinese_name;
}

QString DocumentNaming::get_photoname_jdht(WaypointInformation waypoint_info, float tj_left_right_zoom_threshold) {
	QString tower_chinese_name;
	//无人机在左侧，lr=0
	if (waypoint_info.orient_to_tower == 0) {
		if (waypoint_info.zoom_Magnification >= tj_left_right_zoom_threshold) {
			tower_chinese_name = QString::fromLocal8Bit("右侧-地线金具");
		}
		else {
			tower_chinese_name = QString::fromLocal8Bit("左侧-地线金具");
		}
	}
	if (waypoint_info.orient_to_tower == 0) {
		if (waypoint_info.zoom_Magnification >= tj_left_right_zoom_threshold) {
			tower_chinese_name = QString::fromLocal8Bit("左侧-地线金具");
		}
		else {
			tower_chinese_name = QString::fromLocal8Bit("右侧-地线金具");
		}
	}
	return tower_chinese_name;
}

QString DocumentNaming::get_photoname_td(WaypointInformation waypoint_info) {
	QString tower_chinese_name;
	if (waypoint_info.orient_to_tower == 0) {
		tower_chinese_name = QString::fromLocal8Bit("小号测-通道");
	}
	if (waypoint_info.orient_to_tower == 0) {
		tower_chinese_name = QString::fromLocal8Bit("大号测-通道");
	}
	return tower_chinese_name;
}

QString DocumentNaming::get_photoname_jyz(WaypointInformation waypoint_info,float left_right_zoom_threshold_jj) {
	QString tower_chinese_name;
	//无人机在左侧，lr=0,前侧fb=0--》大号侧-
	if (waypoint_info.orient_to_tower == 0 ) {
		if (waypoint_info.zoom_Magnification >= left_right_zoom_threshold_jj) {
			tower_chinese_name = QString::fromLocal8Bit("大号测-右侧-");
		}
		else if (waypoint_info.zoom_Magnification < left_right_zoom_threshold_jj) {
			tower_chinese_name = QString::fromLocal8Bit("大号测-左侧-");
		}
	}
	//无人机在左侧，lr=0,后侧fb=1--》小号侧-
	if (waypoint_info.orient_to_tower == 0 ) {
		if (waypoint_info.zoom_Magnification >= left_right_zoom_threshold_jj) {
			tower_chinese_name = QString::fromLocal8Bit("小号侧-右侧-");
		}
		else if (waypoint_info.zoom_Magnification < left_right_zoom_threshold_jj) {
			tower_chinese_name = QString::fromLocal8Bit("小号侧-左侧-");
		}
	}
	if (waypoint_info.orient_to_tower == 0) {
		if (waypoint_info.zoom_Magnification >= left_right_zoom_threshold_jj) {
			tower_chinese_name = QString::fromLocal8Bit("大号测-左侧-");
		}
		else if (waypoint_info.zoom_Magnification < left_right_zoom_threshold_jj) {
			tower_chinese_name = QString::fromLocal8Bit("大号测-右侧-");
		}
	}
	if (waypoint_info.orient_to_tower == 0) {
		if (waypoint_info.zoom_Magnification >= left_right_zoom_threshold_jj) {
			tower_chinese_name = QString::fromLocal8Bit("小号测-左侧-");
		}
		else if (waypoint_info.zoom_Magnification < left_right_zoom_threshold_jj) {
			tower_chinese_name = QString::fromLocal8Bit("小号测-右侧-");
		}
	}
	////////////////////////////////目标名称//////////////////////////////////////////////
	if (waypoint_info.Number_of_layers_of_current_waypoint <= 1) {
		tower_chinese_name += QString::fromLocal8Bit("第一层-绝缘子");
	}
	else if (waypoint_info.Number_of_layers_of_current_waypoint > 1 && waypoint_info.Number_of_layers_of_current_waypoint <= 2) {
		if (waypoint_info.stc_gimbalPitch > 0) {
			tower_chinese_name += QString::fromLocal8Bit("第一层-绝缘子");
		}
		else {
			tower_chinese_name += QString::fromLocal8Bit("第二层-绝缘子");
		}
	}
	else if (waypoint_info.Number_of_layers_of_current_waypoint > 2 && waypoint_info.Number_of_layers_of_current_waypoint <= 3) {
		if (waypoint_info.stc_gimbalPitch > 0) {
			tower_chinese_name += QString::fromLocal8Bit("第二层-绝缘子");
		}
		else {
			tower_chinese_name += QString::fromLocal8Bit("第三层-绝缘子");
		}
	}
	else if (waypoint_info.Number_of_layers_of_current_waypoint > 3 && waypoint_info.Number_of_layers_of_current_waypoint <= 4) {
		if (waypoint_info.stc_gimbalPitch > 0) {
			tower_chinese_name += QString::fromLocal8Bit("第三层-绝缘子");
		}
		else {
			tower_chinese_name += QString::fromLocal8Bit("第四层-绝缘子");
		}
	}
	else if (waypoint_info.Number_of_layers_of_current_waypoint > 4 && waypoint_info.Number_of_layers_of_current_waypoint <= 5) {
		if (waypoint_info.stc_gimbalPitch > 0) {
			tower_chinese_name += QString::fromLocal8Bit("第四层-绝缘子");
		}
		else {
			tower_chinese_name += QString::fromLocal8Bit("第五层-绝缘子");
		}
	}
	else if (waypoint_info.Number_of_layers_of_current_waypoint > 5) {
		tower_chinese_name += QString::fromLocal8Bit("底层-绝缘子");
	}
	return tower_chinese_name;
}*/