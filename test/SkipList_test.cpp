/*************************************************************************
	> File Name: SkipList_test.cpp
	> Author: duan 
	> Mail: w_duan2001@163,com
	> Created Time: Wed 25 Sep 2024 11:01:09 AM CST
 ************************************************************************/

#include<iostream>
#include "../duan_src/skipList.h"
#define FILE_PATH "./store/dumpFile"

int main(){
    duan::SkipList<int, std::string> skipList(6);
        skipList.insert_element(1, "学");
        skipList.insert_element(3, "算法");
        skipList.insert_element(7, "认准");
        skipList.insert_element(9, "灵茶山艾府");
        skipList.insert_element(10, "0x3f");
    std::cout << "skipList size:" << skipList.size() << std::endl;

    skipList.dump_file();

    skipList.search_element(9);
    skipList.search_element(10);


    skipList.display_list();

    skipList.delete_element(1);
    skipList.delete_element(10);

    std::cout << "skipList size:" << skipList.size() << std::endl;

    skipList.display_list();

    return 0;
}
