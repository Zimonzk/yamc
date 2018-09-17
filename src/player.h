#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

#include <SDL2/SDL.h>

void move_player(Uint32 difftime);

void get_player_pos(float* out);
void get_player_ori(float* out);
void get_player_right(float* out);

void player_turn(SDL_MouseMotionEvent* event);

#endif // PLAYER_H_INCLUDED
