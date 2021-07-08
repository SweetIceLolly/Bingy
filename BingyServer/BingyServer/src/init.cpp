/*
����: ��ʼ����ش���
����: ����
�ļ�: init.cpp
*/

#include "init.hpp"
#include "database.hpp"
#include "utils.hpp"
#include "player.hpp"
#include "monster.hpp"
#include "game.hpp"
#include "signin_event.hpp"
#include "equipment.hpp"
#include "synthesis.hpp"
#include "config_parser.hpp"
#include "trade.hpp"
#include "http_auth.hpp"
#include <iostream>
#include <fstream>

#define CONFIG_FILE_PATH    "bgConfig.txt"

inline bool bg_load_config();

bool bgInitialized = false;     // Bingy �Ƿ�ɹ�����

// ��ʼ��������
bool bg_init() {
    // ���������ļ�
    console_log("���ڶ�ȡ�����ļ�...");
    if (!bg_load_config()) {
        console_log("��ȡ�����ļ�ʧ��!", LogType::error);
        return false;
    }
    console_log("�ɹ���ȡ�����ļ�: ����" + std::to_string(allSignInEvents.size()) + "��ǩ���, " + std::to_string(allSyntheses.size()) + "��װ���ϳ�");

    // ���ع�������
    console_log("���ڶ�ȡ��������...");
    if (!bg_load_monster_config()) {
        console_log("��ȡ���������ļ�ʧ��!", LogType::error);
        return false;
    }
    bg_init_monster_chances();
    console_log("�ɹ���ȡ��������: ����" + std::to_string(allMonsters.size()) + "������");

    // ����װ������
    console_log("���ڶ�ȡװ������...");
    if (!bg_load_equipment_config()) {
        console_log("��ȡװ�������ļ�ʧ��!", LogType::error);
        return false;
    }
    console_log("�ɹ���ȡװ������: ����" + std::to_string(allEquipments.size()) + "��װ��");

    // �������ݿ�
    console_log("�����������ݿ�...");
    if (!dbInit()) {
        console_log("�������ݿ�ʧ��!", LogType::error);
        return false;
    }
    console_log("�ɹ��������ݿ�");

    // ���������Ϣ
    console_log("���ڶ�ȡ�����������...");
    if (!bg_get_allplayers_from_db()) {
        console_log("��ȡ�������ʧ��!", LogType::error);
        return false;
    }
    console_log("�ɹ���ȡ�������: ����" + std::to_string(allPlayers.size()) + "�����");

    // ���ؽ��׳���Ϣ
    console_log("���ڶ�ȡ���׳�����...");
    bg_trade_get_items();
    console_log("�ɹ���ȡ���׳�����: ����" + std::to_string(allTradeItems.size()) + "����Ŀ, ��һ������ ID Ϊ" + std::to_string(bg_get_tradeId()));

    // ���� BgKeepAlive

    // ��ʼ�����
    bgInitialized = true;
    return true;
}

// ��ȡ��Ϸ����
inline bool bg_load_config() {
    configParser    parser(CONFIG_FILE_PATH);
    signInEvent     *signInEv = nullptr;            // ǩ�����ʱ����
    synthesisInfo   *synInfo = nullptr;             // �ϳ���Ϣ��ʱ����
    dungeonData     *dungeon = nullptr;             // ����������ʱ����
    std::string     httpAppId, httpAppSecret;       // HTTP �ͻ���������ʱ����

    return parser.load(
        // �л� state �ص�����
        [](const std::string &line, char &state) -> bool {
            // state: 0: һ������; 1: ǩ���; 2: װ���ϳ�; 3: ��������; 4: HTTP �ͻ�������
            if (line == "[ǩ���]")
                state = 1;
            else if (line == "[װ���ϳ�]")
                state = 2;
            else if (line == "[��������]")
                state = 3;
            else if (line == "[�ͻ�������]")
                state = 4;
            else
                state = 0;
            return true;
        },

        // ��ȡ����ֵ�ص�����
        [&](const std::string &propName, const std::string &propValue, const char &state, const unsigned int &lineNo) -> bool {
            // ����һ������
            if (state == 0) {
                if (propName == "dburi")                           // ���ݿ� URI
                    dbUri = propValue + std::string("?authSource=admin");
                else if (propName == "dbname")                     // ���ݿ���
                    dbName = propValue;
                else if (propName == "monsters")                   // ��������·��
                    monsterConfigPath = propValue;
                else if (propName == "equipments")                 // װ������·��
                    eqiConfigPath = propValue;
                else if (propName == "admin") {                    // ����Ա
                    try {
                        LL qq = std::stoll(propValue);
                        if (allAdmins.insert(qq).second)
                            console_log("�ɹ���ӹ���Ա: " + propValue);
                        else
                            console_log("�ѹ���Ա" + propValue + "��ӵ�����Ա�б�ʱ��������, ��������Ϊ�ظ���? ����" + std::to_string(lineNo), LogType::warning);
                    }
                    catch (...) {
                        console_log("�޷���\"" + propValue + "\"��ӵ�����Ա�б�, �����Ƿ�Ϊ��Ч��ֵ! ����" + std::to_string(lineNo), LogType::warning);
                    }
                }
                else
                    console_log(std::string("δ֪��������: \"") + propName +
                        std::string("\", ����") + std::to_string(lineNo), LogType::warning);
            }

            // ����ǩ���
            else if (state == 1) {                                          // ����
                try {
                    if (propName == "id")
                        signInEv->id = std::stoll(propValue);
                    else if (propName == "year")
                        signInEv->year = std::stoi(propValue);
                    else if (propName == "month")
                        signInEv->month = std::stoi(propValue);
                    else if (propName == "day")
                        signInEv->day = std::stoi(propValue);
                    else if (propName == "hour")
                        signInEv->hour = static_cast<char>(std::stoi(propValue));
                    else if (propName == "minute")
                        signInEv->minute = static_cast<char>(std::stoi(propValue));
                    else if (propName == "coinfactor")
                        signInEv->coinFactor = std::stod(propValue);
                    else if (propName == "energyfactor")
                        signInEv->energyFactor = std::stod(propValue);
                    else if (propName == "first")
                        signInEv->firstN = std::stoll(propValue);
                    else if (propName == "items") {
                        for (const auto &item : str_split(propValue, ',')) {
                            auto itemId = std::stoll(item);
                            if (itemId == -1)
                                break;
                            else
                                signInEv->items.push_back(itemId);
                        }
                    }
                    else if (propName == "message")
                        signInEv->message = propValue;
                    else
                        console_log(std::string("δ֪��������: \"") + propName +
                            std::string("\", ����") + std::to_string(lineNo), LogType::warning);
                }
                catch (const std::exception &e) {
                    console_log("����ǩ�������ʱ��������: ����" + std::to_string(lineNo) + ", ԭ��: " + e.what(), LogType::warning);
                }
                catch (...) {
                    console_log("����ǩ�������ʱ��������: ����" + std::to_string(lineNo), LogType::warning);
                }
            }

            // ����װ���ϳ���Ϣ
            else if (state == 2) {
                try {
                    if (propName == "requirements") {
                        for (const auto &item : str_split(propValue, ',')) {
                            auto itemId = std::stoll(item);
                            if (itemId == -1)
                                break;
                            else
                                synInfo->requirements.insert(itemId);
                        }
                    }
                    else if (propName == "coins")
                        synInfo->coins = std::stoll(propValue);
                    else if (propName == "target")
                        synInfo->targetId = std::stoll(propValue);
                    else
                        console_log(std::string("δ֪��������: \"") + propName +
                            std::string("\", ����") + std::to_string(lineNo), LogType::warning);
                }
                catch (const std::exception &e) {
                    console_log("��Ӻϳ���Ϣʧ��, ��������ʱ��������: ����" + std::to_string(lineNo) + ", ԭ��: " + e.what(), LogType::warning);
                }
                catch (...) {
                    console_log("��Ӻϳ���Ϣʧ��, ��������ʱ��������: ����" + std::to_string(lineNo), LogType::warning);
                }
            }

            // ����������
            else if (state == 3) {
                try {
                    if (propName == "level")
                        dungeon->level = std::stoll(propValue);
                    else if (propName == "monsters") {
                        for (const auto &idStr : str_split(propValue, ',')) {
                            const auto id = std::stoll(idStr);
                            dungeon->monsters.push_back(id);
                        }
                    }
                    else
                        console_log(std::string("δ֪��������: \"") + propName +
                            std::string("\", ����") + std::to_string(lineNo), LogType::warning);
                }
                catch (const std::exception &e) {
                    console_log("��Ӹ�������ʧ��, ��������ʱ��������: ����" + std::to_string(lineNo) + ", ԭ��: " + e.what(), LogType::warning);
                }
                catch (...) {
                    console_log("��Ӹ�������ʧ��, ��������ʱ��������: ����" + std::to_string(lineNo), LogType::warning);
                }
            }

            // ���� HTTP �ͻ�������
            else if (state == 4) {
                try {
                    if (propName == "appid")
                        httpAppId = propValue;
                    else if (propName == "secret")
                        httpAppSecret = propValue;
                    else
                        console_log(std::string("δ֪��������: \"") + propName +
                            std::string("\", ����") + std::to_string(lineNo), LogType::warning);
                }
                catch (const std::exception &e) {
                    console_log("��ӿͻ�������ʧ��, ��������ʱ��������: ����" + std::to_string(lineNo) + ", ԭ��: " + e.what(), LogType::warning);
                }
                catch (...) {
                    console_log("��ӿͻ�������ʧ��, ��������ʱ��������: ����" + std::to_string(lineNo), LogType::warning);
                }
            }

            return true;
        },

        // ��ʼ��ǻص�����
        [&](const char &state) -> bool {
            if (state == 1)
                signInEv = new signInEvent();
            else if (state == 2)
                synInfo = new synthesisInfo();
            else if (state == 3)
                dungeon = new dungeonData();
            else if (state == 4) {
                httpAppSecret = "";
                httpAppId = "";
            }

            return true;
        },

        // ������ǻص�����
        [&](const char &state) -> bool {
            if (state == 1) {
                allSignInEvents.push_back(*signInEv);
                delete signInEv;
            }
            else if (state == 2) {
                allSyntheses.insert({ synInfo->targetId, *synInfo });
                delete synInfo;
            }
            else if (state == 3) {
                allDungeons.insert({ dungeon->level, *dungeon });
            }
            else if (state == 4) {
                bg_http_add_app(httpAppId, httpAppSecret);
                console_log("�ɹ���� HTTP �ͻ���: " + httpAppId);
            }

            return true;
        }
    );
}
