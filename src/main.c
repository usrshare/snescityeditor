#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "defines.h"
#include "pngmap.h"

#ifndef __CYGWIN__
// fixing a cygwin error
extern char *optarg;
extern int optind, opterr, optopt;
#endif

enum prgmode {
	MODE_NONE,
	MODE_FIX, //do not replace map, only fix cksum
	MODE_EXPORT, //save map from city into PNG
	MODE_IMPORT, //replace map from PNG into city
	MODE_CREATE, //create new SRAM file from PNG map
};

char* namechars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ,.-";
const size_t cityheader[2] = {0, 0x7FF0};
const size_t cityoffset[2] = {0x10, 0x4000};

#define CITYSIZE 0x3FE0 //the size of city data
#define CITYMAPSTART 0xBF0 //offset for the city map
#define CITYMAPLEN 0x3400 //size of the city map

int city_decompress (const uint16_t* in, uint16_t* out, size_t* outsz) { 
	//reimplementation of a procedure located at 03D15F in the American ROM.
	//this one handles 0x4000 ~ 0x7FFF and 0xC000 ~ 0xFFFE?

	size_t inpos = 0;
	size_t outpos = 0;

	uint16_t v = 0;

	int increment = 0;

	while ((v = in[inpos]) != 0xFFFF) {

		if (v & 0x4000) {

			uint8_t c = ((v & 0x3C00) >> 10) + 1;
			v = (v & 0x3FF);

			for (int i=0; i < c; i++)
				out[outpos++] = v + c;
			inpos++;

		} else {
			out[outpos++] = in[inpos++];
		}
	}

	out[outpos] = 0xFFFF;
	printf("1: Decoded %#04X bytes into %#04X bytes.\n",inpos*2,outpos*2);
	if (outsz) *outsz = outpos*2;
	return 0; //EOF


}

int city_decompress2 (const uint16_t* in, uint16_t* out, size_t* outsz) {
	//this function handles 0x0400 ~ 0x3FFF and such.

	size_t inpos = 0;
	size_t outpos = 0;

	uint16_t v = 0;

	while ((v = in[inpos]) != 0xFFFF) {


		if (v & 0x3C00) {

			uint8_t c = ((v & 0x3C00) >> 10) + 1;
			v = (v & 0x03FF);

			for (int i=0; i < c; i++)
				out[outpos++] = v;
			inpos++;

		} else {
			out[outpos++] = in[inpos++];
		}
	}

	out[outpos] = 0xFFFF;
	printf("2: Decoded %#04X bytes into %#04X bytes.\n",inpos*2,outpos*2);
	if (outsz) *outsz = outpos*2;
	return 0; //EOF

}

int city_decompress3 (const uint16_t* in, uint16_t* out, size_t* outsz) {
	//this function handles the 0x8000 ~ 0xFFFE bytes,
	//which are used for placing 3x3 buildings.

	//WARNING: this function assumes that the "out" array is zeroed out.

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
				out[outpos+120+i] = v+i+3;
				out[outpos+240+i] = v+i+6;
			}
			inpos++;
			outpos += 3;

		} else {
			out[outpos++] = in[inpos++];
		}
	}

	out[outpos] = 0xFFFF;
	printf("3: Decoded %#04X bytes into %#04X bytes.\n",inpos*2,outpos*2);
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
				printf("Unknown command byte at %#04X: %02X -> %02X %02X %02X\n",incur - in,v,incur[1],incur[2],incur[3]);
				incur += 1;
				//return 0;
				break;
		}

	}

	if (v == 0xFF) {

		printf("Decoded %#04X bytes into %#04X bytes.\n",incur-in-1,outcur-out);
		if (outsz) *outsz = (outcur-out);
		return 0; //EOF
	}


}

int city_compress (const uint16_t* in, uint16_t* out, size_t* outsz) {	

	size_t inpos = 0;
	size_t outpos = 0;
	
	uint16_t v = 0;
	
	while ((v = in[inpos]) != 0xFFFF) {

		int repeats = 1;
		while ((in[inpos + repeats] == in[inpos]) && (repeats < 16)) repeats++;

		v = v | ((repeats-1) * 0x400);

		out[outpos++] = v;
		inpos += repeats;
	}

	out[outpos] = 0xFFFF;
	printf("C: Encoded %#04X bytes into %#04X bytes.\n",inpos*2,outpos*2);
	if (outsz) *outsz = outpos*2;
	return 0; //EOF
}


int city2png (const char* sfname, const char* mfname, int citynum) {

	FILE* cityfile = fopen(sfname, "rb");
	if (!cityfile) { perror("fopen city file"); return 1; }

	// read city

	fseek(cityfile,cityoffset[citynum] + 0x66,SEEK_SET); // city name location

	char cityname[9];
	uint8_t namelen = 0;
	fread(&namelen,1,1,cityfile);
	fread(cityname,namelen,1,cityfile);
	for (int i=0; i < namelen; i++) cityname[i] = namechars[cityname[i]];
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

	FILE* dbgfile = fopen("debug.bin","wb");

	fwrite(citydata,citysize,1,dbgfile);

	fclose(dbgfile);

	write_png_map(mfname,citydata);

	fclose(cityfile);
}
int vtile(int y, int x) { return ( (y > 0) && (y < (CITYHEIGHT-1)) && (x > 0) && (x < (CITYWIDTH-1)) ); }

enum neighbors {
	N_X = 0,
	N_NW = 1,
	N_N = 2,
	N_NE = 4,
	N_W = 8,
	N_E = 16,
	N_SW = 32,
	N_S = 64,
	N_SE = 128
};

int check_neighbors4(uint16_t* citydata, int y, int x, uint16_t tile_min, uint16_t tile_max) {

	int r = 0;
	if ((y > 0) && ((citydata[(y-1) * CITYWIDTH + (x)] >= tile_min)
	                      && (citydata[(y-1) * CITYWIDTH + (x)] <= tile_max)))
	        		      r |= N_N;
	if ((x > 0) && ((citydata[(y) * CITYWIDTH + (x-1)] >= tile_min)
	            && (citydata[(y) * CITYWIDTH + (x-1)] <= tile_max)))
	        		      r |= N_W;
	if ((x<119) && ((citydata[(y) * CITYWIDTH + (x+1)] >= tile_min)
	            && (citydata[(y) * CITYWIDTH + (x+1)] <= tile_max)))
	        		      r |= N_E;
	if ((y <99) && ((citydata[(y+1) * CITYWIDTH + (x)] >= tile_min)
	            && (citydata[(y+1) * CITYWIDTH + (x)] <= tile_max)))
	        		      r |= N_S;
	return r;
}
int check_neighbors(uint16_t* citydata, int y, int x, uint16_t tile_min, uint16_t tile_max) {

	int r = 0;
	if ((x > 0) && (y > 0) && ((citydata[(y-1) * CITYWIDTH + (x-1)] >= tile_min)
	                      && (citydata[(y-1) * CITYWIDTH + (x-1)] <= tile_max)))
	        		      r |= N_NW;
	if (           (y > 0) && ((citydata[(y-1) * CITYWIDTH + (x)] >= tile_min)
	                      && (citydata[(y-1) * CITYWIDTH + (x)] <= tile_max)))
	        		      r |= N_N;
	if ((x<119) && (y > 0) && ((citydata[(y-1) * CITYWIDTH + (x+1)] >= tile_min)
	                      && (citydata[(y-1) * CITYWIDTH + (x+1)] <= tile_max)))
	        		      r |= N_NE;
	if ((x > 0)            && ((citydata[(y) * CITYWIDTH + (x-1)] >= tile_min)
	                      && (citydata[(y) * CITYWIDTH + (x-1)] <= tile_max)))
	        		      r |= N_W;
	if ((x > 0)            && ((citydata[(y) * CITYWIDTH + (x+1)] >= tile_min)
	                      && (citydata[(y) * CITYWIDTH + (x+1)] <= tile_max)))
	        		      r |= N_E;
	if ((x > 0) && (y <99) && ((citydata[(y+1) * CITYWIDTH + (x-1)] >= tile_min)
	                      && (citydata[(y+1) * CITYWIDTH + (x-1)] <= tile_max)))
	        		      r |= N_SW;
	if (           (y <99) && ((citydata[(y+1) * CITYWIDTH + (x)] >= tile_min)
	                      && (citydata[(y+1) * CITYWIDTH + (x)] <= tile_max)))
	        		      r |= N_S;
	if ((x<119) && (y <99) && ((citydata[(y+1) * CITYWIDTH + (x+1)] >= tile_min)
		              && (citydata[(y+1) * CITYWIDTH + (x+1)] <= tile_max)))
				      r |= N_SE;
	return r;
}

int city_improve (uint16_t* city) {
	
	//this option makes the map look better.

	// let's fix any coastal areas first.
	
	for (int iy = 0; iy < CITYHEIGHT; iy++) {
		for (int ix=0; ix < CITYWIDTH; ix++) {

			int alttile = ( rand() & 1 ) ? 8 : 0; //use alternative tile?

			if ((city[iy * CITYWIDTH + ix] < 2) || (city[iy*CITYWIDTH+ix] > 0x13)) continue;

			int n = check_neighbors4(city,iy,ix,1,0x13); //water

			if (n==0) city[iy*CITYWIDTH+ix] = 0;
		
			if ((n == N_W) || (n == N_S) || (n == N_N) || (n == N_E)) city[iy*CITYWIDTH+ix] = 0;

			if ((n & N_W) && (n & N_S) && (n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = 0x1; //water

			if ((~n & N_W) && (n & N_S) && (~n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x4; //NW
			if ((n & N_W) && (n & N_S) && (~n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x5; //N
			if ((n & N_W) && (n & N_S) && (~n & N_N) && (~n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x6; //NE
			if ((n & N_W) && (n & N_S) && (n & N_N) && (~n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x8; //E
			if ((n & N_W) && (~n & N_S) && (n & N_N) && (~n & N_E)) city[iy*CITYWIDTH+ix] = 0xB; //SE
			if ((n & N_W) && (~n & N_S) && (n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0xA; //S
			if ((~n & N_W) && (~n & N_S) && (n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x9; //SW
			if ((~n & N_W) && (n & N_S) && (n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x7; //W
		}
	}	
	
	// next step: proper forests.

	for (int iy = 0; iy < CITYHEIGHT; iy++) {
		for (int ix=0; ix < CITYWIDTH; ix++) {

			int alttile = ( rand() & 1 ) ? 9 : 0; //use alternative tile?

			if ((city[iy * CITYWIDTH + ix] < 0x14) || (city[iy*CITYWIDTH+ix] > 0x25)) continue;

			int n = check_neighbors4(city,iy,ix,0x14,0x25); //water

			if (n==0) city[iy*CITYWIDTH+ix] = 0;
		
			if ((n == N_W) || (n == N_S) || (n == N_N) || (n == N_E)) city[iy*CITYWIDTH+ix] = 0;

			if ((n & N_W) && (n & N_S) && (n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x18; //water

			if ((~n & N_W) && (n & N_S) && (~n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x14; //NW
			if ((n & N_W) && (n & N_S) && (~n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x15; //N
			if ((n & N_W) && (n & N_S) && (~n & N_N) && (~n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x16; //NE
			if ((n & N_W) && (n & N_S) && (n & N_N) && (~n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x19; //E
			if ((n & N_W) && (~n & N_S) && (n & N_N) && (~n & N_E)) city[iy*CITYWIDTH+ix] = 0x1C; //SE
			if ((n & N_W) && (~n & N_S) && (n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x1C; //S
			if ((~n & N_W) && (~n & N_S) && (n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x1A; //SW
			if ((~n & N_W) && (n & N_S) && (n & N_N) && (n & N_E)) city[iy*CITYWIDTH+ix] = alttile + 0x17; //W
		}
	}	
}


int fixcksum (uint8_t* citysram) {
	
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
	if (!cityfile) { perror("Unable to open city file"); return 1; }

	fixcksum(citysram);

	fwrite(citysram,0x8000,1,cityfile);

	fclose(cityfile);	
}


int png2city (const char* sfname, const char* mfname, int citynum, int improve) {
	//This procedurre shall load a city from a PNG map, improve its looks if necessary, then create a savefile with a city based on that map in it.
	
	uint16_t citydata[CITYWIDTH * CITYHEIGHT];
	memset(citydata,0,sizeof citydata);

	int r = read_png_map(mfname, citydata);	
	if (r != 0) {
		fprintf(stderr,"Failed to read the PNG city map.\n");
		return 1;
	}

	if (improve) city_improve(citydata);

	uint16_t citycomp[CITYWIDTH * CITYHEIGHT];
	
	size_t citysize = 0;

	city_compress((const uint16_t*)citydata,(uint16_t*)citycomp,&citysize);
	
	uint8_t citysram [0x8000];
	memset(citysram,0,sizeof citysram);
	
	FILE* cityfile = fopen(sfname, "rb");
	if (!cityfile) { perror("Unable to open city file. Will create new city file"); } else {

	fread(citysram,0x8000,1,cityfile);
	fclose(cityfile);

	}
	
	cityfile = fopen(sfname,"wb");
	if (!cityfile) { perror("Unable to open city file"); return 1; }

	memcpy(citysram + cityoffset[citynum] + CITYMAPSTART, citycomp, citysize + 2);

	for (int i=0; i < 2; i++)
		citysram[cityheader[i] + 5 + citynum] = 1; //1 means city exists

	fixcksum(citysram);

	fwrite(citysram,0x8000,1,cityfile);

	fclose(cityfile);	


}

void exit_usage_error(char** argv) {
	printf("Usage: %s -<ceif> [-2] [-x #] snescity.srm [citymap.png]\n"
	       " -c: create an SRAM file based on PNG map\n"
	       " -e: export map from SRAM into PNG\n"
	       " -i: import map from PNG into SRAM\n"
	       " -f: fix SRAM file's checksum\n"
	       " -2: operate on the second city\n"
	       " -x #: set level of map improvement.\n"
	       "       if more than 0, this tool will fix shoreline and forests.\n"
	       "\n",argv[0]); exit(1);}

int main (int argc, char** argv) {

	enum prgmode mode = MODE_NONE;
	int citynum = 0;
	int improve = 0;

	int c = -1;
	while ( (c = getopt(argc,argv,"ceif2x:")) != -1) {
		switch(c) {

			case 'c':
				mode = MODE_CREATE;
				break;
			case 'e':
				mode = MODE_EXPORT;
				break;
			case 'i':
				mode = MODE_IMPORT;
				break;
			case 'f':
				mode = MODE_FIX;
				break;
			case 'x':
				improve = atoi(optarg);
				break;
			case '2':
				citynum = 1;
				break;
		}
	}

	if (mode == MODE_NONE) exit_usage_error(argv);

	const char* sfname = argv[optind];
	const char* mfname = argv[optind+1];

	switch(mode) {
		case MODE_CREATE:
			fprintf(stderr,"Create mode not implemented yet.\n");
			break;
		case MODE_EXPORT:
			if ((!sfname) || (!mfname)) exit_usage_error(argv);
			city2png(sfname,mfname,citynum);
			break;
		case MODE_IMPORT:
			if ((!sfname) || (!mfname)) exit_usage_error(argv);
			png2city(sfname,mfname,citynum,improve);
			break;
		case MODE_FIX:
			if ((!sfname)) exit_usage_error(argv);
			fixsram(sfname);
		default:
			break;
	}
	//
	//	FILE* simfile = fopen(sfname,"rb");
	//
	//	if (!simfile) { perror("fopen city file"); return 1; }
	//
	//	FILE* mapfile = fopen(mfname,"rb");
	//
	//	if (!mapfile) {
	//		perror("fopen map file"); return 1; }
	//
	//	char pnghead[8];
	//
	//	if (fread(pnghead,1,8,dfile) != 8) { 
	//		perror("read data"); return 1;}
	//
	//	if (png_sig_cmp(pnghead,0,8) != 0) { 
	//		fprintf(stderr,"data file is not a PNG!\n"); return 1;}
	//
	//	png_structp d_png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	//	if (!d_png) {fprintf(stderr,"Can't create png_structp for data img\n"); return 1;}
	//
	//	png_infop d_inf = png_create_info_struct(d_png);
	//
	//	png_init_io(d_png, dfile);
	//	png_set_sig_bytes(d_png, 8);
	//
	//	png_read_info(d_png, d_inf);
	//
	//	uint32_t d_w = png_get_image_width(d_png, d_inf);
	//	uint32_t d_h = png_get_image_height(d_png, d_inf);
	//
	//	if ((d_w != 160) || (d_h != 205)) {
	//		fprintf(stderr,"Data image doesn't match PICO-8's dimensions.\n"
	//				"PICO-8 cartridge PNGs have a size of 160x205.\n");
	//		return 1;}
	//
	//	int d_col = png_get_color_type(d_png, d_inf);
	//	if (d_col != PNG_COLOR_TYPE_RGB_ALPHA) {
	//		fprintf(stderr,"Data image should be 8-bit RGBA.\n"); return 1; }
	//
	//
	//	int d_passes = png_set_interlace_handling(d_png);
	//	png_read_update_info(d_png,d_inf);
	//
	//	png_bytep* d_rowptrs = malloc(sizeof(png_bytep) * d_h);
	//	for (int y=0; y < d_h; y++) d_rowptrs[y] = malloc(png_get_rowbytes(d_png, d_inf));
	//
	//	png_read_image(d_png, d_rowptrs);
	//	fclose(dfile);
	//
	//	if (fread(pnghead,1,8,pfile) != 8) { 
	//		perror("read picture"); return 1;}
	//
	//	if (png_sig_cmp(pnghead,0,8) != 0) { 
	//		fprintf(stderr,"picture file is not a PNG!\n"); return 1;}
	//
	//	png_structp p_png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	//	if (!p_png) {fprintf(stderr,"Can't create png_structp for picture img\n"); return 1;}
	//
	//	png_infop p_inf = png_create_info_struct(p_png);
	//
	//	png_init_io(p_png, pfile);
	//	png_set_sig_bytes(p_png, 8);
	//
	//	png_read_info(p_png, p_inf);
	//
	//	uint32_t p_w = png_get_image_width(p_png, p_inf);
	//	uint32_t p_h = png_get_image_height(p_png, p_inf);
	//
	//	if ((p_w != 160) || (p_h != 205)) {
	//		fprintf(stderr,"Picture image doesn't match PICO-8's dimensions.\n"
	//				"PICO-8 cartridge PNGs have a size of 160x205.\n");
	//		return 1;}
	//
	//	int p_col = png_get_color_type(p_png, p_inf);
	//
	//	if (p_col != PNG_COLOR_TYPE_RGB_ALPHA)
	//		printf("Picture PNG is not RGBA. Trying to convert...\n");
	//
	//	switch(p_col) {
	//
	//		case PNG_COLOR_TYPE_PALETTE:
	//			png_set_palette_to_rgb(p_png);
	//			if (png_get_valid(p_png,p_inf,PNG_INFO_tRNS))
	//				png_set_tRNS_to_alpha(p_png);
	//			png_set_add_alpha(p_png, 0xFF, PNG_FILLER_AFTER);
	//			break;
	//		case PNG_COLOR_TYPE_GRAY:
	//			png_set_gray_to_rgb(p_png);
	//			png_set_add_alpha(p_png, 0xFF, PNG_FILLER_AFTER);
	//			break;
	//		case PNG_COLOR_TYPE_GRAY_ALPHA:
	//			png_set_gray_to_rgb(p_png);
	//			break;
	//		case PNG_COLOR_TYPE_RGB:
	//			png_set_add_alpha(p_png, 0xFF, PNG_FILLER_AFTER);
	//			break;
	//	}
	//
	//	int p_passes = png_set_interlace_handling(p_png);
	//	png_read_update_info(p_png,p_inf);
	//
	//	png_bytep* p_rowptrs = malloc(sizeof(png_bytep) * p_h);
	//	for (int y=0; y < p_h; y++) p_rowptrs[y] = malloc(png_get_rowbytes(p_png, p_inf));
	//
	//	png_read_image(p_png, p_rowptrs);
	//	fclose(pfile);
	//
	//	// ---- output file
	//
	//	FILE* ofile = fopen(ofname,"wb");
	//	if (!ofile) {
	//		perror("fopen output file"); return 1; }
	//
	//	png_structp o_png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	//	if (!o_png) {fprintf(stderr,"Can't create png_structp for output img\n"); return 1;}
	//
	//	png_infop o_inf = png_create_info_struct(o_png);
	//	if (!o_inf) {fprintf(stderr,"Can't create png_infop for output img\n"); return 1;}
	//
	//	png_init_io(o_png,ofile);
	//
	//	png_set_IHDR(o_png, o_inf, 160, 205, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	//	png_write_info(o_png, o_inf);
	//
	//	for (int iy=0; iy < 205; iy++) {
	//		for (int ix=0; ix < 160; ix++) {
	//			p_rowptrs[iy][ix*4  ] = ((p_rowptrs[iy][ix*4  ] & 0xFC) | (d_rowptrs[iy][ix*4  ] & 0x03));
	//			p_rowptrs[iy][ix*4+1] = ((p_rowptrs[iy][ix*4+1] & 0xFC) | (d_rowptrs[iy][ix*4+1] & 0x03));
	//			p_rowptrs[iy][ix*4+2] = ((p_rowptrs[iy][ix*4+2] & 0xFC) | (d_rowptrs[iy][ix*4+2] & 0x03));
	//			p_rowptrs[iy][ix*4+3] = ((p_rowptrs[iy][ix*4+3] & 0xFC) | (d_rowptrs[iy][ix*4+3] & 0x03));
	//		}
	//	}
	//
	//	png_write_image(o_png, p_rowptrs);
	//	png_write_end(o_png, NULL);
	//
	//	for (int y=0; y < p_h; y++) {
	//		free(d_rowptrs[y]);
	//		free(p_rowptrs[y]); }
	//
	//	free(d_rowptrs); free(p_rowptrs);
	//
	//	fclose(ofile);
	//
}
