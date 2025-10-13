// SPDX-License-Identifier: MIT
//
// Copyright (c) 2022 Antonio Niño Díaz <antonio_nd@outlook.com>

// Example that shows how to play a song with Maxmod and GBT Player at the same
// time and keep them in sync.

// #include "rand.c"
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <math.h>

#include <gba.h>
#include <gba_console.h>
#include <maxmod.h>

#include "toolbox.h"
#include "gbt_player/gbt_player.h"

#include "soundbank.h"
#include "soundbank_bin.h"

extern const uint8_t *starlight_brigade_psg[];
struct Star {
    int x, y, z, clr;
    float dSin, dCos;
};

#define VCOUNT *(volatile u16*)0x4000006
volatile s32 RAND_RandomData;

void SeedRandom(void);
s32 RAND(s32 Value);

// extra stuff, also in tonc_video.h
#define M3_WIDTH    SCREEN_WIDTH
// typedef for a whole mode3 line
typedef COLOR       M3LINE[M3_WIDTH];
// m3_mem is a matrix; m3_mem[y][x] is pixel (x,y)
#define m3_mem    ((M3LINE*)MEM_VRAM)

#define STAR_SPEED 3
bool clr_flag = false;
struct Star * create_star(void)
{
    struct Star *s = malloc (sizeof (struct Star));
    s->x = RAND(240) - 1;
    s->y = RAND(160) - 1;
    // s->z = RAND(3);

    s->clr = clr_flag ? RGB15(205,205,205) : CLR_WHITE;
    // s->clr = CLR_WHITE;
    clr_flag = !clr_flag;

    float dir = atan2f(s->y - 79, s->x - 119);
    s->dSin = sin(dir);
    s->dCos = cos(dir);

    return s;
}

void move_star(struct Star * s)
{
    s->x = s->x + STAR_SPEED * s->dCos;
    s->y = s->y + STAR_SPEED * s->dSin;
}

bool star_oob(struct Star * s) {
    // TODO: include stars that have become "lodged" in image
    return s->x > SCREEN_WIDTH || s->y > SCREEN_HEIGHT || s->x < 0 || s->y < 0;
}

struct Star * stars[20];

void gbt_sync_to_maxmod(void)
{
    if (!gbt_is_playing())
        return;

    if (!mmActive())
        return;

    int tries = 5;

    while (tries > 0)
    {
        tries--;

        gbt_update();

        int order, row, tick;
        gbt_get_position(&order, &row, &tick);

        if (order != mmGetPosition())
            continue;

        if (row != mmGetPositionRow())
            continue;

        if (tick != mmGetPositionTick())
            continue;

        break;
    }

    // Something went wrong somehow!
}

/* clear the screen to black */
void clear_screen(unsigned short color) {
    unsigned short row, col;
    /* set each pixel black */
    for (row = 0; row < SCREEN_HEIGHT; row++) {
        for (col = 0; col < SCREEN_WIDTH; col++) {
            m3_mem[row][col]= color;
        }
    }
}

void clear_edges(unsigned short color) {
    unsigned short row, col;
    for (col = 0; col < 4; col++) {
        for (row = 0; row < SCREEN_HEIGHT; row++) {
            m3_mem[row][col]= color;
        }
    }
    for (col = SCREEN_WIDTH-4; col < SCREEN_WIDTH; col++) {
        for (row = 0; row < SCREEN_HEIGHT; row++) {
            m3_mem[row][col]= color;
        }
    }
}

void vbl_handler(void)
{
    mmVBlank(); // This has to be called exactly at the beginning of VBL

    // This must be called in the VBL handler, but with lower priority
    mmFrame();
    gbt_sync_to_maxmod();


    int i;
    for (i = 0; i < 20; i++) {

        // Move star (while retaining previous position)
        int sx = stars[i]->x;
        int sy = stars[i]->y;

        // Recreate stars that have moved out of bounds
        if (star_oob(stars[i])) {
            free(stars[i]);
            stars[i] = create_star();
        } else {
            // Erase current position
            m3_mem[sy][sx]= CLR_BLACK;
        }

        // Display star at new position
        move_star(stars[i]);
        m3_mem[stars[i]->y][stars[i]->x]= stars[i]->clr;
    }

    clear_edges(CLR_BLACK);
}

s32 RAND(s32 Value)
{
   RAND_RandomData *= 20077;
   RAND_RandomData += 12345;

   return abs(((((RAND_RandomData >> 16) & RAND_MAX) * Value) >> 15)-1);
}

int main(int argc, char *argv[])
{
    RAND_RandomData = (int)VCOUNT;
    irqInit();

    irqSet(IRQ_VBLANK, vbl_handler);
    irqEnable(IRQ_VBLANK);

    // Initialize maxmod with soundbank and 4 channels
    mmInitDefault((mm_addr)soundbank_bin, 4);
    // PSG channels have 1/4th of the range of DMA channels
    mmSetModuleVolume(1024 / 4);

    // Draw some dots on the screen
    REG_DISPCNT= DCNT_MODE3 | DCNT_BG2;

    int i;
    for (i = 0; i < 20; i++) {
        stars[i] = create_star();
    }

    // Start both songs
    gbt_play(starlight_brigade_psg, -1);
    gbt_loop(1);
    mmStart(MOD_STARLIGHT_BRIGADE_DMA, MM_PLAY_LOOP);

    clear_screen(CLR_BLACK); // Why is this needed?

    while (1) {
        VBlankIntrWait();
    }
}
