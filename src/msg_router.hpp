/*
����: ��Ϣ·��ͷ�ļ�, �ṩ��Ϣ�ַ��ӿ�
����: ����
�ļ�: msg_router.hpp
*/

#pragma once

#include "msg_handlers.hpp"

void bg_groupmsg_dispatch(const cq::MessageEvent &ev);
void bg_privatemsg_dispatch(const cq::MessageEvent &ev);

// msg ������ʽΪ: bg ����
void bg_groupmsg_router_add(const std::string &msg, const std::function<void(const cq::MessageEvent &ev)> &handler);
void bg_privatemsg_router_add(const std::string &msg, const std::function<void(const cq::MessageEvent &ev)> &handler);
