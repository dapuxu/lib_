#include "lib_string.h"
#include "includes.h"

/*******************************************************************************************************************
**	������:String_Select_Key
**	��	��:��ѯ�����е��ַ���λ��
**	��	��:[in]str:�ַ���
**		   [in]key:�ؼ���
**	����ֵ:�ַ����йؼ������ڵ�λ��
********************************************************************************************************************/
char *String_Select_Key(char *str, char *key)
{
	char *pos;

	if (NULL == str || NULL == key)
		return NULL;

	pos = str;
	while (NULL != (pos = strchr(pos,key[0]))) {
		if (!memcmp(key,pos,strlen(key))) {
			return str+strlen(str)-strlen(pos);
		}
		pos++;
	}
	return NULL;
}

/*******************************************************************************************************************
**	������:String_Select_Key_Num
**	��	��:��ѯ�ַ����йؼ��ʸ���
**	��	��:[in]str:�ַ���
**		   [in]key:�ؼ���
**	����ֵ:�ؼ��ʸ���
********************************************************************************************************************/
unsigned short String_Select_Key_Num(char *str, char *key)
{
	char *pos;
	unsigned short num = 0;

	if (NULL == str || NULL == key)
		return num;

	pos = str;
	while (NULL != (pos = strchr(pos,key[0]))) {
		if (!memcmp(key,pos,strlen(key))) {
			pos += strlen(key);
			num++;
			continue;
		}
		pos++;
	}
	return num;
}

/*******************************************************************************************************************
**	������:String_Splicing
**	��	��:�ַ���ƴ��
**	��	��:[out]buf:����
**		   [in]buflen:���泤��
**		   [in]front:ǰ�ַ���
**		   [in]last:���ַ���
**	����ֵ:�ַ����йؼ������ڵ�λ��
********************************************************************************************************************/
char String_Splicing(char *buf, unsigned short buflen, char *front, char *last)
{
	char *tmp = NULL;
	unsigned short datalen = 0, len = 0;

	if (NULL == buf || buflen == 0)
		return 0;

	if (NULL == front && NULL == last)
		return 0;

	len = strlen(front)+strlen(last);
	tmp = (char *)malloc(len+1);
	memset(tmp, 0x0, len+1);

	sprintf(tmp, "%s%s", front, last);
	datalen = (len<buflen)?len:buflen;
	memcpy(buf, tmp, datalen);
	free(tmp);

	return datalen;
}


