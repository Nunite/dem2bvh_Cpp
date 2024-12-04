CXX = g++
CXXFLAGS = -I./cpp-exporter/OpenDemo -DUNICODE -D_UNICODE
LDFLAGS = -L./build -lOpenDemo

# 明确列出源文件，排除示例文件
OPENDEMO_SRCS = \
    cpp-exporter/OpenDemo/OpenDemo.cpp \
    cpp-exporter/OpenDemo/filereader.cpp \
    cpp-exporter/OpenDemo/memoryreader.cpp \
    cpp-exporter/OpenDemo/OpenDemoMain.cpp

OPENDEMO_OBJS = $(OPENDEMO_SRCS:.cpp=.o)

all: build/libOpenDemo.a demo_analysis

build/libOpenDemo.a: $(OPENDEMO_OBJS)
	@mkdir -p build
	ar rcs $@ $^

demo_analysis: cpp-exporter/OpenDemo/demo_analysis.cpp build/libOpenDemo.a
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf build *.o demo_analysis

# 调试用：显示找到的源文件
debug:
	@echo "OpenDemo sources: $(OPENDEMO_SRCS)"
	@echo "OpenDemo objects: $(OPENDEMO_OBJS)"