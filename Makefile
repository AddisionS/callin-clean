CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -MMD -MP
LDLIBS   = -lgdi32 -luser32 -lshell32 -ladvapi32 -mwindows
TARGET   = CallInClean.exe

SRCS = src/main.cpp src/hook.cpp src/overlay.cpp src/autostart.cpp
OBJS = $(SRCS:.cpp=.o)
RES  = src/resources.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS) $(RES)
	$(CXX) $(OBJS) $(RES) -o $@ $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(RES): src/resources.rc src/resource.h assets/icon.ico
	windres -I src -i src/resources.rc -o $(RES)

clean:
	del /Q src\*.o src\*.d $(TARGET)

-include $(OBJS:.o=.d)