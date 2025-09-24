#include <gba.h>
#include <maxmod.h>

#include <stdio.h>
#include <stdlib.h>

#include "soundbank.h"
#include "soundbank_bin.h"

int main() {
    // Initialize GBA system
    irqInit();
	irqSet( IRQ_VBLANK, mmVBlank );
    irqEnable(IRQ_VBLANK);

	consoleDemoInit();

    // Initialize Maxmod with the soundbank
    mmInitDefault( (mm_addr)soundbank_bin, 8 );

    // Start playing module
    mmStart(MOD_FLATOUTLIES, MM_PLAY_LOOP);

    do {
        // Update Maxmod
        VBlankIntrWait();
        mmFrame();
    } while(1);
}