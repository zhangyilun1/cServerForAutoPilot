CXX = g++
CXXFLAGS = -std=c++11
LIBS = -lsqlite3 -llog4cplus
INCLUDE_DIRS = -I/opt/homebrew/Cellar/cereal/1.3.2/include -I/opt/homebrew/Cellar/nlohmann-json/3.11.2/include

SRCS = config.cpp drones.cpp main.cpp log.cpp server.cpp session.cpp sessionManager.cpp sqliteConnection.cpp dataProcessor.cpp

OBJS = $(SRCS:.cpp=.o)

TARGET = server

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -o $@ $(OBJS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
