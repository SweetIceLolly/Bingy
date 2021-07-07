/*
����: ��Ϣ·��, ������Ϣ�ַ�����Ӧ����
����: ����
�ļ�: msg_router.cpp
*/

#include "msg_router.hpp"
#include "trie.hpp"
#include "utils.hpp"

typedef std::function<void(const cq::MessageEvent &ev)> handlerFunc;

void bg_msg_parse_and_dispatch(cq::MessageEvent &ev, const bg_trie<handlerFunc> &router);

bg_trie<handlerFunc> gm_router, pm_router;

// ���Ⱥ����Ϣ·��
void bg_groupmsg_router_add(const std::string &msg, const handlerFunc &handler) {
    gm_router.addMsg(msg, handler);
}

// ���˽����Ϣ·��
void bg_privatemsg_router_add(const std::string &msg, const handlerFunc &handler) {
    pm_router.addMsg(msg, handler);
}

// Ⱥ����Ϣ�ַ�
void bg_groupmsg_dispatch(const cq::MessageEvent &ev) {
    bg_msg_parse_and_dispatch((cq::MessageEvent)ev, gm_router);
}

// ˽����Ϣ�ַ�
void bg_privatemsg_dispatch(const cq::MessageEvent &ev) {
    bg_msg_parse_and_dispatch((cq::MessageEvent)ev, pm_router);
}

// ��������, Ȼ��ͨ��ָ����·�ɷַ���Ϣ
void bg_msg_parse_and_dispatch(cq::MessageEvent &ev, const bg_trie<handlerFunc> &router) {
    // ���������ڻ����������
    if (ev.message.length() < 2)
        return;

    // ȥ����ͷ����Ŀո�ͻ��з�
    size_t pos = 0;
    for (pos; pos < ev.message.length(); ++pos) {
        if (ev.message[pos] != ' ' && ev.message[pos] != '\r' && ev.message[pos] != '\n' && ev.message[pos] != '\0')
            break;
    }
    if (ev.message.length() - pos < 2)
        return;

    // ���������"bg"��ͷ
    if ((ev.message[pos] != 'b' && ev.message[pos] != 'B') || (ev.message[pos + 1] != 'g' && ev.message[pos + 1] != 'G'))
        return;

    // �Ѷ���Ļ��з��� '\0' �滻Ϊ�ո�, �������������
    for (size_t i = pos; i < ev.message.length(); ++i) {
        if (ev.message[i] == '\r' || ev.message[i] == '\n' || ev.message[i] == '\0')
            ev.message[i] = ' ';
    }

    // ��ȡ�����ִ�, �ڵڶ����ո񴦽ض�����
    // �����Ͳ���һ���Ӱ�������Ϣ�ַ��������, ��ʡCPU��Դ
    bool spaceFound = false;
    std::string cmd = "";       // ������ɺ������
    for (pos; pos < ev.message.length(); ++pos) {
        if (ev.message[pos] == ' ' && pos + 1 < ev.message.length() && ev.message[pos + 1] != ' ') {
            if (!spaceFound) {
                cmd += ' ';
                spaceFound = true;
            }
            else
                break;
        }
        else if (ev.message[pos] != ' ') {
            if (ev.message[pos] >= '0' && ev.message[pos] <= '9')       // �����־����Ͻض�. ��Ϊ�����в�����������
                break;
            else if (ev.message[pos] == '[')                            // �������������Ͻض�. ��Ϊ������ǰ��ص� CQ ��
                break;
            else if ((ev.message[pos] >= 'A' && ev.message[pos] <= 'Z') || (ev.message[pos] >= 'a' && ev.message[pos] <= 'z'))
                cmd += ev.message[pos] | 32;    // ת��ΪСд��ĸ
            else
                cmd += ev.message[pos];
        }
    }

    // ���Ҷ�Ӧ���������
    auto handler = router.getMsgHandler(cmd);
    if (handler) {
        // �ж�Ӧ����������ż��������ַ���
        // �Ƴ��ַ��������ж���Ŀո�ͻ��з�
        for (pos; pos < ev.message.length(); ++pos) {
            if (ev.message[pos] != ' ' || (ev.message[pos] == ' ' && pos + 1 < ev.message.length() && ev.message[pos + 1] != ' ')) {
                if ((ev.message[pos] >= 'A' && ev.message[pos] <= 'Z') || (ev.message[pos] >= 'a' && ev.message[pos] <= 'z'))
                    cmd += ev.message[pos] | 32;    // ת��ΪСд��ĸ
                else
                    cmd += ev.message[pos];
            }
        }
        ev.message = cmd;

        // �����������
        handler(ev);
    }
}
