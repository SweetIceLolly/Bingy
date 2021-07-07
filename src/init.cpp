/*
����: ��ʼ����ش���
����: ����
�ļ�: init.cpp
*/

#include "init.hpp"
#include "msg_router.hpp"
#include "msg_handlers.hpp"
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
#include <iostream>
#include <fstream>

#define CONFIG_FILE_PATH    "bgConfig.txt"

inline void bg_msgrouter_init();
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
    console_log("�ɹ���ȡ�����ļ�: ����" + std::to_string(allSignInEvents.size()) + "��ǩ���, " + std::to_string(allSynthesises.size()) + "��װ���ϳ�");

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

    // ע����Ϣ·��
    bg_msgrouter_init();

    // ���� BgKeepAlive

    // ��ʼ�����
    bgInitialized = true;
    return true;
}

// ע����������
inline void bg_msgrouter_init() {
    // ע��Ⱥ���������
    bg_groupmsg_router_add("bg", bg_cmd_bg);
    bg_groupmsg_router_add("bg ע��", bg_cmd_register);
    bg_groupmsg_router_add("bg ǩ��", bg_cmd_sign_in);

    bg_groupmsg_router_add("bg �鿴Ӳ��", bg_cmd_view_coins);
    bg_groupmsg_router_add("bg �鿴����", bg_cmd_view_inventory);
    bg_groupmsg_router_add("bg �鿴����", bg_cmd_view_properties);

    bg_groupmsg_router_add("bg �鿴װ��", bg_cmd_view_equipments);
    bg_groupmsg_router_add("bg װ��", bg_cmd_equip);
    bg_groupmsg_router_add("bg ����", bg_cmd_pawn);

    bg_groupmsg_router_add("bg ж��ͷ��", bg_cmd_unequip_helmet);
    bg_groupmsg_router_add("bg ж��ս��", bg_cmd_unequip_body);
    bg_groupmsg_router_add("bg ж�»���", bg_cmd_unequip_leg);
    bg_groupmsg_router_add("bg ж��սѥ", bg_cmd_unequip_boot);
    bg_groupmsg_router_add("bg ж�»���", bg_cmd_unequip_armor);

    bg_groupmsg_router_add("bg ж��������", bg_cmd_unequip_primary);
    bg_groupmsg_router_add("bg ж�¸�����", bg_cmd_unequip_secondary);
    bg_groupmsg_router_add("bg ж������", bg_cmd_unequip_weapon);

    bg_groupmsg_router_add("bg ж�¶���", bg_cmd_unequip_earrings);
    bg_groupmsg_router_add("bg ж�½�ָ", bg_cmd_unequip_rings);
    bg_groupmsg_router_add("bg ж������", bg_cmd_unequip_necklace);
    bg_groupmsg_router_add("bg ж�±�ʯ", bg_cmd_unequip_jewelry);
    bg_groupmsg_router_add("bg ж����Ʒ", bg_cmd_unequip_ornament);

    bg_groupmsg_router_add("bg ж��", bg_cmd_unequip_item);
    bg_groupmsg_router_add("bg ж��װ��", bg_cmd_unequip_item_2);
    bg_groupmsg_router_add("bg ж������", bg_cmd_unequip_all);
    bg_groupmsg_router_add("bg ж������װ��", bg_cmd_unequip_all);
    bg_groupmsg_router_add("bg ж��ȫ��", bg_cmd_unequip_all);
    bg_groupmsg_router_add("bg ж��ȫ��װ��", bg_cmd_unequip_all);

    bg_groupmsg_router_add("bg ǿ��ͷ��", bg_cmd_upgrade_helmet);
    bg_groupmsg_router_add("bg ǿ��ս��", bg_cmd_upgrade_body);
    bg_groupmsg_router_add("bg ǿ������", bg_cmd_upgrade_leg);
    bg_groupmsg_router_add("bg ǿ��սѥ", bg_cmd_upgrade_boot);
    bg_groupmsg_router_add("bg ����ͷ��", bg_cmd_upgrade_helmet);
    bg_groupmsg_router_add("bg ����ս��", bg_cmd_upgrade_body);
    bg_groupmsg_router_add("bg ��������", bg_cmd_upgrade_leg);
    bg_groupmsg_router_add("bg ����սѥ", bg_cmd_upgrade_boot);

    bg_groupmsg_router_add("bg ǿ��������", bg_cmd_upgrade_primary);
    bg_groupmsg_router_add("bg ǿ��������", bg_cmd_upgrade_secondary);
    bg_groupmsg_router_add("bg ����������", bg_cmd_upgrade_primary);
    bg_groupmsg_router_add("bg ����������", bg_cmd_upgrade_secondary);

    bg_groupmsg_router_add("bg ǿ������", bg_cmd_upgrade_earrings);
    bg_groupmsg_router_add("bg ǿ����ָ", bg_cmd_upgrade_rings);
    bg_groupmsg_router_add("bg ǿ������", bg_cmd_upgrade_necklace);
    bg_groupmsg_router_add("bg ǿ����ʯ", bg_cmd_upgrade_jewelry);
    bg_groupmsg_router_add("bg ��������", bg_cmd_upgrade_earrings);
    bg_groupmsg_router_add("bg ������ָ", bg_cmd_upgrade_rings);
    bg_groupmsg_router_add("bg ��������", bg_cmd_upgrade_necklace);
    bg_groupmsg_router_add("bg ������ʯ", bg_cmd_upgrade_jewelry);
    bg_groupmsg_router_add("bg ȷ��", bg_cmd_confirm_upgrade);

    bg_groupmsg_router_add("bg ǿ������", bg_cmd_upgrade_help);
    bg_groupmsg_router_add("bg ǿ������", bg_cmd_upgrade_help);
    bg_groupmsg_router_add("bg ǿ����Ʒ", bg_cmd_upgrade_help);
    bg_groupmsg_router_add("bg ǿ��װ��", bg_cmd_upgrade_help);
    bg_groupmsg_router_add("bg ǿ��", bg_cmd_upgrade_help);
    bg_groupmsg_router_add("bg ��������", bg_cmd_upgrade_help);
    bg_groupmsg_router_add("bg ��������", bg_cmd_upgrade_help);
    bg_groupmsg_router_add("bg ������Ʒ", bg_cmd_upgrade_help);
    bg_groupmsg_router_add("bg ����װ��", bg_cmd_upgrade_help);
    bg_groupmsg_router_add("bg ����", bg_cmd_upgrade_help);

    bg_groupmsg_router_add("bg ���׳�", bg_cmd_view_trade);
    bg_groupmsg_router_add("bg �鿴���׳�", bg_cmd_view_trade);
    bg_groupmsg_router_add("bg ����", bg_cmd_view_trade);
    bg_groupmsg_router_add("bg ����", bg_cmd_buy_trade);
    bg_groupmsg_router_add("bg �ϼ�", bg_cmd_sell_trade);
    bg_groupmsg_router_add("bg �¼�", bg_cmd_recall_trade);
    
    bg_groupmsg_router_add("bg �ϳ�", bg_cmd_synthesis);
    bg_groupmsg_router_add("bg ��ս", bg_cmd_fight);
    bg_groupmsg_router_add("bg ��սɭ��", nullptr);
    bg_groupmsg_router_add("bg pvp", nullptr);
    bg_groupmsg_router_add("bg vip", nullptr);

    bg_groupmsg_router_add("bg /addcoins", bg_cmd_admin_add_coins);
    bg_groupmsg_router_add("bg /addherocoin", bg_cmd_admin_add_heroCoin);
    bg_groupmsg_router_add("bg /addlevel", bg_cmd_admin_add_level);
    bg_groupmsg_router_add("bg /addblessing", bg_cmd_admin_add_blessing);
    bg_groupmsg_router_add("bg /addenergy", bg_cmd_admin_add_energy);
    bg_groupmsg_router_add("bg /addexp", bg_cmd_admin_add_exp);
    bg_groupmsg_router_add("bg /addinvcapacity", bg_cmd_admin_add_invCapacity);
    bg_groupmsg_router_add("bg /addvip", bg_cmd_admin_add_vip);

    // ע��˽���������

}

// ��ȡ��Ϸ����
inline bool bg_load_config() {
    configParser parser(CONFIG_FILE_PATH);
    signInEvent *signInEv = nullptr;        // ǩ�����ʱ����
    synthesisInfo *synInfo = nullptr;       // �ϳ���Ϣ��ʱ����
    dungeonData *dungeon = nullptr;         // ����������ʱ����

    return parser.load(
        // �л� state �ص�����
        [](const std::string &line, char &state) -> bool {
            // state: 0: һ������; 1: ǩ���; 2: װ���ϳ�; 3: ��������
            if (line == "[ǩ���]")
                state = 1;
            else if (line == "[װ���ϳ�]")
                state = 2;
            else if (line == "[��������]")
                state = 3;
            else
                state = 0;
            return true;
        },

        // ��ȡ����ֵ�ص�����
        [&](const std::string &propName, const std::string &propValue, const char &state, const unsigned int &lineNo) -> bool {
            // ����һ������
            if (state == 0) {
                if (propName == "dburi")                              // ���ݿ� URI
                    dbUri = propValue + std::string("?authSource=admin");
                else if (propName == "dbname")                        // ���ݿ���
                    dbName = propValue;
                else if (propName == "monsters")                      // ��������·��
                    monsterConfigPath = propValue;
                else if (propName == "equipments")                    // װ������·��
                    eqiConfigPath = propValue;
                else if (propName == "admin") {                       // ����Ա
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
                    console_log(std::string("δ֪��������: \"") + propName + std::string("\", ����") + std::to_string(lineNo), LogType::warning);
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
                        signInEv->minute = (char)std::stoi(propValue);
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
                        console_log(std::string("δ֪��������: \"") + propName + std::string("\", ����") + std::to_string(lineNo), LogType::warning);
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
                        console_log(std::string("δ֪��������: \"") + propName + std::string("\", ����") + std::to_string(lineNo), LogType::warning);
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
                        console_log(std::string("δ֪��������: \"") + propName + std::string("\", ����") + std::to_string(lineNo), LogType::warning);
                }
                catch (const std::exception &e) {
                    console_log("��Ӹ�������ʧ��, ��������ʱ��������: ����" + std::to_string(lineNo) + ", ԭ��: " + e.what(), LogType::warning);
                }
                catch (...) {
                    console_log("��Ӹ�������ʧ��, ��������ʱ��������: ����" + std::to_string(lineNo), LogType::warning);
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

            return true;
        },

        // ������ǻص�����
        [&](const char &state) -> bool {
            if (state == 1) {
                allSignInEvents.push_back(*signInEv);
                delete signInEv;
            }
            else if (state == 2) {
                allSynthesises.insert({ synInfo->targetId, *synInfo });
                delete synInfo;
            }
            else if (state == 3) {
                allDungeons.insert({ dungeon->level, *dungeon });
            }

            return true;
        }
    );
}
