#pragma once
#include <stdbool.h>

char* create_grid();
void  destroy_grid(char*);
void  draw_grid(char*);
void  init_grid(char*);
bool  is_alive(int, int, char*);
/*void  update_grid(char *src, char *dst); - old version*/
void  update_grid(char*, char*, int, int);
