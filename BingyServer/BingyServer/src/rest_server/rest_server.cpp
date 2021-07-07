/*
����: REST HTTP ���������ʵ��
����: ����
�ļ�: rest_server.cpp
*/

#include "rest_server.hpp"

// Ĭ����������
static void builtInHandler(mg_connection *connection, int ev, mg_http_message *ev_data, void *fn_data);

// �����ɷ�
static void httpRequestDispatch(struct mg_connection *connection, int ev, void *ev_data, void *fn_data);

// �� str �������ַ�ת��Ϊ��д
inline void str_ucase(std::string &str) {
    for (auto &ch : str)
        if (ch < 128)
            ch &= ~32;
}

handler_identifier rest_server::addHandler(const std::string& method, const std::string& path, const handler& eventHandler) {
    handlerInfo info;
    info.eventHandler = eventHandler;
    info.method = method;
    str_ucase(info.method);
    return this->router.insert({ path, info }).first;
}

void rest_server::removeHandler(const handler_identifier& item) {
    this->router.erase(item);
}

void rest_server::startServer(const std::string& connStr, void *userdata) {
    struct mg_mgr mgr;
    dispatcherInfo info;

    info.ptrToClass = this;
    info.userdata = userdata;

    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, connStr.c_str(), httpRequestDispatch, &info);

    for (;;) {
        if (this->stopping) {
            break;
        }
        mg_mgr_poll(&mgr, 1000);
    }
    mg_mgr_free(&mgr);
}

handler rest_server::matchHandler(const std::string& method, const std::string& path) {
    auto info = this->router.find(path);

    if (info != this->router.end()) {
        // ƥ����������
        if (info->second.method.at(0) == '\0' || method == info->second.method) {
            return info->second.eventHandler;
        }
        else
            return builtInHandler;
    }
    else
        return builtInHandler;
}

static void builtInHandler(mg_connection *connection, int ev, mg_http_message *ev_data, void *fn_data) {
    mg_printf(connection,
        "HTTP/1.1 404\r\n"
        "Content-Length: 9\r\n"
        "\r\n"
        "Not found"
    );
}

static void httpRequestDispatch(struct mg_connection *connection, int ev, void *ev_data, void *fn_data) {
    rest_server *ptrToClass = static_cast<dispatcherInfo*>(fn_data)->ptrToClass;

    if (ev == MG_EV_HTTP_MSG) {
        // ���� HTTP ����
        struct mg_http_message *httpMsg = static_cast<mg_http_message*>(ev_data);

        // ƥ���Ӧ�Ĵ�����
        auto reqMethod = std::string(httpMsg->method.ptr, httpMsg->method.len);
        str_ucase(reqMethod);
        auto handler = ptrToClass->matchHandler(
            reqMethod,
            std::string(httpMsg->uri.ptr, httpMsg->uri.len)
        );
        handler(connection, ev, static_cast<mg_http_message*>(ev_data), fn_data);
    }
}
