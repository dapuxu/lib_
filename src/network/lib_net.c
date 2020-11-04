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
#include "lib_debug.h"

#define MAX_FD_NUM 	1024
static int select_fd = 0;
static int select_epoll_fd = 0;

/*******************************************************************************************************************
**	函数名:Net_Data_Send
**	描	述:数据发送
**	参	数:[in]fd:链路描述符
**		   [in]data:数据
**		   [in]datalen:数据长度
**	返回值:无
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
**	函数名:Net_Server_Select_Connect
**	描	述:链路条件查找-描述符
**		   [in]data:数据
**	返回值:1-找到关键词
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
**	函数名:Net_Server_Select_Epoll_Listen
**	描	述:链路条件查找-epoll描述符
**		   [in]data:数据
**	返回值:1-找到关键词
********************************************************************************************************************/
static char Net_Server_Select_Epoll_Listen(void* data)
{
	LIB_NET_CONNECT_T *conn = (LIB_NET_CONNECT_T *)data;

	if (NULL == conn)
		return 0;

	if (select_epoll_fd > 0 && select_epoll_fd == conn->fd)
		return 1;
	return 0;
}

/*******************************************************************************************************************
**	函数名:Net_Set_Nonblock
**	描	述:设置不阻塞
**		   [in]fd:描述符
**	返回值:无
********************************************************************************************************************/
static void Net_Set_Nonblock(int fd)
{
    int flag, ret;
	flag = fcntl(fd, F_GETFL, 0);
    if (flag == -1) {
        DBG_PRINT(1, "get fcntl flag %s\n", strerror(errno));
        return;
    }

   	ret = fcntl(fd, F_SETFL, flag | O_NONBLOCK);
    if (ret == -1) {
        DBG_PRINT(1, "set fcntl non-blocking %s\n", strerror(errno));
        return;
    }
}

/*******************************************************************************************************************
**	函数名:Net_Server_Close_Connect
**	描	述:关闭连接及资源回收
**		   [in]conn:连接信息
**	返回值:无
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
**	函数名:Net_Server_Thread
**	描	述:服务器线程
**	参	数:[in]net:服务器参数
**	返回值:1-成功/0-已有同端口服务器/-1-失败
********************************************************************************************************************/
static void *Net_Recv_Thread(void *net)
{
	int fd = -1;
   	unsigned int n;
	LIB_NET_CONNECT_T conn;
	LIB_NET_SERVER_T *server = (LIB_NET_SERVER_T *)net;
	LIST_T *node = NULL;

   	while (1) {
        pthread_mutex_lock(&server->recv_lock);
        while(server->recv_poll == NULL) {															/* 等待到任务队列不为空 */
            pthread_cond_wait(&server->cond1, &server->recv_lock);
		}

		List_Get_Head(&server->recv_poll, (void *)&conn, sizeof(LIB_NET_CONNECT_T)); 				/* 从任务队列取出一个读任务 */

        pthread_mutex_unlock(&server->recv_lock);

        char recvBuf[1024] = {0};
        int ret = 999;
        int rs = 1;
 
        while(rs) {
       		ret = recv(conn.fd, recvBuf, 1024, 0);													/* 接受客户端消息 */
            if(ret < 0) {																			/* 由于是非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可 */
                if(errno == EAGAIN) {
                    DBG_PRINT(1, "EAGAIN\n");
                    break;
                } else {
					pthread_mutex_lock(&server->conn_lock);
					select_fd = conn.fd;
                   	node = List_Select_Node(server->connect , Net_Server_Select_Connect);
					server->connect_num--;
					pthread_mutex_unlock(&server->conn_lock);
					Net_Server_Close_Connect(&conn);
					List_Del_Node(node);
					DBG_PRINT(1, "Connect to cilent is close!\n");
                    break;
                }
            } else if(ret == 0) {																	/* 这里表示对端的socket已正常关闭. */
                rs = 0;
            }
            if(ret == sizeof(recvBuf)) {
                rs = 1; // 需要再次读取
 
            } else {
                rs = 0;
			}
			server->Net_Server_Data_Handle(&conn, recvBuf, ret);
        }
   	}
	return NULL;
}

/*******************************************************************************************************************
**	函数名:Net_Creat_Recv_Thread
**	描	述:创建数据接收线程
**	参	数:[in/out]net:服务器信息
**	返回值:1-成功/0-已有同端口服务器/-1-失败
********************************************************************************************************************/
static pthread_t Net_Creat_Recv_Thread(LIB_NET_SERVER_T *net)
{
	pthread_t ThreadRecv;

	if (NULL == net) {
		return -1;
	}
	if (pthread_create(&ThreadRecv, NULL, Net_Recv_Thread, (void *)(net)) != 0) {
        DBG_PRINT(1, "pthread_create Net_Recv_Thread failed.\n");
		return 0;
    }
	return ThreadRecv;
}

/*******************************************************************************************************************
**	函数名:Net_Creat_Recv_Thread
**	描	述:创建数据接收线程
**	参	数:[in/out]net:服务器信息
**	返回值:1-成功/0-已有同端口服务器/-1-失败
********************************************************************************************************************/
static char Net_Close_Recv_Thread(pthread_t hreadrecv)
{
	if (0 >= hreadrecv)
		return 0;

	pthread_cancel(hreadrecv);
	return 1;
}

/*******************************************************************************************************************
**	函数名:Net_Server_Thread
**	描	述:服务器线程
**	参	数:[in]net:服务器参数
**	返回值:1-成功/0-已有同端口服务器/-1-失败
********************************************************************************************************************/
static void *Net_Server_Thread(void *net)
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
	LIB_NET_SERVER_T *server = (LIB_NET_SERVER_T *)net;

    if (epfd == -1) {
        DBG_PRINT(1, "epoll create %s\n", strerror(errno));
        return NULL;
    }

	if (server == NULL)
		return NULL;

	DBG_PRINT(1, "creat server of port:%d, and max connect number is %d\n", server->port, server->connect_max);
	server->fd =socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(server->fd<0) {
        perror("cannot create communication socket");
        return NULL;
    }

	flags = fcntl(server->fd, F_GETFL, 0);

	fcntl(server->fd, F_SETFL, flags | O_NONBLOCK);														/* 不阻塞 */

	s_addr.sin_family = AF_INET;
    s_addr.sin_addr.s_addr = INADDR_ANY;
    s_addr.sin_port = htons(server->port);
    s_len=sizeof(s_addr);

	bind(server->fd,(struct sockaddr *)&s_addr, s_len);
    listen(server->fd, 10);
	server->flag_valid = 1;

	memset(&event, 0, sizeof(event));
	event.data.fd = server->fd;
	event.events = EPOLLIN | EPOLLERR | EPOLLHUP;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, server->fd, &event) == -1) {
		DBG_PRINT(1, "epoll ctl %s\n", strerror(errno));
		return NULL;
	}
	server->connect = NULL;
	server->recv_poll = NULL;
	server->recv_thread = NULL;
	server->connect_num = 0;
	server->poll_num = 0;

	pthread_mutex_init(&server->conn_lock, NULL);
	pthread_mutex_init(&server->recv_lock, NULL);
	pthread_cond_init(&server->cond1, NULL);
	Net_Creat_Recv_Thread(server);
	Net_Creat_Recv_Thread(server);
	while(server->flag_valid) {
		int num = epoll_wait(epfd, events, MAX_FD_NUM, -1);
        if (num == -1) {
            DBG_PRINT(1, "epoll wait %s\n", strerror(errno));
            break;
        } else {
            int i = 0;
            for (; i<num; ++i) {
                if (events[i].data.fd == server->fd) {
					c_len = sizeof(c_addr);
                    c_fd = accept(server->fd, (struct sockaddr *)&c_addr, &c_len);
                    if (-1 == c_fd) {
                        DBG_PRINT(1, "socket accept %s\n", strerror(errno));
                        return NULL;
                    }
					if (server->connect_num >= server->connect_max) {
						shutdown(c_fd, SHUT_RDWR);
						continue;
					}
                    Net_Set_Nonblock(c_fd);
					event.data.fd = c_fd;
                    event.events = EPOLLIN | EPOLLERR | EPOLLHUP;
                    if (epoll_ctl(epfd, EPOLL_CTL_ADD, c_fd, &event) == -1) {
                        printf("epoll ctl %s\n", strerror(errno));
                        return 0;
                    }

					conn.fd = c_fd;
					memcpy(conn.addr, inet_ntoa(c_addr.sin_addr), strlen(inet_ntoa(c_addr.sin_addr)));
					conn.port = c_addr.sin_port;
					conn.flag_valid = 1;
					DBG_PRINT(1, "New connect,fd:%d, addr:%s, port:%d\n", c_fd, inet_ntoa(c_addr.sin_addr), c_addr.sin_port);
					pthread_mutex_lock(&server->conn_lock);
					List_Add(&server->connect, (void *)(&conn), sizeof(LIB_NET_CONNECT_T), DATA_TYPE_MASK_NET_CONNECT,0);
					server->connect_num++;
					pthread_mutex_unlock(&server->conn_lock);
                    continue;
                } else if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP) {
                    DBG_PRINT(1, "epoll err\n");
                    close(events[i].data.fd);
                    continue;
                } else {
					pthread_mutex_lock(&server->conn_lock);
					select_fd = events[i].data.fd;
					conn_data = (LIB_NET_CONNECT_T *)List_Select_Node_Data(server->connect, Net_Server_Select_Connect, DATA_TYPE_MASK_NET_CONNECT);
					pthread_mutex_unlock(&server->conn_lock);
					if (conn_data == NULL) {
						DBG_PRINT(1, "fd:%d, conn_data is null\n", events[i].data.fd);
						continue;
					}
					pthread_mutex_lock(&server->recv_lock);
					List_Add(&server->recv_poll , (void *)conn_data, sizeof(LIB_NET_CONNECT_T), DATA_TYPE_MASK_NET_CONNECT, 0);
					pthread_cond_broadcast(&server->cond1);
					pthread_mutex_unlock(&server->recv_lock);
					while(1) {
						sleep(1);
					}
                }
            }
        }
    }
	return NULL;
}

/*******************************************************************************************************************
**	函数名:Net_Creat_Server
**	描	述:创建服务器线程
**	参	数:[in/out]net:服务器信息
**	返回值:1-成功/0-已有同端口服务器/-1-失败
********************************************************************************************************************/
signed char Net_Creat_Server(LIB_NET_SERVER_T *net)
{
	pthread_t ThreadServer;

	if (NULL == net) {
		return -1;
	}
	if (pthread_create(&ThreadServer, NULL, Net_Server_Thread, (void *)(net)) != 0) {
        DBG_PRINT(1, "pthread_create Net_Server_Thread failed.\n");
		return 0;
    }
	return 1;
}

/*******************************************************************************************************************
**	函数名:Net_Delect_Server
**	描	述:关闭服务器，并关闭相关链接
**	参	数:[in/out]server_list:服务器链表
**		   [in]port:监听端口
**		   [in]connect_max:最大连接数量
**	返回值:1-成功/0-已有同端口服务器/-1-失败
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

	if (NULL != server->recv_thread) {
		conn_node = server->recv_thread->prev;
		do {
			if (NULL != conn_node->data) {
				conn = (LIB_NET_CONNECT_T *)conn_node->data;
				if (conn->fd > 0) {
					shutdown(conn->fd, SHUT_RDWR);
				}
				List_Del_Node(conn_node);	
			}
			conn_node = server->recv_thread->prev;
		} while (NULL != conn_node);
	}

	server->port = 0;
	server->connect_max = 0;
	server->connect_num = 0;
	if (server->fd > 0)
		close(server->fd);

	server->flag_valid = 0;
}

