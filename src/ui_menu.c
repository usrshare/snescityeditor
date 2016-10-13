#include "ui_menu.h"
#include <limits.h>

#include "defines.h"
#include "snescity.h"

#include "sdl_ui.h"

int sdl_ui_mode = 0;
int sdl_ui_operation = -1;

char city_fname[PATH_MAX], map_fname[PATH_MAX];

int citynum = 0;

uint16_t citytiles[CITYWIDTH*CITYHEIGHT];

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

enum ui_operations {
	OP_CREATENEW,
	OP_IMPORT,
	OP_FIXCKSUM
};

void ui_updatefunc(void) {

	//fillrect(0,192,0,64,16);
	//char frames[9];
	//snprintf(frames,9,"%08.2f",(double)framecnt/60);
	//s_addstr(frames,192,0);

	switch(sdl_ui_mode) {


		case UI_MAINMENU: {
				// main menu
				
				fillrect(0x140c1c,32,80,192,112);

				int r = sdl_ui_menu(3,(char* []){"Import map into SRAM","Fix SRAM check sum","Exit"});
				if (r >= 0) { sdl_ui_mode = 1; sdl_ui_operation = r; }
				if (r == 2) exit(0);
				break; }
		case UI_DROPSRAM: {
				// drop city file here
				spr(13,24,72,1,1);
				spr(15,224,72,1,1);
				for (int iy=10; iy < 24; iy++) {
					spr(29,24,iy*8,1,1);
					spr(31,224,iy*8,1,1);
				}
				for (int ix=4; ix < 28; ix++) {
					spr(14,ix*8,72,1,1);
					spr(46,ix*8,192,1,1);
				}
				spr(45,24,192,1,1);
				spr(47,224,192,1,1);

				fillrect(0xd2aa99,32,80,192,112);

				s_addstr_c("Drag your SRAM file",80,1);
				s_addstr_c("into this window",96,1);

				if (getdrop(city_fname,PATH_MAX))
					sdl_ui_mode = (sdl_ui_operation == 1 ? 4 : 2);

				break; }
		case UI_SELCITY: {

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
				char city1[17], city2[17];

				int r = describe_cities(city_fname,city1,city2);

				if (r != 0) sdl_ui_mode = 6;

				r = sdl_ui_menu(3,(char* []){city1,city2,"Back"});
				if (r >= 0) { citynum = r; sdl_ui_mode = 3; }
				if (r == 2) sdl_ui_mode = 0;
				// select city 1 or 2
				break; }
		case UI_DROPPNG: {
				// drop png file here
				spr(13,24,72,1,1);
				spr(15,224,72,1,1);
				for (int iy=10; iy < 24; iy++) {
					spr(29,24,iy*8,1,1);
					spr(31,224,iy*8,1,1);
				}
				for (int ix=4; ix < 28; ix++) {
					spr(14,ix*8,72,1,1);
					spr(46,ix*8,192,1,1);
				}
				spr(45,24,192,1,1);
				spr(47,224,192,1,1);

				fillrect(0xd2aa99,32,80,192,112);
				s_addstr_c("Drag your PNG map file",80,1);
				s_addstr_c("into this window",96,1);

				if (getdrop(map_fname,PATH_MAX))
					sdl_ui_mode = 4;

				break; }
		case UI_PROCESSING: {
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

				s_addstr_c("Now processing...",160,0);

				int r = 0;
				switch (sdl_ui_operation) {

					case 0:
						r = png2city(city_fname,map_fname,citynum,1,0);
						break;
					case 1:
						r = fixsram(city_fname);
						break;
				}
				sdl_ui_mode = r ? 6 : 5;
				// working...
				break; }
		case UI_SUCCESS: {
				
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

				s_addstr_c("success.",112,0);
				
				s_addstr_c("close the window to exit.",128,0);

				// operation successful
				break; }
		case UI_ERROR: {

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
				
				s_addstr_c("error.",112,0);
				
				s_addstr_c(city_lasterror,136,0);
				
				
				s_addstr_c("close the window to exit.",160,0);

				// error
				break; }
	}


}
