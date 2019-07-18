#include "uart.h"
#include <lpc17xx.h>
#include "cmsis_os.h"
#include <stdio.h>
#include "GLCD.h"
#include <stdbool.h>

int ADC_Value = 0;
bool gameRunning = true;



// Question 1 
/*
void DisplayBinary(unsigned char Num){
	//Turn off LEDs	
	LPC_GPIO1->FIOCLR |= 0XB0000000; 
	LPC_GPIO2->FIOCLR |= 0x0000007C;
	
	
	LPC_GPIO1->FIOSET |=  ((Num & (1)) << 28);
	LPC_GPIO1->FIOSET |=  ((Num & (1<<1)) << 28);
	LPC_GPIO1->FIOSET |=  ((Num & (1U<<2)) << 29);
	
	LPC_GPIO2->FIOSET |= ((Num >> 3) << 2);
}

int main( void ) 
{
	//Initialize LEDs for output
	LPC_GPIO1->FIODIR |= 0XB0000000; 
	LPC_GPIO2->FIODIR |= 0x0000007C;
	DisplayBinary(010);
	
	int count = 0;
	while(1){
		DisplayBinary(count);
		count ++;
		osDelay(1000);
	}

}
*/

/*
// Question 2
int main( void ) 
{
	LPC_GPIO1->FIODIR |= ~(1<<20 | 1<<23 | 1<<24 | 1<<25 | 1<<26); 
	
	while(1){
		int up = LPC_GPIO1 -> FIOPIN & (1<<24);
		int down = LPC_GPIO1 -> FIOPIN & (1<<26);
		int left = LPC_GPIO1 -> FIOPIN & (1<<23);
		int right = LPC_GPIO1 -> FIOPIN & (1<<25);
		
		int pressed = (LPC_GPIO1 -> FIOPIN & (1<<20));
		
		if (!up)
				printf("UP, ");
		else if (!down)
				printf("DOWN, ");
		else if (!left)
				printf("LEFT, ");
		else if (!right)
				printf("RIGHT, ");
		
		if (!pressed)
			printf("pressed \n");
		else 
				printf("not pressed \n");
}
}
*/


/*
// Question 3
int main(){
	LPC_PINCON -> PINSEL1 &= ~(0x03 << 18);
	LPC_PINCON -> PINSEL1 |= (0x01 << 18);
	
	LPC_SC->PCONP |= (1 << 12);
	
	LPC_ADC->ADCR = (1 << 2) | (4 << 8) | (1 << 21);
	
	while(1){
		LPC_ADC->ADCR |= (1 << 24);
		while(!LPC_ADC->ADGDR & (1U << 31));
		int reading = ((LPC_ADC->ADGDR >> 4) & 0xFFF);
		printf("the potential reading is %d \n", reading);
		LPC_ADC->ADCR &= ~(1 << 24);
		
	}
}
*/

// Interrupt Potentiometer
int main(){
	LPC_PINCON -> PINSEL1 &= ~(0x03 << 18);
	LPC_PINCON -> PINSEL1 |= (0x01 << 18);
	
	LPC_SC->PCONP |= (1 << 12);
	
	LPC_ADC->ADCR = (1 << 2) | (4 << 8) | ( 0 << 24 ) | (1 << 21);
	
	//Enable interrupt for ADC
	LPC_ADC->ADINTEN = ( 1 <<  8); 
	
	NVIC_EnableIRQ( ADC_IRQn );
	
	

	while(gameRunning)
	{
		LPC_ADC->ADCR |= ( 1 << 24 );
		osDelay(10);
		printf("the potential reading is %d \n", ADC_Value);
	}

}

void ADC_IRQHandler( void ) {
	    // Read ADC Status clears the interrupt condition
	     ADCStat = LPC_ADC->ADSTAT;
	     ADC_Value = (LPC_ADC->ADGDR >> 4) & 0xFFF;
       }




/*
// Question 4
int main(){
	
	GLCD_Init();
	GLCD_SetBackColor(Blue);
	GLCD_Clear(Blue);
	GLCD_SetTextColor(White);
	GLCD_DisplayString(4, 164, 1, (unsigned char *)"Hello World");
}
*/

/*
//Question 5

void readPM(void const* arg){
	LPC_PINCON -> PINSEL1 &= ~(0x03 << 18);
	LPC_PINCON -> PINSEL1 |= (0x01 << 18);
	
	LPC_SC->PCONP |= (1 << 12);
	
	LPC_ADC->ADCR = (1 << 2) | (4 << 8) | (1 << 21);
	
	while(1){
		LPC_ADC->ADCR |= (1 << 24);
		while(!LPC_ADC->ADGDR & (1U << 31));
		int reading = ((LPC_ADC->ADGDR >> 4) & 0xFFF);
		printf("the potential reading is %d \n", reading);
		LPC_ADC->ADCR &= ~(1 << 24);
		osDelay(500);
	}
}

void readJOY(void const* arg){
	LPC_GPIO1->FIODIR |= ~(1<<20 | 1<<23 | 1<<24 | 1<<25 | 1<<26); 
	GLCD_Init();
	GLCD_SetBackColor(Blue);
	GLCD_SetTextColor(White);
	int previous = 0;
	
	while(1){

		int up = LPC_GPIO1 -> FIOPIN & (1<<24);
		int down = LPC_GPIO1 -> FIOPIN & (1<<26);
		int left = LPC_GPIO1 -> FIOPIN & (1<<23);
		int right = LPC_GPIO1 -> FIOPIN & (1<<25);
		
		int pressed = (LPC_GPIO1 -> FIOPIN & (1<<20));
		
		int current = up | down | left | right | pressed;
		if (previous != current)
			GLCD_Clear(Blue);
		
		previous = current;
		
		if (!up)
				GLCD_DisplayString(1, 0, 1, (unsigned char *)"UP");
		else if (!down)
				GLCD_DisplayString(1, 0, 1, (unsigned char *)"DOWN");
		else if (!left)
				GLCD_DisplayString(1, 0, 1, (unsigned char *)"LEFT");
		else if (!right)
				GLCD_DisplayString(1, 0, 1, (unsigned char *)"RIGHT");
		else
			GLCD_DisplayString(1, 0, 1, (unsigned char *)"NO DIRECTION");
		
		if (!pressed)
			GLCD_DisplayString(2, 0, 1, (unsigned char *)"Pressed");
		else 
				GLCD_DisplayString(2, 0, 1, (unsigned char *)"Not Pressed");
		
		osDelay(500);
}
}

void readPB(void const* arg){
	
	LPC_GPIO1->FIODIR |= ((1<<28)); 
	LPC_GPIO1->FIOSET &=  (~(1 << 28));
	
	LPC_GPIO2->FIODIR &= (~(1<<10)); 
	int isOn = 0;
	int isPressed = 0;
	
	while (1){
		if (!(LPC_GPIO2->FIOPIN & (1<<10)) && !isPressed){
			isPressed = 1;
		
			if (!isOn){
				LPC_GPIO1->FIOSET |=  (1 << 28);
				isOn = 1;
			}
		else {
				LPC_GPIO1->FIOCLR |=  ((1 << 28));
				isOn = 0;
			}
		}
		else if(LPC_GPIO2->FIOPIN & (1<<10)){
			isPressed = 0;
		}
		osDelay(100);
	}
	
}

osThreadDef(readPM, osPriorityNormal, 1, 0);
osThreadDef(readJOY, osPriorityNormal, 1, 0);
osThreadDef(readPB, osPriorityNormal, 1, 0);

int main(){
	printf("");
	osKernelInitialize();
	osKernelStart();
	osThreadCreate(osThread(readPM), NULL);
	osThreadCreate(osThread(readJOY), NULL);
	osThreadCreate(osThread(readPB), NULL);
}
*/
