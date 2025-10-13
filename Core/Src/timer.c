#include "timer.h"

uint8_t timer_flag[MAX_TIMER]={0};
int timer_counter[5]={0};

void Timer_Init(void){
	HAL_TIM_Base_Start_IT(&HANDLE_SW_TIMER);
}

void Timer_Set(int index, int counter){
	timer_flag[index]=0;
	timer_counter[index] = counter/10;
}

void Timer_Run(){
	for(int i=0;i<MAX_TIMER;i++){
		if(timer_counter[i]>0){
			timer_counter[i]--;
			if(timer_counter[i]<=0){
				 timer_flag[i]=1;
			}
		}
	}
}

