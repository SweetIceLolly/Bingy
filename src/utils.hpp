/*
描述: 一些辅助函数的接口
作者: 冰棍
文件: utlis.cpp
*/

#pragma once

#include <string>
#include <vector>
#include "equipment.hpp"

using LL = long long;

// 控制台日志类型
enum class LogType : unsigned char {
    info,
    warning,
    error
};

// 日志相关
void console_log(const std::string &msg, const LogType &type = LogType::info);

// 字符串相关
std::string str_trim(const std::string &str);
std::vector<std::string> str_split(const std::string &str, const char &delimiter);
void str_lcase(std::string &str);
LL qq_parse(const std::string &str);

// 检查一个字符串是否为整数. 如果不是, 则抛出异常; 如果是, 则返回对应的整数
LL str_to_ll(const std::string &str);

// 获取 type 所对应的装备类型的名称
std::string eqiType_to_str(const EqiType &type);