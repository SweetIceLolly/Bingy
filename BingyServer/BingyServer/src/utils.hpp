/*
����: һЩ���������Ľӿ�
����: ����
�ļ�: utlis.cpp
*/

#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <random>
#include <mutex>
#include "equipment.hpp"

using LL = long long;

// ����̨��־����
enum class LogType : unsigned char {
    info,
    warning,
    error
};

// �������, ���� QQ Ⱥ�ź���Һ�
typedef struct _requestParams {
    int todo;
} requestParams;

// ��־���
void console_log(const std::string &msg, const LogType &type = LogType::info);

// �ַ������

std::string str_trim(const std::string &str);

template <typename TStr, typename TDel>
std::vector<TStr> str_split(const TStr &str, const TDel &delimiter) {
    std::vector<TStr> rtn;
    size_t last = 0;
    size_t next = 0;

    while ((next = str.find(delimiter, last)) != std::string::npos) {
        rtn.push_back(str.substr(last, next - last));
        last = next + 1;
    }
    rtn.push_back(str.substr(last));

    return rtn;
}

template <typename TStr>
void str_lcase(TStr &str) {
    for (size_t i = 0; i < str.length(); ++i) {
        if ((str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= 'a' && str[i] <= 'z'))
            str[i] |= 32;           // �������ĸ, ��ת��ΪСд��ĸ
    }
}

LL qq_parse(const std::string &str);

// ʱ��������
class dateTime {
private:
    tm      _date;
    time_t  _timestamp;

public:
    // ʹ�õ�ǰϵͳʱ��
    dateTime() {
        _timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        _date = *std::localtime(&_timestamp);
    }

    // ʹ���Զ�������ʱ��
    // ע��, �����򲻻���㴦����Ч��ʱ��. �뱣֤����Ĳ�����Ч
    dateTime(const int &year, const int &month, const int &day, const int &hour = 0, const int &minute = 0, const int &second = 0) {
        _date.tm_year = year - 1900;    // ע��, tm_year �� 1900 �꿪ʼ����
        _date.tm_mon = month - 1;       // ע��, tm_month �� 0 ��ʼ����
        _date.tm_mday = day;
        _date.tm_hour = hour;
        _date.tm_min = minute;
        _date.tm_sec = second;
        _timestamp = std::mktime(&_date);
    }

    // ����ʱ���(����Ϊ��λ)��Ϊ����
    dateTime(const time_t &timestamp) {
        _timestamp = timestamp;
        _date = *std::localtime(&_timestamp);
    }

    // ������
    dateTime(const dateTime &dt) {
        _timestamp = dt._timestamp;
        _date = dt._date;
    }

    int get_year() const {
        return _date.tm_year + 1900;
    }

    void set_year(int year) {
        _date.tm_year = year - 1900;
        _timestamp = std::mktime(&_date);
    }

    int get_month() const {
        return _date.tm_mon + 1;
    }

    void set_month(int month) {
        _date.tm_mon = month - 1;
        _timestamp = std::mktime(&_date);
    }

    int get_day() const {
        return _date.tm_mday;
    }

    void set_day(int day) {
        _date.tm_mday = day;
        _timestamp = std::mktime(&_date);
    }

    int get_hour() const {
        return _date.tm_hour;
    }

    void set_hour(int hour) {
        _date.tm_hour = hour;
        _timestamp = std::mktime(&_date);
    }

    int get_minute() const {
        return _date.tm_min;
    }

    void set_minute(int minute) {
        _date.tm_min = minute;
        _timestamp = std::mktime(&_date);
    }

    int get_second() const {
        return _date.tm_sec;
    }

    void set_second(int second) {
        _date.tm_sec = second;
        _timestamp = std::mktime(&_date);
    }

    // ������ǰ����������Ǳ���ʱ��, ���ص�ǰʱ�������Ӧ�� UTC ʱ���
    time_t get_utc_timestamp() const {
        return std::mktime(std::gmtime(&_timestamp));
    }

    // ֱ�ӻ�ȡ��ǰ���������ʱ���
    time_t get_timestamp() const {
        return _timestamp;
    }

    dateTime operator+ (const time_t &b) const {
        return dateTime(_timestamp + b);
    }

    dateTime operator- (const time_t &b) const {
        return dateTime(_timestamp - b);
    }

    dateTime &operator+= (const time_t &b) {
        _timestamp += b;
        _date = *std::localtime(&_timestamp);
        return *this;
    }

    dateTime &operator-= (const time_t &b) {
        _timestamp -= b;
        _date = *std::localtime(&_timestamp);
        return *this;
    }

    static dateTime get_utc_time() {
        return dateTime(dateTime().get_utc_timestamp());
    }
};

// ������� dateTime �Ƿ�Ϊ��������. b ����������ڱ����� a ֮��
bool is_day_sequential(const dateTime &a, const dateTime &b);

// �̰߳�ȫ�ػ�ȡһ�� [min, max] �ڵ������
LL rndRange(const LL &min, const LL &max);

// �̰߳�ȫ�ػ�ȡһ�� [0, max] �ڵ������
LL rndRange(const LL &max);

// �̰߳�ȫ�ĳ齱��
class luckyDraw {
private:
    std::vector<std::pair<LL, std::pair<LL, LL>>>   items;  // (��Ʒ ID, (��ʼ���, �������))
    LL          maxIndex;                                   // �齱��ŷ�Χ
    LL          currIndex;                                  // ��ǰ�齱���
    std::mutex  mutexItems;                                 // �߳���

public:
    luckyDraw() : maxIndex(0), currIndex(0) {};
    luckyDraw(const luckyDraw &b) : items(b.items), maxIndex(b.maxIndex), currIndex(b.currIndex) {};

    // ���һ����Ŀ���齱��. �������ĸ���Ϊ: weight / (������Ŀ�� weight �ܺ�)
    void insertItem(const LL &itemId, const LL &weight);

    // �ӳ齱���Ƴ�һ����Ŀ. ���Ҳ���ָ������Ŀ, �������� false
    bool removeItem(const LL &itemId);

    // �ӳ齱���г�ȡһ����Ŀ
    LL draw();

    // �����Ʒ�б����д�����Ʒ, ����Ҫ���зǳ��� (��������) �εĳ齱, ��ʹ�������������������ִ��Ч��
    // ���������������Ʒ (����һ�ٸ�) �ʹ����� (����ʮ��) �ĳ齱ʹ���������, ��Ϊ������������������Ʒ�����ܲ�����������
    LL massive_draw();
};

// ���һ���ַ����Ƿ�Ϊ����. �������, ���׳��쳣; �����, �򷵻ض�Ӧ������
LL str_to_ll(const std::string &str);

// ��ȡ type ����Ӧ��װ�����͵�����
std::string eqiType_to_str(const EqiType &type);
