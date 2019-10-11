#define _USE_MATH_DEFINES

#include "plogger.h"
#include "player.h"
#include "input.h"
#include "chunk.h"
#include "longpos.h"

#include <SDL2/SDL.h>
#include <math.h>

#ifndef M_PI /* ensure that M_PI is defined as it is not required by the standard */
#define M_PI 3.14159265358979323846
#endif

static float player_pitch_rad = 0.0, player_yaw_rad = 0;
static float player_pitching_speed = 0.003, player_yawing_speed = 0.003;
static float player_speed = 2 * 0.007;

struct longpos player_lpos = {.chunk={0, 0}, .rpos={8, 24, 8}};

void move_player(Uint32 difftime)
{
	char md = get_movement_directions();
	tlog(5, "movement_directions %i.", (int)md);
	char x_active = ((md & DIRECTION_LEFT) || (md & DIRECTION_RIGHT)) &&
		!((md & DIRECTION_LEFT) && (md & DIRECTION_RIGHT));
	char z_active = ((md & DIRECTION_FORWARD) || (md & DIRECTION_BACKWARD))
		&& !((md & DIRECTION_FORWARD) && (md & DIRECTION_BACKWARD));

	float px = (z_active ? 0.7071 : 1) *
		(((md & DIRECTION_LEFT) ? 1 : 0)
		 -((md & DIRECTION_RIGHT) ? 1 : 0)) *
		player_speed;
	float pz = (x_active ? 0.7071 : 1) *
		(((md & DIRECTION_FORWARD) ? 1 : 0)
		 -((md & DIRECTION_BACKWARD) ? 1 : 0)) *
		player_speed;


	player_lpos.rpos[0] += difftime * (px * cos(player_yaw_rad) +
			pz * sin(player_yaw_rad)) ;

	if(md & DIRECTION_UP) {
		if(!(md & DIRECTION_DOWN)) {
			player_lpos.rpos[1] += player_speed * difftime;
		}
	} else if(md & DIRECTION_DOWN) {
		player_lpos.rpos[1] -= player_speed * difftime;
	}

	player_lpos.rpos[2] += difftime * (pz * cos(player_yaw_rad) +
			px * sin(player_yaw_rad)) ;


	/*X chunk changing*/
	if(player_lpos.rpos[0] > (double) CHUNK_LIM_HOR) {
		SDL_Log("Player changed chunk X+");
		player_lpos.rpos[0] -= (double) CHUNK_LIM_HOR;
		player_lpos.chunk[0]++;
	} else if(player_lpos.rpos[0] < 0) {
		SDL_Log("Player changed chunk X-");
		player_lpos.rpos[0] += (double) CHUNK_LIM_HOR;
		player_lpos.chunk[0]--;
	}
	/*Z chunk changing*/
	if(player_lpos.rpos[2] > (double) CHUNK_LIM_HOR) {
		SDL_Log("Player changed chunk Z+");
		player_lpos.rpos[2] -= (double) CHUNK_LIM_HOR;
		player_lpos.chunk[1]++;
	} else if(player_lpos.rpos[2] < 0) {
		SDL_Log("Player changed chunk Z-");
		player_lpos.rpos[2] += (double) CHUNK_LIM_HOR;
		player_lpos.chunk[1]--;
	}
	/*WARNING this implies that a player never moves more than a chunk per movement calculation*/	

	//SDL_Log("Players new rpos: %lf|%lf|%lf", player_lpos.rpos[0], player_lpos.rpos[1], player_lpos.rpos[2]); 
}

void player_turn(SDL_MouseMotionEvent* event)
{
	float new_pitch = player_pitch_rad - event->yrel * player_pitching_speed;
	player_yaw_rad -= event->xrel * player_yawing_speed;
	if(player_yaw_rad > (2.0 * (float) M_PI)) {
		player_yaw_rad = player_yaw_rad - (2.0 * (float) M_PI);
	} else if(player_yaw_rad < (-2.0 * (float) M_PI)) {
		player_yaw_rad = player_yaw_rad + (2.0 * (float) M_PI);
	}
	if(((new_pitch < 0.5 * (float) M_PI) && (event-> yrel < 0.0)) ||
			((new_pitch > -0.5 * (float) M_PI) && (event-> yrel > 0.0))) {
		player_pitch_rad = new_pitch;
	}
	//SDL_Log("YAW: %f, PITCH %f", player_yaw_rad, player_pitch_rad);
}

void get_player_ori(float* out)
{
	out[0] = sin(player_yaw_rad) * cos(player_pitch_rad);
	out[1] = sin(player_pitch_rad);
	out[2] = cos(player_yaw_rad) * cos(player_pitch_rad);
}

void get_player_right(float* out)
{
	out[0] = sin(player_yaw_rad - (float) M_PI/2.0);
	out[1] = 0;
	out[2] = cos(player_yaw_rad - (float) M_PI/2.0);
}
