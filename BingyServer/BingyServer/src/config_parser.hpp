/*
描述: Bingy 配置文件读取接口
作者: 冰棍
文件: config_parser.hpp
*/

#pragma once

#include <string>
#include <functional>

class configParser {
private:
    std::string path;

public:
    configParser() = delete;
    configParser(configParser &) = delete;
    configParser(std::string path);

    /*
    读取配置文件. 以下为通用的回调函数原型:

    bool stateChangeCallback(const std::string &line, char &state)
    切换 state 回调函数
    line: 读取到的行
    state: 在回调函数中对该值进行修改以切换到另一个 state
    返回 false 以中断配置文件读取

    bool propGetCallback(const std::string &propName, const std::string &propValue, char state, const unsigned int &lineNo)
    属性行读取回调函数
    propName: 属性名
    propValue: 属性值
    state: 当前的 state
    lineNo: 当前所在行
    返回 false 以中断配置文件读取

    bool beginCallback(char state)
    读取到开始标记的回调函数
    state: 当前的 state

    bool endCallback(char state)
    读取到结束标记的回调函数
    state: 当前的 state
    */
    bool load(
        std::function<bool(const std::string &line, char &state)> stateChangeCallback,
        std::function<bool(const std::string &propName, const std::string &propValue, char state, const unsigned int &lineNo)> propGetCallback,
        std::function<bool(char state)> beginCallback,
        std::function<bool(char state)> endCallback
    );
};
