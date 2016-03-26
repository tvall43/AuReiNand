/*
*   draw.c
*       by Reisyukaku / Aurora Wright
*   Code to print to the screen by mid-kid @CakesFW
*
*   Copyright (c) 2016 All Rights Reserved
*/

#include "draw.h"
#include "fs.h"
#include "memory.h"
#include "font.h"

#define SCREEN_TOP_WIDTH 400
#define SCREEN_TOP_HEIGHT 240

#define SLEEP_1000 0x5600 // need new measurements, this is WILDLY inaccurate

#define NO_OP "andeq r0, r0, r0" // thanks wikipedia, now I know

//extern u32 rand();
extern void sleep(u32 miliseconds);

static const struct fb {
    u8 *top_left;
    u8 *top_right;
    u8 *bottom;
} *const fb = (struct fb *)0x23FFFE00;

struct framebuffers { // thsnks to mid-kid for fb offsets
    u8 *top_left;
    u8 *top_right;
    u8 *bottom;
} *framebuffers = (struct framebuffers *) 0x23FFFE00;

u32 fb_sz(u8* fb) {
	if (fb == framebuffers->top_left || fb == framebuffers->top_right)
		return TOP_FB_SZ;
	else return BOTTOM_FB_SZ;
}

void SetPixel(u8* fb, u32 x, u32 y, u8 b, u8 g, u8 r) {
	u32 offset = (240*x + 240 - y - 1) * 3;
	*((u8*)fb + offset++) = b;
	*((u8*)fb + offset++) = g;
	*((u8*)fb + offset++) = r;
}


static int strlen(const char *string){
    char *stringEnd = (char *)string;
    while(*stringEnd) stringEnd++;
    return stringEnd - string;
}

void clearScreens(void){
    memset32(fb->top_left, 0, 0x46500);
    memset32(fb->top_right, 0, 0x46500);
    memset32(fb->bottom, 0, 0x38400);
}

void setScreenColor(u8* fb, u32 width, u32 b, u32 g, u32 r) {
	for (u32 x = 0; x < width; x++)
		for (u32 y = 0; y < 240; y++)
			SetPixel(fb, x, y, b, g, r);
}

void corruptScreen(u8* fb, u32 parts) { // when parts go up, speed goes up and randomness goes down

	for (u32 i = 0; i < fb_sz(fb)/parts; i++)
		memset(&fb[i], rand() % 0xFF, 1); // set random data
	
	for (u32 n = 0; n < parts; n++)
		memcpy(&fb[n * (fb_sz(fb)/parts)], fb, fb_sz(fb)/parts); // copy random data over and over
}

void sleep(u32 miliseconds) { // please note: NOT PRECISE AT ALL, BARELY WORKS AS INTENDED
	u32 i = (SLEEP_1000*miliseconds); while(--i) __asm(NO_OP); // no_op for SLEEP_1000*miliseconds, no idea if this will perform differently on N3DS (doubt it)
}

void drawMenuHaxCrash() {
	setScreenColor(fb->bottom, 320, 0, 0, 0);
	sleep(900);
	corruptScreen(fb->top_left, 12);
	setScreenColor(fb->bottom, 320, 255, 255, 255);
	sleep(800);
	setScreenColor(fb->bottom, 320, 0, 255, 0);
	sleep(800);
	setScreenColor(fb->bottom, 320, 0, 255, 255);
	sleep(800);

}

void loadSplash(void){
    clearScreens();
    drawMenuHaxCrash();
    //Don't delay boot if no splash image is on the SD
    if(fileRead(fb->top_left, "/aurei/splash.bin", 0x46500) +
       fileRead(fb->bottom, "/aurei/splashbottom.bin", 0x38400)){
        u64 i = 0x1300000; while(--i) __asm("mov r0, r0"); //Less Ghetto sleep func
    }
}

void drawCharacter(char character, int pos_x, int pos_y, u32 color){
    u8 *const select = fb->top_left;

    for(int y = 0; y < 8; y++){
        unsigned char char_pos = font[character * 8 + y];

        for(int x = 7; x >= 0; x--){
            int screen_pos = (pos_x * SCREEN_TOP_HEIGHT * 3 + (SCREEN_TOP_HEIGHT - y - pos_y - 1) * 3) + (7 - x) * 3 * SCREEN_TOP_HEIGHT;

            if ((char_pos >> x) & 1) {
                select[screen_pos] = color >> 16;
                select[screen_pos + 1] = color >> 8;
                select[screen_pos + 2] = color;
            }
        }
    }
}

int drawString(const char *string, int pos_x, int pos_y, u32 color){
    int length = strlen(string);

    for(int i = 0, line_i = 0; i < length; i++, line_i++){
        if(string[i] == '\n'){
            pos_y += SPACING_VERT;
            line_i = 0;
            i++;
        } else if(line_i >= (SCREEN_TOP_WIDTH - pos_x) / SPACING_HORIZ){
            // Make sure we never get out of the screen.
            pos_y += SPACING_VERT;
            line_i = 2;  // Little offset so we know the same string continues.
            if(string[i] == ' ') i++;  // Spaces at the start look weird
        }

        drawCharacter(string[i], pos_x + line_i * SPACING_HORIZ, pos_y, color);
    }

    return pos_y;
}
