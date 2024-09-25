/*************************************************************************
	> File Name: skipList.h
	> Author: duan 
	> Mail: w_duan2001@163.com
	> Created Time: Tue 24 Sep 2024 02:37:03 PM CST
 ************************************************************************/

#ifndef _SKIPLIST_H
#define _SKIPLIST_H

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <mutex>
#include <vector>
#include <string>

#define STORE_FILE "../store/dumpFile"

static std::string delimiter = ":";

namespace duan{

// Class template to implement node
template <typename K, typename V>
class Node{
public:
    Node(){}

    Node(K k, V v, int);

    ~Node();

    K get_key() const;
    V get_value() const;

    void set_value(V);

    // Linear array to hold pointers to next node of different level
    Node<K, V> **forward;
    int node_level;
private:
    K key;
    V value;
};

template <typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level){
    this->key = k;
    this->value = v;
    this->node_level = level;

    // 数组大小是level + 1， 因为数组下标是从0 - level
    this->forward = new Node<K, V>* [level + 1];
    // 初始化数组
    memset(this->forward, 0, sizeof(Node<K, V> *) * (level + 1));
}

template <typename K, typename V>
Node<K, V>::~Node(){
    delete[] forward;
}

template <typename K, typename V>
K Node<K, V>::get_key() const{
    return key;
}

template <typename K, typename V>
V Node<K, V>::get_value() const{
    return value;
}

template <typename K, typename V>
void Node<K, V>::set_value(V value){
    this->value = value;
}

// Class template for Skip list
template <typename K, typename V>
class SkipList{
public:
    SkipList(int);
    ~SkipList();
    int get_random_level();
    Node<K, V>* create_node(K, V, int);
    int insert_element(K, V);
    void display_list();
    bool search_element(K);
    void delete_element(K);
    void insert_set_element(K&, V&);
    void dump_file();
    void load_file();
    // 递归删除结点
    void clear(Node<K, V> *);
    int size();

private:
    void get_key_value_from_string(const std::string &str, std::string* key, std::string* value);
    bool is_valid_string(const std::string& str);

private:
    // 跳表的最大层数
    int _max_level;
    // 跳表当前层数
    int _skip_list_level;
    // 头结点
    Node<K, V>* _header;

    // 文件操作用到的文件流
    std::ofstream _file_writer;
    std::ifstream _file_reader;

    // 跳表元素的数量
    int _element_count;

    std::mutex _mtx;
};

// 创建新的结点
template <typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(const K k, const V v, int level){
    Node<K, V>* n = new Node<K, V>(k, v, level);
    return n;
}

// 插入k-v到跳表
// return 1 意味着元素已经存在
// return 0 意味着元素插入成功
template <typename K, typename V>
int SkipList<K, V>::insert_element(const K key, const V value){
    _mtx.lock();
    Node<K, V> * current = this->_header;
    // 创建一个数组 用来存放需要更新的结点
    // update是一个数组，
    Node<K, V>* update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V> *) * (_max_level + 1));

    // 从跳表的最大level(每次跳这么多)开始
    // 跳表的每一个结点都有自己的forward 它存储的就是当前结点(即步数0) 以及每隔当前level步的下一个结点
    for(int i = _skip_list_level; i >= 0; i--){
        while(current->forward[i] != NULL && current->forward[i]->get_key() < key){
            current = current->forward[i];
        }
        // 此时的current后面要进行更新
        update[i] = current;
    }
    // 定位到当前层 第一个>=key的结点
    current = current->forward[0];
    // 若可以找到 则存在 返回1
    if(current != NULL && current->get_key() == key){
        std::cout << "key: " << key << ", exists" << std::endl;
        _mtx.unlock();
        return 1;
    }

    // 如果current为空 意味着没有此元素  且需要在最高level添加
    // 如果current不为空 但key不相等 意味着我们不得不添加结点在update[0]到current之间
    if(current == NULL || current->get_key() != key){
        // 给结点生成一个随机level
        int random_level = get_random_level();
        if(random_level > _skip_list_level){
            for(int i = _skip_list_level + 1; i < random_level + 1; ++i){
                update[i] = _header;
            }
            _skip_list_level = random_level;
        }
        Node<K, V>* inserted_node = create_node(key, value, random_level);
        for(int i = 0; i <= random_level; ++i){
            inserted_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = inserted_node;
        }
        std::cout << "Successfully inserted key:" << key << ", value:" << value << std::endl;
        _element_count++;
    }
    _mtx.unlock();
    return 0;
}

// 打印skip list
template <typename K, typename V>
void SkipList<K, V>::display_list(){
    std::cout << "\n*****Skip List*****"
              << "\n";
    for(int i = 0; i <= _skip_list_level; ++i){
        Node<K, V>* node = this->_header->forward[i];
        std::cout << "Level " << i << ":";
        while(node != NULL){
            std::cout << node->get_key() << ":" << node->get_value() << ";";
            node = node->forward[i];
        }
        std::cout << std::endl;
    }
}

// 将数据从内存放到文件
template<typename K, typename V>
void SkipList<K, V>::dump_file(){
    std::cout << "dump_file--------------------" << std::endl;
    _file_writer.open(STORE_FILE);
    Node<K, V>* node = this->_header->forward[0];

    while(node != nullptr){
        _file_writer << node->get_key() << ":" << node->get_value() << "\n";
        std::cout << node->get_key() << ":" << node->get_value() << ";\n";
        node = node->forward[0];
    }

    _file_writer.flush();
    _file_writer.close();
    return;
}

// 从磁盘加载数据
template <typename K, typename V>
void SkipList<K, V>::load_file(){
    _file_reader.open(STORE_FILE);
    std::cout << "load_file---------------------" << std::endl;
    std::string line;
    std::string* key = new std::string();
    std::string* value = new std::string();
    while(getline(_file_reader, line)){
        get_key_value_from_string(line, key, value);
        if(key->empty() || value->empty()){
            continue;
        }
        insert_element(stoi(*key), *value);
        std::cout << "key:" << *key << "value:" << *value << std::endl;
    }
    delete key;
    delete value;
    _file_reader.close();
}

template <typename K, typename V>
int SkipList<K, V>::size(){
    return _element_count;
}

template<typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string& str, std::string* key, std::string* value){
    if(!is_valid_string(str)){
        return;
    }
    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter) + 1, str.length());
}

template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string& str){
    if(str.empty()){
        return false;
    }
    if(str.find(delimiter) == std::string::npos){
        return false;
    }
    return true;
}

// 从跳表中删除元素
template <typename K, typename V>
void SkipList<K, V>::delete_element(K key){
    _mtx.lock();
    Node<K, V>* current = this->_header;
    Node<K, V>* update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V> *) * (_max_level + 1));

    for(int i = _skip_list_level; i >= 0; i--){
        while(current->forward[i] != NULL && current->forward[i]->get_key() < key){
            current = current->forward[i];
        }
        update[i] = current;
    }
    current = current->forward[0];
    if(current != NULL && current->get_key() == key){
        // 从最底层开始 删除每一次的该结点
        for(int i = 0; i <= _skip_list_level; i++){
            if(update[i]->forward[i] != current) break;
            // 如果等于key 就要删除它 后续就要更新key的前一个结点 让其指向key的后一个结点
            update[i]->forward[i] = current->forward[i];
        }
        while(_skip_list_level > 0 && _header->forward[_skip_list_level] == 0){
            _skip_list_level--;
        }

        std::cout << "Successfully delete key " << key << std::endl;
        delete current;
        _element_count--;
    }
    _mtx.unlock();
    return;
}

/*
*作用与 insert_element相同类似
*insert_element是插入新元素
*insert_set_element是插入元素，如果元素存在则改变其值
*/
template <typename K, typename V>
void SkipList<K, V>::insert_set_element(K& key, V& value){
    V oldValue;
    if(search_element(key, oldValue)){
        delete_element(key);
    }
    insert_element(key, value);
}

template <typename K, typename V>
bool SkipList<K, V>::search_element(K key){
    std::cout << "search_element----------------------" << std::endl;
    Node<K, V>* current = _header;

    for(int i = _skip_list_level; i >= 0; i--){
        while(current->forward[i] && current->forward[i]->get_key() < key){
            current = current->forward[i];
        }
    }

    current = current->forward[0];

    if(current and current->get_key() == key){
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return true;
    }

    std::cout << "Not Found key:" << key << std::endl;
    return false;
}

// 跳表的构造函数
template <typename K, typename V>
SkipList<K, V>::SkipList(int max_level){
    this->_max_level = max_level;
    this->_skip_list_level = 0;
    this->_element_count = 0;

    K k;
    V v;
    this->_header = new Node<K, V>(k, v, _max_level);
}

template <typename K, typename V>
SkipList<K, V>::~SkipList(){
    if(_file_writer.is_open()){
        _file_writer.close();
    }
    if(_file_reader.is_open()){
        _file_reader.close();
    }

    if(_header->forward[0] != nullptr){
        clear(_header->forward[0]);
    }
    delete(_header);
}

template <typename K, typename V>
void SkipList<K, V>::clear(Node<K, V>* cur){
    if(cur->forward[0] != nullptr){
        clear(cur->forward[0]);
    }
    delete(cur);
}

template <typename K, typename V>
int SkipList<K, V>::get_random_level(){
    int k = 1;
    while(rand() % 2){
        k++;
    }
    k = (k < _max_level) ? k : _max_level;
    return k;
}

}
#endif
