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
    float dir;
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

bool clr_flag = false;
struct Star * create_star(void)
{
    struct Star *s = malloc (sizeof (struct Star));
    s->x = RAND(240);
    s->y = RAND(160);
    // s->z = RAND(3);

    // s->clr = clr_flag ? RGB15(205,205,205) : CLR_WHITE;
    s->clr = CLR_WHITE;
    clr_flag = !clr_flag;

    s->dir = atan2f(s->y - 79, s->x - 119);

    return s;
}



void display_star(struct Star * s)
{
    m3_mem[s->y][s->x]= s->clr;
}

void move_star(struct Star * s, int dist)
{
    s->x = s->x + dist * cos(s->dir);
    s->y = s->y + dist * sin(s->dir);
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

void vbl_handler(void)
{
    mmVBlank(); // This has to be called exactly at the beginning of VBL

    // This must be called in the VBL handler, but with lower priority
    mmFrame();
    gbt_sync_to_maxmod();

    clear_screen(CLR_BLACK);

    int i;
    for (i = 0; i < 20; i++) {
        move_star(stars[i], 3);
        if (stars[i]->x > 240 || stars[i]->y > 160 || stars[i]->x < 0 || stars[i]->y < 0) {
            free(stars[i]);
            stars[i] = create_star();
        }
        display_star(stars[i]);
    }

    // Print some debug information
    // int order, row, tick;
    // gbt_get_position(&order, &row, &tick);
    // iprintf("(%s) %d %2d %d | (%s) %d %2d %d\n",
    //         gbt_is_playing() ? "ON" : "OFF",
    //         order, row, tick,
    //         mmActive() ? "ON" : "OFF",
    //         mmGetPosition(), mmGetPositionRow(), mmGetPositionTick());
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
    // mmInitDefault((mm_addr)soundbank_bin, 4);
    // PSG channels have 1/4th of the range of DMA channels
    // mmSetModuleVolume(1024 / 4);

    // Draw some dots on the screen
    REG_DISPCNT= DCNT_MODE3 | DCNT_BG2;

    int i;
    for (i = 0; i < 20; i++) {
        stars[i] = create_star();
    }

    // Start both songs
    // gbt_play(starlight_brigade_psg, -1);
    // gbt_loop(1);
    // mmStart(MOD_STARLIGHT_BRIGADE_DMA, MM_PLAY_LOOP);

    while (1) {
        VBlankIntrWait();
    }
}
