#ifndef __LIB_NET_H__
#define __LIB_NET_H__

#include <pthread.h>
#include "lib_list.h"

typedef struct {
	int fd;
	char addr[32];
	unsigned short port;
	unsigned char flag_valid;
}LIB_NET_CONNECT_T;

typedef struct {
	int fd;
	unsigned short port;
	LIST_T *connect;
	unsigned char flag_valid;
	unsigned short connect_num;
	unsigned short connect_max;
	pthread_t thread_server;
	void (*Net_Server_Data_Handle)(LIB_NET_CONNECT_T *conn, char *data, unsigned short datalen);
}LIB_NET_SERVER_T;

/*******************************************************************************************************************
**	������:Net_Data_Send
**	��	��:���ݷ���
**	��	��:[in]fd:��·������
**		   [in]data:����
**		   [in]datalen:���ݳ���
**	����ֵ:��
********************************************************************************************************************/
unsigned short Net_Data_Send(int fd, char *data, unsigned short datalen);

/*******************************************************************************************************************
**	������:Net_Creat_Server
**	��	��:��ѯ�ַ����йؼ��ʸ���
**	��	��:[in/out]server_list:����������
**		   [in]port:�����˿�
**		   [in]connect_max:�����������
**	����ֵ:1-�ɹ�/0-����ͬ�˿ڷ�����/-1-ʧ��
********************************************************************************************************************/
signed char Net_Creat_Server(LIST_T **server_list, unsigned short port, unsigned short connect_max, void (*data_handle)(LIB_NET_CONNECT_T *conn, char *data, unsigned short datalen));

/*******************************************************************************************************************
**	������:Net_Delect_Server
**	��	��:�رշ����������ر��������
**	��	��:[in/out]server_list:����������
**		   [in]port:�����˿�
**		   [in]connect_max:�����������
**	����ֵ:1-�ɹ�/0-����ͬ�˿ڷ�����/-1-ʧ��
********************************************************************************************************************/
signed char Net_Delect_Server(LIST_T **server_list, unsigned short port);

#endif
