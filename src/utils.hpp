/*
描述: 一些辅助函数的接口
作者: 冰棍
文件: utlis.cpp
*/

#pragma once

#include <string>
#include <vector>

// 控制台日志类型
enum class LogType {
    info,
    warning,
    error
};

void console_log(const std::string &msg, const LogType &type = LogType::info);

std::string str_trim(const std::string &str);
std::vector<std::string> str_split(const std::string &str, const char &delimiter);
void str_lcase(std::string &str);
