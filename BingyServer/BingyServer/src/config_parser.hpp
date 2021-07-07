/*
����: Bingy �����ļ���ȡ�ӿ�
����: ����
�ļ�: config_parser.hpp
*/

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
    ��ȡ�����ļ�. ����Ϊͨ�õĻص�����ԭ��:

    bool stateChangeCallback(const std::string &line, char &state)
    �л� state �ص�����
    line: ��ȡ������
    state: �ڻص������жԸ�ֵ�����޸����л�����һ�� state
    ���� false ���ж������ļ���ȡ

    bool propGetCallback(const std::string &propName, const std::string &propValue, const char &state, const unsigned int &lineNo)
    �����ж�ȡ�ص�����
    propName: ������
    propValue: ����ֵ
    state: ��ǰ�� state
    lineNo: ��ǰ������
    ���� false ���ж������ļ���ȡ

    bool beginCallback(const char &state)
    ��ȡ����ʼ��ǵĻص�����
    state: ��ǰ�� state

    bool endCallback(const char &state)
    ��ȡ��������ǵĻص�����
    state: ��ǰ�� state
    */
    bool load(
        std::function<bool(const std::string &line, char &state)> stateChangeCallback,
        std::function<bool(const std::string &propName, const std::string &propValue, const char &state, const unsigned int &lineNo)> propGetCallback,
        std::function<bool(const char &state)> beginCallback,
        std::function<bool(const char &state)> endCallback
    );
};
