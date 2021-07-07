/*
����: Bingy ��Ϸ��صĺ���. ������Ϸ����Ҫ�߼�
����: ����
�ļ�: game.cpp
*/

#include "game.hpp"
#include "player.hpp"
#include "utils.hpp"
#include "signin_event.hpp"
#include "trade.hpp"
#include "synthesis.hpp"
#include "monster.hpp"
#include <unordered_set>
#include <sstream>

std::unordered_set<LL>  blacklist;          // ������ (�޸���Ŀǰ�ǵü���)
std::unordered_set<LL>  allAdmins;          // ����Ա (�޸���Ŀǰ�ǵü���)

// ȡ�ð�������ַ���
std::string bg_at(const cq::MessageEvent &ev) {
    try {
        auto gmi = cq::get_group_member_info(GROUP_ID, USER_ID);
        return "[CQ:at,qq=" + std::to_string(gmi.user_id) + "] ";
    }
    catch (...) {
        return "@" + std::to_string(USER_ID) + " ";
    }
}

// ͨ���˺ż��
bool accountCheck(const cq::MessageEvent &ev) {
    // �������Ƿ��Ѿ�ע��
    if (!bg_player_exist(USER_ID)) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "Ҫ��ע��Ŷ! �췢��\"bg ע��\"������Ϸ��!");
        return false;
    }

    // �������Ƿ���С����
    if (blacklist.find(USER_ID) != blacklist.end()) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�㱻������!");
        return false;
    }

    return true;
}

// ע��ǰ���
bool preRegisterCallback(const cq::MessageEvent &ev) {
    if (bg_player_exist(USER_ID)) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "���Ѿ�ע�����!");
        return false;
    }
    return true;
}

// ע��
void postRegisterCallback(const cq::MessageEvent &ev) {
    try {
        if (bg_player_add(USER_ID))
            cq::send_group_message(GROUP_ID, bg_at(ev) + "ע��ɹ�! ����\"bg ǩ��\"��ȡ����, Ȼ����\"bg ��ս1\"��ʼ����֮�ð�! �����Ҫ����, ���Է���\"bg ����\"Ŷ!");
        else
            cq::send_group_message(GROUP_ID, bg_at(ev) + "ע���ڼ䷢������!");
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("ע���ڼ䷢������: ") + e.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "ע���ڼ䷢������!");
    }
}

// �鿴Ӳ��ǰ���
bool preViewCoinsCallback(const cq::MessageEvent &ev) {
    return accountCheck(ev);
}

// �鿴Ӳ��
void postViewCoinsCallback(const cq::MessageEvent &ev) {
    try {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "Ӳ����: " +
            std::to_string(PLAYER.get_coins())
        );
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("�鿴Ӳ�ҷ�������: ") + e.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�鿴Ӳ�ҷ�������!");
    }
}

// ǩ��ǰ���
bool preSignInCallback(const cq::MessageEvent &ev) {
    if (!accountCheck(ev))
        return false;

    // �������ϴ�ǩ�����ڸ�����һ����ܾ�ǩ��
    dateTime signInDate = dateTime(PLAYER.get_lastSignIn());
    dateTime today = dateTime();

    if (signInDate.get_year() == today.get_year() && signInDate.get_month() == today.get_month() && signInDate.get_day() == today.get_day()) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "������Ѿ�ǩ������!");
        return false;
    }
    return true;
}

// ǩ��
void postSignInCallback(const cq::MessageEvent &ev) {
    try {
        dateTime now;

        // �������ǩ��
        if (is_day_sequential(dateTime(PLAYER.get_lastFight()), now)) {
            if (!PLAYER.inc_signInCountCont(1)) {
                cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("ǩ����������: ��������ǩ������ʧ��"));
                return;
            }
        }
        else {
            if (!PLAYER.set_signInCountCont(1)) {
                cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("ǩ����������: ��������ǩ������ʧ��"));
                return;
            }
        }

        // ��������ǩ��ʱ��, �����ĳ���©������Ҳ���ǩ��
        if (!PLAYER.set_lastSignIn(dateTime().get_timestamp())) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("ǩ����������: ����ǩ��ʱ��ʧ��"));
            return;
        }

        // �������ǩ������
        if (!PLAYER.inc_signInCount(1)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("ǩ����������: ���ǩ������ʧ��"));
            return;
        }

        // ǩ�����Ӳ�� = 500 + 25 * ����ǩ������ + 10 * ��ǩ������ + rnd(20 * ��ǩ������)
        // ǩ��*֮���*���� = 15 * ����ǩ������ + 5 * ��ǩ������
        // ǩ����þ��� = 15 * ����ǩ������ + 5 * ��ǩ������
        LL deltaCoins = 500 + 25 * PLAYER.get_signInCountCont() + 10 * PLAYER.get_signInCount() + rndRange(20 * PLAYER.get_signInCount());
        LL deltaEnergy = 150 + 5 * PLAYER.get_level();
        LL deltaExp = 15 * PLAYER.get_signInCountCont() + 5 * PLAYER.get_signInCount();

        // ���ǩ���
        std::string eventMsg = "";              // ���Ϣ
        std::vector<LL> eventItems;             // �������Ʒ
        bg_match_sign_in_event(now, deltaCoins, deltaEnergy, eventItems, eventMsg);
        for (const auto &item : eventItems) {   // Ϊ��������Ʒ
            inventoryData itemData;
            itemData.id = item;
            itemData.level = 0;
            itemData.wear = allEquipments.at(item).wear;
            if (!PLAYER.add_inventory_item(itemData)) {
                cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("ǩ����������: �����Ʒ\"" + allEquipments.at(item).name + "\"ʧ��"));
            }
        }

        if (!PLAYER.inc_coins(deltaCoins)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("ǩ����������: ���Ӳ��ʧ��"));
        }
        if (!PLAYER.set_energy(static_cast<LL>(PLAYER.get_energy() * 0.75) + deltaEnergy)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("ǩ����������: �������ʧ��"));
        }
        if (!PLAYER.inc_exp(deltaExp)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("ǩ����������: ��Ӿ���ʧ��"));
        }

        cq::send_group_message(GROUP_ID, bg_at(ev) +
            "ǩ���ɹ�, ����ǩ��" + std::to_string(PLAYER.get_signInCountCont()) + "��, �ܹ�ǩ��" + std::to_string(PLAYER.get_signInCount()) + "�졣"
            "���Ӳ��" + std::to_string(deltaCoins) + ", Ŀǰӵ��" + std::to_string(PLAYER.get_coins()) +
            "; �������" + std::to_string(deltaEnergy) + ", Ŀǰӵ��" + std::to_string(PLAYER.get_energy()) +
            "; ��þ���: " + std::to_string(deltaExp) + (eventMsg.empty() ? "" : eventMsg)
        );
    }

    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("ǩ����������: ") + e.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "ǩ����������!");
    }
}

// �鿴����ǰ���
bool preViewInventoryCallback(const cq::MessageEvent &ev) {
    return accountCheck(ev);
}

// ͨ�ò鿴��������
std::string getInventoryStr(const LL &id) {
    auto tmp = allPlayers.at(id).get_inventory();
    if (tmp.size() == 0) {
        return "�����տ���Ҳ, ��ȥ��ȡװ����!";
    }
    else {
        std::string msg = "���� (" + std::to_string(tmp.size()) + "/" + std::to_string(allPlayers.at(id).get_invCapacity()) + ")\n";
        LL index = 1;
        for (const auto &item : tmp) {
            if (allEquipments.at(item.id).type != EqiType::single_use)              // ��һ������Ʒ
                msg += std::to_string(index) + "." + allEquipments.at(item.id).name + "+" + std::to_string(item.level) + " ";
            else                                                                    // һ������Ʒ
                msg += std::to_string(index) + ".[" + allEquipments.at(item.id).name + "] ";
            ++index;
        }
        if (!msg.empty())
            msg.pop_back();
        return msg;
    }
}

// �鿴����
void postViewInventoryCallback(const cq::MessageEvent &ev) {
    try {
        cq::send_group_message(GROUP_ID, bg_at(ev) + getInventoryStr(USER_ID));
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("�鿴������������: ") + e.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�鿴������������!");
    }
}

// ����ǰ���
bool prePawnCallback(const cq::MessageEvent &ev, const std::vector<std::string> &args, std::vector<LL> &rtnItems) {
    if (!accountCheck(ev))
        return false;

    try {
        std::unordered_set<LL> sellList;
        for (const auto &item : args) {
            if (item.empty())
                continue;
            auto tmp = str_to_ll(item);                                         // �ַ���ת������
            if (tmp > PLAYER.get_inventory_size() || tmp < 1) {                 // ����Ƿ񳬳�������Χ
                cq::send_group_message(GROUP_ID, bg_at(ev) + "���\"" + str_trim(item) + "\"�����˱�����Χ!");
                return false;
            }
            if (sellList.find(tmp) == sellList.end()) {                         // ����Ƿ����ظ���Ŀ
                rtnItems.push_back(tmp - 1);                                    // �����ӵ������б� (ע���ڲ��б��� 0 Ϊ��ͷ)
                sellList.insert(tmp);                                           // ��¼����Ŀ
            }
            else {
                cq::send_group_message(GROUP_ID, bg_at(ev) + "���\"" + str_trim(item) + "\"�ظ���!");
                return false;
            }
        }
        return true;
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("����ǰ��鷢������, ��������Ķ�����Ч������: ") + e.what());
        return false;
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�����ʽ����! ��������Ķ�����Ч������?");
        return false;
    }
}

// ����
void postPawnCallback(const cq::MessageEvent &ev, std::vector<LL> &items) {
    try {
        std::sort(items.rbegin(), items.rend());            // �Ӵ�С�������. �Ӻ�����ǰɾ�Ų������

        double  price = 0;
        auto    inv = PLAYER.get_inventory();
        LL      prevIndex = static_cast<LL>(inv.size()) - 1;
        auto    it = inv.rbegin();
        for (const auto &index : items) {                   // ��������ܼ�
            std::advance(it, prevIndex - index);
            if (allEquipments.at(it->id).type == EqiType::single_use)   // һ����װ���۸� = ԭʼ�۸�
                price += allEquipments.at(it->id).price;
            else                                                        // װ���۸� = ԭʼ�۸� + 100 * (1.6 ^ װ���ȼ� - 1) / 0.6
                price += allEquipments.at(it->id).price + 100.0 * (pow(1.6, it->level) - 1) / 0.6;
            prevIndex = index;
        }

        if (!PLAYER.remove_at_inventory(items)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "����ʧ��: �Ƴ�������Ʒʱ��������!");
            return;
        }
        if (!PLAYER.inc_coins(static_cast<LL>(price))) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "����ʧ��: ���Ӳ��ʱ��������!");
            return;
        }
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�ɹ�����" + std::to_string(items.size()) + "����Ʒ, ���" + std::to_string(static_cast<LL>(price)) + "Ӳ��");
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "����ʧ��! ����ԭ��: " + e.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "����ʧ��!");
    }
}

// �鿴����ǰ���
bool preViewPropertiesCallback(const cq::MessageEvent &ev) {
    return accountCheck(ev);
}

// ���˺�
// ��ȡĳ�����Ե��ַ���. �����ȡ���� (atk), ���ַ���Ϊ ����ֵ (�������� + ���׹��� + ��Ʒ����)
#define GET_EQI_PROP_STR(prop)                                                                                  \
    std::setprecision(1) <<                                                                                     \
    allPlayers.at(id).get_##prop##() << " (" <<                                                                 \
    std::setprecision(0) <<                                                                                     \
    allPlayers.at(id).get_equipments().at(EqiType::weapon_primary).calc_##prop##() +      /* ���������ܺ�*/      \
    allPlayers.at(id).get_equipments().at(EqiType::weapon_secondary).calc_##prop##()                            \
    << "+" <<                                                                                                   \
    allPlayers.at(id).get_equipments().at(EqiType::armor_helmet).calc_##prop##() +        /* ���������ܺ�*/      \
    allPlayers.at(id).get_equipments().at(EqiType::armor_body).calc_##prop##() +                                \
    allPlayers.at(id).get_equipments().at(EqiType::armor_leg).calc_##prop##() +                                 \
    allPlayers.at(id).get_equipments().at(EqiType::armor_boot).calc_##prop##()                                  \
    << "+" <<                                                                                                   \
    allPlayers.at(id).get_equipments().at(EqiType::ornament_earrings).calc_##prop##() +   /* ��Ʒ�����ܺ�*/      \
    allPlayers.at(id).get_equipments().at(EqiType::ornament_rings).calc_##prop##() +                            \
    allPlayers.at(id).get_equipments().at(EqiType::ornament_necklace).calc_##prop##() +                         \
    allPlayers.at(id).get_equipments().at(EqiType::ornament_jewelry).calc_##prop##()                            \
    << ")"

// ͨ�ò鿴���Ժ���
std::string getPropertiesStr(const LL &id) {
    std::stringstream msg;

    msg << std::fixed << std::setprecision(1)
        << "�˺�: " << id << "\n"
        << "�ȼ�: " << allPlayers.at(id).get_level() << ", ף��: " << allPlayers.at(id).get_blessing() << "\n"
        << "����: " << allPlayers.at(id).get_exp() << "/" << allPlayers.at(id).get_exp_needed()
            << " (" << static_cast<double>(allPlayers.at(id).get_exp()) / static_cast<double>(allPlayers.at(id).get_exp_needed()) * 100.0 << "%)" << "\n"
        << "����: " << GET_EQI_PROP_STR(hp) << "\n"
        << "����: " << GET_EQI_PROP_STR(atk) << "\n"
        << "����: " << GET_EQI_PROP_STR(def) << "\n"
        << "ħ��: " << GET_EQI_PROP_STR(mp) << "\n"
        << "����: " << GET_EQI_PROP_STR(crt) << "\n"
        << "�Ƽ�: " << GET_EQI_PROP_STR(brk) << "\n"
        << "����: " << GET_EQI_PROP_STR(agi) << "\n"
        << "����: " << allPlayers.at(id).get_energy() << " Ӳ��: " << allPlayers.at(id).get_coins() << "\n"
        << "Ӣ�۱�: " << allPlayers.at(id).get_heroCoin();
    return msg.str();
}

// �鿴����
void postViewPropertiesCallback(const cq::MessageEvent &ev) {
    try {
        cq::send_group_message(GROUP_ID, bg_at(ev) + getPropertiesStr(USER_ID));
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�鿴����ʧ��! ����ԭ��: " + e.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�鿴����ʧ��!");
    }
}

// �鿴װ��ǰ���
bool preViewEquipmentsCallback(const cq::MessageEvent &ev) {
    return accountCheck(ev);
}

// ���˺�
// ���ɶ�Ӧ���͵�װ�����ַ���. �� ID = -1, ����ʾ"δװ��"; ������ʾ װ������+�ȼ� (ĥ��/ԭʼĥ��)
#define GET_EQI_STR(eqitype)   \
    (##eqitype##.id == -1 ? "δװ��" : allEquipments.at(##eqitype##.id).name + "+" + std::to_string(##eqitype##.level) +    \
    " (" + std::to_string(##eqitype##.wear) + "/" + std::to_string(allEquipments.at(##eqitype##.id).wear) + ")")

// ͨ�ò鿴װ������
std::string getEquipmentsStr(const LL &id) {
    const auto& eqi = allPlayers.at(id).get_equipments();
    const auto& singleUseEqi = allPlayers.at(id).get_equipItems();
    
    const auto& armor_helmet = eqi.at(EqiType::armor_helmet);
    const auto& armor_body = eqi.at(EqiType::armor_body);
    const auto& armor_leg = eqi.at(EqiType::armor_leg);
    const auto& armor_boot = eqi.at(EqiType::armor_boot);
    const auto& weapon_primary = eqi.at(EqiType::weapon_primary);
    const auto& weapon_secondary = eqi.at(EqiType::weapon_secondary);
    const auto& ornament_earrings = eqi.at(EqiType::ornament_earrings);
    const auto& ornament_rings = eqi.at(EqiType::ornament_rings);
    const auto& ornament_necklace = eqi.at(EqiType::ornament_necklace);
    const auto& ornament_jewelry = eqi.at(EqiType::ornament_jewelry);

    std::string eqiStr =
        "---����---\n"
        "ͷ��: " + GET_EQI_STR(armor_helmet) + ", " +
        "ս��: " + GET_EQI_STR(armor_body) + "\n" +
        "����: " + GET_EQI_STR(armor_leg) + ", " +
        "սѥ: " + GET_EQI_STR(armor_boot) + "\n" +
        "---����---\n"
        "������: " + GET_EQI_STR(weapon_primary) + ", " +
        "������: " + GET_EQI_STR(weapon_secondary) + "\n" +
        "---��Ʒ---\n"
        "����: " + GET_EQI_STR(ornament_earrings) + ", " +
        "��ָ: " + GET_EQI_STR(ornament_rings) + "\n" +
        "����: " + GET_EQI_STR(ornament_necklace) + ", " +
        "��ʯ: " + GET_EQI_STR(ornament_jewelry);

    // ��ʾ��װ����һ������Ʒ
    if (!singleUseEqi.empty()) {
        LL index = 1;
        eqiStr += "\n---һ����---\n";
        for (const auto &item : singleUseEqi) {
            eqiStr += std::to_string(index) + "." + allEquipments[item.id].name + " ";
            ++index;
        }
        if (!eqiStr.empty())
            eqiStr.pop_back();
    }
    return eqiStr;
}

// �鿴װ��
void postViewEquipmentsCallback(const cq::MessageEvent &ev) {
    try {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "\n" + getEquipmentsStr(USER_ID));
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�鿴װ��ʧ��! ����ԭ��: " + e.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�鿴װ��ʧ��!");
    }
}

// װ��ǰ���
bool preEquipCallback(const cq::MessageEvent &ev, const std::string &arg, LL &equipItem) {
    if (!accountCheck(ev))
        return false;

    try {
        equipItem = str_to_ll(arg) - 1;                                         // �ַ���ת������. ע���ڲ��Ĵ洢����� 0 ��ʼ
        if (equipItem >= PLAYER.get_inventory_size() || equipItem < 0) {        // ����Ƿ񳬳�������Χ
            cq::send_group_message(GROUP_ID, bg_at(ev) + "���\"" + str_trim(arg) + "\"�����˱�����Χ!");
            return false;
        }
        return true;
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "װ��ǰ��鷢������: " + e.what());
        return false;
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�����ʽ����! ��������Ķ�����Ч������?");
        return false;
    }
}

// װ��
void postEquipCallback(const cq::MessageEvent &ev, const LL &equipItem) {
    PLAYER.resetCache();                                            // �����������

    // ��ȡ���������ŵĶ�Ӧ��Ʒ
    auto invItems = PLAYER.get_inventory();
    auto it = invItems.begin();
    std::advance(it, equipItem);
    
    // ��װ����ȥ�˵�װ���ӱ����Ƴ�
    if (!PLAYER.remove_at_inventory({ equipItem })) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "װ��ʱ��������: �ѽ�Ҫװ����װ���ӱ������Ƴ�ʱ��������!");
        return;
    }

    // ����װ��������װ��
    auto eqiType = allEquipments.at(it->id).type;
    if (eqiType != EqiType::single_use) {                               // ����һ������Ʒ
        auto prevEquipItem = PLAYER.get_equipments_item(eqiType);       // ��ȡ���֮ǰװ����װ��

        // ����װ��װ����ȥ
        if (!PLAYER.set_equipments_item(eqiType, *it)) { 
            cq::send_group_message(GROUP_ID, bg_at(ev) + "װ��ʱ��������: �޸����װ��ʱ��������!");
            return;
        }

        // ������֮ǰ��װ����Ϊ��, �ͷŻر�����
        if (prevEquipItem.id != -1) {
            if (!PLAYER.add_inventory_item(prevEquipItem)) {
                cq::send_group_message(GROUP_ID, bg_at(ev) + "װ��ʱ��������: ��֮ǰ��װ���Żر���ʱ��������!");
                return;
            }
        }
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�ɹ�װ��" + eqiType_to_str(eqiType) + ": " + allEquipments.at(it->id).name + "+" +
            std::to_string(it->level) + ", ĥ��" + std::to_string(it->wear) + "/" + std::to_string(allEquipments.at(it->id).wear));
    }
    else {                                                              // ��һ������Ʒ
        // ����Ʒ��ӵ�һ�����б�
        if (!PLAYER.add_equipItems_item(*it)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "װ��ʱ��������: �޸����װ��ʱ��������!");
            return;
        }
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�ɹ�װ��һ������Ʒ: " + allEquipments.at(it->id).name);
    }
}

// ��ָ����ҵ�ָ�����͵�װ��ж�²��Żذ���.
// ���ж����װ��, �򷵻ض�Ӧ������; ���򷵻ؿ��ַ���
// ע��, �ú������ܴ���ж��һ������Ʒ
std::string unequipPlayer(const LL &qq, const EqiType &eqiType) {
    auto &p = allPlayers.at(qq);
    inventoryData eqi = p.get_equipments().at(eqiType);

    if (eqi.id == -1)
        return "";
    else {
        p.resetCache();
        if (!p.set_equipments_item(eqiType, { -1, -1, -1 })) {
            throw std::exception("�������װ��ʱʧ��!");
        }
        if (!p.add_inventory_item(eqi)) {
            throw std::exception("Ϊ������װ��������ʱʧ��!");
        }

        return allEquipments.at(eqi.id).name + "+" + std::to_string(eqi.level);
    }
}

// ���˺�
// ����ж��ָ��װ��
// ���ȼ������Ƿ��ж�Ӧ���͵�װ��, Ȼ��ж��
// name: �ص���������, �� Helmet; type: װ������, �� armor_helmet
#define CALLBACK_UNEQUIP(name, type)                                                                \
    bool preUnequip ##name## Callback(const cq::MessageEvent &ev) {                                 \
        if (!accountCheck(ev))                                                                      \
            return false;                                                                           \
        if (PLAYER.get_equipments().at(EqiType::##type##).id == -1) {                               \
            cq::send_group_message(GROUP_ID, bg_at(ev) + "Ŀǰû��װ��" + eqiType_to_str(EqiType::##type##) + "Ŷ!");  \
            return false;                                                                           \
        }                                                                                           \
        return true;                                                                                \
    }                                                                                               \
                                                                                                    \
    void postUnequip##name##Callback(const cq::MessageEvent &ev) {                                  \
        try {                                                                                       \
            PLAYER.resetCache();                                                                    \
            auto rtn = unequipPlayer(USER_ID, EqiType::##type##);                                   \
            cq::send_group_message(GROUP_ID, bg_at(ev) + "��ж��" + rtn);                           \
        }                                                                                           \
        catch (const std::exception &e) {                                                           \
            cq::send_group_message(GROUP_ID, bg_at(ev) + "ж��װ��ʧ��! ����ԭ��: " + e.what());     \
        }                                                                                           \
        catch (...) {                                                                               \
            cq::send_group_message(GROUP_ID, bg_at(ev) + "ж��װ��ʧ��!");                           \
        }                                                                                           \
    }

CALLBACK_UNEQUIP(Helmet,    armor_helmet);
CALLBACK_UNEQUIP(Body,      armor_body);
CALLBACK_UNEQUIP(Leg,       armor_leg);
CALLBACK_UNEQUIP(Boot,      armor_boot);
CALLBACK_UNEQUIP(Primary,   weapon_primary);
CALLBACK_UNEQUIP(Secondary, weapon_secondary);
CALLBACK_UNEQUIP(Earrings,  ornament_earrings);
CALLBACK_UNEQUIP(Rings,     ornament_rings);
CALLBACK_UNEQUIP(Necklace,  ornament_necklace);
CALLBACK_UNEQUIP(Jewelry,   ornament_jewelry);

// ���˺�
// ����ж��װ�����ַ���
#define UNEQUIP(type)                                       \
    rtn = unequipPlayer(USER_ID, EqiType::##type##);        \
    if (!rtn.empty())                                       \
        msg += rtn + ", ";

// ж�»���ǰ���
bool preUnequipArmorCallback(const cq::MessageEvent & ev) {
    return accountCheck(ev);
}

// ж�»���
void postUnequipArmorCallback(const cq::MessageEvent &ev) {
    try {
        PLAYER.resetCache();                    // �����������

        std::string msg = "";
        auto UNEQUIP(armor_body);
        UNEQUIP(armor_boot);
        UNEQUIP(armor_helmet);
        UNEQUIP(armor_leg);

        if (msg.empty()) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "Ŀǰû��װ���κλ���");
        }
        else {
            msg.pop_back();                     // ȥ����β�� ", "
            msg.pop_back();
            cq::send_group_message(GROUP_ID, bg_at(ev) + "��ж��" + msg);
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "ж��װ��ʧ��! ����ԭ��: " + e.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "ж��װ��ʧ��!");
    }
}

// ж������ǰ���
bool preUnequipWeaponCallback(const cq::MessageEvent &ev) {
    return accountCheck(ev);
}

// ж������
void postUnequipWeaponCallback(const cq::MessageEvent &ev) {
    try {
        PLAYER.resetCache();                    // �����������

        std::string msg = "";
        auto UNEQUIP(weapon_primary);
        UNEQUIP(weapon_secondary);

        if (msg.empty()) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "Ŀǰû��װ���κ�����");
        }
        else {
            msg.pop_back();                     // ȥ����β�� ", "
            msg.pop_back();
            cq::send_group_message(GROUP_ID, bg_at(ev) + "��ж��" + msg);
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "ж��װ��ʧ��! ����ԭ��: " + e.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "ж��װ��ʧ��!");
    }
}

// ж����Ʒǰ���
bool preUnequipOrnamentCallback(const cq::MessageEvent &ev) {
    return accountCheck(ev);
}

// ж����Ʒ
void postUnequipOrnamentCallback(const cq::MessageEvent &ev) {
    try {
        PLAYER.resetCache();                    // �����������

        std::string msg = "";
        auto UNEQUIP(ornament_earrings);
        UNEQUIP(ornament_jewelry);
        UNEQUIP(ornament_necklace);
        UNEQUIP(ornament_rings);

        if (msg.empty()) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "Ŀǰû��װ���κ���Ʒ");
        }
        else {
            msg.pop_back();                     // ȥ����β�� ", "
            msg.pop_back();
            cq::send_group_message(GROUP_ID, bg_at(ev) + "��ж��" + msg);
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "ж��װ��ʧ��! ����ԭ��: " + e.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "ж��װ��ʧ��!");
    }
}

// ж������װ��ǰ���
bool preUnequipAllCallback(const cq::MessageEvent &ev) {
    return accountCheck(ev);
}

// ж������װ��
void postUnequipAllCallback(const cq::MessageEvent &ev) {
    try {
        PLAYER.resetCache();                    // �����������

        std::string msg = "";
        auto UNEQUIP(armor_body);
        UNEQUIP(armor_boot);
        UNEQUIP(armor_helmet);
        UNEQUIP(armor_leg);
        UNEQUIP(weapon_primary);
        UNEQUIP(weapon_secondary);
        UNEQUIP(ornament_earrings);
        UNEQUIP(ornament_jewelry);
        UNEQUIP(ornament_necklace);
        UNEQUIP(ornament_rings);

        auto items = PLAYER.get_equipItems();
        if (!PLAYER.clear_equipItems()) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "ж����Ʒʧ��: ���һ����װ��ʱ��������!");
            return;
        }
        for (const auto &item : items) {
            if (!PLAYER.add_inventory_item(item)) {
                cq::send_group_message(GROUP_ID, bg_at(ev) + "ж����Ʒʧ��: ��һ����װ����ӵ�����ʱ��������!");
                return;
            }
            msg += allEquipments.at(item.id).name + ", ";
        }

        if (msg.empty()) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "Ŀǰû��װ���κ�װ��");
        }
        else {
            msg.pop_back();                     // ȥ����β�� ", "
            msg.pop_back();
            cq::send_group_message(GROUP_ID, bg_at(ev) + "��ж��" + msg);
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "ж��װ��ʧ��! ����ԭ��: " + e.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "ж��װ��ʧ��!");
    }
}

// ж��ָ����Ʒǰ���
bool preUnequipSingleCallback(const cq::MessageEvent &ev, const std::string &arg, LL &unequipItem) {
    if (!accountCheck(ev))
        return false;
    
    try {
        auto tmp = str_to_ll(arg);                                              // �ַ���ת������
        if (tmp < 1 || tmp > PLAYER.get_equipItems_size()) {                    // �������Ƿ񳬳�װ����Χ
            cq::send_group_message(GROUP_ID, bg_at(ev) + "û���ҵ����Ϊ\"" + std::to_string(tmp) + "\"����Ʒ!");
            return false;
        }
        unequipItem = tmp - 1;                                                  // ע���ڲ��б��� 0 Ϊ��ͷ
        return true;
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�����ʽ����: " + e.what() + ", ��������Ķ�����Ч������?");
        return false;
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�����ʽ����! ��������Ķ�����Ч������?");
        return false;
    }
}

// ж��ָ����Ʒ
void postUnequipSingleCallback(const cq::MessageEvent &ev, const LL &unequipItem) {
    try {
        PLAYER.resetCache();                    // �����������

        // �Ȱ�Ҫж�µ�װ����¼����, ����֮����ӵ�����
        auto items = PLAYER.get_equipItems();
        auto it = items.begin();
        std::advance(it, unequipItem);

        if (!PLAYER.remove_at_equipItems(unequipItem)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "ж����Ʒʧ��: �Ƴ�װ��ʱ��������!");
            return;
        }
        if (!PLAYER.add_inventory_item(*it)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "ж����Ʒʧ��: ��װ����ӵ�����ʱ��������!");
            return;
        }
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�ɹ�ж��һ������Ʒ: " + allEquipments.at(it->id).name);
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "ж����Ʒʧ��! ����ԭ��: " + e.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "ж����Ʒʧ��!");
    }
}

// ǿ��װ��֮ǰ���
bool preUpgradeCallback(const cq::MessageEvent &ev, const EqiType &eqiType, const std::string &arg, LL &upgradeTimes, LL &coinsNeeded) {
    if (!accountCheck(ev))
        return false;

    // ����Ƿ���װ����Ӧ���͵�װ��
    if (PLAYER.get_equipments().at(eqiType).id == -1) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "Ŀǰû��װ��" + eqiType_to_str(eqiType) + "Ŷ!");
        return false;
    }

    // ���ַ�����ȡ��������
    try {
        upgradeTimes = str_to_ll(arg);
        if (upgradeTimes < 1) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "��Ч����������");
            return false;
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�����ʽ����: " + e.what() + ", ��������Ķ�����Ч������?");
        return false;
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�����ʽ����! ��������Ķ�����Ч������?");
        return false;
    }
    if (upgradeTimes > 20) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�����������20��!");
        return false;
    }

    // ������������Ӳ��
    double currEqiLevel = static_cast<double>(PLAYER.get_equipments().at(eqiType).level);
    if (upgradeTimes == 1) {
        // װ����һ���۸� = 210 * 1.61 ^ ��ǰװ���ȼ�
        coinsNeeded = static_cast<LL>(210.0 * pow(1.61, currEqiLevel));
        if (PLAYER.get_coins() < coinsNeeded) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "����Ӳ��Ŷ! ����Ҫ" + std::to_string(coinsNeeded - PLAYER.get_coins()) + "Ӳ��");
            return false;
        }
    }
    else {
        //   װ���� n ���۸�
        // = sum(210 * 1.61 ^ x, x, ��ǰ�ȼ�, ��ǰ�ȼ� + n - 1)
        // = 21000 * (1.61 ^ (��ǰ�ȼ� + n) - 1.61 ^ ��ǰ�ȼ�) / 61
        // �� -344.262295082 * exp(0.476234178996 * ��ǰ�ȼ�) + 344.262295082 * exp(0.476234178996 * (��ǰ�ȼ� + n))
        coinsNeeded = static_cast<LL>(-344.262295082 * exp(0.476234178996 * currEqiLevel) + 344.262295082 * exp(0.476234178996 * (currEqiLevel + upgradeTimes)));
        LL currCoins = PLAYER.get_coins();
        if (currCoins < coinsNeeded) {
            // �������Ӳ��, ������û������������ټ�, �������з����е� n ���:
            // 21000 * (1.61 ^ (��ǰ�ȼ� + n) - 1.61 ^ ��ǰ�ȼ�) / 61 = ��ǰӲ��
            // ���:
            // n = (��ǰ�ȼ� * ln(7) - 2 * ��ǰ�ȼ� * ln(10) + ��ǰ�ȼ� * ln(23) + ln(3) + ln(7) + 3 * ln(10) - ln(61 * ��ǰӲ�� + 21000 * (161 / 100) ^ ��ǰ�ȼ�)) / (-ln(7) + 2 * ln(10) - ln(23))
            //   �� -2.09980728831 * (-ln(21000.0 * exp(0.476234178996 * ��ǰ�ȼ�) + 61.0 * ��ǰӲ��) + 0.476234178996 * ��ǰ�ȼ� + 9.95227771671)
            // ��ȡ floor
            upgradeTimes = static_cast<LL>(floor(
                -2.09980728831 * (-log(21000.0 * exp(0.476234178996 * currEqiLevel) + 61.0 * currCoins) + 0.476234178996 * currEqiLevel + 9.95227771671)
            ));

            if (upgradeTimes < 1) {
                // ��ҵ�Ǯһ�ζ���������
                cq::send_group_message(GROUP_ID, bg_at(ev) + "����Ӳ��Ŷ! ����Ҫ" + std::to_string(coinsNeeded - currCoins) + "Ӳ��");
            }
            else {
                // ������ҪӲ��
                coinsNeeded = static_cast<LL>(-344.262295082 * exp(0.476234178996 * currEqiLevel) + 344.262295082 * exp(0.476234178996 * (currEqiLevel + upgradeTimes)));

                cq::send_group_message(GROUP_ID, bg_at(ev) + "����Ӳ��Ŷ! ��ǰӲ��ֻ������" + std::to_string(upgradeTimes) + "��, ǿ����֮��ʣ" + std::to_string(currCoins - coinsNeeded) + "Ӳ��");
            }
            return false;
        }
        else {
            // ��Ǯ���ж������, ����һ��ȷ�ϲ���
            cq::send_group_message(GROUP_ID, bg_at(ev) + "�㽫Ҫ��������" + eqiType_to_str(eqiType) + std::to_string(upgradeTimes) + "��, "
                "��Ứ��" + std::to_string(coinsNeeded) + "Ӳ�ҡ�����\"bg ȷ��\"����, ��20���û��ȷ��, �����ȡ����");

            if (PLAYER.confirmInProgress) {
                // �����ҵ�ǰ�н����е�ȷ��, ��ôȡ�������ڽ��е�ȷ��, ���ȴ��Ǹ�ȷ���˳�
                PLAYER.abortUpgrade();
                PLAYER.waitConfirmComplete();
            }
            bool confirm = PLAYER.waitUpgradeConfirm();     // �ȴ���ҽ���ȷ��
            if (!confirm) {
                cq::send_group_message(GROUP_ID, bg_at(ev) + "ȡ������������" + eqiType_to_str(eqiType) + std::to_string(upgradeTimes) + "��");
            }
            return confirm;
        }
    }
    return true;
}

// ǿ��װ��
void postUpgradeCallback(const cq::MessageEvent &ev, const EqiType &eqiType, const LL &upgradeTimes, const LL &coinsNeeded) {
    try {
        // ��Ӳ��
        if (!PLAYER.inc_coins(-coinsNeeded)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "ǿ���ڼ���ִ���: �۳����Ӳ�ҵ�ʱ��������!");
            return;
        }

        // ����
        auto currItem = PLAYER.get_equipments_item(eqiType);
        currItem.level += upgradeTimes;
        if (!PLAYER.set_equipments_item(eqiType, currItem)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "ǿ���ڼ���ִ���: �������װ��ʱ��������!");
            return;
        }

        // ����ȷ����Ϣ
        cq::send_group_message(GROUP_ID, (std::stringstream() <<
            "�ɹ�ǿ��" << eqiType_to_str(eqiType) << upgradeTimes << "��: " << allEquipments.at(currItem.id).name << "+" <<
            currItem.level << ", ����" << coinsNeeded << "Ӳ��, ��ʣ" << PLAYER.get_coins() << "Ӳ��"
        ).str());
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "ǿ���ڼ���ִ���! ����ԭ��: " + e.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "ǿ���ڼ���ִ���!");
    }
}

// ȷ�϶��ǿ��ǰ���
bool preConfirmUpgradeCallback(const cq::MessageEvent &ev) {
    if (!accountCheck(ev))
        return false;

    if (!PLAYER.confirmInProgress) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "Ŀǰû����Ҫȷ�ϵ�����ǿ������");
        return false;
    }
    return true;
}

// ȷ�϶��ǿ��
void postConfirmUpgradeCallback(const cq::MessageEvent &ev) {
    PLAYER.confirmUpgrade();
}

// �鿴���׳�ǰ���
bool preViewTradeCallback(const cq::MessageEvent &ev) {
    return accountCheck(ev);
}

// �鿴���׳�
void postViewTradeCallback(const cq::MessageEvent &ev) {
    try {
        cq::send_group_message(GROUP_ID, bg_trade_get_string());
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�鿴���׳����ִ���! ����ԭ��: " + e.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�鿴���׳����ִ���!");
    }
}

// �����׳���Ŀǰ���
bool preBuyTradeCallback(const cq::MessageEvent &ev, const std::vector<std::string> &args, LL &tradeId) {
    try {
        if (!accountCheck(ev))
            return false;

        tradeId = str_to_ll(args[0]);

        // ���ָ�� ID �Ƿ����
        if (allTradeItems.find(tradeId) == allTradeItems.end()) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "����ID" + std::to_string(tradeId) + "���ڽ��׳���");
            return false;
        }

        // ����Ƿ�������
        if (allTradeItems.at(tradeId).hasPassword) {
            if (args.size() == 1) {
                // �����뵫�����û���ṩ����
                cq::send_group_message(GROUP_ID, bg_at(ev) + "�ý�����Ҫ�ṩ����Ŷ!");
                return false;
            }
            else {
                // ��������Ƿ�ƥ��
                if (args[1] != allTradeItems.at(tradeId).password) {
                    cq::send_group_message(GROUP_ID, bg_at(ev) + "�������벻��ȷ!");
                    return false;
                }
            }
        }
        else {
            if (args.size() == 2) {
                // û�����뵫������ṩ������
                cq::send_group_message(GROUP_ID, bg_at(ev) + "�ý��ײ���Ҫ�ṩ����! ֱ�ӷ���\"bg ���� \"" + std::to_string(tradeId) + "���ɹ���");
                return false;
            }
        }

        // ����Ƿ�Ǯ��
        if (PLAYER.get_coins() < allTradeItems.at(tradeId).price) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "������Ǯ��Ŷ! ����" +
                std::to_string(allTradeItems.at(tradeId).price - PLAYER.get_coins()) + "Ӳ��");
            return false;
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "����ǰ���ʱ���ִ���! ��ȷ���������ֵ��ȷ? ����ԭ��: " + e.what());
        return false;
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "����ǰ���ʱ���ִ���! ��ȷ���������ֵ��ȷ?");
        return false;
    }
    return true;
}

// �����׳���Ŀ
void postBuyTradeCallback(const cq::MessageEvent &ev, const LL &tradeId) {
    try {
        tradeData item = allTradeItems.at(tradeId);

        // ���׳��Ƴ���Ӧ��Ŀ
        if (!bg_trade_remove_item(tradeId)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "�ӽ��׳��Ƴ���Ӧ��Ʒʱ���ִ���!");
            return;
        }

        // �򷽿�Ǯ
        if (!PLAYER.inc_coins(-item.price)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "�۳����Ӳ��ʱ���ִ���!");
            return;
        }

        // ���������Ӧ��Ʒ
        if (!PLAYER.add_inventory_item(item.item)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "����ұ������װ��ʱ���ִ���!");
            return;
        }

        // ������Ǯ
        if (!allPlayers.at(item.sellerId).inc_coins(item.price)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "Ϊ������Ǯ��ʱ����ִ���!");
            return;
        }

        // ����װ�����ͷ��͹���ɹ���Ϣ
        if (allEquipments.at(item.item.id).type == EqiType::single_use) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "�ɹ�����һ������Ʒ: " + allEquipments.at(item.item.id).name +
                ", ����" + std::to_string(item.price) + "Ӳ��");
        }
        else {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "�ɹ�����װ��: " + allEquipments.at(item.item.id).name +
                "+" + std::to_string(item.item.level) + ", ĥ���" + std::to_string(item.item.wear) + "/" +
                std::to_string(allEquipments.at(item.item.id).wear) + ", ����" + std::to_string(item.price) + "Ӳ��");
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "����ʱ���ִ���! ����ԭ��: " + e.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "����ʱ���ִ���!");
    }
}

// �ϼܽ��׳���Ŀǰ���
bool preSellTradeCallback(const cq::MessageEvent &ev, const std::vector<std::string> &args, LL &invId, bool &hasPassword, LL &price) {
    try {
        if (!accountCheck(ev))
            return false;

        invId = str_to_ll(args[0]) - 1;
        price = str_to_ll(args[1]);

        // ���ڶ��������Ƿ���ȷ
        if (args.size() == 3) {
            if (args[2] == "˽") {
                hasPassword = true;
            }
            else {
                cq::send_group_message(GROUP_ID, bg_at(ev) + "�����ʽ����Ŷ! �ϼ�ָ���ʽΪ: \"bg �ϼ� ������� �۸�\"��"
                    "��Ҫָ��Ϊ������Ľ���, �����������Ӹ��ո��\"˽\"��: \"bg �ϼ� ������� ˽\"");
                return false;
            }
        }
        else
            hasPassword = false;

        // ��鱳������Ƿ񳬳���Χ
        if (invId >= PLAYER.get_inventory_size() || invId < 0) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "���\"" + str_trim(args[0]) + "\"�����˱�����Χ!");
            return false;
        }

        // ���۸��Ƿ����
        // Ĭ�ϼ۸� * 0.25 <= �۸� <= Ĭ�ϼ۸� * 10
        auto playerInv = PLAYER.get_inventory();
        auto it = playerInv.begin();
        std::advance(it, invId);                                                                                    // ��ȡ ID ��Ӧ�ı�����Ʒ
        LL defPrice = static_cast<LL>(allEquipments.at(it->id).price + 100.0 * (pow(1.6, it->level) - 1) / 0.6);    // ��ȡ����Ʒ��Ĭ�ϳ��ۼ۸�
        if (defPrice / 4 > price || price > defPrice * 10) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "�ϼܼ۸���߻��߹���: �۸񲻿ɵ��ڳ��ۼ۸��25% (" +
                std::to_string(defPrice / 4) + "), Ҳ���ɸ��ڳ��ۼ۸��ʮ�� (" + std::to_string(defPrice * 10) + ")");
            return false;
        }

        // ����Ƿ�Ǯ��˰
        if (PLAYER.get_coins() < price * 0.05) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "����Ǯ��5%��˰Ŷ! ����Ҫ" +
                std::to_string(static_cast<LL>(price * 0.05 - PLAYER.get_coins())) + "Ӳ��");
            return false;
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�ϼ�ǰ���ʱ���ִ���! �����������ֵ�Ƿ���Ч? ����ԭ��: " + e.what());
        return false;
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�ϼ�ǰ���ʱ���ִ���! �����������ֵ�Ƿ���Ч?");
        return false;
    }
    return true;
}

// �ϼܽ��׳���Ŀ
void postSellTradeCallback(const cq::MessageEvent &ev, const LL &invId, const bool &hasPassword, const LL &price) {
    try {
        // �����������Ľ���, ���������һ������
        std::string password = "";
        if (hasPassword) {
            password = std::to_string(rndRange(100000, 999999));
        }

        // ����ұ����Ƴ�ָ����Ŀ
        auto playerInv = PLAYER.get_inventory();
        auto it = playerInv.begin();
        std::advance(it, invId);                        // ��ȡ ID ��Ӧ�ı�����Ʒ
        if (!PLAYER.remove_at_inventory(invId)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "�ϼ�ʱ���ִ���: ����ұ������Ƴ���Ʒʧ��!");
            return;
        }

        // �۳�˰��
        LL tax = static_cast<LL>(price * 0.05);
        if (tax < 1)
            tax = 1;
        if (!PLAYER.inc_coins(-tax)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "�ϼ�ʱ���ִ���: ����ұ������Ƴ���Ʒʧ��!");
            return;
        }

        // ��ȡ�����½��׳� ID
        tradeData   item;
        item.item = *it;
        item.password = password;
        item.hasPassword = hasPassword;
        item.price = price;
        item.sellerId = USER_ID;
        item.tradeId = bg_get_tradeId();
        item.addTime = dateTime().get_timestamp();
        if (!bg_inc_tradeId()) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "�ϼ�ʱ���ִ���: ���½��׳�IDʧ��!");
            return;
        }

        // �����Ʒ�����׳�
        if (!bg_trade_insert_item(item)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "�ϼ�ʱ���ִ���: ����Ʒ��ӵ����׳�ʱ��������!");
            return;
        }

        // ������Ϣ
        if (hasPassword) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "�ɹ��ϼ�, ����IDΪ" + std::to_string(item.tradeId) +
                ", ��ȡ˰��" + std::to_string(tax) + "Ӳ�ҡ����������Ѿ�ͨ��˽�ķ�������");
            cq::send_private_message(USER_ID, "����IDΪ" + std::to_string(item.tradeId) + "�Ľ��׵Ĺ�������Ϊ: " + password);
        }
        else
            cq::send_group_message(GROUP_ID, bg_at(ev) + "�ɹ��ϼ�, ����IDΪ" + std::to_string(item.tradeId) +
                ", ��ȡ˰��" + std::to_string(tax) + "Ӳ��");
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�ϼ�ʱ���ִ���! ����ԭ��: " + e.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�ϼ�ʱ���ִ���!");
    }
}

// �¼ܽ��׳���Ŀǰ���
bool preRecallTradeCallback(const cq::MessageEvent &ev, const std::string &arg, LL &tradeId) {
    if (!accountCheck(ev))
        return false;

    try {
        tradeId = str_to_ll(arg);

        // ��齻�� ID �Ƿ��ڽ��׳���
        if (allTradeItems.find(tradeId) == allTradeItems.end()) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "����ID" + str_trim(arg) + "���ڽ��׳���!");
            return false;
        }

        // ��������Ƿ������
        if (allTradeItems.at(tradeId).sellerId != USER_ID) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "�����Ŷ! ��������ϼܵ�װ����?");
            return false;
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�¼�ǰ���ʱ���ִ���! �����������ֵ�Ƿ���Ч? ����ԭ��: " + e.what());
        return false;
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�¼�ǰ���ʱ���ִ���! �����������ֵ�Ƿ���Ч?");
        return false;
    }
    return true;
}

// �¼ܽ��׳���Ŀ
void postRecallTradeCallback(const cq::MessageEvent &ev, const LL &tradeId) {
    try {
        // �ӽ��׳��Ƴ���Ӧ��Ʒ
        tradeData item = allTradeItems.at(tradeId);
        if (!bg_trade_remove_item(tradeId)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "�¼�ʱ���ִ���: �ӽ��׳��Ƴ���Ʒʧ��!");
            return;
        }

        // �����Ʒ����ұ���
        if (!PLAYER.add_inventory_item(item.item)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "�¼�ʱ���ִ���: ����Ʒ�Ż���ұ���ʧ��!");
            return;
        }

        const auto& eqi = allEquipments.at(item.item.id);
        if (eqi.type == EqiType::single_use) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "�ɹ��¼ܽ���" + std::to_string(tradeId) + ": �Ѱ�" + eqi.name + "�Żر���, ��˰����˻�Ŷ!");
        }
        else {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "�ɹ��¼ܽ���" + std::to_string(tradeId) + ": �Ѱ�" + eqi.name +
                "+" + std::to_string(item.item.level) + "�Żر���, ��˰����˻�Ŷ!");
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�¼�ʱ���ִ���! ����ԭ��: " + e.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�¼�ʱ���ִ���!");
    }
}

// �ϳ�װ��ǰ���
bool preSynthesisCallback(const cq::MessageEvent &ev, const std::vector<std::string> &args, std::set<LL, std::greater<LL>> &invList, LL &targetId, LL &coins, LL &level) {
    if (!accountCheck(ev))
        return false;

    try {
        targetId = str_to_ll(args[0]);                      // ��ȡĿ��װ�� ID

        // ���Ŀ��װ���Ƿ����
        if (allEquipments.find(targetId) == allEquipments.end()) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "ָ����Ŀ��װ��ID\"" + args[0] + "\"������!");
            return false;
        }

        // ���ָֻ����Ŀ��װ��, ���г����õĺϳ�
        if (args.size() == 1) {
            const auto result = allSynthesises.equal_range(targetId);       // ��ȡ���еĺϳɷ���
            if (std::distance(result.first, result.second) == 0) {          // û�кϳɷ���
                cq::send_group_message(GROUP_ID, bg_at(ev) + "װ��\"" + allEquipments.at(targetId).name + "\"���ɺϳ�!");
                return false;
            }
            std::string msg = "װ��\"" + allEquipments.at(targetId).name + "\"�ĺϳɷ�ʽ:\n";
            for (auto it = result.first; it != result.second; ++it) {       // �������кϳɷ������ִ�
                for (const auto &item : it->second.requirements) {
                    msg += allEquipments.at(item).name + "+";
                }
                msg += "$" + std::to_string(it->second.coins) + "\n";
            }
            msg.pop_back();                                                 // ȥ��ĩβ�� '\n'
            cq::send_group_message(GROUP_ID, msg);
            return false;
        }

        auto inventory = PLAYER.get_inventory();                            // ��ȡ��ұ���
        for (size_t i = 1; i < args.size(); ++i) {                          // ��ȡ���б������
            LL tmp = str_to_ll(args[i]) - 1;
            if (invList.find(tmp) != invList.end()) {                       // ������������ظ�
                cq::send_group_message(GROUP_ID, bg_at(ev) + "ָ���ı������\"" + args[i] + "\"�ظ���!");
                return false;
            }
            if (tmp < 0 || tmp >= inventory.size()) {                       // ��������ų���������Χ
                cq::send_group_message(GROUP_ID, bg_at(ev) + "ָ���ı������\"" + args[i] + "\"�����˱�����Χ!");
                return false;
            }
            invList.insert(tmp);
        }

        std::unordered_multiset<LL> materials;                              // �����ṩ�Ĳ���
        auto invIter = inventory.begin();
        LL prevInvId = 0;
        for (auto invIdIter = invList.begin(); invIdIter != invList.end(); ++invIdIter) {
            std::advance(invIter, *invIdIter - prevInvId);
            materials.insert(invIter->id);
            if (invIter->level > level)                                     // ��ȡ���ϵ���ߵȼ�
                level = invIter->level;
            prevInvId = *invIdIter;
        }

        if (!bg_match_synthesis(targetId, materials, coins)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "û���ҵ�����ϳ�Ŷ! ����Է���\"bg �ϳ� \"" + std::to_string(targetId) + "����ȡ�ϳɷ�ʽ��");
            return false;
        }

        if (PLAYER.get_coins() < coins) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "������Ǯ��������ϳ�Ŷ! ����Ҫ" + std::to_string(coins - PLAYER.get_coins()) + "Ӳ�ҡ�");
            return false;
        }
        return true;
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("�ϳ�ǰ��鷢������, ��������Ķ�����Ч������: ") + e.what());
        return false;
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�����ʽ����! ��������Ķ�����Ч������?");
        return false;
    }
}

// �ϳ�װ��
void postSynthesisCallback(const cq::MessageEvent &ev, const std::set<LL, std::greater<LL>> &invList, const LL &targetId, const LL &coins, const LL &level) {
    try {
        // �۳�Ӳ��
        if (!PLAYER.inc_coins(-coins)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "�ϳ�װ����������: �۳����Ӳ��ʧ��!");
            return;
        }

        // ����ұ����Ƴ�װ��
        std::vector<LL> invId;
        for (const auto &inv : invList)
            invId.push_back(inv);
        if (!PLAYER.remove_at_inventory(invId)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "�ϳ�װ����������: ����ұ����Ƴ�װ��ʧ��!");
            return;
        }

        // ��Ӻϳɺ��װ������ұ���
        inventoryData newItem;
        newItem.id = targetId;
        newItem.level = level;
        newItem.wear = allEquipments.at(targetId).wear;
        if (!PLAYER.add_inventory_item(newItem)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "�ϳ�װ����������: ����ұ������" +
                allEquipments.at(targetId).name + "+" + std::to_string(level) + "ʧ��!");
            return;
        }

        cq::send_group_message(GROUP_ID, bg_at(ev) + "�ɹ��ϳ�" +
            allEquipments.at(targetId).name + "+" + std::to_string(level) + ", ����" + std::to_string(coins) + "Ӳ��");
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("�ϳ�װ����������: ") + e.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "�ϳ�װ����������!");
    }
}

// ��ս����ǰ���
bool preFightCallback(const cq::MessageEvent &ev, const std::string &arg, LL &dungeonLevel) {
    if (!accountCheck(ev))
        return false;

    try {
        dungeonLevel = str_to_ll(arg);

        // ���ȼ��Ƿ���Ч
        if (allDungeons.find(dungeonLevel) == allDungeons.end()) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "��Ч�ĸ�����!");
            return false;
        }

        // �������Ƿ�������
        if (PLAYER.get_energy() < 10) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "��������Ѿ��ľ���!");
            return false;
        }

        // �����ȴʱ��
        const auto timeDiff = dateTime().get_timestamp() - PLAYER.get_lastFight();
        if (timeDiff < PLAYER.get_cd()) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "��ս��ȴ��û����, ��ʣ" + std::to_string(timeDiff / 60) + ":" + std::to_string(timeDiff % 60));
            return false;
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "��սǰ���ʱ���ִ���! ��������ĸ������Ƿ���Ч? ����ԭ��: " + e.what());
        return false;
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "��սǰ���ʱ���ִ���! ��������ĸ������Ƿ���Ч?");
        return false;
    }
    return true;
}

// ��ս����
void postFightCallback(const cq::MessageEvent &ev, const LL &dungeonLevel) {
    cq::send_group_message(GROUP_ID, bg_at(ev) + std::to_string(dungeonLevel));
}

// =====================================================================================================
// ����ָ��
// =====================================================================================================

// ���˺�
// Ϊָ��������ָ�����Ե���ֵ
#define ADMIN_INC_FIELD(funcName, fieldStr, field)                                              \
    void adminAdd##funcName##Callback(const cq::MessageEvent &ev, const std::string &arg) {     \
        try {                                                                                   \
            auto params = str_split(str_trim(arg), ' ');                                        \
            if (params.size() != 2) {                                                           \
                cq::send_group_message(GROUP_ID, bg_at(ev) + "�����ʽ: bg /add" #field " qq/all " fieldStr "��");   \
                return;                                                                         \
            }                                                                                   \
                                                                                                \
            auto val = str_to_ll(params[1]);                                                    \
            if (params[0] != "all") {                                                           \
                /* Ϊ��һ������ */                                                            \
                auto targetId = qq_parse(params[0]);                                            \
                if (!allPlayers.at(targetId).inc_##field##(val)) {                              \
                    cq::send_group_message(GROUP_ID, bg_at(ev) + "����ָ��: ���" fieldStr "���ִ���: �������" fieldStr "��ʱ��������");   \
                    return;                                                                     \
                }                                                                               \
                cq::send_group_message(GROUP_ID, bg_at(ev) + "�ɹ�Ϊ���" + std::to_string(targetId) + "���" + std::to_string(val) + fieldStr);   \
            }                                                                                   \
            else {                                                                              \
                /* Ϊ���������� */                                                            \
                if (!bg_all_player_inc_##field##(val)) {                                        \
                    cq::send_group_message(GROUP_ID, bg_at(ev) + "����ָ��: ���" fieldStr "���ִ���: �����������" fieldStr "��ʱ��������"); \
                    return;                                                                     \
                }                                                                               \
                cq::send_group_message(GROUP_ID, bg_at(ev) + "�ɹ�Ϊ" + std::to_string(allPlayers.size()) + "λ������" + std::to_string(val) + fieldStr); \
            }                                                                                   \
        }                                                                                       \
        catch (const std::exception &e) {                                                      \
            cq::send_group_message(GROUP_ID, bg_at(ev) + "����ָ��: ���" fieldStr "���ִ���! ����ԭ��: " + e.what());   \
        }                                                                                       \
        catch (...) {                                                                           \
            cq::send_group_message(GROUP_ID, bg_at(ev) + "����ָ��: ���" fieldStr "���ִ���!"); \
        }                                                                                       \
    }

ADMIN_INC_FIELD(Coins, "Ӳ��", coins);
ADMIN_INC_FIELD(HeroCoin, "Ӣ�۱�", heroCoin);
ADMIN_INC_FIELD(Level, "�ȼ�", level);
ADMIN_INC_FIELD(Blessing, "ף��", blessing);
ADMIN_INC_FIELD(Energy, "����", energy);
ADMIN_INC_FIELD(Exp, "����", exp);
ADMIN_INC_FIELD(InvCapacity, "��������", invCapacity);
ADMIN_INC_FIELD(Vip, "VIP�ȼ�", vip);
