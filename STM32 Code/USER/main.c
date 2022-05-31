//头文件
#include "all.h"

unsigned short timeCount = 0;	//发送间隔变量
unsigned short pingCount = 0;	//发送间隔变量
unsigned char *dataPtr = NULL;

void OLED_Show(void);

char temp_data[16];
u8 fall_flag=0;    //跌倒发送标志位
float pitch,roll,yaw; 	//欧拉角
short aacx,aacy,aacz;		//加速度传感器原始数据
short gyrox,gyroy,gyroz;	//陀螺仪原始数据
char Buf[30] = {0};
u8 mpu_flag = 0;      				//人体跌倒检测标志位 标志位1、2任意一个异常 该标志位为1 	【1：跌倒，0：正常】
_Bool mpu_1_flag = 0;      		//人体跌倒检测标志位1	角度异常标志		【1：异常，0：正常】
_Bool mpu_2_flag = 0;      		//人体跌倒检测标志位2	加速度异常标志	【1：异常，0：正常】
int SVM;
u8 t=0,i=10,t1=10;

u32 HeartRatetime=0;
u32 gpstime=0;
float temp=0;
signed short zitai; 
signed short HeartRate_val; //心跳速率
u8 SPO2_val;							//血氧浓度
void GPRS_Init()
{
	
}
 int main(void)
{	
	SystemInit();	// 配置系统时钟为72M 
  delay_init();    //延时初始化	
	GPS_USART2_Init(9600);
	UART1_Init();;
  MPU_Init();					//初始化MPU6050
	delay_ms(300);			//等待初始化稳定
	while(mpu_dmp_init())
 	{
		printf("error");
 		delay_ms(300);
	}	
	TIM3_Int_Init(100-1,720-1);
	time2_config();
	OLED_Init();
	OLED_Show();
  //IIC_Init();					//心率血氧浓度模块IIC-----SCLK接到“B8”脚 SDIN接到“B9”脚
//	SPO2_Init();
	Ds18b20_Init();//初始化DS18B20
		OLED_ShowString(32,0,(u8*)":");
	printf("Hardware init OK\r\n");
	M6312_Init();					//初始化SIM800C
	while(OneNet_DevLink())			//接入OneNET
	delay_ms(500);
 	while(1)  
	{
		
		  POupdate();	
	  if(mpu_dmp_get_data(&pitch,&roll,&yaw)==0)
			{ 
				MPU_Get_Accelerometer(&aacx,&aacy,&aacz);	//得到加速度传感器数据		
				SVM = sqrt(pow(aacx,2)+  pow(aacy,2) + pow(aacz,2));	
			//	printf2("%.1f",roll);
			//	printf2("%d",SVM);
				//分析x、y、z角度的异常判断
				if( fabs(pitch)>50 || fabs(roll)>50 || fabs(yaw)>50 )//倾斜角度的 【绝对值】 大于40°SVM大于设定的阈值时，即认为摔倒
					mpu_1_flag = 1;	
				else 
					mpu_1_flag = 0;
				
				//分析加速度SVM的异常判断
				if( SVM>23000 || SVM<12000 )i = 0;
				i++;
				
				if( i<=10 )mpu_2_flag = 1;
				else 
				{
					i = 10;
					mpu_2_flag = 0;
				}
		
				//综合欧拉角、加速度SVM异常判断异常	
				if( mpu_2_flag || mpu_1_flag )mpu_flag = 1;
				else mpu_flag = 0;
				
			if(mpu_flag==1){
					t1=0;
					
				  zitai=1;
					GPIO_ResetBits(GPIOC,GPIO_Pin_0);	
				}
				else{
					zitai=0;
					
					GPIO_SetBits(GPIOC,GPIO_Pin_0);
				}
			t=0;
		}
	 
		t++;
		temp=(Ds18b20ReadTemp()*0.0625);   //获取人体温度
		if(gpstime>=5)
		{
		  OLED_ShowNum(40,0,HeartRate_val,2,16);
      OLED_ShowNum(104,0,SPO2_val,2,16);			
			
			
	 if(zitai==1)
	 {
		// printf("摔倒");
          OLED_ShowString(72,6,(u8*)"     ");
					OLED_ShowCHinese(72,6,3);
	        OLED_ShowCHinese(72+16,6,4);
	 }
	 else
	 {
	//	 printf("正常");
					OLED_ShowString(72,6,(u8*)"     ");
					OLED_ShowCHinese(72,6,5);
	        OLED_ShowCHinese(72+16,6,6);
	 }
			memset(temp_data,0x00,16);
			sprintf(temp_data,"%.2f",temp);
			
			OLED_ShowString(40,3,(u8*)temp_data);
			printf("%.2f",temp);
			gpstime=0;
			OneNet_SendData();									//发送数据
			M6312_Clear();
	  	dataPtr = M6312_GetIPD(0);
		  if(dataPtr != NULL)
			OneNet_RevPro(dataPtr);	
		}
 }
}

void OLED_Show()
{
	OLED_ShowCHinese(0,0,12);
	OLED_ShowCHinese(16,0,13);
	OLED_ShowString(32,0,(u8*)":");
  OLED_ShowCHinese(64,0,14);
	OLED_ShowCHinese(64+16,0,15);
	OLED_ShowString(80+16,0,(u8*)":");
	
	OLED_ShowCHinese(0,3,7);
	OLED_ShowCHinese(16,3,8);
	OLED_ShowCHinese(80,3,11);
	 OLED_ShowString(32,3,(u8*)":");
	
  OLED_ShowCHinese(0,6,3);
	OLED_ShowCHinese(16,6,4);
	OLED_ShowCHinese(32,6,9);
	OLED_ShowCHinese(48,6,10);
  OLED_ShowString(64,6,(u8*)":");
}





