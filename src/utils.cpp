/*
描述: 一些辅助函数
作者: 冰棍
文件: utlis.cpp
*/

#include "utils.hpp"
#include <iostream>
#include <cqcppsdk/cqcppsdk.hpp>
#include <mutex>
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

// 初始化随机数产生器
std::mt19937_64 rndGen(std::chrono::high_resolution_clock::now().time_since_epoch().count());
std::mutex mutexRndGen;

void console_log(const std::string &msg, const LogType &type) {
#ifdef WIN32
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

#ifdef WIN32
    // 为 Windows 控制台恢复输出颜色
    SetConsoleTextAttribute(hConsole, WHITE);
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
            throw std::exception("无效的 at 指令");
        else
            return std::stoll(str.substr(10));
    }
    else
        return std::stoll(str);
}

// --------------------------------------------------------------

LL rndRange(const LL &min, const LL &max) {
    std::scoped_lock<std::mutex> lock(mutexRndGen);
    std::uniform_int_distribution<std::mt19937_64::result_type> rnd(min, max);
    return rnd(rndGen);
}

LL rndRange(const LL &max) {
    std::scoped_lock<std::mutex> lock(mutexRndGen);
    std::uniform_int_distribution<std::mt19937_64::result_type> rnd(0, max);
    return rnd(rndGen);
}

// --------------------------------------------------------------

bool is_leap_year(const int &year) {
    if (year % 4 == 0) {
        if (year % 100 == 0)
            return (year % 400 == 0);
        else
            return true;
    }
    else
        return false;
}

bool is_day_sequential(const dateTime &a, const dateTime &b) {
    const int year_a = a.get_year(), month_a = a.get_month(), day_a = a.get_day(),
        year_b = b.get_year(), month_b = b.get_month(), day_b = b.get_day();

    if (day_b == 1) {
        // 如果 b 的天数是 1, 那么检查 a 是否为上一个月的最后一天

        if (month_b == 1) {
            // 如果 b 的月份是 1, 那么检查 a 是否为上一年的 12 月 31 日
            return (year_b == year_a + 1 && month_a == 12 && day_a == 31);
        }
        else {
            // 如果 b 的月份不是 1, 那么检查 a 是否为上一月的最后一天
            unsigned char month_days[] = { 0, 31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
            month_days[2] = is_leap_year(year_a) ? 29 : 28;             // 处理润年的二月天数

            return (year_b == year_a && month_b == month_a + 1 && day_a == static_cast<int>(month_days[month_a]));
        }
    }
    else {
        // 如果 b 的天数不是 1, 那么检查 a 是否为这个月对应的上一天
        return (year_b == year_a && month_b == month_a && day_b == day_a + 1);
    }
}

// --------------------------------------------------------------

void luckyDraw::insertItem(const LL &itemId, const LL &weight) {
    std::scoped_lock<std::mutex> lock(mutexItems);
    maxIndex = currIndex + weight;
    items.push_back(std::make_pair(itemId, std::make_pair(currIndex, maxIndex)));
    currIndex = maxIndex;
}

bool luckyDraw::removeItem(const LL &itemId) {
    std::scoped_lock<std::mutex> lock(mutexItems);
    for (auto it = items.begin(); it != items.end(); ++it) {
        if (it->first == itemId) {
            // 把后面条目的序号前移
            LL weight = it->second.second - it->second.first;
            for (auto it_after = it + 1; it_after != items.end(); ++it_after) {
                it_after->second.first -= weight;
                it_after->second.second -= weight;
            }

            // 修改总数量
            maxIndex -= weight;
            currIndex -= weight;

            // 移除条目
            items.erase(it);

            return true;
        }
    }
    return false;
}

LL luckyDraw::draw() {
    LL rnd = rndRange(maxIndex - 1);

    for (const auto &it : items) {
        // 格式: (物品 ID, (起始序号, 结束序号))
        if (it.second.first <= rnd && rnd < it.second.second)
            return it.first;
    }
    return LLONG_MIN;       // 希望不会来到这里吧...
}

LL luckyDraw::massive_draw() {
    LL rnd = rndRange(maxIndex - 1);

    // 对物品进行二分搜索
    // 格式: (物品 ID, (起始序号, 结束序号))
    // 若 起始序号 <= rnd < 结束序号, 则抽中对应物品
    size_t start = 0, end = items.size(), curr = (start + end) / 2;
    while (end > start) {
        if (items[curr].second.first <= rnd && rnd < items[curr].second.second)
            // 刚好落在范围内
            return items[curr].first;
        else if (rnd < items[curr].second.first) {
            // 目标范围在当前范围的前面
            end = curr - 1;
            curr = (start + end) / 2;
        }
        else {
            // 目标范围在当前范围的后面
            start = curr + 1;
            curr = (start + end) / 2;
        }
    }
    return LLONG_MIN;       // 希望不会来到这里吧...
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
                throw std::exception("无效的字符串");
        }
        numStarted = true;
    }
    return std::stoll(str.substr(start, i - start + 1));
}

std::string eqiType_to_str(const EqiType &type) {
    const std::string names[] = { "头盔", "战甲", "护腿", "战靴", "主武器", "副武器", "耳环", "戒指", "项链", "宝石" };
    return names[static_cast<LL>(type)];
}
