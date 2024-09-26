/*************************************************************************
	> File Name: util.h
	> Author: duan 
	> Mail: w_duan2001@163.com
	> Created Time: Wed 25 Sep 2024 04:35:01 PM CST
 ************************************************************************/

#ifndef _UTIL_H
#define _UTIL_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/access.hpp>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <random>
#include <sstream>
#include <thread>
#include "config.h"

namespace duan{

template <class F>
class DeferClass{
public:
    DeferClass(F&& f) : m_func(std::forward<F>(f)){}
    DeferClass(const F& f) : m_func(f) {}
    ~DeferClass() { m_func(); }

    DeferClass(const DeferClass& e) = delete;
    DeferClass& operator=(const DeferClass& e) = delete;

private:
    F m_func;
};

#define _CONCAT(a, b) a##b
#define _MAKE_DEFER_(line) DeferClass _CONCAT(defer_placeholder, line) = [&]()

#undef DEFER
#define DEFER _MAKE_DEFER_(__LINE__)

void DPrintf(const char* format, ...);

void myAssert(bool condition, std::string message = "Assertion failed!");

template <typename... Args>
std::string format(const char* format_str, Args... args){
    // 设将可变参数... 按照format格式化成字符串 并将字符串复制到str中 size是要写入的字符的最大数目
    // 末尾多一个 \0
    int size_s = std::snprintf(nullptr, 0, format_str, args...) + 1;
    if(size_s <= 0){
        throw std::runtime_error("Error during formatting.");
    }
    auto size = static_cast<size_t>(size_s);
    std::vector<char> buf(size);
    // data返回数组第一个元素的指针 该指针在数组内部使用
    std::snprintf(buf.data(), size, format_str, args...);
    return std::string(buf.data(), buf.data() + size - 1);   // remove '\0'
}

std::chrono::_V2::system_clock::time_point now();

std::chrono::milliseconds getRandomizedElectionTimeout();
void sleepNMilliseconds(int N);


// //////////////////////////////// 异步写日志的日志队列
// read is blockiong!!! Like go chan
template <typename T>
class LockQueue{
public:
    // 多个woker线程都会写日志queue
    void Push(const T& data){
        // 使用lock_gurad, 即RAII的思想保证锁正确释放
        std::lock_gurad<std::mutex> lock(m_mutex);
        m_queue.push(data);
        m_condvariable.notify_one();
    }

    // 一个线程读日志queue 写日志文件
    T Pop(){
        std::unique_lock<std::mutex> lock(m_mutex);
        while(m_queue.empty()){
            // 这里用unique_lock的原因是lock_guard不支持解锁 而unique_lock支持
            m_condvariable.wait(lock);
        }
        T data = m_queue.front();
        m_queue.pop();
        return data;
    }

// 添加一个超时时间参数 默认为50ms
bool timeOutPop(int timeout, T* ResData){
    std::unique_lock<std::mutex> lock(m_mutex);

    // 获取当前时间点，并计算出超时时刻
    auto now = std::chrono::system_clock::now();
    auto timeout_time = now + std::chrono::milliseconds(timeout);

    // 在超时之前 不断检查队列是否为空
    while(m_queue.empty()){
        // 如果已经超时了 就返回一个空对象
        if(m_condvariable.wait_until(lock, timeout_time) == std::cv_status::timeout){
            return false;
        }
        else{
            continue;
        }
    }

    T data = m_queue.front();
    m_queue.pop();
    *ResData = data;
    return true;
}

private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condvariable;
};

// 这个Op是kv传递给raft的command
class Op{
public:
    std::string Operation;       // "Get" "Put" "Append"
    std::string Key;
    std::string Value;
    std::string CliendId;        // 客户端号码
    int RequestId;               // 客户端号码请求的Request的序列号 为了保证线性一致性

public:
    std::string asString() const{
        std::stringstream ss;
        boost::archive::text_oarchive oa(ss);

        // write class instance to archive
        oa << *this;
        // close archive
        return ss.str();
    }

    bool parseFromString(std::string str){
        std::stringstream iss(str);
        boost::archive::text_iarchive ia(iss);
        // read class state from archive
        ia >> *this;
        // 这里涉及到解析失败要如何处理  需要看一下boost库
        return true;
    }

public:
    friend std::ostream& operator<<(std::ostream& os, const Op& obj){
        os << "[MyClass:Operation{" + obj.Operation + "}, Key{" + obj.key + "}, Value{" + obj.Value + "}, CliendId{" + obj.CliendId + "}, ResquestId{" + std::to_string(obj.ResquestId) + "}";
        return os;
    }

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version){
        ar& Operation;
        ar& Key;
        ar& Value;
        ar& CliendId;
        ar& ResquestId;
    }
};

// ///////////////////////////////// kvserver reply err to clerk

const std::string OK = "OK";
const std::string ErrNokey = "ErrNokey";
const std::string ErrWrongLeader = "ErrWrongLeader";

// ////////////////////////////////////获取可用的端口
bool isReleasePort(unsigned short usPort);

bool getReleasePort(short& port);

}
#endif
