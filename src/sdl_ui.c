#include "sdl_ui.h"
#include "textures.xpm"
#include <SDL.h>
#include <SDL_image.h>
#include <stdlib.h>
#include <stdint.h>

SDL_Surface* tsurf = 0; //texture surface (128x128)
SDL_Surface* scs = 0; //screen surface (256x224)

unsigned int win_w = 640;
unsigned int win_h = 480;

SDL_Rect boxify() {

	double ratio = (win_w * 0.75) / win_h;

	unsigned int true_w = (ratio >= 1) ? win_h / 0.75 : win_w;
	unsigned int true_h = (ratio >= 1) ? win_h : win_w * 0.75;

	SDL_Rect viewport = {.x = (win_w - true_w) / 2, .y = (win_h - true_h) / 2, .w = true_w, .h = true_h};
	return viewport;
}

int spr(uint8_t spr, uint8_t x, uint8_t y, uint8_t w, uint8_t h) {

	SDL_Rect sr = {.x = (spr % 16)*8, .y = (spr / 16)*8, .w = w*8, .h = h*8};
	SDL_Rect dr = {.x = x, .y = y, .w = w*8, .h = h*8};
	return SDL_BlitSurface(tsurf,&sr,scs,&dr);
}

const char* sprchars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ,.- >";

int s_addstr(const char* text, uint8_t x, uint8_t y) {

	uint8_t cx = x;
	uint8_t cy = y;
	const char* ct = text;

	while (*ct) {

		if (*ct == '\n') {
			cx = x;
			cy += 16;
		} else {

			char* tchar = strchr(sprchars,toupper(*ct));

			if (tchar) {
				int ci = (tchar - sprchars);
				uint8_t si = 96 + (ci / 16)*32 + (ci % 16);
				spr(si,cx,cy,1,2);
			}
			cx += 8;
		}
		ct++;
	}
	return 0;
}

int s_addstr_c(const char* text, uint8_t y) {
	return s_addstr(text, 128 - (strlen(text)*4), y);
}

int sdl_ui_main(void) {

	int r = SDL_Init(SDL_INIT_VIDEO);
	if (r != 0) {
		fprintf(stderr,"Unable to start SDL video: %s\n",SDL_GetError()); exit(1); }

	atexit(SDL_Quit);

	SDL_Window* mainwin = SDL_CreateWindow("snescityeditor",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,win_w,win_h,SDL_WINDOW_RESIZABLE);

	if (!mainwin) {
		fprintf(stderr,"Unable to create window: %s\n",SDL_GetError()); exit(1); }

	SDL_Renderer* ren = SDL_CreateRenderer(mainwin,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	//SDL_RenderSetLogicalSize(ren, 256, 224);

	if (!ren) {
		fprintf(stderr,"Unable to create renderer: %s\n",SDL_GetError()); exit(1); }


	scs = SDL_CreateRGBSurface(0,256,224,32,0x00FF0000,0x0000FF00,0x000000FF,0xFF000000);
	
	if (!scs) {
		fprintf(stderr,"Unable to create surface: %s\n",SDL_GetError()); exit(1); }

	SDL_Texture* tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,256,224);
	
	if (!tex) {
		fprintf(stderr,"Unable to create texture: %s\n",SDL_GetError()); exit(1); }

	tsurf = IMG_ReadXPMFromArray(textures_xpm);

	for (int ix=0; ix < 16; ix++) {
		spr(68,ix*16,0,2,2);
		
		for (int iy=4; iy < 18; iy++) spr(68,ix*16,iy*16,2,2);
	}


	//SDL_Rect bgrect = {.x=0,.y=0,.w=256,.h=224};
	//SDL_FillRect(scs, &bgrect, SDL_MapRGB(scs->format,79,40,15));

	s_addstr_c("SNESCITYEDITOR",40);

	//        123456789012345678901234
	s_addstr("  THIS GRAPHICAL UI IS  \n"
		 "  NOT YET IMPLEMENTED.\n"
		 "BUT YOU CAN ALREADY SEE\n"
		 "THE SNES-LIKE NON-SQUARE\n"
		 "PIXEL SCALING AND SUCH.\n"
		 "\n",32,80);


	SDL_Event lastevent;

	int loop = 1;
	int rerender = 1;

	while ( (loop) && (SDL_WaitEvent(&lastevent)) ) {

		if (rerender) {
	
			SDL_UpdateTexture(tex, NULL, scs->pixels, scs->pitch);
			SDL_RenderClear(ren);
			SDL_RenderCopy(ren, tex, NULL, NULL);
			SDL_RenderPresent(ren);

			rerender = 0;
		}

		switch(lastevent.type) {
			case SDL_WINDOWEVENT:
				if (lastevent.window.event != SDL_WINDOWEVENT_SIZE_CHANGED) break;
				
				SDL_GetWindowSize(mainwin,&win_w,&win_h);
				SDL_Rect newsize = boxify();
				SDL_RenderSetViewport(ren, &newsize);
				rerender = 1;
				break;
			case SDL_QUIT:
				loop = 0;
		}

		if (rerender) {
	
			SDL_UpdateTexture(tex, NULL, scs->pixels, scs->pitch);
			SDL_RenderClear(ren);
			SDL_RenderCopy(ren, tex, NULL, NULL);
			SDL_RenderPresent(ren);

			rerender = 0;
		}

	}

	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(mainwin);

	return 0;
}
