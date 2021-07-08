/*
����: һЩ��������
����: ����
�ļ�: utils.cpp
*/

#include "utils.hpp"
#include <codecvt>
#include <iostream>
#include <mutex>
#include "equipment.hpp"

#ifdef _WIN32
#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif
#include <Windows.h>
#define RED         12
#define YELLOW      14
#define WHITE       15
#endif

#define TOKEN_LEN   32

// ��ʼ�������������
std::mt19937_64 rndGen(std::chrono::high_resolution_clock::now().time_since_epoch().count());
std::mutex mutexRndGen;

void console_log(const std::string &msg, const LogType &type) {
#ifdef _WIN32
    // Ϊ Windows ����̨�޸������ɫ
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

    // ���Ĳ�ʹ�� string_to_coolq ת�������
    switch (type) {
    case LogType::info:
        std::cout << "[��Ϣ] " + msg << "\n";
        break;

    case LogType::warning:
        std::cout << "[����] " + msg << "\n";
        break;

    case LogType::error:
        std::cout << "[����] " + msg << "\n";
        break;
    }

#ifdef _WIN32
    // Ϊ Windows ����̨�ָ������ɫ
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
            str[i] |= 32;           // �������ĸ, ��ת��ΪСд��ĸ
    }
}

std::string generate_token() {
    const char ch[] =
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string rtn;
    for (int i = 0; i < TOKEN_LEN; ++i) {
        rtn += ch[rndRange(sizeof(ch) - 1)];
    }
    return rtn;
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
        // ��� b �������� 1, ��ô��� a �Ƿ�Ϊ��һ���µ����һ��

        if (month_b == 1) {
            // ��� b ���·��� 1, ��ô��� a �Ƿ�Ϊ��һ��� 12 �� 31 ��
            return (year_b == year_a + 1 && month_a == 12 && day_a == 31);
        }
        else {
            // ��� b ���·ݲ��� 1, ��ô��� a �Ƿ�Ϊ��һ�µ����һ��
            unsigned char month_days[] = { 0, 31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
            month_days[2] = is_leap_year(year_a) ? 29 : 28;             // ��������Ķ�������

            return (year_b == year_a && month_b == month_a + 1 && day_a == static_cast<int>(month_days[month_a]));
        }
    }
    else {
        // ��� b ���������� 1, ��ô��� a �Ƿ�Ϊ����¶�Ӧ����һ��
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
            // �Ѻ�����Ŀ�����ǰ��
            LL weight = it->second.second - it->second.first;
            for (auto it_after = it + 1; it_after != items.end(); ++it_after) {
                it_after->second.first -= weight;
                it_after->second.second -= weight;
            }

            // �޸�������
            maxIndex -= weight;
            currIndex -= weight;

            // �Ƴ���Ŀ
            items.erase(it);

            return true;
        }
    }
    return false;
}

LL luckyDraw::draw() {
    LL rnd = rndRange(maxIndex - 1);

    for (const auto &it : items) {
        // ��ʽ: (��Ʒ ID, (��ʼ���, �������))
        if (it.second.first <= rnd && rnd < it.second.second)
            return it.first;
    }
    return LLONG_MIN;       // ϣ���������������...
}

LL luckyDraw::massive_draw() {
    LL rnd = rndRange(maxIndex - 1);

    // ����Ʒ���ж�������
    // ��ʽ: (��Ʒ ID, (��ʼ���, �������))
    // �� ��ʼ��� <= rnd < �������, ����ж�Ӧ��Ʒ
    size_t start = 0, end = items.size(), curr = (start + end) / 2;
    while (end > start) {
        if (items[curr].second.first <= rnd && rnd < items[curr].second.second)
            // �պ����ڷ�Χ��
            return items[curr].first;
        else if (rnd < items[curr].second.first) {
            // Ŀ�귶Χ�ڵ�ǰ��Χ��ǰ��
            end = curr - 1;
            curr = (start + end) / 2;
        }
        else {
            // Ŀ�귶Χ�ڵ�ǰ��Χ�ĺ���
            start = curr + 1;
            curr = (start + end) / 2;
        }
    }
    return LLONG_MIN;       // ϣ���������������...
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
                throw std::exception("��Ч���ַ���");
        }
        numStarted = true;
    }
    return std::stoll(str.substr(start, i - start + 1));
}

std::string eqiType_to_str(const EqiType &type) {
    const std::string names[] = { "ͷ��", "ս��", "����", "սѥ", "������", "������", "����", "��ָ", "����", "��ʯ" };
    return names[static_cast<LL>(type)];
}
