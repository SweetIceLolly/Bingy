/*
描述: REST HTTP 服务器相关实现
作者: 冰棍
文件: rest_server.cpp
*/

#include "rest_server.hpp"

// 默认请求处理函数
static void builtInHandler(mg_connection *connection, int ev, mg_http_message *ev_data, void *fn_data);

// 请求派发
static void httpRequestDispatch(struct mg_connection *connection, int ev, void *ev_data, void *fn_data);

// 把 str 中所有字符转换为大写
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

void rest_server::startServer(const std::string& connStr, const int &pollFreq, void *userdata) {
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
        mg_mgr_poll(&mgr, pollFreq);
    }
    mg_mgr_free(&mgr);
}

void rest_server::stopServer() {
    this->stopping = true;
}

handler rest_server::matchHandler(const std::string& method, const std::string& path) {
    auto info = this->router.find(path);

    if (info != this->router.end()) {
        // 匹配请求类型
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
        // 处理 HTTP 请求
        struct mg_http_message *httpMsg = static_cast<mg_http_message*>(ev_data);

        // 匹配对应的处理函数
        auto reqMethod = std::string(httpMsg->method.ptr, httpMsg->method.len);
        str_ucase(reqMethod);
        auto handler = ptrToClass->matchHandler(
            reqMethod,
            std::string(httpMsg->uri.ptr, httpMsg->uri.len)
        );
        handler(connection, ev, static_cast<mg_http_message*>(ev_data), fn_data);
    }
}

std::string get_query_param(mg_http_message *ev_data, const char *fieldName) {
    char buf[255];
    int len = mg_http_get_var(&ev_data->query, fieldName, buf, 255);
    if (len < 1)
        return "";
    return std::string(buf, len);
}
