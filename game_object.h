#ifndef __GAME_OBJECTS_H
#define __GAME_OBJECTS_H

#define LCD_HEIGHT  240
#define LCD_WIDTH 320

#define PLAYER_START_X 0
#define PLAYER_START_Y 120 
#define PLAYER_WIDTH 8
#define PLAYER_HEIGHT 8 

#define BULLET_VEL_X 12
#define BULLET_VEL_Y 0
#define BULLET_WIDTH 8
#define BULLET_HEIGHT 4 

#define GOOSE_MIN_X_VEL 3
#define GOOSE_MAX_X_VEL 6
#define GOOSE_MIN_Y_VEL 3
#define GOOSE_MAX_Y_VEL 6
#define GOOSE_HEIGHT 20
#define GOOSE_WIDTH 20

#define GOOSE_MIN_SPAWN_HEIGHT 40
#define GOOSE_MAX_SPAWN_HEIGHT 200
#define GOOSE_MIN_SPAWN_INTERVAL 1200
#define GOOSE_MAX_SPAWN_INTERVAL 1400

typedef struct{
	 int x_pos, y_pos, width, height, x_vel, y_vel;
	 unsigned short* object_bitmap;
}_GAME_OBJECT_T;

typedef struct node{
	_GAME_OBJECT_T* self;
	struct node* next;
	struct node* prev;
}GAME_OBJECT_T;

typedef struct{
	GAME_OBJECT_T* head;
	GAME_OBJECT_T* tail;
	int size;
}OBJECT_LIST_T;


_GAME_OBJECT_T* create_player(int x_pos, int y_pos);
_GAME_OBJECT_T* create_bullet(int x_pos, int y_pos);
_GAME_OBJECT_T* create_goose(int x_pos, int y_pos, int x_vel, int y_vel);

void clear(_GAME_OBJECT_T* obj);
void draw(_GAME_OBJECT_T* obj);

OBJECT_LIST_T* create_game_object_list(void);
void add_object(OBJECT_LIST_T* list, _GAME_OBJECT_T* object);
int remove_object(OBJECT_LIST_T* list, GAME_OBJECT_T* object);
void empty_list(OBJECT_LIST_T* list);
void draw_list(OBJECT_LIST_T* list);

int collide(_GAME_OBJECT_T* obj1, _GAME_OBJECT_T* obj2);

#endif
