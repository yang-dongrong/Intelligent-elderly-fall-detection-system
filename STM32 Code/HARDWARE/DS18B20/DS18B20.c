#include "stdio.h"
#include "delay.h"
#include "DS18B20.h"
#include "GPIOLIKE51.h"

int Ds18b20_Init(){
		int i=0;
		RCC->APB2ENR|=(1<<3);//开启GPIO时钟
		GPIOB->ODR |= 1<<7;//将GPIOB_PB7设为高电平
		DS18B20_OutPut_Mode();//让PB7进入输出模式
		PBout(7)=0;//输出0 拉低
		delay_us(642);//642 查看时序图得出 大于480小于960微秒
		PBout(7)=1;//输出1 拉高 Mark1
		delay_us(30);//查时序图得出 大于15小于60微秒
		
		DS18B20_InPut_Mode();//让PB7进入输入模式
		while(PBin(7)==1){//收到低电平则存在，否则循环5ms告诉不存在 看时序图应答是在60-240微秒 
					delay_ms(1);
					i++;
					if(i>5){
							return 0;//不存在返回0 Mark2
					}
		}
		return 1;//存在返回1
		//时序图可以看出从Mark1到Mark2之间不能多于300微秒，否则检测出错。
}


void Ds18b20WriteByte(uchar dat){
	int i;
	DS18B20_OutPut_Mode();
	for(i=0;i<8;i++){
		PBout(7)=0;	//进入写时序拉低
		delay_us(15);//写入时先拉低大于15微秒 
		PBout(7)=dat&0x01;//写入0或1
		delay_us(60);//写入1或0时都需要至少60微秒的间隙 
		PBout(7)=1;//再拉高恢复可写状态
		dat>>=1;//一共8位右移一位把下一位数据放在最右边
	}
}
uchar Ds18b20ReadByte(){
	int j;
	uchar dat,byte;
	for(j=8;j>0;j--){
	DS18B20_OutPut_Mode();
	PBout(7)=0;//看读时序
	delay_us(1);//拉低延迟1微秒
	PBout(7)=1;
	delay_us(10);//进入读的准备阶段10微秒
	DS18B20_InPut_Mode();
	dat=PBin(7);
	byte=(byte>>1)|(dat<<7);
	delay_us(45);//延迟45微秒读完1位
	DS18B20_OutPut_Mode();
	PBout(7)=1;//继续拉高为读下一位做准备
	}
	return byte;
}

void Ds18b20ChangeTemp(){
	Ds18b20_Init();
	delay_ms(1);
	Ds18b20WriteByte(0xcc);//跳过ROM直接发送温度转换命令
	Ds18b20WriteByte(0x44);//发送指令RAM设为0x44为温度变换
}

void Ds18b20ReadTempCom(){
	Ds18b20_Init();
	delay_ms(1);
	Ds18b20WriteByte(0xcc);//跳过ROM直接发送温度转换命令
	Ds18b20WriteByte(0xbe);//发送指令RAM设为0xBE为读暂时寄存器
}

int Ds18b20ReadTemp(void){
	int temp=0;
	uchar tml,tmh;	
	Ds18b20ChangeTemp();
	Ds18b20ReadTempCom();
	tml=Ds18b20ReadByte();//读低8位数据
	tmh=Ds18b20ReadByte();//读高8位数据
	temp=tmh;
	temp<<=8;
	temp|=tml;//拼接为16位数据
	return temp;//返回16位数据
}
