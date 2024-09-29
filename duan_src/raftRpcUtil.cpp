/*************************************************************************
	> File Name: raftRpcUtil.cpp
	> Author: duan 
	> Mail: w_duan2001@163.com
	> Created Time: Sun 29 Sep 2024 10:46:49 AM CST
 ************************************************************************/

#include "raftRpcUtil.h"
#include <mprpcchannel.h>
#include <mprpccontroller.h>

namespace duan{

bool RaftRpcUtil::AppendEntries(raftRpcProctoc::AppendEntriesArgs *args, raftRpcProctoc::AppendEntriesReply *response){
    MprpcController controller;
    stub_->AppendEntries(&controller, args, response, nullptr);
    return !controller.Failed();
}

bool RaftRpcUtil::InstallSnapshot(raftRpcProctoc::InstallSnapshotRequest *args, raftRpcProctoc::InstallSnapshotResponse *response){
    MprpcController controller;
    stub_->InstallSnapshot(&controller, args, response, nullptr);
    return !controller.Failed();
}

bool RaftRpcUtil::RequestVote(raftRpcProctoc::ResquestVoteArgs *args, raftRpcProctoc::ResquestVoteReply *response){
    MprpcController controller;
    stub_->RequestVote(&controller, args, response, nullptr);
    return !controller.Failed();
}

// 先开启服务器 再尝试连接其他的结点
// 中间给一个间隔时间 等待其他的rpc服务器结点启动
RaftRpcUtil::RaftRpcUtil(std::string ip, short port){
    // 发送rpc设置
    stub_ = new raftRpcProctoc::raftRpc_Stub(new MprpcChannel(ip, port, true));
}

RaftRpcUtil::~RaftRpcUtil() { delete stub_; }

}
