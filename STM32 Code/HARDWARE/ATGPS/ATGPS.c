#include "ATGPS.h"

char rxdatabufer;
u16 point1 = 0;

char USART2_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
_SaveData Save_Data;

//初始化函数
void GPS_USART2_Init(u32 My_BaudRate)
{
	GPIO_InitTypeDef GPIO_InitStrue;
	USART_InitTypeDef USART_InitStrue;
	NVIC_InitTypeDef NVIC_InitStrue;
	
	// 外设使能时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	USART_DeInit(USART2);  //复位串口2 -> 可以没有
	
	// 初始化 串口对应IO口  TX-PA2  RX-PA3
	GPIO_InitStrue.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStrue.GPIO_Pin=GPIO_Pin_2;
	GPIO_InitStrue.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStrue);
	
	GPIO_InitStrue.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_InitStrue.GPIO_Pin=GPIO_Pin_3;
  GPIO_Init(GPIOA,&GPIO_InitStrue);
	
	// 初始化 串口模式状态
	USART_InitStrue.USART_BaudRate=My_BaudRate; // 波特率
	USART_InitStrue.USART_HardwareFlowControl=USART_HardwareFlowControl_None; // 硬件流控制
	USART_InitStrue.USART_Mode=USART_Mode_Tx|USART_Mode_Rx; // 发送 接收 模式都使用
	USART_InitStrue.USART_Parity=USART_Parity_No; // 没有奇偶校验
	USART_InitStrue.USART_StopBits=USART_StopBits_1; // 一位停止位
	USART_InitStrue.USART_WordLength=USART_WordLength_8b; // 每次发送数据宽度为8位
	USART_Init(USART2,&USART_InitStrue);
	
	USART_Cmd(USART2,ENABLE);//使能串口
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);//开启接收中断
	
	// 初始化 中断优先级
	NVIC_InitStrue.NVIC_IRQChannel=USART2_IRQn;
	NVIC_InitStrue.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStrue.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStrue.NVIC_IRQChannelSubPriority=1;
	NVIC_Init(&NVIC_InitStrue);
}





 
void USART2_IRQHandler(void) // 串口2中断服务函数
{
	u8 Res;
	if(USART_GetITStatus(USART2,USART_IT_RXNE)) // 中断标志
 {
     Res= USART_ReceiveData(USART2);  // 串口2 接收
    //USART_SendData(USART2,Res);   // 串口1 发送
	 if(Res == '$')
	{
		point1 = 0;	
	}
		

	  USART2_RX_BUF[point1++] = Res;

	if(USART2_RX_BUF[0] == '$' && USART2_RX_BUF[4] == 'M' && USART2_RX_BUF[5] == 'C')			//确定是否收到"GPRMC/GNRMC"这一帧数据
	{
		if(Res == '\n')									   
		{
			memset(Save_Data.GPS_Buffer, 0, GPS_Buffer_Length);      //清空
			memcpy(Save_Data.GPS_Buffer, USART2_RX_BUF, point1); 	//保存数据
			Save_Data.isGetData = true;
			point1 = 0;
			memset(USART2_RX_BUF, 0, USART_REC_LEN);      //清空				
		}	
				
	}
	
	if(point1 >= USART_REC_LEN)
	{
		point1 = USART_REC_LEN;
	}	 
  }
}
u8 Hand(char *a)                   // 串口命令识别函数
{ 
    if(strstr(USART2_RX_BUF,a)!=NULL)
	    return 1;
	else
		return 0;
}

void CLR_Buf(void)                           // 串口缓存清理
{
	memset(USART2_RX_BUF, 0, USART_REC_LEN);      //清空
  point1 = 0;                    
}

void clrStruct(void)
{
	Save_Data.isGetData = false;
	Save_Data.isParseData = false;
	Save_Data.isUsefull = false;
	memset(Save_Data.GPS_Buffer, 0, GPS_Buffer_Length);      //清空
	memset(Save_Data.UTCTime, 0, UTCTime_Length);
	memset(Save_Data.latitude, 0, latitude_Length);
	memset(Save_Data.N_S, 0, N_S_Length);
	memset(Save_Data.longitude, 0, longitude_Length);
	memset(Save_Data.E_W, 0, E_W_Length);
}

void printf2(char *fmt, ...) 
{ 
    char buffer[CMD_BUFFER_LEN2+1]; 
    u8 i = 0; 
    va_list arg_ptr; 
    va_start(arg_ptr, fmt);   
    vsnprintf(buffer, CMD_BUFFER_LEN2+1, fmt, arg_ptr); 
    while ((i < CMD_BUFFER_LEN2) && buffer[i]) 
    { 
            USART_SendData(USART2, (u8) buffer[i++]); 
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);  
    } 
    va_end(arg_ptr); 
}

void errorLog(int num)
{
	
	while (1)
	{
	  	//printf2("ERROR%d\r\n",num);
	}
}

void parseGpsBuffer()
{
			
	char *subString;
	char *subStringNext;
	char i = 0;
	if (Save_Data.isGetData)
	{
		Save_Data.isGetData = false;
		//printf2("**************\r\n");
		//printf2(Save_Data.GPS_Buffer);
		for (i = 0 ; i <= 6 ; i++)
		{
			if (i == 0)
			{
				if ((subString = strstr(Save_Data.GPS_Buffer, ",")) == NULL)
					errorLog(1);	//解析错误
			}
			else
			{
				subString++;
				if ((subStringNext = strstr(subString, ",")) != NULL)
				{
					char usefullBuffer[2]; 
					switch(i)
					{
						case 1:memcpy(Save_Data.UTCTime, subString, subStringNext - subString);break;	//获取UTC时间
						case 2:memcpy(usefullBuffer, subString, subStringNext - subString);break;	//获取UTC时间
						case 3:memcpy(Save_Data.latitude, subString, subStringNext - subString);break;	//获取纬度信息
						case 4:memcpy(Save_Data.N_S, subString, subStringNext - subString);break;	//获取N/S
						case 5:memcpy(Save_Data.longitude, subString, subStringNext - subString);break;	//获取经度信息
						case 6:memcpy(Save_Data.E_W, subString, subStringNext - subString);break;	//获取E/W

						default:break;
					}

					subString = subStringNext;
					Save_Data.isParseData = true;
					if(usefullBuffer[0] == 'A')
						Save_Data.isUsefull = true;
					else if(usefullBuffer[0] == 'V')
						Save_Data.isUsefull = false;

				}
				else
				{
					errorLog(2);	//解析错误
				}
			}


		}
	}
}

//void printGpsBuffer()
//{
//	if (Save_Data.isParseData)
//	{
//		Save_Data.isParseData = false;
//		
//		printf("Save_Data.UTCTime = ");
//		printf(Save_Data.UTCTime);//获取到的时间
//		printf("\r\n");

//		if(Save_Data.isUsefull)
//		{
//			Save_Data.isUsefull = false;
//			printf("Save_Data.latitude = ");
//			printf(Save_Data.latitude);//经度
//			printf("\r\n");


//			printf("Save_Data.N_S = ");
//			printf(Save_Data.N_S);
//			printf("\r\n");

//			printf("Save_Data.longitude = ");
//			printf(Save_Data.longitude);//纬度
//			printf("\r\n");

//			printf("Save_Data.E_W = ");
//			printf(Save_Data.E_W);
//			printf("\r\n");
//		}
//		else
//		{
//			printf("GPS 获取失败!\r\n");
//		}
//		
//	}
//}

