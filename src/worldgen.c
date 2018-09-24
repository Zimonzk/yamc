#include "worldgen.h"

void generate_chunk(chunk* chu, struct osn_context* ctn)
{
    unsigned int x, y, z;
    double scale = 64.0;

    SDL_Log("Generation chunk #x,#z: [%ld, %ld].\n", chu->offset_X, chu->offset_Z);
    //open_simplex_noise(TEST_SEED, &ctn);
    for(x = 0; x < CHUNK_LIM_HOR; x++) {
	    for(z = 0; z < CHUNK_LIM_HOR; z++) {
		    //SDL_Log("Noise: %u", (unsigned int) (10.0 * (open_simplex_noise2(ctn, x, z) + 1)));
		    y = (unsigned int) (10.0 * (open_simplex_noise2(ctn, (x + (CHUNK_LIM_HOR * chu->offset_X))/scale, (z + (CHUNK_LIM_HOR * chu->offset_Z))/scale) + 1));
		    //SDL_Log("y: %u", y);
		    while(y > 0) {
			    y--;
			    //SDL_Log("y: %u", y);
			    if(y < CHUNK_LIM_VER) {
				    chu->data[x][y][z].id = 1;
				    chu->data[x][y][z].properties = BLOCK_OPAQUE;
			    }
		    }
	    }
    }
    SDL_Log("Done generating this chunk.");
}
