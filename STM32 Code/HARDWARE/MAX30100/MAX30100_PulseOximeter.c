#include "MAX30100_PulseOximeter.h"
#include "timer3.h"
#include "stdlib.h"
#include "uart.h"
#include "myiic.h"
#include "delay.h"
float onBeatDetected;

PulseOximeterState state;
PulseOximeterDebuggingMode debuggingMode;
uint32_t tsFirstBeatDetected;
uint32_t tsLastBeatDetected;
uint32_t tsLastSample;
uint32_t tsLastBiasCheck;
uint32_t tsLastCurrentAdjustment;
uint8_t redLedPower;

DCRemover irDCRemover;
DCRemover redDCRemover;

u8  ID=0;
u8  TempINT=0;
u8  TempFRAC=0;

void SPO2_Init()
{
	ID=Read_One_Byte(MAX30100_REG_PART_ID);//??ID
	if(ID==17)
	{
		delay_ms(100);
		Write_One_Byte(MAX30100_REG_MODE_CONFIGURATION, MAX30100_MC_TEMP_EN);//??????
		TempINT=Read_One_Byte(MAX30100_REG_TEMPERATURE_DATA_INT);						 //??????
		TempFRAC=Read_One_Byte(MAX30100_REG_TEMPERATURE_DATA_FRAC);				   //??????
		delay_ms(300);
	setMode(MAX30100_MODE_SPO2_HR);																//SPO2 HR??
	setLedsPulseWidth(MAX30100_SPC_PW_1600US_16BITS);							//16bit AD
	setSamplingRate(MAX30100_SAMPRATE_100HZ);											//???? 100HZ
	setLedsCurrent( MAX30100_LED_CURR_24MA,MAX30100_LED_CURR_24MA);//24MA 24MA(??)
	setHighresModeEnabled(1);//????????
	BeatDetector();
	setOnBeatDetectedCallback(&onBeatDetected);
	begin(PULSEOXIMETER_DEBUGGINGMODE_PULSEDETECT);								//????
	}
		
}


void DisplayChar(unsigned char c)
{
	
	while((USART1->SR&0X40)==0);//等待上一次发送完毕   
	USART1->DR=c;
} 

void DisplayCurve(float ch0,float ch1)
{
	signed short temp; 
	DisplayChar(0x03);
	DisplayChar(0xfc);
	
	temp=(signed short)ch0;	
	DisplayChar(temp&0x00ff);
	DisplayChar(temp>>8);
	temp=(signed short)ch1;
	DisplayChar(temp&0x00ff);
	DisplayChar(temp>>8);


	DisplayChar(0xfc);
	DisplayChar(0x03);
}


void PulseOximeter()
{ 
    state=PULSEOXIMETER_STATE_INIT;
    tsFirstBeatDetected=0;
    tsLastBeatDetected=0;
    tsLastSample=0;
    tsLastBiasCheck=0;
    tsLastCurrentAdjustment=0;
    redLedPower=((uint8_t)RED_LED_CURRENT_START);
    onBeatDetected=NULL;
}

void begin(PulseOximeterDebuggingMode mode)
{
    debuggingMode = mode;
	
		PulseOximeter();
		//消除Ir  Red 直流分量
		DCRemover1(DC_REMOVER_ALPHA,&irDCRemover.alpha,&irDCRemover.dcw);
		DCRemover1(DC_REMOVER_ALPHA,&redDCRemover.alpha,&redDCRemover.dcw);
	
    state = PULSEOXIMETER_STATE_IDLE;//闲置状态
}

void POupdate()
{
    checkSample();
   // checkCurrentBias();
}

float getHeartRate()
{
    return getRate();
}

uint8_t POgetSpO2()
{
    return getSpO2();
}

uint8_t getRedLedCurrentBias()
{
    return redLedPower;
}

void setOnBeatDetectedCallback(float *cb)
{
    onBeatDetected = *cb;
}

extern signed short HeartRate_val; //心跳速率
extern u8 SPO2_val;							//血氧浓度

void checkSample()
{
	
	      u8 beatDetected;
	      float filteredPulseValue;
	      float irACValue;
	      float redACValue;
				signed short HeartRate=0;
				u8  SPO2=0;
			
			if (millis() - tsLastSample > 1.0 / SAMPLING_FREQUENCY * 1000.0) 
			{
        tsLastSample = millis();
			  update();
				
				irACValue = step(rawIRValue,&irDCRemover.alpha,&irDCRemover.dcw);
			  redACValue = step(rawRedValue,&redDCRemover.alpha,&redDCRemover.dcw);
				
				filteredPulseValue = FBstep(-irACValue);		//红外
			
			  beatDetected = addSample(filteredPulseValue);
				
				
        if (getRate() > 0) 
				{
						
            state = PULSEOXIMETER_STATE_DETECTING;
						SPO2update(irACValue, redACValue, beatDetected);
					
						HeartRate=getRate();
						SPO2=getSpO2();

				}
				else if (state == PULSEOXIMETER_STATE_DETECTING) 
				{
            state = PULSEOXIMETER_STATE_IDLE;
					  reset();
        }

        switch (debuggingMode) 
					{
            case PULSEOXIMETER_DEBUGGINGMODE_RAW_VALUES:

//						    printf("R:");
//                printf("%d",rawIRValue);
//                printf(",");
//                printf("%d\r\n",rawRedValue);
						
								DisplayCurve(rawIRValue,rawRedValue);
                break;

            case PULSEOXIMETER_DEBUGGINGMODE_AC_VALUES:

//						    printf("R:");
//                printf("%lf",irACValue);
//                printf(",");
//                printf("%lf\r\n",redACValue);
						
//								mpu6050_send_data(irACValue,redACValue,0,0,0,0);
										DisplayCurve(irACValue,redACValue);
                break;

            case PULSEOXIMETER_DEBUGGINGMODE_PULSEDETECT:

//						    printf("R:");
//                printf("%lf",filteredPulseValue);
//                printf(",");
//                printf("%lf\r\n",getCurrentThreshold());
						
//								mpu6050_send_data(filteredPulseValue,0,0,0,0,0);
									//DisplayCurve(filteredPulseValue,HeartRate*30);
									//LCD_ShowNum(130,140,HeartRate,2,16);
									//LCD_ShowNum(130,160,SPO2,2,16);
						HeartRate_val=HeartRate;
						SPO2_val=SPO2;
//						printf("\r\n心率：%d\r\n",HeartRate);
//						printf("\r\n血氧：%d\r\n",SPO2);
                break;

            default:
                break;
        }

        if (beatDetected && onBeatDetected) 
				{
            onBeatDetected1();
        }
    }
}

void onBeatDetected1(void)
{
  
}
void checkCurrentBias()
{

    if (millis() - tsLastBiasCheck > CURRENT_ADJUSTMENT_PERIOD_MS) 
			{
				
			  u8 changed = 0;
				
        if (getDCW(&irDCRemover.dcw)-getDCW(&redDCRemover.dcw) > 70000 && redLedPower < MAX30100_LED_CURR_50MA) 
				{
            ++redLedPower;
					changed = 1;
        } 
				else if (getDCW(&redDCRemover.dcw) - getDCW(&irDCRemover.dcw) > 70000 && redLedPower > 0) 
				{
            --redLedPower;
					changed = 1;
        }
			
        if (changed) 
				{
						
				   	setLedsCurrent(IR_LED_CURRENT, (LEDCurrent)redLedPower);
            tsLastCurrentAdjustment = millis();
        }

        tsLastBiasCheck = millis();
    }
}



