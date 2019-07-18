#include <cmsis_os2.h>
#include <lpc17xx.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "uart.h"
#include "lfsr113.h"
#include "GLCD.h"
#include "game_object.h"

#define GAME_START 1
#define GAME_END 1
#define BUTTON_PRESSED 1<<1
#define LIFE_LOST 1<<2

#define BUTTON_PRESS_MAX_RATE 4 // number of button press allowed per second
#define ADC_MAX 4096.
#define adc_to_player_pos(x) (int)(x * (LCD_HEIGHT - PLAYER_HEIGHT) / ADC_MAX ) 
	
#define MAX_LIFE 1

OBJECT_LIST_T* geese_list, * bullet_list;
_GAME_OBJECT_T* player;
int32_t lives_remaining;

osEventFlagsId_t game_states, game_start;
osMutexId_t lcd_mutex;

void init_game(void){
	player = create_player(PLAYER_START_X, PLAYER_START_Y);
	geese_list = create_game_object_list();
	bullet_list = create_game_object_list();
	
	lives_remaining = MAX_LIFE;
}

volatile int ADC_val = 0;
bool button_press_ready = true;

void input_reader(void * arg){
	int reading_rate = 100;
	
	// Setup ADC
	LPC_PINCON -> PINSEL1 &= ~(0x03 << 18);
	LPC_PINCON -> PINSEL1 |= (0x01 << 18);
	
	LPC_SC->PCONP |= (1 << 12);
	
	LPC_ADC->ADCR = (1 << 2) | (4 << 8) | ( 0 << 24 ) | (1 << 21);
	
	//Enable interrupt for ADC
	LPC_ADC->ADINTEN = ( 1 <<  8); 
	NVIC_EnableIRQ( ADC_IRQn );
	
	//Setup push button
	LPC_PINCON->PINSEL4 &= ~( 3<<20 );
	LPC_GPIO2->FIODIR &= ~( 1<<10 );
	
	//Enable interrupt on the falling edge 
	LPC_GPIOINT->IO2IntEnF |=  ( 1 << 10 );
	NVIC_EnableIRQ(EINT3_IRQn);
	
	int loop_count = 0;
	while (true){
		// start ADC conversion
		LPC_ADC->ADCR |= ( 1 << 24 );	
		
		loop_count ++;
		// this limits how fast the button can be pressed, and also acts as a debounce for the push button
		if (loop_count % (reading_rate / BUTTON_PRESS_MAX_RATE) == 0){
			button_press_ready = true;
		}
		
		// if game ends, a button press would start the game
		uint32_t game_flags = osEventFlagsGet(game_states);
		if( (game_flags & BUTTON_PRESSED) && (game_flags & GAME_END) ){
			osEventFlagsClear(game_states, GAME_END);
			osEventFlagsClear(game_states, BUTTON_PRESSED);
			init_game();
		}
		
		osDelay(osKernelGetTickFreq() / reading_rate);
	}

}

void ADC_IRQHandler(void) {
	ADC_val = (LPC_ADC->ADGDR >> 4) & 0xFFF;
	// Read ADC Status clears the interrupt condition
	int ADC_stat = LPC_ADC->ADSTAT;
}

void EINT3_IRQHandler(void){
	if (button_press_ready){
		button_press_ready = false;
		osEventFlagsSet(game_states, BUTTON_PRESSED);
	}
	//clear interrupt
	LPC_GPIOINT->IO2IntClr |=  (1 << 10);
}


void bullet_spawner(void* arg){
	int rate = 100;
	
	while(true){
		uint32_t game_flags = osEventFlagsGet(game_states);
		if( (game_flags & BUTTON_PRESSED) && !(game_flags & GAME_END)){
			osEventFlagsClear(game_states, BUTTON_PRESSED);
			add_object(bullet_list, create_bullet(player->x_pos, player->y_pos));
			//printf("bullet spawned\n");
		}
		
		osDelay(osKernelGetTickFreq()/ rate);
	}
}

uint32_t random_range(uint32_t min, uint32_t max){
	return min + lfsr113()%(max-min);
}

void goose_spawner(void* arg){
	
	while(true){
		if(!(osEventFlagsGet(game_states) & GAME_END)){
			int32_t goose_pos_y = random_range(GOOSE_MIN_SPAWN_HEIGHT, GOOSE_MAX_SPAWN_HEIGHT);
			int32_t goose_pos_x = LCD_WIDTH - GOOSE_WIDTH;
			int32_t goose_vel_x = -1 * random_range(GOOSE_MIN_X_VEL, GOOSE_MAX_X_VEL);
			int32_t goose_vel_y = (lfsr113() % 2 == 0 ? 1 : -1) * random_range(GOOSE_MIN_Y_VEL, GOOSE_MAX_Y_VEL);
			
			//printf("goose spawned y: %d, vx: %d vy: %d\n", goose_pos_y, goose_vel_x, goose_vel_y);
			add_object(geese_list, create_goose(goose_pos_x, goose_pos_y, goose_vel_x, goose_vel_y));
		}
		osDelay(random_range(GOOSE_MIN_SPAWN_INTERVAL, GOOSE_MAX_SPAWN_INTERVAL));
		
	}
}

void LCD_updater(void* arg){
	int fps = 10;
	
	GAME_OBJECT_T* currentBullet;
	GAME_OBJECT_T* currentGoose;
	GAME_OBJECT_T* nextBullet;
	GAME_OBJECT_T* nextGoose;
	
	while(true){
		if(!(osEventFlagsGet(game_states) & GAME_END)){
			
			// update player position
			player->y_pos = adc_to_player_pos(ADC_val);
			
			// update bullet position
			currentBullet = bullet_list ->head ;
			if (currentBullet != NULL){
				do {
						nextBullet = currentBullet->next;
						currentBullet->self->x_pos += currentBullet->self->x_vel;
						//Check if bullet moves past right side
						if(currentBullet->self->x_pos - BULLET_WIDTH >= LCD_WIDTH)
						{
							remove_object(bullet_list, currentBullet);
						}
				
					currentBullet = nextBullet;
					}while(currentBullet != NULL);
				}
			
				currentBullet = bullet_list ->head ;
				currentGoose = geese_list ->head ;
				
			if (currentGoose != NULL){
				do {
					//printf("gx: %d, gy: %d \n", currentGoose->self->x_pos, currentGoose->self->y_pos);
					nextGoose = currentGoose->next;
					
					// geese bounce off the edges
					if (currentGoose->self->y_pos <= 0 || currentGoose->self->y_pos >= LCD_HEIGHT)
					{
						currentGoose->self->y_vel = -currentGoose->self->y_vel;
					}
					
					// update geese position
					currentGoose->self->x_pos += currentGoose->self->x_vel;
					currentGoose->self->y_pos += currentGoose->self->y_vel;
					
					//Check if goose flies past left side
					if(currentGoose->self->x_pos + GOOSE_WIDTH <= 0)
					{
						remove_object(geese_list, currentGoose);
						osEventFlagsSet(game_states, LIFE_LOST);
					}
					
					else{
						//Check if goose hit by any bullet
						if(currentBullet != NULL){
							do {
								nextBullet = currentBullet->next;
								if (collide(currentGoose->self, currentBullet->self))
								{
									GLCD_Bitmap(currentGoose->self->x_pos-currentGoose->self->x_vel, 
											currentGoose->self->y_pos-currentGoose->self->y_vel, 
											currentGoose->self->width, 
											currentGoose->self->height, 
											(unsigned char*)currentGoose->self->clear_box_bitmap);
									
									GLCD_Bitmap(currentBullet->self->x_pos-currentBullet->self->width,
											currentBullet->self->y_pos, 
											currentBullet->self->x_vel, 
											currentBullet->self->height, 
											(unsigned char*)currentBullet->self->clear_box_bitmap);
									
									remove_object(geese_list, currentGoose);
									remove_object(bullet_list, currentBullet);
									
									//clear the position of the goose and bullet
									
								}
								currentBullet = nextBullet;
							}while (currentBullet != NULL);
						}
					}
				currentGoose = nextGoose;
				}while(currentGoose != NULL);
			}
			
			// use mutex to protect the LCD resource
			osMutexAcquire(lcd_mutex, osWaitForever);
			
			currentBullet = bullet_list ->head ;
			currentGoose = geese_list ->head ;

			//Draw Geese on screen
			if (currentGoose != NULL){
				do{
					GLCD_Bitmap(currentGoose->self->x_pos-currentGoose->self->x_vel, 
											currentGoose->self->y_pos-currentGoose->self->y_vel, 
											currentGoose->self->width, 
											currentGoose->self->height, 
											(unsigned char*)currentGoose->self->clear_box_bitmap);
					
					GLCD_Bitmap(currentGoose->self->x_pos,currentGoose->self->y_pos, 
											currentGoose->self->width, currentGoose->self->height, 
											(unsigned char*)currentGoose->self->object_bitmap);
					
					currentGoose = currentGoose->next;
				}while(currentGoose != NULL);
			}

			//Draw Bullets on screen
			if (currentBullet != NULL){
				do{
					GLCD_Bitmap(currentBullet->self->x_pos-currentBullet->self->width,
											currentBullet->self->y_pos, 
											currentBullet->self->x_vel, 
											currentBullet->self->height, 
											(unsigned char*)currentBullet->self->clear_box_bitmap);
					
					GLCD_Bitmap(currentBullet->self->x_pos,currentBullet->self->y_pos, 
											currentBullet->self->width, currentBullet->self->height, 
											(unsigned char*)currentBullet->self->object_bitmap);
					
					currentBullet = currentBullet->next;
				}while(currentBullet != NULL);
			}

			//Draw Player on screen
			GLCD_Bitmap(0, 0, player->width, LCD_HEIGHT, (unsigned char*)player->clear_box_bitmap);
			GLCD_Bitmap(player->x_pos, player->y_pos, player->width, player->height, (unsigned char*)player->object_bitmap);
		}
		
		osMutexRelease(lcd_mutex);
		
		osDelay(osKernelGetTickFreq() / fps);
	}
}


void num_to_led(uint32_t num){
	//Turn off LEDs	
	LPC_GPIO1->FIOCLR |= 0XB0000000; 
	LPC_GPIO2->FIOCLR |= 0x0000007C;
	
	
	uint32_t bin = 0;
	while (num > 0){
		bin = bin << 1;
		bin |= 1;
		num --;
	}
	
	//Set LEDs
	LPC_GPIO1->FIOSET |=  ((bin & (1)) << 28);
	LPC_GPIO1->FIOSET |=  ((bin & (1<<1)) << 28);
	LPC_GPIO1->FIOSET |=  ((bin & (1U<<2)) << 29);
	
	LPC_GPIO2->FIOSET |= ((bin >> 3) << 2);
}


void game_end(void){
	// use mutex to protect the LCD resource
	osMutexAcquire(lcd_mutex, osWaitForever);
	GLCD_Clear(White);
	osMutexRelease(lcd_mutex);
	
	//remove all game object and clear the screen
	free(player);
	
	GAME_OBJECT_T* currentBullet = bullet_list ->head ;
	if (currentBullet != NULL){
		do {
			remove_object(bullet_list, currentBullet);
			currentBullet = currentBullet->next;
			} while(currentBullet != NULL);
	}
	
	GAME_OBJECT_T* currentGoose = geese_list ->head ;
	if (currentGoose != NULL){
		do {
			remove_object(geese_list, currentGoose);
			currentGoose = currentGoose->next;
			} while(currentGoose != NULL);
	}
}


void state_monitor(void* arg){
	int rate = 100;
	
	while(true){
		uint32_t game_flags = osEventFlagsGet(game_states);
		
		// if game is not ended, display the remaining lives as LEDs
		if (!(game_flags & GAME_END)){
			if(game_flags & LIFE_LOST){
				osEventFlagsClear(game_states, LIFE_LOST);
				lives_remaining --;
			}

			if (lives_remaining >= 0){
				num_to_led(lives_remaining);
			}
			if(lives_remaining == 0) {
				// if game ends, display the final score and prompt for button press to restart
				game_end();
				osEventFlagsSet(game_states, GAME_END);
			}
		} 
		
		osDelay(osKernelGetTickFreq() / rate);
	}
	
}

void init_peripherals(void){
	// init uart
	printf("Init\n");
	
	//setup LCD
	GLCD_Init();
	GLCD_Clear(White);
	GLCD_SetBackColor(White);
	GLCD_SetTextColor(Black);
	
	//setup LED
	LPC_GPIO1->FIODIR |= 0XB0000000; 
	LPC_GPIO2->FIODIR |= 0x0000007C;
}



void welcome(void){
	GLCD_DisplayString(1,0,1,  (unsigned char*)"     q1     q2");
}

int main(){
	init_peripherals();
	
	welcome();
	
	// initialize the kernel
	osKernelInitialize();
		
	game_states = osEventFlagsNew(NULL);
	game_start = osEventFlagsNew(NULL);
	osEventFlagsSet(game_states, GAME_END); // set the game to be ended initially
	osEventFlagsClear(game_start, GAME_START);
	
	lcd_mutex = osMutexNew(NULL);
	
	osThreadNew(input_reader, NULL, NULL);
	osThreadNew(bullet_spawner, NULL, NULL);
	osThreadNew(goose_spawner, NULL, NULL);
	osThreadNew(LCD_updater, NULL, NULL);
	osThreadNew(state_monitor, NULL, NULL);
	
	osKernelStart();
	
	for (;;){}
}