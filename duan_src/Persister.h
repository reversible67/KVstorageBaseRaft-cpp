/*************************************************************************
	> File Name: Persister.h
	> Author: duan 
	> Mail: w_duan2001@163.com
	> Created Time: Wed 25 Sep 2024 03:48:57 PM CST
 ************************************************************************/

#ifndef _PERSISTER_H
#define _PERSISTER_H
#include <fstream>
#include <mutex>

namespace duan{

class Persister{
private:
    std::mutex m_mtx;
    std::string m_raftState;
    std::string m_snapshot;
    const std::string m_raftStateFileName;
    const std::string m_snapshotFileName;
    std::ofstream m_raftStateOutStream;
    std::ofstream m_snapshotOutStream;
    long long m_raftStateSize;

public:
    void Save(std::string raftstate, std::string snapshot);
    std::string readSnapshot();
    void SaveRaftState(const std::string& data);
    long long raftStateSize();
    std::string ReadRaftState();
    explicit Persister(int me);
    ~Persister();

private:
    void clearRaftState();
    void clearSnapshot();
    void clearRaftStateAndSnapshot();
};

}
#endif
