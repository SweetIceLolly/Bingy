/*
����: �ֵ���ͷ�ļ�, Ϊ Bingy �ṩ�ֵ�����ؽӿ�
����: ����
�ļ�: trie.hpp
*/

#pragma once

#include <unordered_map>
#include <string>

// �ֵ����ڵ�
template <typename T>
class trieNode {
public:
    trieNode() {};
    std::unordered_map<char, trieNode *> child;
    T handler;                        // �����Ӧ������
};

// �ֵ�����
template <typename T>
class bg_trie {
public:
    bg_trie() {
        root = new trieNode<T>();
    }

    ~bg_trie() {
        delete root;
    }

    // ���ֵ������������Ŀ
    void addMsg(const std::string &str, const T &handler);

    // ���ֵ����л�ȡ�����Ӧ����. ���û���ҵ���Ӧ������, �򷵻� nullptr
    T getMsgHandler(const std::string &str) const;

private:
    trieNode<T> *root;
};

template <typename T>
void bg_trie<T>::addMsg(const std::string &str, const T &handler) {
    if (!root)
        throw std::exception("Trie root is NULL!");         // ϣ�����ᷢ�������°�
    if (str.empty())
        return;

    trieNode<T> *curr = root;
    for (const auto &ch : str) {
        // ���ֵ���һ��һ����Ҷ�Ӧ���ַ�
        auto it = curr->child.find(ch);
        if (curr->child.end() == it) {
            // �Ҳ�����Ӧ�ַ�, �����½ڵ�
            auto newNode = new trieNode<T>();
            curr->child.insert({ ch, newNode });
            curr = newNode;
        }
        else {
            // �ҵ���Ӧ�ַ�, ʹ��������Ϊ�ڵ�
            curr = it->second;
        }
    }

    // ���ö�Ӧ����Ĵ�����
    curr->handler = handler;
}

template <typename T>
T bg_trie<T>::getMsgHandler(const std::string &str) const {
    if (!root)
        throw std::exception("Trie root is NULL!");         // ϣ�����ᷢ�������°�
    if (str.empty())
        return nullptr;

    trieNode<T> *curr = root;
    for (const auto &ch : str) {
        // ���ֵ���һ��һ����Ҷ�Ӧ���ַ�
        auto it = curr->child.find(ch);
        if (curr->child.end() == it) {
            // �Ҳ�����Ӧ�ַ�
            return nullptr;
        }
        else {
            // �ҵ���Ӧ�ַ�, ʹ��������Ϊ�ڵ�
            curr = it->second;
        }
    }

    // ��ȡ��Ӧ����Ĵ�����
    return curr->handler;
}
