#include "fileSessionProcessor.h"
using namespace std;

static SQLiteConnection* SQLiteConnector;

//static FileSessionProcessor* fileSessionProcessor;
//
//FileSessionProcessor* FileSessionProcessor::GetInstance(int port, int socket)
//{
//    if (fileSessionProcessor == nullptr)
//        fileSessionProcessor = new FileSessionProcessor(port,socket);
//
//    return fileSessionProcessor;
//}


//FileSessionProcessor::FileSessionProcessor(int port, int socket):fileName(""), fileLength(0), fileBuff() {
//    if (port == 32322)
//    {
//        std::thread processFileThread(HandleConnectionOnPort32322, this, socket);
//        processFileThread.detach();
//    }
//
//}


FileSessionProcessor::FileSessionProcessor() :fileName(""), fileLength(0), fileBuff() {
}

FileSessionProcessor::~FileSessionProcessor() {
};


void FileSessionProcessor::PfnProcessFileThread(int64_t socket)
{
    LOG(ERROR) << "Create thread for process File ";
    std::thread processFileThread(HandleConnectionOnPort32322, this, socket);
    processFileThread.detach();
}

void FileSessionProcessor::HandleConnectionOnPort32322(FileSessionProcessor* fileSessionProcessor, int64_t socket)
{
    LOG(ERROR) << "HandleConnectionOnPort32322 ";
    thread::id tid = this_thread::get_id();
    std::stringstream ss;
    ss << tid;
    LOG(ERROR) << "tid" << ss.str();
    fileSessionProcessor->processFile(socket);

}


void FileSessionProcessor::processFile(int64_t socket) {
    LOG(ERROR) << "socket :" << socket;
    while (true) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(socket, &read_fds);

        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        int activity = select(socket + 1, &read_fds, NULL, NULL, &timeout);
        if (activity > 0) {
            if (FD_ISSET(socket, &read_fds)) {
                LOG(ERROR) << "handleConnection";
                handleConnection(socket);
                break;
            }
        }
        else if (activity == 0) {
            LOG(ERROR) << "time out";
            handleTimeOut(socket);
            break;
        }
        else {
            break;
        }
        Sleep(20);
    }
}

bool FileSessionProcessor::handleConnection(int64_t socket) {
    LOG(ERROR) << " ==== handleConnection ==== "<< " socket : " << socket << " THREAD ID:  " << this_thread::get_id();
    string snCodeString;
    string fileNameString;
    string md5String;

    uint8_t snCode[24];
    LOG(ERROR) << "sizeof(snCode): " << sizeof(snCode) << " socket : " << socket << " THREAD ID:  " << this_thread::get_id();
    int recvSnCode = recvAll(socket, snCode, sizeof(snCode));
    if (recvSnCode  == 0) {
        LOG(ERROR)<< "Failed to receive SN code.";
        closesocket(socket);
        return 0;
    }
    else if (recvSnCode == 24) {
       string snCodeString1(reinterpret_cast<char*>(snCode), strnlen(reinterpret_cast<char*>(snCode), sizeof(snCode)));
       snCodeString = snCodeString1;
       LOG(ERROR) << "snCodeString: " << snCodeString << " socket : " << socket << " THREAD ID:  " << this_thread::get_id();

       if (snCodeString.find("/") != string::npos || snCodeString.find(" ") != string::npos) {
           closesocket(socket);
           return 0;
       }
      
    } 
    
    uint8_t mfileName[64];
    LOG(ERROR) <<"sizeof(fileName): " << sizeof(mfileName);
    int recvMfileName = recvAll(socket, mfileName, sizeof(mfileName));
    if (recvMfileName == 0) {
        LOG(ERROR)<< "Failed to receive file name.";
        closesocket(socket);
        return 0;
    }
    else if (recvMfileName == 64) {
        string fileNameString1(reinterpret_cast<char*>(mfileName), strnlen(reinterpret_cast<char*>(mfileName), sizeof(mfileName)));
        fileNameString = fileNameString1;
        LOG(ERROR) << "fileNameString: " << fileNameString << " THREAD ID:  " << this_thread::get_id();

        if (fileNameString.find("/") != string::npos || fileNameString.find(" ") != string::npos) {
            closesocket(socket);
            return 0;
        }
      
    }

    //for (size_t i = 0; i < sizeof(mfileName); ++i) {
    //    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int> (mfileName[i]) << " ";
    //}
   
    string fileName = snCodeString + "_" + fileNameString;
    LOG(ERROR) << "fileName: " << fileName << " "  << fileName.length() << " THREAD ID:  " << this_thread::get_id();

    uint32_t mfileLength;
    LOG(ERROR) << "sizeof(mfileLength): " << sizeof(mfileLength);
    int recvMfileLength = recvAll(socket, reinterpret_cast<uint8_t*> (&mfileLength), sizeof(mfileLength));
    if (recvMfileLength == 0) {
        LOG(ERROR)<< "Failed to receive file length.";
        closesocket(socket);
        return 0;
    }
    else if (recvMfileLength == 4) {
        fileLength = mfileLength;
        LOG(ERROR) << "fileLength: " << fileLength << " THREAD ID:  " << this_thread::get_id();
        if (fileLength < 0) {
            closesocket(socket);
            return 0;
        }
    }

    uint8_t md5[32];
    LOG(ERROR) << "sizeof(md5): " << sizeof(md5);
    int recvMd5Size = recvAll(socket, md5, sizeof(md5));
    if (recvMd5Size == 0) {
        LOG(ERROR)<< "Failed to receive MD5.";
        closesocket(socket);
        return 0;
    }
    else if(recvMd5Size == 32){
        string md5String1(reinterpret_cast<char*>(md5), strnlen(reinterpret_cast<char*>(md5), sizeof(md5)));
        md5String = md5String1;
    }

    int result = recvfile(socket, fileName, fileLength, md5String);
    LOG(ERROR) << "result : " << result;
    if (!result) {
        closesocket(socket);
        return 0;
    }

    LOG(ERROR) << "File data received successfully.";

    closesocket(socket);
    LOG(ERROR) << "socket already close";
    return 0;
    
}


bool FileSessionProcessor::recvfile(int socket, string fileName, uint32_t fileLength, string md5) {

    string fileValidName = convertValidSymbol(fileName);
    LOG(ERROR) << "fileValidName : " << fileValidName;

    std::string filePath = "./droneLog/" + fileValidName;
    LOG(ERROR) << "filePath : " << filePath;

    if (isFileExists(filePath)) {
        LOG(ERROR) << "file already existed";
        uint32_t existingFileSize = getFileSize(filePath);
        LOG(ERROR) << "existingFileSize : " << (int)existingFileSize;
        send(socket, reinterpret_cast<char*> (&existingFileSize), sizeof(existingFileSize), 0);

        LOG(ERROR)<< "File already exists. Size: "<< existingFileSize;
        if (fileLength < existingFileSize) {
             LOG(ERROR) << "file error ";
             return false;
        }
        std::ofstream outputFile(filePath, std::ios::binary | std::ios::app);
        if (!outputFile.is_open()) {
            LOG(ERROR)<< "Failed to open file for append.";
            return false;
        }
        uint32_t remainingFileSize = fileLength - existingFileSize;
        if (remainingFileSize > 0) {
            LOG(ERROR) << "remainingFileSize " << (int)remainingFileSize;
            uint8_t* fileData = new uint8_t[remainingFileSize];
            if (recvAll(socket, fileData, remainingFileSize) <= 0) {
                delete[] fileData;
                return false;
            }
            outputFile.write(reinterpret_cast <char*&>(fileData), remainingFileSize);
            delete[] fileData;
        }
        else if (remainingFileSize == 0) {
            LOG(ERROR) << "file already existed ";
        }
       
        LOG(ERROR)<< "File data appended successfully.";
    }
    else {
        LOG(ERROR) << "file not existed";
        LOG(ERROR) << "filePath : " << filePath;
        std::ofstream newFile(filePath, std::ios::binary);
        if (!newFile.is_open()) {
            LOG(ERROR)<< "Failed to create file.";
            //perror(" == failed  == ");
            LOG(ERROR) << "Failed to create file. Error: " << strerror(errno);
            return false;
        }
        uint32_t zero = 0;
        uint8_t* fileData = new uint8_t[fileLength];
        LOG(ERROR) << "fileLength " << (int)fileLength;
        int recvNum = recvAll(socket, fileData, fileLength);
        LOG(ERROR) << "recvNum : " << recvNum;
        if (recvNum == 0 || recvNum != fileLength) {
            LOG(ERROR)<< "Failed to receive file data.";
            delete[] fileData;
            return false;
        }
        else if (recvNum == fileLength) {
            newFile.write(reinterpret_cast<char*> (fileData), fileLength);
        }

        delete[] fileData;

        LOG(ERROR)<< "New file is been created and receive data successfully.";
    }
    string md5Hash = calculateMD5byOpenSSL(filePath);
    LOG(ERROR) << "md5Hash: " << md5Hash;
    uint8_t answer = 1;
    if (md5Hash != md5)
    {
        LOG(ERROR) << "MD5 NOT SAME, An error occurred in storing the file";
        send(socket, reinterpret_cast<char*>(&answer), 1, 0);
        return false;
    }
    if (!insertFileToDB(fileValidName)) {
        send(socket, reinterpret_cast<char*>(&answer), 1, 0);
        return false;
    }
    answer = 0;
    send(socket, reinterpret_cast<char*> (&answer), 1, 0);

    LOG(ERROR) << "response to socket successful ";
    return true;
}



bool FileSessionProcessor::insertFileToDB(string filePath) {
    SQLiteConnection* SQLiteConnector = &SQLiteConnection::getInstance();
    string result;
    string selectQuery = "SELECT COUNT(*) FROM droneLog WHERE logName = '" + filePath + "';";
    if (SQLiteConnector->executeScalar(selectQuery, result)) {
        int count = stoi(result);
        if (count == 0) {
            string insertQuery = "INSERT INTO droneLog (logName) VALUES (?);";
            vector<string> placeholders = {
                        filePath
            };
            if (SQLiteConnector->executeDroneInsertQuery(insertQuery, placeholders)) {
                LOG(ERROR) << "Insertion successful";
                return true;
            }
            else {
                LOG(ERROR) << "Insertion failed";
                return false;
            }
        }
        else {
            LOG(ERROR) << "log already exists";
            return true;
        }
    }
}

int FileSessionProcessor::recvAll(int64_t socket, uint8_t* buffer, size_t length) {
    size_t totalReceived = 0;
    while (totalReceived < length) {
        int bytesRead = recv(socket, reinterpret_cast <char*> (buffer + totalReceived), length - totalReceived, 0);
        if (bytesRead == 0) {
            return bytesRead; 
        }
        else if (bytesRead < 0) {
            LOG(ERROR) << "bytesRead < 0 ";
            if (bytesRead == SOCKET_ERROR) {
                LOG(ERROR) << "SOCKET ERROR";
                int error = WSAGetLastError();
                LOG(ERROR) << "ERROR" << error;
            }
            LOG(ERROR) << errno;

            if (errno == EAGAIN || errno == EWOULDBLOCK) {

                LOG(ERROR) << "retry send data ";
                this_thread::sleep_for(chrono::seconds(3));
                continue;
            }
            else {
                LOG(ERROR) << "Error sending data: :" << strerror(errno);
                return bytesRead;
            }
            break;
        }
        /*  for (size_t i = 0; i < bytesRead; ++i) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast< int > (buffer[totalReceived + i]) << " ";
        }*/
        totalReceived += bytesRead;
    }
    return totalReceived;
}

void FileSessionProcessor::handleTimeOut(int64_t socket) {
    //if (file.is_open()) {
    //    LOG(ERROR) << "file already existed";
    //    uint32_t existingFileSize = static_cast<uint32_t> (file.tellg());
    //    LOG(ERROR) << "existingFileSize : " << (int)existingFileSize;
    //    send(socket, reinterpret_cast<char*> (&existingFileSize), sizeof(existingFileSize), 0);
    //    LOG(ERROR) << "File already exists. Size: " << existingFileSize;
    //    fileLength -= existingFileSize;
    //    std::ofstream outputFile(filePath, std::ios::binary | std::ios::app);
    //    if (!outputFile.is_open()) {
    //        LOG(ERROR) << "Failed to open file for append.";
    //        closesocket(socket);
    //        return;
    //    }
    //    // 接收并追加剩余的文件数据
    //    uint32_t remainingFileSize = fileLength - existingFileSize;
    //    uint8_t* fileData = new uint8_t[remainingFileSize];
    //    if (recvAll(socket, fileData, remainingFileSize) <= 0) {
    //        LOG(ERROR) << "Failed to receive file data.";
    //        delete[] fileData;
    //        closesocket(socket);
    //        return;
    //    }
    //    outputFile.write(reinterpret_cast <char*&>(fileData), remainingFileSize);
    //    delete[] fileData;
    //    LOG(ERROR) << "File data appended successfully.";
    //}
    closesocket(socket);
}



bool FileSessionProcessor::isFileExists(const std::string&filePath) {
    namespace fs = std::filesystem;
    return fs::exists(filePath) && fs::is_regular_file(filePath);
}

uint32_t FileSessionProcessor::getFileSize(const std::string& filePath) {
    namespace fs = std::filesystem;
    if (isFileExists(filePath)) {
        return static_cast <uint32_t> (fs::file_size(filePath));
    }
    return 0;
}

//bool FileSessionProcessor::recvfile(int socket, const std::string& amp; fileName, uint32_t fileLength, const std::string& amp; md5) {
//    std::string filePath = "./" + fileName;
//    LOG(ERROR)& lt; &lt; "filePath : " & lt; &lt; filePath;
//
//    if (isFileExists(filePath)) {
//        LOG(ERROR)& lt; &lt; "file already existed";
//        uint32_t existingFileSize = getFileSize(filePath);
//        LOG(ERROR)& lt; &lt; "existingFileSize : " & lt; &lt; existingFileSize;
//        send(socket, reinterpret_cast & lt; char*& gt; (&amp; existingFileSize), sizeof(existingFileSize), 0);
//        LOG(ERROR)& lt; &lt; "File already exists. Size: " & lt; &lt; existingFileSize;
//
//        if (fileLength& lt; existingFileSize) {
//            LOG(ERROR)& lt; &lt; "file error ";
//            return false;
//        }
//
//        std::ofstream outputFile(filePath, std::ios::binary | std::ios::app);
//
//        if (!outputFile.is_open()) {
//            LOG(ERROR)& lt; &lt; "Failed to open file for append.";
//            closesocket(socket);
//            return false;
//        }
//
//        uint32_t remainingFileSize = fileLength - existingFileSize;
//
//        if (remainingFileSize& gt; 0) {
//            LOG(ERROR)& lt; &lt; "remainingFileSize " & lt; &lt; remainingFileSize;
//            uint8_t* fileData = new uint8_t[remainingFileSize];
//
//            if (recvAll(socket, fileData, remainingFileSize) & lt; = 0) {
//                delete[] fileData;
//                closesocket(socket);
//                return false;
//            }
//
//            outputFile.write(reinterpret_cast & lt; char*& amp; &gt; (fileData), remainingFileSize);
//            delete[] fileData;
//        }
//        else if (remainingFileSize == 0) {
//            LOG(ERROR)& lt; &lt; "file already existed ";
//        }
//
//        LOG(ERROR)& lt; &lt; "File data appended successfully.";
//    }
//    else {
//        LOG(ERROR)& lt; &lt; "filePath : " & lt; &lt; filePath;
//        std::ofstream newFile(filePath, std::ios::binary);




string FileSessionProcessor::convertValidSymbol(string& fileName) {
    size_t found = fileName.find(":");
    while (found != std::string::npos) {
        fileName.replace(found, 1, "_");
        found = fileName.find(":", found + 1);
    }
    return fileName;
}