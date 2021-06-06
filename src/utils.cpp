/*
描述: 一些辅助函数
作者: 冰棍
文件: utlis.cpp
*/

#include "utils.hpp"
#include <iostream>
#include <cqcppsdk/cqcppsdk.hpp>

#ifdef WIN32
#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif
#include <Windows.h>
#define RED     12
#define YELLOW  14
#define WHITE   15
#endif

void console_log(const std::string &msg, const LogType &type) {
#ifdef WIN32
    // 为 Windows 控制台修改输出颜色
    static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    switch (type) {
    case LogType::info:
        SetConsoleTextAttribute(hConsole, WHITE);
        break;

    case LogType::warning:
        SetConsoleTextAttribute(hConsole, YELLOW);
        break;

    case LogType::error:
        SetConsoleTextAttribute(hConsole, RED);
        break;
    }
#endif

    // 中文不使用 string_to_coolq 转码会乱码
    switch (type) {
    case LogType::info:
        std::cout << cq::utils::string_to_coolq("[信息] " + msg) << "\n";
        break;

    case LogType::warning:
        std::cout << cq::utils::string_to_coolq("[警告] " + msg) << "\n";
        break;

    case LogType::error:
        std::cout << cq::utils::string_to_coolq("[错误] " + msg) << "\n";
        break;
    }

#ifdef WIN32
    // 为 Windows 控制台恢复输出颜色
    SetConsoleTextAttribute(hConsole, WHITE);
#endif
}

std::string str_trim(const std::string &str) {
    size_t start = str.find_first_not_of(' ');
    size_t end = str.find_last_not_of(' ');

    if (std::string::npos != end)
        return str.substr(start, end - start + 1);
    else
        return "";
}

std::vector<std::string> str_split(const std::string &str, const char &delimiter) {
    std::vector<std::string> rtn;
    size_t last = 0;
    size_t next = 0;
    
    while ((next = str.find(delimiter, last)) != std::string::npos) {
        rtn.push_back(str.substr(last, next - last));
        last = next + 1;
    }
    rtn.push_back(str.substr(last));

    return rtn;
}

void str_lcase(std::string &str) {
    for (size_t i = 0; i < str.length(); ++i) {
        if ((str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= 'a' && str[i] <= 'z'))
            str[i] |= 32;           // 如果是字母, 则转换为小写字母
    }
}
