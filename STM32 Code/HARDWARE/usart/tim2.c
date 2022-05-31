#include "tim2.h"

void NVIC_config(void)
{
     NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);//优先级分组
     
     NVIC_InitTypeDef nvic;
     nvic.NVIC_IRQChannel=TIM2_IRQn;
     nvic.NVIC_IRQChannelCmd=ENABLE;
      nvic.NVIC_IRQChannelPreemptionPriority=0;
     nvic.NVIC_IRQChannelSubPriority=0;
     NVIC_Init(&nvic);
}

void time2_config(void)
{
     TIM_TimeBaseInitTypeDef tim;//结构体
     NVIC_config();//优先级函数调用
     RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);//开启时钟
     TIM_DeInit(TIM2);//？？？？？
     tim.TIM_ClockDivision=TIM_CKD_DIV1;//采样分频
    tim.TIM_CounterMode=TIM_CounterMode_Up;//向上计数
     tim.TIM_Period=4999;//自动重装载寄存器的值
     tim.TIM_Prescaler=7199;//时钟预分频
     //tim.TIM_RepetitionCounter=
     TIM_TimeBaseInit(TIM2,&tim);//初始化结构体
     TIM_ClearFlag(TIM2,TIM_FLAG_Update);//清除溢出中断标志
     TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
     TIM_Cmd(TIM2,ENABLE);//开启时钟
 }

void TIM2_IRQHandler(void)
{
     if(TIM_GetITStatus(TIM2,TIM_IT_Update)!=RESET)//判断中断标志是否发生
     {
			  gpstime++;
			  HeartRatetime++;
     }
     TIM_ClearITPendingBit(TIM2,TIM_FLAG_Update);    
}
