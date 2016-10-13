#include "ui_menu.h"
#include <limits.h>
#include <string.h>

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
	UI_PROCESSING,
	UI_SUCCESS,
	UI_ERROR
};

enum import_modes {
	I_NOIMPROVE,
	I_NOCOAST,
	I_SIMPLECOAST,
	I_THICKCOAST,
	I_COUNT
};

enum ui_operations {
	OP_CREATENEW,
	OP_IMPORT,
	OP_FIXCKSUM,
	OP_EXIT
};

int import_mode = 0;

int button(uint8_t spr, const char* text, uint8_t x, uint8_t y, uint8_t w) {

	box(spr,x,y,w,2,1);
	s_addstr_cx(text,x + (4*w),y+4,1);

	if (hold(x,y,w*8,16)) box(61,x,y,w,2,1);

	if (click(x,y,w*8,16)) return 1; else return 0;
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

					  int r = sdl_ui_menu(4,(char* []){"New SRAM from map","Load map into SRAM","Fix SRAM check sum","Exit"},80);
					  if (r == OP_CREATENEW) { sdl_ui_mode = UI_DROPPNG; sdl_ui_operation = r; }
					  if (r >= OP_IMPORT) { sdl_ui_mode = UI_DROPSRAM; sdl_ui_operation = r; }
					  if (r == OP_EXIT) exit(0);
					  break; }
		case UI_DROPSRAM: {
					  // drop city file here

					  box(13,16,64,28,18,1);

					  s_addstr_c("Drag your SRAM file",80,1);
					  s_addstr_c("into this window",96,1);

					  if (getdrop(city_fname,PATH_MAX))
						  sdl_ui_mode = (sdl_ui_operation == OP_FIXCKSUM ? UI_PROCESSING : UI_SELCITY);

					  if (button(58,"BACK",176,192,7)) sdl_ui_mode = UI_MAINMENU;

					  break; }
		case UI_SELCITY: {

					 box(6,16,64,28,18,1);

					 char city1[17], city2[17];

					 int r = describe_cities(city_fname,city1,city2);

					 if (r != 0) sdl_ui_mode = UI_ERROR;

					 r = sdl_ui_menu(3,(char* []){city1,city2,"Back"},80);
					 if (r >= 0) { citynum = r; sdl_ui_mode = UI_DROPPNG; }
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

					 if (button(58,"BACK",176,192,7)) sdl_ui_mode = UI_MAINMENU;

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
						 }
						 reload_city = 0;
					 }

					 box(13,16,64,28,18,1);
					 box(10,32,80,17,15,1);

					 strcpy(newfile,map_fname);
					 strcat(newfile,".srm");
					 find_png_filename(map_fname,cityname);

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
						 "NONE","SIMPLE","COAST 1","COAST 2"
					 };

					 if (button(58,import_desc[import_mode],176,96,7)) {
						 import_mode += 1;
						 if (import_mode >= I_COUNT) import_mode = 0;
						 reload_city = 1;
					 }

					 if (button(58,"START",176,176,7)) {
						 sdl_ui_mode = UI_PROCESSING;
					 }

					 if (button(58,"BACK",176,192,7)) sdl_ui_mode = UI_MAINMENU;

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
					    }
					    sdl_ui_mode = r ? UI_ERROR : UI_SUCCESS;
					    // working...
					    break; }
		case UI_SUCCESS: {

					 box(6,16,64,28,18,1);

					 s_addstr_c("success.",112,0);

					 s_addstr_c("close the window to exit.",128,0);

					 // operation successful
					 break; }
		case UI_ERROR: {

				       box(6,16,64,28,18,1);

				       s_addstr_c("error.",112,0);

				       s_addstr_c(city_lasterror,136,0);


				       s_addstr_c("close the window to exit.",160,0);

				       // error
				       break; }
	}


}
