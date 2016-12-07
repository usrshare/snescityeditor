#include "ui_menu.h"
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "defines.h"
#include "snescity.h"
#include "pngmap.h"
#include "sdl_ui.h"
#include "llist.h"

char city_fname[PATH_MAX], map_fname[PATH_MAX];

char newfile[PATH_MAX];
char cityname[9];

int citynum = 0;

uint16_t citytiles[CITYWIDTH*CITYHEIGHT];

int transform_city = 0;
uint16_t citytiles_trans[CITYWIDTH*CITYHEIGHT];

// editor-related parameters

int16_t edit_scrollx = -1;
int16_t edit_scrolly = -1;

enum brushtypes {
	BT_EMPTY,
	BT_WATER,
	BT_WATERSHIP,
	BT_WATERCOAST,
	BT_FOREST,
	BT_ROAD,
	BT_RAIL,
	BT_TILE,
	BT_PICKER,
	BT_COUNT
};

uint8_t brushtype = 0;
uint16_t curtile = 0; //current tile
uint8_t tilepalette = 0; //tile palette mode?
uint8_t smoothmode = 1; //smooth mode enabled?

int16_t holddiff_x = 0, holddiff_y = 0;
int16_t scrdiff_x = 0, scrdiff_y = 0;

uint8_t oldtilepos_x = 0, oldtilepos_y = 0;

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
	UI_SAVEMENU,
	UI_PROCESSING,
	UI_SUCCESS,
	UI_ERROR
};

enum ui_operations {
	OP_LOADING, // no map loaded yet.
	OP_CREATENEW, // create new SRAM based on map
	OP_MAP_TO_SRAM, // save map into existing SRAM
	OP_MAP_TO_PNG, // save map as PNG
	OP_EXIT
};

enum import_modes {
	I_NOIMPROVE,
	I_NOCOAST,
	I_SIMPLECOAST,
	I_THICKCOAST,
	I_TESTING,
	I_COUNT
};

int sdl_ui_mode = 0;
int sdl_ui_operation = OP_LOADING;

int import_mode = 0;

int button(uint8_t spr, const char* text, uint8_t x, uint8_t y, uint8_t w) {

	box(spr,x,y,w,2,1);
	s_addstr_cx(text,x + (4*w),y+4,1);

	if (hold(x,y,w*8,16,1)) box(61,x,y,w,2,1);

	if (click(x,y,w*8,16,1)) return 1; else return 0;
}

int editbutton(uint8_t s, uint8_t x, uint8_t y) {

	if (hold(x,y,16,16,1)) {
		spr(50,x,y,2,2);
		spr(s,x+4,y+4,1,1);
	} else {
		spr(48,x,y,2,2);
		spr(s,x+4,y+2,1,1);
	}

	if (click(x,y,16,16,1)) return 1; else return 0;
}

uint8_t citysprite(uint16_t tile) {

	switch(tile) {
		case 0x00: return 16;
		case 0x01:
		case 0x02: return 71; //water
		case 0x03: return 52; //water with path for ships
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

		case 0x30: return 0xf7; //road bridge H
		case 0x31: return 0xf8; //road bridge V
		case 0x32: return 0xc3;
		case 0x33: return 0xc4;
		case 0x34: return 0xca;
		case 0x35: return 0xaa;
		case 0x36: return 0xac;
		case 0x37: return 0xcc;
		case 0x38: return 0xcb;
		case 0x39: return 0xba;
		case 0x3a: return 0xab;
		case 0x3b: return 0xbc;
		case 0x3c: return 0xbb;

		case 0x70: return 0xf9; //rail bridge H
		case 0x71: return 0xfa; //rail bridge V
		case 0x72: return 0xc5;
		case 0x73: return 0xc6;
		case 0x74: return 0xc7;
		case 0x75: return 0xa7;
		case 0x76: return 0xa9;
		case 0x77: return 0xc9;
		case 0x78: return 0xc8;
		case 0x79: return 0xb7;
		case 0x7a: return 0xa8;
		case 0x7b: return 0xb9;
		case 0x7c: return 0xb8;
		case 0x7d: return 0xc1; //road V + rail H
		case 0x7e: return 0xc2; //road H + rail V

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

struct _citycoord {
	uint8_t x;
	uint8_t y;
};

typedef struct _citycoord citycoord;

LLIST_DECLARE(citycoord);

bool tile_water(uint8_t x, uint8_t y) {
	if ( (x >= CITYWIDTH) || (y >= CITYHEIGHT) ) return false;
	uint16_t tile = citytiles[y * CITYWIDTH + x];
	if ((tile >= 1) && (tile <= 0x13)) return true;
	return false;
}
bool tile_forest(uint8_t x, uint8_t y) {
	if ( (x >= CITYWIDTH) || (y >= CITYHEIGHT) ) return false;
	uint16_t tile = citytiles[y * CITYWIDTH + x];
	if ((tile >= 0x14) && (tile <= 0x25)) return true;
	return false;
}
bool tile_road(uint8_t x, uint8_t y) {
	if ( (x >= CITYWIDTH) || (y >= CITYHEIGHT) ) return false;
	uint16_t tile = citytiles[y * CITYWIDTH + x];
	if ((tile >= 0x32) && (tile <= 0x3c)) return true;
	return false;
}
bool tile_rail(uint8_t x, uint8_t y) {
	if ( (x >= CITYWIDTH) || (y >= CITYHEIGHT) ) return false;
	uint16_t tile = citytiles[y * CITYWIDTH + x];
	if ((tile >= 0x72) && (tile <= 0x7c)) return true;
	return false;
}

uint16_t add_water(uint16_t c1, uint16_t c2) {

	if (c1 == 0) return c2; if (c2 == 0) return c1;
	if ((c1 >= 1) || (c1 <= 3)) return c1;
	if ((c2 >= 1) || (c2 <= 3)) return c2;
}

uint16_t tile_equality[] = {
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x04,0x05,0x06,0x07,
	0x08,0x09,0x0a,0x0b,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x14,0x15,0x16,
	0x17,0x18,0x19,0x1a,0x1b,0x1c,0x26,0x27};

#define EQTILE_CNT (sizeof(tile_equality) / sizeof(*tile_equality))

bool tile_equal(uint16_t t1, uint16_t t2) {

	//compares two tiles, considering alternate water/forests equal to regular ones
	if ((t1 >= EQTILE_CNT) || (t2 >= EQTILE_CNT)) return (t1 == t2);
	return (tile_equality[t1] == tile_equality[t2]); 

}

void edit_spreadroad(uint8_t x, uint8_t y) {

	put_proper_road(citytiles,x,y);

	if (tile_road(x,y-1)) put_proper_road(citytiles,x,y-1); 
	if (tile_road(x-1,y)) put_proper_road(citytiles,x-1,y); 
	if (tile_road(x+1,y)) put_proper_road(citytiles,x+1,y); 
	if (tile_road(x,y+1)) put_proper_road(citytiles,x,y+1); 

}

void edit_spreadrail(uint8_t x, uint8_t y) {

	put_proper_rail(citytiles,x,y);

	if (tile_rail(x,y-1)) put_proper_rail(citytiles,x,y-1); 
	if (tile_rail(x-1,y)) put_proper_rail(citytiles,x-1,y); 
	if (tile_rail(x+1,y)) put_proper_rail(citytiles,x+1,y); 
	if (tile_rail(x,y+1)) put_proper_rail(citytiles,x,y+1); 

}

void edit_spreadfix(uint8_t x, uint8_t y) {

	LLIST_NEW(citycoord, queue);

	if (y>0) llist_citycoord_push(&queue, (citycoord){x,y-1}); //N
	if (x>0) llist_citycoord_push(&queue, (citycoord){x-1,y}); //W
	if (x<(CITYWIDTH-1)) llist_citycoord_push(&queue, (citycoord){x+1,y}); //E
	if (y < (CITYHEIGHT-1)) llist_citycoord_push(&queue, (citycoord){x,y+1}); //S

	if ((y>0) && (x>0)) llist_citycoord_push(&queue, (citycoord){x-1,y-1}); //NW
	if ((y>0) && (x<(CITYWIDTH-1))) llist_citycoord_push(&queue, (citycoord){x+1,y-1}); //NE
	if ((y < (CITYHEIGHT-1)) && (x>0)) llist_citycoord_push(&queue, (citycoord){x-1,y+1}); //SW
	if ((y < (CITYHEIGHT-1)) && (x < (CITYWIDTH-1)) ) llist_citycoord_push(&queue, (citycoord){x+1,y+1}); //SE

	citycoord n;
	while (LLIST_EXISTS(queue)) {

		n = LLIST_POP(citycoord,queue);

		uint16_t tile = citytiles[n.y * CITYWIDTH + n.x];

		if ( (tile == 0) || ((tile >= 4) && (tile <= 0x13)) ) 
			city_water_spread(citytiles,n.x,n.y, tile, smoothmode ? 4 : 1);
		else if (tile_forest(n.x,n.y)) {
			city_fix_forests(citytiles,n.x,n.y);
		}

		if (!tile_equal(citytiles[n.y * CITYWIDTH + n.x],tile)) {

			llist_citycoord_push(&queue,(citycoord){x,y-1});
			llist_citycoord_push(&queue,(citycoord){x-1,y});
			llist_citycoord_push(&queue,(citycoord){x+1,y});
			llist_citycoord_push(&queue,(citycoord){x,y+1});

			llist_citycoord_push(&queue,(citycoord){x-1,y-1});
			llist_citycoord_push(&queue,(citycoord){x+1,y-1});
			llist_citycoord_push(&queue,(citycoord){x-1,y+1});
			llist_citycoord_push(&queue,(citycoord){x+1,y+1});
		}

	}
}

typedef int (*line_cb)(uint16_t* map, uint8_t x, uint8_t y, void* param);

int plotcb_spreadwater(uint16_t* map, uint8_t x, uint8_t y, void* param) {

	edit_spreadfix(x,y);
	return 0;
}

struct plot_param {
	uint16_t tile;
	line_cb next;
	void* nextparam;
};

int plot_cb(uint16_t* map, uint8_t x, uint8_t y, void* param) {
	
	struct plot_param* ctx = param;
	map[y * CITYWIDTH + x] = ctx->tile;
	if (ctx->next) return ctx->next(map,x,y,ctx->nextparam);
	return 0;
}


int lineofsight(uint16_t* map, uint8_t sx, uint8_t sy, uint8_t tx, uint8_t ty, line_cb cb, void* cbparam) {

	// this function draws a line from (sx,sy) to (tx,ty), calls the callback
	// function with the x and y values and returns the highest value
	// it encountered.

	int dx = (tx - sx);
	int dy = (ty - sy);

	int ix = dx ? (dx / abs(dx)) : 0;
	int iy = dy ? (dy / abs(dy)) : 0;

	dx = abs(dx) << 1; dy = abs(dy) << 1;

	int res = 0, maxres = 0;
			
	cb(map,sx,sy,cbparam);

	if (dx >= dy) {

		int error = (dy - (dx >> 1));

		while (sx != tx) {

			if ((error >= 0) && (error || (ix > 0)))
			{
				error -= dx;
				sy += iy;
			}

			error += dy;
			sx += ix;

			res = cb(map,sx,sy,cbparam);
			if (res > maxres) maxres=res;
		}

	} else {

		int error = (dx - (dy >> 1));

		while (sy != ty) {

			if ((error >= 0) && (error || (iy > 0)))
			{
				error -= dy;
				sx += ix;
			}

			error += dx;
			sy += iy;

			res = cb(map,sx,sy,cbparam);
			if (res > maxres) maxres=res;
		}
	}

	return maxres;
}

void plot_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t tile, line_cb func) {

	struct plot_param pp = {.tile = tile, .next = func, .nextparam = 0};
	lineofsight(citytiles,x1,y1,x2,y2,plot_cb,&pp);
	return;
}


void ui_updatefunc(void) {

	//fillrect(0,192,0,64,16);
	//char frames[9];
	//snprintf(frames,9,"%08.2f",(double)framecnt/60);
	//s_addstr(frames,192,0);

	switch(sdl_ui_mode) {

		case UI_MAINMENU: {
					  // main/load menu

					  box(6,16,64,28,18,1);

					  enum loadmenuops {
						  MM_EMPTY,
						  MM_LOAD_PNG,
						  MM_LOAD_SRAM,
						  MM_EXIT
					  };

					  int r = sdl_ui_menu(4,(char* []){"Open map editor","Load map from PNG","Load map from SRAM","Exit"},80);

					  if (r == MM_EMPTY) { sdl_ui_mode = UI_EDITOR; }
					  if (r == MM_LOAD_PNG) { sdl_ui_mode = UI_DROPPNG; cityname[0] = 0; sdl_ui_operation = OP_LOADING; }
					  if (r == MM_LOAD_SRAM) { sdl_ui_mode = UI_DROPSRAM; cityname[0] = 0; sdl_ui_operation = OP_LOADING; }
					  if (r == MM_EXIT) exit(0);

					  break; }
		case UI_SAVEMENU: {
					  box(6,16,64,28,18,1);

					  enum savemenuops {
						  SM_SAVE_PNG,
						  SM_NEW_SRAM,
						  SM_EXIST_SRAM,
						  SM_BACK
					  };

					  int r = sdl_ui_menu(4,(char* []){"Save map as PNG","Save into new SRAM","Save into existing SRAM","Continue editing"},80);

					  if (r == SM_SAVE_PNG) { sdl_ui_operation = OP_MAP_TO_PNG; sdl_ui_mode = UI_PROCESSING; }
					  if (r == SM_NEW_SRAM) { sdl_ui_operation = OP_CREATENEW; sdl_ui_mode = UI_PROCESSING; }
					  if (r == SM_EXIST_SRAM) { sdl_ui_operation = OP_MAP_TO_SRAM; sdl_ui_mode = UI_DROPSRAM; }
					  if (r == SM_BACK) { sdl_ui_mode = UI_EDITOR; }

					  break; }
		case UI_DROPSRAM: {
					  // drop city file here

					  box(13,16,64,28,18,1);

					  s_addstr_c("Drag your SRAM file",80,1);
					  s_addstr_c("into this window",96,1);

					  if (getdrop(city_fname,PATH_MAX))
						  sdl_ui_mode = UI_SELCITY;

					  if (button(58,"BACK",176,184,7)) sdl_ui_mode = UI_MAINMENU;

					  break; }
		case UI_SELCITY: {

					 box(6,16,64,28,18,1);

					 char city1[17], city2[17];

					 int r = describe_cities(city_fname,city1,city2);

					 if (r != 0) sdl_ui_mode = UI_ERROR;

					 r = sdl_ui_menu(3,(char* []){city1,city2,"Back"},80);
					 if (r >= 0) {
						 citynum = r;

						 if (sdl_ui_operation == OP_LOADING) {

							 loadsramcity(city_fname,citytiles,citynum,cityname);
							 sdl_ui_mode = UI_SAVEMENU;

						 } else {

							 sdl_ui_mode = UI_PROCESSING;
						 }
					 }

					 if (r == 2) sdl_ui_mode = UI_MAINMENU;
					 // select city 1 or 2
					 break; }
		case UI_DROPPNG: {

					 box(13,16,64,28,18,1);
					 // drop png file here

					 s_addstr_c("Drag your PNG map file",80,1);
					 s_addstr_c("into this window",96,1);

					 if (getdrop(map_fname,PATH_MAX)) {
						 int r = read_png_map(map_fname,citytiles);
						 transform_city = 1;
						 sdl_ui_mode = UI_OPTIONS;
						 import_mode = 0;
					 }

					 if (button(58,"BACK",176,184,7)) sdl_ui_mode = UI_MAINMENU;

					 break; }
		case UI_EDITOR: {

					drawcity(edit_scrollx,edit_scrolly);

					fillspr(1,0,0,32,1);

					uint8_t citytop = tilepalette ? 32 : 16;
					uint8_t cityhgt = tilepalette ? 192 : 208;

					if (tilepalette) {

						fillspr(1,0,8,32,1);
						fillspr(1,0,16,32,1);
						fillspr(17,0,24,32,1);
					} else {

						fillspr(17,0,8,32,1);
					}

					spr(2,0,0,2,2); //current tile box

					uint8_t ctilespr = 0;

					switch(brushtype) {
						case BT_EMPTY: ctilespr = 32; break;
						case BT_WATER: ctilespr = 33; break;
						case BT_WATERSHIP: ctilespr = 52; break;
						case BT_WATERCOAST: ctilespr = 53; break;
						case BT_FOREST: ctilespr = 34; break;
						case BT_ROAD: ctilespr = 35; break;
						case BT_RAIL: ctilespr = 36; break;
						case BT_TILE: ctilespr = citysprite(curtile); break;
						case BT_PICKER: ctilespr = 81; break;
					}
					spr(ctilespr,4,3,1,1); //current tile

					if (tilepalette) {

						uint16_t tilelist[] = {
							// 1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17
							0x00,0x04,0x05,0x06,0x14,0x15,0x16,0x32,0x35,0x3a,0x36,0x30,0x72,0x75,0x7a,0x76,0x7d,
							0x02,0x07,0x01,0x08,0x17,0x18,0x19,0x33,0x3b,0x3c,0x39,0x31,0x73,0x7b,0x7c,0x79,0x7e,
							0x03,0x09,0x0a,0x0b,0x1a,0x1b,0x1c,0x00,0x34,0x38,0x37,0x00,0x70,0x74,0x78,0x77,0x71,
						};

#define PWIDTH 17

						for (int i = 0; i < (sizeof(tilelist) / sizeof(tilelist[0])); i++)
							spr(citysprite(tilelist[i]), 8*(i%PWIDTH)+24, 4 + (i/PWIDTH)*8, 1, 1);

						if (hover(24,4,PWIDTH*8,24)) {

							int16_t tilepos_x = (mousecoords.x - 24) & 0xFFF8;
							int16_t tilepos_y = (mousecoords.y - 4) & 0xFFF8;
							spr(80, tilepos_x + 24, tilepos_y + 4, 1,1);
						}

						if (hold(24,4,PWIDTH*8,24,1)) {

							int16_t tilepos_x = (mousecoords.x - 24) & 0xFFF8;
							int16_t tilepos_y = (mousecoords.y - 4) & 0xFFF8;

							int16_t tilenum = (tilepos_x / 8) + (PWIDTH * (tilepos_y/8));
							curtile = tilelist[tilenum];

						}
						if (click(24,4,PWIDTH*8,24,1)) {

							tilepalette = 0;
						}
#undef PWIDTH
						if (editbutton(41,240,0)) tilepalette = 0;

					} else {

						if (editbutton(32,16,0)) brushtype = BT_EMPTY;  //ground
						if (editbutton(33,32,0)) brushtype = BT_WATER; //water
						if (editbutton(34,48,0)) brushtype = BT_FOREST; //forest
						if (editbutton(35,64,0)) brushtype = BT_ROAD; //road
						if (editbutton(36,80,0)) brushtype = BT_RAIL; //rail
						if (editbutton(37,96,0)) { brushtype = BT_TILE; tilepalette = 1; } //custom tile


						if (editbutton(smoothmode ? 69 : 68,112,0)) smoothmode = !smoothmode; //smooth mode
						if (editbutton(81,128,0)) brushtype = BT_PICKER; //tile picker
						editbutton(82,144,0); //undo?

						if (editbutton(57,192,0)) { transform_city = 1; sdl_ui_mode = UI_OPTIONS; } //options
						if (editbutton(83,208,0)) sdl_ui_mode = UI_MAINMENU; //load
						if (editbutton(84,224,0)) sdl_ui_mode = UI_SAVEMENU; //save

						if (editbutton(85,240,0)) { //exit
							sdl_ui_mode = UI_MAINMENU;
						} 

					}

					if (hover(0,citytop,255,cityhgt)) {

						int16_t tilepos_x = (mousecoords.x + edit_scrollx) & 0xFFF8;
						int16_t tilepos_y = (mousecoords.y + edit_scrolly) & 0xFFF8;

						spr(80, tilepos_x - edit_scrollx, tilepos_y - edit_scrolly, 1,1);

					}

					if (hold(0,citytop,255,cityhgt,1)) {
						//left mouse button held, paint

						int16_t tilepos_x = (mousecoords.x + edit_scrollx) / 8;
						int16_t tilepos_y = (mousecoords.y + edit_scrolly - 16) / 8;

						if ((tilepos_x >= 0) && (tilepos_x < CITYWIDTH) && (tilepos_y >= 0) && (tilepos_y < CITYHEIGHT)) {

							switch (brushtype) {
								case BT_EMPTY: plot_line(oldtilepos_x,oldtilepos_y,tilepos_x,tilepos_y,0,plotcb_spreadwater); break;
								case BT_WATER: plot_line(oldtilepos_x,oldtilepos_y,tilepos_x,tilepos_y,1,plotcb_spreadwater); break;
								case BT_WATERSHIP: citytiles[CITYWIDTH * tilepos_y + tilepos_x] = 3; break;
								case BT_WATERCOAST: citytiles[CITYWIDTH * tilepos_y + tilepos_x] = 2; break;

								case BT_FOREST: citytiles[CITYWIDTH*tilepos_y + tilepos_x] = 0x14;
									        if (smoothmode) {
											if ( (tilepos_y < (CITYHEIGHT-1)) && (citytiles[CITYWIDTH*(tilepos_y+1) + tilepos_x] == 0) ) citytiles[CITYWIDTH*(tilepos_y+1) + tilepos_x] = 0x14;
											if ( (tilepos_x < (CITYWIDTH-1)) && (citytiles[CITYWIDTH*tilepos_y + (tilepos_x+1)] == 0) ) citytiles[CITYWIDTH*tilepos_y + (tilepos_x+1)] = 0x14;
											if ( (tilepos_x < (CITYWIDTH-1)) && (tilepos_y < (CITYHEIGHT-1)) && (citytiles[CITYWIDTH*(tilepos_y+1) + (tilepos_x+1)] == 0) ) citytiles[CITYWIDTH*(tilepos_y+1) + (tilepos_x+1)] = 0x14;
										}
										city_fix_forests(citytiles,tilepos_x,tilepos_y);
										edit_spreadfix(tilepos_x,tilepos_y); break;

								case BT_ROAD: citytiles[CITYWIDTH*tilepos_y + tilepos_x] = 0x32; edit_spreadroad(tilepos_x,tilepos_y); break;
								case BT_RAIL: citytiles[CITYWIDTH*tilepos_y + tilepos_x] = 0x72; edit_spreadrail(tilepos_x,tilepos_y); break;

								case BT_TILE: citytiles[CITYWIDTH*tilepos_y + tilepos_x] = curtile; break;	      

								case BT_PICKER: brushtype = BT_TILE; curtile = citytiles[tilepos_y * CITYWIDTH + tilepos_x];
							}
							// paint.
						}

					} else if (hold(0,citytop,255,cityhgt,4)) {
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
					
					if (hover(0,citytop,255,cityhgt)) {

						int16_t tilepos_x = (mousecoords.x + edit_scrollx) / 8;
						int16_t tilepos_y = (mousecoords.y + edit_scrolly - 16) / 8;

						oldtilepos_x = tilepos_x;
						oldtilepos_y = tilepos_y;
					}

					break; }
		case UI_OPTIONS: {
					 if (transform_city) {

						 memcpy(citytiles_trans,citytiles,sizeof citytiles);

						 switch(import_mode) {
							 case I_NOCOAST:
								 city_improve(citytiles_trans,0);
								 break;
							 case I_SIMPLECOAST:
								 city_improve(citytiles_trans,1);
								 break;
							 case I_THICKCOAST:
								 city_improve(citytiles_trans,3);
								 break;
							 case I_TESTING:
								 city_improve(citytiles_trans,5);
								 break;
						 }
						 transform_city = 0;
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

							 uint16_t v = citytiles_trans[iy*CITYWIDTH+ix];
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
						 transform_city = 1;
					 }

					 if (button(58,"EDIT",176,128,7)) {
						 memcpy(citytiles,citytiles_trans,sizeof citytiles);
						 sdl_ui_mode = UI_EDITOR;
					 }

					 if (button(58,"SAVE",176,156,7)) {
						 memcpy(citytiles,citytiles_trans,sizeof citytiles);
						 sdl_ui_mode = UI_SAVEMENU;
					 }

					 if (button(58,"BACK",176,184,7)) sdl_ui_mode = UI_MAINMENU;

					 break; }
		case UI_PROCESSING: {
					    box(6,16,64,28,18,1);

					    s_addstr_c("Now processing...",160,0);

					    int r = 0;
					    switch (sdl_ui_operation) {
						    case OP_CREATENEW:
							    strcpy(newfile,cityname);
							    strcat(newfile,".srm");
							    r = write_new_city(newfile,citytiles,cityname,0);
							    break;
						    case OP_MAP_TO_SRAM: r = replace_city(city_fname,citytiles,citynum);
									 break;
						    case OP_MAP_TO_PNG: strcpy(newfile,cityname);
									strcat(newfile,".png");
									r = write_png_map (newfile, citytiles);
									break;
					    }
					    sdl_ui_mode = r ? UI_ERROR : UI_SUCCESS;
					    // working...
					    break; }
		case UI_SUCCESS: {

					 box(6,16,64,28,18,1);

					 s_addstr_c("success.",112,0);

					 if (button(58,"BACK",176,184,7)) sdl_ui_mode = UI_MAINMENU;
					 // operation successful
					 break; }
		case UI_ERROR: {

				       box(6,16,64,28,18,1);

				       s_addstr_c("error.",112,0);

				       s_addstr_c(city_lasterror,136,0);

				       if (button(58,"BACK",176,184,7)) sdl_ui_mode = UI_MAINMENU;
				       // error
				       break; }
	}


}
