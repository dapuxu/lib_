#include "lib_string.h"
#include "includes.h"

/*******************************************************************************************************************
**	函数名:String_Select_Key
**	描	述:查询缓存中的字符串位置
**	参	数:[in]str:字符串
**		   [in]key:关键词
**	返回值:字符串中关键词所在的位置
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
**	函数名:String_Select_Key_Num
**	描	述:查询字符串中关键词个数
**	参	数:[in]str:字符串
**		   [in]key:关键词
**	返回值:关键词个数
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
**	函数名:String_Splicing
**	描	述:字符串拼接
**	参	数:[out]buf:缓存
**		   [in]buflen:缓存长度
**		   [in]front:前字符串
**		   [in]last:后字符串
**	返回值:字符串中关键词所在的位置
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

/*******************************************************************************************************************
**	函数名:String_Splicing
**	描	述:字符串拼接
**	参	数:[in]data:数据
**	返回值:数据长度
********************************************************************************************************************/
char String_Length(char *data)
{

	if (NULL == data)
		return 0;

	return strlen(data);
}

