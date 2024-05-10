#include "jsonData.h"
using namespace std;
using json = nlohmann::json;
JsonData::JsonData(/* args */)
{
}

JsonData::~JsonData()
{
}


bool JsonData::waypoint2JSONSplitByTowerID(const vector<WaypointInformation>& vec, string& wholejson,
                                        vector<int>& toweridvec, vector<string>& towerstrvec)
{
    //TRACE("=== waypoint2JSONSplitByTowerID ===");
    json jsonObject;
    json waypointArray = json::array();
    std::vector<json> towerwaypointarray;
    for (const WaypointInformation& waypointInfo : vec) {
        // 将WaypointInformation信息填充到json对象中，根据实际情况填写
        json detection_box;
        detection_box["index"] = waypointInfo.detection_box.index;
        detection_box["obj_class"] = waypointInfo.detection_box.obj_class;
        detection_box["xmin"] = waypointInfo.detection_box.xmin;
        detection_box["ymin"] = waypointInfo.detection_box.ymin;
        detection_box["xmax"] = waypointInfo.detection_box.xmax;
        detection_box["ymax"] = waypointInfo.detection_box.ymax;
        detection_box["width"] = waypointInfo.detection_box.width;
        detection_box["height"] = waypointInfo.detection_box.height;
        detection_box["xcent"] = waypointInfo.detection_box.xcent;
        detection_box["ycent"] = waypointInfo.detection_box.ycent;
        detection_box["prob"] = waypointInfo.detection_box.prob;
        detection_box["v_width"] = waypointInfo.detection_box.v_width;
        detection_box["v_height"] = waypointInfo.detection_box.v_height;
        detection_box["v_xcent"] = waypointInfo.detection_box.v_xcent;
        detection_box["v_ycent"] = waypointInfo.detection_box.v_ycent;
        detection_box["last_v_width"] = waypointInfo.detection_box.last_v_width;
        detection_box["last_v_height"] = waypointInfo.detection_box.last_v_height;
        detection_box["last_v_xcent"] = waypointInfo.detection_box.last_v_xcent;
        detection_box["last_v_ycent"] = waypointInfo.detection_box.last_v_ycent;
        detection_box["frame_count"] = waypointInfo.detection_box.frame_count;
        json classProbArray = json::array();
        for (float prob : waypointInfo.detection_box.class_prob) {
            classProbArray.push_back(prob);
        }
		detection_box["class_prob"] = classProbArray;
      
        // QJsonObject bb;
		// bb.insert("index", (int)vec[i].detection_box.index);
		// bb.insert("obj_class", vec[i].detection_box.obj_class);
		// bb.insert("xmin", vec[i].detection_box.xmin);
		// bb.insert("ymin", vec[i].detection_box.ymin);
		// bb.insert("xmax", vec[i].detection_box.xmax);
		// bb.insert("ymax", vec[i].detection_box.ymax);
		// bb.insert("width", vec[i].detection_box.width);
		// bb.insert("height", vec[i].detection_box.height);
		// bb.insert("xcent", vec[i].detection_box.xcent);
		// bb.insert("ycent", vec[i].detection_box.ycent);
		// bb.insert("prob", vec[i].detection_box.prob);
		// bb.insert("v_width", vec[i].detection_box.v_width);
		// bb.insert("v_height", vec[i].detection_box.v_height);
		// bb.insert("v_xcent", vec[i].detection_box.v_xcent);
		// bb.insert("v_ycent", vec[i].detection_box.v_ycent);
		// bb.insert("last_v_width", vec[i].detection_box.last_v_width);
		// bb.insert("last_v_height", vec[i].detection_box.last_v_height);
		// bb.insert("last_v_xcent", vec[i].detection_box.last_v_xcent);
		// bb.insert("last_v_ycent", vec[i].detection_box.last_v_ycent);
		// bb.insert("frame_count", vec[i].detection_box.frame_count);
		// QJsonArray class_prob_array;
		// for (int j = 0; j < vec[i].detection_box.class_prob.size(); j++)
		// {
		// 	class_prob_array.append(vec[i].detection_box.class_prob[j]);
		// }
		// bb.insert("class_prob", class_prob_array);


        json datumPointArray = json::array();
        for (const Gps& gps : waypointInfo.Datum_point) {
            json gpsJsonObject;
            gpsJsonObject["stc_lat"] = gps.stc_lat;
            gpsJsonObject["stc_lon"] = gps.stc_lon;
            gpsJsonObject["stc_alt"] = gps.stc_alt;
            datumPointArray.push_back(gpsJsonObject);
        }
        // QJsonArray Datum_point_array;
		// for (int j = 0; j < vec[i].Datum_point.size(); j++)
		// {
		// 	QJsonObject obj_gps;
		// 	obj_gps.insert("Datum_point_lat", vec[i].Datum_point[j].stc_lat);
		// 	obj_gps.insert("Datum_point_lon", vec[i].Datum_point[j].stc_lon);
		// 	obj_gps.insert("Datum_point_alt", vec[i].Datum_point[j].stc_alt);
		// 	Datum_point_array.append(obj_gps);
		// }


        json targetClassIdArray = json::array();
        for (uint8_t value : waypointInfo.target_class_id) {
            targetClassIdArray.push_back(value);
        }
		// QJsonArray target_class_id_array;
		// for (int j = 0; j < vec[i].target_class_id.size(); j++)
		// {
		// 	target_class_id_array.append(vec[i].target_class_id[j]);
		// }


        json waypoint;
        waypoint["lon"] = waypointInfo.stc_x;
        waypoint["lat"] = waypointInfo.stc_y;
        waypoint["alt"] = waypointInfo.stc_z;
        waypoint["index"] = waypointInfo.stc_index;
        waypoint["yaw"] = waypointInfo.stc_yaw;
        waypoint["pitch"] = waypointInfo.stc_gimbalPitch;
        waypoint["shootPhoto"] = waypointInfo.stc_shootPhoto;
        waypoint["camera_type"] = waypointInfo.camera_type;
        waypoint["ISO"] = waypointInfo.ISO;
        //waypoint["opticalZoomFocalLength"] = waypointInfo.zoom_Magnification;
        waypoint["layers"] = waypointInfo.Number_of_layers_of_current_waypoint;
        waypoint["time"] = waypointInfo.Photo_time;
        waypoint["taskID"] = waypointInfo.task_id;
        waypoint["TowerID"] = waypointInfo.tower_id;
        waypoint["Magnification"] = waypointInfo.zoom_Magnification;
        waypoint["exposure_compensation"] = waypointInfo.expo_comp;
        waypoint["necessary"] = waypointInfo.necessary;
        waypoint["orient_to_tower"] = waypointInfo.orient_to_tower;
        waypoint["maxFlightSpeed"] = waypointInfo.maxFlightSpeed;
        waypoint["waypointType"] = waypointInfo.waypointType;
        waypoint["back_Type"] = waypointInfo.back_Type;
        waypoint["CONFIG_NO"] = waypointInfo.CONFIG_NO;
        waypoint["POI_X"] = waypointInfo.POI_X;
        waypoint["POI_Y"] = waypointInfo.POI_Y;
        waypoint["POI_Z"] = waypointInfo.POI_Z;
        waypoint["gimbal_roll"] = waypointInfo.gimbal_roll;
        waypoint["bb"] = detection_box;
        waypoint["Datum_point"] = datumPointArray;
        waypoint["target_class_id"] = targetClassIdArray;

		// QJsonObject temp;
		// temp.insert("lon", vec[i].stc_x);
		// temp.insert("lat", vec[i].stc_y);
		// temp.insert("alt", vec[i].stc_z);
		// temp.insert("index", vec[i].stc_index);
		// temp.insert("yaw", vec[i].stc_yaw);
		// temp.insert("pitch", vec[i].stc_gimbalPitch);
		// temp.insert("shootPhoto", vec[i].stc_shootPhoto);
		// temp.insert("camera_type", vec[i].camera_type);
		// temp.insert("ISO", vec[i].ISO);
		// //temp.insert("opticalZoomFocalLength", vec[i].zoom_Magnification);
		// temp.insert("layers", vec[i].Number_of_layers_of_current_waypoint);
		// temp.insert("time", QString::fromStdString(vec[i].Photo_time));
		// temp.insert("taskID", (int)vec[i].task_id);
		// temp.insert("TowerID", (int)vec[i].tower_id);
		// temp.insert("Magnification", vec[i].zoom_Magnification);
		// temp.insert("exposure_compensation", vec[i].expo_comp);
		// temp.insert("necessary", vec[i].necessary);
		// temp.insert("orient_to_tower", vec[i].orient_to_tower);
		// temp.insert("maxFlightSpeed", vec[i].maxFlightSpeed);
		// temp.insert("waypointType", vec[i].waypointType);
		// temp.insert("back_Type", vec[i].back_Type);
		// temp.insert("CONFIG_NO", vec[i].CONFIG_NO);
		// temp.insert("POI_X", vec[i].POI_X);
		// temp.insert("POI_Y", vec[i].POI_Y);
		// temp.insert("POI_Z", vec[i].POI_Z);
		// temp.insert("gimbal_roll", vec[i].gimbal_roll);
		// temp.insert("bb", bb);
		// temp.insert("Datum_point", Datum_point_array);
		// temp.insert("target_class_id", target_class_id_array);
        waypointArray.push_back(waypoint);
        auto iter = toweridvec.begin();
        bool bexist = false;
        int x = 0;
        for (; iter != toweridvec.end(); iter++) {
            if (*iter == waypointInfo.tower_id) {
                bexist = true;
                towerwaypointarray[x].push_back(waypoint);
                break;
            }
            x++;
        }
        if (!bexist) {
            toweridvec.push_back(waypointInfo.tower_id);
            json jatmp;
            jatmp.push_back(waypoint);
            towerwaypointarray.push_back(jatmp);
        }
    }
    jsonObject["WaypointInformation"] = waypointArray;
    wholejson = jsonObject.dump();
    int x = 0;
	auto iter = toweridvec.begin();
    for (; iter != toweridvec.end(); iter++) {
        json jsonObjectArr;
        jsonObjectArr["WaypointInformation"] = towerwaypointarray[x];
        towerstrvec.push_back(jsonObjectArr.dump());
        x++;
    }
    return true;
}


vector<WaypointInformation> JsonData::Converting_the_aircraft_waypoint_structure_into_a_structure(const string& jsonStr){
    std::vector<WaypointInformation> vec;
    //TRACE("=== Converting_the_aircraft_waypoint_structure_into_a_structure ===");
    try {
        json jsonData = json::parse(jsonStr);
        for (const auto& waypoint : jsonData["WaypointInformation"]) {
            WaypointInformation temp;
            temp.stc_x = waypoint["lon"].get<double>();
            temp.stc_y = waypoint["lat"].get<double>();
            temp.stc_z = waypoint["alt"].get<float>();
            temp.stc_index = waypoint["index"].get<uint16_t>();
            temp.stc_yaw = waypoint["yaw"].get<float>();
            temp.stc_gimbalPitch = waypoint["pitch"].get<float>();
            temp.stc_shootPhoto = waypoint["shootPhoto"].get<bool>();
            temp.camera_type = waypoint["camera_type"].get<int>();
            temp.ISO = waypoint["ISO"].get<int>();

            for (const auto& class_id : waypoint["target_class_id"]) {
                temp.target_class_id.push_back(class_id.get<uint8_t>());
            }

            temp.Number_of_layers_of_current_waypoint = waypoint["layers"].get<int>();
            temp.Photo_time = waypoint["time"].get<std::string>();
            temp.task_id = waypoint["taskID"].get<uint32_t>();
            temp.tower_id = waypoint["TowerID"].get<uint32_t>();

            for (const auto& gps : waypoint["Datum_point"]) {
                Gps temp_gps;
                temp_gps.stc_alt = gps["stc_lat"].get<double>();
                temp_gps.stc_lat = gps["stc_alt"].get<double>();
                temp_gps.stc_lon = gps["stc_lon"].get<double>();
                temp.Datum_point.push_back(temp_gps);
            }

            temp.zoom_Magnification = waypoint["Magnification"].get<float>();
            temp.expo_comp = waypoint["exposure_compensation"].get<int>();
            temp.shutter_speed = waypoint["shutter_speed"].get<int>();
            temp.necessary = waypoint["neccesarry"].get<bool>();
            temp.orient_to_tower = waypoint["orient_to_tower"].get<int>();

            const auto& detection_box = waypoint["bb"];
            temp.detection_box.frame_count = detection_box["frame_count"].get<int>();
            temp.detection_box.height = detection_box["height"].get<int>();
            temp.detection_box.index = detection_box["index"].get<uint32_t>();
            temp.detection_box.last_v_height = detection_box["last_v_height"].get<float>();
            temp.detection_box.last_v_width = detection_box["last_v_width"].get<float>();
            temp.detection_box.last_v_xcent = detection_box["last_v_xcent"].get<float>();
            temp.detection_box.last_v_ycent = detection_box["last_v_ycent"].get<float>();
            temp.detection_box.obj_class = detection_box["obj_class"].get<int>();
            temp.detection_box.prob = detection_box["prob"].get<float>();
            temp.detection_box.v_height = detection_box["v_height"].get<float>();
            temp.detection_box.v_width = detection_box["v_width"].get<float>();
            temp.detection_box.v_xcent = detection_box["v_xcent"].get<float>();
            temp.detection_box.v_ycent = detection_box["v_ycent"].get<float>();
            temp.detection_box.width = detection_box["width"].get<int>();
            temp.detection_box.xcent = detection_box["xcent"].get<int>();
            temp.detection_box.xmax = detection_box["xmax"].get<int>();
            temp.detection_box.xmin = detection_box["xmin"].get<int>();
            temp.detection_box.ycent = detection_box["ycent"].get<int>();
            temp.detection_box.ymax = detection_box["ymax"].get<int>();
            temp.detection_box.ymin = detection_box["ymin"].get<int>();

            for (const auto& prob : detection_box["class_prob"]) {
                temp.detection_box.class_prob.push_back(prob.get<float>());
            }

            temp.maxFlightSpeed = waypoint["maxFlightSpeed"].get<float>();
            temp.waypointType = waypoint["waypointType"].get<int>();
            temp.back_Type = waypoint["back_Type"].get<int>();
            temp.CONFIG_NO = waypoint["CONFIG_NO"].get<int>();
            temp.POI_X = waypoint["POI_X"].get<float>();
            temp.POI_Y = waypoint["POI_Y"].get<float>();
            temp.POI_Z = waypoint["POI_Z"].get<float>();
            temp.gimbal_roll = waypoint["gimbal_roll"].get<float>();

            vec.push_back(temp);
        }
    } catch (const std::exception& e) {
        //TRACE("Error parsing JSON : " + (string)e.what());
        // std::cerr << "Error parsing JSON: " << e.what() << std::endl;
    }
    return vec;
	// std::vector<WaypointInformation> vec;
	// QJsonParseError jsonError;
	// QJsonDocument doucment = QJsonDocument::fromJson(json, &jsonError);
	// if (!doucment.isNull() && (jsonError.error == QJsonParseError::NoError)) {
		
	// 	QJsonObject obj = doucment.object();
	// 	for (int i = 0; i < obj["WaypointInformation"].toArray().size(); i++)
	// 	{
	// 		WaypointInformation temp;
	// 		temp.stc_x = obj["WaypointInformation"].toArray().at(i).toObject()["lon"].toDouble();
	// 		temp.stc_y = obj["WaypointInformation"].toArray().at(i).toObject()["lat"].toDouble();
	// 		temp.stc_z = obj["WaypointInformation"].toArray().at(i).toObject()["alt"].toDouble();
	// 		temp.stc_index = obj["WaypointInformation"].toArray().at(i).toObject()["index"].toInt();
	// 		temp.stc_yaw = obj["WaypointInformation"].toArray().at(i).toObject()["yaw"].toDouble();
	// 		temp.stc_gimbalPitch = obj["WaypointInformation"].toArray().at(i).toObject()["pitch"].toDouble();
	// 		temp.stc_shootPhoto = obj["WaypointInformation"].toArray().at(i).toObject()["shootPhoto"].toBool();
	// 		temp.camera_type = obj["WaypointInformation"].toArray().at(i).toObject()["camera_type"].toInt();
	// 		temp.ISO = obj["WaypointInformation"].toArray().at(i).toObject()["ISO"].toInt();

	// 		for (int j = 0; j < obj["WaypointInformation"].toArray().at(i).toObject()["target_class_id"].toArray().size(); j++)
	// 		{
	// 			temp.target_class_id.push_back(obj["WaypointInformation"].toArray().at(i).toObject()["target_class_id"].toArray().at(j).toInt());
	// 			qDebug() << obj["WaypointInformation"].toArray().at(i).toObject()["target_class_id"].toArray().at(j).toInt();
	// 		}

	// 		temp.Number_of_layers_of_current_waypoint = obj["WaypointInformation"].toArray().at(i).toObject()["layers"].toInt();
	// 		temp.Photo_time = obj["WaypointInformation"].toArray().at(i).toObject()["time"].toString().toStdString();
	// 		temp.task_id = obj["WaypointInformation"].toArray().at(i).toObject()["taskID"].toInt();
	// 		temp.tower_id = obj["WaypointInformation"].toArray().at(i).toObject()["TowerID"].toInt();

	// 		Gps temp_gps;
	// 		for (int j = 0; j < obj["WaypointInformation"].toArray().at(i).toObject()["Datum_point"].toArray().size(); j++)
	// 		{
	// 			temp_gps.stc_alt = obj["WaypointInformation"].toArray().at(i).toObject()["Datum_point"].toArray().at(j).toObject()["Datum_point_alt"].toDouble();
	// 			temp_gps.stc_lat = obj["WaypointInformation"].toArray().at(i).toObject()["Datum_point"].toArray().at(j).toObject()["Datum_point_lat"].toDouble();
	// 			temp_gps.stc_lon = obj["WaypointInformation"].toArray().at(i).toObject()["Datum_point"].toArray().at(j).toObject()["Datum_point_lon"].toDouble();
	// 			temp.Datum_point.push_back(temp_gps);
	// 		}

	// 		temp.zoom_Magnification = obj["WaypointInformation"].toArray().at(i).toObject()["Magnification"].toDouble();
	// 		temp.expo_comp = obj["WaypointInformation"].toArray().at(i).toObject()["exposure_compensation"].toInt();
	// 		temp.shutter_speed = obj["WaypointInformation"].toArray().at(i).toObject()["shutter_speed"].toInt();
	// 		temp.necessary = obj["WaypointInformation"].toArray().at(i).toObject()["neccesarry"].toBool();
	// 		temp.orient_to_tower = obj["WaypointInformation"].toArray().at(i).toObject()["orient_to_tower"].toInt();

	// 		temp.detection_box.frame_count = obj["WaypointInformation"].toArray().at(i).toObject()["bb"].toObject()["frame_count"].toInt();
	// 		temp.detection_box.height = obj["WaypointInformation"].toArray().at(i).toObject()["bb"].toObject()["height"].toInt();
	// 		temp.detection_box.index = obj["WaypointInformation"].toArray().at(i).toObject()["bb"].toObject()["index"].toInt();
	// 		temp.detection_box.last_v_height = obj["WaypointInformation"].toArray().at(i).toObject()["bb"].toObject()["last_v_height"].toDouble();
	// 		temp.detection_box.last_v_width = obj["WaypointInformation"].toArray().at(i).toObject()["bb"].toObject()["last_v_width"].toDouble();
	// 		temp.detection_box.last_v_xcent = obj["WaypointInformation"].toArray().at(i).toObject()["bb"].toObject()["last_v_xcent"].toDouble();
	// 		temp.detection_box.last_v_ycent = obj["WaypointInformation"].toArray().at(i).toObject()["bb"].toObject()["last_v_ycent"].toDouble();
	// 		temp.detection_box.obj_class = obj["WaypointInformation"].toArray().at(i).toObject()["bb"].toObject()["obj_class"].toInt();
	// 		temp.detection_box.prob = obj["WaypointInformation"].toArray().at(i).toObject()["bb"].toObject()["prob"].toDouble();
	// 		temp.detection_box.v_height = obj["WaypointInformation"].toArray().at(i).toObject()["bb"].toObject()["v_height"].toDouble();
	// 		temp.detection_box.v_width = obj["WaypointInformation"].toArray().at(i).toObject()["bb"].toObject()["v_width"].toDouble();
	// 		temp.detection_box.v_xcent = obj["WaypointInformation"].toArray().at(i).toObject()["bb"].toObject()["v_xcent"].toDouble();
	// 		temp.detection_box.v_ycent = obj["WaypointInformation"].toArray().at(i).toObject()["bb"].toObject()["v_ycent"].toDouble();

	// 		temp.detection_box.width = obj["WaypointInformation"].toArray().at(i).toObject()["bb"].toObject()["width"].toInt();
	// 		temp.detection_box.xcent = obj["WaypointInformation"].toArray().at(i).toObject()["bb"].toObject()["xcent"].toInt();
	// 		temp.detection_box.xmax = obj["WaypointInformation"].toArray().at(i).toObject()["bb"].toObject()["xmax"].toInt();
	// 		temp.detection_box.xmin = obj["WaypointInformation"].toArray().at(i).toObject()["bb"].toObject()["xmin"].toInt();
	// 		temp.detection_box.ycent = obj["WaypointInformation"].toArray().at(i).toObject()["bb"].toObject()["ycent"].toInt();
	// 		temp.detection_box.ymax = obj["WaypointInformation"].toArray().at(i).toObject()["bb"].toObject()["ymax"].toInt();
	// 		temp.detection_box.ymin = obj["WaypointInformation"].toArray().at(i).toObject()["bb"].toObject()["ymin"].toInt();

	// 		for (int k = 0; k < obj["WaypointInformation"].toArray().at(i).toObject()["bb"].toObject()["class_prob"].toArray().size(); k++)
	// 		{
	// 			temp.detection_box.class_prob.push_back(obj["WaypointInformation"].toArray().at(i).toObject()["bb"].toObject()["class_prob"].toArray().at(k).toDouble());
	// 		}

	// 		temp.maxFlightSpeed = obj["WaypointInformation"].toArray().at(i).toObject()["maxFlightSpeed"].toDouble();
	// 		//temp.zoom_Magnification = obj["WaypointInformation"].toArray().at(i).toObject()["opticalZoomFocalLength"].toDouble();
	// 		temp.waypointType = obj["WaypointInformation"].toArray().at(i).toObject()["waypointType"].toInt();
	// 		temp.back_Type = obj["WaypointInformation"].toArray().at(i).toObject()["back_Type"].toInt();
	// 		temp.CONFIG_NO = obj["WaypointInformation"].toArray().at(i).toObject()["CONFIG_NO"].toInt();
	// 		temp.POI_X = obj["WaypointInformation"].toArray().at(i).toObject()["POI_X"].toDouble();
	// 		temp.POI_Y = obj["WaypointInformation"].toArray().at(i).toObject()["POI_Y"].toDouble();
	// 		temp.POI_Z = obj["WaypointInformation"].toArray().at(i).toObject()["POI_Z"].toDouble();
	// 		temp.gimbal_roll = obj["WaypointInformation"].toArray().at(i).toObject()["gimbal_roll"].toDouble();

	// 		vec.push_back(temp);
	// 	}

		
	// }

}