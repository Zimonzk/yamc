#define _USE_MATH_DEFINES

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
	struct keystates ks = *get_keystates();
	if(ks.UP) {
		player_lpos.rpos[0] += player_speed * difftime * sin(player_yaw_rad) * cos(player_pitch_rad);
		player_lpos.rpos[1] += player_speed * difftime * sin(player_pitch_rad);
		player_lpos.rpos[2] += player_speed * difftime * cos(player_yaw_rad) * cos(player_pitch_rad);
	}
	if(ks.DOWN) {
		player_lpos.rpos[0] -= player_speed * difftime * sin(player_yaw_rad) * cos(player_pitch_rad);
		player_lpos.rpos[1] -= player_speed * difftime * sin(player_pitch_rad);
		player_lpos.rpos[2] -= player_speed * difftime * cos(player_yaw_rad) * cos(player_pitch_rad);
	}
	if(ks.DOWN || ks.UP) {
		/*X chunk changing*/
		if(player_lpos.rpos[0] > (double) CHUNK_LIM_HOR) {
			player_lpos.rpos[0] -= (double) CHUNK_LIM_HOR;
			player_lpos.chunk[0]++;
		} else if(player_lpos.rpos[0] < 0) {
			player_lpos.rpos[0] += (double) CHUNK_LIM_HOR;
			player_lpos.chunk[0]--;
		}
		/*Z chunk changing*/
		if(player_lpos.rpos[2] > (double) CHUNK_LIM_HOR) {
			player_lpos.rpos[2] -= (double) CHUNK_LIM_HOR;
			player_lpos.chunk[1]++;
		} else if(player_lpos.rpos[2] < 0) {
			player_lpos.rpos[2] += (double) CHUNK_LIM_HOR;
			player_lpos.chunk[1]--;
		}
		/*WARNING this implies that a player never moves more than a chunk per movement calculation*/	
	}
}

void player_turn(SDL_MouseMotionEvent* event)
{
	player_yaw_rad -= event->xrel * player_yawing_speed;
	if(player_yaw_rad > (2.0 * (float) M_PI)) {
		player_yaw_rad = player_yaw_rad - (2.0 * (float) M_PI);
	} else if(player_yaw_rad < (-2.0 * (float) M_PI)) {
		player_yaw_rad = player_yaw_rad + (2.0 * (float) M_PI);
	}
	if((player_pitch_rad < 0.5 * (float) M_PI) && (event-> yrel < 0.0) ||
			(player_pitch_rad > -0.5 * (float) M_PI) && (event-> yrel > 0.0)) {
		player_pitch_rad -= event->yrel * player_pitching_speed;
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
