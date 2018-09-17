#include "block.h"

#include <gl/gl.h>

static int (*texiretrievalptrs[BLOCKID_MAX])(enum side);

GLuint getblocksidetexi(unsigned int bid, enum side bsi)
{
    return texiretrievalptrs[bid-1](bsi);
}

void registerblocktexiretrieval(int bid, int (*texiretrievalptr)(enum side))
{
    texiretrievalptrs[bid-1] = texiretrievalptr;
}
