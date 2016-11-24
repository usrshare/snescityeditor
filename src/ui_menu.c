#include "ui_menu.h"
#include <limits.h>
#include <string.h>
#include <ctype.h>

#include "defines.h"
#include "snescity.h"
#include "pngmap.h"
#include "sdl_ui.h"

int sdl_ui_mode = 0;
int sdl_ui_operation = -1;

char city_fname[PATH_MAX], map_fname[PATH_MAX];

char newfile[PATH_MAX];
char cityname[9];

int citynum = 0;

int reload_city = 0;
uint16_t citytiles[CITYWIDTH*CITYHEIGHT];

int16_t edit_scrollx = -1;
int16_t edit_scrolly = -1;

int16_t holddiff_x = 0, holddiff_y = 0;
int16_t scrdiff_x = 0, scrdiff_y = 0;

void fillspr(uint8_t s, uint8_t x, uint8_t y, uint8_t w, uint8_t h) {

	for (int iy=0; iy < h; iy++)
		for (int ix=0; ix < w; ix++)
			spr(s,x+(ix*8),y+(iy*8),1,1);
}

void box(uint8_t s, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t sz) {

	spr(s,x,y,sz,sz);
	for (int ix=1; ix<(w-1); ix++)
		spr(s+sz,x+(ix*8*sz),y,sz,sz);

	spr(s+(2*sz),x+(w-1)*8*sz,y,sz,sz);

	for (int iy=1; iy<(h-1); iy++) {

		spr(s+(16*sz),x,y+(iy*8*sz),sz,sz);
		for (int ix=1; ix<(w-1); ix++)
			spr(s+(16*sz)+sz,x+(ix*8*sz),y+(iy*8*sz),sz,sz);

		spr(s+(16*sz)+(2*sz),x+(w-1)*8*sz,y+(iy*8*sz),sz,sz);

	}

	spr(s+(32*sz),x,y+(h-1)*8*sz,sz,sz);
	for (int ix=1; ix<(w-1); ix++)
		spr(s+(32*sz)+sz,x+(ix*8*sz),y+(h-1)*8*sz,sz,sz);

	spr(s+(32*sz)+(2*sz),x+(w-1)*8*sz,y+(h-1)*8*sz,sz,sz);

}

void ui_initfunc(void) {

	memset(citytiles,0,sizeof citytiles);

	//background

	for (int iy=0; iy < 14; iy++) {
		for (int ix=0; ix < 16; ix++) {
			spr(68,ix*16,iy*16,2,2);
		}
	}

	// title

	for (int iy=0; iy < 14; iy++) for (int ix=0; ix<16; ix++) spr(4,ix*16,iy*16,2,2);

	s_addstr_c("SNESCITYEDITOR",32,0);

	//menu backdrop

	box(6,16,64,28,18,1);

}

enum ui_modes {
	UI_MAINMENU,
	UI_DROPSRAM,
	UI_SELCITY,
	UI_DROPPNG,
	UI_OPTIONS,
	UI_EDITOR,
	UI_PROCESSING,
	UI_SUCCESS,
	UI_ERROR
};

enum import_modes {
	I_NOIMPROVE,
	I_NOCOAST,
	I_SIMPLECOAST,
	I_THICKCOAST,
	I_TESTING,
	I_COUNT
};

enum ui_operations {
	OP_CREATENEW,
	OP_IMPORT,
	OP_FIXCKSUM,
	OP_EXPORT,
	OP_EDITOR,
	OP_EXIT
};

int import_mode = 0;

int button(uint8_t spr, const char* text, uint8_t x, uint8_t y, uint8_t w) {

	box(spr,x,y,w,2,1);
	s_addstr_cx(text,x + (4*w),y+4,1);

	if (hold(x,y,w*8,16,1)) box(61,x,y,w,2,1);

	if (click(x,y,w*8,16,1)) return 1; else return 0;
}

int editbutton(uint8_t s, uint8_t x, uint8_t y) {

	if (hold(x,y,16,16,1)) {
		spr(171,x,y,2,2);
		spr(s,x+4,y+4,1,1);
	} else {
		spr(169,x,y,2,2);
		spr(s,x+4,y+2,1,1);
	}

	if (click(x,y,16,16,1)) return 1; else return 0;
}

uint8_t citysprite(uint16_t tile) {

	switch(tile) {
		case 0x00: return 9;
		case 0x01:
		case 0x02: return 72; //water
		case 0x03: return 57; //water with path for ships
			   //shores
		case 0x0c:
		case 0x04: return 54;
		case 0x0d:
		case 0x05: return 55;
		case 0x0e:
		case 0x06: return 56;
		case 0x0f:
		case 0x07: return 54+16;
		case 0x10:
		case 0x08: return 56+16;
		case 0x11:
		case 0x09: return 54+32;
		case 0x12:
		case 0x0a: return 55+32;
		case 0x13:
		case 0x0b: return 56+32;

		case 0x14:
		case 0x1d: return 0xad;
		case 0x15:
		case 0x1e: return 0xae;
		case 0x16:
		case 0x1f: return 0xaf;
		case 0x17:
		case 0x20: return 0xbd;
		case 0x18:
		case 0x21: return 0xbe;
		case 0x19:
		case 0x22: return 0xbf;
		case 0x1a:
		case 0x23: return 0xcd;
		case 0x1b:
		case 0x24: return 0xce;
		case 0x1c:
		case 0x25: return 0xcf;

		case 0x32: return 0xc0;
		case 0x33: return 0xc1;
		case 0x34: return 0xc2;
		case 0x35: return 0xc3;
		case 0x36: return 0xc4;
		case 0x37: return 0xc5;
		case 0x38: return 0xc6;
		case 0x39: return 0xc7;
		case 0x3a: return 0xc8;
		case 0x3b: return 0xc9;
		case 0x3c: return 0xca;

		default: return 253; //weird tile
	}
}

void drawcity(int16_t sx, int16_t sy) {


	int minx = sx/8 - 2;
	int miny = sy/8 - 2;

	int maxx = sx/8 + 33;
	int maxy = sy/8 + 27;

	for (int iy = miny; iy < maxy; iy++) {
		for (int ix = minx; ix < maxx; ix++) {

			if ((iy >= 0) && (iy < CITYHEIGHT) && (ix >= 0) && (ix < CITYWIDTH)) {
				uint8_t cspr = citysprite(citytiles[iy * CITYWIDTH + ix]);
				spr(cspr, (ix*8) - sx, 16+(iy*8) - sy,1,1);
			} else spr(254, (ix*8) - sx, 16+(iy*8) - sy,1,1); //out of bounds

		}
	}

}

void ui_updatefunc(void) {

	//fillrect(0,192,0,64,16);
	//char frames[9];
	//snprintf(frames,9,"%08.2f",(double)framecnt/60);
	//s_addstr(frames,192,0);

	switch(sdl_ui_mode) {


		case UI_MAINMENU: {
					  // main menu

					  box(6,16,64,28,18,1);

					  int r = sdl_ui_menu(6,(char* []){"New SRAM from map","Load map into SRAM","Fix SRAM check sum","Export map into PNG","TODO - Draw new map","Exit"},80);
					  if (r == OP_CREATENEW) { sdl_ui_mode = UI_DROPPNG; sdl_ui_operation = r; }
					  if (r == OP_EDITOR) { sdl_ui_mode = UI_EDITOR; }
					  if ((r == OP_IMPORT) || (r == OP_FIXCKSUM) || (r == OP_EXPORT)) { sdl_ui_mode = UI_DROPSRAM; sdl_ui_operation = r; }
					  if (r == OP_EXIT) exit(0);
					  break; }
		case UI_DROPSRAM: {
					  // drop city file here

					  box(13,16,64,28,18,1);

					  s_addstr_c("Drag your SRAM file",80,1);
					  s_addstr_c("into this window",96,1);

					  if (getdrop(city_fname,PATH_MAX))
						  sdl_ui_mode = (sdl_ui_operation == OP_FIXCKSUM ? UI_PROCESSING : UI_SELCITY);

					  if (button(58,"BACK",176,184,7)) sdl_ui_mode = UI_MAINMENU;

					  break; }
		case UI_SELCITY: {

					 box(6,16,64,28,18,1);

					 char city1[17], city2[17];

					 int r = describe_cities(city_fname,city1,city2);

					 if (r != 0) sdl_ui_mode = UI_ERROR;

					 r = sdl_ui_menu(3,(char* []){city1,city2,"Back"},80);
					 if (r >= 0) { citynum = r; sdl_ui_mode = (sdl_ui_operation == OP_EXPORT ? UI_PROCESSING : UI_DROPPNG); }
					 if (r == 2) sdl_ui_mode = UI_MAINMENU;
					 // select city 1 or 2
					 break; }
		case UI_DROPPNG: {

					 box(13,16,64,28,18,1);
					 // drop png file here

					 s_addstr_c("Drag your PNG map file",80,1);
					 s_addstr_c("into this window",96,1);

					 if (getdrop(map_fname,PATH_MAX)) {
						 reload_city = 1;
						 sdl_ui_mode = UI_OPTIONS;
						 import_mode = 0;
					 }

					 if (button(58,"BACK",176,184,7)) sdl_ui_mode = UI_MAINMENU;

					 break; }
		case UI_EDITOR: {

					drawcity(edit_scrollx,edit_scrolly);

					fillspr(25,0,0,32,1);
					fillspr(41,0,8,32,1);

					editbutton(249,0,0);  //ground
					editbutton(250,16,0); //water
					editbutton(251,32,0); //forest
					editbutton(252,48,0); //road

					editbutton(83,208,0); //load
					editbutton(84,224,0); //save

					if (editbutton(85,240,0)) { //exit
						sdl_ui_mode = UI_MAINMENU;
					} 

					if (hold(0,16,255,208,1)) {
						//left mouse button held, paint

						int16_t mappos_x = mousecoords.x + edit_scrollx;
						int16_t mappos_y = mousecoords.y + edit_scrolly;

					} else if (hold(0,16,255,208,4)) {
						//right mouse button held, scroll
						holddiff_x = (mousecoords.x - mousecoords.press_x);
						holddiff_y = (mousecoords.y - mousecoords.press_y);

						if ((holddiff_x - scrdiff_x) != 0 ) { edit_scrollx -= (holddiff_x - scrdiff_x); scrdiff_x = holddiff_x; }
						if ((holddiff_y - scrdiff_y) != 0 ) { edit_scrolly -= (holddiff_y - scrdiff_y); scrdiff_y = holddiff_y; }

						if (edit_scrollx < -64) edit_scrollx = -64; if (edit_scrollx > 96*8) edit_scrollx = 96*8;
						if (edit_scrolly < -64) edit_scrolly = -64; if (edit_scrolly > 82*8) edit_scrolly = 82*8;

					} else {
						scrdiff_x = 0; scrdiff_y = 0;
					}

					break; }
		case UI_OPTIONS: {
					 if (reload_city) {
						 int r = read_png_map(map_fname,citytiles);

						 switch(import_mode) {
							 case I_NOCOAST:
								 city_improve(citytiles,0);
								 break;
							 case I_SIMPLECOAST:
								 city_improve(citytiles,1);
								 break;
							 case I_THICKCOAST:
								 city_improve(citytiles,3);
								 break;
							 case I_TESTING:
								 city_improve(citytiles,5);
								 break;
						 }
						 reload_city = 0;
					 }

					 box(13,16,64,28,18,1);
					 box(10,32,80,17,15,1);

					 strcpy(newfile,map_fname);
					 strcat(newfile,".srm");
					 find_png_filename(map_fname,cityname);

					 for (int i=0; i < 8; i++) cityname[i] = toupper(cityname[i]);

					 uint32_t c = 0xFF0000;

					 s_addstr(cityname,40,82,1);

					 for (int iy=0; iy < CITYHEIGHT; iy++) {
						 for (int ix=0; ix < CITYWIDTH; ix++) {

							 uint16_t v = citytiles[iy*CITYWIDTH+ix];
							 if (v < pngcolor_c) c = pngcolors[v]; else c = 0xFF0000;

							 pset(c, 40 + ix, 92+iy);

						 }
					 }

					 s_addstr("MODIFY:",176,80,1);

					 box(58,176,96,7,2,1);

					 const char* import_desc[] = {
						 "NONE","SIMPLE","COAST1","COAST2","EXTRA"
					 };

					 if (button(58,import_desc[import_mode],176,96,7)) {
						 import_mode += 1;
						 if (import_mode >= I_COUNT) import_mode = 0;
						 reload_city = 1;
					 }

					 if (button(58,"START",176,156,7)) {
						 sdl_ui_mode = UI_PROCESSING;
					 }

					 if (button(58,"BACK",176,184,7)) sdl_ui_mode = UI_MAINMENU;

					 break; }
		case UI_PROCESSING: {
					    box(6,16,64,28,18,1);

					    s_addstr_c("Now processing...",160,0);

					    int r = 0;
					    switch (sdl_ui_operation) {
						    case OP_CREATENEW: r = write_new_city(newfile,citytiles,cityname,0);
								       break;
						    case OP_IMPORT: r = replace_city(city_fname,citytiles,citynum);
								    break;
						    case OP_FIXCKSUM: r = fixsram(city_fname);
								      break;
						    case OP_EXPORT: strcpy(newfile,city_fname);
								    strcat(newfile,".png");
								    city2png(city_fname,newfile,citynum);
								    break;
					    }
					    sdl_ui_mode = r ? UI_ERROR : UI_SUCCESS;
					    // working...
					    break; }
		case UI_SUCCESS: {

					 box(6,16,64,28,18,1);

					 s_addstr_c("success.",112,0);

					 s_addstr_c("close the window to exit.",128,0);

					 if (button(58,"BACK",176,184,7)) sdl_ui_mode = UI_MAINMENU;
					 // operation successful
					 break; }
		case UI_ERROR: {

				       box(6,16,64,28,18,1);

				       s_addstr_c("error.",112,0);

				       s_addstr_c(city_lasterror,136,0);


				       s_addstr_c("close the window to exit.",160,0);

				       if (button(58,"BACK",176,184,7)) sdl_ui_mode = UI_MAINMENU;
				       // error
				       break; }
	}


}
