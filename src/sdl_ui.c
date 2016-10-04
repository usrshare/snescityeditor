#include "sdl_ui.h"
#include "textures.xpm"
#include <SDL.h>
#include <SDL_image.h>
#include <stdlib.h>
#include <stdint.h>

// generic variables

SDL_Window* mainwin = 0; //main window
SDL_Renderer* ren = 0; //renderer
SDL_Surface* tsurf = 0; //texture surface (128x128)
SDL_Surface* scs = 0; //screen surface (256x224)
SDL_Texture* tex = 0; //screen texture (256x224)

unsigned int win_w = 640;
unsigned int win_h = 480;

uint32_t ticks = 0;
uint32_t framecnt = 0;

SDL_Rect viewport = {.x = 0, .y = 0, .w = 640, .h = 480};

struct mousecoord {
	uint8_t valid;
	uint8_t x;
	uint8_t y;
	uint8_t buttons;
};

struct mousecoord mousecoords;

// map editor specific

int menu_cnt = -1;
int menu_foc = -1;
int menu_sel = -1;

SDL_Rect boxify() {

	double ratio = (win_w * 0.75) / win_h;

	unsigned int true_w = (ratio >= 1) ? win_h / 0.75 : win_w;
	unsigned int true_h = (ratio >= 1) ? win_h : win_w * 0.75;

	viewport = (SDL_Rect){.x = (win_w - true_w) / 2, .y = (win_h - true_h) / 2, .w = true_w, .h = true_h};
	return viewport;
}

struct mousecoord get_mouse() {

	return mousecoords;
}

int fillrect(uint32_t color, uint8_t x, uint8_t y, uint8_t w, uint8_t h) {

	SDL_Rect sr = {.x = x, .y = y, .w = w, .h = h};
	return SDL_FillRect(scs,&sr,color);
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

int sdl_ui_menu(int choice_c, char** choice_v) {

	menu_cnt = choice_c;

	for (int i=0; i < choice_c; i++) {
		s_addstr(choice_v[i],48,80+(16*i));
	}

	struct mousecoord m = get_mouse();
	if ((m.y >= 80) && (m.y < (80 + (16*choice_c))) ) menu_foc = ((m.y - 80) / 16); else menu_foc = -1;

	for (int i=0; i < choice_c; i++) {

		if (menu_foc == i) {
			s_addstr(">",32,80+(16*i));
		} else {
			spr(40,32,80+(16*i),1,2);
		}

	}
	return menu_sel;
}

int sdl_ui_mode = 0;

int sdl_ui_update(void) {

	//fillrect(0,192,0,64,16);
	//char frames[9];
	//snprintf(frames,9,"%08.2f",(double)framecnt/60);
	//s_addstr(frames,192,0);

	switch(sdl_ui_mode) {


		case 0: {
				int r = sdl_ui_menu(4,(char* []){"menu item 1","menu item 2","menu item 3","menu item 4"});
				if (r == 0) sdl_ui_mode = 1;
				break; }
		case 1: {
				break; }
		case 2: {
				break; }
		case 3: {
				break; }
		case 4: {
				break; }
	}


}

int sdl_ui_main(void) {

	int r = SDL_Init(SDL_INIT_VIDEO);
	if (r != 0) {
		fprintf(stderr,"Unable to start SDL video: %s\n",SDL_GetError()); exit(1); }

	atexit(SDL_Quit);

	mainwin = SDL_CreateWindow("snescityeditor",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,win_w,win_h,SDL_WINDOW_RESIZABLE);

	if (!mainwin) {
		fprintf(stderr,"Unable to create window: %s\n",SDL_GetError()); exit(1); }

	ren = SDL_CreateRenderer(mainwin,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	//SDL_RenderSetLogicalSize(ren, 256, 224);

	if (!ren) {
		fprintf(stderr,"Unable to create renderer: %s\n",SDL_GetError()); exit(1); }


	scs = SDL_CreateRGBSurface(0,256,224,32,0x00FF0000,0x0000FF00,0x000000FF,0xFF000000);

	if (!scs) {
		fprintf(stderr,"Unable to create surface: %s\n",SDL_GetError()); exit(1); }

	tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,256,224);

	if (!tex) {
		fprintf(stderr,"Unable to create texture: %s\n",SDL_GetError()); exit(1); }

	tsurf = IMG_ReadXPMFromArray(textures_xpm);

	//background

	for (int iy=0; iy < 14; iy++) {
		for (int ix=0; ix < 16; ix++) {
			spr(68,ix*16,iy*16,2,2);
		}
	}

	// title

	spr(0,16,16,2,2);
	for (int ix=2; ix<14; ix++) spr(2,ix*16,16,2,2);
	spr(4,224,16,2,2);

	spr(32,16,32,2,2);
	for (int ix=2; ix<14; ix++) spr(34,ix*16,32,2,2);
	spr(36,224,32,2,2);

	spr(64,16,48,2,2);
	for (int ix=2; ix<14; ix++) spr(66,ix*16,48,2,2);
	spr(68,224,48,2,2);

	s_addstr_c("SNESCITYEDITOR",32);

	//menu backdrop

	spr(6,16,64,2,2);
	for (int ix=2; ix<14; ix++) spr(8,ix*16,64,2,2);
	spr(10,224,64,2,2);

	for (int iy=5; iy < 12; iy++) {
		spr(38,16,iy*16,2,2);
		for (int ix=2; ix<14; ix++) spr(40,ix*16,iy*16,2,2);
		spr(42,224,iy*16,2,2);
	}

	spr(70,16,192,2,2);
	for (int ix=2; ix<14; ix++) spr(72,ix*16,192,2,2);
	spr(74,224,192,2,2);
	int quit = 0;

	int rerender = 0;

	while (!quit) {

		SDL_Event lastevent;

		while (SDL_PollEvent(&lastevent) != 0) {

			switch(lastevent.type) {
				case SDL_MOUSEMOTION:
					mousecoords.x = (lastevent.motion.x - viewport.x) * 256 / viewport.w;
					mousecoords.y = (lastevent.motion.y - viewport.y) * 224 / viewport.h;

					mousecoords.valid = 1;
					if ((mousecoords.x < 0) || mousecoords.y >= 256) mousecoords.valid = 0;
					if ((mousecoords.y < 0) || mousecoords.y >= 224) mousecoords.valid = 0;
					break;
				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:

					if (lastevent.button.state == SDL_PRESSED)
						mousecoords.buttons |= (1 << (lastevent.button.button));
					else 
						mousecoords.buttons &= ~ (1 << (lastevent.button.button));
					break;
				case SDL_WINDOWEVENT:
					if (lastevent.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {rerender = 1; break; }
					if (lastevent.window.event != SDL_WINDOWEVENT_SIZE_CHANGED) break;

					SDL_GetWindowSize(mainwin,&win_w,&win_h);
					SDL_Rect newsize = boxify();
					SDL_RenderSetViewport(ren, &newsize);
					rerender = 1;
					break;
				case SDL_QUIT:
					quit = 1;
			}

		}

		if (rerender) {

			SDL_UpdateTexture(tex, NULL, scs->pixels, scs->pitch);
			SDL_RenderClear(ren);
			SDL_RenderCopy(ren, tex, NULL, NULL);
			SDL_RenderPresent(ren);

			rerender = 0;
		}

		// this elaborate mechanism tries to call sdl_ui_update and rerender
		// everything at least 60 times per second.
		uint32_t cticks = SDL_GetTicks();
		uint32_t nframecnt = (cticks * 60)/1000 ;
		if (nframecnt == framecnt) {

			uint32_t nxtticks = ((framecnt+1) * 1000) / 60;
			SDL_Delay(nxtticks - cticks);
		} else {
			while (nframecnt > framecnt) {

				sdl_ui_update();
				framecnt = nframecnt;
			}
			rerender = 1;
		}


	}




	/*s_addstr(
	//               123456789012345678901234
	"  THIS GRAPHICAL UI IS  \n"
	"  NOT YET IMPLEMENTED.\n"
	"BUT YOU CAN ALREADY SEE\n"
	"THE SNES-LIKE NON-SQUARE\n"
	"PIXEL SCALING AND SUCH.\n"
	"\n",32,80);

	 */


	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(mainwin);

	return 0;
}
