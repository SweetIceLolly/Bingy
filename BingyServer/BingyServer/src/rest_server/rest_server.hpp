/*
����: REST HTTP ��������ؽӿ�
����: ����
�ļ�: rest_server.hpp
*/

#pragma once

#if !defined(_MSC_VER)
#include "mongoose.h"
#else
extern "C" {
#include "mongoose.h"
}
#endif

#include <unordered_map>
#include <string>
#include <functional>

// ��������
typedef std::function<void(mg_connection *connection, int &ev, mg_http_message *ev_data, void *fn_data)> handler;

// ����������Ϣ
typedef struct _handlerInfo {
    std::string     method;
    handler         eventHandler;
} handlerInfo;

// ����ĳ������������¼
typedef std::unordered_map<std::string, handlerInfo>::iterator handler_identifier;

class rest_server {
public:
    // ���������
    handler_identifier addHandler(const std::string &method, const std::string &path, const handler &eventHandler);

    // �Ƴ�������
    void removeHandler(const handler_identifier &item);

    // ����������
    void startServer(const std::string &connStr, void *userdata);

    // �ڲ�ʹ��, ƥ����ʵ���������. �ú�����֤����ֵ��Ϊ NULL
    handler matchHandler(const std::string &method, const std::string &path);

private:
    // ·��
    std::unordered_map<std::string, handlerInfo> router;

    // �������Ƿ�����ֹͣ
    bool stopping = false;
};

typedef struct _dispatcherInfo {
    rest_server  *ptrToClass;
    void        *userdata;
} dispatcherInfo;
