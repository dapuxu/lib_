#include "lib_file.h"
#include <fcntl.h>
#include "includes.h"

/*******************************************************************************************************************
**	函数名:File_Init
**	描	述:文件块初始化
**	参	数:[in]block:文件块
**         [in]file_path:文件路径
**	返回值:1-存在/0-不存在
********************************************************************************************************************/
char File_Init(FILE_BLOCK_T *block,char *file_path)
{

	if (NULL == file_path || NULL == block)
		return 0;

	if (strlen(file_path) > sizeof(block->file_path))
		return 0;

	memcpy(block->file_path, file_path, strlen(file_path));
	block->flag_lock = 0;
	return 1;
}

/*******************************************************************************************************************
**	函数名:File_Access
**	描	述:检测文件是否存在
**	参	数:[in]file_path:文件路径
**	返回值:1-存在/0-不存在
********************************************************************************************************************/
char File_Access(char *file_path)
{
	if (NULL == file_path)
		return 0;

	if (0 == access(file_path,F_OK))
		return 1;

	return 0;
}

/*******************************************************************************************************************
**	函数名:File_Size
**	描	述:获取文件大小
**	参	数:[in]file_path:文件路径
**	返回值:文件数据大小
********************************************************************************************************************/
unsigned int File_Size(char *file_path)
{
	FILE *fp;
	int size = 0;

	if (NULL == file_path || 0 == File_Access(file_path))
		return 0;
	
	fp=fopen(file_path,"r");
    if(!fp)
		return 0;
    fseek(fp,0L,SEEK_END);
    size=ftell(fp);
    fclose(fp);

    return size;
}

/*******************************************************************************************************************
**	函数名:File_Write_Cover
**	描	述:覆盖写入文件
**	参	数:[in]file_path:文件路径
**		   [in]data:数据
**		   [in]datalen:数据大小
**	返回值:数据写入大小
********************************************************************************************************************/
signed int File_Write_Cover(FILE_BLOCK_T *block, char *data, int datalen)
{
	FILE *fp;
	int ret;

	if (NULL == block || NULL == data || 0 >= datalen)
		return -1;

	if (1 == block->flag_lock)
		return 0;
	block->flag_lock = 1;
	fp=fopen(block->file_path,"w");
    if(!fp) {
		block->flag_lock = 0;
		return -1;
	}
    ret = fwrite(data, datalen, 1, fp);
    fclose(fp);
	block->flag_lock = 0;
    return ret;
}

/*******************************************************************************************************************
**	函数名:File_Write_Add
**	描	述:追加写入文件
**	参	数:[in]file_path:文件路径
**		   [in]data:数据
**		   [in]datalen:数据大小
**	返回值:数据写入大小
********************************************************************************************************************/
signed int File_Write_Add(FILE_BLOCK_T *block, char *data, int datalen)
{
	FILE *fp;
	int ret = 0;

	if (NULL == block || NULL == data || 0 >= datalen)
		return 0;

	if (1 == block->flag_lock)
		return 0;
	block->flag_lock = 1;
	fp=fopen(block->file_path,"a");
    if(!fp) {
		block->flag_lock = 0;
		return 0;
	}
    ret = fwrite(data, datalen, 1, fp);
    fclose(fp);
	block->flag_lock = 0;
    return ret;
}

/*******************************************************************************************************************
**	函数名:File_Read
**	描	述:读取文件数据
**	参	数:[in]block:文件块
**		   [in]buf:缓存
**		   [in]buflen:缓存大小
**	返回值:读取到的数据大小
********************************************************************************************************************/
unsigned int File_Read(FILE_BLOCK_T *block, char *buf, int buflen)
{
	FILE *fp;
	int size = 0, ret = 0;

	if (NULL == block || NULL == buf || 0 >= buflen)
		return 0;

	if (0 == File_Access(block->file_path))
		return 0;
	size = File_Size(block->file_path);
	size = (size<buflen)?size:buflen;
	fp=fopen(block->file_path,"r");
    if(!fp)
		return 0;
    
    ret = fread(buf, size, 1, fp);
    fclose(fp);

    return ret;
}

/*******************************************************************************************************************
**	函数名:File_Read
**	描	述:读取文件指定行数据
**	参	数:[in]block:文件块
**		   [in]line:指定行
**		   [in]buf:缓存
**		   [in]buflen:缓存大小
**	返回值:读取到的数据大小
********************************************************************************************************************/
unsigned int File_Read_Line(FILE_BLOCK_T *block, unsigned short line, char *buf, int buflen)
{
	FILE *fp;
	int size = 0, ret = 0;
	unsigned short line_cur = 0;
	char *linedata = NULL;

	if (NULL == block || NULL == buf || 0 >= buflen)
		return 0;

	if (0 == File_Access(block->file_path))
		return 0;

	linedata = (char *)malloc(buflen);
	memset(linedata, 0x0, buflen);
	if (NULL == linedata)
		return 0;
	size = File_Size(block->file_path);
	size = (size<buflen)?size:buflen;
	fp=fopen(block->file_path,"r");
    if(!fp) {
		free(linedata);
		return 0;
	}

	while (fgets(linedata, buflen, fp) != NULL) {
		if (++line_cur == line) {
			memcpy(buf, linedata, buflen);
			ret = strlen(linedata);
		break;
		}
	}
    fclose(fp);
	free(linedata);

    return ret;
}


