/*
描述: 一些辅助函数的接口
作者: 冰棍
文件: utlis.cpp
*/

#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <random>
#include <mutex>

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

// 时间日期类
class dateTime {
private:
    tm      _date;
    time_t  _timestamp;

public:
    // 使用当前系统时间
    dateTime() {
        _timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        _date = *std::localtime(&_timestamp);
    }

    // 使用自定义日期时间
    // 注意, 本程序不会帮你处理无效的时间. 请保证输入的参数有效
    dateTime(const int &year, const int &month, const int &day, const int &hour = 0, const int &minute = 0, const int &second = 0) {
        _date.tm_year = year - 1900;    // 注意, tm_year 从 1900 年开始计算
        _date.tm_mon = month - 1;       // 注意, tm_month 从 0 开始计算
        _date.tm_mday = day;
        _date.tm_hour = hour;
        _date.tm_min = minute;
        _date.tm_sec = second;
        _timestamp = std::mktime(&_date);
    }

    // 接受时间戳(以秒为单位)作为参数
    dateTime(const time_t &timestamp) {
        _timestamp = timestamp;
        _date = *std::localtime(&_timestamp);
    }

    // 复制类
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

    // 假若当前类所代表的是本地时间, 返回当前时间戳所对应的 UTC 时间戳
    time_t get_utc_timestamp() const {
        return std::mktime(std::gmtime(&_timestamp));
    }

    // 直接获取当前类所代表的时间戳
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

// 检查两个 dateTime 是否为连续日期. b 所代表的日期必须在 a 之后
bool is_day_sequential(const dateTime &a, const dateTime &b);

// 线程安全地获取一个 [min, max] 内的随机数
LL rndRange(const LL &min, const LL &max);

// 线程安全地获取一个 [0, max] 内的随机数
LL rndRange(const LL &max);

// 线程安全的抽奖机
class luckyDraw {
private:
    std::vector<std::pair<LL, std::pair<LL, LL>>>   items;  // (物品 ID, (起始序号, 结束序号))
    LL          maxIndex;                                   // 抽奖序号范围
    LL          currIndex;                                  // 当前抽奖序号
    std::mutex  mutexItems;                                 // 线程锁

public:
    luckyDraw() : maxIndex(0), currIndex(0) {};
    luckyDraw(const luckyDraw &b) : items(b.items), maxIndex(b.maxIndex), currIndex(b.currIndex) {};

    // 添加一个条目到抽奖机. 抽中它的概率为: weight / (所有条目的 weight 总和)
    void insertItem(const LL &itemId, const LL &weight);

    // 从抽奖机移除一个条目. 若找不到指定的条目, 函数返回 false
    bool removeItem(const LL &itemId);

    // 从抽奖机中抽取一个条目
    LL draw();

    // 如果物品列表里有大量物品, 而且要进行非常多 (例如上万) 次的抽奖, 则使用这个函数能显著提升执行效率
    // 不建议对于少量物品 (例如一百个) 和次数少 (例如十次) 的抽奖使用这个函数, 因为二分搜索对于少量物品的性能不如线性搜索
    LL massive_draw();
};
