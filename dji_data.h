#ifndef __DJI_DATA_H_INCLUDE__
#define __DJI_DATA_H_INCLUDE__

#include <cstdint>

#define DEV_SEARCH_MARK          "DJI0"
#define DEV_SEARCH_PORT          10087

enum en_cmd : uint8_t
{
	cmd_heart
    , take_start
    , cmd_task
	, cmd_distribution_task
	, cmd_pointtask
	, cmd_home_point
	, cmd_rtkhome_point
	, cmd_mission_point
    , cmd_get_record
    , cmd_local_gps
	, cmd_sn
	, cmd_tower_point
	, cmd_error
};

#pragma pack(push)
#pragma pack(1)
struct DJIData{
    en_cmd cmd_;
	uint16_t data_len_;
    uint8_t data_[0];
};
#pragma pack(pop)


#endif
