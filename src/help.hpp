/*
描述: Bingy 帮助相关接口
作者: 冰棍
文件: help.hpp
*/

#include <string>

// 获取总体帮助
std::string bg_get_help_general();

// 获取帮助主题
std::string bg_get_help_topic(const std::string &topic);
