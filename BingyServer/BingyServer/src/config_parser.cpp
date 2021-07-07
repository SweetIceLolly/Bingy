/*
����: Bingy �����ļ���ȡ��غ���
����: ����
�ļ�: config_parser.hpp
*/

#include "config_parser.hpp"
#include "utils.hpp"
#include <fstream>

configParser::configParser(std::string path) {
    this->path = path;
}

bool configParser::load(
    std::function<bool(const std::string &line, char &state)> stateChangeCallback,
    std::function<bool(const std::string &propName, const std::string &propValue, const char &state, const unsigned int &lineNo)> propGetCallback,
    std::function<bool(const char &state)> beginCallback,
    std::function<bool(const char &state)> endCallback
) {
    std::ifstream  file;
    file.open(this->path);
    if (!file.is_open())
        return false;

    // ���ж�ȡ�����ļ�
    std::string     line;
    unsigned int    currLine = 0;
    char            state = 0;

    while (std::getline(file, line)) {
        ++currLine;
        line = str_trim(line);
        if (line.empty())               // ���Կ���
            continue;

        if (line[0] == '[') {           // ����״̬��
            if (!stateChangeCallback(line, state))
                return false;
            continue;
        }
        else if (line == "begin") {    // ��ʼ���
            if (!beginCallback(state))
                return false;
        }
        else if (line == "end") {      // �������
            if (!endCallback(state))
                return false;
        }
        else {
            auto tmp = str_split(line, '=');
            str_lcase(tmp[0]);
            if (!propGetCallback(tmp[0], tmp[1], state, currLine))
                return false;
        }
    }

    return true;
}
