#include "demo_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lib_list.h"
#include "lib_net.h"

void Server_Data_Handle(LIB_NET_CONNECT_T *conn, char *data, unsigned short datalen)
{
	int i = 0;
	if (NULL == conn)
		return;

	printf("[%s:%d] The client(fd:%d, IP:%s, Port:%d) recv data(%d):", __FUNCTION__, __LINE__, conn->fd, conn->addr, conn->port, datalen);
	for (;i < datalen; i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
}

int main (void)
{
	LIST_T *list_server;

	list_server = (LIST_T *)malloc(sizeof(LIST_T));
	memset(list_server, 0x0, sizeof(LIST_T));
	printf("[%s:%d] creat server, port is 9200, max connect num is 20\n", __FUNCTION__, __LINE__);
	Net_Creat_Server(&list_server, 9200, 20, Server_Data_Handle);

	while(1) {
		sleep(2);
	}
	free(list_server);
	return 1;
}
