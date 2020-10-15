#include "lib_list.h"
#include "includes.h"

/*******************************************************************************************************************
**	������:List_Add
**	��	��:�������ӽڵ㣬������ʱ���´�����
**	��	��:[in/out]head:����ͷ�ڵ�
**		   [in]data:���������������
**		   [in]datalen:���ݳ���
**		   [in]mask:������������
**	����ֵ:����ͷ�ڵ�
********************************************************************************************************************/
LIST_T *List_Add(LIST_T **head, void *data, int datalen, char mask)
{
	LIST_T *node = NULL;
	LIST_T *tail_node = NULL;

	if (data == NULL || datalen <= 0)
		return *head;

	node = (LIST_T *)malloc(sizeof(LIST_T));
	if (node == NULL)
		return *head;

	node->data = (void *)malloc(datalen);
	if (node->data == NULL)
		return *head;

	memcpy(node->data, data, datalen);
	node->datalen = datalen;
	node->data_mask = mask;
	if (head == NULL) {
		node->next = node;
		node->prev = node;
		return node;
	} else {
		tail_node = (*head)->prev;
		(*head)->prev = node;
		node->next = (*head);
		tail_node->next = node;
		node->prev = tail_node;
		if (*head == NULL)
			*head = node;
		return *head;
	}
}

/*******************************************************************************************************************
**	������:List_Get_Head
**	��	��:��ȡ����ͷ�ڵ�
**	��	��:[in/out]head:����ͷ�ڵ�
**		   [in]data:��ȡ�ڵ����ݻ�����
**		   [in]buflen:���泤��
**	����ֵ:����ͷ�ڵ�
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
	free(node->data);
	free(node);
	return len;
}

/*******************************************************************************************************************
**	������:List_Get_Tail
**	��	��:��ȡ����ͷ�ڵ�
**	��	��:[in/out]head:����β�ڵ�
**		   [in]data:��ȡ�ڵ����ݻ�����
**		   [in]buflen:���泤��
**	����ֵ:����ͷ�ڵ�
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
	free(node->data);
	free(node);
	return len;
}

/*******************************************************************************************************************
**	������:List_Del_Node
**	��	��:ɾ���ڵ�
**	��	��:[in]node:��ɾ���ڵ�
**	����ֵ:��
********************************************************************************************************************/
void List_Del_Node(LIST_T *node)
{
	LIST_T *node_next = NULL, *node_prev = NULL;
	int len = 0;

	if (node == NULL)
		return;

	node_next = node->next;
	node_prev = node->prev;

	if (node_next != node) {
		node_next->prev = node_prev;
		node_prev->next = node_next;
	}

	if (node->data != NULL)
		free(node->data);

	node_prev->datalen = 0;
	node_prev->data_mask = 0;
	free(node);
}

/*******************************************************************************************************************
**	������:List_Del_List
**	��	��:ɾ����������
**	��	��:[in/out]head:����β�ڵ�
**	����ֵ:��
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
**	������:List_Get_Tail
**	��	��:��ȡ����ͷ�ڵ�
**	��	��:[in/out]head:����β�ڵ�
**		   [in]data:��ȡ�ڵ����ݻ�����
**		   [in]buflen:���泤��
**	����ֵ:����ͷ�ڵ�
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
**	������:List_Select_Node
**	��	��:��ȡ����ĳ���ڵ�
**	��	��:[in]head:����β�ڵ�
**		   [in]select_node:�ⲿʵ�ֲ�ѯ����
**	����ֵ:����ڵ�
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
**	������:List_Select_Node_Data
**	��	��:��ȡ����ĳ���ڵ�������
**	��	��:[in]head:����β�ڵ�
**		   [in]select_node:�ⲿʵ�ֲ�ѯ����
**		   [in]mask:����������
**	����ֵ:����ڵ�����
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

