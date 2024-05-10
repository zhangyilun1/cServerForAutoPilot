#include "log1.h"
#include <filesystem>
using namespace std;
#define LOG4CPLUS_CONF_FILE "C:/Users/123/Desktop/console_socket/console_socket/log4cplus.properties"

Log& log1 = Log::getInstance();

using namespace std;

Log::Log()
{
    std::cout<<"constructor called! "<<std::endl;
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::cout << "Current working directory: " << currentPath << std::endl;
    log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT(LOG4CPLUS_CONF_FILE));
}
 
Log::~Log()
{
    log4cplus::Logger::shutdown();
    std::cout<<"destructor called!"<<std::endl;
}

Log& Log::getInstance()
{
    static Log instance;
    return instance;
}
