#include "file.h"

using namespace std;
FileSystem::FileSystem(){

}

FileSystem::~FileSystem(){

}



bool FileSystem::writeRoute(const std::vector<WaypointInformation>& vec, int task_id, uint32_t timestamp) {
    uint32_t tmptimestamp = 0;
    if (checkBreakTask(task_id, tmptimestamp)) {
        deleteFileOrFolder("./route/" + std::to_string(task_id) + "_" + std::to_string(tmptimestamp));
    }
    deleteFileOrFolder("./route/" + std::to_string(task_id));

    std::string name = std::to_string(task_id) + "_" + std::to_string(timestamp);
    return getJsonDataAndWriteFile(vec, name);
}


bool FileSystem::writeRoute(std::vector<WaypointInformation> vec, int task_id)
{
	uint32_t tmptimestamp = 0;
    if (checkBreakTask(task_id, tmptimestamp)) {
        deleteFileOrFolder("./route/" + std::to_string(task_id) + "_" + std::to_string(tmptimestamp));
    }
    deleteFileOrFolder("./route/" + std::to_string(task_id));

	string name = to_string(task_id);
	return getJsonDataAndWriteFile(vec, name);
}


bool FileSystem::checkBreakTask(int subtaskid, uint32_t &timestamp)
{
    //TRACE("begin checkBreakTask");
    string directoryPath = "./route"; // 指定目录路径
    //string directoryPath = "C:/Users/123/Desktop/console_socket/x64/Debug/route";
    // 使用C++17中的filesystem库来遍历目录
    for (const auto &entry : filesystem::directory_iterator(directoryPath))
    {
        if (entry.is_regular_file())
        {
            //TRACE("is_regular_file");
            string name = entry.path().filename().string();

            if (name.find("_") == string::npos)
            {
                // 普通任务文件
                if (name == to_string(subtaskid))
                {
                    return false;
                }
            }
            else
            {
                // 中断任务文件
                //TRACE("=== not ==");
                size_t pos = name.find("_");
                std::string leftname = name.substr(0, pos);
                if (leftname == std::to_string(subtaskid))
                {
                    std::string timestampStr = name.substr(pos + 1);
                    try
                    {
                        timestamp = std::stoul(timestampStr);
                    }
                    catch (...)
                    {
                        // 处理转换异常
                        return false;
                    }
                    return true;
                }
            }
        }
    }
    return false;
}

bool FileSystem::getJsonDataAndWriteFile(std::vector<WaypointInformation> vec, const std::string& filename) {
    string file_content;
    vector<int> toweridvec;
    vector<std::string> towerstrvec;
    JsonData jsonData;
    jsonData.waypoint2JSONSplitByTowerID(vec, file_content, toweridvec, towerstrvec);
    //m_cruiseJson.waypoint_to_JSON_splitbytowerid(vec, file_content, toweridvec, towerstrvec);
    isDirExist("./route");

    bool result = writeDataToFile("./route/" + filename, file_content);
    return result;
}

bool FileSystem::writeDataToFile(const std::string& file_path, const std::string& file_content) {
    std::ofstream outfile(file_path, std::ios::binary);
    if (!outfile.is_open()) {
        return false;
    }
        
    outfile.write(file_content.c_str(), file_content.size());
    outfile.close();
        
    return true;
}

bool FileSystem::isDirExist(const std::string& fullPath) {
    std::filesystem::path dirPath(fullPath);
        
    if (std::filesystem::exists(dirPath)) {
        return true;
    } else {
        bool ok = std::filesystem::create_directory(dirPath);
        return ok;
    }
}

bool FileSystem::deleteFileOrFolder(const std::string& strPath) {
    if (strPath.empty() || !filesystem::exists(strPath)) {
        return false;
    }
    filesystem::path path(strPath);
    if (filesystem::is_regular_file(path)) {
        filesystem::remove(path);
    } else if (filesystem::is_directory(path)) {
        filesystem::remove_all(path);
    }
    return true;
}

bool FileSystem::mergeContinueWaypoints(int submissionID, std::vector<WaypointInformation>& waypoints) {
    if (waypoints.empty()) {
        //TRACE("MergeContinueWaypoints waypoints size == 0!!!!!");
        return false;
    }

    uint32_t temptimestamp = 0;
    if (!checkBreakTask(submissionID, temptimestamp)) {
        //TRACE("checkBreakTask false!");
        return false;
    }

    vector<WaypointInformation> completepoints = readRoute(submissionID);
    if (completepoints.empty()) {
        //TRACE("MergeContinueWaypoints completepoints size == 0!!!!!");
        return false;
    }

    int index = -1;
    for (int i = 0; i < waypoints.size(); i++) {
        if (completepoints.back().tower_id == waypoints[i].tower_id) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        //TRACE("MergeContinueWaypoints can't find same towerid!!!!!");
        return false;
    }

    completepoints.insert(completepoints.end(), waypoints.begin() + index + 1, waypoints.end());
    deleteFileOrFolder("/route/" + std::to_string(submissionID) + "_" + std::to_string(temptimestamp));
    writeRoute(completepoints, submissionID);

    return true;
}

vector<WaypointInformation> FileSystem::readRoute(int task_id){
	uint32_t timestamp = 0;
    string name;
    if (checkBreakTask(task_id, timestamp)) {
        name = to_string(task_id) + "_" + to_string(timestamp);
    } else {
        name = to_string(task_id);
    }
    return readDataFromFile(name);
}

vector<WaypointInformation> FileSystem::readDataFromFile(string name){
	//直接读文件readDataFromFile
	//std::vector<int> vecSubTaskTower_id = m_cruiseSql.Query_submissiontower(task_id);//这里有bug，只会返回一个tower_id
	//if (vecSubTaskTower_id.size() == 0) {
	//	return std::vector<WaypointInformation>();
	//}
	//QString tmp = "/XJ_FineRouteTower/" + QString::number(vecSubTaskTower_id[0]);
	//直接读文件end
    JsonData jsonData;
    std::string file_path = "./route/" + name;
    std::ifstream file(file_path);
    if (!file.is_open()) {
        return std::vector<WaypointInformation>();
    }

    string file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    return jsonData.Converting_the_aircraft_waypoint_structure_into_a_structure(file_content);
}