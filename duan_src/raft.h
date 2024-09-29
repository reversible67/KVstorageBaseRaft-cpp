/*************************************************************************
	> File Name: raft.h
	> Author: duan 
	> Mail: w_duan2001@163.com
	> Created Time: Sun 29 Sep 2024 11:32:33 AM CST
 ************************************************************************/

#ifndef _RAFT_H
#define _RAFT_H

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include "ApplyMsg.h"
#include "Persister.h"
#include "boost/any.hpp"
#include "boost/serialization/serialization.hpp"
#include "config.h"
#include "duan.h"
#include "raftRpcUtil.h"
#include "util.h"

namespace duan{

// 网络状态表示
// 方便网络分区的时候debug 网络异常的时候为disconnected，只要网络正常就为AppNormal
// 防止matchIndex[]数组异常减小
constexpr int Disconnected = 0;
constexpr int AppNormal = 1;

// ////////////////////////投票状态
constexpr int killed = 0;
constexpr int Voted = 1;   // 本轮已经投过票了
constexpr int Expire = 2;  // 投票(消息、竞选者)过期
constexpr int Norkam = 3;

class Raft : public raftRpcProtoc::raftRpc{
private:
    std::mutex m_mtx;
    std::vector<std::shared_ptr<RaftRpcUtil>> m_peers;
    std::shared_ptr<Persister> m_persister;
    int m_me;
    int m_currentTerm;
    int m_votedFor;
    // 日志条目数组，包含了状态机要执行的指令集 以及收到领导时的任期号
    std::vector<raftRpcProtoc::LogEntry> m_logs;
    // 这两个状态所有结点都在维护 易失
    int m_commitIndex;
    int m_lastApplied;

    // 这两个状态是由服务器来维护 易失
    // 这两个状态的下标从1开始 因为通常commitIndex和lastApplied从0开始
    std::vector<int> m_nextIndex;
    std::vector<int> m_matchIndex;

    enum Status { Followe, Candidate, Leader };
    // 身份
    Status m_status;

    // client从这里取日志 client与raft通信的接口
    std::shared_ptr<LockQueue<ApplyMsg>> applyChan;

    // 选举超时
    std::chrono::_V2::system_clock::time_point m_lastResetElectionTime;
    // 心跳超时 用于leader
    std::chrono::_V2::system_clock::time_point m_lastResetHeartBeatTime;

    // 2D中用于传入快照点
    // 存储了快照中最后一个日志的Index 和 Term
    int m_lastSnapshotIncludeIndex;
    int m_lastSnapshotIncludeTerm;

    // 协程
    std::unique_ptr<IOManager> m_ioManager = nullptr;

public:
    void AppendEntries1(const raftRpcProtoc::AppendEntriesArgs *args, raftRpcProtoc::AppendEntriesReply * reply);
    void applierTicker();
    bool CondInstallSnapshot(int lastIncludeTerm, int lastIncludeIndex, std::string snapshot);
    void doElection();
    // 发起心跳 只有leader才需要发起心跳
    void doHeartBeat();
    // 每隔一段时间检查睡眠时间内有没有重置定时器 没有则说明超时了
    // 如果有则设置合适睡眠时间：睡眠到重置时间+超时时间
    void electionTimeOutTicker();
    std::vector<ApplyMsg> getApplyLogs();
    int getNewCommandIndex();
    void getPrevLogInfo(int server, int *preIndex, int *preTerm);
    void GetState(int *term, bool *isLeader);
    void InstallSnapshot(const raftRpcProtoc::InstallSnapshotRequest *args, raftRpcProtoc::InstallSnapshotResponse* reply);
    void leaderHeartBeatTicker();
    void leaderSendSnapsht(int server);
    void leaderUpdateCommitIndex();
    bool matchLog(int logIndex, int logTerm);
    void persist();
    void RequestVote(const raftRpcProtoc::RequestVoteArgs *args, raftRpcProtoc::RequestVoteReply *reply);
    bool UpToDate(int index, int term);
    int getLastLogIndex();
    int getLastLogTerm();
    void getLastLogIndexAndTerm(int *lastLogIndex, int *lastLogTerm);
    int getLogTermFromLogIndex(int logIndex);
    int GetRaftStateSize();
    int getSlicesIndexFromLogIndex(int logIndex);
    bool sendRequestVote(int server, std::shared_ptr<raftRpcProtoc::RequestVoteArgs> args, std::shared_ptr<raftRpcProtoc::RequestVoteReply> reply, std::shared_ptr<int> votedNum);
    bool sendAppendEntries(int server, std::shared_ptr<raftRpcProtoc::AppendEntriesArgs> args, std::shared_ptr<raftRpcProtoc::AppendEntriesReply> reply, std::shared_ptr<int> appendNums);

    void pushMsgToKvServer(ApplyMsg msg);
    void readPersist(std::string data);
    std::string persistData();

    void Start(Op command, int *newLogIndex, int *newLogTerm, bol *isLeader);

    // index代表是快照apply应用的index 而snapshot代表的是上层service传来的快照字节流 包括了index之前的数据
    // 这个函数的目的是把安装到快照里的日志抛弃 并安装快照数据 同时更新快照下标 输入peers自身主动更新 与leader发送快照不冲突
    // 即服务层主动发起请求raft保存snapshot里面的数据 index是用来表示snapshot快照到了哪条命令
    void Snapshot(int index, std::string snapshot);

public:
    // 重写基类方法 因为rpc远程调用真正调用的是这个方法
    // 序列化 反序列化等操作 rpc框架都已经做完了 因此这里只需要获取值 然后真正调用本地方法即可
    void AppendEntries(google::protobuf::RpcController *controller, const ::raftRpcProtoc::AppendEntriesArgs *request, ::raftRpcProtoc::AppendEntriesReply* response, ::google;:protobuf::Closure *done) override;
    void InstallSnapshot(google::protobuf::RpcController *controller, const ::raftRpcProtoc::InstallSnapshotRequest *request, ::raftRpcProtoc::InstallSnapshotResponse *response, ::google::protobuf::Closure *done) override;
    void RequestVote(goole::protobuf::RpcController *controller, const ::raftRpcProtoc::RequestVoteArgs *request, ::raftRpcProtoc::RequestVoteReply *response, ::google::protobuf::Closure *done) override;

public:
    void init(std::vector<std::shared_ptr<RaftRpcUtil>> peers, int me, std::shared_ptr<Persister> persister, std::shared_ptr<LockQueue<ApplyMsg>> applyChan);

private
    // for persist
    class BoostPersistRaftNode{
    public:
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive &ar, const unsigned int version){
            ar &m_currentTerm;
            ar &m_votedFor;
            ar &m_lastSnapshotIncludeIndex;
            ar &m_lastSnapshotIncludeTerm;
            ar &m_logs;
        }
        int m_currentTerm;
        int m_votedFor;
        int m_lastSnapshotIncludeIndex;
        int m_lastSnapshotIncludeTerm;
        std::vector<std::string> m_logs;
        std::unordered_map<std::string, int> umap;
    public:
    };
};
    
}
#endif
