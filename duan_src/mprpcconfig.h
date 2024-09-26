/*************************************************************************
	> File Name: mprpcconfig.h
	> Author: duan 
	> Mail: w_duan2001@163.com
	> Created Time: Thu 26 Sep 2024 03:16:47 PM CST
 ************************************************************************/

#ifndef _MPRPCCONFIG_H
#define _MPRPCCONFIG_H

#include <string>
#include <unordered_map>

namespace duan{

// rpcserverip   rpcserverport   zookeeperip   zookeeperport
// 框架读取配置文件类
class MprpcConfig{
public:
    // 负责解析加载配置文件
    void LoadConfigFile(const char* config_file);

    // 查询配置项信息
    std::string Load(const std::string& key);

private:
    std::unordered_map<std::string, std::string> m_configMap;
    // 去掉字符串前后的空格
    void Trim(std::string& src_buf);
};

}
#endif
