#ifndef _LOG_H_
#define _LOG_H_

#include <iostream>
#include <string>
#include <sstream>
#include <log4cplus/configurator.h>
#include <log4cplus/logger.h>
#include <log4cplus/log4cplus.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/initializer.h>


#define TRACE(info)    LOG4CPLUS_TRACE(log4cplus::Logger::getRoot(), info)
#define DEBUG(info)    LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), info)
#define INFO(info)     LOG4CPLUS_INFO(log4cplus::Logger::getRoot(), info)
#define WARN(info)     LOG4CPLUS_WARN(log4cplus::Logger::getRoot(), info)
#define ERROR(info)    LOG4CPLUS_ERROR(log4cplus::Logger::getRoot(), info)
#define FATAL(info)    LOG4CPLUS_FATAL(log4cplus::Logger::getRoot(), info)

class Log
{

public:
 ~Log();
 Log(const Log&)=delete;
 static Log& getInstance();

private:
 Log();

};

#endif