/*
����: Bingy �������� HTTP ������ӿ�
����: ����
�ļ�: http_handlers.hpp
*/

#pragma once

#include "rest_server/rest_server.hpp"

#define CMD(cmd) void bg_cmd_##cmd##(mg_connection *connection, int ev, mg_http_message *ev_data, void *fn_data)

/**
 * �ͻ�����֤
 * ����:
 *  appid: Ӧ�� ID
 *  secret: �ܳ�
 * ����ֵ:
 *  200: �ɹ�
 *  400: ʧ��, ��������ص� msg
 *  500: �ڲ�����, ��������ص� msg
 */
CMD(auth);

/**
 * �����ע��
 * ����:
 *  token: ��֤
 *  groupid: Ⱥ��
 *  qq: QQ ��
 * ����ֵ:
 *  200: �ɹ�
 *  400: ʧ��, ��������ص� msg
 *  500: �ڲ�����, ��������ص� msg
 */
CMD(register);
