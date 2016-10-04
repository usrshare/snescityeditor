#include "sdl_ui.h"
#include <SDL.h>
#include <stdlib.h>

int sdl_ui_main(void) {

	int r = SDL_Init(SDL_INIT_VIDEO);
	if (r != 0) {
		fprintf(stderr,"Unable to start SDL video: %s\n",SDL_GetError()); exit(1); }

	atexit(SDL_Quit);

	SDL_Window* mainwin = SDL_CreateWindow("snescityeditor",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,640,480,SDL_WINDOW_RESIZABLE);

	if (!mainwin) {
		fprintf(stderr,"Unable to create window: %s\n",SDL_GetError()); exit(1); }

	SDL_Renderer* ren = SDL_CreateRenderer(mainwin,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(ren, 256, 224);

	if (!ren) {
		fprintf(stderr,"Unable to create renderer: %s\n",SDL_GetError()); exit(1); }


	SDL_Surface* scs = SDL_CreateRGBSurface(0,256,224,32,0x00FF0000,0x0000FF00,0x000000FF,0xFF000000);
	
	if (!scs) {
		fprintf(stderr,"Unable to create surface: %s\n",SDL_GetError()); exit(1); }

	SDL_Texture* tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,256,224);
	
	if (!tex) {
		fprintf(stderr,"Unable to create texture: %s\n",SDL_GetError()); exit(1); }

	SDL_Rect bgrect = {.x=0,.y=0,.w=256,.h=224};
	SDL_FillRect(scs, &bgrect, SDL_MapRGB(scs->format,79,40,15));

	SDL_UpdateTexture(tex, NULL, scs->pixels, scs->pitch);
	SDL_RenderClear(ren);
	SDL_RenderCopy(ren, tex, NULL, NULL);
	SDL_RenderPresent(ren);

	SDL_Event lastevent;

	int loop = 1;

	while ( (loop) && (SDL_WaitEvent(&lastevent)) ) {

		switch(lastevent.type) {
			case SDL_WINDOWEVENT:
				SDL_RenderPresent(ren);
				break;
			case SDL_QUIT:
				loop = 0;
		}
	}

	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(mainwin);

	return 0;
}
