#include "lib_net.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <linux/poll.h>
#include <sys/ipc.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include "includes.h"

#define MAX_FD_NUM 	10
static int select_fd = 0;

/*******************************************************************************************************************
**	������:Net_Data_Send
**	��	��:���ݷ���
**	��	��:[in]fd:��·������
**		   [in]data:����
**		   [in]datalen:���ݳ���
**	����ֵ:��
********************************************************************************************************************/
unsigned short Net_Data_Send(int fd, char *data, unsigned short datalen)
{
	unsigned short total = 0;
	int bytes_send = 0;

	if (0 >= fd || NULL == data || 0 == datalen)
		return 0;

	while (total < datalen) {
		bytes_send = send(fd, data + total, datalen - total, 0);
		if (bytes_send <= 0) {
			break;
		}
		total += bytes_send;
	}
	return total;
}

/*******************************************************************************************************************
**	������:Net_Server_Recv_Thread
**	��	��:���������ݽ����߳�
**	��	��:[in]str:�ַ���
**		   [in]key:�ؼ���
**	����ֵ:��
********************************************************************************************************************/
static char Net_Server_Select_Connect(void* data)
{
	LIB_NET_CONNECT_T *conn = (LIB_NET_CONNECT_T *)data;

	if (NULL == conn)
		return 0;

	if (select_fd > 0 && select_fd == conn->fd)
		return 1;
	return 0;
}

/*******************************************************************************************************************
**	������:Net_SetNonblock
**	��	��:���÷�����
**	��	��:[in]fd:��·������
**	����ֵ:��
********************************************************************************************************************/
static void Net_Set_Nonblock(int fd)
{
    int flag, ret;
	flag = fcntl(fd, F_GETFL, 0);
    if (flag == -1) {
        printf("get fcntl flag %s\n", strerror(errno));
        return;
    }

   	ret = fcntl(fd, F_SETFL, flag | O_NONBLOCK);
    if (ret == -1) {
        printf("set fcntl non-blocking %s\n", strerror(errno));
        return;
    }
}

/*******************************************************************************************************************
**	������:Net_Server_Close_Connect
**	��	��:�رշ�����������·
**	��	��:[in]fd:��·������
**	����ֵ:��
********************************************************************************************************************/
static void Net_Server_Close_Connect(LIB_NET_CONNECT_T *conn)
{
    if (NULL == conn)
		return;

	shutdown(conn->fd, SHUT_RDWR);
	conn->flag_valid = 0;
	conn->fd = -1;
	memset(conn->addr, 0x0, sizeof(conn->addr));
}

/*******************************************************************************************************************
**	������:Net_Creat_Server
**	��	��:��ѯ�ַ����йؼ��ʸ���
**	��	��:[in/out]server_list:����������
**		   [in]port:�����˿�
**		   [in]connect_max:�����������
**	����ֵ:1-�ɹ�/0-����ͬ�˿ڷ�����/-1-ʧ��
********************************************************************************************************************/
signed char Net_Creat_Server(LIB_NET_SERVER_T *server)

{
	struct sockaddr_in s_addr;
	struct sockaddr_in c_addr;
	struct epoll_event event, events[MAX_FD_NUM];
	int flags, s_len, ret = 0, c_fd = 0;
	LIST_T *node = NULL;
	LIB_NET_CONNECT_T *conn_data = NULL;
	LIB_NET_CONNECT_T conn;
	char buf[1024] = {0x0};
	int epfd = epoll_create(MAX_FD_NUM);
	unsigned int c_len = 0;

    if (epfd == -1) {
        printf("epoll create %s\n", strerror(errno));
        return -1;
    }

	if (server == NULL)
		return -1;

	printf("[%s:%d] creat server of port:%d, and max connect number is %d\n", __FUNCTION__, __LINE__, server->port, server->connect_max);
	server->fd =socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(server->fd<0) {
        perror("cannot create communication socket");
        return 0;
    }

	flags = fcntl(server->fd, F_GETFL, 0);

	fcntl(server->fd, F_SETFL, flags | O_NONBLOCK);														/* ���ó�����ģʽ */

	s_addr.sin_family = AF_INET;
    s_addr.sin_addr.s_addr = INADDR_ANY;
    s_addr.sin_port = htons(server->port);
    s_len=sizeof(s_addr);

	bind(server->fd,(struct sockaddr *)&s_addr, s_len);
    listen(server->fd, 10);
	server->flag_valid = 1;

	memset(&event, 0, sizeof(event));
	event.data.fd = server->fd;																			/* ������������·����epoll */
	event.events = EPOLLIN | EPOLLERR | EPOLLHUP;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, server->fd, &event) == -1) {
		printf("epoll ctl %s\n", strerror(errno));
		return 0;
	}

	while(server->flag_valid) {
		int num = epoll_wait(epfd, events, MAX_FD_NUM, -1);
        if (num == -1) {
            printf("epoll wait %s\n", strerror(errno));
            break;
        } else {
            int i = 0;
            for (; i<num; ++i) {
                if (events[i].data.fd == server->fd) {											/* �ͻ��˽����¼� */
					c_len = sizeof(c_addr);
                    c_fd = accept(server->fd, (struct sockaddr *)&c_addr, &c_len);
                    if (-1 == c_fd) {
                        printf("socket accept %s\n", strerror(errno));
                        return 0;
                    }
					if (server->connect_num >= server->connect_max) {							/* ���ضϿ����� */
						shutdown(c_fd, SHUT_RDWR);
						continue;
					}
                    Net_Set_Nonblock(c_fd);
                    event.data.fd = c_fd;														/* �ͻ�����·����epoll */
                    event.events = EPOLLIN | EPOLLERR | EPOLLHUP;
                    if (epoll_ctl(epfd, EPOLL_CTL_ADD, c_fd, &event) == -1) {
                        printf("epoll ctl %s\n", strerror(errno));
                        return 0;
                    }

					conn.fd = c_fd;
					memcpy(conn.addr, inet_ntoa(c_addr.sin_addr), strlen(inet_ntoa(c_addr.sin_addr)));
					conn.port = c_addr.sin_port;
					conn.flag_valid = 1;
					List_Add(&server->connect, (void *)(&conn), sizeof(LIB_NET_CONNECT_T), DATA_TYPE_MASK_NET_CONNECT);
					server->connect_num++;
                    continue;
                } else if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP) {
                    printf("epoll err\n");
                    close(events[i].data.fd);
                    continue;
                } else {
                    memset(buf, 0, sizeof(buf));
                    ret = recv(events[i].data.fd, buf, sizeof(buf), 0);
					select_fd = events[i].data.fd;
					conn_data = (LIB_NET_CONNECT_T *)List_Select_Node_Data(server->connect, Net_Server_Select_Connect, DATA_TYPE_MASK_NET_CONNECT);
					if (conn_data == NULL)
						continue;
					if (ret > 0) {
						server->Net_Server_Data_Handle(conn_data, buf, ret);
					} else {
						List_Select_Node(node, Net_Server_Select_Connect);
						Net_Server_Close_Connect(conn_data);
						List_Del_Node(node);
						printf("Connect to cilent is close!\n");
					}
                    continue;
                }
            }
        }
    }
	return 1;
}

/*******************************************************************************************************************
**	������:Net_Delect_Server
**	��	��:�رշ����������ر��������
**	��	��:[in/out]server_list:����������
**		   [in]port:�����˿�
**		   [in]connect_max:�����������
**	����ֵ:1-�ɹ�/0-����ͬ�˿ڷ�����/-1-ʧ��
********************************************************************************************************************/
void Net_Delect_Server(LIB_NET_SERVER_T *server)
{
	LIST_T *conn_node = NULL;
	LIB_NET_CONNECT_T *conn = NULL;

	if (NULL == server)
		return;

	if (NULL != server->connect) {
		conn_node = server->connect->prev;
		do {
			if (NULL != conn_node->data) {
				conn = (LIB_NET_CONNECT_T *)conn_node->data;
				if (conn->fd > 0) {
					shutdown(conn->fd, SHUT_RDWR);
				}
				List_Del_Node(conn_node);	
			}
			conn_node = server->connect->prev;
		} while (NULL != conn_node);
	}
	server->port = 0;
	server->connect_max = 0;
	server->connect_num = 0;
	if (server->fd > 0)
		close(server->fd);

	server->flag_valid = 0;
}

