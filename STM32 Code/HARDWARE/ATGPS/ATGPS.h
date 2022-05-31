/******************** *************************

 * 说明UART2
 * 硬件连接: TX->PA2;RX->PA3

**********************************************/	

#include "stdio.h"	
#include "sys.h" 
#include "stdarg.h"	 	 	 
#include "string.h"	


/****************                       ******   */


/************************************/
//定义数组长度
#define GPS_Buffer_Length 80
#define UTCTime_Length 11
#define latitude_Length 11
#define N_S_Length 2
#define longitude_Length 12
#define E_W_Length 2 
#define false 0
#define true 1
#define USART_REC_LEN  			200  	//定义最大接收字节数 200	

#define CMD_BUFFER_LEN2 255
/*******************************/

typedef struct SaveData 
{
	char GPS_Buffer[GPS_Buffer_Length];
	char isGetData;		//是否获取到GPS数据
	char isParseData;	//是否解析完成
	char UTCTime[UTCTime_Length];		//UTC时间
	char latitude[latitude_Length];		//纬度
	char N_S[N_S_Length];		//N/S
	char longitude[longitude_Length];		//经度
	char E_W[E_W_Length];		//E/W
	char isUsefull;		//定位信息是否有效
} _SaveData;
/********************************/

extern char rxdatabufer;
extern u16 point1;
extern char USART2_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
extern _SaveData Save_Data;



/****************函数初始化**********************/
u8 Hand(char *a);
void clrStruct(void);
void CLR_Buf(void);
void GPS_USART2_Init(u32 My_BaudRate);
void UART2_Send_Str(unsigned char *s);
void errorLog(int num);
void parseGpsBuffer(void);
void printGpsBuffer(void);
void printf2(char *fmt, ...) ;
