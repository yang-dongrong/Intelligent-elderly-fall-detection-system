/*
Arduino-MAX30100 oximetry / heart rate integrated sensor library
Copyright (C) 2016  OXullo Intersecans <x@brainrapers.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//#include <Wire.h>

#include "MAX30100.h"
#include "uart.h"
#include "delay.h"
#include "myiic.h"
#include "MAX30100_Filters.h"
uint16_t rawIRValue;		//红外FIFO数据
uint16_t rawRedValue;		//红光FIFO数据
uint8_t PartID;					//设备ID

void MAX30100Init(void)
{

	Write_One_Byte(MAX30100_REG_MODE_CONFIGURATION, MAX30100_MC_RESET);

	
}

void max_begin()
{

		IIC_Init();						
    setMode(DEFAULT_MODE);								// HR_ONLY
    setLedsPulseWidth(DEFAULT_PULSE_WIDTH);//16bits
    setSamplingRate(DEFAULT_SAMPLING_RATE);//100HZ
    setLedsCurrent(DEFAULT_IR_LED_CURRENT, DEFAULT_RED_LED_CURRENT);//Ir=Red=50MA
	  setHighresModeEnabled(1);
}

void setMode(Mode mode)
{

	   Write_One_Byte(MAX30100_REG_MODE_CONFIGURATION, mode);
//	printf("%d\r\n",Write_One_Byte(MAX30100_REG_MODE_CONFIGURATION, mode));
}

void setLedsPulseWidth(LEDPulseWidth ledPulseWidth)
{


	  uint8_t previous;  previous=Read_One_Byte(MAX30100_REG_SPO2_CONFIGURATION);
    Write_One_Byte(MAX30100_REG_SPO2_CONFIGURATION, (previous & 0xfc) | ledPulseWidth);
}

void setSamplingRate(SamplingRate samplingRate)
{

			uint8_t previous; previous=Read_One_Byte(MAX30100_REG_SPO2_CONFIGURATION);
	    Write_One_Byte(MAX30100_REG_SPO2_CONFIGURATION, (previous & 0xe3) | (samplingRate << 2));

}

void setLedsCurrent(LEDCurrent irLedCurrent, LEDCurrent redLedCurrent)
{

			Write_One_Byte(MAX30100_REG_LED_CONFIGURATION, redLedCurrent << 4 | irLedCurrent);
}

void setHighresModeEnabled(u8 enabled)
{

	   uint8_t previous; 
	   previous=Read_One_Byte(MAX30100_REG_SPO2_CONFIGURATION);
	   if (enabled) {
        Write_One_Byte(MAX30100_REG_SPO2_CONFIGURATION, previous | MAX30100_SPC_SPO2_HI_RES_EN);
    } else {
        Write_One_Byte(MAX30100_REG_SPO2_CONFIGURATION, previous & ~MAX30100_SPC_SPO2_HI_RES_EN);
    }
}

void update()
{
    readFifoData();
}

void readFifoData()
{
    uint8_t buffer[4];

    Buff_Read(MAX30100_REG_FIFO_DATA,buffer, 4);

    
    rawIRValue = (buffer[0] << 8) | buffer[1];
    rawRedValue = (buffer[2] << 8) | buffer[3];
}

u8 Write_One_Byte(u8 addr,u8 data)
{
	 IIC_Start();
    IIC_Send_Byte(0xAE);        //发送地址+读命令
    if(IIC_Wait_Ack())          //等待ACK
    {
        goto RESTATE;
    }
    IIC_Send_Byte(addr);         //发送寄存器地址
    if(IIC_Wait_Ack())          //等待ACK
    {
        goto RESTATE;
    }
    IIC_Send_Byte(data);        //发送数据
    if(IIC_Wait_Ack())          //等待ACK
    {
        goto RESTATE;;
    }
 
    IIC_Stop();
    return 1;
         
RESTATE:        
    IIC_Stop();
    return 0;
}
u8 Read_One_Byte(u8 addr)
{
   uint8_t res;
    IIC_Start();
    IIC_Send_Byte(0xAE); //发送期间地址+写命令
    if(IIC_Wait_Ack())          //等待ACK
    {
        goto RESTATE;
    }
    IIC_Send_Byte(addr);         //发送寄存器地址
    if(IIC_Wait_Ack())          //等待ACK
    {
        goto RESTATE;
    }
      IIC_Start();   
    IIC_Send_Byte(0xAF);  //发送器件地址+读命令
    if(IIC_Wait_Ack())          //等待ACK
    {
        goto RESTATE;
    }
    res=IIC_Read_Byte(0);       //
    IIC_Stop();                 //
    return res; 
     
RESTATE:        
    IIC_Stop();
    return 0;
}

u8 Buff_Read(u8 address,u8 *buf, u8 len)
{
  IIC_Start(); 
	IIC_Send_Byte(0xae);//发送器件地址+写命令	
	if(IIC_Wait_Ack())	//等待应答
	{
		IIC_Stop();		 
		return 1;		
	}
	IIC_Send_Byte(address);	//写寄存器地址
	IIC_Wait_Ack();		//等待应答
	IIC_Start();
	IIC_Send_Byte(0xaf);//发送器件地址+读命令	
  IIC_Wait_Ack();		//等待应答 
	while(len)
	{
		if(len==1)*buf=IIC_Read_Byte(0);//读数据,发送nACK 
		else *buf=IIC_Read_Byte(1);		//读数据,发送ACK  
		len--;
		buf++; 
	}
  IIC_Stop();	//产生一个停止条件 
	return 0;	
}

