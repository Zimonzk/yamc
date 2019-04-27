#include "chunk.h"

#define GL3_PROTOTYPES 1
#if defined(__APPLE__)
#include <OpenGL/glew.h>
#include <OpenGL/gl.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif
#include <SDL2/SDL.h>

Uint32 blocks_filled(chunk* cchunk)
{
    Uint32 num = 0;
    unsigned short x,y,z;
    for(x=0; x < CHUNK_LIM_HOR; x++) {
        for(z=0; z < CHUNK_LIM_HOR; z++) {
		for(y=0; y < CHUNK_LIM_VER; y++) {
			if(cchunk->data[x][y][z].id != 0) {
				num++;
			}
		}
	}
    }
    return num;
}

/* ammount of triangles in mescha */
Uint32 determine_mescha_size(chunk* cchunk, chunk* neighbours[4]) /*x+,x-,z+,z-*/
{
	Uint32 uncovered_sides = 0;
	unsigned short x,y,z;

	for(x=0; x < CHUNK_LIM_HOR; x++) {
		for(y=0; y < CHUNK_LIM_VER; y++) {
			for(z=0; z < CHUNK_LIM_HOR; z++) {
				if(cchunk->data[x][y][z].id != 0) {
					/*X*/
					if(x == (CHUNK_LIM_HOR - 1)) {
						//for now ignore the neighbours thing, revisit if we get too many triangles
						/*if(neighbours[0]) {
							if(!(neighbours[0]->data[0][y][z].properties & BLOCK_OPAQUE)) {*/
								uncovered_sides++;
							/*}
						}*/
						if(!(cchunk->data[x-1][y][z].properties & BLOCK_OPAQUE)) {
							uncovered_sides++;
						}
					} else if(x == 0) {
						//for now ignore the neighbours thing, revisit if we get too many triangles
						/*if(neighbours[1]) {
							if(!(neighbours[1]->data[CHUNK_LIM_HOR-1][y][z].properties & BLOCK_OPAQUE)) {*/
								uncovered_sides++;
							/*}
						}*/
						if(!(cchunk->data[x+1][y][z].properties & BLOCK_OPAQUE)) {
							uncovered_sides++;
						}
					} else {
						if(!(cchunk->data[x-1][y][z].properties & BLOCK_OPAQUE)) {
							uncovered_sides++;
						}
						if(!(cchunk->data[x+1][y][z].properties & BLOCK_OPAQUE)) {
							uncovered_sides++;
						}
					}
					/*Z*/
					if(z == (CHUNK_LIM_HOR - 1)) {
						//for now ignore the neighbours thing, revisit if we get too many triangles
						/*if(neighbours[2]) {
							if(!(neighbours[2]->data[x][y][0].properties & BLOCK_OPAQUE)) {*/
								uncovered_sides++;
							/*}
						}*/
						if(!(cchunk->data[x][y][z-1].properties & BLOCK_OPAQUE)) {
							uncovered_sides++;
						}
					} else if(z == 0) {
						//for now ignore the neighbours thing, revisit if we get too many triangles
						/*if(neighbours[3]) {
							if(!(neighbours[3]->data[x][y][CHUNK_LIM_HOR-1].properties & BLOCK_OPAQUE)) {*/
								uncovered_sides++;
							/*}
						}*/
						if(!(cchunk->data[x][y][z+1].properties & BLOCK_OPAQUE)) {
							uncovered_sides++;
						}
					} else {
						if(!(cchunk->data[x][y][z-1].properties & BLOCK_OPAQUE)) {
							uncovered_sides++;
						}
						if(!(cchunk->data[x][y][z+1].properties & BLOCK_OPAQUE)) {
							uncovered_sides++;
						}
					}
					/*Y*/
					if(y == (CHUNK_LIM_VER - 1)) {
						uncovered_sides++;
						if(!(cchunk->data[x][y-1][z].properties & BLOCK_OPAQUE)) {
							uncovered_sides++;
						}
					} else if(y == 0) {
						uncovered_sides++;
						if(!(cchunk->data[x][y+1][z].properties & BLOCK_OPAQUE)) {
							uncovered_sides++;
						}
					} else {
						if(!(cchunk->data[x][y-1][z].properties & BLOCK_OPAQUE)) {
							uncovered_sides++;
						}
						if(!(cchunk->data[x][y+1][z].properties & BLOCK_OPAQUE)) {
							uncovered_sides++;
						}
					}
				}
			}
		}
	}

	//SDL_Log("end");

	return (uncovered_sides * 2);
}

static void generate_block_side(const char* side, unsigned short x, unsigned short y, unsigned short z, GLfloat* mescha, Uint32 baseOffset)
{
	switch(side[0]) {
		case 'x':
			if(side[1] == '+') {
				mescha[baseOffset]   = (GLfloat) x+1;
				mescha[baseOffset+1] = (GLfloat) y;
				mescha[baseOffset+2] = (GLfloat) z+1;

				mescha[baseOffset+3] = (GLfloat) x+1;
				mescha[baseOffset+4] = (GLfloat) y;
				mescha[baseOffset+5] = (GLfloat) z;

				mescha[baseOffset+6] = (GLfloat) x+1;
				mescha[baseOffset+7] = (GLfloat) y+1;
				mescha[baseOffset+8] = (GLfloat) z;


				mescha[baseOffset+9]  = (GLfloat) x+1;
				mescha[baseOffset+10] = (GLfloat) y;
				mescha[baseOffset+11] = (GLfloat) z+1;

				mescha[baseOffset+12] = (GLfloat) x+1;
				mescha[baseOffset+13] = (GLfloat) y+1;
				mescha[baseOffset+14] = (GLfloat) z;

				mescha[baseOffset+15] = (GLfloat) x+1;
				mescha[baseOffset+16] = (GLfloat) y+1;
				mescha[baseOffset+17] = (GLfloat) z+1;
			} else {
				mescha[baseOffset]   = (GLfloat) x;
				mescha[baseOffset+1] = (GLfloat) y;
				mescha[baseOffset+2] = (GLfloat) z;

				mescha[baseOffset+3] = (GLfloat) x;
				mescha[baseOffset+4] = (GLfloat) y;
				mescha[baseOffset+5] = (GLfloat) z+1;

				mescha[baseOffset+6] = (GLfloat) x;
				mescha[baseOffset+7] = (GLfloat) y+1;
				mescha[baseOffset+8] = (GLfloat) z+1;


				mescha[baseOffset+9]  = (GLfloat) x;
				mescha[baseOffset+10] = (GLfloat) y;
				mescha[baseOffset+11] = (GLfloat) z;

				mescha[baseOffset+12] = (GLfloat) x;
				mescha[baseOffset+13] = (GLfloat) y+1;
				mescha[baseOffset+14] = (GLfloat) z+1;

				mescha[baseOffset+15] = (GLfloat) x;
				mescha[baseOffset+16] = (GLfloat) y+1;
				mescha[baseOffset+17] = (GLfloat) z;
			}
			break;
		case 'y':
			if(side[1] == '+') {
				mescha[baseOffset]   = (GLfloat) x+1;
				mescha[baseOffset+1] = (GLfloat) y+1;
				mescha[baseOffset+2] = (GLfloat) z+1;

				mescha[baseOffset+3] = (GLfloat) x+1;
				mescha[baseOffset+4] = (GLfloat) y+1;
				mescha[baseOffset+5] = (GLfloat) z;

				mescha[baseOffset+6] = (GLfloat) x;
				mescha[baseOffset+7] = (GLfloat) y+1;
				mescha[baseOffset+8] = (GLfloat) z;


				mescha[baseOffset+9]  = (GLfloat) x+1;
				mescha[baseOffset+10] = (GLfloat) y+1;
				mescha[baseOffset+11] = (GLfloat) z+1;

				mescha[baseOffset+12] = (GLfloat) x;
				mescha[baseOffset+13] = (GLfloat) y+1;
				mescha[baseOffset+14] = (GLfloat) z;

				mescha[baseOffset+15] = (GLfloat) x;
				mescha[baseOffset+16] = (GLfloat) y+1;
				mescha[baseOffset+17] = (GLfloat) z+1;
			} else {
				mescha[baseOffset]   = (GLfloat) x;
				mescha[baseOffset+1] = (GLfloat) y;
				mescha[baseOffset+2] = (GLfloat) z+1;

				mescha[baseOffset+3] = (GLfloat) x;
				mescha[baseOffset+4] = (GLfloat) y;
				mescha[baseOffset+5] = (GLfloat) z;

				mescha[baseOffset+6] = (GLfloat) x+1;
				mescha[baseOffset+7] = (GLfloat) y;
				mescha[baseOffset+8] = (GLfloat) z;


				mescha[baseOffset+9]  = (GLfloat) x;
				mescha[baseOffset+10] = (GLfloat) y;
				mescha[baseOffset+11] = (GLfloat) z+1;

				mescha[baseOffset+12] = (GLfloat) x+1;
				mescha[baseOffset+13] = (GLfloat) y;
				mescha[baseOffset+14] = (GLfloat) z;

				mescha[baseOffset+15] = (GLfloat) x+1;
				mescha[baseOffset+16] = (GLfloat) y;
				mescha[baseOffset+17] = (GLfloat) z+1;
			}
			break;
		case 'z':
			if(side[1] == '+') {
				mescha[baseOffset]   = (GLfloat) x;
				mescha[baseOffset+1] = (GLfloat) y;
				mescha[baseOffset+2] = (GLfloat) z+1;

				mescha[baseOffset+3] = (GLfloat) x+1;
				mescha[baseOffset+4] = (GLfloat) y;
				mescha[baseOffset+5] = (GLfloat) z+1;

				mescha[baseOffset+6] = (GLfloat) x+1;
				mescha[baseOffset+7] = (GLfloat) y+1;
				mescha[baseOffset+8] = (GLfloat) z+1;


				mescha[baseOffset+9]  = (GLfloat) x;
				mescha[baseOffset+10] = (GLfloat) y;
				mescha[baseOffset+11] = (GLfloat) z+1;

				mescha[baseOffset+12] = (GLfloat) x+1;
				mescha[baseOffset+13] = (GLfloat) y+1;
				mescha[baseOffset+14] = (GLfloat) z+1;

				mescha[baseOffset+15] = (GLfloat) x;
				mescha[baseOffset+16] = (GLfloat) y+1;
				mescha[baseOffset+17] = (GLfloat) z+1;
			} else {
				mescha[baseOffset]   = (GLfloat) x+1;
				mescha[baseOffset+1] = (GLfloat) y;
				mescha[baseOffset+2] = (GLfloat) z;

				mescha[baseOffset+3] = (GLfloat) x;
				mescha[baseOffset+4] = (GLfloat) y;
				mescha[baseOffset+5] = (GLfloat) z;

				mescha[baseOffset+6] = (GLfloat) x;
				mescha[baseOffset+7] = (GLfloat) y+1;
				mescha[baseOffset+8] = (GLfloat) z;


				mescha[baseOffset+9]  = (GLfloat) x+1;
				mescha[baseOffset+10] = (GLfloat) y;
				mescha[baseOffset+11] = (GLfloat) z;

				mescha[baseOffset+12] = (GLfloat) x;
				mescha[baseOffset+13] = (GLfloat) y+1;
				mescha[baseOffset+14] = (GLfloat) z;

				mescha[baseOffset+15] = (GLfloat) x+1;
				mescha[baseOffset+16] = (GLfloat) y+1;
				mescha[baseOffset+17] = (GLfloat) z;
			}
			break;
		default:
			SDL_Log("not x,y or z used");
			break;
	}
}

static void generate_block_texis(unsigned int block_id, Uint32 sideIndex, GLuint* texis, enum block_side face)
{
	struct block_register* blrp = get_registered_block(block_id);
	unsigned int tid = blrp->texis[face];
	/* TODO add facing of blocks */
	texis[6*sideIndex] = tid;
	texis[6*sideIndex+1] = tid;
	texis[6*sideIndex+2] = tid;
	texis[6*sideIndex+3] = tid;
	texis[6*sideIndex+4] = tid;
	texis[6*sideIndex+5] = tid;
}

Uint32 generate_mescha(chunk* cchunk, chunk* neighbours[4], GLuint vertexbuffer, GLuint texibuffer)
{
	Uint32 mescha_size = determine_mescha_size(cchunk, neighbours);

	GLuint texis[mescha_size*3];

	GLfloat* mescha;
	Uint32 sideIndex = 0;

	unsigned short x,y,z;


	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 9 * mescha_size, NULL, GL_STATIC_DRAW);

	mescha = (GLfloat*) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	for(x=0; x < CHUNK_LIM_HOR; x++) {
		for(y=0; y < CHUNK_LIM_VER; y++) {
			for(z=0; z < CHUNK_LIM_HOR; z++) {
				if(cchunk->data[x][y][z].id != 0) {
					/*X*/
					if(x == (CHUNK_LIM_HOR - 1)) {
						//for now ignore the neighbours thing, revisit if we get too many triangles
						/*if(neighbours[0]) { 
							if(!(neighbours[0]->data[0][y][z].properties & BLOCK_OPAQUE)) {*/
								generate_block_side("x+", x,y,z, mescha, 18*sideIndex);
								generate_block_texis(cchunk->data[x][y][z].id, sideIndex, texis, BACK);
								sideIndex++;
							/*}
						}*/
						if(!(cchunk->data[x-1][y][z].properties & BLOCK_OPAQUE)) {
							generate_block_side("x-", x,y,z, mescha, 18*sideIndex);
							generate_block_texis(cchunk->data[x][y][z].id, sideIndex, texis, FRONT);
							sideIndex++;
						}
					} else if(x == 0) {
						//for now ignore the neighbours thing, revisit if we get too many triangles
						/*if(neighbours[1]) {
							if(!(neighbours[1]->data[CHUNK_LIM_HOR-1][y][z].properties & BLOCK_OPAQUE)) {*/
								generate_block_side("x-", x,y,z, mescha, 18*sideIndex);
								generate_block_texis(cchunk->data[x][y][z].id, sideIndex, texis, FRONT);
								sideIndex++;
							/*}
						}*/
						if(!(cchunk->data[x+1][y][z].properties & BLOCK_OPAQUE)) {
							generate_block_side("x+", x,y,z, mescha, 18*sideIndex);
							generate_block_texis(cchunk->data[x][y][z].id, sideIndex, texis, BACK);
							sideIndex++;
						}
					} else {
						if(!(cchunk->data[x-1][y][z].properties & BLOCK_OPAQUE)) {
							generate_block_side("x-", x,y,z, mescha, 18*sideIndex);
							generate_block_texis(cchunk->data[x][y][z].id, sideIndex, texis, FRONT);
							sideIndex++;
						}
						if(!(cchunk->data[x+1][y][z].properties & BLOCK_OPAQUE)) {
							generate_block_side("x+", x,y,z, mescha, 18*sideIndex);
							generate_block_texis(cchunk->data[x][y][z].id, sideIndex, texis, BACK);
							sideIndex++;
						}
					}
					/*Z*/
					if(z == (CHUNK_LIM_HOR - 1)) {
						//for now ignore the neighbours thing, revisit if we get too many triangles
						/*if(neighbours[2]) {
							if(!(neighbours[2]->data[x][y][0].properties & BLOCK_OPAQUE)) {*/
								generate_block_side("z+", x,y,z, mescha, 18*sideIndex);
								generate_block_texis(cchunk->data[x][y][z].id, sideIndex, texis, RIGHT);
								sideIndex++;
							/*}
						}*/
						if(!(cchunk->data[x][y][z-1].properties & BLOCK_OPAQUE)) {
							generate_block_side("z-", x,y,z, mescha, 18*sideIndex);
							generate_block_texis(cchunk->data[x][y][z].id, sideIndex, texis, LEFT);
							sideIndex++;
						}
					} else if(z == 0) {
						//for now ignore the neighbours thing, revisit if we get too many triangles
						/*if(neighbours[3]) {
							if(!(neighbours[3]->data[x][y][CHUNK_LIM_HOR-1].properties & BLOCK_OPAQUE)) {*/
								generate_block_side("z-", x,y,z, mescha, 18*sideIndex);
								generate_block_texis(cchunk->data[x][y][z].id, sideIndex, texis, LEFT);
								sideIndex++;
							/*}
						}*/
						if(!(cchunk->data[x][y][z+1].properties & BLOCK_OPAQUE)) {
							generate_block_side("z+", x,y,z, mescha, 18*sideIndex);
							generate_block_texis(cchunk->data[x][y][z].id, sideIndex, texis, RIGHT);
							sideIndex++;
						}
					} else {
						if(!(cchunk->data[x][y][z-1].properties & BLOCK_OPAQUE)) {
							generate_block_side("z-", x,y,z, mescha, 18*sideIndex);
							generate_block_texis(cchunk->data[x][y][z].id, sideIndex, texis, LEFT);
							sideIndex++;
						}
						if(!(cchunk->data[x][y][z+1].properties & BLOCK_OPAQUE)) {
							generate_block_side("z+", x,y,z, mescha, 18*sideIndex);
							generate_block_texis(cchunk->data[x][y][z].id, sideIndex, texis, RIGHT);
							sideIndex++;
						}
					}
					/*Y*/
					if(y == (CHUNK_LIM_VER - 1)) {
						generate_block_side("y+", x,y,z, mescha, 18*sideIndex);
						generate_block_texis(cchunk->data[x][y][z].id, sideIndex, texis, UPPER);
						sideIndex++;

						if(!(cchunk->data[x][y-1][z].properties & BLOCK_OPAQUE)) {
							generate_block_side("y-", x,y,z, mescha, 18*sideIndex);
							generate_block_texis(cchunk->data[x][y][z].id, sideIndex, texis, LOWER);
							sideIndex++;
						}
					} else if(y == 0) {
						generate_block_side("y-", x,y,z, mescha, 18*sideIndex);
						generate_block_texis(cchunk->data[x][y][z].id, sideIndex, texis, LOWER);
						sideIndex++;

						if(!(cchunk->data[x][y+1][z].properties & BLOCK_OPAQUE)) {
							generate_block_side("y+", x,y,z, mescha, 18*sideIndex);
							generate_block_texis(cchunk->data[x][y][z].id, sideIndex, texis, UPPER);
							sideIndex++;
						}
					} else {
						if(!(cchunk->data[x][y-1][z].properties & BLOCK_OPAQUE)) {
							generate_block_side("y-", x,y,z, mescha, 18*sideIndex);
							generate_block_texis(cchunk->data[x][y][z].id, sideIndex, texis, LOWER);
							sideIndex++;
						}
						if(!(cchunk->data[x][y+1][z].properties & BLOCK_OPAQUE)) {
							generate_block_side("y+", x,y,z, mescha, 18*sideIndex);
							generate_block_texis(cchunk->data[x][y][z].id, sideIndex, texis, UPPER);
							sideIndex++;
						}
					}
				}
			}
		}
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);

	glBindBuffer(GL_ARRAY_BUFFER, texibuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * 3 * mescha_size, texis, GL_STATIC_DRAW);

	return mescha_size;
}

void fillchunktest(chunk* cchunk) {
	short x, y, z;
	for(x=1; x < (CHUNK_LIM_HOR - 1); x++) {
		for(z=1; z < (CHUNK_LIM_HOR - 1); z++) {
			for(y=0; y < (CHUNK_LIM_VER/8); y++) {
				cchunk->data[x][y][z].id = (y % 2) + 1;
				cchunk->data[x][y][z].properties = 0b1;
			}
		}
	}
}
