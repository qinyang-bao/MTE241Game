//#define DEBUG

#include <cmsis_os2.h>
#include <lpc17xx.h>
#include <stdlib.h>
#include <stdio.h>
#include "lfsr113.h"
#include "GLCD.h"
#include "game_object.h"

#ifdef DEBUG
#include "uart.h"
#endif

#define true 1
#define false 0

#define GAME_START 1
#define GAME_END 1
#define BUTTON_PRESSED 1<<1
#define LIFE_LOST 1<<2

#define BUTTON_PRESS_MAX_RATE 4 // number of button press allowed per second
#define ADC_MAX 4096.
#define adc_to_player_pos(x) (int)(x * (LCD_HEIGHT - PLAYER_HEIGHT) / ADC_MAX ) 
	
#define MAX_LIFE 8

OBJECT_LIST_T* geese_list, * bullet_list;
_GAME_OBJECT_T* player;
int32_t lives_remaining, highest_score = 0, current_score;

osEventFlagsId_t game_states, game_start;
osMutexId_t lcd_mutex;

void init_game(void){
	player = create_player(PLAYER_START_X, PLAYER_START_Y);
	geese_list = create_game_object_list();
	bullet_list = create_game_object_list();
	
	lives_remaining = MAX_LIFE;
	current_score = 0;
	
	// use mutex to protect the LCD resource
	osMutexAcquire(lcd_mutex, osWaitForever);
	GLCD_Clear(White);
	osMutexRelease(lcd_mutex);
}

volatile int ADC_val = 0;
int button_press_ready = true;

void input_reader(void * arg){
	int reading_rate = 25;
	
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
		int game_flags = osEventFlagsGet(game_states);
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
		int game_flags = osEventFlagsGet(game_states);
		if( (game_flags & BUTTON_PRESSED) && !(game_flags & GAME_END)){
			osEventFlagsClear(game_states, BUTTON_PRESSED);
			add_object(bullet_list, create_bullet(player->x_pos, player->y_pos));
		}
		
		osDelay(osKernelGetTickFreq()/ rate);
	}
}

int random_range(int min, int max){
	return min + lfsr113()%(max-min);
}

void goose_spawner(void* arg){
	
	while(true){
		if(!(osEventFlagsGet(game_states) & GAME_END)){
			int32_t goose_pos_y = random_range(GOOSE_MIN_SPAWN_HEIGHT, GOOSE_MAX_SPAWN_HEIGHT);
			int32_t goose_pos_x = LCD_WIDTH - GOOSE_WIDTH;
			int32_t goose_vel_x = -1 * random_range(GOOSE_MIN_X_VEL, GOOSE_MAX_X_VEL);
			int32_t goose_vel_y = (lfsr113() % 2 == 0 ? 1 : -1) * random_range(GOOSE_MIN_Y_VEL, GOOSE_MAX_Y_VEL);
			
			add_object(geese_list, create_goose(goose_pos_x, goose_pos_y, goose_vel_x, goose_vel_y));
		}

		osDelay(random_range(GOOSE_MIN_SPAWN_INTERVAL, GOOSE_MAX_SPAWN_INTERVAL));// / (current_score / 250 + 1) * 1.5 );
		
	}
}

void LCD_updater(void* arg){
	int fps = 20;
	
	GAME_OBJECT_T* currentBullet;
	GAME_OBJECT_T* currentGoose;
	GAME_OBJECT_T* nextBullet;
	GAME_OBJECT_T* nextGoose;
	
	while(true){
		if(!(osEventFlagsGet(game_states) & GAME_END)){
			
			/* update */
			// update player position. The player's position is directly correlated to the
			// position of the potentiometer. It is not updated using a velocity. Here we
			// record its "sudo" velocity to be used later for drawing and clearing
			int player_new_y_pos = adc_to_player_pos(ADC_val);
			player->y_vel = player_new_y_pos - player->y_pos;
			player->y_pos = player_new_y_pos;
			
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
									clear(currentGoose->self);
									clear(currentBullet->self);
									
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
			
			/* draw new positions */
			// use mutex to protect the LCD resource
			osMutexAcquire(lcd_mutex, osWaitForever);
			
			draw_list(bullet_list);
			draw_list(geese_list);
			
			clear(player);
			draw(player);
			
			osMutexRelease(lcd_mutex);
		}
		
		osDelay(osKernelGetTickFreq() / fps);
	}
}


void num_to_led(int num){
	//Turn off LEDs	
	LPC_GPIO1->FIOCLR |= 0XB0000000; 
	LPC_GPIO2->FIOCLR |= 0x0000007C;
	
	
	int bin = 0;
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
	//update highest score if needed
	if(current_score > highest_score){
		highest_score = current_score;
	}
	
	// use mutex to protect the LCD resource
	osMutexAcquire(lcd_mutex, osWaitForever);
	
	//clear the LCD
	GLCD_Clear(White);
	
	GLCD_DisplayString(1,0,1,  (unsigned char*)"Game End!");
	
	char* score_string = (char*)malloc(50 * sizeof(char));
	sprintf(score_string, "score: %d, highest score: %d", current_score, highest_score);
	GLCD_DisplayString(9,0,0,  (unsigned char*)score_string);
	
	GLCD_DisplayString(11,0,0,  (unsigned char*)"Press pushbutton to start again");
	
	osMutexRelease(lcd_mutex);
	
	//remove all game object and clear the screen
	free(player);
	empty_list(bullet_list);
	empty_list(geese_list);
}


void state_monitor(void* arg){
	int rate = 10;
	
	while(true){
		int game_flags = osEventFlagsGet(game_states);
		
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
			
			//update score
			current_score += 1;
		} 
		
		osDelay(osKernelGetTickFreq() / rate);
	}
	
}

void init_peripherals(void){
	// init uart
	#ifdef DEBUG
	printf("Init\n");
	#endif 
	
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
	
	GLCD_DisplayString(1,0,0,  (unsigned char*)"Welcome, warrior!");
	GLCD_DisplayString(3,0,0,  (unsigned char*)"Use potentiometer to move up and down");
	GLCD_DisplayString(5,0,0,  (unsigned char*)"Use pushbutton to shoot");
	GLCD_DisplayString(7,0,0,  (unsigned char*)"Press push button to start!");
	
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
