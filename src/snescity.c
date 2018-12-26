#include "snescity.h"

#include <inttypes.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "defines.h"
#include "pngmap.h"

#ifndef NESMODE
unsigned char* namechars = (unsigned char*) "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ,.- ";
#endif

#ifdef NESMODE
#define NUMBER_OF_CITIES 1
const size_t cityheader[1] = {0x0865}; //actual SRAM data starts here.
//the NES game actually stores temporary data in the first part of SRAM!
const size_t cityoffset[1] = {0x0867}; //city data starts here.

const uint8_t nes_loadvalues[256] = {
	[0x90] = 0, //land
	[0xBD] = 1, [0xFE] = 2, [0xFF] = 3, //water
	[0XBE] = 0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b, //shore
	[0xC6] = 0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13, //shore
	[0xCE] = 0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C, //forest
	[0xD7] = 0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25, //forest
	[0xB2] = 0x26,0x27, //park

	[0xE0] = 0x30,0x31, //road over water
	[0x91] = 0x32,0x33,0x35,0x36,0x34,0x37,0x38,0x39,0x3a,0x3b,0x3c, //roads
	[0xB9] = 0x3d,0x3e, //roads with power lines across

	[0xe4] = 0x60,0x61, //power line over water
	[0xA7] = 0x62,0x63,0x65,0x66,0x64,0x67,0x68,0x69,0x6a,0x6b,0x6c, //power lines
	[0xbb] = 0x6d,0x6e, //rails with power lines across

	[0xE2] = 0x70,0x71, //rails over water
	[0x9C] = 0x72,0x73,0x75,0x76,0x74,0x77,0x78,0x79,0x7a,0x7b,0x7c, //rails
	[0xB7] = 0x7d,0x7e, //rails over road
	[0xE6] = 0x7f, //fire	

};

uint8_t nes_savevalues_ready = 0;
uint8_t nes_savevalues[256];

#else
#define NUMBER_OF_CITIES 2
const size_t cityheader[2] = {0, 0x7FF0};
const size_t cityoffset[2] = {0x10, 0x4000};

const mapvalues[] = {};
#endif

#define MAPVALUES_C ( sizeof(mapvalues) / sizeof(mapvalues[0]) )

const char* months[] = {"???","JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};

char* city_lasterror = "no error.";

#ifdef NESMODE

#define SRAM_OFFSET 0x0865 //offset for the actual SRAM

#define CITYMAPSTART 0x0078 //offset for the city map: 120 bytes (before that is graph data)
#define CITYMAPLEN 0x1690 //the size of city data
#define CITYNAMEOFFSET (CITYMAPSTART + CITYMAPLEN) //offset for city name
#else

#define SRAM_OFFSET 0

#define CITYSIZE 0x3FE0 //the size of city data
#define CITYMAPSTART 0xBF0 //offset for the city map
#define CITYMAPLEN 0x3400 //size of the city map
#define CITYNAMEOFFSET 0x66 //offset for city name
#endif

int find_png_filename(const char* filename, char* o_f) {

	const char* nstart = filename;

	char* x = strrchr(filename,'/');
	if (x) nstart = x+1; else {
		x = strrchr(filename,'\\');
		if (x) nstart = x+1;
	}

	size_t strl = strlen(nstart);
	char* d = strrchr(filename,'.');

	if (d) strl = (d-nstart);
	strncpy(o_f,nstart,(strl < 8) ? strl : 8);
	return strl;
}

#ifdef NESMODE
int city_convert_from_nes (const uint8_t* in, uint16_t* out) {

	size_t inpos = 0, outpos = 0;

	while (outpos < (CITYWIDTH * CITYHEIGHT)) {
		uint8_t v = in[inpos];

		out[outpos] = nes_loadvalues[v];

		inpos++; outpos++;
	}
	return 0;
}

int city_convert_to_nes (const uint16_t* in, uint8_t* out) {

	if (!nes_savevalues_ready) {

		memset(nes_savevalues, 0, sizeof nes_savevalues);

		nes_savevalues[0] = 0x90;

		for (int i=0; i < 256; i++) {
			uint8_t l = nes_loadvalues[i];
			if ((l) && (nes_savevalues[l] == 0)) nes_savevalues[l] = i; }

		nes_savevalues_ready = 1;
	}

	size_t inpos = 0, outpos = 0;

	while (outpos < (CITYWIDTH * CITYHEIGHT)) {
		uint16_t v = in[inpos];

		out[outpos] = nes_savevalues[v];

		inpos++; outpos++;
	}
	return 0;
}
#endif

int city_decompress (const uint16_t* in, uint16_t* out, size_t* outsz) { 
	//reimplementation of a procedure located at 03D15F in the American ROM.
	//this one handles 0x4000 ~ 0x7FFF and 0xC000 ~ 0xFFFE?

	//words that have bit 14 enabled function as backref.
	// v
	//01CCCCLL LLLLLLLL
	//the following C tiles are repeated from
	//the last L bytes found.
	//if L < C, then the L-byte pattern is repeated.

	size_t inpos = 0;
	size_t outpos = 0;

	uint16_t v = 0;

	while ((v = in[inpos]) != 0xFFFF) {

		if (v & 0x4000) {

			uint8_t c = ((v & 0x3C00) >> 10);
			v = (v & 0x03FF);

			// these operations work on single bytes.
			size_t rem = 2*c;
			size_t i = 0;
			while (rem > 0) {
				memcpy((uint8_t*)(out + outpos)+i,(uint8_t*)(out + outpos) - v,rem < v ? rem : v);
				i += v;
				rem -= v;
			}

			outpos += c;
			inpos++;

		} else {
			out[outpos++] = in[inpos++];
		}
	}

	out[outpos] = 0xFFFF;
	printf("1: Decoded %#04zX bytes into %#04zX bytes.\n",inpos*2,outpos*2);
	if (outsz) *outsz = outpos*2;
	return 0; //EOF


}

int city_decompress2 (const uint16_t* in, uint16_t* out, size_t* outsz) {
	//this function handles 0x0400 ~ 0x3FFF and such.

	//words that have bits 10~13 enabled work as simple RLE.
	//  vvvv
	//T0CCCCTT TTTTTTTT
	//the tile specified by T is repeated C+1 times.

	size_t inpos = 0;
	size_t outpos = 0;

	uint16_t v = 0;

	while ((v = in[inpos]) != 0xFFFF) {


		if (v & 0x3C00) {

			uint8_t c = ((v & 0x3C00) >> 10) + 1;
			v = (v & 0x83FF);

			for (int i=0; i < c; i++)
				out[outpos++] = v;
			inpos++;

		} else {
			out[outpos++] = in[inpos++];
		}
	}

	out[outpos] = 0xFFFF;
	printf("2: Decoded %#04zX bytes into %#04zX bytes.\n",inpos*2,outpos*2);
	if (outsz) *outsz = outpos*2;
	return 0; //EOF

}

int city_decompress3 (const uint16_t* in, uint16_t* out, size_t* outsz) {
	//this function handles the 0x8000 ~ 0xFFFE bytes,
	//which are used for placing 3x3 buildings.

	//WARNING: this function assumes that the "out" array is zeroed out.
	//
	//words that have bit 15 set place 3x3 sized buildings.
	//v
	//100000TT TTTTTTTT
	//the buildings are placed in the tiles immediately below and to the right of
	//the tile currently being decompressed.

	size_t inpos = 0;
	size_t outpos = 0;

	uint16_t v = 0;

	while ((v = in[inpos]) != 0xFFFF) {

		while (out[outpos] != 0) outpos++; //means we already placed something there. this can only happen if a 3x3 building was placed.

		if (v & 0x8000) {

			uint8_t c = 3;
			v = (v & 0x7FFF);

			for (int i=0; i < c; i++) {
				out[outpos+i] = v+i;
				out[outpos+CITYWIDTH+i] = v+i+3;
				out[outpos+(CITYWIDTH*2)+i] = v+i+6;
			}
			inpos++;
			outpos += 3;

		} else {
			out[outpos++] = in[inpos++];
		}
	}

	out[outpos] = 0xFFFF;
	printf("3: Decoded %#04zX bytes into %#04zX bytes.\n",inpos*2,outpos*2);
	if (outsz) *outsz = outpos*2;
	return 0; //EOF

}

int gfx_decompress (const uint8_t* in, uint8_t* out, size_t* outsz) {

	const uint8_t* incur = in;
	uint8_t* outcur = out;

	uint8_t v = 0;

	while ((v = *incur) != 0xFF) {
		//binary format: CCCVVVVV.

		uint8_t command = v >> 5;
		uint8_t length = v & 0x1F;

		switch (command) {

			case 0:
				// copy VVVVV+1 bytes directly into output.
				memcpy(outcur,incur+1,length + 1);
				incur += (length + 2);
				outcur += (length + 1);
				break;

			case 1:
				// copy the following byte VVVVV+1 times.
				memset(outcur,incur[1],length + 1);
				incur += 2;
				outcur += (length + 1);
				break;

			case 2:
				// copy the following two bytes into the next VVVVV+1 bytes.
				for (int i=0; i < length + 1; i++)
					outcur[i] = incur[1 + (i & 1)];
				incur += 3;
				outcur += (length + 1);
				break;

			case 3:
				// copy a rising sequence of VVVVV+1 bytes that starts with the following byte.
				for (int i=0; i < length + 1; i++)
					outcur[i] = (incur[1])+i;
				incur += 2;
				outcur += (length + 1);
				break;

			case 4:
			case 7:
				// copy a VVVVV+1 byte long sequence of previously decompressed data.
				// read said data from the first bytes unpacked, specified by next byte.
				memcpy(outcur,out + incur[1],(length + 1));
				incur += 2;
				outcur += (length + 1);
				break;

			case 6:
				// copy a VVVVV+1 byte long sequence of previously decompressed data.
				// read said data from the last bytes unpacked, specified by next byte.
				memcpy(outcur,outcur - incur[1],(length + 1));
				incur += 2;
				outcur += (length + 1);
				break;

			default:
				printf("Unknown command byte at %#04zX: %02X -> %02X %02X %02X\n",incur - in,v,incur[1],incur[2],incur[3]);
				incur += 1;
				//return 0;
				break;
		}

	}

	if (v == 0xFF) {

		printf("Decoded %#04zX bytes into %#04zX bytes.\n",incur-in-1,outcur-out);
		if (outsz) *outsz = (outcur-out);
		return 0; //EOF
	}
	return 1;
}

int city_compress (const uint16_t* in, uint16_t* out, size_t* outsz, size_t max_outsz) {	

	//This method only does simple RLE compression. It doesn't encode buildings and/or patterns.
	//It may fail when asked to compress a complicated map.

	size_t inpos = 0;
	size_t outpos = 0;

	uint16_t v = 0;

	while (inpos < CITYAREA) {

		int repeats = 1;
		while ( ((inpos + repeats) < CITYAREA) && (in[inpos + repeats] == in[inpos]) && (repeats < 16) ) repeats++;

		v = in[inpos] | ((repeats-1) * 0x400);

		out[outpos++] = v;
		inpos += repeats;

		if ( (max_outsz) && (outpos >= (max_outsz/2)) ) {
			fprintf(stderr,"Unable to compress the map to fit into the buffer size.\n");
			city_lasterror = "City map too complex.";
			return 1;
		}
	}
	out[outpos++] = 0xFFFF;
	printf("C: Encoded %#04zX bytes into %#04zX bytes.\n",inpos*2,outpos*2);
	if (outsz) *outsz = outpos*2;
	return 0; //EOF
}

int loadsramcity (const char* sfname, uint16_t* citydata, int citynum, char* o_cityname) {

	FILE* cityfile = fopen(sfname, "rb");
	if (!cityfile) { perror("fopen city file"); city_lasterror = "unable to open city file."; return 1; }

	// read city
	fseek(cityfile,cityoffset[citynum] + CITYNAMEOFFSET,SEEK_SET); // city name location

	unsigned char cityname[CITYMAXLEN+1];
	uint8_t namelen = 0;
	fread(&namelen,1,1,cityfile);
#ifdef NESMODE
	if (namelen > 0) namelen--;
#endif
	fread(cityname,namelen,1,cityfile);
#ifdef NESMODE
#else
	for (uint8_t i=0; i < namelen; i++) cityname[i] = namechars[cityname[i]];
#endif
	cityname[namelen] = 0;
	printf("City name: %s\n",cityname);

	if (o_cityname) strncpy(o_cityname, (char*) cityname, CITYMAXLEN+1);

	uint8_t sramfile[CITYMAPLEN];
#ifdef NESMODE

	fseek(cityfile,cityoffset[citynum] + CITYMAPSTART,SEEK_SET);
	fread(sramfile,CITYMAPLEN,1,cityfile);

	city_convert_from_nes(sramfile, citydata);

#else
	memset(sramfile,0xFF,CITYMAPLEN);	

	fseek(cityfile,cityoffset[citynum] + CITYMAPSTART,SEEK_SET);

	fread(sramfile,CITYMAPLEN,1,cityfile);

	uint16_t citytemp[CITYWIDTH * CITYHEIGHT];
	uint16_t citytemp2[CITYWIDTH * CITYHEIGHT];
	memset(citytemp,0,sizeof citytemp);
	memset(citytemp2,0,sizeof citytemp2);
	memset(citydata,0,CITYWIDTH * CITYHEIGHT * 2);

	size_t citysize = 0;

	city_decompress((const uint16_t*)sramfile,(uint16_t*)citytemp,&citysize);
	city_decompress2((const uint16_t*)citytemp,(uint16_t*)citytemp2,&citysize);
	city_decompress3((const uint16_t*)citytemp2,(uint16_t*)citydata,&citysize);
#endif

	fclose(cityfile);
	return 0;
}

int city2png (const char* sfname, const char* mfname, int citynum) {

	FILE* cityfile = fopen(sfname, "rb");
	if (!cityfile) { perror("fopen city file"); city_lasterror = "unable to open city file."; return 1; }

	// read city

	fseek(cityfile,cityoffset[citynum] + 0x66,SEEK_SET); // city name location

	unsigned char cityname[CITYMAXLEN];
	uint8_t namelen = 0;
	fread(&namelen,1,1,cityfile);
	fread(cityname,namelen,1,cityfile);
#ifdef NESMODE
#else
	for (int i=0; i < namelen; i++) cityname[i] = namechars[cityname[i]];
#endif
	cityname[namelen] = 0;
	printf("City name: %s\n",cityname);

	uint8_t sramfile[CITYMAPLEN];
	memset(sramfile,0xFF,CITYMAPLEN);	

	fseek(cityfile,cityoffset[citynum] + CITYMAPSTART,SEEK_SET);

	fread(sramfile,CITYMAPLEN,1,cityfile);

	uint16_t citytemp[CITYWIDTH * CITYHEIGHT];
	uint16_t citytemp2[CITYWIDTH * CITYHEIGHT];
	uint16_t citydata[CITYWIDTH * CITYHEIGHT];
	memset(citytemp,0,sizeof citytemp);
	memset(citytemp2,0,sizeof citytemp);
	memset(citydata,0,sizeof citydata);

	size_t citysize = 0;

	city_decompress((const uint16_t*)sramfile,(uint16_t*)citytemp,&citysize);
	city_decompress2((const uint16_t*)citytemp,(uint16_t*)citytemp2,&citysize);
	city_decompress3((const uint16_t*)citytemp2,(uint16_t*)citydata,&citysize);

	write_png_map(mfname,citydata);

	fclose(cityfile);
	return 0;
}
int vtile(int x, int y) { return ( (y >= 0) && (y < CITYHEIGHT) && (x >= 0) && (x < CITYWIDTH) ); }

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

void ror6502 ( uint8_t* byte, bool* carry ) {

	bool old_carry = *carry;
	(*carry) = ((*byte) & 1);
	(*byte) >>= 1;
	if (old_carry) (*byte) |= 0x80;
}

uint16_t _sum_calc(uint16_t cksum, uint8_t byte) {

	uint8_t _tmp4 = cksum & 0xFF;
	uint8_t _tmp5 = (cksum >> 8);

	for (int i=0; i < 8; i++) {

		bool carry = (byte & 1);
		byte >>= 1;

		ror6502 ( &_tmp5, &carry );
		ror6502 ( &_tmp4, &carry );

		if (carry) {

			_tmp4 ^= 0x05;
			_tmp5 ^= 0xC0;

		}

	}

	uint16_t r = _tmp4 + (_tmp5 << 8);


	return r;
}

int fixcksum (uint8_t* citysram) {
#ifdef NESMODE
	uint16_t cksum = 0;

	for (int i=SRAM_OFFSET + 0x2; i < 0x2000; i++) {
		uint16_t oldcksum = cksum;
		cksum = _sum_calc(cksum,citysram[i]);
		printf("cksum @ %04" PRIx16" : (%04" PRIx16 ", %02" PRIx8 ") = %04" PRIx16 "\n", 0x8000 + i, oldcksum, citysram[i], cksum);
	}
	cksum = _sum_calc(cksum, 0);
	cksum = _sum_calc(cksum, 0);

	printf("New check sum for SRAM: %04X\n",cksum);
	memcpy(citysram + SRAM_OFFSET, &cksum, 2); //checksum
	return 0;
#else
	uint16_t cksum1 = 0;
	uint16_t cksum2 = 0;

	for (int i=0; i < CITYSIZE; i++) { //everything BUT the headers at the start and end.

		cksum1 += citysram[i + 0x10]; //cityoffset(0)  / 2
		cksum2 += citysram[i + 0x4000]; //cityoffset(1) / 2
	}
	printf("New check sum for city 1: %04X\n",cksum1);
	printf("New check sum for city 2: %04X\n",cksum2);

	for (int i=0; i < 2; i++) {
		memcpy(citysram + cityheader[i],"SIM",3); //magic word
		memcpy(citysram + cityheader[i] + 10,&cksum1,2); //checksum
		memcpy(citysram + cityheader[i] + 12,&cksum2,2); //checksum
	}	

	uint16_t hdrcksum = 0;
	for (int i=0; i < 14; i++) { //the first 14 bytes of the header. (the last 2 are the checksum.)

		hdrcksum += citysram[i];
	}
	printf("New check sum for headers: %04X\n",hdrcksum);

	for (int i=0; i < 2; i++) {
		memcpy(citysram + cityheader[i] + 14,&hdrcksum,2); //checksum
	}	
#endif
	return 0;
}

int describe_cities (const char* sfname, char* city1, char* city2) {

	uint8_t citysram [0x8000];
	memset(citysram,0,sizeof citysram);

	FILE* cityfile = fopen(sfname, "rb");
	if (!cityfile) { perror("Unable to open city file."); city_lasterror = "unable to open city file."; return 1; }

	fread(citysram,0x8000,1,cityfile);
	fclose(cityfile);

	for (int ci=0; ci < NUMBER_OF_CITIES; ci++) {
#ifdef NESMODE
		uint8_t city_exists = citysram[cityoffset[ci] + CITYNAMEOFFSET + 128];
#else
		uint8_t city_exists = citysram[0x5 + ci];
#endif

		if (city_exists) {

			uint8_t namelen = citysram[cityoffset[ci] + CITYNAMEOFFSET];

			char name[CITYMAXLEN];

			uint16_t year;

#ifdef NESMODE
			namelen--;
			for (int i=0; i < namelen; i++) name[i] = citysram[cityoffset[ci] + CITYNAMEOFFSET + 1 + i];
			name[namelen] = 0;
			memcpy(&year, citysram + cityoffset[ci] + 8126, 2);
#else
			for (int i=0; i < namelen; i++) name[i] = namechars[citysram[cityoffset[ci] + CITYNAMEOFFSET + 1 + i]];
			name[namelen] = 0;
			memcpy(&year, citysram + cityoffset[ci] + 0x20, 2);
#endif



			snprintf(ci ? city2 : city1, 24, "%d. %4d %s",ci+1,year,name);
		} else {

			snprintf(ci ? city2 : city1, 24, "%d. ---- --------",ci+1);
		}

	}

	return 0;
}

int fixsram(const char* sfname) {

	uint8_t citysram [0x8000];
	memset(citysram,0,sizeof citysram);

	FILE* cityfile = fopen(sfname, "rb");
	if (!cityfile) { perror("Unable to open city file. Will create new city file"); } else {

		fread(citysram,0x8000,1,cityfile);
		fclose(cityfile);

	}

	cityfile = fopen(sfname,"wb");
	if (!cityfile) { perror("Unable to open city file"); city_lasterror = "unable to open city file."; return 1; }

	fixcksum(citysram);

	fwrite(citysram,0x8000,1,cityfile);

	fclose(cityfile);	
	return 0;
}

int replace_city(const char* sfname, const uint16_t* citydata, int citynum) {

#ifdef NESMODE

	uint8_t citycomp[CITYMAPLEN];
	memset(citycomp, 0x90, CITYMAPLEN);
	city_convert_to_nes(citydata, citycomp);

	const size_t citysize = CITYMAPLEN;

#else

	uint16_t citycomp[(CITYMAPLEN/2)];
	memset(citycomp,0, CITYMAPLEN);

	size_t citysize = 0;

	int r = city_compress(citydata,citycomp,&citysize,CITYMAPLEN);
	if (r) {
		fprintf(stderr,"Unable to compress map for SRAM.\n");
		return 1; }
#endif

	uint8_t citysram [0x8000];
	memset(citysram,0,sizeof citysram);

	FILE* cityfile = fopen(sfname, "rb");
	if (!cityfile) { perror("Unable to open city file."); return 1; }

	fread(citysram,0x8000,1,cityfile);
	fclose(cityfile);

	cityfile = fopen(sfname,"wb");
	if (!cityfile) { perror("Unable to open city file"); 
		city_lasterror = "unable to open city file.";
		return 1; }

	memcpy(citysram + cityoffset[citynum] + CITYMAPSTART, citycomp, citysize);
	memset(citysram + cityoffset[citynum] + CITYMAPSTART + citysize, 0, CITYMAPLEN - citysize);

#ifdef NESMODE
	citysram[cityoffset[0] + CITYNAMEOFFSET + 128] = 1; //1 means city exists
#else
	for (int i=0; i < 2; i++)
		citysram[cityheader[i] + 5 + citynum] = 1; //1 means city exists
#endif

	fixcksum(citysram);

	fwrite(citysram,0x8000,1,cityfile);

	fclose(cityfile);	
	return 0;
}

int write_new_city(const char* sfname, const uint16_t* citydata, const char* cityname, int citynum) {

#ifdef NESMODE
	
	uint8_t citycomp[CITYMAPLEN];
	memset(citycomp, 0x90, CITYMAPLEN);
	city_convert_to_nes(citydata, citycomp);

	const size_t citysize = CITYMAPLEN;

#else

	uint16_t citycomp[(CITYMAPLEN/2)];
	memset(citycomp,0, CITYMAPLEN);

	size_t citysize = 0;

	int r = city_compress(citydata,citycomp,&citysize,CITYMAPLEN);
	if (r) {
		fprintf(stderr,"Unable to compress map for SRAM.\n");
		return 1; }

#endif

	uint8_t citysram [0x8000];
	memset(citysram,0,sizeof citysram);

	FILE* cityfile = fopen(sfname,"wb");
	if (!cityfile) { perror("Unable to open city file"); 
		city_lasterror = "unable to open city file.";
		return 1; }

#ifdef NESMODE

	*(uint16_t*)(citysram + cityoffset[citynum] + CITYNAMEOFFSET + 76) = 500; //overall city score

	*(uint16_t*)(citysram + cityoffset[citynum] + CITYNAMEOFFSET + 78) = 20000; //money bits 0~15
	*(uint8_t*)(citysram + cityoffset[citynum] + CITYNAMEOFFSET + 80) = 0; //money bits 16~23
	
	*(uint16_t*)(citysram + cityoffset[citynum] + CITYNAMEOFFSET + 81) = 1900; //year
	*(uint8_t*)(citysram + cityoffset[citynum] + CITYNAMEOFFSET + 83) = 0; //month
	*(uint8_t*)(citysram + cityoffset[citynum] + CITYNAMEOFFSET + 84) = 0; //week
	*(uint8_t*)(citysram + cityoffset[citynum] + CITYNAMEOFFSET + 85) = 7; //tax rate
	*(uint8_t*)(citysram + cityoffset[citynum] + CITYNAMEOFFSET + 86) = 9; //options: auto-bulldoze, BGM on
	*(uint8_t*)(citysram + cityoffset[citynum] + CITYNAMEOFFSET + 87) = 0; //game speed
	*(uint16_t*)(citysram + cityoffset[citynum] + CITYNAMEOFFSET + 92) = 0xFFFF; //year to bank
	*(uint16_t*)(citysram + cityoffset[citynum] + CITYNAMEOFFSET + 94) = 0xFFFF; //year to expo
	*(uint16_t*)(citysram + cityoffset[citynum] + CITYNAMEOFFSET + 96) = 0xFFFF; //year to liberty

#else
	
	*(uint16_t*)(citysram + cityoffset[citynum] + 0x18) = 20000; //money
	*(uint8_t*)(citysram + cityoffset[citynum] + 0x1C) = 2; //speed: medium
	*(uint8_t*)(citysram + cityoffset[citynum] + 0x1E) = 13; //options: auto-bulldoze, auto-goto, no auto-budget, BGM on
	*(uint16_t*)(citysram + cityoffset[citynum] + 0x20) = 1900; //year
	*(uint8_t*)(citysram + cityoffset[citynum] + 0x22) = 1; //month
	*(uint8_t*)(citysram + cityoffset[citynum] + 0x24) = 7; //tax rate

	*(uint8_t*)(citysram + cityoffset[citynum] + 0x5C) = 2; //regular (not practice or scenario) city
	*(uint8_t*)(citysram + cityoffset[citynum] + 0x60) = 7; //not in a scenario

	*(uint16_t*)(citysram + cityoffset[citynum] + 0x74) = 0x0; //city founded
	*(uint16_t*)(citysram + cityoffset[citynum] + 0x76) = 1900; //year 1900
	*(uint16_t*)(citysram + cityoffset[citynum] + 0x78) = 1; //january

	for (int i=1; i < 10; i++) { //history
		*(uint16_t*)(citysram + cityoffset[citynum] + 0x74 + (6*i)) = 0xFFFF; //no event
	}

#endif

	char convname[CITYMAXLEN];
	size_t namelen = (strlen(cityname) < CITYMAXLEN) ? strlen(cityname) : CITYMAXLEN;

	for (int i=0; i < namelen; i++) {
#ifdef NESMODE
		convname[i] = cityname[i];
#else
		char* x = strchr((char*)namechars,cityname[i]);
		if (x) convname[i] = (x - (char*)namechars); else convname[i] = 0x27;
#endif
	}


#ifdef NESMODE
	*(uint8_t*)(citysram + cityoffset[citynum] + CITYNAMEOFFSET) = namelen + 1; //city name length
	memcpy(citysram + cityoffset[citynum] + CITYNAMEOFFSET + 1, convname, namelen); //city name
#else
	*(uint8_t*)(citysram + cityoffset[citynum] + CITYNAMEOFFSET) = namelen; //city name length
	memcpy(citysram + cityoffset[citynum] + CITYNAMEOFFSET + 1, convname, namelen); //city name

	memcpy(citysram + cityoffset[citynum] + 0x70, "\x0E\x0C\x1C", 3); //backwards 'SCE' instead of map number.

#endif
	memcpy(citysram + cityoffset[citynum] + CITYMAPSTART, citycomp, citysize);
	memset(citysram + cityoffset[citynum] + CITYMAPSTART + citysize, 0, CITYMAPLEN - citysize);

#ifdef NESMODE
	citysram[cityoffset[0] + CITYNAMEOFFSET + 128] = 1; //1 means city exists
#else
	for (int i=0; i < 2; i++)
		citysram[cityheader[i] + 5 + citynum] = 1; //1 means city exists
#endif

	fixcksum(citysram);

	fwrite(citysram,0x8000,1,cityfile);

	fclose(cityfile);
	return 0;	
}

int newcity(const char* sfname, const char* mfname, const char* cityname, int improve, int improve_flags) {

	uint16_t citydata[CITYWIDTH * CITYHEIGHT];

	int r = read_png_map(mfname, citydata);	
	if (r != 0) {
		fprintf(stderr,"Failed to read the PNG city map.\n");
		city_lasterror = "Can not read the PNG.\n";
		return 1;
	}

	if (improve) city_improve(citydata, improve_flags);

	return write_new_city(sfname,citydata,cityname,0);

}

int png2city (const char* sfname, const char* mfname, int citynum, int improve, int improve_flags) {
	//This procedurre shall load a city from a PNG map, improve its looks if necessary, then create a savefile with a city based on that map in it.

	uint16_t citydata[CITYWIDTH * CITYHEIGHT];

	int r = read_png_map(mfname, citydata);	
	if (r != 0) {
		fprintf(stderr,"Failed to read the PNG city map.\n");
		city_lasterror = "Can not read the PNG.\n";
		return 1;
	}

	if (improve) city_improve(citydata, improve_flags);

	return replace_city(sfname,citydata,0);


}

