#include "updateSessionProcessor.h"
using namespace std;

static SQLiteConnection* SQLiteConnector;


UpdateSessionProcessor::UpdateSessionProcessor() :fileName(""), seekg(0), fileBuff() {
}

UpdateSessionProcessor::~UpdateSessionProcessor() {
};


void UpdateSessionProcessor::PfnProcessUpdateThread(int64_t socket)
{
    LOG(ERROR) << "Create thread for update ";
    std::thread processFileThread(HandleConnectionOnPort32330, this, socket);
    processFileThread.detach();
}

void UpdateSessionProcessor::HandleConnectionOnPort32330(UpdateSessionProcessor* updateSessionProcessor, int64_t socket)
{
    LOG(ERROR) << "HandleConnectionOnPort32330 for update ";
    thread::id tid = this_thread::get_id();
    std::stringstream ss;
    ss << tid;
    LOG(ERROR) << "tid" << ss.str();
    updateSessionProcessor->processUpdate(socket);

}


void UpdateSessionProcessor::processUpdate(int64_t socket) {
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

bool UpdateSessionProcessor::handleConnection(int64_t socket) {
    LOG(ERROR) << " ==== handleConnection ==== " << " socket : " << socket << " THREAD ID:  " << this_thread::get_id();
    DataProcessor* dataProcessor = new DataProcessor();
    string snCodeString;
    string versionNumberString;
    string md5String;

    uint8_t snCode[24];
    LOG(ERROR) << "sizeof(snCode): " << sizeof(snCode) << " socket : " << socket << " THREAD ID:  " << this_thread::get_id();
    int recvSnCode = recvAll(socket, snCode, sizeof(snCode));
    if (recvSnCode == 0) {
        LOG(ERROR) << "Failed to receive SN code.";
        closesocket(socket);
        return 0;
    }
    else if (recvSnCode == 24) {
        string snCodeString1(reinterpret_cast<char*>(snCode), strnlen(reinterpret_cast<char*>(snCode), sizeof(snCode)));
        snCodeString = snCodeString1;
        LOG(ERROR) << "snCodeString: " << snCodeString << " socket : " << socket << " THREAD ID:  " << this_thread::get_id();

    }

    uint8_t versionNumber[12];
    LOG(ERROR) << "sizeof(versionNumber): " << sizeof(versionNumber);
    int recvVersionNumberString = recvAll(socket, versionNumber, sizeof(versionNumber));
    if (recvVersionNumberString == 0) {
        LOG(ERROR) << "Failed to receive file name.";
        closesocket(socket);
        return 0;
    }
    else if (recvVersionNumberString == 12) {
        string versionNumberString1(reinterpret_cast<char*>(versionNumber), strnlen(reinterpret_cast<char*>(versionNumber), sizeof(versionNumber)));
        versionNumberString = versionNumberString1;
        LOG(ERROR) << "fileNameString: " << versionNumberString << " THREAD ID:  " << this_thread::get_id();
    }


    uint32_t mSeekg;
    LOG(ERROR) << "sizeof(mSeekg): " << sizeof(mSeekg);
    int recvSeekg = recvAll(socket, reinterpret_cast<uint8_t*> (&mSeekg), sizeof(mSeekg));
    if (recvSeekg == 0) {
        LOG(ERROR) << "Failed to receive file length.";
        closesocket(socket);
        return 0;
    }
    else if (recvSeekg == 4) {
        seekg = mSeekg;
        if (seekg < 0) {
            closesocket(socket);
            return 0;
        }
        LOG(ERROR) << "seekg: " << seekg << " THREAD ID:  " << this_thread::get_id();
    }

    vector<vector<string>> versionAndFilePath = dataProcessor->querySystemVersionAndFilePath();
    if (!versionAndFilePath.empty()) {

        string dbVersionNumber = versionAndFilePath[0][0];
        LOG(ERROR) << "dbVersionNumber :" << dbVersionNumber;
        string dbFilePath = versionAndFilePath[0][1];

        int versionNumberFromDrone;
        int a, b, c;
        if (!dataProcessor->str2Version(versionNumberString, a, b, c))
            versionNumberFromDrone = 0;
        else {
            versionNumberFromDrone = a * 1000000 + b * 1000 + c;
        }
        int versionNumberFromDB;
        int one, two, three;
        if (!dataProcessor->str2Version(dbVersionNumber, one, two, three))
            versionNumberFromDB = 0;
        else {
            versionNumberFromDB = one * 1000000 + two * 1000 + three;
        }

        LOG(ERROR) << "versionNumberFromDrone :" << versionNumberFromDrone;
        LOG(ERROR) << "versionNumberFromDB :" << versionNumberFromDB;

        if (versionNumberFromDrone == 0 || versionNumberFromDB == 0) {
            LOG(ERROR) << "versionNumberFromDrone OR versionNumberFromDB ERROR ";
            closesocket(socket);
            return 0;
        }

        int result = 0;
        if (versionNumberFromDrone > versionNumberFromDB) {
            result = -1;
        }
        else if (versionNumberFromDrone < versionNumberFromDB) {
            result = 1;
        }
        else if (versionNumberFromDrone == versionNumberFromDB) {
            result = 0;
        }

        char confirm = 0;
        ConfigFile* mConfig = new ConfigFile("./config.txt");
        int rollBack = dataProcessor->whetherRoolBack(mConfig);
        LOG(ERROR) << "rollBack : " << rollBack;

        if (rollBack == 1) {
            LOG(ERROR) << "need roll back ";
            confirm = 2;
            send(socket, &confirm, 1, 0);
        }
        else if (result == 0 || result == -1) {
            LOG(ERROR) << "cancel update : ";
            send(socket, &confirm, 1, 0);
        }
        else if (result == 1) {
            LOG(ERROR) << "need update  ";
            string md5Hash = calculateMD5byOpenSSL(dbFilePath);
            LOG(ERROR) << "md5Hash in string :" << md5Hash;
            dataProcessor->sendFile(socket, dbFilePath, md5Hash, seekg);
        }
        delete mConfig;
    }
    delete dataProcessor;
    closesocket(socket);
    LOG(ERROR) << "socket already close";
    return 0;

}

int UpdateSessionProcessor::recvAll(int64_t socket, uint8_t* buffer, size_t length) {
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
        totalReceived += bytesRead;
    }
    return totalReceived;
}

void UpdateSessionProcessor::handleTimeOut(int64_t socket) {
    closesocket(socket);
}

