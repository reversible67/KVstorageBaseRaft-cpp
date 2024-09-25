/*************************************************************************
	> File Name: config.h
	> Author: duan 
	> Mail: w_duan2001@163.com
	> Created Time: Wed 25 Sep 2024 04:17:24 PM CST
 ************************************************************************/

#ifndef _CONFIG_H
#define _CONFIG_H
namespace duan{

const bool Debug = true;

// 时间单位：time.Millisecond，不同网络环境rpc速度不同，因此需要乘以一个系数
const int debugMul = 1;
// 心跳时间一般要比选举超时小一个数量级
const int HearBeatTimeout = 25 * debugMul;
const int minRandomizedElectionTime = 300 * debugMul;    // ms
const int maxRandomizedElectionTime = 500 * debugMul;    // ms

const int CONSENSUS_TIMEOUT = 500 * debugMul;            // ms

// 协程相关配置

const int FIBER_THREAD_NUM = 1;                     // 协程库中线程池大小
const bool FIBER_USE_CALLER_THREAD = false;         // 是否使用caller_thread执行调度任务

}
#endif
