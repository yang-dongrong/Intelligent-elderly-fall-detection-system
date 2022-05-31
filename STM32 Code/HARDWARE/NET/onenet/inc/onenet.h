#ifndef _ONENET_H_
#define _ONENET_H_

#include "stm32f10x.h"

_Bool OneNet_DevLink(void);

_Bool OneNet_Ping(void);

void OneNet_SendData(void);

void OneNet_RevPro(unsigned char *cmd);

extern signed short zitai; //shuaidao
extern signed short HeartRate_val; //心跳速率
extern u8 SPO2_val;							//血氧浓度
extern float temp;   //跌倒发送标志位
extern float longitude;
extern float latitude;

#endif
