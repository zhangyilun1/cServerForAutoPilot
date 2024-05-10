#pragma once
#ifndef GLOG_NO_ABBREVIATED_SEVERITIES
#define GLOG_NO_ABBREVIATED_SEVERITIES // 如果不加这个宏定义代码就会报错
#endif

#include <glog/logging.h>

void initLog();
void releaseLogData();