/*
描述: Bingy 配置文件读取相关函数
作者: 冰棍
文件: config_parser.hpp
*/

#include "config_parser.hpp"
#include "utils.hpp"
#include <fstream>

configParser::configParser(std::string path) {
    this->path = path;
}

bool configParser::load(
    std::function<bool(const std::string &line, char &state)> stateChangeCallback,
    std::function<bool(const std::string &propName, const std::string &propValue, char state, unsigned int lineNo)> propGetCallback,
    std::function<bool(char state)> beginCallback,
    std::function<bool(char state)> endCallback
) {
    std::ifstream  file;
    file.open(this->path);
    if (!file.is_open())
        return false;

    // 逐行读取配置文件
    std::string     line;
    unsigned int    currLine = 0;
    char            state = 0;

    while (std::getline(file, line)) {
        ++currLine;
        line = str_trim(line);
        if (line.empty())               // 忽略空行
            continue;

        if (line[0] == '[') {           // 设置状态行
            if (!stateChangeCallback(line, state))
                return false;
            continue;
        }
        else if (line == "begin") {    // 开始标记
            if (!beginCallback(state))
                return false;
        }
        else if (line == "end") {      // 结束标记
            if (!endCallback(state))
                return false;
        }
        else {
            auto pos = line.find_first_of('=');
            std::string key = line.substr(0, pos);
            std::string val = line.substr(pos + 1);
            str_lcase(key);
            if (!propGetCallback(key, val, state, currLine))
                return false;
        }
    }

    return true;
}
