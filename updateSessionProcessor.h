#pragma once
#include <string>
#include <thread>
#include <sstream>
#include <filesystem>
#include <iostream>
#include "sqliteConnection.h"
#include "dataProcessor.h"
#include "log.h"
#include "file.h"
#include "md5.h"
class UpdateSessionProcessor
{


public:
	UpdateSessionProcessor();
	~UpdateSessionProcessor();
	void PfnProcessUpdateThread(int64_t socket);


private:
	static void HandleConnectionOnPort32330(UpdateSessionProcessor* updateSessionProcessor, int64_t socket);
	void processUpdate(int64_t socket);
	bool handleConnection(int64_t socket);
	void handleTimeOut(int64_t socket);
	bool recvfile(int socket, std::string fileName, uint32_t fileLength, std::string md5);
	int recvAll(int64_t socket, uint8_t* bufer, size_t length);
	bool insertFileToDB(std::string filePath);
	std::string convertValidSymbol(std::string& fileName);
	bool isFileExists(const std::string& filePath);
	uint32_t getFileSize(const std::string& filePath);

private:
	std::string fileName;
	int seekg;
	std::vector<uint8_t> fileBuff;
};

