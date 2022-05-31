#ifndef _DS18B20_H
#define _DS18B20_H

#ifndef uchar
#define uchar unsigned char
#endif

#ifndef uint
#define uint unsigned int
#endif
//为什么要定义输入和输出模式？因为DS18B20是单总线，一根线进行输入和输出，所以要不停的切换模式
#define DS18B20_OutPut_Mode() {GPIOB->CRL &= 0x0FFFFFFF;GPIOB->CRL |= 0x30000000;}//输出模式
#define DS18B20_InPut_Mode()  {GPIOB->CRL &= 0x0FFFFFFF;GPIOB->CRL |= 0x80000000;}//输入模式

int Ds18b20_Init(void);
int Ds18b20ReadTemp(void);
#endif
