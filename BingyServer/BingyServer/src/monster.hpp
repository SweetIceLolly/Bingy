/*
����: Bingy ������
����: ����
�ļ�: monster.hpp
*/

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "utils.hpp"

using LL = long long;

extern std::string monsterConfigPath;

class monsterDrop {
public:
    LL          id;
    double      chance;
};

class monsterData {
public:
    LL                          id;
    std::string                 name;
    LL                          atk, def, brk, agi, hp; // ��������Ѫ
    LL                          coin;                   // ����Ӳ��
    LL                          exp;                    // ��������
    LL                          dungeonWeight;          // �ڸ����еĳ��ָ���Ȩ��
    LL                          forestWeight;           // ��ɭ���еĳ��ָ���Ȩ��
    std::string                 message;                // ������Ϣ
    std::vector<monsterDrop>    drop;                   // ����
    luckyDraw                   dropDraw;               // �����ȡ��

    void initDropDraw();                                // ���ݵ�ǰ�ĵ����б��������ɵ����ȡ��
};

class dungeonData {
public:
    LL                              level;              // ������
    std::vector<LL>                 monsters;           // �����еĹ��� ID �б�
    luckyDraw                       monstersDraw;       // ���������ȡ��
};

extern std::unordered_map<LL, monsterData> allMonsters; // ע��: ��ȡ��ʱ����Բ��ü���, ���ǲ�Ҫʹ��[], ��Ҫʹ�� at(). ���߳�д���ʱ��������
extern std::unordered_map<LL, dungeonData> allDungeons; // ע��: ��ȡ��ʱ����Բ��ü���, ���ǲ�Ҫʹ��[], ��Ҫʹ�� at(). ���߳�д���ʱ��������
extern luckyDraw                           forestDraw;  // ɭ�ֳ齱��

bool bg_load_monster_config();                          // ��ȡ��������
void bg_init_monster_chances();                         // ��ʼ��������ֺ͵������
