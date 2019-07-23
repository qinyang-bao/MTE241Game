#include "game_object.h"
#include <stdlib.h>
#include "GLCD.h"

#define true 1
#define false 0

#define COLLID_TOL 6
#define CLEAR_BOX_SIZE 4

unsigned short player_map [] = 
{Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue, Blue};
unsigned short bullet_map [] = 
{Red, Red, Red, Red, Red, Red, Red, Red, Red, Red, Red, Red, Red, Red, Red, Red, Red, Red, Red, Red, Red, Red, Red, Red, Red, Red, Red, Red, Red, Red, Red, Red};
unsigned short goose_map [] = 
{White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, Black, Black, White, White, White, White, White, White, White, White, White, White, White, White, White, Black, Black, Black, Black, Black, Black, Black, Black, White, White, White, White, White, White, White, White, White, White, Black, Black, Black, Black, Black, Black, Black, White, White, White, White, White, White, White, White, White, White, White, Black, Black, Black, Black, Black, Black, Black, Black, White, White, White, White, White, White, White, White, White, White, White, Black, Black, Black, Black, Black, Black, Black, Black, Black, White, White, White, White, White, White, White, White, White, White, Black, Black, Black, Black, Black, Black, Black, Black, Black, Black, Black, White, White, White, White, Black, Black, Black, Black, Black, Black, Black, Black, Black, Black, Black, Black, Black, Black, Black, Black, Black, White, White, White, Black, Black, Black, Black, White, White, White, White, Black, Black, Black, Black, Black, Black, Black, Black, Black, White, White, White, White, White, White, White, White, White, White, White, Black, Black, Black, Black, Black, Black, Black, Black, Black, White, White, White, White, White, White, White, White, White, White, White, Black, Black, Black, Black, Black, Black, Black, Black, Black, Black, White, White, White, White, White, White, White, White, White, White, Black, Black, Black, Black, Black, Black, Black, Black, Black, Black, White, White, White, White, White, White, White, White, White, White, Black, Black, Black, Black, Black, Black, Black, Black, Black, Black, White, White, White, White, White, White, White, White, White, White, Black, Black, Black, Black, White, Black, Black, Black, Black, Black, White, White, White, White, White, White, White, White, White, White, Black, Black, Black, White, White, Black, Black, Black, Black, Black, White, White, White, White, White, White, White, White, White, White, Black, Black, White, White, White, White, Black, Black, Black, Black, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, Black, Black, Black, Black, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, Black, Black, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, Black, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White};
//unsigned short clear_map [] = {White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White};
unsigned short clear_map [] = 
{White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White, White};
 
_GAME_OBJECT_T* create_player(int x_pos, int y_pos){
	_GAME_OBJECT_T* player = malloc(sizeof(_GAME_OBJECT_T));
	player->x_pos = x_pos;
	player->y_pos = y_pos;
	player->x_vel = 0;
	player->y_vel = 0;
	player->object_bitmap = player_map;
	player->width = PLAYER_WIDTH;
	player->height = PLAYER_HEIGHT;
	return player;
}

_GAME_OBJECT_T* create_bullet(int x_pos, int y_pos){
	_GAME_OBJECT_T* bullet = malloc(sizeof(_GAME_OBJECT_T));
	bullet->x_pos = x_pos;
	bullet->y_pos = y_pos;
	bullet->x_vel = BULLET_VEL_X;
	bullet->y_vel = BULLET_VEL_Y;
	bullet->object_bitmap = bullet_map;
	bullet->width = BULLET_WIDTH;
	bullet->height = BULLET_HEIGHT;
	return bullet;
}

_GAME_OBJECT_T* create_goose(int x_pos, int y_pos, int x_vel, int y_vel){
	_GAME_OBJECT_T* goose = malloc(sizeof(_GAME_OBJECT_T));
	goose->x_pos = x_pos;
	goose->y_pos = y_pos;
	goose->x_vel = x_vel;
	goose->y_vel = y_vel;
	goose->object_bitmap = goose_map;
	goose->width = GOOSE_WIDTH;
	goose->height = GOOSE_HEIGHT;
	return goose;
}


OBJECT_LIST_T* create_game_object_list(void){
	OBJECT_LIST_T* object_list = malloc(sizeof(OBJECT_LIST_T));
	object_list->size = 0;
	object_list->head = NULL;
	object_list->tail = NULL;
	return object_list;
}

void clear(_GAME_OBJECT_T* obj){
	GLCD_Bitmap(obj->x_pos - obj->x_vel, obj->y_pos -  obj->y_vel, obj->width, obj->height, (unsigned char*)clear_map); 
	
	/*
	int x = obj->x_pos - obj->x_vel, y = obj->y_pos -  obj->y_vel, w = obj->width, h = obj->height;
	GLCD_SetTextColor(White);
	for (int i=0; i<w; i+=CLEAR_BOX_SIZE){
		for(int j=0; j<h; j+=CLEAR_BOX_SIZE){
			//GLCD_PutPixel(x+i, y+j);
			GLCD_Bitmap(x+i, y+j, CLEAR_BOX_SIZE, CLEAR_BOX_SIZE, (unsigned char*)clear_map); 
		}
	}
	GLCD_SetTextColor(Black);
	*/
}

void draw(_GAME_OBJECT_T* obj){
	GLCD_Bitmap(obj->x_pos, obj->y_pos, obj->width, obj->height, (unsigned char*)obj->object_bitmap);
}



void add_object(OBJECT_LIST_T* list, _GAME_OBJECT_T* object){
	
	GAME_OBJECT_T* obj = malloc(sizeof(GAME_OBJECT_T));
	obj->self = object;
	
	//if list is empty, add as head
	if (list->size == 0){
		obj->prev = NULL;
		obj->next = NULL;
		list->head = obj;
		list->tail = obj;
	}
	
	//else add as tail
	else {
		GAME_OBJECT_T* prev_tail = list->tail;
		prev_tail->next = obj;
		obj->prev = prev_tail;
		obj->next = NULL;
		list->tail = obj;
	}
	
	list->size += 1;
}

int remove_object(OBJECT_LIST_T* list, GAME_OBJECT_T* object){
	if (list->size == 0){
		return false;
	}
	
	if (list->head == object){
		list->head = object->next;
		
		if (list->head != NULL){
			list->head->prev = NULL; 
		}
		free(object->self);
		free(object);
		list->size -= 1;
		return true;
	}
	
	if (list->tail == object){
		list->tail = object->prev;
		
		if(list->tail != NULL){
			list->tail->next = NULL; 
		}
		free(object->self);
		free(object);
		list->size -= 1;
		return true;
	}

	GAME_OBJECT_T* prev_obj =  list->head;
	GAME_OBJECT_T* curr_obj =  prev_obj->next;

	while(curr_obj != object && curr_obj != NULL){
		prev_obj = curr_obj;
		curr_obj = curr_obj->next;
	}

	// list has been exhasuted, object is not in the list
	if (curr_obj == NULL){
		return false;
	}
	
	prev_obj->next = curr_obj->next;
	curr_obj->next->prev = prev_obj;
	free(object->self);
	free(curr_obj);
	
	list->size -= 1;
	return true;
}

void empty_list(OBJECT_LIST_T* list){
	GAME_OBJECT_T* current_obj = list ->head;
	if (current_obj != NULL){
		do {
			remove_object(list, current_obj);
			current_obj = current_obj->next;
			} while(current_obj != NULL);
	}
}

void draw_list(OBJECT_LIST_T* list){
	GAME_OBJECT_T* current_obj = list ->head;
	if (current_obj != NULL){
		do{
			clear(current_obj->self);
			draw(current_obj->self);
			current_obj = current_obj->next;
		}while(current_obj != NULL);
	}
}

/*
for two rectangles (A and B), any of these would guarantee there is no overlap:
Cond1. If A's left edge is to the right of the B's right edge, - then A is Totally to right Of B
Cond2. If A's right edge is to the left of the B's left edge, - then A is Totally to left Of B
Cond3. If A's top edge is below B's bottom edge, - then A is Totally below B
Cond4. If A's bottom edge is above B's top edge, - then A is Totally above B

for non-overlap:
Cond1 Or Cond2 Or Cond3 Or Cond4

for overlap:
Not Cond1 And Not Cond2 And Not Cond3 And Not Cond4
*/
int collide(_GAME_OBJECT_T* obj1, _GAME_OBJECT_T* obj2){
	int obj1_left = obj1->x_pos, obj1_right = obj1->x_pos + obj1->width, obj1_top = obj1->y_pos, obj1_bot = obj1->y_pos + obj1->height,
					obj2_left = obj2->x_pos, obj2_right = obj2->x_pos + obj2->width, obj2_top = obj2->y_pos, obj2_bot = obj2->y_pos + obj2->height;
	
																															// note that the LCD'y goes larger downwards, so we need to flip here
	return (obj1_left < obj2_right+COLLID_TOL && obj1_right > obj2_left-COLLID_TOL && obj1_top < obj2_bot+COLLID_TOL && obj1_bot > obj2_top-COLLID_TOL);
	
}
