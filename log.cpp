#include "log.h"


bool gbLogInit = false;


void initLog()
{
	//FLAGS_log_dir = "C:\\Users\\123\\Desktop\\console_socket\\console_socket\\logs";
	FLAGS_log_dir = "./logs";
	FLAGS_logtostderr = false;
	//单个日志文件最大，单位M
	FLAGS_max_log_size = 1024 * 32;
	//缓存的最大时长，超时会写入文件
	FLAGS_logbufsecs = 0;
#ifdef _DEBUG
	FLAGS_alsologtostderr = true;//是否将日志输出到文件和stderr
#else
	FLAGS_alsologtostderr = false;//是否将日志输出到文件和stderr
#endif
	google::InitGoogleLogging("guard");

	//FLAGS_colorlogtostderr = true;//是否启用不同颜色显示
	google::SetLogFilenameExtension(".log");
	google::SetLogDestination(google::GLOG_INFO, ".\\logs\\INFO_");//INFO级别的日志都存放到logs目录下且前缀为INFO_
	google::SetLogDestination(google::GLOG_WARNING, ".\\logs\\WARNING_");//WARNING级别的日志都存放到logs目录下且前缀为WARNING_
	google::SetLogDestination(google::GLOG_ERROR, ".\\logs\\ERROR_");	//ERROR级别的日志都存放到logs目录下且前缀为ERROR_
	google::SetLogDestination(google::GLOG_FATAL, ".\\logs\\FATAL_");	//FATAL级别的日志都存放到logs目录下且前缀为FATAL_
	gbLogInit = true;
}

void releaseLogData()
{
	if (gbLogInit) {
		google::ShutdownGoogleLogging();
		gbLogInit = false;
	}
}
