/*
描述: 一些辅助函数
作者: 冰棍
文件: utlis.cpp
*/

#include "utils.hpp"
#include <iostream>
#include <cqcppsdk/cqcppsdk.hpp>
#include <equipment.hpp>

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
#ifdef _WIN32
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
#else
    // 为终端修改输出颜色
    switch (type) {
    case LogType::warning:
        std::cout << "\033[33m";
        break;

    case LogType::error:
        std::cout << "\033[31m";
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

#ifdef _WIN32
    // 为 Windows 控制台恢复输出颜色
    SetConsoleTextAttribute(hConsole, WHITE);
#else
    // 为终端恢复输出颜色
    std::cout << "\033[0m";
#endif
}

// --------------------------------------------------------------

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

LL qq_parse(const std::string &str) {
    if (str[0] == '[') {
        if (str.length() < 11)
            throw std::runtime_error("无效的 at 指令");
        else
            return std::stoll(str.substr(10));
    }
    else
        return std::stoll(str);
}

LL str_to_ll(const std::string &str) {
    bool numStarted = false;
    auto start = str.find_first_not_of(' ');
    auto i = start;

    for (i; i <= str.find_last_not_of(' '); ++i) {
        if (str[i] < '0' || str[i] > '9') {
            if (str[i] == '-' && !numStarted)
                continue;
            else
                throw std::runtime_error("无效的字符串");
        }
        numStarted = true;
    }
    return std::stoll(str.substr(start, i - start + 1));
}

std::string eqiType_to_str(const EqiType &type) {
    const std::string names[] = { "头盔", "战甲", "护腿", "战靴", "主武器", "副武器", "耳环", "戒指", "项链", "宝石", "一次性物品" };
    return names[static_cast<LL>(type)];
}
