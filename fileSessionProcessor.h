#pragma once
#include <string>
#include <thread>
#include <sstream>
#include <filesystem>
#include <iostream>
#include "sqliteConnection.h"
#include "log.h"
#include "file.h"
#include "md5.h"
class FileSessionProcessor
{

//public:
//	static FileSessionProcessor* GetInstance(int port,int socket);

public:
	//FileSessionProcessor(int port, int socket);
	FileSessionProcessor();
	~FileSessionProcessor();
	void PfnProcessFileThread(int64_t socket);


private:
	static void HandleConnectionOnPort32322(FileSessionProcessor* fileSessionProcessor, int64_t socket);
	//void HandleConnectionOnPort32322(int64_t socket);
	void processFile(int64_t socket);
	bool handleConnection(int64_t socket);
	void handleTimeOut(int64_t socket);
	// std::vector<std::vector<uint8_t>> dataSetPacket(uint8_t buffer[], int ret);
	bool recvfile(int socket, std::string fileName, uint32_t fileLength, std::string md5);
	int recvAll(int64_t socket, uint8_t* bufer, size_t length);
	bool insertFileToDB(std::string filePath);
	std::string convertValidSymbol(std::string& fileName);
	bool isFileExists(const std::string& filePath);
	uint32_t getFileSize(const std::string& filePath);


private:
	std::string fileName;
	int fileLength;
	std::vector<uint8_t> fileBuff;
};

