/*
����: ���� Bingy ��ָ��, Ȼ����ж�Ӧ�ĺ���
����: ����
�ļ�: msg_handler.cpp
*/

#include "msg_handlers.hpp"
#include "game.hpp"
#include "utils.hpp"

// ���˺�
// �������Ҫȫ��ƥ��ź��ж�Ӧ�� pre �� post �ص�����
#define MATCH(str, callbackName)                                    \
    if (ev.message == "bg " str) {                                  \
        if (pre##callbackName##Callback(ev))                        \
            post##callbackName##Callback(ev);                       \
    }                                                               \
    else {                                                          \
        cq::send_group_message(GROUP_ID, bg_at(ev) +                \
            "�����ʽ����Ŷ! Ҫ" str "�Ļ�����\"bg " str "\"����");  \
    }

// ping
CMD(bg) {
    if (ev.message == "bg")
        cq::send_group_message(ev.target.group_id.value(), "����ѽ!");
}

// ע��
CMD(register) {
    MATCH("ע��", Register);
}

// �鿴Ӳ��
CMD(view_coins) {
    MATCH("�鿴Ӳ��", ViewCoins);
}

// ǩ��
CMD(sign_in) {
    MATCH("ǩ��", SignIn);
}

// �鿴����
CMD(view_inventory) {
    MATCH("�鿴����", ViewInventory);
}

// ����
CMD(pawn) {
    if (ev.message.length() < 9) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�����ʽ����Ŷ! ����ָ���ʽΪ: \"bg ���� �������1 �������2 ...\"");
        return;
    }
    auto params = str_split(ev.message.substr(9), ' ');         // ȥ�������ַ���, Ȼ���Կո�ָ�������
    if (params.size() > 0) {
        std::vector<LL> items;                                  // prePawnCallback ���صĴ���֮�������б�
        if (prePawnCallback(ev, params, items))
            postPawnCallback(ev, items);
    }
    else {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�����ʽ����Ŷ! ����ָ���ʽΪ: \"bg ���� �������1 �������2 ...\"");
    }
}

// �鿴����
CMD(view_properties) {
    MATCH("�鿴����", ViewProperties);
}

// �鿴װ��
CMD(view_equipments) {
    MATCH("�鿴װ��", ViewEquipments);
}

// װ��
CMD(equip) {
    if (ev.message.length() < 10) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�����ʽ����Ŷ! װ��ָ���ʽΪ: \"bg װ�� �������\"");
        return;
    }
    auto param = ev.message.substr(9);                          // ȥ�������ַ���, ֻ��������
    LL item = -1;
    if (preEquipCallback(ev, param, item))
        postEquipCallback(ev, item);
}

// ж��ͷ��
CMD(unequip_helmet) {
    MATCH("ж��ͷ��", UnequipHelmet);
}

// ж��ս��
CMD(unequip_body) {
    MATCH("ж��ս��", UnequipBody);
}

// ж�»���
CMD(unequip_leg) {
    MATCH("ж�»���", UnequipLeg);
}

// ж��սѥ
CMD(unequip_boot) {
    MATCH("ж��սѥ", UnequipBoot);
}

// ж�»���
CMD(unequip_armor) {
    MATCH("ж�»���", UnequipArmor);
}

// ж��������
CMD(unequip_primary) {
    MATCH("ж��������", UnequipPrimary);
}

// ж�¸�����
CMD(unequip_secondary) {
    MATCH("ж�¸�����", UnequipSecondary);
}

// ж������
CMD(unequip_weapon) {
    MATCH("ж������", UnequipWeapon);
}

// ж�¶���
CMD(unequip_earrings) {
    MATCH("ж�¶���", UnequipEarrings);
}

// ж�½�ָ
CMD(unequip_rings) {
    MATCH("ж�½�ָ", UnequipRings);
}

// ж������
CMD(unequip_necklace) {
    MATCH("ж������", UnequipNecklace);
}

// ж�±�ʯ
CMD(unequip_jewelry) {
    MATCH("ж�±�ʯ", UnequipJewelry);
}

// ж����Ʒ
CMD(unequip_ornament) {
    MATCH("ж����Ʒ", UnequipOrnament);
}

// ж��һ������Ʒ
CMD(unequip_item) {
    if (ev.message.length() < 10) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�����ʽ����Ŷ! ж��һ������Ʒָ���ʽΪ: \"bg ж�� �������\"\n"
            "����ж��ָ�����͵���Ʒ: ����: bg ж��ͷ�� (ֻж��ͷ��); bg ж����Ʒ (ж��������Ʒ); bg ж������ (ж������װ��)");
        return;
    }
    auto param = ev.message.substr(9);                          // ȥ�������ַ���, ֻ��������
    LL item = -1;
    if (preUnequipSingleCallback(ev, param, item))
        postUnequipSingleCallback(ev, item);
}

// ж��һ������Ʒ (��һ����)
CMD(unequip_item_2) {
    if (ev.message.length() < 14) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�����ʽ����Ŷ! ж��һ������Ʒָ���ʽΪ: \"bg ж�� �������\"\n"
            "����ж��ָ�����͵���Ʒ: ����: bg ж��ͷ�� (ֻж��ͷ��); bg ж����Ʒ (ж��������Ʒ); bg ж������ (ж������װ��)");
        return;
    }
    auto param = ev.message.substr(13);                         // ȥ�������ַ���, ֻ��������
    LL item = -1;
    if (preUnequipSingleCallback(ev, param, item))
        postUnequipSingleCallback(ev, item);
}

// ж������
CMD(unequip_all) {
    if (ev.message == "bg ж������" || ev.message == "bg ж��ȫ��" || ev.message == "bg ж������װ��" || ev.message == "bg ж��ȫ��װ��") {
        if (preUnequipAllCallback(ev))
            postUnequipAllCallback(ev);
    }
    else {
        cq::send_group_message(GROUP_ID, bg_at(ev) +
            "�����ʽ����Ŷ! Ҫж������װ���Ļ����Է���: \"bg ж������\"");
    }
}

// ���˺�
// ����ǿ��װ����ص��������, ������ǿ���ص�����
#define UPGRADE_CMD(name, type, cmdLen)                                                                 \
    CMD(upgrade_##name##) {                                                                             \
        LL upgradeTimes = 0;                                /* ǿ������ */                              \
        LL coinsNeeded = 0;                                 /* ��ҪӲ�� */                              \
                                                                                                        \
        if (ev.message.length() < cmdLen) {                 /* �޲��� */                                \
            if (preUpgradeCallback(ev, EqiType::##type##, "1", upgradeTimes, coinsNeeded))              \
                postUpgradeCallback(ev, EqiType::##type##, upgradeTimes, coinsNeeded);                  \
        }                                                                                               \
        else {                                              /* �в��� */                                \
            auto param = ev.message.substr(cmdLen - 1);     /* ȥ�������ַ���, ֻ�������� */             \
            if (preUpgradeCallback(ev, EqiType::##type##, param, upgradeTimes, coinsNeeded))            \
                postUpgradeCallback(ev, EqiType::##type##, upgradeTimes, coinsNeeded);                  \
        }                                                                                               \
    }

UPGRADE_CMD(helmet, armor_helmet, 16);
UPGRADE_CMD(body, armor_body, 16);
UPGRADE_CMD(leg, armor_leg, 16);
UPGRADE_CMD(boot, armor_boot, 16);
UPGRADE_CMD(primary, weapon_primary, 20);
UPGRADE_CMD(secondary, weapon_secondary, 20);
UPGRADE_CMD(earrings, ornament_earrings, 16);
UPGRADE_CMD(rings, ornament_rings, 16);
UPGRADE_CMD(necklace, ornament_necklace, 16);
UPGRADE_CMD(jewelry, ornament_jewelry, 16);

// ȷ��ǿ��
CMD(confirm_upgrade) {
    MATCH("ȷ��", ConfirmUpgrade);
}

// ǿ��װ������
CMD(upgrade_help) {
    cq::send_group_message(ev.target.group_id.value(), "��ָ����Ҫǿ����װ������, ���������Ը���Ҫǿ���Ĵ���������: \"bg ǿ��������\", \"bg ǿ��ս�� 5\"");
}

// �鿴���׳�
CMD(view_trade) {
    if (ev.message == "bg ���׳�" || ev.message == "bg �鿴���׳�" || ev.message == "bg ����") {
        if (preViewTradeCallback(ev))
            postViewTradeCallback(ev);
    }
    else {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�����ʽ����Ŷ! Ҫ�鿴���׳��Ļ����Է���: \"bg ���׳�\"");
    }
}

// �����׳���Ŀ
CMD(buy_trade) {
    if (ev.message.length() < 10) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�����ʽ����Ŷ! ����ָ���ʽΪ: \"bg ���� ����ID\"���������������, ��Ϊ: \"bg ���� ����ID ����\"");
        return;
    }
    auto params = str_split(str_trim(ev.message.substr(9)), ' ');
    if (params.size() > 2) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�����ʽ����Ŷ! ����ָ���ʽΪ: \"bg ���� ����ID\"���������������, ��Ϊ: \"bg ���� ����ID ����\"");
        return;
    }
    LL item = -1;
    if (preBuyTradeCallback(ev, params, item))
        postBuyTradeCallback(ev, item);
}

// ���׳��ϼ�
CMD(sell_trade) {
    if (ev.message.length() < 10) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�����ʽ����Ŷ! �ϼ�ָ���ʽΪ: \"bg �ϼ� ������� �۸�\"��"
            "��Ҫָ��Ϊ������Ľ���, �����������Ӹ��ո��\"˽\"��: \"bg �ϼ� ������� ˽\"");
        return;
    }

    auto params = str_split(str_trim(ev.message.substr(9)), ' ');
    if (params.size() != 2 && params.size() != 3) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�����ʽ����Ŷ! �ϼ�ָ���ʽΪ: \"bg �ϼ� ������� �۸�\"��"
            "��Ҫָ��Ϊ������Ľ���, �����������Ӹ��ո��\"˽\"��: \"bg �ϼ� ������� �۸� ˽\"");
        return;
    }
    LL invId = -1;
    bool hasPassword = false;
    LL price = 0;
    if (preSellTradeCallback(ev, params, invId, hasPassword, price))
        postSellTradeCallback(ev, invId, hasPassword, price);
}

// ���׳��¼�
CMD(recall_trade) {
    if (ev.message.length() < 10) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�����ʽ����Ŷ! �¼�ָ���ʽΪ: \"bg �¼� ����ID\"");
        return;
    }
    auto param = ev.message.substr(9);                         // ȥ�������ַ���, ֻ��������
    LL tradeId = -1;
    if (preRecallTradeCallback(ev, param, tradeId))
        postRecallTradeCallback(ev, tradeId);
}

// ��ս����
CMD(fight) {
    if (ev.message.length() < 10) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�����ʽ����Ŷ! �¼�ָ���ʽΪ: \"bg ��ս ������ID\"");
        return;
    }
    auto param = ev.message.substr(9);                         // ȥ�������ַ���, ֻ��������
    LL dungeonLevel = -1;
    if (preFightCallback(ev, param, dungeonLevel))
        postFightCallback(ev, dungeonLevel);
}

// �ϳ�װ��
CMD(synthesis) {
    if (ev.message.length() < 10) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�ϳ�װ��ָ���ʽΪ: \"bg �ϳ� Ŀ��װ��ID(������) �������1 �������2 ...\"��"
            "����: bg �ϳ� �ռ�ħ�� 1 2 3������\"bg �ϳ� װ��ID(������)\"�ɲ鿴���õĺϳɡ�");
        return;
    }
    auto params = str_split(str_trim(ev.message.substr(9)), ' ');
    LL targetId = -1, coins = 0, level = 0;
    std::set<LL, std::greater<LL>> invList;
    if (preSynthesisCallback(ev, params, invList, targetId, coins, level))
        postSynthesisCallback(ev, invList, targetId, coins, level);
}

// ���˺�
// ����Ϊָ��������ָ��������ֵ�Ĺ���ָ��
#define CMD_ADMIN_INC_FIELD(funcName, commandStr, fieldStr, callbackFuncName)                               \
    CMD(admin_add_##funcName##) {                                                                           \
        if (allAdmins.find(USER_ID) == allAdmins.end())                                                     \
            return;                                                                                         \
        if (ev.message.length() < sizeof(commandStr)) {                                                     \
            cq::send_group_message(GROUP_ID, bg_at(ev) + "�����ʽ: " commandStr " qq/all " fieldStr "��");  \
            return;                                                                                         \
        }                                                                                                   \
        auto param = ev.message.substr(sizeof(commandStr) - 1);         /* ȥ�������ַ���, ֻ�������� */     \
        adminAdd##callbackFuncName##Callback(ev, param);                                                    \
    }

CMD_ADMIN_INC_FIELD(coins, "bg /addcoins", "Ӳ��", Coins);
CMD_ADMIN_INC_FIELD(heroCoin, "bg /addherocoin", "Ӣ�۱�", HeroCoin);
CMD_ADMIN_INC_FIELD(level, "bg /addlevel", "�ȼ�", Level);
CMD_ADMIN_INC_FIELD(blessing, "bg /addblessing", "ף��", Blessing);
CMD_ADMIN_INC_FIELD(energy, "bg /addenergy", "����", Energy);
CMD_ADMIN_INC_FIELD(exp, "bg /addexp", "����", Exp);
CMD_ADMIN_INC_FIELD(invCapacity, "bg /addinvcapacity", "��������", InvCapacity);
CMD_ADMIN_INC_FIELD(vip, "bg /addvip", "VIP�ȼ�", Vip);
