#include "demo_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "lib_list.h"
#include "lib_net.h"
#include "lib_debug.h"

void Server_Data_Handle(LIB_NET_CONNECT_T *conn, char *data, unsigned short datalen)
{
	int i = 0;

	if (NULL == conn)
		return;

	DBG_PRINT(1, "The client(fd:%d, IP:%s, Port:%d) recv data(%d):", conn->fd, conn->addr, conn->port, datalen);
	for (;i < datalen; i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	Net_Data_Send(conn->fd, data, datalen);
}

/*******************************************************************************************************************
**	������:Net_Server_Thread
**	��	��:�����������߳�
**	��	��:[in]param:�������ڵ����
**	����ֵ:��
********************************************************************************************************************/
static void *Demo_Net_Server_Thread(void* param)
{
	LIB_NET_SERVER_T server;

	server.port = 9200;
	server.connect_max = 20;
	server.Net_Server_Data_Handle = Server_Data_Handle;

	Net_Creat_Server(&server);
	DBG_PRINT(1, "Exit server\n");
	return NULL;
}

int main (void)
{
	DEBUG_INFO debug;
	pthread_t DemoThreadServer;

	debug.InterfaceType = 1 << INTERFACE_TYPE_CONSOLE;
	Debug_Init(&debug);
	if (pthread_create(&DemoThreadServer, NULL, Demo_Net_Server_Thread, NULL) != 0) {
        DBG_PRINT(1, "pthread_create Demo_Net_Server_Thread failed.\n");
    }
	
	while(1) {
		sleep(2);
	}
	
	return 1;
}
