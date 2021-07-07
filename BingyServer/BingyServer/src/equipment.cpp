/*
����: Bingy װ������ز���
����: ����
�ļ�: equipment.cpp
*/

#include "equipment.hpp"
#include "utils.hpp"
#include "config_parser.hpp"

#define DEFAULT_EQI_PATH    "equipments.txt"

std::string                             eqiConfigPath = DEFAULT_EQI_PATH;
std::unordered_map<LL, equipmentData>   allEquipments;      // ע��: ��ȡ��ʱ����Բ��ü���, ���ǲ�Ҫʹ��[], ��Ҫʹ�� at(). ���߳�д���ʱ��������

// ��ȡ����װ����Ϣ
bool bg_load_equipment_config() {
    configParser    parser(eqiConfigPath);
    equipmentData   *temp = nullptr;                        // װ����Ϣ��ʱ����

    return parser.load(
        // �л� state �ص�����. �����ò���. state һֱ����Ϊ 0
        [](const std::string &line, char &state) -> bool {
            return true;
        },

        // ��ȡ����ֵ�ص�����
        [&](const std::string &propName, const std::string &propValue, const char &state, const unsigned int &lineNo) -> bool {
            try {
                if (propName == "id")
                    temp->id = std::stoll(propValue);
                else if (propName == "type")
                    temp->type = static_cast<EqiType>(static_cast<unsigned char>(std::stoi(propValue)));
                else if (propName == "name")
                    temp->name = propValue;
                else if (propName == "atk")
                    temp->atk = std::stoll(propValue);
                else if (propName == "def")
                    temp->def = std::stoll(propValue);
                else if (propName == "brk")
                    temp->brk = std::stoll(propValue);
                else if (propName == "agi")
                    temp->agi = std::stoll(propValue);
                else if (propName == "hp")
                    temp->hp = std::stoll(propValue);
                else if (propName == "mp")
                    temp->mp = std::stoll(propValue);
                else if (propName == "crt")
                    temp->crt = std::stoll(propValue);
                else if (propName == "wear")
                    temp->wear = std::stoll(propValue);
                else if (propName == "price")
                    temp->price = std::stoll(propValue);
            }
            catch (const std::exception &e) {
                console_log("����װ������ʱ��������: ����" + std::to_string(lineNo) + ", ԭ��: " + e.what(), LogType::warning);
            }
            catch (...) {
                console_log("����װ������ʱ��������: ����" + std::to_string(lineNo), LogType::warning);
            }
            return true;
        },

        // ��ʼ��ǻص�����
        [&](const char &state) -> bool {
            temp = new equipmentData();
            return true;
        },

        // ������ǻص�����
        [&](const char &state) -> bool {
            bool rtn = allEquipments.insert({ temp->id, *temp }).second;
            delete temp;
            if (!rtn)
                console_log("�޷���װ�� ID = " + std::to_string(temp->id) + " ��ӵ�װ���б�, ��������Ϊ ID �ظ�", LogType::error);
            return rtn;
        }
    );
}
