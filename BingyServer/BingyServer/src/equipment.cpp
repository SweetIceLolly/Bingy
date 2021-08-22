/*
描述: Bingy 装备类相关操作
作者: 冰棍
文件: equipment.cpp
*/

#include "equipment.hpp"
#include "utils.hpp"
#include "config_parser.hpp"

#define DEFAULT_EQI_PATH    "equipments.txt"

std::string                             eqiConfigPath = DEFAULT_EQI_PATH;
std::unordered_map<LL, equipmentData>   allEquipments;

// 读取所有装备信息
bool bg_load_equipment_config() {
    configParser    parser(eqiConfigPath);
    equipmentData   *temp = nullptr;                        // 装备信息临时变量

    return parser.load(
        // 切换 state 回调函数. 这里用不上. state 一直保持为 0
        [](const std::string &line, char &state) -> bool {
            return true;
        },

        // 获取属性值回调函数
        [&](const std::string &propName, const std::string &propValue, char state, const unsigned int &lineNo) -> bool {
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
                else {
                    console_log(std::string("未知的配置名: \"") + propName +
                        std::string("\", 于行") + std::to_string(lineNo), LogType::warning);
                }
            }
            catch (const std::exception &e) {
                console_log("处理装备配置时发生错误: 于行" + std::to_string(lineNo) + ", 原因: " + e.what(), LogType::warning);
            }
            catch (...) {
                console_log("处理装备配置时发生错误: 于行" + std::to_string(lineNo), LogType::warning);
            }
            return true;
        },

        // 开始标记回调函数
        [&](char state) -> bool {
            temp = new equipmentData();
            return true;
        },

        // 结束标记回调函数
        [&](char state) -> bool {
            bool rtn = allEquipments.insert({ temp->id, *temp }).second;
            delete temp;
            if (!rtn)
                console_log("无法把装备 ID = " + std::to_string(temp->id) + " 添加到装备列表, 可能是因为 ID 重复", LogType::error);
            return rtn;
        }
    );
}
