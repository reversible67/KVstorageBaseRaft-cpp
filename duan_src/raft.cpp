/*************************************************************************
	> File Name: raft.cpp
	> Author: duan 
	> Mail: w_duan2001@163.com
	> Created Time: Tue 08 Oct 2024 10:31:12 AM CST
 ************************************************************************/

#include "raft.h"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <memory>
#include "config.h"
#include "util.h"

void Raft::AppendEntries1(const raftRpcProctoc::AppendEntriesArgs* args, raftRpcProctoc::AppendEntriesReply* reply){
    std::lock_guard<std::mutex> locker(m_mtx);
    // 能接收到代表网络是正常的
    reply->set_appstate(AppNormal);
    if(args->term() < m_currentTerm){
        reply->set_success(false);
        reply->set_term(m_currentTerm);
        // 论文中：让领导人可以即时更新自己
        reply->set_updatenextindex(-100);
        DPrintf("[func->AppendEntries-rf{%d}] 拒绝??? 因为Leader{%d}的term{%v} < rf{%d}.term{%d}\n", m_me, args->leaderid(), args->term(), m_me, m_currentTerm);
        return;
    }

    // 由于这个局部变量创建在锁之后，因此执行persist的时候应该也是拿到锁???
    if(args->term() > m_currentTerm){
        // 三变 防止遗漏 无论什么时候都是三???
        m_status = Follower;
        m_currentTerm = args->term();
        m_votedFor = -1;

    }
    myAssert(args->term() == m_currentTerm, format("assert {args.Term == rf.currentTerm} fail"));
    m_status = Follower;
    m_lastResetElectionTime = now();

    // 不能无脑的从prevlogIndex开始阶段日??? 因为rpc可能会延迟，导致发过来的log是很久之前的
    // 那么就比较日??? 日志有三种情???
    if(args->prevlogIndex() > getLastLogIndex()){
        reply->set_success(false);
        reply->set_term(m_currentTerm);
        reply->set_updatenextindex(getLastLogIndex() + 1);
        return;
    }
    else if (args->prevlogindex() < m_lastSnapshotIncludeIndex) {
    // ????prevlogIndex??û?и??Ͽ???
    reply->set_success(false);
    reply->set_term(m_currentTerm);
    reply->set_updatenextindex(
        m_lastSnapshotIncludeIndex + 1);
    }
    if (matchLog(args->prevlogindex(), args->prevlogterm())) {
    //	todo??	????logs
    //??????ֱ?ӽضϣ?????һ??һ??????????Ϊ????��??log??????֮ǰ?ģ?ֱ?ӽضϿ??ܵ??¡?ȡ?ء??Ѿ???follower??־?е???Ŀ
    // ????˼?ǲ??ǿ??ܻ???һ?η?��??AE?е?logs??ǰ????ƥ???ģ??????ǲ?ƥ???ģ?????Ӧ?ã?1.follower???δ????? 2.???θ?leader?ظ?
    // 3. leader???δ???
        for (int i = 0; i < args->entries_size(); i++) {
            auto log = args->entries(i);
            if (log.logindex() > getLastLogIndex()) {
            m_logs.push_back(log);
            } 
            else{
                if (m_logs[getSlicesIndexFromLogIndex(log.logindex())].logterm() == log.logterm() &&
                m_logs[getSlicesIndexFromLogIndex(log.logindex())].command() != log.command()) {
                myAssert(false, format("[func-AppendEntries-rf{%d}] 两节点logIndex{%d}和term{%d}相同，但是其commmand{%d:%d}   "
                                 " {%d:%d}却不同！！\n",
                                 m_me, log.logindex(), log.logterm(), m_me,
                                 m_logs[getSlicesIndexFromLogIndex(log.logindex())].command(), args->leaderid(),
                                 log.command()));
                }
                if(m_logs[getSlicesIndexFromLogIndex(log.logindex())].logterm != log.logterm()){
                // 不匹配就更新
                    m_logs[getSlicesIndexFromLogIndex(log.logindex())] = log;
                }
            }
        }

        // 因为可能会收到过期的log！！！因此这里是大于等于
        myAssert(getLastLogIndex() >= args->prevlogIndex() + args->entries_size(), format("[func-AppendEntries1-rf{%d}]rf.getLastLogIndex(){%d} != args.PrevLogIndex{%d} + len(args.Entries){%d}", m_me, getLastLogIndex(). args->prevlogindex(), args->entries_size()));

        if(args->leadercommit() > m_commitIndex){
            m_commitIndex = std::min(args->leadercommit(), getLastLogIndex());
        }

    // 领导会一次发送完所有的日志
        myAssert(getLastLogIndex() >= m_commitIndex, format("[func-AppendEntries1-rf{%d}] rf.getLastLogIndex{%d} < rf.commitIndex{%d}". m_me, getLastLogIndex(), m_commitIndex));
        reply->set_success(true);
        reply->set_term(m_currentTerm);

        return;
    }
    else{
        reply->set_updatenextindex(args->prevlogindex());

        for(int index = args->prevlogindex(); index >= m_lastSnapshotIncludeIndex; --index){
            if(getLogTermFromLogIndex(index) != getLogTermFromLogIndex(args->prevlogindex())){
                reply->set_updatenextindex(index + 1);
                break;
            }
        }
        reply->set_success(false);
        reply->set_term(m_currentTerm);
        return;
    }
}

void Raft::applierTicker(){
    while(true){
        m_mtx.lock();
        if(m_status == Leader){
            DPrintf("[Raft::applierTicker() - raft{%d}] m_lastApplied{%d} m_commitIndex{%d}, m_me, m_lastApplied, m_commitIndex);
        }
        auto applyMsgs = getApplyLogs();
        m_mtx.unlock();
        if(!applyMsgs.empty()){
            DPrintf("[func - Raft::applierTicker()-raft{%d}] 向kvserver报告的applyMsgs长度为：{%d}", m_me, applyMsgs.size());
        }
        for(auto& message : applyMsgs){
            applyChan->Push(message);
        }
        sleepNMilliseconds(ApplyInterval);
    }
}
