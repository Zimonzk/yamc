#ifndef BLOCK_H_INCLUDED
#define BLOCK_H_INCLUDED

#define BLOCKID_MAX 4096

#define BLOCK_OPAQUE 0b1

typedef struct s_block
{
    unsigned int id;
    unsigned char meta;
    unsigned char properties;
} block;

enum side {UPPER, LOWER, LEFT, RIGHT, FRONT, BACK};

#endif // BLOCK_H_INCLUDED
