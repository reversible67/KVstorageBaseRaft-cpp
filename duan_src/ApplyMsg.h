/*************************************************************************
	> File Name: ApplyMsg.h
	> Author: duan 
	> Mail: w_duan2001@163.com
	> Created Time: Wed 25 Sep 2024 03:37:00 PM CST
 ************************************************************************/

#ifndef _APPLYMSG_H
#define _APPLYMSG_H
#include <string>
namespace duan{

// 存储应用消息相关信息的类
class ApplyMsg{
public:
    // 表示命令是否有效
    bool CommandValid;
    // 存储命令
    std::string Command;
    // 存储命令的索�?
    int CommandIndex;
    // 表示快照是否有效
    bool SnapshotValid;
    // 存储快照
    std::string Snapshot;
    // 存储快照的任�?
    int SnapshotTerm;
    // 存储快照的索�?
    int SnapshotIndex;

public:
    // 两个valid最开始要赋予false！！
    ApplyMsg()
        : CommandValid(false)
        , Command(),
        , CommandIndex(-1),
        , SnapshotValid(false),
        , SnapshotTerm(-1),
        , SnapshotIndex(-1){
    }
};

}
#endif
