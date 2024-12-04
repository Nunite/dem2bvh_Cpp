#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include "OpenDemo.h"
#include "vector.h"

struct Frame {
    vec3_t playerPosition;
    vec3_t rotation;
};

// 处理角度突变的辅助函数
std::vector<float> interpolateAngles(const std::vector<float>& times, 
                                   std::vector<float> angles, 
                                   const std::vector<float>& targetTimes) {
    // 处理角度突变
    for(size_t i = 1; i < angles.size(); i++) {
        float diff = angles[i] - angles[i-1];
        if(std::abs(diff) > 180.0f) {
            // 如果角度差超过180度，说明发生了循环
            if(diff > 0) {
                for(size_t j = i; j < angles.size(); j++) {
                    angles[j] -= 360.0f;
                }
            } else {
                for(size_t j = i; j < angles.size(); j++) {
                    angles[j] += 360.0f;
                }
            }
        }
    }

    // 线性插值
    std::vector<float> result(targetTimes.size());
    size_t j = 0;
    for(size_t i = 0; i < targetTimes.size(); i++) {
        while(j < times.size()-1 && times[j+1] < targetTimes[i]) {
            j++;
        }
        
        if(j >= times.size()-1) {
            result[i] = angles.back();
        } else {
            float t = (targetTimes[i] - times[j]) / (times[j+1] - times[j]);
            result[i] = angles[j] + t * (angles[j+1] - angles[j]);
        }
        
        // 将结果映射回0-360范围
        result[i] = fmod(result[i], 360.0f);
        if(result[i] < 0) result[i] += 360.0f;
    }

    return result;
}

// 添加一个线性插值函数
std::vector<float> interpolateLinear(const std::vector<float>& times,
                                   const std::vector<float>& values,
                                   const std::vector<float>& targetTimes) {
    std::vector<float> result(targetTimes.size());
    size_t j = 0;
    
    for(size_t i = 0; i < targetTimes.size(); i++) {
        // 找到正确的时间区间
        while(j < times.size()-1 && times[j+1] < targetTimes[i]) {
            j++;
        }
        
        if(j >= times.size()-1) {
            result[i] = values.back();
        } else {
            float t = (targetTimes[i] - times[j]) / (times[j+1] - times[j]);
            result[i] = values[j] + t * (values[j+1] - values[j]);
        }
    }
    
    return result;
}

// 修改采样函数
std::vector<Frame> resampleFrames(const std::vector<Frame>& frames, 
                                float sourceFps, 
                                float targetFps) {
    if(sourceFps == targetFps) return frames;

    std::vector<float> originalTimes;
    std::vector<float> targetTimes;
    
    // 生成时间序列
    float totalTime = frames.size() / sourceFps;
    for(size_t i = 0; i < frames.size(); i++) {
        originalTimes.push_back(i / sourceFps);
    }
    
    for(float t = 0; t < totalTime; t += 1.0f/targetFps) {
        targetTimes.push_back(t);
    }

    std::vector<Frame> result(targetTimes.size());
    std::vector<float> values;

    // 位置插值 - 使用线性插值
    for(int i = 0; i < 3; i++) {
        values.clear();
        for(const auto& frame : frames) {
            float* pos = (float*)&frame.playerPosition;
            values.push_back(pos[i]);
        }
        
        std::vector<float> resampled;
        resampled = interpolateLinear(originalTimes, values, targetTimes);  // 使用线性插值
        
        for(size_t j = 0; j < resampled.size(); j++) {
            float* pos = (float*)&result[j].playerPosition;
            pos[i] = resampled[j];
        }
    }

    // 角度插值 - 保持原来的角度插值方法
    for(int i = 0; i < 3; i++) {
        values.clear();
        for(const auto& frame : frames) {
            float* rot = (float*)&frame.rotation;
            values.push_back(rot[i]);
        }
        
        std::vector<float> resampled;
        resampled = interpolateAngles(originalTimes, values, targetTimes);  // 角度仍使用角度插值
        
        for(size_t j = 0; j < resampled.size(); j++) {
            float* rot = (float*)&result[j].rotation;
            rot[i] = resampled[j];
        }
    }

    return result;
}

void writeBVH(const std::string& filename, 
              const std::vector<Frame>& frames,
              float frameTime) {
    std::ofstream out(filename);
    if(!out.is_open()) {
        std::cerr << "Failed to open output file: " << filename << std::endl;
        return;
    }

    // 写入BVH头部
    out << "HIERARCHY\n";
    out << "ROOT MdtCam\n";
    out << "{\n";
    out << "\tOFFSET 0.00 0.00 0.00\n";
    out << "\tCHANNELS 6 Xposition Yposition Zposition Zrotation Xrotation Yrotation\n";
    out << "\tEnd Site\n";
    out << "\t{\n";
    out << "\t\tOFFSET 0.00 0.00 -1.00\n";
    out << "\t}\n";
    out << "}\n";

    // 写入动作数据
    out << "MOTION\n";
    out << "Frames: " << frames.size() << "\n";
    out << "Frame Time: " << std::fixed << std::setprecision(6) << frameTime << "\n";

    // 写入每一帧的数据
    for(const auto& frame : frames) {
        out << -frame.playerPosition.y << " " 
            << frame.playerPosition.z + 16 << " " 
            << -frame.playerPosition.x << " ";

        out << "0.000000" << " "
            << (360.0f - frame.rotation.x) << " "
            << frame.rotation.y << "\n";
    }
}

int main(int argc, char* argv[]) {
    // 如果没有参数，显示帮助信息
    if(argc == 1) {
        std::cout << "Analyze GoldSrc demo file and convert to BVH\n\n";
        std::cout << "Usage: " << argv[0] << " <demo_file> [-fps fps_value]\n\n";
        std::cout << "Arguments:\n";
        std::cout << "  <demo_file>        Path to the demo file (*.dem)\n";
        std::cout << "  -fps fps_value     Target FPS (default: 30)\n\n";
        std::cout << "Author: Lws \n";
        std::cout << "Reference project: https://github.com/tpetrina/WebKZ\n";
        return 0;
    }

    try {
        std::string demoFile = argv[1];
        float targetFps = 30.0f;

        // 解析命令行参数
        for(int i = 2; i < argc; i++) {
            std::string arg = argv[i];
            if(arg == "-fps" && i + 1 < argc) {
                targetFps = std::stof(argv[++i]);
            }
        }

        // 检查文件扩展名
        if(demoFile.substr(demoFile.find_last_of(".") + 1) != "dem") {
            std::cout << "Warning: File does not have .dem extension\n";
        }

        // 使用OpenDemo库解析demo文件
        OpenDemo::Demo demo;
        std::wstring wDemoFile(demoFile.begin(), demoFile.end());
        if(!demo.Open(const_cast<wchar_t*>(wDemoFile.c_str()))) {
            std::cerr << "Error: Failed to open demo file: " 
                      << static_cast<int>(demo.GetLastError()) << std::endl;
            return 1;
        }

        std::cout << "Successfully opened demo file\n";

        std::vector<Frame> frames;
        float sourceFrameTime = 1.0f / 60.0f; // 默认值
        bool gotFrameTime = false;

        // 获取总帧数
        size_t frameCount = demo.GetNumberOfFrames();
        std::cout << "Raw frame count: " << frameCount << std::endl;

        if(frameCount == 0) {
            std::cerr << "No frames found in demo\n";
            return 1;
        }

        // 先尝试获取第一帧的时间
        float frameTime;
        if(demo.GetFrameTime(0, &frameTime) && frameTime > 0) {
            sourceFrameTime = frameTime;
            gotFrameTime = true;
            std::cout << "Source frame time: " << sourceFrameTime << std::endl;
        } else {
            std::cout << "Failed to get frame time, using default: " << sourceFrameTime << std::endl;
        }

        // 处理每一帧
        for(size_t i = 0; i < frameCount; i++) {
            Frame frame;
            OpenDemo::PlayerData playerData;
            float frameLength;
            
            if(demo.GetFrameLength(i, &frameLength) &&
               demo.GetPlayerData(i, &playerData)) {
                frame.playerPosition = playerData.position;
                frame.rotation = playerData.orientation;
                frames.push_back(frame);
            }

            // 如果还没有获取到帧时间，继续尝试
            if(!gotFrameTime && demo.GetFrameTime(i, &frameTime) && frameTime > 0) {
                sourceFrameTime = frameTime;
                gotFrameTime = true;
            }
        }

        std::cout << "Processed frames: " << frames.size() << std::endl;

        // 重采样
        float sourceFps = 1.0f / sourceFrameTime;
        std::cout << "Source FPS: " << sourceFps << std::endl;

        if(std::abs(sourceFps - targetFps) > 0.001f) {  // 添加一个小的容差
            std::cout << "Resampling from " << sourceFps << "fps to " << targetFps << "fps...\n";
            auto resampled = resampleFrames(frames, sourceFps, targetFps);
            if(!resampled.empty()) {
                frames = std::move(resampled);
            } else {
                std::cout << "Resampling failed, using original frames\n";
            }
        } else {
            std::cout << "No resampling needed\n";
        }

        // 保存为BVH文件
        std::string outputFile = demoFile.substr(0, demoFile.find_last_of('.')) + "_camera.bvh";
        writeBVH(outputFile, frames, 1.0f/targetFps);

        std::cout << "Camera motion saved to: " << outputFile << "\n";
        std::cout << "Total frames: " << frames.size() << "\n";
        std::cout << "Frame time: " << 1.0f/targetFps << " (" << targetFps << " fps)\n";

        return 0;
    }
    catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    catch(...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }
} 