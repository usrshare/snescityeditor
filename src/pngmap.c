#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <png.h>

#include "defines.h"

uint32_t pngcolors[] = {
	//this is the table we are going to use to convert raw data values into proper tiles.
	
	0xB59273, //00 land
	0x0030AC, //01 water 1
	0x212031, //02 water 2
	0x5A595A, //03 water 3
	0x1848C4, //04 shore NW
	0x1646C2, //05 shore N
	0x1444C0, //06 shore NE
	0x1242BE, //07 shore W
	0x103EBA, //08 shore E
	0x0838B4, //09 shore SW
	0x0636B2, //0A shore S
	0x0434B0, //0B shore SE
	0x1949C5, //0C shore alt NW
	0x1747C3, //0D shore alt N
	0x1545C1, //0E shore alt NE
	0x1343BF, //0F shore alt W
	0x1141BD, //10 shore alt E
	0x0F3FBB, //11 shore alt SW
	0x0D3DB9, //12 shore alt S
	0x0B3BB7, //13 shore alt SE
	0x109310, //14 forest NW
	0x0E9108, //15 forest N
	0x0C8F06, //16 forest NE
	0x0A8D04, //17 forest W
	0x008300, //18 forest
	0x008100, //19 forest E
	0x007F00, //1A forest SW
	0x007D00, //1B forest S
	0x007B00, //1C forest SE
	0x119411, //1D forest alt NW
	0x0F920F, //1E forest alt N
	0x0D900D, //2F forest alt NE
	0x0A8D04, //20 forest alt W
	0x018401, //21 forest alt
	0x018201, //22 forest alt E
	0x018001, //23 forest alt SW
	0x017E01, //24 forest alt S
	0x017C01, //25 forest alt SE
	0x20C020, //26 park
	0x20A020, //27 park with tree?
	0xE08000, //28 polluted 1
	0xE88808, //29 polluted 2
	0xeeee00, //2a explosion 1
	0xf0f000, //2b explosion 2
	0xf4f400, //2c explosion 3
	0xf8f800, //2d explosion 4
	0x00f8f8, //2e weird 1
	0x00fcfc, //2f weird 2
	0x424646, //30 horiz bridge
	0x434747, //31 vert bridge
	0x5b5b5b, //32 horiz road
	0x5c5c5c, //33 vert road
	0x5d5d5d, //34 N-E turn
	0x5e5e5e, //35 S-E turn
	0x5f5f5f, //36 S-W turn
	0x606060, //37 N-W turn
	0x616161, //38 W-N-E crossing
	0x626262, //39 N-E-S crossing
	0x636363, //3a W-S-E crossing
	0x646464, //3b N-W-S crossing
	0x656565, //3c 4-way crossing
	0x666666, //3d horiz road w/ power
	0x676767, //3e vert road w/ power
	0x686868, //3f unknown tile.
	0x424646, //40 road+traffic 1
	0x434747, //41 road+traffic 2
	0x5b5b5b, //42 road+traffic 3
	0x5c5c5c, //43 road+traffic 4
	0x5d5d5d, //44 road+traffic 5
	0x5e5e5e, //45 road+traffic 6
	0x5f5f5f, //46 road+traffic 7
	0x606060, //47 road+traffic 8
	0x616161, //48 road+traffic 9
	0x626262, //49 road+traffic 10
	0x636363, //4a road+traffic 11
	0x646464, //4b road+traffic 12
	0x656565, //4c road+traffic 13
	0x666666, //4d road+traffic 14
	0x676767, //4e road+traffic 15
	0x686868, //4f road+traffic 16 
	0x424646, //50 road+more traffic 1
	0x434747, //51 road+more traffic 2
	0x5b5b5b, //52 road+more traffic 3
	0x5c5c5c, //53 road+more traffic 4
	0x5d5d5d, //54 road+more traffic 5
	0x5e5e5e, //55 road+more traffic 6
	0x5f5f5f, //56 road+more traffic 7
	0x606060, //57 road+more traffic 8
	0x616161, //58 road+more traffic 9
	0x626262, //59 road+more traffic 10
	0x636363, //5a road+more traffic 11
	0x646464, //5b road+more traffic 12
	0x656565, //5c road+more traffic 13
	0x666666, //5d road+more traffic 14
	0x676767, //5e road+more traffic 15
	0x686868, //5f road+more traffic 16 
};

const size_t pngcolor_c = sizeof(pngcolors) / sizeof(*pngcolors);

void setpixel(png_byte* pixel,uint16_t value) {
	
	for (int i=0; i < 3; i++)
	pixel[i] = (uint8_t) (pngcolors[value] >> (16-(8*i))) ;
}

int write_png_map (const char* pngname, const uint16_t* citydata) {
	
	FILE* mapfile = fopen(pngname, "wb");
	if (!mapfile) { perror("fopen map file"); return 1; }
	
	// create PNG

	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fprintf(stderr, "Could not allocate write struct\n");
		return 1;
	}

	// Initialize info structure
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fprintf(stderr, "Could not allocate info struct\n");
		return 1;
	}

	png_init_io(png_ptr, mapfile);

	png_set_IHDR(png_ptr,info_ptr,CITYWIDTH, CITYHEIGHT,8,PNG_COLOR_TYPE_RGB,PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_BASE,PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);

	png_byte row[3 * CITYWIDTH * sizeof(png_byte)];


	for (int iy=0; iy < CITYHEIGHT; iy++) {

		for (int ix=0; ix < CITYWIDTH; ix++) {

			if (citydata[iy*CITYWIDTH+ix] < pngcolor_c)
				setpixel(row+(ix*3),citydata[iy*CITYWIDTH+ix]);
			else {
			row[ix*3+0] = 255;
			row[ix*3+1] = (citydata[(iy * CITYWIDTH + ix)] >> 8);
			row[ix*3+2] = (citydata[(iy * CITYWIDTH + ix)] & 0xFF);
			}
		}
		png_write_row(png_ptr,row);
	}

	png_write_end(png_ptr,NULL);

	if (info_ptr != NULL) png_free_data(png_ptr,info_ptr,PNG_FREE_ALL,-1);
	if (png_ptr != NULL) png_destroy_write_struct(&png_ptr,NULL);
	
	fclose(mapfile);

	return 0;

}

int read_png_map(const char* pngname, uint16_t* citydata) {

	FILE* mapfile = fopen(pngname, "rb");
	if (!mapfile) { perror("fopen map file"); return 1; }
	
	// read PNG
	
	char pnghead[8];
	
	if (fread(pnghead,1,8,mapfile) != 8) { 
		perror("read data"); return 1;}
	
	if (png_sig_cmp(pnghead,0,8) != 0) { 
		fprintf(stderr,"Map file is not a PNG!\n"); return 1;}

	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fprintf(stderr, "Could not allocate read struct\n");
		return 1;
	}

	// Initialize info structure
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fprintf(stderr, "Could not allocate info struct\n");
		return 1;
	}

	png_init_io(png_ptr, mapfile);
	
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	uint32_t d_w = png_get_image_width(png_ptr, info_ptr);
	uint32_t d_h = png_get_image_height(png_ptr, info_ptr);

	if ((d_w != 120) || (d_h != 100)) {
		fprintf(stderr, "This image is not a valid city map.\n"
			        "City maps have a resolution of 120x100 pixels.\n");
		return 1;}

	int d_col = png_get_color_type(png_ptr, info_ptr);
	if (d_col != PNG_COLOR_TYPE_RGB) {
		fprintf(stderr,"Map image should be 8-bit RGB.\n"); return 1; }

	int d_passes = png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr,info_ptr);

	png_bytep* d_rowptrs = malloc(sizeof(png_bytep) * d_h);
	for (int y=0; y < d_h; y++) d_rowptrs[y] = malloc(png_get_rowbytes(png_ptr, info_ptr));

	png_read_image(png_ptr, d_rowptrs);

	for (int iy=0; iy < CITYHEIGHT; iy++) {
		for (int ix=0; ix < CITYWIDTH; ix++) {

			uint32_t rgb = (d_rowptrs[iy][ix*3] << 16) + (d_rowptrs[iy][ix*3+1] << 8) + d_rowptrs[iy][ix*3+2];
			uint16_t blk = 0xFFFF; for (int i=0; i < pngcolor_c; i++) if (rgb == pngcolors[i]) {blk = i; citydata[iy * CITYWIDTH + ix] = i; break; }
			if ((blk == 0xFFFF) && (rgb & 0xFF0000)) citydata[iy * CITYWIDTH + ix] = (rgb & 0xFFFF);
		}
	}
	
	fclose(mapfile);
	return 0;
}
