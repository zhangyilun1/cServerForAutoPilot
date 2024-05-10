#include "memoryTrack.h"
#include "log.h"
using namespace std;


#define MEMORY_TRACKER_NEW(Type, varName) new(MemoryTracker::operator new(sizeof(Type), #varName)) Type
#define MEMORY_TRACKER_DELETE(Type, varName) MemoryTracker::operator delete(varName)


std::unordered_map<void*, std::string> MemoryTracker::allocationMap;

void* MemoryTracker::operator new(size_t size, const char* variableName) {
    LOG(ERROR) << "new operator ";
    void* memory = ::operator new(size);
    LOG(ERROR) << "size : " << size;
    allocationMap[memory] = variableName;
    return memory;
}

void MemoryTracker::operator delete(void* memory) {
    auto it = allocationMap.find(memory);
    if (it != allocationMap.end()) {
        LOG(ERROR)<< "Deleting memory for variable: " << it -> second;
        allocationMap.erase(it);
    }
    ::operator delete(memory);
}