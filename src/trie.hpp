/*
描述: 字典树头文件, 为 Bingy 提供字典树相关接口
作者: 冰棍
文件: trie.hpp
*/

#pragma once

#include <unordered_map>
#include <string>

// 字典树节点
template <typename T>
class trieNode {
public:
    trieNode() {};
    std::unordered_map<char, trieNode *> child;
    T handler;                        // 命令对应的内容
};

// 字典树类
template <typename T>
class bg_trie {
public:
    bg_trie() {
        root = new trieNode<T>();
    }

    ~bg_trie() {
        delete root;
    }

    // 往字典树中添加新项目
    void addMsg(const std::string &str, const T &handler);

    // 从字典树中获取命令对应内容. 如果没有找到对应的内容, 则返回 nullptr
    T getMsgHandler(const std::string &str) const;

private:
    trieNode<T> *root;
};

template <typename T>
void bg_trie<T>::addMsg(const std::string &str, const T &handler) {
    if (!root)
        throw std::exception("Trie root is NULL!");         // 希望不会发生这种事吧
    if (str.empty())
        return;

    trieNode<T> *curr = root;
    for (const auto &ch : str) {
        // 从字典树一层一层查找对应的字符
        auto it = curr->child.find(ch);
        if (curr->child.end() == it) {
            // 找不到对应字符, 插入新节点
            auto newNode = new trieNode<T>();
            curr->child.insert({ ch, newNode });
            curr = newNode;
        }
        else {
            // 找到对应字符, 使用它来作为节点
            curr = it->second;
        }
    }

    // 设置对应命令的处理函数
    curr->handler = handler;
}

template <typename T>
T bg_trie<T>::getMsgHandler(const std::string &str) const {
    if (!root)
        throw std::exception("Trie root is NULL!");         // 希望不会发生这种事吧
    if (str.empty())
        return nullptr;

    trieNode<T> *curr = root;
    for (const auto &ch : str) {
        // 从字典树一层一层查找对应的字符
        auto it = curr->child.find(ch);
        if (curr->child.end() == it) {
            // 找不到对应字符
            return nullptr;
        }
        else {
            // 找到对应字符, 使用它来作为节点
            curr = it->second;
        }
    }

    // 获取对应命令的处理函数
    return curr->handler;
}
