#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <string.h>
#include <termios.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/time.h>

#include "lib_debug.h"

#define Max_Msg_Info_Len 100

DEBUG_INFO Dbg_Info;

/********************************************************************************************/
/*	函数名:Debug_Info_Free 																	*/
/*	描	述:调试注册信息回收接口	 																	*/
/*	参	数:[in]info:调试接口注册信息																	*/
/*	返回值:无																					*/
/********************************************************************************************/
static void Debug_Info_Free(DEBUG_INFO *info, DEBUG_TYPE_T type)
{
	if (info == NULL)
		return;

	switch(type) {
		case DEBUG_TYPE_CONSOLE:
			info->DebugType &= (~(1 < DEBUG_TYPE_CONSOLE));
			break;
		case DEBUG_TYPE_UART:
			if (info->Uart) {
				if (info->Uart->uart_fd > 0)
					close(info->Uart->uart_fd);
				free(info->Uart);
			}
			info->Uart = NULL;
			info->DebugType &= (~(1 < DEBUG_TYPE_UART));
			break;
		case DEBUG_TYPE_NET:
			if (info->Net) {
				if (info->Net->net_fd > 0)
					close(info->Net->net_fd);
				free(info->Net);
			}
			info->Net = NULL;
			info->DebugType &= (~(1 < DEBUG_TYPE_NET));
			break;
		case DEBUG_TYPE_LOG:
			if (info->Log) {
				free(info->Log);
			}
			info->Log = NULL;
			info->DebugType &= (~(1 < DEBUG_TYPE_LOG));
			break;
		case DEBUG_TYPE_LCD:
			if (info->Lcd) {
				if (info->Lcd->lcd_fd > 0)
					close(info->Lcd->lcd_fd);
				free(info->Lcd);
			}
			info->Lcd = NULL;
			info->DebugType &= (~(1 < DEBUG_TYPE_LCD));
			break;
		default:
			break;
	}
	
}

/********************************************************************************************/
/*	函数名:Debug_Msg_Uart 																		*/
/*	描	述:调试信息输出到串口		 																	*/
/*	参	数:[in]info:调试	注册信息																*/
/*		   [in]data:调试	信息	数据																*/
/*	返回值:无																					*/
/********************************************************************************************/
static void Debug_Msg_Uart(DEBUG_INFO *info, char *data)
{
	char ret = 0;
	int data_len = strlen(data);
	int total = 0;

	if (info == NULL || data == NULL)
		return;

	if (info->Uart == NULL) {
		Debug_Info_Free(info,DEBUG_TYPE_UART);
		return;
	}

	if (info->Uart->uart_fd) {
		total = 0;
		do
		{
			ret = write(info->Uart->uart_fd, (data + total), (data_len - total));
			if(ret < 0 && (errno == EINTR || errno == EAGAIN)) {
				usleep(1000);
				continue;
			} else if (ret <= 0) {																					/*发送失败，则关闭串口,回收相关资源*/
				Debug_Info_Free(info,DEBUG_TYPE_UART);
				return;
			}
			total += ret;
			if(total == data_len) break;
		}while(1);
		return;
	}
}

/********************************************************************************************/
/*	函数名:Debug_Msg_Net 																		*/
/*	描	述:调试信息输出到网络		 																	*/
/*	参	数:[in]info:调试	注册信息																*/
/*		   [in]data:调试	信息	数据																*/
/*	返回值:无																					*/
/********************************************************************************************/
static void Debug_Msg_Net(DEBUG_INFO *info, char *data)
{
	char ret = 0;
	int data_len = strlen(data);

	if (info == NULL || data == NULL)
		return;

	if (info->Net == NULL) {
		Debug_Info_Free(info,DEBUG_TYPE_NET);
		return;
	}

	if (info->Net->net_fd > 0 && info->Net->Debug_Net_Interface != NULL) {
		ret = info->Net->Debug_Net_Interface(data,data_len);

		if (ret = FALSE) {
			Debug_Info_Free(info,DEBUG_TYPE_NET);
		}
	}
}

/********************************************************************************************/
/*	函数名:Debug_Get_File_Size 																*/
/*	描	述:获取文件大小		 																	*/
/*	参	数:[in]filename:日志文件名																*/
/*	返回值:文件大小																				*/
/********************************************************************************************/
static long Debug_Get_File_Size(const char *filename)
{
    long size = 0;
    FILE *fp = NULL;

    fp = fopen(filename, "r");

    if (fp) {
        fseek(fp, 0L, SEEK_END);
        size = ftell(fp);
        fclose(fp);
    }

    return size;
}

/********************************************************************************************/
/*	函数名:Debug_Msg_Log 																		*/
/*	描	述:调试信息输出到日志文件		 																*/
/*	参	数:[in]info:调试	注册信息																*/
/*		   [in]data:调试	信息	数据																*/
/*	返回值:无																					*/
/********************************************************************************************/
static void Debug_Msg_Log(DEBUG_INFO *info, char *data)
{
	long size = 0;
	if (info == NULL || data == NULL)
		return;

	if (info->Log == NULL) {
		Debug_Info_Free(info,DEBUG_TYPE_LOG);
		return;
	}

	if (strlen(info->Log->log_name) < 2) {
		Debug_Info_Free(info,DEBUG_TYPE_LOG);
		return;
	}
		
	size = Debug_Get_File_Size(info->Log->log_name);

    if (size >= info->Log->max_log_size || size == 0) {
       	info->Log->log_fd = fopen(info->Log->log_name, "w");
    } else {
    	info->Log->log_fd = fopen(info->Log->log_name, "a");
        fseek(info->Log->log_fd, 0L, SEEK_END);
    }

	if (info->Log->log_fd != NULL ) {
		fprintf(info->Log->log_fd, "%s",data);
		fclose(info->Log->log_fd);
	} else {
		Debug_Info_Free(info,DEBUG_TYPE_LOG);
	}
}

/********************************************************************************************/
/*	函数名:Debug_Msg_Lcd 																		*/
/*	描	述:调试信息输出到lcd屏		 																*/
/*	参	数:[in]info:调试	注册信息																*/
/*		   [in]data:调试	信息	数据																*/
/*	返回值:无																					*/
/********************************************************************************************/
static void Debug_Msg_Lcd(DEBUG_INFO *info, char *data)
{
	char ret = 0;
	int data_len = strlen(data);

	if (info == NULL || data == NULL)
		return;

	if (info->Lcd == NULL) {
		Debug_Info_Free(info,DEBUG_TYPE_LCD);
		return;
	}

	if (info->Lcd->lcd_fd > 0 && info->Lcd->Debug_LCD_Interface != NULL) {
		ret = info->Lcd->Debug_LCD_Interface(data,data_len);

		if (ret = FALSE) {
			Debug_Info_Free(info,DEBUG_TYPE_LCD);
		}
	}
}

/********************************************************************************************/
/*	函数名:Debug_Msg 																			*/
/*	描	述:调试信息输出		 																	*/
/*	参	数:[in]debug_type:调试信息输出类型															*/
/*		   [in]swit:调试输出开关																	*/
/*		   [in]data:调试输出信息																	*/
/*	返回值:无																					*/
/********************************************************************************************/
void Debug_Msg(unsigned char debug_type, DEBUG_SWITCH_T swit, char *data)
{
	if (debug_type == 0 || swit == DEBUG_SWITCH_CLOSE || data == NULL)
		return;

	if (debug_type & (1 < DEBUG_TYPE_CONSOLE)) {
		printf("%s",data);
	}

	if (debug_type & (1 < DEBUG_TYPE_UART) && Dbg_Info.Uart) {
		Debug_Msg_Uart(&Dbg_Info,data);
	}

	if (debug_type & (1 < DEBUG_TYPE_NET) && Dbg_Info.Net) {
		Debug_Msg_Net(&Dbg_Info,data);
	}

	if (debug_type & (1 < DEBUG_TYPE_LOG) && Dbg_Info.Log) {
		Debug_Msg_Log(&Dbg_Info,data);
	}

	if (debug_type & (1 < DEBUG_TYPE_LCD) && Dbg_Info.Lcd) {
		Debug_Msg_Lcd(&Dbg_Info,data);
	}
	
}

/********************************************************************************************/
/*	函数名:Debug_Get_Uart_Bandrate 															*/
/*	描	述:转换串口波特率		 																	*/
/*	参	数:[in]bandrate:波特率																	*/
/*	返回值:波特率 		-1:无效																	*/
/********************************************************************************************/
static int Debug_Get_Uart_Bandrate(unsigned int bandrate)
{
	switch(bandrate) {
		case 50: return B50;
        case 75: return B75;
        case 110: return B110;
        case 134: return B134;
        case 150: return B150;
        case 200: return B200;
        case 300: return B300;
        case 600: return B600;
        case 1200: return B1200;
        case 1800: return B1800;
        case 2400: return B2400;
        case 4800: return B4800;
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200;
        case 230400: return B230400;
        case 460800: return B460800;
        case 500000: return B500000;
        case 576000: return B576000;
        case 921600: return B921600;
        case 1000000: return B1000000;
        case 1152000: return B1152000;
        case 1500000: return B1500000;
        case 2000000: return B2000000;
		default: return -1;
	}
}

/********************************************************************************************/
/*	函数名:Debug_Uart_Open 																	*/
/*	描	述:打开uart串口设备	 																	*/
/*	参	数:[in]uart:串口设备信息																	*/
/*	返回值:1-成功,0-失败																			*/
/********************************************************************************************/
static char Debug_Uart_Open(DEBUG_UART_INFO *uart)
{
	struct termios termios_settings;
	int controlbits = 0;
	int bandrate = 0;

	if (uart == NULL)
		return 0;

	bandrate = Debug_Get_Uart_Bandrate(uart->baudfate);
	if (bandrate == -1)
		return -1;

	if (uart->databits != UART_DATABIT_5 && uart->databits != UART_DATABIT_6 &&
		uart->databits != UART_DATABIT_7 && uart->databits != UART_DATABIT_8)
		return -1;

	if (uart->stopbits != UART_STOPBIT_1 && uart->stopbits != UART_STOPBIT_2)
		return -1;

	if (uart->parity != UART_PARITY_NONE && uart->parity != UART_PARITY_ODD &&
		uart->parity != UART_PARITY_EVEN)
		return -1;

	if ((uart->uart_fd = open(uart->uart_name, O_RDWR | O_NOCTTY)) < 0){
        return -1;
    }

	memset(&termios_settings, 0, sizeof(termios_settings));

	/* c_iflag */

    /* Ignore break characters */
    termios_settings.c_iflag = IGNBRK;
    if (uart->parity != UART_PARITY_NONE)
        termios_settings.c_iflag |= (INPCK | ISTRIP);        

    /* c_oflag */
    termios_settings.c_oflag = 0;

    /* c_lflag */
    termios_settings.c_lflag = 0;

    /* c_cflag */
    /* Enable receiver, ignore modem control lines */
    termios_settings.c_cflag = CREAD | CLOCAL;

    /* Databits */
    if (uart->databits == UART_DATABIT_5)
        termios_settings.c_cflag |= CS5;
    else if (uart->databits == UART_DATABIT_6)
        termios_settings.c_cflag |= CS6;
    else if (uart->databits == UART_DATABIT_7)
        termios_settings.c_cflag |= CS7;
    else if (uart->databits == UART_DATABIT_8)
        termios_settings.c_cflag |= CS8;

    /* Parity */
    if (uart->parity == UART_PARITY_EVEN)
        termios_settings.c_cflag |= PARENB;
    else if (uart->parity == UART_PARITY_ODD)
        termios_settings.c_cflag |= (PARENB | PARODD);

    /* Stopbits */
    if (uart->stopbits == UART_STOPBIT_2)
        termios_settings.c_cflag |= CSTOPB;

    /* RTS/CTS */
    if (uart->flowctrl == UART_FLOWCTRL_SOFT)
    {
        termios_settings.c_iflag |= (IXON | IXOFF);
    } else if (uart->flowctrl == UART_FLOWCTRL_HARD) {
		termios_settings.c_cflag |= CRTSCTS;
	}

    /* Baudrate */
    cfsetispeed(&termios_settings, bandrate);
    cfsetospeed(&termios_settings, bandrate);

    /* Set termios attributes */
    if (tcsetattr(uart->uart_fd, TCSANOW, &termios_settings) < 0) {
        close(uart->uart_fd);
		perror("set serial port:");
        return -1;
    }

    ioctl(uart->uart_fd, TIOCMGET, &controlbits);

    /* RTS/CTS */
    if (uart->flowctrl == UART_FLOWCTRL_HARD)
    {
        controlbits |= TIOCM_RTS;
    } else {
        controlbits &= ~TIOCM_RTS;
    }

    ioctl(uart->uart_fd, TIOCMSET, &controlbits);

	return 1;	
}


/********************************************************************************************/
/*	函数名:Debug_Init 																			*/
/*	描	述:调试注册信息回收接口	 																	*/
/*	参	数:[in]info:调试接口注册信息																	*/
/*	返回值:注册成功的接口,bit[0]-控制台 bit[1]-串口 bit[2]-网络												*/
/*			bit[3]-日志文件 bit[4]-LCD屏通道														*/
/********************************************************************************************/
char Debug_Init(DEBUG_INFO *info)
{
	unsigned char ret;
	if (info == NULL)
		return -1;

	if (info->DebugType == 0) {
		return -1;
	}

	if (info->DebugType & (1 < DEBUG_TYPE_CONSOLE)) {
		Debug_Info_Free(&Dbg_Info,DEBUG_TYPE_CONSOLE);
		Dbg_Info.DebugType |= (1 < DEBUG_TYPE_CONSOLE);
	}

	ret = 0;
	if (info->DebugType & (1 < DEBUG_TYPE_UART) && info->Uart) {
		Debug_Info_Free(&Dbg_Info,DEBUG_TYPE_UART);
		Dbg_Info.Uart = (DEBUG_UART_INFO *)malloc(sizeof(DEBUG_UART_INFO));
		if (Dbg_Info.Uart) {

			memcpy(Dbg_Info.Uart,info->Uart,sizeof(DEBUG_UART_INFO));
			ret = Debug_Uart_Open(Dbg_Info.Uart);

			if (ret == 1) {
				Dbg_Info.DebugType |= (1 < DEBUG_TYPE_UART);
			} else {
				free(Dbg_Info.Uart);
				Dbg_Info.Uart = NULL;
			}	
		}
	}

	ret = 0;
	if (info->DebugType & (1 < DEBUG_TYPE_NET) && info->Net) {
		Debug_Info_Free(&Dbg_Info,DEBUG_TYPE_NET);
		Dbg_Info.Net = (DEBUG_NET_INFO *)malloc(sizeof(DEBUG_NET_INFO));
		if (Dbg_Info.Net) {
			if (info->Net->Debug_Net_Interface != NULL && info->Net->net_fd >0) {
				Dbg_Info.Net->net_fd = info->Net->net_fd;
				Dbg_Info.Net->Debug_Net_Interface = info->Net->Debug_Net_Interface;
				ret = 1;
			}

			if (ret == 1) {
				Dbg_Info.DebugType |= (1 < DEBUG_TYPE_UART);
			} else {
				free(Dbg_Info.Net);
				Dbg_Info.Net = NULL;
			}	
		}
	}

	if (info->DebugType & (1 < DEBUG_TYPE_LOG) && info->Log) {
		Debug_Info_Free(&Dbg_Info,DEBUG_TYPE_LOG);
		Dbg_Info.Log = (DEBUG_LOG_INFO *)malloc(sizeof(DEBUG_LOG_INFO));
		if (Dbg_Info.Log) {
			memcpy(Dbg_Info.Log,info->Log,sizeof(DEBUG_UART_INFO));
			Dbg_Info.DebugType |= (1 < DEBUG_TYPE_UART);
	
		}
	}

	ret = 0;
	if (info->DebugType & (1 < DEBUG_TYPE_LCD) && info->Lcd) {
		Debug_Info_Free(&Dbg_Info,DEBUG_TYPE_LCD);
		Dbg_Info.Lcd = (DEBUG_LCD_INFO *)malloc(sizeof(DEBUG_LCD_INFO));
		if (Dbg_Info.Lcd) {
			if (info->Lcd->Debug_LCD_Interface != NULL && info->Lcd->lcd_fd >0) {
				Dbg_Info.Lcd->lcd_fd = info->Lcd->lcd_fd;
				Dbg_Info.Lcd->Debug_LCD_Interface = info->Lcd->Debug_LCD_Interface;
				ret = 1;
			}

			if (ret == 1) {
				Dbg_Info.DebugType |= (1 < DEBUG_TYPE_UART);
			} else {
				free(Dbg_Info.Lcd);
				Dbg_Info.Lcd = NULL;
			}	
		}
	}

	return Dbg_Info.DebugType;
}


