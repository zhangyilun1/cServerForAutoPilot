#include <iostream>
#include <unordered_map>
class MemoryTracker {
	public:
		static std::unordered_map<void*, std::string> allocationMap;
		static void* operator new(size_t size, const char* variableName);
		static void operator delete(void* memory);

};