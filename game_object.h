#ifndef __GAME_OBJECTS_H
#define __GAME_OBJECTS_H

#include <stdint.h>
#include <stdbool.h>

#define LCD_HEIGHT  240
#define LCD_WIDTH 320

#define PLAYER_START_X 0
#define PLAYER_START_Y 120 
#define PLAYER_WIDTH 8
#define PLAYER_HEIGHT 8 

#define BULLET_VEL_X 10
#define BULLET_VEL_Y 0
#define BULLET_WIDTH 10
#define BULLET_HEIGHT 5 

#define GOOSE_MIN_X_VEL 5
#define GOOSE_MAX_X_VEL 10
#define GOOSE_MIN_Y_VEL 5
#define GOOSE_MAX_Y_VEL 10
#define GOOSE_HEIGHT 25
#define GOOSE_WIDTH 25

#define GOOSE_MIN_SPAWN_HEIGHT 40
#define GOOSE_MAX_SPAWN_HEIGHT 200
#define GOOSE_MIN_SPAWN_INTERVAL 1000
#define GOOSE_MAX_SPAWN_INTERVAL 1500

typedef enum{
	PLAYER,
	BULLET,
	GOOSE,
}TYPE;

typedef struct{
	int32_t x_pos, y_pos, width, height, x_vel, y_vel;
	TYPE type;
	unsigned short* object_bitmap;
	unsigned short* clear_box_bitmap;
}_GAME_OBJECT_T;

typedef struct node{
	_GAME_OBJECT_T* self;
	struct node* next;
	struct node* prev;
}GAME_OBJECT_T;

typedef struct{
	GAME_OBJECT_T* head;
	GAME_OBJECT_T* tail;
	uint32_t size;
}OBJECT_LIST_T;


_GAME_OBJECT_T* create_player(int32_t x_pos, int32_t y_pos);
_GAME_OBJECT_T* create_bullet(int32_t x_pos, int32_t y_pos);
_GAME_OBJECT_T* create_goose(int32_t x_pos, int32_t y_pos, int32_t x_vel, int32_t y_vel);

OBJECT_LIST_T* create_game_object_list(void);
void add_object(OBJECT_LIST_T* list, _GAME_OBJECT_T* object);
bool remove_object(OBJECT_LIST_T* list, GAME_OBJECT_T* object);

bool collide(_GAME_OBJECT_T* obj1, _GAME_OBJECT_T* obj2);

#endif
