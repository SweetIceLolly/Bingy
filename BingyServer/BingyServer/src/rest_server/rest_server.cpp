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

// 把请求类型字符串 (长度最多为 8, 除非另有指定) 中所有字符转换为大写
inline void str_ucase(char *str) {
    for (unsigned int i = 0; i < 8; ++i) {
        if (str[i] == '\0')
            return;
        str[i] &= ~32;
    }
}

// 把请求类型字符串 (指定的长度) 中所有字符转换为大写
inline void str_ucase(char *str, int len) {
    for (unsigned int i = 0; i < len; ++i) {
        if (str[i] == '\0')
            return;
        str[i] &= ~32;
    }
}

handler_identifier rest_server::addHandler(const std::string& method, const std::string& path, const handler& eventHandler) {
    handlerInfo info;
    info.eventHandler = eventHandler;
    strncpy(info.method, method.c_str(), method.length());
    str_ucase(info.method);
    return this->router.insert({ path, info }).first;
}

void rest_server::removeHandler(const handler_identifier& item) {
    this->router.erase(item);
}

void rest_server::setPollHandler(const handler& eventHandler) {
    this->pollHandler = eventHandler;
}

void rest_server::removePollHandler() {
    this->pollHandler = nullptr;
}

void rest_server::startServer(const std::string& connStr, int pollFreq, void *userdata) {
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

handler rest_server::matchHandler(const struct mg_str &method, const std::string& path) {
    auto info = this->router.find(path);

    if (info != this->router.end()) {
        // 匹配请求类型
        if (strncmp(info->second.method, method.ptr, method.len) == 0) {
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
        str_ucase(const_cast<char*>(httpMsg->method.ptr), httpMsg->method.len);
        auto handler = ptrToClass->matchHandler(
            httpMsg->method,
            std::string(httpMsg->uri.ptr, httpMsg->uri.len)
        );
        handler(connection, ev, static_cast<mg_http_message*>(ev_data), fn_data);
    }
    else if (ev == MG_EV_POLL) {
        // 响应 poll 事件
        if (ptrToClass->pollHandler)
            ptrToClass->pollHandler(connection, ev, static_cast<mg_http_message *>(ev_data), fn_data);
    }
}

std::string get_query_param(const struct mg_str *query, const char *fieldName) {
    char buf[1024];
    int len = mg_http_get_var(query, fieldName, buf, 1024);
    if (len < 1)
        return "";
    return std::string(buf, len);
}
