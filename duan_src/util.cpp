/*************************************************************************
	> File Name: util.cpp
	> Author: duan 
	> Mail: w_duan2001@163.com
	> Created Time: Thu 26 Sep 2024 11:41:42 AM CST
 ************************************************************************/

#include "util.h"
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <iomanip>

namespace duan{

void myAssert(bool condition, std::string message){
    if(!condition){
        std::cerr << "Error: " << message << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

std::chrono::_V2::system_clock::time_point now() { return std::chrono::high_resolution_clock::now(); }

// 获取随机选举超时时间
std::chrono::milliseconds getRandomizedElectionTimeout(){
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(minRndomizedElectionTime, maxRandomizedElectionTime);

    return std::chrono::milliseconds(dist(rng));
}

void sleepNMilliseconds(int N) { std::this_thread::sleep_for(std::chrono::milliseconds(N)); }

bool getReleasePort(short& port){
    short num = 0;
    while(!isReleasePort(port) && num < 30){
        ++port;
        ++num;
    }
    if(num >= 30){
        port = -1;
        return false;
    }
    return true;
}

bool isReleasePort(unsigned short usPort){
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(usPort);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    int ret = ::bind(s, (sockaddr *)&addr, sizeof(addr));
    if(ret != 0){
        close(s);
        return false;
    }
    close(s);
    return true;
}

void DPrintf(const char* format, ...){
    if(Debug){
        // 获取当前时间的秒数
        time_t now = time(nullptr);
        // 将时间转换为本地时间结构tm
        tm* nowtm = localtime(&now);
        // 声明一个va_list类型的变量 用于存储可变参数
        va_list args;
        // 初始化args 以便后续访问可变参数
        va_start(args, format);
        std::printf("[%d-%d-%d-%d-%d-%d] ", nowtm->tm_year + 1900, nowtm->tm_mon + 1, nowtm->tm_mday, nowtm->tm_hour, nowtm->tm_min, nowtm->tm_sec);
        // 根据提供的格式字符串和可变参数输出调试信息
        std::vprintf(format, args);
        std::printf("\n");
        // 清理args 以避免内存泄露
        va_end(args);
    }
}

}
