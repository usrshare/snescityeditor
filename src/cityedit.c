// vim: sw=4
#include "cityedit.h"
#include "defines.h"
#include <stdlib.h>

enum neighbors {
    N_X = 0,
    N_N = 1,
    N_E = 2,
    N_S = 4,
    N_W = 8,
    N_NW = 16,
    N_NE = 32,
    N_SW = 64,
    N_SE = 128
};

int check_ntile_a(uint16_t* citydata, int y, int x, uint16_t* tiles) {
    //tiles is a zero-terminated array.

    uint16_t* ctile = tiles;

    if ((y < 0) || (y >= CITYHEIGHT) || (x < 0) || (x >= CITYWIDTH)) return 0;

    while (*ctile != 0) {
	if (citydata[y*CITYWIDTH+x] == *ctile) return 1;
	ctile++;
    }
    return 2;
}

int check_ntile(uint16_t* citydata, int y, int x, uint16_t tmin, uint16_t tmax) {

    return ( (y >= 0) && (y < CITYHEIGHT) && (x >= 0) && (x < CITYWIDTH) &&
	    (citydata[y*CITYWIDTH+x] >= tmin) &&
	    (citydata[y*CITYWIDTH+x] <= tmax));

}

int check_neighbors4(uint16_t* citydata, int y, int x, uint16_t tile_min, uint16_t tile_max) {

    int r = 0;
    if (check_ntile(citydata,y-1,x,tile_min,tile_max)) r |= N_N;
    if (check_ntile(citydata,y,x-1,tile_min,tile_max)) r |= N_W;
    if (check_ntile(citydata,y,x+1,tile_min,tile_max)) r |= N_E;
    if (check_ntile(citydata,y+1,x,tile_min,tile_max)) r |= N_S;
    return r;
}
int check_neighbors(uint16_t* citydata, int y, int x, uint16_t tile_min, uint16_t tile_max) {

    int r = 0;
    if (check_ntile(citydata,y-1,x,tile_min,tile_max)) r |= N_N;
    if (check_ntile(citydata,y,x-1,tile_min,tile_max)) r |= N_W;
    if (check_ntile(citydata,y,x+1,tile_min,tile_max)) r |= N_E;
    if (check_ntile(citydata,y+1,x,tile_min,tile_max)) r |= N_S;

    if (check_ntile(citydata,y-1,x-1,tile_min,tile_max)) r |= N_NW;
    if (check_ntile(citydata,y-1,x+1,tile_min,tile_max)) r |= N_NE;
    if (check_ntile(citydata,y+1,x-1,tile_min,tile_max)) r |= N_SW;
    if (check_ntile(citydata,y+1,x+1,tile_min,tile_max)) r |= N_SE;
    return r;
}

int valid_neighbors(uint16_t tile) {
    switch (tile) {

	case 4:
	case 12: return 255 & ~N_NW;
	case 5:
	case 13: return 255 & ~N_N;
	case 6:
	case 14: return 255 & ~N_NE;
	case 7: 
	case 15: return 255 & ~N_W;
	case 8:
	case 16: return 255 & ~N_E;
	case 9:
	case 17: return 255 & ~N_SW;
	case 10:
	case 18: return 255 & ~N_S;
	case 11:
	case 19: return 255 & ~N_SE;
	default: return 255;
    }
}

uint16_t improve4(int n) {

    switch(n) {

	case N_NW | N_W | N_S | N_SE:
	case N_NW | N_W | N_SW | N_S | N_SE:
	case N_NE | N_E | N_S | N_SW:
	case N_NE | N_E | N_SE | N_S | N_SW:
	case N_SW | N_W | N_N | N_NE:
	case N_SW | N_W | N_NW | N_N | N_NE:
	case N_SE | N_E | N_N | N_NW:
	case N_SE | N_E | N_NE | N_N | N_NW:
	    return 0x1; //water

	case N_E | N_SE:
	case N_S | N_SE:
	case N_S | N_E:
	case N_S | N_SE | N_E: 
	case N_S | N_SE | N_E | N_NW:
	    return 0x4; //northwestern shore

	case N_W | N_SW | N_S | N_SE:
	case N_W | N_SW | N_S | N_SE | N_E:
	case N_W | N_SW | N_S | N_SE | N_E | N_NW:
	case N_W | N_SW | N_S | N_SE | N_E | N_NE:
	case N_W | N_SW | N_S | N_E | N_NE:
	case N_SW | N_S | N_SE | N_NW:
	case N_SW | N_S | N_SE | N_NE:
	case N_SW | N_S | N_SE | N_E:
	case N_SW | N_S | N_SE:
	case N_SW | N_S | N_E:
	case N_W | N_S | N_E:
	case N_W | N_S | N_SE:
	case N_W | N_S | N_SE | N_E | N_NW:
	    return 0x5; //northern shore

	case N_SW | N_W:
	case N_SW | N_S:
	case N_S | N_W:
	case N_W | N_SW | N_S:
	case N_S | N_SW | N_W | N_NE:
	    return 0x6; //northeastern shore

	case N_NW | N_W | N_S:
	case N_W | N_SW | N_N:
	case N_N | N_NW | N_W | N_SW:
	case N_N | N_NW | N_W | N_SW | N_S:
	case N_N | N_NW | N_W | N_SW | N_S | N_NE:
	case N_N | N_NW | N_W | N_SW | N_S | N_SE:
	case N_NW | N_W | N_SW | N_S:
	case N_NW | N_W | N_SW | N_SE: 
	case N_NW | N_W | N_SW | N_NE: 
	case N_NW | N_W | N_SW: 
	case N_W | N_S | N_N:
	    return 0x8; //eastern shore

	case N_NW | N_W:
	case N_NW | N_N: 
	case N_N | N_W:
	case N_N | N_NW | N_W: 
	case N_N | N_NW | N_W | N_SE:
	    return 0xB; //southeastern shore

	case N_W | N_NW | N_N | N_NE:
	case N_W | N_NW | N_N | N_NE | N_E:
	case N_W | N_NW | N_N | N_NE | N_E | N_SW:
	case N_W | N_NW | N_N | N_NE | N_E | N_SE:
	case N_W | N_NW | N_N | N_E | N_SE:
	case N_NW | N_N | N_NE | N_E:
	case N_NW | N_N | N_NE | N_SW:
	case N_NW | N_N | N_NE | N_SE:
	case N_NW | N_N | N_NE: 
	case N_NW | N_N | N_E:
	case N_W | N_N | N_NE:
	case N_W | N_N | N_E:
	    return 0xA; //southern shore

	case N_NE | N_E:	
	case N_N | N_NE:
	case N_N | N_E:
	case N_N | N_NE | N_E: 
	case N_N | N_NE | N_E | N_SW:
	    return 0x9; //southwestern shore	

	case N_NE | N_E | N_S:
	case N_E | N_SE | N_N:
	case N_N | N_NE | N_E | N_SE:
	case N_N | N_NE | N_E | N_SE | N_S:
	case N_N | N_NE | N_E | N_SE | N_S | N_NW:
	case N_N | N_NE | N_E | N_SE | N_S | N_SW:
	case N_NE | N_E | N_SE | N_S:
	case N_NE | N_E | N_SE | N_SW:
	case N_NE | N_E | N_SE | N_NW:
	case N_NE | N_E | N_SE: 
	case N_N | N_S | N_E:
	    return 0x7; //western shore



    }

    return 0;
}

int simple_coast_fit (uint16_t* city, uint8_t ix, uint8_t iy, uint16_t v, int improve_flags) {

    int alttile = (improve_flags & 32) ? 0 : (( rand() & 1 ) ? 8 : 0); //use alternative tile?

    int n = check_neighbors4(city,iy,ix,1,0x13); //water
    n |= check_neighbors4(city,iy,ix,0x30,0x31); //bridge
    if (improve_flags & 8) n &= valid_neighbors(v);

    if (n==0) city[iy*CITYWIDTH+ix] = 0;

    if ((n == N_W) || (n == N_S) || (n == N_N) || (n == N_E)) city[iy*CITYWIDTH+ix] = 0;

    if ((n & N_W) && (n & N_S) && (n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = 1; //water

    if ((~n & N_W) && (n & N_S) && (~n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x4; //NW
    if ((n & N_W) && (n & N_S) && (~n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x5; //N
    if ((n & N_W) && (n & N_S) && (~n & N_N) && (~n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x6; //NE
    if ((n & N_W) && (n & N_S) && (n & N_N) && (~n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x8; //E
    if ((n & N_W) && (~n & N_S) && (n & N_N) && (~n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0xB; //SE
    if ((n & N_W) && (~n & N_S) && (n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0xA; //S
    if ((~n & N_W) && (~n & N_S) && (n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x9; //SW
    if ((~n & N_W) && (n & N_S) && (n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x7; //W

    return 0;
}

int city_water_spread( uint16_t* city, uint8_t ix, uint8_t iy, uint16_t v, int improve_flags) {

    int alttile = (improve_flags & 32) ? 0 : (( rand() & 1 ) ? 8 : 0); //use alternative tile?

    int n = check_neighbors4(city,iy,ix,1,3); //water
    n |= check_neighbors4(city,iy,ix,0x30,0x31); //bridge
    if (improve_flags & 8) n &= valid_neighbors(v);

    if (improve_flags & 4) {

	n = check_neighbors(city,iy,ix,1,3); //water
	n |= check_neighbors(city,iy,ix,0x30,0x31); //bridge
	if (improve_flags & 8) n &= valid_neighbors(v);

	city[iy*CITYWIDTH+ix] = (improve4(n) >= 4) ? alttile + improve4(n) : improve4(n);

    } else if (improve_flags & 2) {

	if (n == N_S) city[iy*CITYWIDTH+ix] = alttile + 0x5; //N
	if (n == N_W) city[iy*CITYWIDTH+ix] = alttile + 0x8; //E
	if (n == N_E) city[iy*CITYWIDTH+ix] = alttile + 0x7; //W
	if (n == N_N) city[iy*CITYWIDTH+ix] = alttile + 0xA; //S

	if (n == (N_S | N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x4; //NW
	if (n == (N_S | N_W)) city[iy*CITYWIDTH+ix] = alttile + 0x6; //NE
	if (n == (N_N | N_W)) city[iy*CITYWIDTH+ix] = alttile + 0xB; //SE
	if (n == (N_N | N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x9; //SW

	if (n == (N_W | N_S | N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x5; //N
	if (n == (N_N | N_S | N_W)) city[iy*CITYWIDTH+ix] = alttile + 0x8; //E
	if (n == (N_N | N_E | N_S)) city[iy*CITYWIDTH+ix] = alttile + 0x7; //W
	if (n == (N_N | N_E | N_W)) city[iy*CITYWIDTH+ix] = alttile + 0xa; //S
    } else {
	if ((~n & N_W) && (n & N_S) && (~n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x4; //NW
	if ((n & N_W) && (n & N_S) && (~n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x5; //N
	if ((n & N_W) && (n & N_S) && (~n & N_N) && (~n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x6; //NE
	if ((n & N_W) && (n & N_S) && (n & N_N) && (~n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x8; //E
	if ((n & N_W) && (~n & N_S) && (n & N_N) && (~n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0xB; //SE
	if ((n & N_W) && (~n & N_S) && (n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0xA; //S
	if ((~n & N_W) && (~n & N_S) && (n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x9; //SW
	if ((~n & N_W) && (n & N_S) && (n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x7; //W
    }
    return 0;
}

void put_proper_road(uint16_t* city, uint8_t ix, uint8_t iy) {

    int n = check_neighbors4(city,iy,ix,0x30,0x3E); //roads + road with power
    n |= check_neighbors4(city,iy,ix,0x7d,0x7e); //road+rail combo 

    switch(n) {
	case  0:
	case  2:
	case  8:
	case 10: city[iy*CITYWIDTH+ix] = ((city[iy*CITYWIDTH+ix] & 0xFFFE) == 0x30) ? 0x30 : 0x32; break;

	case  1:
	case  4:
	case  5: city[iy*CITYWIDTH+ix] = ((city[iy*CITYWIDTH+ix] & 0xFFFE) == 0x30) ? 0x31 : 0x33; break;

	case  6: city[iy*CITYWIDTH+ix] = 0x35; break;
	case  3: city[iy*CITYWIDTH+ix] = 0x34; break;
	case 12: city[iy*CITYWIDTH+ix] = 0x36; break;
	case  9: city[iy*CITYWIDTH+ix] = 0x37; break;

	case 11: city[iy*CITYWIDTH+ix] = 0x38; break;
	case  7: city[iy*CITYWIDTH+ix] = 0x39; break;
	case 13: city[iy*CITYWIDTH+ix] = 0x3B; break;
	case 14: city[iy*CITYWIDTH+ix] = 0x3A; break;
	case 15: city[iy*CITYWIDTH+ix] = 0x3C; break; 
    }
}

void put_proper_power(uint16_t* city, uint8_t ix, uint8_t iy) {

    int n = check_neighbors4(city,iy,ix,0x60,0x6E); //power lines

    switch(n) {
	case  0:
	case  2:
	case  8:
	case 10: city[iy*CITYWIDTH+ix] = ((city[iy*CITYWIDTH+ix] & 0xFFFE) == 0x60) ? 0x60 : 0x62; break;

	case  1:
	case  4:
	case  5: city[iy*CITYWIDTH+ix] = ((city[iy*CITYWIDTH+ix] & 0xFFFE) == 0x60) ? 0x61 : 0x63; break;

	case  6: city[iy*CITYWIDTH+ix] = 0x65; break;
	case  3: city[iy*CITYWIDTH+ix] = 0x64; break;
	case 12: city[iy*CITYWIDTH+ix] = 0x66; break;
	case  9: city[iy*CITYWIDTH+ix] = 0x67; break;

	case 11: city[iy*CITYWIDTH+ix] = 0x68; break;
	case  7: city[iy*CITYWIDTH+ix] = 0x69; break;
	case 13: city[iy*CITYWIDTH+ix] = 0x6B; break;
	case 14: city[iy*CITYWIDTH+ix] = 0x6A; break;
	case 15: city[iy*CITYWIDTH+ix] = 0x6C; break; 
    }
}

void put_proper_rail(uint16_t* city, uint8_t ix, uint8_t iy) {

    int n = check_neighbors4(city,iy,ix,0x70,0x7E); //railroads

    switch(n) {
	case  0:
	case  2:
	case  8:
	case 10: city[iy*CITYWIDTH+ix] = ((city[iy*CITYWIDTH+ix] & 0xFFFE) == 0x70) ? 0x70 : 0x72; break;

	case  1:
	case  4:
	case  5: city[iy*CITYWIDTH+ix] = ((city[iy*CITYWIDTH+ix] & 0xFFFE) == 0x70) ? 0x71 : 0x73; break;

	case  6: city[iy*CITYWIDTH+ix] = 0x75; break;
	case  3: city[iy*CITYWIDTH+ix] = 0x74; break;
	case 12: city[iy*CITYWIDTH+ix] = 0x76; break;
	case  9: city[iy*CITYWIDTH+ix] = 0x77; break;

	case 11: city[iy*CITYWIDTH+ix] = 0x78; break;
	case  7: city[iy*CITYWIDTH+ix] = 0x79; break;
	case 13: city[iy*CITYWIDTH+ix] = 0x7B; break;
	case 14: city[iy*CITYWIDTH+ix] = 0x7A; break;
	case 15: city[iy*CITYWIDTH+ix] = 0x7C; break; 
    }
}

void city_fix_forests (uint16_t* city, uint8_t ix, uint8_t iy) {

    int alttile = ( rand() & 1 ) ? 9 : 0; //use alternative tile?

    int n = check_neighbors4(city,iy,ix,0x14,0x25); //forest

    if (n==0) city[iy*CITYWIDTH+ix] = 0x14;

    if ((n == N_W) || (n == N_S) || (n == N_N) || (n == N_E)) city[iy*CITYWIDTH+ix] = 0x14;

    if ((n & N_W) && (n & N_S) && (n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x18; //water

    if ((~n & N_W) && (n & N_S) && (~n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x14; //NW
    if ((n & N_W) && (n & N_S) && (~n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x15; //N
    if ((n & N_W) && (n & N_S) && (~n & N_N) && (~n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x16; //NE
    if ((n & N_W) && (n & N_S) && (n & N_N) && (~n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x19; //E
    if ((n & N_W) && (~n & N_S) && (n & N_N) && (~n & N_E)) city[iy*CITYWIDTH+ix] = 0x1C; //SE
    if ((n & N_W) && (~n & N_S) && (n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x1B; //S
    if ((~n & N_W) && (~n & N_S) && (n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x1A; //SW
    if ((~n & N_W) && (n & N_S) && (n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x17; //W
}

int city_improve (uint16_t* city, int improve_flags) {

    //this option makes the map look better.

    // let's fix any coastal areas first.

    for (int iy = 0; iy < CITYHEIGHT; iy++) {
	for (int ix=0; ix < CITYWIDTH; ix++) {

	    uint16_t v = city[iy*CITYWIDTH+ix];

	    if ((improve_flags & 1) && (v == 0x00)) {

		if (!(improve_flags & 16)) 
		    city_water_spread(city,ix,iy,v,improve_flags);

	    } else if ((v >= 0x04) && (v <= 0x13)) {

		int alttile = ( rand() & 1 ) ? 8 : 0; //use alternative tile?

		int n = check_neighbors4(city,iy,ix,1,0x13); //water
		n |= check_neighbors4(city,iy,ix,0x30,0x31); //bridge

		if (n==0) city[iy*CITYWIDTH+ix] = 0;

		if ((n == N_W) || (n == N_S) || (n == N_N) || (n == N_E)) city[iy*CITYWIDTH+ix] = 0;

		if ((n & N_W) && (n & N_S) && (n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = 0x1; //water

		if ((~n & N_W) && (n & N_S) && (~n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x4; //NW
		if ((n & N_W) && (n & N_S) && (~n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x5; //N
		if ((n & N_W) && (n & N_S) && (~n & N_N) && (~n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x6; //NE
		if ((n & N_W) && (n & N_S) && (n & N_N) && (~n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x8; //E
		if ((n & N_W) && (~n & N_S) && (n & N_N) && (~n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0xB; //SE
		if ((n & N_W) && (~n & N_S) && (n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0xA; //S
		if ((~n & N_W) && (~n & N_S) && (n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x9; //SW
		if ((~n & N_W) && (n & N_S) && (n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x7; //W

	    }
	}
    }

    for (int iy = 0; iy < CITYHEIGHT; iy++) {
	for (int ix=0; ix < CITYWIDTH; ix++) {

	    uint16_t v = city[iy*CITYWIDTH+ix];

	    if ( ((improve_flags & 3) == 3) && ((v >= 0x04) && (v <= 0x13)) ) {

		simple_coast_fit(city,ix,iy,v,improve_flags);
	    }

	    if ((v >= 0x14) && (v <= 0x25)) {

		// next step: proper forests.
		city_fix_forests (city, ix, iy);


	    } else if ((v >= 0x30) && (v <= 0x31)) {

		//this fixes bridges.

		int n = check_neighbors4(city,iy,ix,0x30,0x3C); //roads

		if ((n & N_W) || (n & N_E)) city[iy*CITYWIDTH+ix] = 0x30; //h bridge
		if ((n & N_N) || (n & N_S)) city[iy*CITYWIDTH+ix] = 0x31; //v bridge

	    } else if ((v >= 0x32) && (v <= 0x3C)) {

		//this fixes regular roads.

		put_proper_road(city,ix,iy);
	    }
	}
    }	
    return 0;
}

