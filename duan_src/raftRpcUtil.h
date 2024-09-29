/*************************************************************************
	> File Name: raftRpcUtil.h
	> Author: duan 
	> Mail: w_duan2001@163.com
	> Created Time: Sun 29 Sep 2024 10:27:48 AM CST
 ************************************************************************/

#ifndef _RAFTRPCUTIL_H
#define _RAFTRPCUTIL_H

#include "raftRPC.pb.h"
namespace duan{

// 维护当前结点对其他某一个结点的所以rpc发送通信的功能
// 对于一个raft结点来说 对于任意其他的结点都要维护一个rpc连接 即MprpcChannel
class RaftRpcUtil{
private:
    raftRpcProtoc::raftRpc_Stub *stub_;

public:
    // 主动调用其他结点的三个方法，但是别的结点调用自己的好像就不行了，
    // 要继承protoc提供的service类才行
    bool AppendEntries(raftRpcProtoc::AppendEntriesArgs *args, raftRpcProtoc::AppendEntriesReply *response);
    bool InstallSnapshot(raftRpcProtoc::InstallSnapshotRequest *args, raftRpcProtoc::InstallSnapshotResponse *response);
    bool RequestVote(raftRpcProtoc::RequestVoteArgs *args, raftRpcProtoc::RequestVoteReply *response);

    // 响应其他结点的方法
    // ip 远端ip   port 远端端口
    RaftRpcUtil(std::string ip, short port);
    ~RaftRpcUtil();
};

}
#endif
