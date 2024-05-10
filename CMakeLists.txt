cmake_minimum_required(VERSION 2.8)

project(microscope_adapter)

set(CMAKE_CXX_STANDARD 17)

add_definitions("-Wall -g3 -O0")

#相当于 -i
# include_directories("/usr/local/opt/cpprestsdk/include/")
# include_directories("/usr/local/opt/openssl@3/include/")
# include_directories("/usr/local/opt/boost/include/")
# include_directories("/usr/local/opt/log4cplus/include/")

#相当于 -L
# link_directories("/usr/local/opt/openssl@3/lib")
# link_directories("/usr/local/opt/cpprestsdk/lib")
# link_directories("/usr/local/opt/boost/lib")
# link_directories("/usr/local/opt/log4cplus/lib")

set( SRC_LIST
	 ./main.cpp
	 ./config.cpp
	 ./microscope_adapter.cpp
	 ./server.cpp
	 ./session_manager.cpp
	 ./session.cpp
	 ./fileDetector.cpp
	 ./multipart_parser.cpp
	 ./controlFile.cpp
	 ./uploadFile.cpp
	 ./tinyxml2.cpp
	 ./log.cpp)


add_executable(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries( ${PROJECT_NAME} -lsqlite3 -llog4cplus)