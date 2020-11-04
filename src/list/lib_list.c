#include "lib_list.h"
#include "includes.h"

/*******************************************************************************************************************
**	函数名:List_Add
**	描	述:链表增加节点，无链表时，新创链表
**	参	数:[in/out]head:链表头节点
**		   [in]data:待加入链表的数据
**		   [in]datalen:数据长度
**		   [in]mask:数据类型掩码
**	返回值:链表头节点
********************************************************************************************************************/
LIST_T *List_Add(LIST_T **head, void *data, int datalen, char mask, char flag_mount)
{
	LIST_T *node = NULL;
	LIST_T *tail_node = NULL;

	if (data == NULL || datalen <= 0)
		return *head;

	node = (LIST_T *)malloc(sizeof(LIST_T));
	if (node == NULL)
		return *head;

	node->flag_mount = 0;
	if (1 == flag_mount) {
		node->data = data;
		node->flag_mount = 1;
	} else {
		node->data = (void *)malloc(datalen);
		if (node->data == NULL) {
			free(node);
			return *head;
		}
		memcpy(node->data, data, datalen);
	}
	node->datalen = datalen;
	node->data_mask = mask;
	if (head == NULL) {
		node->next = node;
		node->prev = node;
		return node;
	} else {
		if (*head != NULL) {
			tail_node = (*head)->prev;
			(*head)->prev = node;
			node->next = (*head);
			tail_node->next = node;
			node->prev = tail_node;
		} else {
			node->next = node;
			node->prev = node;
			*head = node;
		}
		return *head;
	}
}

/*******************************************************************************************************************
**	函数名:List_Get_Head
**	描	述:获取链表头节点
**	参	数:[in/out]head:链表头节点
**		   [in]data:待取节点数据缓存区
**		   [in]buflen:缓存长度
**	返回值:链表头节点
********************************************************************************************************************/
char List_Get_Head(LIST_T **head, void *buf, int buflen)
{
	LIST_T *node = NULL;
	LIST_T *tail_node = NULL;
	int len = 0;

	if (head == NULL || buf == NULL || buflen <= 0)
		return 0;

	node = *head;
	if ((*head)->next == *head) {
		*head = NULL;
	} else {
		tail_node = (*head)->prev;
		*head = node->next;
		tail_node->next = *head;
		(*head)->prev = tail_node;
	}

	if (node->data == NULL) {
		free(node);
		return 0;
	}
	len = (node->datalen <= buflen)?node->datalen:buflen;
	memcpy(buf, node->data, len);
	node->next = NULL;
	node->prev = NULL;
	node->datalen = 0;
	if (0 == node->flag_mount) {
		free(node->data);
	}
	free(node);
	return len;
}

/*******************************************************************************************************************
**	函数名:List_Get_Tail
**	描	述:获取链表头节点
**	参	数:[in/out]head:链表尾节点
**		   [in]data:待取节点数据缓存区
**		   [in]buflen:缓存长度
**	返回值:链表头节点
********************************************************************************************************************/
char List_Get_Tail(LIST_T **head, void *buf, int buflen)
{
	LIST_T *node = NULL;
	LIST_T *tail_node = NULL;
	int len = 0;

	if (head == NULL || buf == NULL || buflen <= 0)
		return 0;

	node = (*head)->prev;
	if ((*head)->next == *head) {
		*head = NULL;
	} else {
		tail_node = node->prev;
		tail_node->next = *head;
		(*head)->prev = tail_node;
	}

	if (node->data == NULL) {
		free(node);
		return 0;
	}
	len = (node->datalen <= buflen)?node->datalen:buflen;
	memcpy(buf, node->data, len);
	node->next = NULL;
	node->prev = NULL;
	node->datalen = 0;
	if (0 == node->flag_mount) {
		free(node->data);
	}
	free(node);
	return len;
}

/*******************************************************************************************************************
**	函数名:List_Del_Node
**	描	述:删除节点
**	参	数:[in]node:待删除节点
**	返回值:无
********************************************************************************************************************/
void List_Del_Node(LIST_T *node)
{
	LIST_T *node_next = NULL, *node_prev = NULL;

	if (node == NULL)
		return;

	node_next = node->next;
	node_prev = node->prev;

	if (node_next != node) {
		node_next->prev = node_prev;
		node_prev->next = node_next;
	}

	if (node->data != NULL && 0 == node->flag_mount) {
		free(node->data);
	}

	node->datalen = 0;
	node->data_mask = 0;
	node->flag_mount = 0;
	node->next = NULL;
	node->prev = NULL;
	free(node);
}

/*******************************************************************************************************************
**	函数名:List_Del_List
**	描	述:删除整条链表
**	参	数:[in/out]head:链表尾节点
**	返回值:无
********************************************************************************************************************/
void List_Del_List(LIST_T **head)
{
	LIST_T *node = NULL, *head_node = NULL;

	if (*head == NULL)
		return;

	head_node = *head;

	do {
		node = head_node->prev;
		List_Del_Node(node);
	} while (node != head_node);

	*head = NULL;
}

/*******************************************************************************************************************
**	函数名:List_Get_Tail
**	描	述:获取链表头节点
**	参	数:[in/out]head:链表尾节点
**		   [in]data:待取节点数据缓存区
**		   [in]buflen:缓存长度
**	返回值:链表头节点
********************************************************************************************************************/
LIST_T *List_Node_Poll(LIST_T **head)
{
	LIST_T *node;

	if (head == NULL)
		return NULL;

	if (*head == NULL)
		return NULL;

	node = *head;
	*head = node->next;
	return node;
}

/*******************************************************************************************************************
**	函数名:List_Select_Node
**	描	述:获取链表某个节点
**	参	数:[in]head:链表尾节点
**		   [in]select_node:外部实现查询策略
**	返回值:链表节点
********************************************************************************************************************/
LIST_T *List_Select_Node(LIST_T *head, char (*select_node)(void *data))
{
	LIST_T *node = head;

	if (NULL == head || NULL == select_node)
		return NULL;

	do {
		if (select_node(node->data) == 1)
			return node;
		node = node->next;
	} while (node != head);
	
	return NULL;	
}

/*******************************************************************************************************************
**	函数名:List_Select_Node_Data
**	描	述:获取链表某个节点数据域
**	参	数:[in]head:链表尾节点
**		   [in]select_node:外部实现查询策略
**		   [in]mask:数据域掩码
**	返回值:链表节点数据
********************************************************************************************************************/
void *List_Select_Node_Data(LIST_T *head, char (*select_node)(void *data), char mask)
{
	LIST_T *node = NULL;

	if (NULL == head || NULL == select_node)
		return NULL;

	node = List_Select_Node(head, select_node);

	if (node != NULL && mask == node->data_mask)
		return node->data;
	return NULL;	
}

