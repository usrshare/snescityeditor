#include "sdl_ui.h"
#include "textures.xpm"
#include <SDL.h>
#include <SDL_image.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

// generic variables

SDL_Window* mainwin = 0; //main window
SDL_Renderer* ren = 0; //renderer
SDL_Surface* tsurf = 0; //texture surface (128x256)
SDL_Surface* scs = 0; //screen surface (256x224)

#define SCREENW 256
#define SCREENH 224

#ifdef PRESCALE
SDL_Texture* pre_tex = 0;

uint32_t output_w = 2 * SCREENW;
uint32_t output_h = 2 * SCREENH;

#else
#endif

SDL_Texture* tex = 0; //screen texture (OUTPUTWxOUTPUTH)

uint32_t start_ticks = 0;
uint32_t framecnt = 0;

int win_w = 640;
int win_h = 480;
SDL_Rect viewport = {.x = 0, .y = 0, .w = 640, .h = 480};

struct mousecoord mousecoords;

char keyboard_buffer[16];
char keys_held[16];
uint8_t kbd_open = 0;

int got_drop = 0;
char dropfilename[PATH_MAX];

// map editor specific

int menu_cnt = -1;
int menu_foc = -1;

int hover(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {

	if ((mousecoords.x >= x) && (mousecoords.x < (x+w)) &&
			(mousecoords.y >= y) && (mousecoords.y < (y+h))) return 1; else return 0;
}

int hold(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t bmask) {

	if ( (mousecoords.press_x >= x) && (mousecoords.press_x < (x+w)) &&
			(mousecoords.press_y >= y) && (mousecoords.press_y < (y+h)) &&
			(mousecoords.x >= x) && (mousecoords.x < (x+w)) &&
			(mousecoords.y >= y) && (mousecoords.y < (y+h)) &&
			(mousecoords.buttons & bmask)) {

		return 1;
	}
	return 0;
}

int click(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t bmask) {

	if ( (mousecoords.press_x >= x) && (mousecoords.press_x < (x+w)) &&
			(mousecoords.press_y >= y) && (mousecoords.press_y < (y+h)) &&
			(mousecoords.release_x >= x) && (mousecoords.release_x < (x+w)) &&
			(mousecoords.release_y >= y) && (mousecoords.release_y < (y+h)) && 
			(mousecoords.b_release & bmask) ) {

		mousecoords.press_x = 255; mousecoords.press_y = 255;
		mousecoords.release_x = 255; mousecoords.release_y = 255;
		return 1;
	}
	return 0;
}

int getdrop(char* filename, size_t fnlen) {

	if (got_drop) {

		strncpy(filename,dropfilename,fnlen);
		got_drop = 0;
		SDL_RaiseWindow(mainwin);
		return 1;
	} else return 0;
}

SDL_Rect boxify() {

	double ratio = (win_w * 0.75) / win_h;

	unsigned int true_w = (ratio >= 1) ? win_h / 0.75 : win_w;
	unsigned int true_h = (ratio >= 1) ? win_h : win_w * 0.75;

	viewport = (SDL_Rect){.x = (win_w - true_w) / 2, .y = (win_h - true_h) / 2, .w = true_w, .h = true_h};
	return viewport;
}

int strdel(char* string, char character) {

	int r=0;
	if (character == 0) return -1;

	char* fchar = 0;
	while ( (fchar = strchr(string,character)) != NULL) {
		memmove(fchar,fchar+1,strlen(fchar)); r++; }

	return r;
}
uint8_t open_kbd(uint8_t state) {
	uint8_t r = kbd_open;
	kbd_open = state;
	return r;
}

char read_kbd(void) {
	char r = keyboard_buffer[0];
	if (r) strdel(keyboard_buffer,r);
	return r;
}

struct mousecoord get_mouse() {

	struct mousecoord mc = mousecoords;
	return mc;
}

int fillrect(uint32_t color, uint8_t x, uint8_t y, uint8_t w, uint8_t h) {

	SDL_Rect sr = {.x = x, .y = y, .w = w, .h = h};
	return SDL_FillRect(scs,&sr,color);
}

int pset(uint32_t color, uint8_t x, uint8_t y) {
	return fillrect(color,x,y,1,1);
}

int spr4(uint16_t spr, int16_t x, int16_t y, uint8_t w, uint8_t h) {

	SDL_Rect sr = {.x = (spr % 32)*4, .y = (spr / 32)*8, .w = w*4, .h = h*8};
	SDL_Rect dr = {.x = x, .y = y, .w = w*4, .h = h*8};
	return SDL_BlitSurface(tsurf,&sr,scs,&dr);
}

int spr(uint16_t spr, int16_t x, int16_t y, uint8_t w, uint8_t h) {

	SDL_Rect sr = {.x = (spr % 16)*8, .y = (spr / 16)*8, .w = w*8, .h = h*8};
	SDL_Rect dr = {.x = x, .y = y, .w = w*8, .h = h*8};
	return SDL_BlitSurface(tsurf,&sr,scs,&dr);
}

const char* sprchars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ,.- abcdefghijklmnopqrstuvwxyz:?!";

int s_addstr(const char* text, uint8_t x, uint8_t y, uint8_t font) {

	uint8_t cx = x;
	uint8_t cy = y;
	const char* ct = text;

	while (*ct) {

		if (*ct == '\n') {
			cx = x;
			cy += font ? 8 : 16;
		} else {

			if (*ct != ' ') {
				char* tchar = strchr(sprchars, (font != 1) ? toupper(*ct) : (int)*ct);

				if (tchar) {
					int ci = (tchar - sprchars);
					uint16_t si;
					switch (font) {
						case 0:
							si = 96 + (ci/16) * 32 + (ci % 16);
							if (*tchar == ':') si = 0x109;
							spr(si,cx,cy,1,2);
							break;
						case 1:
							si = 400 + ci;
							spr(si,cx,cy,1,1);
							break;
						case 2:
							si = 960 + ci;
							if (*tchar == ':') si = 960 + 71;
							spr4(si,cx,cy,1,1);
							break;
					}

				}}
			cx += (font == 2) ? 4 : 8;
		}
		ct++;
	}
	return 0;
}

int s_addstr_cx(const char* text, uint8_t x, uint8_t y, uint8_t font) {
	return s_addstr(text, x - (strlen(text)*4), y, font);
}

int s_addstr_c(const char* text, uint8_t y, uint8_t font) {
	return s_addstr(text, 128 - (strlen(text)*4), y, font);
}

int sdl_ui_menu(int choice_c, char** choice_v) {

	menu_cnt = choice_c;
	int sy = 136 - (choice_c * 12);

	for (int i=0; i < choice_c; i++) {
		s_addstr(choice_v[i],48,sy+(24*i),0);
	}

	struct mousecoord m = get_mouse();
	if ((m.x >= 32) && (m.x < 224) && (m.y >= 80) && (m.y < (sy + (24*choice_c))) && ( ((m.y - sy) % 24) < 16) ) menu_foc = ((m.y - sy) / 24); else menu_foc = -1;

	for (int i=0; i < choice_c; i++) {

		if (menu_foc == i) {
			spr(9,32,sy+(24*i),1,2);
		} else {
			spr(23,32,sy+(24*i),1,1);
			spr(23,32,sy+(24*i)+8,1,1);
		}

	}

	if ((m.b_release) && (menu_foc >= 0)) return menu_foc; else return -1;
}

struct sdl_frame_cb_ctx {
	cb_noparam updatefunc;
};
	
int rerender = 0;

uint32_t sdl_frame_cb(uint32_t interval, void* param) {

	struct sdl_frame_cb_ctx* ctx = param;


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

				if (lastevent.button.state == SDL_PRESSED) {
					mousecoords.buttons |= SDL_BUTTON(lastevent.button.button);
					mousecoords.b_press |= SDL_BUTTON(lastevent.button.button); 
					mousecoords.press_x = mousecoords.x;
					mousecoords.press_y = mousecoords.y;
				}
				else {
					mousecoords.buttons &= ~ SDL_BUTTON(lastevent.button.button);
					mousecoords.b_release |= SDL_BUTTON(lastevent.button.button); 
					mousecoords.release_x = mousecoords.x;
					mousecoords.release_y = mousecoords.y;
				}
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP: {
						if ((kbd_open) && (lastevent.key.keysym.sym <= 127)) {

							char kcode = lastevent.key.keysym.sym;

							if (lastevent.key.state == SDL_PRESSED) {
								//SDL_PRESSED

								if (strchr(keys_held,kcode) == NULL) {
									if ( (strlen(keys_held) < 15) && (strlen(keyboard_buffer) < 15) ) {
										keys_held[strlen(keys_held)] = kcode;
										keyboard_buffer[strlen(keyboard_buffer)] = kcode;
										printf("Got key %d\n", kcode); }
									else printf("Keyboard buffer full.\n");
								}
							} else {
								//SDL_RELEASED
								strdel(keys_held,kcode);
							}
						}
						break; }
			case SDL_WINDOWEVENT:
					if (lastevent.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {rerender = 1; break; }
					if (lastevent.window.event != SDL_WINDOWEVENT_SIZE_CHANGED) break;

					SDL_GetWindowSize(mainwin,&win_w,&win_h);

#ifdef PRESCALE
					uint32_t new_output_w = win_w - (win_w % SCREENW);
					if (new_output_w == 0) new_output_w = SCREENW;
					uint32_t new_output_h = win_h - (win_h % SCREENH);
					if (new_output_h == 0) new_output_h = SCREENH;

					if ((output_w != new_output_w) || (output_h != new_output_h)) {
						SDL_DestroyTexture(tex);
						tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, new_output_w, new_output_h);
						if (!tex) fprintf(stderr, SDL_GetError());
						output_w = new_output_w; output_h = new_output_h;
					}
#endif

					SDL_Rect newsize = boxify();
					SDL_RenderSetViewport(ren, &newsize);
					rerender = 1;
					break;
			case SDL_DROPFILE:

					strncpy(dropfilename,lastevent.drop.file,PATH_MAX);
					got_drop = 1;
					free(lastevent.drop.file);
					break;
			case SDL_QUIT:

					exit(0);
		}

	}

	// this elaborate mechanism tries to call sdl_ui_update and rerender
	// everything at least 60 times per second.

	uint32_t cticks = SDL_GetTicks();

	uint32_t frameticks = start_ticks + (framecnt * 1000 / 60); //when should we do this frame?

	while (frameticks <= cticks) { //it's about time

		cticks = SDL_GetTicks();
		ctx->updatefunc();
		mousecoords.b_press = mousecoords.b_release = 0;
		rerender = 1;
		//fillrect(0xFF0000, (framecnt % 256), 0, 1, 16);
		//printf("frame %u performed at %u\n", framecnt, cticks);
		framecnt++;
		frameticks = start_ticks + (framecnt * 1000 / 60); //figure out the next frame instead
	}
	
	if (rerender) {

#ifdef PRESCALE
		int r = SDL_UpdateTexture(pre_tex, NULL, scs->pixels, scs->pitch);

		r = SDL_SetRenderTarget(ren, tex);
		r = SDL_RenderCopy(ren, pre_tex, NULL, NULL);
		r = SDL_SetRenderTarget(ren, NULL);
#else
		SDL_UpdateTexture(tex, NULL, scs->pixels, scs->pitch);
#endif
		SDL_RenderClear(ren);
		SDL_RenderCopy(ren, tex, NULL, NULL);
		SDL_RenderPresent(ren);

		rerender = 0;
	}
	
	return (frameticks - cticks);
}

int sdl_ui_main(cb_noparam mainfunc, cb_noparam updatefunc) {

	memset(keyboard_buffer,0,sizeof keyboard_buffer);
	memset(keys_held,0,sizeof keys_held);

	int r = SDL_Init(SDL_INIT_VIDEO);
	if (r != 0) {
		fprintf(stderr,"Unable to start SDL video: %s\n",SDL_GetError()); exit(1); }

	atexit(SDL_Quit);

#ifdef NESMODE
#define WINDOWTITLE "nescityeditor"
#else
#define WINDOWTITLE "snescityeditor"
#endif

	mainwin = SDL_CreateWindow(WINDOWTITLE,SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,win_w,win_h,SDL_WINDOW_RESIZABLE);

	if (!mainwin) {
		fprintf(stderr,"Unable to create window: %s\n",SDL_GetError()); exit(1); }

	ren = SDL_CreateRenderer(mainwin,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	//SDL_RenderSetLogicalSize(ren, 256, 224);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	if (!ren) {
		fprintf(stderr,"Unable to create renderer: %s\n",SDL_GetError()); exit(1); }


	scs = SDL_CreateRGBSurface(0,256,224,32,0x00FF0000,0x0000FF00,0x000000FF,0xFF000000);

	if (!scs) {
		fprintf(stderr,"Unable to create surface: %s\n",SDL_GetError()); exit(1); }

#ifdef PRESCALE

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"0");

	pre_tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREENW, SCREENH);

	if (!pre_tex) {
		fprintf(stderr,"Unable to create texture: %s\n",SDL_GetError()); exit(1); }

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"2");

	tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET,output_w,output_h);
#else

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"2");

	tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,SCREENW,SCREENH);

#endif


	if (!tex) {
		fprintf(stderr,"Unable to create texture: %s\n",SDL_GetError()); exit(1); }

	tsurf = IMG_ReadXPMFromArray(textures_xpm);

	mainfunc();

	start_ticks = SDL_GetTicks();
	framecnt = 0;

	struct sdl_frame_cb_ctx ctx = {.updatefunc = updatefunc};

	SDL_AddTimer( 1000 / 60, sdl_frame_cb, &ctx); 

	pause();

	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(mainwin);

	return 0;
}
